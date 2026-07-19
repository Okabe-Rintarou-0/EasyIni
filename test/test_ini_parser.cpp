#include "../ini_parser.h"
#include <gtest/gtest.h>

#include <string>

// ============================================================
// Test models
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

struct TestSectionRenameRoot {
    [[=rename("RenamedSection")]]
    TestSectionRename section;
};

struct TestMultiA {
    std::string name;
    int value;
};

struct TestMultiB {
    std::string label;
};

struct TestMultiRoot {
    TestMultiA a;
    TestMultiB b;
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

struct TestUnknownSectionRoot {
    TestUnknownSection section;
};

// ============================================================
// Enum test models
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
// Tests
// ============================================================

TEST(IniParser, simple_parse) {
    TestSimple m;
    IniParser::parse("test/simple.ini", m);
    EXPECT_EQ(m.name, "Alice");
    EXPECT_EQ(m.age, 25);
}

TEST(IniParser, default_values) {
    TestDefaults m;
    IniParser::parse("test/defaults.ini", m);
    EXPECT_EQ(m.name, "default_name");
    EXPECT_EQ(m.value, 42);
}

TEST(IniParser, field_rename) {
    TestFieldRename m;
    IniParser::parse("test/field_rename.ini", m);
    EXPECT_EQ(m.name, "Bob");
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
    TestComments m;
    IniParser::parse("test/comments.ini", m);
    EXPECT_EQ(m.name, "Alice");
    EXPECT_EQ(m.age, 30);
}

TEST(IniParser, whitespace) {
    TestWhitespace m;
    IniParser::parse("test/whitespace.ini", m);
    EXPECT_EQ(m.name, "hello");
    EXPECT_EQ(m.value, 42);
}

TEST(IniParser, unknown_key) {
    TestUnknownKey m;
    IniParser::parse("test/unknown_key.ini", m);
    EXPECT_EQ(m.name, "found");
}

TEST(IniParser, unknown_section) {
    TestUnknownSectionRoot m;
    IniParser::parse("test/unknown_section.ini", m);
    EXPECT_EQ(m.section.name, "");
}

TEST(IniParser, range_valid) {
    TestRangeValid m;
    IniParser::parse("test/range_valid.ini", m);
    EXPECT_EQ(m.value, 50);
}

TEST(IniParser, length_valid) {
    TestLengthValid m;
    IniParser::parse("test/length_valid.ini", m);
    EXPECT_EQ(m.name, "hello");
}

TEST(IniParser, enum_parse) {
    TestEnumModel m;
    IniParser::parse("test/enum_parse.ini", m);
    EXPECT_EQ(m.color, TestEnumColor::Green);
}

TEST(IniParser, enum_rename) {
    TestEnumRenameModel m;
    IniParser::parse("test/enum_rename.ini", m);
    EXPECT_EQ(m.color, TestEnumRenamed::Red);
}

// ============================================================
// Main
// ============================================================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
