/*  Copyright (c) 2016 Michael Hansen

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE. */

#include "st_format.h"
#include "st_assert.h"

#include <gtest/gtest.h>
#include <string>
#include <limits>
#include <iostream>
#include <cstdint>

namespace ST
{
    // Teach GTest how to print an ST::string
    static void PrintTo(const ST::string &str, std::ostream *os)
    {
        *os << "ST::string{\"" << str.c_str() << "\"}";
    }
}

TEST(format, escapes)
{
    EXPECT_EQ(ST_LITERAL("{x"), ST::format("{{{}", "x"));
    EXPECT_EQ(ST_LITERAL("x{"), ST::format("{}{{", "x"));
    EXPECT_EQ(ST_LITERAL("{{{{"), ST::format("{{{}{{{{", "{"));
    EXPECT_EQ(ST_LITERAL("{xxx{{yyy{"), ST::format("{{{}{{{{{}{{", "xxx", "yyy"));

    // Both single and double '}' are a valid escape for '}'
    EXPECT_EQ(ST_LITERAL("}"), ST::format("}"));
    EXPECT_EQ(ST_LITERAL("}"), ST::format("}}"));
    EXPECT_EQ(ST_LITERAL("{}"), ST::format("{{}"));
    EXPECT_EQ(ST_LITERAL("{}"), ST::format("{{}}"));
    EXPECT_EQ(ST_LITERAL("x}"), ST::format("{}}", "x"));
    EXPECT_EQ(ST_LITERAL("x}"), ST::format("{}}}", "x"));
    EXPECT_EQ(ST_LITERAL("{x}"), ST::format("{{{}}", "x"));
    EXPECT_EQ(ST_LITERAL("{x}"), ST::format("{{{}}}", "x"));
    EXPECT_EQ(ST_LITERAL("}}}x}"), ST::format("{>4_}}}", "x"));
    EXPECT_EQ(ST_LITERAL("}}}x}"), ST::format("{>4_}}}}", "x"));

    EXPECT_EQ(ST_LITERAL("}x"), ST::format("}x"));
    EXPECT_EQ(ST_LITERAL("}x"), ST::format("}}x"));
    EXPECT_EQ(ST_LITERAL("{}x"), ST::format("{{}x"));
    EXPECT_EQ(ST_LITERAL("{}x"), ST::format("{{}}x"));
    EXPECT_EQ(ST_LITERAL("x}x"), ST::format("{}}x", "x"));
    EXPECT_EQ(ST_LITERAL("x}x"), ST::format("{}}}x", "x"));
}

TEST(format, errors)
{
    EXPECT_THROW({ (void)ST::format("{", 1); }, ST::bad_format);
    EXPECT_THROW({ (void)ST::format("{.", 1); }, ST::bad_format);
    EXPECT_THROW({ (void)ST::format("{_", 1); }, ST::bad_format);
    EXPECT_THROW({ (void)ST::format("{&", 1); }, ST::bad_format);
    EXPECT_THROW({ (void)ST::format("{\x7f", 1); }, ST::bad_format);
    EXPECT_THROW({ (void)ST::format(nullptr, 1); }, std::invalid_argument);

    // Too many actual parameters is no longer an error due to arg references
    // However, attempting to use a parameter number that is not provided
    // should still throw an exception
    EXPECT_THROW({ (void)ST::format("{}{}", 1); }, std::out_of_range);
    EXPECT_THROW({ (void)ST::format("{&0}", 1); }, std::out_of_range);
    EXPECT_THROW({ (void)ST::format("{&2}", 1); }, std::out_of_range);
    EXPECT_THROW({ (void)ST::format("{}"); }, std::out_of_range);
}

TEST(format, strings)
{
    EXPECT_EQ(ST_LITERAL("TEST"), ST::format("{}", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{2}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{>2}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{<2}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{_-2}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTEST  xx"), ST::format("xx{6}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTEST  xx"), ST::format("xx{<6}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xx  TESTxx"), ST::format("xx{>6}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTEST--xx"), ST::format("xx{_-6}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTEST--xx"), ST::format("xx{<_-6}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xx--TESTxx"), ST::format("xx{>_-6}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxONE  TWO    THREExx"), ST::format("xx{5}{<5}{>7}xx", "ONE", "TWO", "THREE"));

    // Ensure braces are parsed properly within padding chars
    EXPECT_EQ(ST_LITERAL("xxTEST}}xx"), ST::format("xx{_}6}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTEST}}xx"), ST::format("xx{6_}}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTEST{{xx"), ST::format("xx{_{6}xx", "TEST"));
    EXPECT_EQ(ST_LITERAL("xxTEST{{xx"), ST::format("xx{6_{}xx", "TEST"));

    // Specifying precision on string formatting should truncate the string
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", "TESTXX"));
    EXPECT_EQ(ST_LITERAL("xxTEST  xx"), ST::format("xx{6.4}xx", "TESTXX"));
    EXPECT_EQ(ST_LITERAL("xxTESTXXxx"), ST::format("xx{4.6}xx", "TESTXX"));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{4.6}xx", "TEST"));

    // Test other string literal types
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{}xx", L"TEST"));
    EXPECT_EQ(ST_LITERAL("xxTEST-\xe8\x80\x81-\xf4\x8f\xbf\xbfxx"),
              ST::format("xx{}xx", u"TEST-\u8001-\U0010ffff"));
    EXPECT_EQ(ST_LITERAL("xxTEST-\xe8\x80\x81-\xf4\x8f\xbf\xbfxx"),
              ST::format("xx{}xx", U"TEST-\u8001-\U0010ffff"));
#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    EXPECT_EQ(ST_LITERAL("xxTEST-\xe8\x80\x81-\xf4\x8f\xbf\xbfxx"),
              ST::format("xx{}xx", u8"TEST-\u8001-\U0010ffff"));
#endif

    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", L"TESTXX"));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", u"TESTXX"));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", U"TESTXX"));
#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", u8"TESTXX"));
#endif
}

TEST(format, string_classes)
{
    // These should be handled just like normal const char* string params
    // (see above), so just need to test that the wrappers are working
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{}xx", ST_LITERAL("TEST")));

#if defined(ST_ENABLE_STL_STRINGS)
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{}xx", std::string("TEST")));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{}xx", std::wstring(L"TEST")));
    EXPECT_EQ(ST_LITERAL("xxTEST-\xe8\x80\x81-\xf4\x8f\xbf\xbfxx"),
              ST::format("xx{}xx", std::u16string(u"TEST-\u8001-\U0010ffff")));
    EXPECT_EQ(ST_LITERAL("xxTEST-\xe8\x80\x81-\xf4\x8f\xbf\xbfxx"),
              ST::format("xx{}xx", std::u32string(U"TEST-\u8001-\U0010ffff")));
#if defined(ST_HAVE_CXX20_CHAR8_TYPES)
    EXPECT_EQ(ST_LITERAL("xxTEST-\xe8\x80\x81-\xf4\x8f\xbf\xbfxx"),
              ST::format("xx{}xx", std::u8string(u8"TEST-\u8001-\U0010ffff")));
#endif

#if defined(ST_HAVE_CXX17_STRING_VIEW)
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{}xx", std::string_view("TEST")));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{}xx", std::wstring_view(L"TEST")));
    EXPECT_EQ(ST_LITERAL("xxTEST-\xe8\x80\x81-\xf4\x8f\xbf\xbfxx"),
              ST::format("xx{}xx", std::u16string_view(u"TEST-\u8001-\U0010ffff")));
    EXPECT_EQ(ST_LITERAL("xxTEST-\xe8\x80\x81-\xf4\x8f\xbf\xbfxx"),
              ST::format("xx{}xx", std::u32string_view(U"TEST-\u8001-\U0010ffff")));
#if defined(ST_HAVE_CXX20_CHAR8_TYPES)
    EXPECT_EQ(ST_LITERAL("xxTEST-\xe8\x80\x81-\xf4\x8f\xbf\xbfxx"),
              ST::format("xx{}xx", std::u8string_view(u8"TEST-\u8001-\U0010ffff")));
#endif
#endif  // defined(ST_HAVE_CXX17_STRING_VIEW)
#endif  // defined(ST_ENABLE_STL_STRINGS)

    // Specifying precision on string formatting should truncate the string
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", ST_LITERAL("TESTXX")));

#if defined(ST_ENABLE_STL_STRINGS)
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", std::string("TESTXX")));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", std::wstring(L"TESTXX")));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", std::u16string(u"TESTXX")));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", std::u32string(U"TESTXX")));
#if defined(ST_HAVE_CXX20_CHAR8_TYPES)
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", std::u8string(u8"TESTXX")));
#endif

#if defined(ST_HAVE_CXX17_STRING_VIEW)
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", std::string_view("TESTXX")));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", std::wstring_view(L"TESTXX")));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", std::u16string_view(u"TESTXX")));
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", std::u32string_view(U"TESTXX")));
#if defined(ST_HAVE_CXX20_CHAR8_TYPES)
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), ST::format("xx{.4}xx", std::u8string(u8"TESTXX")));
#endif
#endif  // defined(ST_HAVE_CXX17_STRING_VIEW)
#endif  // defined(ST_ENABLE_STL_STRINGS)
}

TEST(format, chars)
{
    EXPECT_EQ(ST_LITERAL("xxAxx"), ST::format("xx{c}xx", 'A'));
    EXPECT_EQ(ST_LITERAL("xxAxx"), ST::format("xx{c}xx", (signed char)'A'));
    EXPECT_EQ(ST_LITERAL("xxAxx"), ST::format("xx{c}xx", (unsigned char)'A'));
    EXPECT_EQ(ST_LITERAL("xxAxx"), ST::format("xx{c}xx", u'A'));
    EXPECT_EQ(ST_LITERAL("xxAxx"), ST::format("xx{c}xx", U'A'));

    // UTF-8 encoding of wide (16-bit) char
    EXPECT_EQ(ST_LITERAL("xx\xef\xbf\xbexx"), ST::format("xx{c}xx", L'\ufffe'));
    EXPECT_EQ(ST_LITERAL("xx\xe7\xbf\xbexx"), ST::format("xx{c}xx", (short)0x7ffe));
    EXPECT_EQ(ST_LITERAL("xx\xef\xbf\xbexx"), ST::format("xx{c}xx", (unsigned short)0xfffe));
    EXPECT_EQ(ST_LITERAL("xx\xef\xbf\xbexx"), ST::format("xx{c}xx", u'\ufffe'));

    // UTF-8 encoding of UCS4 (32-bit) char
    EXPECT_EQ(ST_LITERAL("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (int)0x10ffff));
    EXPECT_EQ(ST_LITERAL("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (unsigned int)0x10ffff));
    EXPECT_EQ(ST_LITERAL("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (long)0x10ffff));
    EXPECT_EQ(ST_LITERAL("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (unsigned long)0x10ffff));
    EXPECT_EQ(ST_LITERAL("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (long long)0x10ffff));
    EXPECT_EQ(ST_LITERAL("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", (unsigned long long)0x10ffff));
    EXPECT_EQ(ST_LITERAL("xx\xf4\x8f\xbf\xbfxx"), ST::format("xx{c}xx", U'\U0010ffff'));

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    // Leave UTF-8 code points alone
    EXPECT_EQ(ST_LITERAL("xxAxx"), ST::format("xx{c}xx", u8'A'));
    EXPECT_EQ(ST_LITERAL("xx\xef\xbf\xbexx"), ST::format("xx{c}{c}{c}xx",
              (char8_t)0xef, (char8_t)0xbf, (char8_t)0xbe));
#endif

    // char and wchar_t without the {c} format are now treated as integers!
    // See https://github.com/zrax/string_theory/issues/13 for details
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (char)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (char)123));
    if (std::numeric_limits<char>::is_signed) {
        EXPECT_EQ(ST_LITERAL("xx-123xx"), ST::format("xx{}xx", (char)-123));
    }
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (wchar_t)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (wchar_t)123));
    if (std::numeric_limits<wchar_t>::is_signed) {
        EXPECT_EQ(ST_LITERAL("xx-123xx"), ST::format("xx{}xx", (wchar_t)-123));
    }
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (char16_t)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (char16_t)123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (char32_t)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (char32_t)123));

#ifdef ST_HAVE_CXX20_CHAR8_TYPES
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (char8_t)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (char8_t)123));
#endif
}

TEST(format, decimal)
{
    EXPECT_EQ(ST_LITERAL("xx1234xx"), ST::format("xx{}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx1234xx"), ST::format("xx{d}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx1234xx"), ST::format("xx{2}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx1234xx"), ST::format("xx{>2}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx1234xx"), ST::format("xx{<2}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  1234xx"), ST::format("xx{6}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  1234xx"), ST::format("xx{>6}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx1234  xx"), ST::format("xx{<6}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   -1234xx"), ST::format("xx{8}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx   +1234xx"), ST::format("xx{+8}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-1234   xx"), ST::format("xx{<8}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+1234   xx"), ST::format("xx{<+8}xx", 1234));

    // Ensure precision isn't used for numeric formatting
    EXPECT_EQ(ST_LITERAL("xx1234xx"), ST::format("xx{.2}xx", 1234));

    // Numeric padding
    EXPECT_EQ(ST_LITERAL("xx001234xx"), ST::format("xx{06}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx001234xx"), ST::format("xx{<06}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0001234xx"), ST::format("xx{08}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0001234xx"), ST::format("xx{+08}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0001234xx"), ST::format("xx{<08}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0001234xx"), ST::format("xx{<+08}xx", 1234));

    // Character types
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{d}xx", '\0'));
    EXPECT_EQ(ST_LITERAL("xx65xx"), ST::format("xx{d}xx", 'A'));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{d}xx", L'\0'));
    EXPECT_EQ(ST_LITERAL("xx65xx"), ST::format("xx{d}xx", L'A'));
    EXPECT_EQ(ST_LITERAL("xx32767xx"), ST::format("xx{d}xx", L'\u7fff'));

    // 8-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (signed char)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (signed char)123));
    EXPECT_EQ(ST_LITERAL("xx-123xx"), ST::format("xx{}xx", (signed char)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (unsigned char)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (unsigned char)123));
    EXPECT_EQ(ST_LITERAL("xx127xx"), ST::format("xx{}xx", std::numeric_limits<int8_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+127xx"), ST::format("xx{+}xx", std::numeric_limits<int8_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-128xx"), ST::format("xx{}xx", std::numeric_limits<int8_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-128xx"), ST::format("xx{+}xx", std::numeric_limits<int8_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx255xx"), ST::format("xx{}xx", std::numeric_limits<uint8_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+255xx"), ST::format("xx{+}xx", std::numeric_limits<uint8_t>::max()));

    // 16-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (short)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (short)123));
    EXPECT_EQ(ST_LITERAL("xx-123xx"), ST::format("xx{}xx", (short)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (unsigned short)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (unsigned short)123));
    EXPECT_EQ(ST_LITERAL("xx32767xx"), ST::format("xx{}xx", std::numeric_limits<int16_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+32767xx"), ST::format("xx{+}xx", std::numeric_limits<int16_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-32768xx"), ST::format("xx{}xx", std::numeric_limits<int16_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-32768xx"), ST::format("xx{+}xx", std::numeric_limits<int16_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx65535xx"), ST::format("xx{}xx", std::numeric_limits<uint16_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+65535xx"), ST::format("xx{+}xx", std::numeric_limits<uint16_t>::max()));

    // 32-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (int)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (int)123));
    EXPECT_EQ(ST_LITERAL("xx-123xx"), ST::format("xx{}xx", (int)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (unsigned int)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (unsigned int)123));
    EXPECT_EQ(ST_LITERAL("xx2147483647xx"), ST::format("xx{}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+2147483647xx"), ST::format("xx{+}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-2147483648xx"), ST::format("xx{}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-2147483648xx"), ST::format("xx{+}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx4294967295xx"), ST::format("xx{}xx", std::numeric_limits<uint32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+4294967295xx"), ST::format("xx{+}xx", std::numeric_limits<uint32_t>::max()));

    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (long)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (long)123));
    EXPECT_EQ(ST_LITERAL("xx-123xx"), ST::format("xx{}xx", (long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (unsigned long)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (unsigned long)123));

    // 64-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (long long)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (long long)123));
    EXPECT_EQ(ST_LITERAL("xx-123xx"), ST::format("xx{}xx", (long long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{}xx", (unsigned long long)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{}xx", (unsigned long long)123));
#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("xx9223372036854775807xx"), ST::format("xx{}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+9223372036854775807xx"), ST::format("xx{+}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-9223372036854775808xx"), ST::format("xx{}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-9223372036854775808xx"), ST::format("xx{+}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx18446744073709551615xx"), ST::format("xx{}xx", std::numeric_limits<uint64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+18446744073709551615xx"), ST::format("xx{+}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, decimal_prefix)
{
    EXPECT_EQ(ST_LITERAL("xx1234xx"), ST::format("xx{#}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx1234xx"), ST::format("xx{#d}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx1234xx"), ST::format("xx{#2}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx1234xx"), ST::format("xx{>#2}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx1234xx"), ST::format("xx{<2#}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  1234xx"), ST::format("xx{#6}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  1234xx"), ST::format("xx{>#6}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx1234  xx"), ST::format("xx{<#6}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   +1234xx"), ST::format("xx{#+8}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   -1234xx"), ST::format("xx{#8}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+1234   xx"), ST::format("xx{<#+8}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-1234   xx"), ST::format("xx{<#8}xx", -1234));

    // Numeric padding
    EXPECT_EQ(ST_LITERAL("xx001234xx"), ST::format("xx{#06}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx001234xx"), ST::format("xx{<0#6}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0001234xx"), ST::format("xx{#08}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0001234xx"), ST::format("xx{#+08}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0001234xx"), ST::format("xx{<#08}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0001234xx"), ST::format("xx{<#+08}xx", 1234));

    // Character types
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#d}xx", '\0'));
    EXPECT_EQ(ST_LITERAL("xx65xx"), ST::format("xx{#d}xx", 'A'));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#d}xx", L'\0'));
    EXPECT_EQ(ST_LITERAL("xx65xx"), ST::format("xx{#d}xx", L'A'));
    EXPECT_EQ(ST_LITERAL("xx32767xx"), ST::format("xx{#d}xx", L'\u7fff'));

    // 32-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#}xx", (int)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{#}xx", (int)123));
    EXPECT_EQ(ST_LITERAL("xx-123xx"), ST::format("xx{#}xx", (int)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#}xx", (unsigned int)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{#}xx", (unsigned int)123));
    EXPECT_EQ(ST_LITERAL("xx2147483647xx"), ST::format("xx{#}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+2147483647xx"), ST::format("xx{+#}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-2147483648xx"), ST::format("xx{#}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-2147483648xx"), ST::format("xx{+#}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx4294967295xx"), ST::format("xx{#}xx", std::numeric_limits<uint32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+4294967295xx"), ST::format("xx{+#}xx", std::numeric_limits<uint32_t>::max()));

    // 64-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#}xx", (long long)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{#}xx", (long long)123));
    EXPECT_EQ(ST_LITERAL("xx-123xx"), ST::format("xx{#}xx", (long long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#}xx", (unsigned long long)0));
    EXPECT_EQ(ST_LITERAL("xx123xx"), ST::format("xx{#}xx", (unsigned long long)123));
#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("xx9223372036854775807xx"), ST::format("xx{#}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+9223372036854775807xx"), ST::format("xx{+#}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-9223372036854775808xx"), ST::format("xx{#}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-9223372036854775808xx"), ST::format("xx{+#}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx18446744073709551615xx"), ST::format("xx{#}xx", std::numeric_limits<uint64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+18446744073709551615xx"), ST::format("xx{+#}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, hex)
{
    EXPECT_EQ(ST_LITERAL("xx4d2xx"), ST::format("xx{x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx4d2xx"), ST::format("xx{x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx4d2xx"), ST::format("xx{2x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx4d2xx"), ST::format("xx{>2x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx4d2xx"), ST::format("xx{<2x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   4d2xx"), ST::format("xx{6x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   4d2xx"), ST::format("xx{>6x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx4d2   xx"), ST::format("xx{<6x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  +4d2xx"), ST::format("xx{+6x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  -4d2xx"), ST::format("xx{6x}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+4d2  xx"), ST::format("xx{<+6x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-4d2  xx"), ST::format("xx{<6x}xx", -1234));

    // Numeric padding
    EXPECT_EQ(ST_LITERAL("xx0004d2xx"), ST::format("xx{06x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0004d2xx"), ST::format("xx{<06x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-004d2xx"), ST::format("xx{06x}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+004d2xx"), ST::format("xx{+06x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-004d2xx"), ST::format("xx{<06x}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+004d2xx"), ST::format("xx{<+06x}xx", 1234));

    // Character types
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", '\0'));
    EXPECT_EQ(ST_LITERAL("xx41xx"), ST::format("xx{x}xx", 'A'));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", L'\0'));
    EXPECT_EQ(ST_LITERAL("xx41xx"), ST::format("xx{x}xx", L'A'));
    EXPECT_EQ(ST_LITERAL("xx7fffxx"), ST::format("xx{x}xx", L'\u7fff'));

    // 8-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", (signed char)0));
    EXPECT_EQ(ST_LITERAL("xx7bxx"), ST::format("xx{x}xx", (signed char)123));
    EXPECT_EQ(ST_LITERAL("xx-7bxx"), ST::format("xx{x}xx", (signed char)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", (unsigned char)0));
    EXPECT_EQ(ST_LITERAL("xx7bxx"), ST::format("xx{x}xx", (unsigned char)123));
    EXPECT_EQ(ST_LITERAL("xx7fxx"), ST::format("xx{x}xx", std::numeric_limits<int8_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+7fxx"), ST::format("xx{+x}xx", std::numeric_limits<int8_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-80xx"), ST::format("xx{x}xx", std::numeric_limits<int8_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-80xx"), ST::format("xx{+x}xx", std::numeric_limits<int8_t>::min()));
    EXPECT_EQ(ST_LITERAL("xxffxx"), ST::format("xx{x}xx", std::numeric_limits<uint8_t>::max()));

    // 16-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", (short)0));
    EXPECT_EQ(ST_LITERAL("xx7bxx"), ST::format("xx{x}xx", (short)123));
    EXPECT_EQ(ST_LITERAL("xx-7bxx"), ST::format("xx{x}xx", (short)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", (unsigned short)0));
    EXPECT_EQ(ST_LITERAL("xx7bxx"), ST::format("xx{x}xx", (unsigned short)123));
    EXPECT_EQ(ST_LITERAL("xx7fffxx"), ST::format("xx{x}xx", std::numeric_limits<int16_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+7fffxx"), ST::format("xx{+x}xx", std::numeric_limits<int16_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-8000xx"), ST::format("xx{x}xx", std::numeric_limits<int16_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-8000xx"), ST::format("xx{+x}xx", std::numeric_limits<int16_t>::min()));
    EXPECT_EQ(ST_LITERAL("xxffffxx"), ST::format("xx{x}xx", std::numeric_limits<uint16_t>::max()));

    // 32-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", (int)0));
    EXPECT_EQ(ST_LITERAL("xx7bxx"), ST::format("xx{x}xx", (int)123));
    EXPECT_EQ(ST_LITERAL("xx-7bxx"), ST::format("xx{x}xx", (int)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", (unsigned int)0));
    EXPECT_EQ(ST_LITERAL("xx7bxx"), ST::format("xx{x}xx", (unsigned int)123));
    EXPECT_EQ(ST_LITERAL("xx7fffffffxx"), ST::format("xx{x}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+7fffffffxx"), ST::format("xx{+x}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-80000000xx"), ST::format("xx{x}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-80000000xx"), ST::format("xx{+x}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xxffffffffxx"), ST::format("xx{x}xx", std::numeric_limits<uint32_t>::max()));

    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", (long)0));
    EXPECT_EQ(ST_LITERAL("xx7bxx"), ST::format("xx{x}xx", (long)123));
    EXPECT_EQ(ST_LITERAL("xx-7bxx"), ST::format("xx{x}xx", (long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", (unsigned long)0));
    EXPECT_EQ(ST_LITERAL("xx7bxx"), ST::format("xx{x}xx", (unsigned long)123));

    // 64-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", (long long)0));
    EXPECT_EQ(ST_LITERAL("xx7bxx"), ST::format("xx{x}xx", (long long)123));
    EXPECT_EQ(ST_LITERAL("xx-7bxx"), ST::format("xx{x}xx", (long long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{x}xx", (unsigned long long)0));
    EXPECT_EQ(ST_LITERAL("xx7bxx"), ST::format("xx{x}xx", (unsigned long long)123));
#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("xx7fffffffffffffffxx"), ST::format("xx{x}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+7fffffffffffffffxx"), ST::format("xx{+x}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-8000000000000000xx"), ST::format("xx{x}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-8000000000000000xx"), ST::format("xx{+x}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xxffffffffffffffffxx"), ST::format("xx{x}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, hex_prefix)
{
    EXPECT_EQ(ST_LITERAL("xx0x4d2xx"), ST::format("xx{#x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0x4d2xx"), ST::format("xx{#x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0x4d2xx"), ST::format("xx{#2x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0x4d2xx"), ST::format("xx{>#2x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0x4d2xx"), ST::format("xx{<#2x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   0x4d2xx"), ST::format("xx{#8x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   0x4d2xx"), ST::format("xx{>#8x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0x4d2   xx"), ST::format("xx{<#8x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  +0x4d2xx"), ST::format("xx{+#8x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  -0x4d2xx"), ST::format("xx{#8x}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0x4d2  xx"), ST::format("xx{<+#8x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0x4d2  xx"), ST::format("xx{<#8x}xx", -1234));

    // Numeric padding
    EXPECT_EQ(ST_LITERAL("xx0x0004d2xx"), ST::format("xx{#08x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0x0004d2xx"), ST::format("xx{<#08x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0x004d2xx"), ST::format("xx{#08x}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0x004d2xx"), ST::format("xx{+#08x}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0x004d2xx"), ST::format("xx{<#08x}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0x004d2xx"), ST::format("xx{<+#08x}xx", 1234));

    // Character types
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#x}xx", '\0'));
    EXPECT_EQ(ST_LITERAL("xx0x41xx"), ST::format("xx{#x}xx", 'A'));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#x}xx", L'\0'));
    EXPECT_EQ(ST_LITERAL("xx0x41xx"), ST::format("xx{#x}xx", L'A'));
    EXPECT_EQ(ST_LITERAL("xx0x7fffxx"), ST::format("xx{#x}xx", L'\u7fff'));

    // 32-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#x}xx", (int)0));
    EXPECT_EQ(ST_LITERAL("xx0x7bxx"), ST::format("xx{#x}xx", (int)123));
    EXPECT_EQ(ST_LITERAL("xx-0x7bxx"), ST::format("xx{#x}xx", (int)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#x}xx", (unsigned int)0));
    EXPECT_EQ(ST_LITERAL("xx0x7bxx"), ST::format("xx{#x}xx", (unsigned int)123));
    EXPECT_EQ(ST_LITERAL("xx0x7fffffffxx"), ST::format("xx{#x}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+0x7fffffffxx"), ST::format("xx{+#x}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-0x80000000xx"), ST::format("xx{#x}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-0x80000000xx"), ST::format("xx{+#x}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx0xffffffffxx"), ST::format("xx{#x}xx", std::numeric_limits<uint32_t>::max()));

    // 64-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#x}xx", (long long)0));
    EXPECT_EQ(ST_LITERAL("xx0x7bxx"), ST::format("xx{#x}xx", (long long)123));
    EXPECT_EQ(ST_LITERAL("xx-0x7bxx"), ST::format("xx{#x}xx", (long long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#x}xx", (unsigned long long)0));
    EXPECT_EQ(ST_LITERAL("xx0x7bxx"), ST::format("xx{#x}xx", (unsigned long long)123));
#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("xx0x7fffffffffffffffxx"), ST::format("xx{#x}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+0x7fffffffffffffffxx"), ST::format("xx{+#x}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-0x8000000000000000xx"), ST::format("xx{#x}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-0x8000000000000000xx"), ST::format("xx{+#x}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx0xffffffffffffffffxx"), ST::format("xx{#x}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, hex_upper)
{
    EXPECT_EQ(ST_LITERAL("xx4D2xx"), ST::format("xx{X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx4D2xx"), ST::format("xx{X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx4D2xx"), ST::format("xx{2X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx4D2xx"), ST::format("xx{>2X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx4D2xx"), ST::format("xx{<2X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   4D2xx"), ST::format("xx{6X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   4D2xx"), ST::format("xx{>6X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx4D2   xx"), ST::format("xx{<6X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  +4D2xx"), ST::format("xx{+6X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  -4D2xx"), ST::format("xx{6X}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+4D2  xx"), ST::format("xx{<+6X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-4D2  xx"), ST::format("xx{<6X}xx", -1234));

    // Numeric padding
    EXPECT_EQ(ST_LITERAL("xx0004D2xx"), ST::format("xx{06X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0004D2xx"), ST::format("xx{<06X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-004D2xx"), ST::format("xx{06X}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+004D2xx"), ST::format("xx{+06X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-004D2xx"), ST::format("xx{<06X}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+004D2xx"), ST::format("xx{<+06X}xx", 1234));

    // Character types
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", '\0'));
    EXPECT_EQ(ST_LITERAL("xx41xx"), ST::format("xx{X}xx", 'A'));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", L'\0'));
    EXPECT_EQ(ST_LITERAL("xx41xx"), ST::format("xx{X}xx", L'A'));
    EXPECT_EQ(ST_LITERAL("xx7FFFxx"), ST::format("xx{X}xx", L'\u7fff'));

    // 8-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", (signed char)0));
    EXPECT_EQ(ST_LITERAL("xx7Bxx"), ST::format("xx{X}xx", (signed char)123));
    EXPECT_EQ(ST_LITERAL("xx-7Bxx"), ST::format("xx{X}xx", (signed char)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", (unsigned char)0));
    EXPECT_EQ(ST_LITERAL("xx7Bxx"), ST::format("xx{X}xx", (unsigned char)123));
    EXPECT_EQ(ST_LITERAL("xx7Fxx"), ST::format("xx{X}xx", std::numeric_limits<int8_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+7Fxx"), ST::format("xx{+X}xx", std::numeric_limits<int8_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-80xx"), ST::format("xx{X}xx", std::numeric_limits<int8_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-80xx"), ST::format("xx{+X}xx", std::numeric_limits<int8_t>::min()));
    EXPECT_EQ(ST_LITERAL("xxFFxx"), ST::format("xx{X}xx", std::numeric_limits<uint8_t>::max()));

    // 16-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", (short)0));
    EXPECT_EQ(ST_LITERAL("xx7Bxx"), ST::format("xx{X}xx", (short)123));
    EXPECT_EQ(ST_LITERAL("xx-7Bxx"), ST::format("xx{X}xx", (short)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", (unsigned short)0));
    EXPECT_EQ(ST_LITERAL("xx7Bxx"), ST::format("xx{X}xx", (unsigned short)123));
    EXPECT_EQ(ST_LITERAL("xx7FFFxx"), ST::format("xx{X}xx", std::numeric_limits<int16_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+7FFFxx"), ST::format("xx{+X}xx", std::numeric_limits<int16_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-8000xx"), ST::format("xx{X}xx", std::numeric_limits<int16_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-8000xx"), ST::format("xx{+X}xx", std::numeric_limits<int16_t>::min()));
    EXPECT_EQ(ST_LITERAL("xxFFFFxx"), ST::format("xx{X}xx", std::numeric_limits<uint16_t>::max()));

    // 32-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", (int)0));
    EXPECT_EQ(ST_LITERAL("xx7Bxx"), ST::format("xx{X}xx", (int)123));
    EXPECT_EQ(ST_LITERAL("xx-7Bxx"), ST::format("xx{X}xx", (int)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", (unsigned int)0));
    EXPECT_EQ(ST_LITERAL("xx7Bxx"), ST::format("xx{X}xx", (unsigned int)123));
    EXPECT_EQ(ST_LITERAL("xx7FFFFFFFxx"), ST::format("xx{X}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+7FFFFFFFxx"), ST::format("xx{+X}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-80000000xx"), ST::format("xx{X}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-80000000xx"), ST::format("xx{+X}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xxFFFFFFFFxx"), ST::format("xx{X}xx", std::numeric_limits<uint32_t>::max()));

    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", (long)0));
    EXPECT_EQ(ST_LITERAL("xx7Bxx"), ST::format("xx{X}xx", (long)123));
    EXPECT_EQ(ST_LITERAL("xx-7Bxx"), ST::format("xx{X}xx", (long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", (unsigned long)0));
    EXPECT_EQ(ST_LITERAL("xx7Bxx"), ST::format("xx{X}xx", (unsigned long)123));

    // 64-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", (long long)0));
    EXPECT_EQ(ST_LITERAL("xx7Bxx"), ST::format("xx{X}xx", (long long)123));
    EXPECT_EQ(ST_LITERAL("xx-7Bxx"), ST::format("xx{X}xx", (long long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{X}xx", (unsigned long long)0));
    EXPECT_EQ(ST_LITERAL("xx7Bxx"), ST::format("xx{X}xx", (unsigned long long)123));
#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("xx7FFFFFFFFFFFFFFFxx"), ST::format("xx{X}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+7FFFFFFFFFFFFFFFxx"), ST::format("xx{+X}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-8000000000000000xx"), ST::format("xx{X}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-8000000000000000xx"), ST::format("xx{+X}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xxFFFFFFFFFFFFFFFFxx"), ST::format("xx{X}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, hex_upper_prefix)
{
    EXPECT_EQ(ST_LITERAL("xx0X4D2xx"), ST::format("xx{#X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0X4D2xx"), ST::format("xx{#X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0X4D2xx"), ST::format("xx{#2X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0X4D2xx"), ST::format("xx{>#2X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0X4D2xx"), ST::format("xx{<#2X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   0X4D2xx"), ST::format("xx{#8X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   0X4D2xx"), ST::format("xx{>#8X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0X4D2   xx"), ST::format("xx{<#8X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  +0X4D2xx"), ST::format("xx{+#8X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  -0X4D2xx"), ST::format("xx{#8X}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0X4D2  xx"), ST::format("xx{<+#8X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0X4D2  xx"), ST::format("xx{<#8X}xx", -1234));

    // Numeric padding
    EXPECT_EQ(ST_LITERAL("xx0X0004D2xx"), ST::format("xx{#08X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0X0004D2xx"), ST::format("xx{<#08X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0X004D2xx"), ST::format("xx{#08X}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0X004D2xx"), ST::format("xx{+#08X}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0X004D2xx"), ST::format("xx{<#08X}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0X004D2xx"), ST::format("xx{<+#08X}xx", 1234));

    // Character types
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#X}xx", '\0'));
    EXPECT_EQ(ST_LITERAL("xx0X41xx"), ST::format("xx{#X}xx", 'A'));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#X}xx", L'\0'));
    EXPECT_EQ(ST_LITERAL("xx0X41xx"), ST::format("xx{#X}xx", L'A'));
    EXPECT_EQ(ST_LITERAL("xx0X7FFFxx"), ST::format("xx{#X}xx", L'\u7fff'));

    // 32-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#X}xx", (int)0));
    EXPECT_EQ(ST_LITERAL("xx0X7Bxx"), ST::format("xx{#X}xx", (int)123));
    EXPECT_EQ(ST_LITERAL("xx-0X7Bxx"), ST::format("xx{#X}xx", (int)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#X}xx", (unsigned int)0));
    EXPECT_EQ(ST_LITERAL("xx0X7Bxx"), ST::format("xx{#X}xx", (unsigned int)123));
    EXPECT_EQ(ST_LITERAL("xx0X7FFFFFFFxx"), ST::format("xx{#X}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+0X7FFFFFFFxx"), ST::format("xx{+#X}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-0X80000000xx"), ST::format("xx{#X}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-0X80000000xx"), ST::format("xx{+#X}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx0XFFFFFFFFxx"), ST::format("xx{#X}xx", std::numeric_limits<uint32_t>::max()));

    // 64-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#X}xx", (long long)0));
    EXPECT_EQ(ST_LITERAL("xx0X7Bxx"), ST::format("xx{#X}xx", (long long)123));
    EXPECT_EQ(ST_LITERAL("xx-0X7Bxx"), ST::format("xx{#X}xx", (long long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#X}xx", (unsigned long long)0));
    EXPECT_EQ(ST_LITERAL("xx0X7Bxx"), ST::format("xx{#X}xx", (unsigned long long)123));
#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("xx0X7FFFFFFFFFFFFFFFxx"), ST::format("xx{#X}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+0X7FFFFFFFFFFFFFFFxx"), ST::format("xx{+#X}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-0X8000000000000000xx"), ST::format("xx{#X}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-0X8000000000000000xx"), ST::format("xx{+#X}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx0XFFFFFFFFFFFFFFFFxx"), ST::format("xx{#X}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, octal)
{
    EXPECT_EQ(ST_LITERAL("xx2322xx"), ST::format("xx{o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx2322xx"), ST::format("xx{o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx2322xx"), ST::format("xx{2o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx2322xx"), ST::format("xx{>2o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx2322xx"), ST::format("xx{<2o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  2322xx"), ST::format("xx{6o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  2322xx"), ST::format("xx{>6o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx2322  xx"), ST::format("xx{<6o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   +2322xx"), ST::format("xx{+8o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   -2322xx"), ST::format("xx{8o}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+2322   xx"), ST::format("xx{<+8o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-2322   xx"), ST::format("xx{<8o}xx", -1234));

    // Numeric padding
    EXPECT_EQ(ST_LITERAL("xx002322xx"), ST::format("xx{06o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx002322xx"), ST::format("xx{<06o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0002322xx"), ST::format("xx{08o}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0002322xx"), ST::format("xx{+08o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0002322xx"), ST::format("xx{<08o}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0002322xx"), ST::format("xx{<+08o}xx", 1234));

    // Character types
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", '\0'));
    EXPECT_EQ(ST_LITERAL("xx101xx"), ST::format("xx{o}xx", 'A'));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", L'\0'));
    EXPECT_EQ(ST_LITERAL("xx101xx"), ST::format("xx{o}xx", L'A'));
    EXPECT_EQ(ST_LITERAL("xx77777xx"), ST::format("xx{o}xx", L'\u7fff'));

    // 8-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", (signed char)0));
    EXPECT_EQ(ST_LITERAL("xx173xx"), ST::format("xx{o}xx", (signed char)123));
    EXPECT_EQ(ST_LITERAL("xx-173xx"), ST::format("xx{o}xx", (signed char)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", (unsigned char)0));
    EXPECT_EQ(ST_LITERAL("xx173xx"), ST::format("xx{o}xx", (unsigned char)123));
    EXPECT_EQ(ST_LITERAL("xx177xx"), ST::format("xx{o}xx", std::numeric_limits<int8_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+177xx"), ST::format("xx{+o}xx", std::numeric_limits<int8_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-200xx"), ST::format("xx{o}xx", std::numeric_limits<int8_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-200xx"), ST::format("xx{+o}xx", std::numeric_limits<int8_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx377xx"), ST::format("xx{o}xx", std::numeric_limits<uint8_t>::max()));

    // 16-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", (short)0));
    EXPECT_EQ(ST_LITERAL("xx173xx"), ST::format("xx{o}xx", (short)123));
    EXPECT_EQ(ST_LITERAL("xx-173xx"), ST::format("xx{o}xx", (short)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", (unsigned short)0));
    EXPECT_EQ(ST_LITERAL("xx173xx"), ST::format("xx{o}xx", (unsigned short)123));
    EXPECT_EQ(ST_LITERAL("xx77777xx"), ST::format("xx{o}xx", std::numeric_limits<int16_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+77777xx"), ST::format("xx{+o}xx", std::numeric_limits<int16_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-100000xx"), ST::format("xx{o}xx", std::numeric_limits<int16_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-100000xx"), ST::format("xx{+o}xx", std::numeric_limits<int16_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx177777xx"), ST::format("xx{o}xx", std::numeric_limits<uint16_t>::max()));

    // 32-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", (int)0));
    EXPECT_EQ(ST_LITERAL("xx173xx"), ST::format("xx{o}xx", (int)123));
    EXPECT_EQ(ST_LITERAL("xx-173xx"), ST::format("xx{o}xx", (int)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", (unsigned int)0));
    EXPECT_EQ(ST_LITERAL("xx173xx"), ST::format("xx{o}xx", (unsigned int)123));
    EXPECT_EQ(ST_LITERAL("xx17777777777xx"), ST::format("xx{o}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+17777777777xx"), ST::format("xx{+o}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-20000000000xx"), ST::format("xx{o}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-20000000000xx"), ST::format("xx{+o}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx37777777777xx"), ST::format("xx{o}xx", std::numeric_limits<uint32_t>::max()));

    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", (long)0));
    EXPECT_EQ(ST_LITERAL("xx173xx"), ST::format("xx{o}xx", (long)123));
    EXPECT_EQ(ST_LITERAL("xx-173xx"), ST::format("xx{o}xx", (long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", (unsigned long)0));
    EXPECT_EQ(ST_LITERAL("xx173xx"), ST::format("xx{o}xx", (unsigned long)123));

    // 64-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", (long long)0));
    EXPECT_EQ(ST_LITERAL("xx173xx"), ST::format("xx{o}xx", (long long)123));
    EXPECT_EQ(ST_LITERAL("xx-173xx"), ST::format("xx{o}xx", (long long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{o}xx", (unsigned long long)0));
    EXPECT_EQ(ST_LITERAL("xx173xx"), ST::format("xx{o}xx", (unsigned long long)123));
#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("xx777777777777777777777xx"), ST::format("xx{o}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+777777777777777777777xx"), ST::format("xx{+o}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-1000000000000000000000xx"), ST::format("xx{o}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-1000000000000000000000xx"), ST::format("xx{+o}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx1777777777777777777777xx"), ST::format("xx{o}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, octal_prefix)
{
    EXPECT_EQ(ST_LITERAL("xx02322xx"), ST::format("xx{#o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx02322xx"), ST::format("xx{#o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx02322xx"), ST::format("xx{#2o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx02322xx"), ST::format("xx{>#2o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx02322xx"), ST::format("xx{<#2o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  02322xx"), ST::format("xx{#7o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  02322xx"), ST::format("xx{>#7o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx02322  xx"), ST::format("xx{<#7o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  +02322xx"), ST::format("xx{#+8o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  -02322xx"), ST::format("xx{#8o}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+02322  xx"), ST::format("xx{<#+8o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-02322  xx"), ST::format("xx{<#8o}xx", -1234));

    // Numeric padding
    EXPECT_EQ(ST_LITERAL("xx0002322xx"), ST::format("xx{#07o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0002322xx"), ST::format("xx{<#07o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0002322xx"), ST::format("xx{#08o}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0002322xx"), ST::format("xx{#+08o}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0002322xx"), ST::format("xx{<#08o}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0002322xx"), ST::format("xx{<#+08o}xx", 1234));

    // Character types
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#o}xx", '\0'));
    EXPECT_EQ(ST_LITERAL("xx0101xx"), ST::format("xx{#o}xx", 'A'));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#o}xx", L'\0'));
    EXPECT_EQ(ST_LITERAL("xx0101xx"), ST::format("xx{#o}xx", L'A'));
    EXPECT_EQ(ST_LITERAL("xx077777xx"), ST::format("xx{#o}xx", L'\u7fff'));

    // 32-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#o}xx", (int)0));
    EXPECT_EQ(ST_LITERAL("xx0173xx"), ST::format("xx{#o}xx", (int)123));
    EXPECT_EQ(ST_LITERAL("xx-0173xx"), ST::format("xx{#o}xx", (int)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#o}xx", (unsigned int)0));
    EXPECT_EQ(ST_LITERAL("xx0173xx"), ST::format("xx{#o}xx", (unsigned int)123));
    EXPECT_EQ(ST_LITERAL("xx017777777777xx"), ST::format("xx{#o}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+017777777777xx"), ST::format("xx{+#o}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-020000000000xx"), ST::format("xx{#o}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-020000000000xx"), ST::format("xx{+#o}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx037777777777xx"), ST::format("xx{#o}xx", std::numeric_limits<uint32_t>::max()));

    // 64-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#o}xx", (long long)0));
    EXPECT_EQ(ST_LITERAL("xx0173xx"), ST::format("xx{#o}xx", (long long)123));
    EXPECT_EQ(ST_LITERAL("xx-0173xx"), ST::format("xx{#o}xx", (long long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#o}xx", (unsigned long long)0));
    EXPECT_EQ(ST_LITERAL("xx0173xx"), ST::format("xx{#o}xx", (unsigned long long)123));
#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("xx0777777777777777777777xx"), ST::format("xx{#o}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+0777777777777777777777xx"), ST::format("xx{+#o}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-01000000000000000000000xx"), ST::format("xx{#o}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-01000000000000000000000xx"), ST::format("xx{+#o}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx01777777777777777777777xx"), ST::format("xx{#o}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, binary)
{
    EXPECT_EQ(ST_LITERAL("xx10011010010xx"), ST::format("xx{b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx10011010010xx"), ST::format("xx{b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx10011010010xx"), ST::format("xx{2b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx10011010010xx"), ST::format("xx{>2b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx10011010010xx"), ST::format("xx{<2b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx     10011010010xx"), ST::format("xx{16b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx     10011010010xx"), ST::format("xx{>16b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx10011010010     xx"), ST::format("xx{<16b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx    +10011010010xx"), ST::format("xx{+16b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx    -10011010010xx"), ST::format("xx{16b}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+10011010010    xx"), ST::format("xx{<+16b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-10011010010    xx"), ST::format("xx{<16b}xx", -1234));

    // Numeric padding
    EXPECT_EQ(ST_LITERAL("xx0000010011010010xx"), ST::format("xx{016b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0000010011010010xx"), ST::format("xx{<016b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-000010011010010xx"), ST::format("xx{016b}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+000010011010010xx"), ST::format("xx{+016b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-000010011010010xx"), ST::format("xx{<016b}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+000010011010010xx"), ST::format("xx{<+016b}xx", 1234));

    // Character types
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", '\0'));
    EXPECT_EQ(ST_LITERAL("xx1000001xx"), ST::format("xx{b}xx", 'A'));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", L'\0'));
    EXPECT_EQ(ST_LITERAL("xx1000001xx"), ST::format("xx{b}xx", L'A'));
    EXPECT_EQ(ST_LITERAL("xx111111111111111xx"), ST::format("xx{b}xx", L'\u7fff'));

    // Numeric char types
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", (signed char)0));
    EXPECT_EQ(ST_LITERAL("xx1111011xx"), ST::format("xx{b}xx", (signed char)123));
    EXPECT_EQ(ST_LITERAL("xx-1111011xx"), ST::format("xx{b}xx", (signed char)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", (unsigned char)0));
    EXPECT_EQ(ST_LITERAL("xx1111011xx"), ST::format("xx{b}xx", (unsigned char)123));
    EXPECT_EQ(ST_LITERAL("xx1111111xx"), ST::format("xx{b}xx", std::numeric_limits<int8_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+1111111xx"), ST::format("xx{+b}xx", std::numeric_limits<int8_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-10000000xx"), ST::format("xx{b}xx", std::numeric_limits<int8_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-10000000xx"), ST::format("xx{+b}xx", std::numeric_limits<int8_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx11111111xx"), ST::format("xx{b}xx", std::numeric_limits<uint8_t>::max()));

    // 16-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", (short)0));
    EXPECT_EQ(ST_LITERAL("xx1111011xx"), ST::format("xx{b}xx", (short)123));
    EXPECT_EQ(ST_LITERAL("xx-1111011xx"), ST::format("xx{b}xx", (short)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", (unsigned short)0));
    EXPECT_EQ(ST_LITERAL("xx1111011xx"), ST::format("xx{b}xx", (unsigned short)123));
    EXPECT_EQ(ST_LITERAL("xx111111111111111xx"), ST::format("xx{b}xx", std::numeric_limits<int16_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+111111111111111xx"), ST::format("xx{+b}xx", std::numeric_limits<int16_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-1000000000000000xx"), ST::format("xx{b}xx", std::numeric_limits<int16_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-1000000000000000xx"), ST::format("xx{+b}xx", std::numeric_limits<int16_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx1111111111111111xx"), ST::format("xx{b}xx", std::numeric_limits<uint16_t>::max()));

    // 32-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", (int)0));
    EXPECT_EQ(ST_LITERAL("xx1111011xx"), ST::format("xx{b}xx", (int)123));
    EXPECT_EQ(ST_LITERAL("xx-1111011xx"), ST::format("xx{b}xx", (int)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", (unsigned int)0));
    EXPECT_EQ(ST_LITERAL("xx1111011xx"), ST::format("xx{b}xx", (unsigned int)123));
    EXPECT_EQ(ST_LITERAL("xx1111111111111111111111111111111xx"),
              ST::format("xx{b}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+1111111111111111111111111111111xx"),
              ST::format("xx{+b}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-10000000000000000000000000000000xx"),
              ST::format("xx{b}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-10000000000000000000000000000000xx"),
              ST::format("xx{+b}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx11111111111111111111111111111111xx"),
              ST::format("xx{b}xx", std::numeric_limits<uint32_t>::max()));

    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", (long)0));
    EXPECT_EQ(ST_LITERAL("xx1111011xx"), ST::format("xx{b}xx", (long)123));
    EXPECT_EQ(ST_LITERAL("xx-1111011xx"), ST::format("xx{b}xx", (long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", (unsigned long)0));
    EXPECT_EQ(ST_LITERAL("xx1111011xx"), ST::format("xx{b}xx", (unsigned long)123));

    // 64-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", (long long)0));
    EXPECT_EQ(ST_LITERAL("xx1111011xx"), ST::format("xx{b}xx", (long long)123));
    EXPECT_EQ(ST_LITERAL("xx-1111011xx"), ST::format("xx{b}xx", (long long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{b}xx", (unsigned long long)0));
    EXPECT_EQ(ST_LITERAL("xx1111011xx"), ST::format("xx{b}xx", (unsigned long long)123));
#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("xx111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{b}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{+b}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-1000000000000000000000000000000000000000000000000000000000000000xx"),
              ST::format("xx{b}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-1000000000000000000000000000000000000000000000000000000000000000xx"),
              ST::format("xx{+b}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx1111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{b}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, binary_prefix)
{
    EXPECT_EQ(ST_LITERAL("xx0b10011010010xx"), ST::format("xx{#b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0b10011010010xx"), ST::format("xx{#b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0b10011010010xx"), ST::format("xx{#2b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0b10011010010xx"), ST::format("xx{>#2b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0b10011010010xx"), ST::format("xx{<#2b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   0b10011010010xx"), ST::format("xx{#16b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx   0b10011010010xx"), ST::format("xx{>#16b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0b10011010010   xx"), ST::format("xx{<#16b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  +0b10011010010xx"), ST::format("xx{#+16b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx  -0b10011010010xx"), ST::format("xx{#16b}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0b10011010010  xx"), ST::format("xx{<#+16b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0b10011010010  xx"), ST::format("xx{<#16b}xx", -1234));

    // Numeric padding
    EXPECT_EQ(ST_LITERAL("xx0b00010011010010xx"), ST::format("xx{#016b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx0b00010011010010xx"), ST::format("xx{<#016b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0b0010011010010xx"), ST::format("xx{#016b}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0b0010011010010xx"), ST::format("xx{#+016b}xx", 1234));
    EXPECT_EQ(ST_LITERAL("xx-0b0010011010010xx"), ST::format("xx{<#016b}xx", -1234));
    EXPECT_EQ(ST_LITERAL("xx+0b0010011010010xx"), ST::format("xx{<#+016b}xx", 1234));

    // Character types
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#b}xx", '\0'));
    EXPECT_EQ(ST_LITERAL("xx0b1000001xx"), ST::format("xx{#b}xx", 'A'));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#b}xx", L'\0'));
    EXPECT_EQ(ST_LITERAL("xx0b1000001xx"), ST::format("xx{#b}xx", L'A'));
    EXPECT_EQ(ST_LITERAL("xx0b111111111111111xx"), ST::format("xx{#b}xx", L'\u7fff'));

    // 32-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#b}xx", (int)0));
    EXPECT_EQ(ST_LITERAL("xx0b1111011xx"), ST::format("xx{#b}xx", (int)123));
    EXPECT_EQ(ST_LITERAL("xx-0b1111011xx"), ST::format("xx{#b}xx", (int)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#b}xx", (unsigned int)0));
    EXPECT_EQ(ST_LITERAL("xx0b1111011xx"), ST::format("xx{#b}xx", (unsigned int)123));
    EXPECT_EQ(ST_LITERAL("xx0b1111111111111111111111111111111xx"),
              ST::format("xx{#b}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+0b1111111111111111111111111111111xx"),
              ST::format("xx{+#b}xx", std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-0b10000000000000000000000000000000xx"),
              ST::format("xx{#b}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-0b10000000000000000000000000000000xx"),
              ST::format("xx{+#b}xx", std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx0b11111111111111111111111111111111xx"),
              ST::format("xx{#b}xx", std::numeric_limits<uint32_t>::max()));

    // 64-bit ints
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#b}xx", (long long)0));
    EXPECT_EQ(ST_LITERAL("xx0b1111011xx"), ST::format("xx{#b}xx", (long long)123));
    EXPECT_EQ(ST_LITERAL("xx-0b1111011xx"), ST::format("xx{#b}xx", (long long)-123));
    EXPECT_EQ(ST_LITERAL("xx0xx"), ST::format("xx{#b}xx", (unsigned long long)0));
    EXPECT_EQ(ST_LITERAL("xx0b1111011xx"), ST::format("xx{#b}xx", (unsigned long long)123));
#ifdef ST_HAVE_INT64
    EXPECT_EQ(ST_LITERAL("xx0b111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{#b}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx+0b111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{+#b}xx", std::numeric_limits<int64_t>::max()));
    EXPECT_EQ(ST_LITERAL("xx-0b1000000000000000000000000000000000000000000000000000000000000000xx"),
              ST::format("xx{#b}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx-0b1000000000000000000000000000000000000000000000000000000000000000xx"),
              ST::format("xx{+#b}xx", std::numeric_limits<int64_t>::min()));
    EXPECT_EQ(ST_LITERAL("xx0b1111111111111111111111111111111111111111111111111111111111111111xx"),
              ST::format("xx{#b}xx", std::numeric_limits<uint64_t>::max()));
#endif
}

TEST(format, floating_point)
{
    // The actual formatting is handled by libc, so we just need to test
    // that the flags get passed along properly.

    EXPECT_EQ(ST_LITERAL("xx1.5xx"), ST::format("xx{}xx", 1.5));
    EXPECT_EQ(ST_LITERAL("xx+1.5xx"), ST::format("xx{+}xx", 1.5));
    EXPECT_EQ(ST_LITERAL("xx-1.5xx"), ST::format("xx{}xx", -1.5));
    EXPECT_EQ(ST_LITERAL("xx-1.5xx"), ST::format("xx{+}xx", -1.5));

    // Padding
    EXPECT_EQ(ST_LITERAL("xx  1.50xx"), ST::format("xx{6.2f}xx", 1.5));
    EXPECT_EQ(ST_LITERAL("xx -1.50xx"), ST::format("xx{6.2f}xx", -1.5));
    EXPECT_EQ(ST_LITERAL("xx1.50  xx"), ST::format("xx{<6.2f}xx", 1.5));
    EXPECT_EQ(ST_LITERAL("xx-1.50 xx"), ST::format("xx{<6.2f}xx", -1.5));

    // Fixed notation
    EXPECT_EQ(ST_LITERAL("xx3.14xx"), ST::format("xx{.2f}xx", 3.14159));
    EXPECT_EQ(ST_LITERAL("xx3.141590xx"), ST::format("xx{.6f}xx", 3.14159));
    EXPECT_EQ(ST_LITERAL("xx16384.00xx"), ST::format("xx{.2f}xx", 16384.0));
    EXPECT_EQ(ST_LITERAL("xx0.02xx"), ST::format("xx{.2f}xx", 0.0234));

#if defined(__MINGW32__) && !defined(_UCRT)
    // MSVC uses 3 digits for the exponent by default, up to VC 2013.
    // We don't support MSVC versions older than 2015, but MinGW still uses
    // the old format in non-UCRT targets, presumably for MSVC compatibility.
    _set_output_format(_TWO_DIGIT_EXPONENT);
#endif

    // Scientific notation
    EXPECT_EQ(ST_LITERAL("xx3.14e+00xx"), ST::format("xx{.2e}xx", 3.14159));
    EXPECT_EQ(ST_LITERAL("xx3.141590e+00xx"), ST::format("xx{.6e}xx", 3.14159));
    EXPECT_EQ(ST_LITERAL("xx1.64e+04xx"), ST::format("xx{.2e}xx", 16384.0));
    EXPECT_EQ(ST_LITERAL("xx2.34e-02xx"), ST::format("xx{.2e}xx", 0.0234));

    // Scientific notation (upper-case E)
    EXPECT_EQ(ST_LITERAL("xx3.14E+00xx"), ST::format("xx{.2E}xx", 3.14159));
    EXPECT_EQ(ST_LITERAL("xx3.141590E+00xx"), ST::format("xx{.6E}xx", 3.14159));
    EXPECT_EQ(ST_LITERAL("xx1.64E+04xx"), ST::format("xx{.2E}xx", 16384.0));
    EXPECT_EQ(ST_LITERAL("xx2.34E-02xx"), ST::format("xx{.2E}xx", 0.0234));

    // Automatic (based on input)
    EXPECT_EQ(ST_LITERAL("xx3.14xx"), ST::format("xx{.3}xx", 3.14159));
    EXPECT_EQ(ST_LITERAL("xx3.14159xx"), ST::format("xx{.7}xx", 3.14159));
    EXPECT_EQ(ST_LITERAL("xx1.6e+04xx"), ST::format("xx{.2}xx", 16384.0));
    EXPECT_EQ(ST_LITERAL("xx0.0234xx"), ST::format("xx{.3}xx", 0.0234f));

    // Special values (Different CRTs have very different ways of representing
    // infinity and NaN textually :( )
    EXPECT_TRUE(ST::format("xx{f}xx", std::numeric_limits<float>::infinity())
                .find("inf", ST::case_insensitive) >= 2);
    EXPECT_TRUE(ST::format("xx{f}xx", std::numeric_limits<double>::infinity())
                .find("inf", ST::case_insensitive) >= 2);
    EXPECT_TRUE(ST::format("xx{f}xx", std::numeric_limits<float>::quiet_NaN())
                .find("nan", ST::case_insensitive) >= 2);
    EXPECT_TRUE(ST::format("xx{f}xx", std::numeric_limits<double>::quiet_NaN())
                .find("nan", ST::case_insensitive) >= 2);
}

TEST(format, booleans)
{
    // This basically just uses the string formatter with constant strings
    EXPECT_EQ(ST_LITERAL("xxtrue xx"), ST::format("xx{5}xx", true));
    EXPECT_EQ(ST_LITERAL("xxfalsexx"), ST::format("xx{5}xx", false));
}

TEST(format, references)
{
    EXPECT_EQ(ST_LITERAL("2, one"), ST::format("{&2}, {&1}", "one", 2));
    EXPECT_EQ(ST_LITERAL("2, 2"), ST::format("{&2}, {&2}", "one", 2, 3.5));
    EXPECT_EQ(ST_LITERAL("42|0042|0x2a"), ST::format("{&2}{&1c}{04&2}{&1c}{&2#x}", '|', 42));

    // Mixing ordered and referenced args -- references should not interfere
    // with ordered parameters
    EXPECT_EQ(ST_LITERAL("one, 2, 3.5"), ST::format("{}, {&3}, {}", "one", 3.5, 2));
    EXPECT_EQ(ST_LITERAL("one, 2, 2"), ST::format("{&3}, {&1}, {}", 2, 3.5, "one"));

    // No used actual parameters
    EXPECT_EQ(ST_LITERAL("xxxx"), ST::format("xxxx", "one", 2, 3.5));
    EXPECT_EQ(ST_LITERAL("xxxx"), ST::format("xxxx"));
}

TEST(format, udl_format)
{
    using namespace ST::literals;

    // Only need to test the UDL usage -- formatting is identical
    EXPECT_EQ(ST_LITERAL("xxxx"), "xxxx"_stfmt());
    EXPECT_EQ(ST_LITERAL("xxTESTxx"), "xx{}xx"_stfmt("TEST"));
    EXPECT_EQ(ST_LITERAL("xxTESTxx123xx"), "xx{}xx{}xx"_stfmt("TEST", 123));
}

struct TestStruct { int x; float y; };

void format_type(const ST::format_spec &format, ST::format_writer &output,
                 const TestStruct &value)
{
    output.append("TestStruct{");
    ST::format_type(format, output, value.x);
    output.append_char(',');
    ST::format_type(format, output, value.y);
    output.append_char('}');
}

TEST(format, custom_formatter)
{
    EXPECT_EQ(ST_LITERAL("xxTestStruct{3,1.5}xx"),
              ST::format("xx{}xx", TestStruct{3, 1.5}));
}
