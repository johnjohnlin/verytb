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
		m1.reset(new Module<Module1>[4]);
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
