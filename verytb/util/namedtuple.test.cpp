// Direct include
#include "util/namedtuple.h"
// C system headers
// C++ standard library headers
#include <array>
#include <string>
// Other libraries' .h files.
#include "gtest/gtest.h"
// Your project's .h files.

struct TestStruct {
	int a;
	double b;
	char c;
	float d;
	MAKE_NAMEDTUPLE((a)(b)(c)(d))
};

TEST(namedtuple, Getter) {
	// ensure get<0..3>() works correctly
	// we check the pointer directly
	TestStruct ts;
	EXPECT_EQ(&ts.nt_get<0>(), &ts.a);
	EXPECT_EQ(&ts.nt_get<1>(), &ts.b);
	EXPECT_EQ(&ts.nt_get<2>(), &ts.c);
	EXPECT_EQ(&ts.nt_get<3>(), &ts.d);
	// also test std::integral_constant version
	EXPECT_EQ(&ts.nt_get(std::integral_constant<int, 0>{}), &ts.a);
	EXPECT_EQ(&ts.nt_get(std::integral_constant<int, 1>{}), &ts.b);
	EXPECT_EQ(&ts.nt_get(std::integral_constant<int, 2>{}), &ts.c);
	EXPECT_EQ(&ts.nt_get(std::integral_constant<int, 3>{}), &ts.d);
}

TEST(namedtuple, names) {
	const std::array<std::string, 4>& names = TestStruct::nt_names;
	EXPECT_EQ(names[0], "a");
	EXPECT_EQ(names[1], "b");
	EXPECT_EQ(names[2], "c");
	EXPECT_EQ(names[3], "d");
}
