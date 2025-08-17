// Direct include
#include "verilog/dtype/venum.h"
// C system headers
// C++ standard library headers
#include <vector>
// Other libraries' .h files.
#include <gtest/gtest.h>
// Your project's .h files.

VENUM_T(MyEnum, unsigned) {
	enum etype : unsigned {
		eValue1 = 1,
		eValue2 = 2,
		eValue2_2 = 2,
		eValue2_3 = 2,
		eValue3 = 3,
		eValue3_2 = 3,
	};
	MAKE_VENUM_CTOR(MyEnum)
	MAKE_VENUM(
		(eValue1)
		(eValue2)
		(eValue2_2)
		(eValue2_3)
		(eValue3)
		(eValue3_2)
	)
};

TEST(venum, Names) {
	const std::array<std::string, 6>& names = MyEnum::nt_names;
	EXPECT_EQ(names[0], "eValue1");
	EXPECT_EQ(names[1], "eValue2");
	EXPECT_EQ(names[2], "eValue2_2");
	EXPECT_EQ(names[3], "eValue2_3");
	EXPECT_EQ(names[4], "eValue3");
	EXPECT_EQ(names[5], "eValue3_2");
}

TEST(venum, Implementation) {
	MyEnum e1 = MyEnum::eValue1;
	static_assert(std::is_same_v<decltype(e1.value), unsigned>);
	EXPECT_EQ(e1.value, 1u);
}

TEST(venum, ToString) {
	MyEnum e;
	e = MyEnum::eValue1;
	EXPECT_EQ(e.to_string(), "eValue1");
	e = MyEnum::eValue2_2;
	EXPECT_EQ(e.to_string(), "eValue2/...");
	e = MyEnum::eValue3_2;
	EXPECT_EQ(e.to_string(), "eValue3/eValue3_2");
	// directly assign an non-enum value is not allowed
	// but we can assign an invalid value through .value
	e.value = 0;
	EXPECT_EQ(e.to_string(), "INVALID_0");
}

// Add a second enum type to test for interference
VENUM_T(AnotherEnum, unsigned) {
	enum etype : unsigned {
		eFirst = 1,
		eSecond = 2,
	};
	MAKE_VENUM_CTOR(AnotherEnum)
	MAKE_VENUM(
		(eFirst)
		(eSecond)
	)
};

TEST(venum, NoInterferenceBetweenEnumTypes) {
	// Test that different enum types with same underlying type don't interfere
	MyEnum e1 = MyEnum::eValue1;
	AnotherEnum e2 = AnotherEnum::eFirst;

	// Both have value 1, but should have different string representations
	EXPECT_EQ(e1.value, 1);
	EXPECT_EQ(e2.value, 1);
	EXPECT_EQ(e1.to_string(), "eValue1");
	EXPECT_EQ(e2.to_string(), "eFirst");

	// Test with value 2 as well
	MyEnum e3 = MyEnum::eValue2;
	AnotherEnum e4 = AnotherEnum::eSecond;

	EXPECT_EQ(e3.value, 2);
	EXPECT_EQ(e4.value, 2);
	EXPECT_EQ(e3.to_string(), "eValue2/...");
	EXPECT_EQ(e4.to_string(), "eSecond");
}
