# EasyIni — C++26 Reflection INI Parser

A compile-time reflection-driven INI parser using C++26 `std::meta` (P2996) with `clang-p2996`.

## Supported Features

| Feature | Annotation / Mechanism | Example |
|---|---|---|
| **Multi-section** | Each root struct member = one `[Section]` in INI | `struct Root { Person p; Config c; }` |
| **Section rename** | `rename("...")` on root member | `[[=rename("Server")]] Config cfg;` |
| **Field rename** | `rename("...")` on field | `[[=rename("full_name")]] string name;` |
| **Default value** | `default_value(v)` | `[[=default_value(32)]] int age;` |
| **Range constraint** | `range(min, max)` | `[[=range(0, 65535)]] int port;` |
| **Length constraint** | `length(min, max)` | `[[=length(0, 4)]] string code;` |
| **Naming convention** | `naming_convention(Type)` on root member, inherited by fields | `[[=naming_convention(Snake)]] Section sec;` |
| **Field naming override** | `naming_convention(Type)` on field, overrides parent | `[[=naming_convention(Pascal)]] int field;` |
| **Ignore field** | `ignore()` | `[[=ignore()]] string cache;` |
| **Optional field** | `std::optional<T>` (no annotation needed) | `std::optional<int> port;` |
| **Deny unknown keys** | `deny_unknown()` on root member | `[[=deny_unknown()]] Section sec;` |
| **Deny unknown sections** | `deny_unknown()` on root member | triggers abort on unrecognized `[Section]` |
| **Time format** | `time_format("...")` + `std::chrono::sys_days` | `[[=time_format("%Y-%m-%d")]] sys_days date;` |
| **Enum class** | Automatic string-to-enum matching | `enum class Color { Red, Green };` |
| **Compile-time hash dispatch** | FNV-1a — O(1) field lookup | automatic |

## Environment

Requires Bloomberg's [clang-p2996](https://github.com/bloomberg/clang-p2996) fork.

```bash
docker run --rm -it -v .:/work ghcr.io/bloomberg/clang-p2996:latest
cd /work
bash build.sh
./ini_parser
```

## Usage

```cpp
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

struct AllModels {
    Person person;
    Config config;
};

AllModels all;
IniParser::parse("combined.ini", all);
```

## Annotations

| Annotation | Applies to | Purpose |
|---|---|---|
| `rename("x")` | field / root member | Override INI key/section name |
| `range(min, max)` | field | Arithmetic range validation |
| `default_value(v)` | field | Default when key missing |
| `length(min, max)` | field | String length validation |
| `naming_convention(t)` | root member / field | Auto-transform field names |
| `ignore()` | field | Skip field entirely |
| `time_format("fmt")` | field | Parse `sys_days` with format |
| `deny_unknown()` | root member | Abort on unknown key or section |

## Naming Convention Types

| Type | `myFieldName` → |
|---|---|
| `Snake` | `my_field_name` |
| `Camel` | `myFieldName` |
| `Pascal` | `MyFieldName` |
| `Upper` | `MY_FIELD_NAME` |
| `Kebab` | `my-field-name` |

## Build & Test

```bash
bash build.sh          # build main + run unit tests
bash build_bench.sh    # benchmark comparison vs SimpleIni
```

## Benchmark

Comparison against [SimpleIni](https://github.com/brofield/simpleini). 1000-line INI file,
100 iterations per case.

| Metric | Reflection | SimpleIni |
|---|---|---|
| Total (parse+bind) | **6,918 µs** | 7,753 µs |
| Bind only | automatic (0 lines) | 21 µs (20× `GetValue`) |
| Lines of binding code | 0 | 20 |

![Benchmark comparison](figures/bench_comparison.png)

12% faster overall, zero manual binding code, O(1) field lookup via compile-time FNV-1a hashing.

## How It Works

```cpp
template <typename T>
void print_model(const T& value) {
    template for (constexpr auto member : std::define_static_array(
        std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current())))
    {
        std::cout << std::meta::identifier_of(member) << "="
                  << value.[:member:] << " ";
    }
}
```
