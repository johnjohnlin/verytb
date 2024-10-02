// direct include
#include "model/core/Module.h"
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

namespace verytb::model {

namespace detail {

struct ModuleBase_impl {
	template<typename T> friend class Module;
	std::string basename_;
	ModuleBase* parent_ = nullptr;
	std::vector<ModuleBase*> children_;
	unsigned index_ = ModuleBase::kNotIndexed;
	bool initialized_ = false;
};

} // namespace detail

void ModuleBase::AppendChild(ModuleBase* child) {
	if (build_stack_.empty()) {
		return;
	}
	ModuleBase* parent = build_stack_.back();
	parent->impl_->children_.push_back(child);
	child->impl_->parent_ = parent;
}

void ModuleBase::CheckDoubleConstruct() const {
	if (impl_->initialized_) {
		spdlog::critical("{} is already Construct()-ed", HierarchicalName());
	}
}

void ModuleBase::BeginInitModule(ModuleBase* m, const std::string_view basename) {
	m->CheckDoubleConstruct();
	m->impl_->basename_ = basename;
	module_list_.push_back(m);
	build_stack_.push_back(m);
}

void ModuleBase::EndInitModule() {
	ModuleBase* parent = build_stack_.back();
	for (ModuleBase* child : parent->impl_->children_) {
		if (child->impl_->initialized_) {
			child->DefaultConstructIfPossible();
		}
	}
	build_stack_.pop_back();
}

void ModuleBase::HierarchicalName(std::string &s) const {
	if (impl_->parent_ != nullptr) {
		HierarchicalName(s);
		s += '.';
	}
	s += Name();
}

std::string ModuleBase::HierarchicalName() const {
	std::string name;
	name.reserve(32);
	HierarchicalName(name);
	return name;
}

void ModuleBase::BaseName(const std::string_view basename) {
	CheckDoubleConstruct();
	impl_->basename_ = basename;
}

const std::string& ModuleBase::BaseName() const {
	return impl_->basename_;
}

std::string ModuleBase::Name() const {
	auto name = impl_->basename_;
	if (impl_->index_ != kNotIndexed) {
		name += '[';
		name += std::to_string(impl_->index_);
		name += ']';
	}
	return name;
}

ModuleBase::ModuleBase() {}
ModuleBase::~ModuleBase() {}

} // namespace verytb::model
