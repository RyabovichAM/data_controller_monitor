#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "json_framer.h"

namespace {

class Framed {
public:
    Framed()
        : framer_{[this](const std::string& json) { objects_.push_back(json); }} {
    }

    void Feed(const std::string& bytes) {
        framer_.Feed(bytes.data(), bytes.size());
    }

    const std::vector<std::string>& objects() const {
        return objects_;
    }

private:
    std::vector<std::string> objects_;
    transfer::JsonFramer framer_{nullptr};
};

}   //namespace

TEST(JsonFramer, OneObjectInOneFeed) {
    Framed framed;
    framed.Feed(R"({"a":1})");

    ASSERT_EQ(framed.objects().size(), 1u);
    EXPECT_EQ(framed.objects()[0], R"({"a":1})");
}

TEST(JsonFramer, SeveralObjectsInOneFeed) {
    Framed framed;
    framed.Feed(R"({"a":1}{"b":2})");

    ASSERT_EQ(framed.objects().size(), 2u);
    EXPECT_EQ(framed.objects()[1], R"({"b":2})");
}

TEST(JsonFramer, ObjectSplitAcrossFeeds) {
    // What a socket actually does: a message arrives in pieces.
    Framed framed;
    framed.Feed(R"({"tempera)");
    EXPECT_TRUE(framed.objects().empty());

    framed.Feed(R"(ture":21.5})");

    ASSERT_EQ(framed.objects().size(), 1u);
    EXPECT_EQ(framed.objects()[0], R"({"temperature":21.5})");
}

// The Qt version took the first closing brace it saw, so a nested object was
// never framed whole.
TEST(JsonFramer, NestedObjectIsWhole) {
    Framed framed;
    framed.Feed(R"({"outer":{"inner":1},"after":2})");

    ASSERT_EQ(framed.objects().size(), 1u);
    EXPECT_EQ(framed.objects()[0], R"({"outer":{"inner":1},"after":2})");
}

TEST(JsonFramer, BracesInsideStringsDoNotCount) {
    Framed framed;
    framed.Feed(R"({"text":"} { not a brace"})");

    ASSERT_EQ(framed.objects().size(), 1u);
    EXPECT_EQ(framed.objects()[0], R"({"text":"} { not a brace"})");
}

TEST(JsonFramer, EscapedQuoteKeepsTheStringOpen) {
    Framed framed;
    framed.Feed(R"({"text":"a \" }"})");

    ASSERT_EQ(framed.objects().size(), 1u);
    EXPECT_EQ(framed.objects()[0], R"({"text":"a \" }"})");
}

TEST(JsonFramer, RubbishBetweenObjectsIsDropped) {
    // Line breaks, spaces and whatever else a controller puts between messages.
    Framed framed;
    framed.Feed("\r\n  {\"a\":1}\r\n garbage \n{\"b\":2}\n");

    ASSERT_EQ(framed.objects().size(), 2u);
    EXPECT_EQ(framed.objects()[0], R"({"a":1})");
    EXPECT_EQ(framed.objects()[1], R"({"b":2})");
}

TEST(JsonFramer, IncompleteObjectYieldsNothing) {
    Framed framed;
    framed.Feed(R"({"a":1)");

    EXPECT_TRUE(framed.objects().empty());
}
