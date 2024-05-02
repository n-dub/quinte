#include <Core/String.hpp>
#include <gtest/gtest.h>

using namespace quinte;

TEST(Strings, EmptySizeCapacity)
{
    String str;
    ASSERT_EQ(str.Capacity(), 22);
    ASSERT_EQ(str.Size(), 0);
}

TEST(Strings, SmallSizeCapacity)
{
    String str = "q";
    ASSERT_EQ(str.Capacity(), 22);
    ASSERT_EQ(str.Size(), 1);
}

TEST(Strings, LongSizeCapacity)
{
    const char* cstr = "loooooooooooooooooooooooooooooooooooooooooong";
    String str = cstr;
    ASSERT_GE(str.Capacity(), 35);
    ASSERT_EQ(str.Size(), strlen(cstr));
}

TEST(Strings, SmallByteAt)
{
    String str = "0123456789";
    EXPECT_EQ(str.ByteAt(0), '0');
    EXPECT_EQ(str.ByteAt(9), '9');
}

TEST(Strings, LongByteAt)
{
    String str = "loooooooooooooooooooooooooooooooooooong 0123456789";
    EXPECT_EQ(str.ByteAt(40), '0');
    EXPECT_EQ(str.ByteAt(49), '9');
}

TEST(Strings, Length)
{
    String smalls = "0123";
    String longs = "loooooooooooooooooooooooooooooooooooong";
    EXPECT_EQ(smalls.Length(), 4);
    EXPECT_EQ(longs.Length(), 39);
}

TEST(Strings, SmallCodepointAt)
{
    const char* utf8 = "qЯwgЫЧ";
    ASSERT_TRUE(utf8::Valid(utf8));

    String str = utf8;
    EXPECT_EQ(str.CodePointAt(0), L'q');
    EXPECT_EQ(str.CodePointAt(3), L'g');
    // EXPECT_EQ(str.CodePointAt(4), L'Ы');
}

TEST(Strings, LongCodepointAt)
{
    const char* utf8 = "loooooooooooooooooooooooooooooooooooong qЯwgЫЧ";
    ASSERT_TRUE(utf8::Valid(utf8));

    String str = utf8;
    EXPECT_EQ(str.CodePointAt(40), L'q');
#if !defined ACTIAS_COMPILER_MSVC
    EXPECT_EQ(str.CodePointAt(44), L'Ы');
#endif
}

TEST(Strings, Equals)
{
    String a = "abc";
    String b = "abc";
    String c = "xyz";
    String d = "qqqq";
    String e = "";
    String f;

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_EQ(e, f);
    EXPECT_NE(a, e);
    EXPECT_NE(d, f);
}

TEST(Strings, Slice)
{
    String str = "===slice===";
    ASSERT_EQ(str(3, 8), "slice");
}

TEST(Strings, SmallConcat)
{
    String a = "A";
    String b = "B";
    ASSERT_EQ(a + b, "AB");
}

TEST(Strings, LongConcat)
{
    String a(128, 'A');
    String b(128, 'B');
    auto c = a + b;
    ASSERT_EQ(c(0, 128), a);
    ASSERT_EQ(c(128, 256), b);
}

TEST(Strings, ShrinkReserve)
{
    String str;
    EXPECT_EQ(str.Capacity(), 22); // initially small

    str.Reserve(128); // small -> long
    EXPECT_GE(str.Capacity(), 128);

    str.Append("123");
    str.Shrink(); // long -> small
    EXPECT_EQ(str.Capacity(), 22);

    const char l[] = "looooooooooooooooooooooooooooooooooooong";
    str.Append(l); // small -> long
    str.Shrink();  // long -> long
    EXPECT_GE(str.Capacity(), sizeof(l) + 3 - 1);
}

TEST(Strings, Compare)
{
    EXPECT_EQ(String{}.Compare(String{}), 0);

    EXPECT_EQ(String("abc").Compare(StringSlice("abc")), 0);
    EXPECT_EQ(String("abc").Compare(String("abc")), 0);

    EXPECT_EQ(String("abc").Compare("abc"), 0);

    EXPECT_GT(String("abcd").Compare("abc"), 0);
    EXPECT_LT(String("abc").Compare("abcd"), 0);

    EXPECT_GT(String("az").Compare("aa"), 0);
    EXPECT_LT(String("aa").Compare("az"), 0);

    EXPECT_GT(String("azz").Compare("aa"), 0);
    EXPECT_GT(String("az").Compare("aaz"), 0);
    EXPECT_GT(String("aza").Compare("aa"), 0);
    EXPECT_GT(String("az").Compare("aaa"), 0);
}

TEST(Strings, Strip)
{
    EXPECT_EQ(String("123").Strip(), StringSlice("123"));
    EXPECT_EQ(String("    123    ").StripRight(), StringSlice("    123"));
    EXPECT_EQ(String("    123    ").StripLeft(), StringSlice("123    "));
    EXPECT_EQ(String(" \t \n \r   ").StripLeft(), StringSlice{});
}

TEST(Strings, StartsWith)
{
    EXPECT_TRUE(String("").StartsWith(""));
    EXPECT_TRUE(String("1234").StartsWith(""));
    EXPECT_TRUE(String("1234").StartsWith("1"));
    EXPECT_TRUE(String("1234").StartsWith("12"));
    EXPECT_TRUE(String("1234").StartsWith("1234"));
    EXPECT_FALSE(String("1234").StartsWith("21"));
    EXPECT_FALSE(String("1234").StartsWith("12345"));
}

TEST(Strings, EndsWith)
{
    EXPECT_TRUE(String("").EndsWith(""));
    EXPECT_TRUE(String("1234").EndsWith(""));
    EXPECT_TRUE(String("1234").EndsWith("4"));
    EXPECT_TRUE(String("1234").EndsWith("34"));
    EXPECT_TRUE(String("1234").EndsWith("1234"));
    EXPECT_FALSE(String("1234").EndsWith("21"));
    EXPECT_FALSE(String("1234").EndsWith("12345"));
}
