// direct include
#include "model/core/Module.h"
// C system headers
// C++ standard library headers
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
// Other libraries' .h files.
#include <spdlog/spdlog.h>
// Your project's .h files.

using namespace std;

namespace verytb::model {

namespace detail {

struct ModuleBase_impl {
	template<typename T> friend class Module;
	string basename_;
	ModuleBase* parent_ = nullptr;
	vector<ModuleBase*> children_;
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
		SPDLOG_CRITICAL("{} is already Construct()-ed", HierarchicalName());
	}
}

void ModuleBase::BeginInitModule(ModuleBase* m, const string_view basename) {
	m->CheckDoubleConstruct();
	if (auto &n = m->impl_->basename_; n.empty()) {
		n = basename;
	}
	module_list_.push_back(m);
	build_stack_.push_back(m);
}

void ModuleBase::EndInitModule() {
	ModuleBase* parent = build_stack_.back();
	unordered_map<string, vector<ModuleBase*>> basename_to_modules;
	parent->impl_->initialized_ = true;
	// Initialize module if they are not un-initialized
	// Count the number of names appeared as well
	for (ModuleBase* child : parent->impl_->children_) {
		if (not child->impl_->initialized_) {
			child->DefaultConstructIfPossible();
		}
		basename_to_modules[child->impl_->basename_].push_back(child);
	}
	// Assign a unique index to children
	for (auto &[basename, children] : basename_to_modules) {
		if (children.size() == 1) {
			continue;
		}
		for (unsigned i = 0; i < children.size(); ++i) {
			children[i]->impl_->index_ = i;
		}
	}
	build_stack_.pop_back();
}

void ModuleBase::HierarchicalName(string &s) const {
	if (impl_->parent_ != nullptr) {
		impl_->parent_->HierarchicalName(s);
		s += '.';
	}
	s += Name();
}

string ModuleBase::HierarchicalName() const {
	string name;
	name.reserve(32);
	HierarchicalName(name);
	return name;
}

void ModuleBase::BaseName(const string_view basename) {
	CheckDoubleConstruct();
	impl_->basename_ = basename;
}

const string& ModuleBase::BaseName() const {
	return impl_->basename_;
}

string ModuleBase::Name() const {
	auto name = impl_->basename_;
	if (impl_->index_ != kNotIndexed) {
		name += '[';
		name += to_string(impl_->index_);
		name += ']';
	}
	return name;
}

ModuleBase::ModuleBase() : impl_(new detail::ModuleBase_impl) {}
ModuleBase::~ModuleBase() {}

} // namespace verytb::model
