#include "../ini_parser.h"
#include <gtest/gtest.h>

#include <string>

// ============================================================
// Leaf models (each maps to an INI section)
// ============================================================

struct TestSimple {
    std::string name;
    int age;
};

struct TestDefaults {
    [[=default_value("default_name")]]
    std::string name;
    [[=default_value(42)]]
    int value;
};

struct TestFieldRename {
    [[=rename("full_name")]]
    std::string name;
};

struct TestSectionRename {
    std::string value;
};

struct TestMultiA {
    std::string name;
    int value;
};

struct TestMultiB {
    std::string label;
};

struct TestRangeValid {
    [[=range(0, 100)]]
    int value;
};

struct TestLengthValid {
    [[=length(1, 10)]]
    std::string name;
};

struct TestComments {
    std::string name;
    int age;
};

struct TestWhitespace {
    std::string name;
    int value;
};

struct TestUnknownKey {
    std::string name;
};

struct TestUnknownSection {
    std::string name;
};

// ============================================================
// Enum models
// ============================================================

enum class TestEnumColor {
    Red,
    Green,
    Blue
};

struct TestEnumModel {
    TestEnumColor color;
};

enum class TestEnumRenamed {
    Red,
    Green,
    Blue
};

struct TestEnumRenameModel {
    TestEnumRenamed color;
};

// ============================================================
// Naming convention models
// ============================================================

struct TestNamingSnake {
    int myFieldValue;
};

struct TestNamingInherit {
    int myFieldValue;
};

struct TestNamingOverride {
    [[=naming_convention(NamingConvention::Pascal)]]
    int myFieldValue;
};

struct TestNamingPrecedence {
    [[=rename("custom_key")]]
    int myFieldValue;
};

// ============================================================
// Ignore models
// ============================================================

struct TestIgnoreField {
    std::string name;
    [[=ignore()]]
    std::string skip;
    int age;
};

struct TestIgnoreDefault {
    [[=default_value("should_not_apply")]]
    [[=ignore()]]
    std::string skip;
    std::string name;
};

// ============================================================
// Root containers (each member = one section in INI)
// ============================================================

struct TestSimpleRoot          { TestSimple val; };
struct TestDefaultsRoot        { TestDefaults val; };
struct TestFieldRenameRoot     { TestFieldRename val; };
struct TestSectionRenameRoot   { [[=rename("RenamedSection")]] TestSectionRename section; };
struct TestMultiRoot           { TestMultiA a; TestMultiB b; };
struct TestCommentsRoot        { TestComments val; };
struct TestWhitespaceRoot      { TestWhitespace val; };
struct TestUnknownKeyRoot      { TestUnknownKey val; };
struct TestUnknownSectionRoot  { TestUnknownSection section; };
struct TestRangeValidRoot      { TestRangeValid val; };
struct TestLengthValidRoot     { TestLengthValid val; };
struct TestEnumModelRoot       { TestEnumModel val; };
struct TestEnumRenameModelRoot { TestEnumRenameModel val; };
struct TestNamingSnakeRoot     { TestNamingSnake val; };
struct TestNamingInheritRoot   { [[=naming_convention(NamingConvention::Snake)]] TestNamingInherit val; };
struct TestNamingOverrideRoot  { TestNamingOverride val; };
struct TestNamingPrecedenceRoot{ TestNamingPrecedence val; };
struct TestIgnoreFieldRoot    { TestIgnoreField val; };
struct TestIgnoreDefaultRoot  { TestIgnoreDefault val; };

// ============================================================
// Tests
// ============================================================

TEST(IniParser, simple_parse) {
    TestSimpleRoot m;
    IniParser::parse("test/simple.ini", m);
    EXPECT_EQ(m.val.name, "Alice");
    EXPECT_EQ(m.val.age, 25);
}

TEST(IniParser, default_values) {
    TestDefaultsRoot m;
    IniParser::parse("test/defaults.ini", m);
    EXPECT_EQ(m.val.name, "default_name");
    EXPECT_EQ(m.val.value, 42);
}

TEST(IniParser, field_rename) {
    TestFieldRenameRoot m;
    IniParser::parse("test/field_rename.ini", m);
    EXPECT_EQ(m.val.name, "Bob");
}

TEST(IniParser, section_rename) {
    TestSectionRenameRoot m;
    IniParser::parse("test/section_rename.ini", m);
    EXPECT_EQ(m.section.value, "renamed_value");
}

TEST(IniParser, multi_section) {
    TestMultiRoot m;
    IniParser::parse("test/multi_section.ini", m);
    EXPECT_EQ(m.a.name, "First");
    EXPECT_EQ(m.a.value, 10);
    EXPECT_EQ(m.b.label, "Second");
}

TEST(IniParser, comments) {
    TestCommentsRoot m;
    IniParser::parse("test/comments.ini", m);
    EXPECT_EQ(m.val.name, "Alice");
    EXPECT_EQ(m.val.age, 30);
}

TEST(IniParser, whitespace) {
    TestWhitespaceRoot m;
    IniParser::parse("test/whitespace.ini", m);
    EXPECT_EQ(m.val.name, "hello");
    EXPECT_EQ(m.val.value, 42);
}

TEST(IniParser, unknown_key) {
    TestUnknownKeyRoot m;
    IniParser::parse("test/unknown_key.ini", m);
    EXPECT_EQ(m.val.name, "found");
}

TEST(IniParser, unknown_section) {
    TestUnknownSectionRoot m;
    IniParser::parse("test/unknown_section.ini", m);
    EXPECT_EQ(m.section.name, "");
}

TEST(IniParser, range_valid) {
    TestRangeValidRoot m;
    IniParser::parse("test/range_valid.ini", m);
    EXPECT_EQ(m.val.value, 50);
}

TEST(IniParser, length_valid) {
    TestLengthValidRoot m;
    IniParser::parse("test/length_valid.ini", m);
    EXPECT_EQ(m.val.name, "hello");
}

TEST(IniParser, enum_parse) {
    TestEnumModelRoot m;
    IniParser::parse("test/enum_parse.ini", m);
    EXPECT_EQ(m.val.color, TestEnumColor::Green);
}

TEST(IniParser, enum_rename) {
    TestEnumRenameModelRoot m;
    IniParser::parse("test/enum_rename.ini", m);
    EXPECT_EQ(m.val.color, TestEnumRenamed::Red);
}

TEST(IniParser, naming_snake) {
    TestNamingSnakeRoot m;
    IniParser::parse("test/naming_snake.ini", m);
    EXPECT_EQ(m.val.myFieldValue, 42);
}

TEST(IniParser, naming_inherit) {
    TestNamingInheritRoot m;
    IniParser::parse("test/naming_inherit.ini", m);
    EXPECT_EQ(m.val.myFieldValue, 99);
}

TEST(IniParser, naming_override) {
    TestNamingOverrideRoot m;
    IniParser::parse("test/naming_override.ini", m);
    EXPECT_EQ(m.val.myFieldValue, 77);
}

TEST(IniParser, naming_precedence) {
    TestNamingPrecedenceRoot m;
    IniParser::parse("test/naming_precedence.ini", m);
    EXPECT_EQ(m.val.myFieldValue, 55);
}

TEST(IniParser, ignore_field) {
    TestIgnoreFieldRoot m;
    IniParser::parse("test/ignore_field.ini", m);
    EXPECT_EQ(m.val.name, "hello");
    // 'skip' has [[=ignore]] — value from INI must NOT be loaded
    EXPECT_EQ(m.val.skip, "");
    EXPECT_EQ(m.val.age, 99);
}

TEST(IniParser, ignore_default) {
    TestIgnoreDefaultRoot m;
    IniParser::parse("test/ignore_default.ini", m);
    // 'skip' has [[=ignore]] AND [[=default_value(...)]] — default must NOT apply
    EXPECT_EQ(m.val.skip, "");
    EXPECT_EQ(m.val.name, "found");
}

// ============================================================
// Main
// ============================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
