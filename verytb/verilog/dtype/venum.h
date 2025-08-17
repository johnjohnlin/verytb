#pragma once
// Direct include
#include "util/namedtuple.h"
#include "verilog/dtype/dtype_base.h"
// C system headers
// C++ standard library headers
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <array>
// Other libraries' .h files.
#include <boost/preprocessor/seq/enum.hpp>
// Your project's .h files.

namespace verytb::verilog::dtype::detail {

template<typename UnderlyingType>
struct venum_base {
	using itype = UnderlyingType;
	itype value;

	venum_base() = default;
	venum_base(itype v) : value(v) {}
	template<typename EnumType>
	venum_base(EnumType e) : value(static_cast<itype>(e)) {}
};

template<typename T, size_t N>
std::unordered_map<T, std::string> ConstructValueToNameMap(
	const std::array<T, N>& values,
	const std::array<std::string, N>& names
) {
	std::unordered_map<T, std::string> result;
	std::unordered_map<T, std::vector<std::string>> value_to_all_names;

	// Group all names by their values
	for (size_t i = 0; i < N; ++i) {
		value_to_all_names[values[i]].push_back(names[i]);
	}

	// Create the final mapping with alias handling
	for (const auto& [value, name_list] : value_to_all_names) {
		if (name_list.size() == 1) {
			result[value] = name_list[0];
		} else if (name_list.size() == 2) {
			result[value] = name_list[0] + "/" + name_list[1];
		} else {
			result[value] = name_list[0] + "/...";
		}
	}

	return result;
}

template<typename T>
std::string GetEnumStringRepresentation(
	T value,
	const std::unordered_map<T, std::string>& value_to_name_map
) {
	if (auto it = value_to_name_map.find(value); it != value_to_name_map.end()) {
		return it->second;
	}
	return "INVALID_" + std::to_string(value);
}

} // namespace verytb::verilog::dtype::detail

#define VENUM_T(name, ...) \
struct name : public verytb::verilog::dtype::detail::venum_base<__VA_ARGS__>

#define _MAKE_NAMEDTUPLE_VALUES(seq) \
	inline static const std::array<etype, BOOST_PP_SEQ_SIZE(seq)> venum_values = { \
		BOOST_PP_SEQ_ENUM(seq) \
	};

#define MAKE_VENUM_CTOR(cls) \
	TAG_AS_VENUM \
	cls() = default; \
	cls(const cls&) = default; \
	cls(etype v) { value = v; } \
	cls& operator=(const cls&) = default; \
	cls& operator=(etype v) { value = v; return *this;}

#define MAKE_VENUM(seq) \
	_MAKE_NAMEDTUPLE_NAMES(seq) \
	_MAKE_NAMEDTUPLE_VALUES(seq) \
	etype operator+() const { return static_cast<etype>(+value); } \
	operator etype() const { return operator+(); } \
	std::string to_string() const { \
		static auto kValueToNameMap = verytb::verilog::dtype::detail::ConstructValueToNameMap(venum_values, nt_names); \
		return verytb::verilog::dtype::detail::GetEnumStringRepresentation(operator+(), kValueToNameMap); \
	}
