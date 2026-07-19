#include "ini_parser.h"

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
// Composite model — all sections in one struct
// ============================================================

struct AllModels {
    Person person;
    Config config;
    FailExample fail;
};

// ============================================================
// Demo
// ============================================================

int main() {
    AllModels all;
    IniParser::parse("combined.ini", all);
    print_model(all.person);
    print_model(all.config);
    print_model(all.fail);

    return 0;
}
