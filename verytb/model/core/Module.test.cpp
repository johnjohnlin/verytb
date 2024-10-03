// direct include
#include "model/core/Module.h"
// C system headers
// C++ standard library headers
#include <memory>
#include <string_view>
// Other libraries' .h files.
#include <gtest/gtest.h>
// Your project's .h files.

using verytb::model::Module;
using namespace std;

class Module1 {
	IS_VERYTB_MODULE
protected:
	Module1() {}
public:
	static constexpr std::string_view kDefaultName = "u_m1";
};

class Module2 {
	IS_VERYTB_MODULE
protected:
	Module2(unsigned n) {
		m1.reset(new Module<Module1>[n]);
		// prepare a special name
		m1[n-1].BaseName("u_last_m1");
	}
public:
	unique_ptr<Module<Module1>[]> m1;
	static constexpr std::string_view kDefaultName = "u_m2";
};

class Module3 {
	IS_VERYTB_MODULE
protected:
	Module3() {
		m2.Construct(4);
	}
public:
	Module<Module1> m1;
	Module<Module2> m2;
};

TEST(Module, StaticBehavior) {
	static_assert(Module<Module1>::kDefaultName == "u_m1");
	static_assert(Module<Module2>::kDefaultName == "u_m2");
	static_assert(Module<Module3>::kDefaultName == "u_module");
};

TEST(Module, Names) {
	spdlog::set_level(spdlog::level::debug);
	spdlog::set_pattern("[%L+%o %s:%#] %v");

	Module<Module3> m3;
	m3.Construct();

	EXPECT_EQ(m3.BaseName(), "u_module");
	EXPECT_EQ(m3->m1.BaseName(), "u_m1");
	EXPECT_EQ(m3->m2.BaseName(), "u_m2");
	EXPECT_EQ(m3->m2->m1[0].BaseName(), "u_m1");
	EXPECT_EQ(m3->m2->m1[1].BaseName(), "u_m1");
	EXPECT_EQ(m3->m2->m1[2].BaseName(), "u_m1");
	EXPECT_EQ(m3->m2->m1[3].BaseName(), "u_last_m1");

	EXPECT_EQ(m3.Name(), "u_module");
	EXPECT_EQ(m3->m1.Name(), "u_m1");
	EXPECT_EQ(m3->m2.Name(), "u_m2");
	EXPECT_EQ(m3->m2->m1[0].Name(), "u_m1[0]");
	EXPECT_EQ(m3->m2->m1[1].Name(), "u_m1[1]");
	EXPECT_EQ(m3->m2->m1[2].Name(), "u_m1[2]");
	EXPECT_EQ(m3->m2->m1[3].Name(), "u_last_m1");

	EXPECT_EQ(m3.HierarchicalName(), "u_module");
	EXPECT_EQ(m3->m1.HierarchicalName(), "u_module.u_m1");
	EXPECT_EQ(m3->m2.HierarchicalName(), "u_module.u_m2");
	EXPECT_EQ(m3->m2->m1[0].HierarchicalName(), "u_module.u_m2.u_m1[0]");
	EXPECT_EQ(m3->m2->m1[1].HierarchicalName(), "u_module.u_m2.u_m1[1]");
	EXPECT_EQ(m3->m2->m1[2].HierarchicalName(), "u_module.u_m2.u_m1[2]");
	EXPECT_EQ(m3->m2->m1[3].HierarchicalName(), "u_module.u_m2.u_last_m1");
};
