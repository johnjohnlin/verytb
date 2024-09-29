#pragma once
// direct include
// C system headers
// C++ standard library headers
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
// Other libraries' .h files.
#include <spdlog/spdlog.h>
// Your project's .h files.

namespace verytb::vmodule {

template<typename T> class Module;

struct ModuleBase {
	const unsigned kNotIndexed = 0;
private:
	template<typename T> friend class Module;
	std::string basename_;
	ModuleBase* parent_ = nullptr;
	std::vector<ModuleBase*> children_;
	unsigned index_ = kNotIndexed;
	bool initialized_ = false;

	inline static std::vector<ModuleBase*> build_stack_;
	inline static std::vector<ModuleBase*> module_list_;

	ModuleBase() {}
	virtual ~ModuleBase() {}
	virtual void DefaultConstructIfNeeded() = 0;

	static void AppendChild(ModuleBase* child) {
		if (build_stack_.empty()) {
			return;
		}
		ModuleBase* parent = build_stack_.back();
		parent->children_.push_back(child);
		child->parent_ = parent;
	}

	static void EnterModule(ModuleBase* m) {
		module_list_.push_back(m);
		build_stack_.push_back(m);
	}

	static void LeaveModule();

	const void HierarchicalName(std::string &s) const {
		if (parent_ != nullptr) {
			HierarchicalName(s);
			s += '.';
		}
		s += Name();
	}

public:
	const std::string HierarchicalName() const {
		std::string name;
		name.reserve(32);
		HierarchicalName(name);
		return name;
	}

	void BaseName(const std::string_view basename) {
		if (not initialized_) spdlog::critical("{} is constructed twice", HierarchicalName());
		basename_ = basename;
	}

	const std::string& BaseName() const {
		auto name = basename_;
		if (index_ != kNotIndexed) {
			name += '[';
			name += std::to_string(index_);
			name += ']';
		}
		return name;
	}

	const std::string& Name() const {
		auto name = basename_;
		if (index_ != kNotIndexed) {
			name += '[';
			name += std::to_string(index_);
			name += ']';
		}
		return name;
	}

	std::string IndexedName() const {
		auto ret = name_;
		if (index_ != kNotIndexed) {
			ret = ret + "[" + std::to_string(index_) + "]";
		}
		return ret;
	}
};

template<typename T>
class Module : public virtual ModuleBase {
	// Test whether we can access T::T, we don't split this into another class,
	// since we usually require you to friend Module<T>
	// base class
	template<typename U, typename = void> struct CanAccessCtor : public std::false_type {};
	// specialized class
	template<typename U> struct CanAccessCtor<U, decltype(U(), void())> : public std::true_type {};
	static constexpr bool kCanAccessCtor = CanAccessCtor<T>::value;
	// Test whether we can access T::kDefaultName, otherwise use "u_module".
	// While we generally don't require kDefaultName protected, we put similar thing here
	// base class
	template<typename U, typename = void>
	struct GetDefaultName { constexpr static std::string_view kDefaultName = "u_module"; };
	// specialized class
	template<typename U>
	struct GetDefaultName<U, decltype(U::kDefaultName, void())> { constexpr static std::string_view kDefaultName = U::kDefaultName; };
public:
	static constexpr std::string_view kDefaultName = GetDefaultName<T>::kDefaultName;
private:

	alignas(T) char storage_[sizeof(T)];
	T& self() { return *reinterpret_cast<T*>(storage_); }

public:
	Module(const Module&) = delete;
	Module(Module&&) = delete;
	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) = delete;

	T* operator->() { }
	const T* operator->() const { }
	Module() {
		ModuleBase::AppendChild(this);
	}

	template <typename ...Args>
	void Construct(Args&&... args) {
		name_ = T::kDefaultName;
		ModuleBase::EnterModule(this);
		// CHECK(not initialized_) << "Module is " << HierarchicalName() << " already constructed";
		new (storage_) T (std::forward<Args>(args)...);
		initialized_ = true;
		ModuleBase::LeaveModule();
	}

	virtual void DefaultConstructIfNeeded() override {
		if (initialized_) {
			return;
		}
		if constexpr (kCanAccessCtor) {
			Construct();
		} else {
			spdlog::critical("{} is but", HierarchicalName());
		}
	}

	~Module() {
		// call desctuctor
		self().~T();
	}
};

inline void ModuleBase::LeaveModule() {
	ModuleBase* parent = build_stack_.back();
	for (ModuleBase* child : parent->children_) {
		child->DefaultConstructIfNeeded();
	}
	build_stack_.pop_back();
}

} // verytb::vmodule

#define IS_VERYTB_MODULE template<typename T> friend class ::verytb::vmodule::Module;
