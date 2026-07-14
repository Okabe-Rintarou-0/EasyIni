#include <meta>
#include <string>
#include <string_view>
#include <fstream>
#include <optional>
#include <iostream>
#include <cstdlib>
#include <type_traits>
#include <format>
#include <algorithm>
#include <charconv>

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

// ============================================================
// Field metadata (for external consteval use only)
// ============================================================

struct FieldIdentifier {
    std::string_view name;
    std::optional<Rename> rename;

    std::string_view get_ini_name() const {
        return rename ? std::string_view{rename->name} : name;
    }
};

template <typename T>
struct FieldMeta {
    FieldIdentifier identifier;
    std::string_view parent;
    std::optional<Range<T>> range;
    std::optional<Length> length;
    std::optional<Default<T>> default_value;
    std::optional<Rename> rename;
};


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

template <std::meta::info field>
consteval auto get_field_identifier() {
    auto rename = get_rename_for<field>();
    auto name = std::meta::identifier_of(field);
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

template <std::meta::info member>
consteval auto get_field_meta() {
    using T = [:std::meta::type_of(member):];
    FieldMeta<T> meta{};
    meta.identifier = get_field_identifier<member>();
    constexpr auto annons = std::define_static_array(std::meta::annotations_of(member));
    template for (constexpr auto ann : annons) {
        template for (constexpr auto ann : annons) {
        if constexpr (is_length_annotation<ann>()) {
            meta.length = std::meta::extract<Length>(ann);
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
    }
    return meta;
}

template <typename T>
consteval auto get_field_identifiers() {
    constexpr auto fields = 
            std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current());
    std::vector<FieldIdentifier> field_identifiers;
    template for (constexpr auto field: fields) {
        auto identifier = get_field_identifier<field>();
        field_identifiers.emplace_back(std::move(identifier));
    }
    return field_identifiers;
    // return meta;
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
        constexpr auto section = std::meta::identifier_of(^^T);
        constexpr auto members = std::define_static_array(
            std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));

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
        template for (constexpr auto member : members) {
            template for (constexpr auto member : members) {
                using FieldType = [:std::meta::type_of(member):];
                auto& field = model.[:member:];
                auto meta = get_field_meta<member>();
                if (meta.default_value) {
                    field = meta.default_value->value;
                }
            }
        }

        std::string line;
        bool in_section = false;
        while (std::getline(file, line)) {
            std::string_view sv = line;
            sv = trim(sv);
            if (sv.empty() || sv[0] == ';' || sv[0] == '#') continue;

            if (sv[0] == '[') {
                auto c = sv.find(']');
                if (c == std::string_view::npos) continue;
                in_section = (sv.substr(1, c - 1) == section);
                continue;
            }

            if (!in_section) continue;

            auto eq = sv.find('=');
            if (eq == std::string_view::npos) continue;
            auto key = trim(sv.substr(0, eq));
            auto val = trim(sv.substr(eq + 1));

            template for (constexpr auto member : members) {
                auto meta = get_field_meta<member>();
                if (meta.identifier.get_ini_name() != key) continue;
                using FieldType = [:std::meta::type_of(member):];
                auto& field = model.[:member:];
                field = parse_string<FieldType>(val);

                check_value(section, field, meta);
            }
        }
    }
};

// ============================================================
// Domain models
// ============================================================

struct Person {
    [[=rename("full_name")]]
    std::string name;
    [[=default_value(32)]]
    int age;
    [[=default_value("Chinese")]]
    std::string nationality;
};

struct Config {
    std::string host;

    [[=range(0, 65535)]]
    int port;

    bool debug;

    [[=default_value(5.0f)]]
    float timeout;
};

struct FailExample {
    [[=length(0, 4)]]
    [[=rename("string_val")]]
    std::string StringTooLong;
};

// ============================================================
// Demo
// ============================================================

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

int main() {
    Person person;
    IniParser::parse("person.ini", person);
    print_model(person);

    Config config;
    IniParser::parse("config.ini", config);
    print_model(config);

    FailExample fail;
    IniParser::parse("fail.ini", fail);
    print_model(fail);

    return 0;
}
