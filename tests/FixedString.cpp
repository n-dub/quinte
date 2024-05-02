#include <Core/FixedString.hpp>
#include <gtest/gtest.h>

using namespace quinte;

using FixStr = FixStr256;

TEST(FixedStr, EmptyCapacity)
{
    FixedString<123> str;
    ASSERT_EQ(str.Capacity(), 123);
    ASSERT_EQ(str.Size(), 0);
}

TEST(FixedStr, Capacity)
{
    FixedString<1> str = "q";
    ASSERT_EQ(str.Capacity(), 1);
    ASSERT_EQ(str.Size(), 1);
}

TEST(FixedStr, ByteAt)
{
    FixStr str = "0123456789";
    EXPECT_EQ(str.ByteAt(0), '0');
    EXPECT_EQ(str.ByteAt(9), '9');
}

TEST(FixedStr, Length)
{
    FixStr str = "0123";
    EXPECT_EQ(str.Length(), 4);
    EXPECT_TRUE(strcmp(str.Data(), "0123") == 0);
}

TEST(FixedStr, CodepointAt)
{
    const char* utf8 = "qЯwgЫЧ";
    ASSERT_TRUE(utf8::Valid(utf8));

    FixStr str = utf8;
    EXPECT_EQ(str.CodePointAt(0), L'q');
    EXPECT_EQ(str.CodePointAt(3), L'g');
    // EXPECT_EQ(str.CodePointAt(4), L'Û');
}

TEST(FixedStr, Equals)
{
    FixStr a = "abc";
    FixStr b = "abc";
    FixStr c = "xyz";
    FixStr d = "qqqq";
    FixStr e = "";
    FixStr f;

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_EQ(e, f);
    EXPECT_NE(a, e);
    EXPECT_NE(d, f);
}

TEST(FixedStr, Slice)
{
    FixStr str = "===slice===";
    ASSERT_EQ(str(3, 8), "slice");
}

TEST(FixedStr, SmallConcat)
{
    FixStr a = "A";
    FixStr b = "B";
    ASSERT_EQ(a + b, "AB");
}

TEST(FixedStr, LongConcat)
{
    FixStr a(128, 'A');
    FixStr b(128, 'B');
    auto c = a + b;
    ASSERT_EQ(c(0, 128), a);
    ASSERT_EQ(c(128, 256), b);
}

TEST(FixedStr, Compare)
{
    EXPECT_EQ(FixStr32{}.Compare(FixStr64{}), 0);

    EXPECT_EQ(FixStr("abc").Compare(StringSlice("abc")), 0);
    EXPECT_EQ(FixStr("abc").Compare(FixStr("abc")), 0);

    EXPECT_EQ(FixStr("abc").Compare("abc"), 0);

    EXPECT_GT(FixStr("abcd").Compare("abc"), 0);
    EXPECT_LT(FixStr("abc").Compare("abcd"), 0);

    EXPECT_GT(FixStr("az").Compare("aa"), 0);
    EXPECT_LT(FixStr("aa").Compare("az"), 0);

    EXPECT_GT(FixStr("azz").Compare("aa"), 0);
    EXPECT_GT(FixStr("az").Compare("aaz"), 0);
    EXPECT_GT(FixStr("aza").Compare("aa"), 0);
    EXPECT_GT(FixStr("az").Compare("aaa"), 0);
}
