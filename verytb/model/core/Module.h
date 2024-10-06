#pragma once
// direct include
// C system headers
// C++ standard library headers
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
// Other libraries' .h files.
// Your project's .h files.
#include "common/logging.h"

namespace verytb::model {

template<typename T> class Module;

namespace detail {
	struct ModuleBase_impl;
} // namespace detail

class ModuleBase {
	template<typename T> friend class Module;
	inline static std::vector<ModuleBase*> build_stack_;
	inline static std::vector<ModuleBase*> module_list_;
	// We decide to make everything hidden and leave only one pointer,
	// by which we lose some performance but win some memory usage and compile time.
	// It is ok since these function are usually not frequently called.
	detail::ModuleBase_impl* impl_;

protected:
	ModuleBase();
	virtual ~ModuleBase();
	virtual void DefaultConstructIfPossible() = 0;
	static void AppendChild(ModuleBase* child);
	static void BeginInitModule(ModuleBase* m, const std::string_view basename);
	static void EndInitModule();

private:
	void HierarchicalName(std::string &s) const;
	void CheckDoubleConstruct() const;
	bool InitDone() const;

public:
	std::string HierarchicalName() const;
	void BaseName(const std::string_view basename);
	const std::string& BaseName() const;
	std::string Name() const;
	static constexpr unsigned kNotIndexed = -1u;
};

template<typename T>
class Module: public virtual ModuleBase {
	// Test whether we can access T::T, we don't split this into another class,
	// since we usually require you to friend Module<T> and protect constructor
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
	T& self() {
		// InitDone();
		return *reinterpret_cast<T*>(storage_);
	}

	virtual void DefaultConstructIfPossible() override {
		if constexpr (kCanAccessCtor) {
			Construct();
		} else {
			SPDLOG_CRITICAL("{} is not Construct()-ed explicitly but cannot be default initialized", HierarchicalName());
		}
	}

public:
	Module(const Module&) = delete;
	Module(Module&&) = delete;
	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) = delete;

	T* operator->() { return &self(); }
	const T* operator->() const { return &self(); }
	Module() {
		ModuleBase::AppendChild(this);
	}

	template <typename ...Args>
	void NamedConstruct(const std::string_view basename, Args&&... args) {
		ModuleBase::BeginInitModule(this, basename);
		new (storage_) T (std::forward<Args>(args)...);
		ModuleBase::EndInitModule();
	}

	template <typename ...Args>
	void Construct(Args&&... args) {
		// Just forward to NamedConstruct
		NamedConstruct(kDefaultName, std::forward<Args>(args)...);
	}

	~Module() {
		// call desctuctor
		self().~T();
	}
};

} // namespace verytb::model

#define IS_VERYTB_MODULE template<typename T> friend class ::verytb::model::Module;
