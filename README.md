# INI Parser — C++26 Reflection (P2996)

A compile-time reflection-driven INI parser using C++26 `std::meta` (P2996) with `clang-p2996`.

## Environment

This project requires Bloomberg's [clang-p2996](https://github.com/bloomberg/clang-p2996) fork.  
Use the provided Docker image:

```bash
docker run --rm -it -v .:/work ghcr.io/bloomberg/clang-p2996:latest
```

Inside the container:

```bash
cd /work
bash build.sh
./ini_parser
```

## AN INI Parser Usage

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

Person person;
Config config;

IniParser::parse("person.ini", person);
IniParser::parse("config.ini", config);
```

## INI Example

**person.ini:**
```ini
[Person]
full_name = Alice
; age = 25   (default_value(32) will be used)
```

**config.ini:**
```ini
[Config]
host = localhost
port = 8080
debug = true
timeout = 30.0
```

**fail.ini** (triggers validation error):
```ini
[FailExample]
string_val="TOOOOOOOOOOOOO_LOOOOOOOOOOONG"
```

## Annotations

| Annotation | Purpose | Example |
|---|---|---|
| `rename("key")` | Map field to different INI key | `[[=rename("full_name")]]` |
| `range(min, max)` | Arithmetic range constraint | `[[=range(0, 65535)]]` |
| `default_value(v)` | Default when key missing | `[[=default_value(32)]]` |
| `length(min, max)` | String length constraint | `[[=length(0, 4)]]` |

## Build

```bash
bash build.sh
```

## Reflection

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
