/*
 * test_inifile.cpp
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *    License: MIT
 */

#include "inicpp.h"
#include <catch.hpp>

#include <sstream>

TEST_CASE("parse ini file", "IniFile")
{
    std::istringstream ss(("[Foo]\nbar=hello world\n[Test]"));
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 2);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "hello world");
    REQUIRE(inif["Test"].size() == 0);
}

TEST_CASE("parse empty file", "IniFile")
{
    std::istringstream ss("");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 0);
}

TEST_CASE("parse comment only file", "IniFile")
{
    std::istringstream ss("# this is a comment");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 0);
}


TEST_CASE("parse empty section", "IniFile")
{
    std::istringstream ss("[Foo]");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 0);
}

TEST_CASE("parse empty field", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "");
}

TEST_CASE("parse section with duplicate field", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=hello\nbar=world");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "world");
}

TEST_CASE("parse field as double", "IniFile")
{
    std::istringstream ss("[Foo]\nbar1=1.2\nbar2=1\nbar3=-2.4");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 3);
    REQUIRE(inif["Foo"]["bar1"].as<double>() == 1.2);
    REQUIRE(inif["Foo"]["bar2"].as<double>() == 1.0);
    REQUIRE(inif["Foo"]["bar3"].as<double>() == -2.4);
}

TEST_CASE("parse field as int", "IniFile")
{
    std::istringstream ss("[Foo]\nbar1=1\nbar2=-2");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 2);
    REQUIRE(inif["Foo"]["bar1"].as<int>() == 1);
    REQUIRE(inif["Foo"]["bar2"].as<int>() == -2);
}

TEST_CASE("parse field as bool", "IniFile")
{
    std::istringstream ss("[Foo]\nbar1=true\nbar2=false\nbar3=tRuE");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 3);
    REQUIRE(inif["Foo"]["bar1"].as<bool>());
    REQUIRE_FALSE(inif["Foo"]["bar2"].as<bool>());
    REQUIRE(inif["Foo"]["bar3"].as<bool>());
}

TEST_CASE("parse field with custom field sep", "IniFile")
{
    std::istringstream ss("[Foo]\nbar1:true\nbar2:false\nbar3:tRuE");
    ini::IniFile inif(ss, ':');

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 3);
    REQUIRE(inif["Foo"]["bar1"].as<bool>());
    REQUIRE_FALSE(inif["Foo"]["bar2"].as<bool>());
    REQUIRE(inif["Foo"]["bar3"].as<bool>());
}

TEST_CASE("parse with comment", "IniFile")
{
    std::istringstream ss("[Foo]\n# this is a test\nbar=bla");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "bla");
}

TEST_CASE("parse with custom comment char prefix", "IniFile")
{
    std::istringstream ss("[Foo]\n$ this is a test\nbar=bla");
    ini::IniFile inif(ss, '=', '$');

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "bla");
}

TEST_CASE("parse with multi char comment prefix", "IniFile")
{
    std::istringstream ss("[Foo]\nREM this is a test\nbar=bla");
    ini::IniFile inif(ss, '=', {"REM"});

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "bla");
}

TEST_CASE("parse with multiple multi char comment prefixes", "IniFile")
{
    std::istringstream ss("[Foo]\n"
                          "REM this is a comment\n"
                          "#Also a comment\n"
                          "//Even this is a comment\n"
                          "bar=bla");
    ini::IniFile inif(ss, '=', {"REM", "#", "//"});

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "bla");
}

TEST_CASE("comment prefixes can be set after construction", "IniFile")
{
    std::istringstream ss("[Foo]\n"
                          "REM this is a comment\n"
                          "#Also a comment\n"
                          "//Even this is a comment\n"
                          "bar=bla");
    ini::IniFile inif;
    inif.setCommentPrefixes({"REM", "#", "//"});
    inif.decode(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "bla");
}

TEST_CASE("comments are allowed after escaped comments", "IniFile")
{
    std::istringstream ss("[Foo]\n"
                          "hello=world \\## this is a comment\n"
                          "more=of this \\# \\#\n");
    ini::IniFile inif(ss);

    REQUIRE(inif["Foo"]["hello"].as<std::string>() == "world #");
    REQUIRE(inif["Foo"]["more"].as<std::string>() == "of this # #");
}

TEST_CASE(
    "escape char right before a comment prefix escapes all the comment prefix",
    "IniFile")
{
    std::istringstream ss("[Foo]\n"
                          "weird1=note \\### this is not a comment\n"
                          "weird2=but \\#### this is a comment");
    ini::IniFile inif(ss, '=', {"##"});

    REQUIRE(inif["Foo"]["weird1"].as<std::string>() ==
            "note ### this is not a comment");
    REQUIRE(inif["Foo"]["weird2"].as<std::string>() == "but ##");
}

TEST_CASE("save with bool fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] = true;
    inif["Foo"]["bar2"] = false;

    std::string result = inif.encode();
    REQUIRE(result == "[Foo]\nbar1=true\nbar2=false\n");
}

TEST_CASE("save with int fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] = 1;
    inif["Foo"]["bar2"] = -2;

    std::string result = inif.encode();
    REQUIRE(result == "[Foo]\nbar1=1\nbar2=-2\n");
}

TEST_CASE("save with double fields", "IniFile")
{
    ini::IniFile inif;
    inif["Foo"]["bar1"] = 1.2;
    inif["Foo"]["bar2"] = -2.4;

    std::string result = inif.encode();
    REQUIRE(result == "[Foo]\nbar1=1.2\nbar2=-2.4\n");
}

TEST_CASE("save with custom field sep", "IniFile")
{
    ini::IniFile inif(':', '#');
    inif["Foo"]["bar1"] = true;
    inif["Foo"]["bar2"] = false;

    std::string result = inif.encode();
    REQUIRE(result == "[Foo]\nbar1:true\nbar2:false\n");
}

TEST_CASE("inline comments in sections are discarded", "IniFile")
{
    std::istringstream ss("[Foo] # This is an inline comment\nbar=Hello world!");
    ini::IniFile inif(ss);

    REQUIRE(inif.find("Foo") != inif.end());
}

TEST_CASE("inline comments in fields are discarded", "IniFile")
{
    std::istringstream ss("[Foo]\n"
                          "bar=Hello #world!");
    ini::IniFile inif(ss);

    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "Hello");
}

TEST_CASE("inline comments can be escaped", "IniFile")
{
    std::istringstream ss("[Foo]\n"
                          "bar=Hello \\#world!");
    ini::IniFile inif(ss);

    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "Hello #world!");
}

TEST_CASE("escape characters are kept if not before a comment prefix", "IniFile")
{
    std::istringstream ss("[Foo]\n"
                          "bar=Hello \\world!");
    ini::IniFile inif(ss);

    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "Hello \\world!");
}

/***************************************************
 *                Failing Tests
 ***************************************************/

TEST_CASE("fail to load unclosed section", "IniFile")
{
    ini::IniFile inif;
    REQUIRE_THROWS(inif.decode("[Foo\nbar=bla"));
}

TEST_CASE("fail to load field without equal", "IniFile")
{
    ini::IniFile inif;
    REQUIRE_THROWS(inif.decode("[Foo]\nbar"));
}

TEST_CASE("fail to parse as bool", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=bla");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE_THROWS(inif["Foo"]["bar"].as<bool>());
}

TEST_CASE("fail to parse as int", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=bla");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE_THROWS(inif["Foo"]["bar"].as<int>());
}

TEST_CASE("fail to parse as double", "IniFile")
{
    std::istringstream ss("[Foo]\nbar=bla");
    ini::IniFile inif(ss);

    REQUIRE(inif.size() == 1);
    REQUIRE(inif["Foo"].size() == 1);
    REQUIRE_THROWS(inif["Foo"]["bar"].as<double>());
}

TEST_CASE("fail to parse field without section", "IniFile")
{
    ini::IniFile inif;
    REQUIRE_THROWS(inif.decode("bar=bla"));
}

TEST_CASE("spaces are not taken into account in field names", "IniFile")
{
    std::istringstream ss(("[Foo]\n  \t  bar  \t  =hello world"));
    ini::IniFile inif(ss);

    REQUIRE(inif["Foo"].find("bar") != inif["Foo"].end());
    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "hello world");
}

TEST_CASE("spaces are not taken into account in field values", "IniFile")
{
    std::istringstream ss(("[Foo]\nbar=  \t  hello world  \t  "));
    ini::IniFile inif(ss);

    REQUIRE(inif["Foo"]["bar"].as<std::string>() == "hello world");
}

TEST_CASE("spaces are not taken into account in sections", "IniFile")
{
    std::istringstream ss("  \t  [Foo]  \t  \nbar=bla");
    ini::IniFile inif(ss);

    REQUIRE(inif.find("Foo") != inif.end());
}
