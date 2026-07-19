#pragma once

#include <meta>
#include <string>
#include <string_view>
#include <fstream>
#include <optional>
#include <iostream>
#include <cstdlib>
#include <type_traits>
#include <charconv>
#include <cstdint>

// ============================================================
// Compile-time FNV-1a hash for fast field lookup
// ============================================================

constexpr uint32_t fnv1a_hash(std::string_view s) {
    uint32_t h = 2166136261u;
    for (auto c : s) {
        h ^= static_cast<uint8_t>(c);
        h *= 16777619u;
    }
    return h;
}

// ============================================================
// Annotation types
// ============================================================

template <typename T>
struct Range {
    T min;
    T max;
    friend constexpr bool operator==(const Range&, const Range&) = default;
};

struct Length {
    size_t min;
    size_t max;
    friend constexpr bool operator==(const Length&, const Length&) = default;
};

template <typename T>
struct Default {
    T value;
    friend constexpr bool operator==(const Default&, const Default&) = default;
};

struct Rename {
    const char *name;
    friend constexpr bool operator==(const Rename&, const Rename&) = default;
};

struct NamingConvention {
    enum Type : unsigned char { Snake, Camel, Pascal, Upper, Kebab };
    Type type;
    friend constexpr bool operator==(const NamingConvention&, const NamingConvention&) = default;
};

consteval Rename rename(std::string_view n) {
    return {std::define_static_string(n)};
}

consteval Length length(size_t min, size_t max) {
    return Length{min, max};
}

template <typename T>
consteval Range<T> range(T min, T max) { return {min, max}; }

template <typename T>
consteval Default<T> default_value(T v) { return {v}; }

template <>
consteval Default<const char*> default_value(const char* v) {
    return Default<const char*>{std::define_static_string(v)};
}

consteval NamingConvention naming_convention(NamingConvention::Type t) {
    return {t};
}

struct Ignore {
    friend constexpr bool operator==(const Ignore&, const Ignore&) = default;
};

consteval Ignore ignore() { return {}; }

// ============================================================
// Naming convention string transform (compile-time)
// ============================================================

consteval std::string_view apply_naming_convention(std::string_view src, NamingConvention nc) {
    if (src.empty()) return src;

    char buf[128] = {};
    int pos = 0;
    bool first = true;
    bool boundary = false;

    auto emit_sep = [&] {
        if (first) return;
        switch (nc.type) {
            case NamingConvention::Snake:
            case NamingConvention::Upper: buf[pos++] = '_'; break;
            case NamingConvention::Kebab: buf[pos++] = '-'; break;
            default: break;
        }
    };

    auto emit_char = [&](char c, bool cap) {
        if (cap && c >= 'a' && c <= 'z') buf[pos++] = c - 'a' + 'A';
        else if (!cap && c >= 'A' && c <= 'Z') buf[pos++] = c - 'A' + 'a';
        else buf[pos++] = c;
    };

    for (size_t i = 0; i < src.size() && pos < 120; i++) {
        char c = src[i];

        if (c == '_') {
            if (!first) boundary = true;
            continue;
        }

        bool is_up = (c >= 'A' && c <= 'Z');
        bool is_lo = (c >= 'a' && c <= 'z');

        // Detect camelCase / PascalCase boundary
        if (!first && is_up && !boundary) {
            char p = src[i-1];
            if (p == '_') boundary = true;
            else if (p >= 'a' && p <= 'z') boundary = true;
            else if (p >= 'A' && p <= 'Z' && i+1 < src.size()
                     && src[i+1] >= 'a' && src[i+1] <= 'z')
                boundary = true;
        }

        // Determine target case for this character
        bool want_cap = false;
        switch (nc.type) {
            case NamingConvention::Snake:
            case NamingConvention::Kebab:
                want_cap = false;
                break;
            case NamingConvention::Camel:
                want_cap = boundary && !first;
                break;
            case NamingConvention::Pascal:
                want_cap = first || boundary;
                break;
            case NamingConvention::Upper:
                want_cap = true;
                break;
        }

        if (boundary && !first) {
            emit_sep();
            boundary = false;
        }

        emit_char(c, want_cap);
        first = false;
    }

    buf[pos] = '\0';
    return std::string_view{std::define_static_string(std::string_view(buf, pos))};
}

// ============================================================
// Field metadata
// ============================================================

struct FieldIdentifier {
    std::string_view name;
    std::optional<Rename> rename;

    constexpr std::string_view get_ini_name() const {
        return rename ? std::string_view{rename->name} : name;
    }
};

template <typename T>
struct FieldMeta {
    FieldIdentifier identifier;
    std::optional<Range<T>> range;
    std::optional<Length> length;
    std::optional<Default<T>> default_value;
    bool ignore = false;
};

// ============================================================
// Reflection helpers
// ============================================================

template <std::meta::info member>
consteval std::optional<NamingConvention> get_naming_for() {
    for (auto ann : std::meta::annotations_of(member)) {
        if (std::meta::type_of(ann) == ^^NamingConvention) {
            return std::meta::extract<NamingConvention>(ann);
        }
    }
    return std::nullopt;
}

template <std::meta::info member>
consteval std::optional<Rename> get_rename_for() {
    for (auto ann : std::meta::annotations_of(member)) {
        if (std::meta::type_of(ann) == ^^Rename) {
            auto rename = std::meta::extract<Rename>(ann);
            return {rename};
        }
    }
    return std::nullopt;
}

template <std::meta::info member>
consteval std::string_view get_section_name_for() {
    auto rename = get_rename_for<member>();
    if (rename) return std::string_view{rename->name};
    return std::meta::identifier_of(std::meta::type_of(member));
}

template <std::meta::info field>
consteval auto get_field_identifier(std::optional<NamingConvention> parent_naming = {}) {
    auto rename = get_rename_for<field>();
    auto field_naming = get_naming_for<field>();
    auto effective = rename ? std::nullopt
                  : (field_naming ? field_naming : parent_naming);
    auto name = std::meta::identifier_of(field);
    if (effective) name = apply_naming_convention(name, *effective);
    return FieldIdentifier{name, rename};
}

template <std::meta::info ann>
consteval bool is_range_annotation() {
    return std::meta::template_of(std::meta::type_of(ann)) == ^^Range;
}

template <std::meta::info ann>
consteval bool is_length_annotation() {
    return std::meta::type_of(ann) == ^^Length;
}

template <std::meta::info ann>
consteval bool is_default_annotation() {
    return std::meta::template_of(std::meta::type_of(ann)) == ^^Default;
}

template <std::meta::info ann>
consteval bool is_ignore_annotation() {
    return std::meta::type_of(ann) == ^^Ignore;
}

template <std::meta::info member>
consteval auto get_field_meta(std::optional<NamingConvention> parent_naming = {}) {
    using T = [:std::meta::type_of(member):];
    FieldMeta<T> meta{};
    meta.identifier = get_field_identifier<member>(parent_naming);
    constexpr auto annons = std::define_static_array(std::meta::annotations_of(member));
    template for (constexpr auto ann : annons) {
        if constexpr (is_length_annotation<ann>()) {
            meta.length = std::meta::extract<Length>(ann);
        }
        if constexpr (is_ignore_annotation<ann>()) {
            meta.ignore = true;
        }
        if constexpr (std::meta::has_template_arguments(std::meta::type_of(ann))) {
            if constexpr (is_range_annotation<ann>()) {
                meta.range = std::meta::extract<Range<T>>(ann);
            }
            if constexpr (is_default_annotation<ann>()) {
                if constexpr (std::is_same_v<T, std::string>) {
                    auto dv = std::meta::extract<Default<const char*>>(ann);
                    meta.default_value.emplace();
                    meta.default_value->value = dv.value;
                } else {
                    meta.default_value = std::meta::extract<Default<T>>(ann);
                }
            }
        }
    }
    return meta;
}

template <std::meta::info field>
consteval uint32_t field_hash(std::optional<NamingConvention> parent_naming = {}) {
    return fnv1a_hash(get_field_meta<field>(parent_naming).identifier.get_ini_name());
}

// ============================================================
// Value parsing
// ============================================================

template <typename T>
T parse_string(std::string_view sv);

template <>
inline int parse_string<int>(std::string_view sv) {
    int v{};
    std::from_chars(sv.data(), sv.data() + sv.size(), v);
    return v;
}

template <>
inline long parse_string<long>(std::string_view sv) {
    long v{};
    std::from_chars(sv.data(), sv.data() + sv.size(), v);
    return v;
}

template <>
inline long long parse_string<long long>(std::string_view sv) {
    long long v{};
    std::from_chars(sv.data(), sv.data() + sv.size(), v);
    return v;
}

template <>
inline unsigned parse_string<unsigned>(std::string_view sv) {
    unsigned v{};
    std::from_chars(sv.data(), sv.data() + sv.size(), v);
    return v;
}

template <>
inline float parse_string<float>(std::string_view sv) {
    return std::stof(std::string(sv));
}

template <>
inline double parse_string<double>(std::string_view sv) {
    return std::stod(std::string(sv));
}

template <>
inline std::string parse_string<std::string>(std::string_view sv) {
    return std::string(sv);
}

template <>
inline bool parse_string<bool>(std::string_view sv) {
    return sv == "true" || sv == "1" || sv == "yes" || sv == "on";
}

template <typename T>
    requires std::is_enum_v<T>
T parse_string(std::string_view sv) {
    constexpr auto enumerators = std::define_static_array(
        std::meta::enumerators_of(^^T));
    template for (constexpr auto e : enumerators) {
        auto rename = get_rename_for<e>();
        auto name = rename ? std::string_view{rename->name}
                           : std::meta::identifier_of(e);
        if (sv != name) continue;
        return [:e:];
    }
    std::cerr << "Unknown enumerator \"" << sv << "\" for "
              << std::meta::identifier_of(^^T) << std::endl;
    std::abort();
}

// ============================================================
// INI parser
// ============================================================

struct IniParser {
    template <typename T>
    static void check_value(std::string_view parent, const T &value, const FieldMeta<T> &meta) {
        if constexpr (std::is_arithmetic_v<T>) {
            if (meta.range && (value < meta.range->min || value > meta.range->max)) {
                std::cerr << parent << "::" << meta.identifier.name
                        << "'s value must be within [" << meta.range->min
                        << ", " << meta.range->max << "], "
                        << "got " << value
                        << std::endl;
                std::abort();
            }
        }
        if constexpr (std::is_same_v<T, std::string>) {
            if (meta.length && (value.size() < meta.length->min || value.size() > meta.length->max)) {
                std::cerr << parent << "::" << meta.identifier.name
                        << "'s length must be within [" << meta.length->min
                        << ", " << meta.length->max << "], "
                        << "got " << value.size()
                        << std::endl;
                std::abort();
            }
        }
    }

    template <typename T>
    static void parse(std::string_view path, T& model) {
        std::ifstream file{std::string(path)};
        if (!file.is_open()) {
            std::cerr << "Cannot open: " << path << "\n";
            std::abort();
        }

        auto trim = [](std::string_view s) -> std::string_view {
            auto b = s.find_first_not_of(" \t\r");
            if (b == std::string_view::npos) return {};
            auto e = s.find_last_not_of(" \t\r");
            return s.substr(b, e - b + 1);
        };

        constexpr auto sections = std::define_static_array(
            std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));

        template for (constexpr auto sm : sections) {
            constexpr auto pn = get_naming_for<sm>();
            auto& so = model.[:sm:];
            using ST = [:std::meta::type_of(sm):];
            constexpr auto sf = std::define_static_array(
                std::meta::nonstatic_data_members_of(^^ST, std::meta::access_context::current()));
            template for (constexpr auto f : sf) {
                using FT = [:std::meta::type_of(f):];
                auto m = get_field_meta<f>(pn);
                if (m.ignore) continue;
                if (m.default_value) so.[:f:] = m.default_value->value;
            }
        }

        std::string line;
        int cur = -1;

        while (std::getline(file, line)) {
            std::string_view sv = line;
            sv = trim(sv);
            if (sv.empty() || sv[0] == ';' || sv[0] == '#') continue;

            if (sv[0] == '[') {
                auto c = sv.find(']');
                if (c == std::string_view::npos) continue;
                auto name = sv.substr(1, c - 1);
                cur = -1;
                { int i = 0;
                  template for (constexpr auto sm : sections) {
                      constexpr auto sn = get_section_name_for<sm>();
                      if (name == sn) cur = i;
                      ++i;
                  } }
                continue;
            }

            if (cur < 0) continue;

            auto eq = sv.find('=');
            if (eq == std::string_view::npos) continue;
            auto key = trim(sv.substr(0, eq));
            auto val = trim(sv.substr(eq + 1));
            auto kh = fnv1a_hash(key);

            { int i = 0;
              template for (constexpr auto sm : sections) {
                  if (i != cur) { ++i; continue; }
                  constexpr auto sn = get_section_name_for<sm>();
                  constexpr auto pn = get_naming_for<sm>();
                  auto& so = model.[:sm:];
                  using ST = [:std::meta::type_of(sm):];
                  constexpr auto sf = std::define_static_array(
                      std::meta::nonstatic_data_members_of(^^ST, std::meta::access_context::current()));
                  template for (constexpr auto f : sf) {
                      auto m = get_field_meta<f>(pn);
                      if (m.ignore) continue;
                      constexpr auto fh = field_hash<f>(pn);
                      if (kh != fh) continue;
                      if (m.identifier.get_ini_name() != key) continue;
                      using FT = [:std::meta::type_of(f):];
                      so.[:f:] = parse_string<FT>(val);
                      check_value(sn, so.[:f:], m);
                  }
                  ++i;
              } }
        }
    }
};

template <typename T>
void print_model(const T& value) {
    template for (constexpr auto member : std::define_static_array(
        std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current())))
    {
        std::cout << std::meta::identifier_of(member) << "="
                  << value.[:member:] << " ";
    }
    std::cout << std::endl;
}
