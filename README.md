# EasyIni — Reflection-based INI Parser for C++26

A lightweight INI parser powered by **C++26 compile-time reflection (`std::meta`)**.

Using Bloomberg's **clang-p2996**, EasyIni automatically maps INI files to user-defined structs **without registration, macros, or manual binding code**.

```cpp
struct Config {
    std::string host;

    [[=range(0,65535)]]
    int port;

    [[=default_value(false)]]
    bool debug;
};

Config cfg;
IniParser::parse("config.ini", cfg);
```

---

## Highlights

- Zero registration or binding code
- Compile-time reflection (`std::meta`)
- Automatic section-to-struct mapping
- Annotation-based customization
- Built-in validation (range, length, defaults)
- Optional values and enum support
- O(1) field dispatch using compile-time FNV-1a hashing

---

## Supported Features

| Feature | Annotation | Example |
|----------|------------|---------|
| Multi-section mapping | root struct | `Root { Server server; Client client; }` |
| Rename section | `rename("Server")` | `[[=rename("Server")]] Config cfg;` |
| Rename field | `rename("full_name")` | `[[=rename("full_name")]] std::string name;` |
| Default value | `default_value(v)` | `[[=default_value(32)]] int age;` |
| Range validation | `range(min, max)` | `[[=range(0,65535)]] int port;` |
| String length validation | `length(min, max)` | `[[=length(0,8)]] std::string code;` |
| Naming convention | `naming_convention(Snake)` | automatic key conversion |
| Ignore field | `ignore()` | skipped during parsing |
| Optional field | `std::optional<T>` | key may be omitted |
| Reject unknown entries | `deny_unknown()` | unknown keys or sections become errors |
| Time parsing | `time_format("%Y-%m-%d")` | `std::chrono::sys_days` |
| Enum parsing | automatic | `Color::Red ⇄ "Red"` |

---

## Example

```cpp
struct Person {
    [[=rename("full_name")]]
    std::string name;

    [[=default_value(32)]]
    int age;

    [[=default_value("Chinese")]]
    std::string nationality;
};

struct Server {
    std::string host;

    [[=range(0,65535)]]
    int port;

    [[=default_value(false)]]
    bool debug;
};

struct Config {
    Person person;
    Server server;
};

Config cfg;
IniParser::parse("config.ini", cfg);
```

---

## Benchmark

Benchmark against **SimpleIni** using a **1000-line INI file**, averaged over **100 iterations**.

| Metric | EasyIni | SimpleIni |
|---------|---------|-----------|
| Parse + bind | **6,918 µs** | 7,753 µs |
| Binding code | **0 lines** | ~20 `GetValue()` calls |
| Field lookup | Compile-time FNV-1a hash | Runtime string lookup |

### Results

- **~12% faster** than SimpleIni
- **Zero manual binding code**
- **O(1) field lookup** via compile-time hashing

---

## Requirements

EasyIni currently requires Bloomberg's experimental **clang-p2996** compiler with C++26 reflection support.

```bash
docker run --rm -it -v .:/work ghcr.io/bloomberg/clang-p2996:latest

cd /work
bash build.sh
./ini_parser
```
