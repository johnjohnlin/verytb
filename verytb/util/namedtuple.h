#pragma once
// Direct include
// C system headers
// C++ standard library headers
#include <array>
#include <string>
#include <type_traits>
// Other libraries' .h files.
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
// Your project's .h files.

// Helper macro to generate overloaded getter methods
#define _MAKE_NAMEDTUPLE_GETTER(r, data, i, elem) \
	auto& nt_get(std::integral_constant<int, i>) { return elem; } \
	auto& nt_get(std::integral_constant<int, i>) const { return elem; }

// Helper macro to generate name string
#define _MAKE_NAMEDTUPLE_NAME(r, data, i, elem) \
	BOOST_PP_STRINGIZE(elem),

// Helper macro to generate all getter methods
#define _MAKE_NAMEDTUPLE_GETTERS(seq) \
	BOOST_PP_SEQ_FOR_EACH_I(_MAKE_NAMEDTUPLE_GETTER, _, seq) \
	template<int N> auto& nt_get() { return nt_get(std::integral_constant<int, N>{}); } \
	template<int N> auto& nt_get() const { return nt_get(std::integral_constant<int, N>{}); }

// Helper macro to generate names array
#define _MAKE_NAMEDTUPLE_NAMES(seq) \
	inline static const std::array<std::string, BOOST_PP_SEQ_SIZE(seq)> nt_names = { \
		BOOST_PP_SEQ_FOR_EACH_I(_MAKE_NAMEDTUPLE_NAME, _, seq) \
	};

// Main macro to generate all getter methods and names
#define MAKE_NAMEDTUPLE(seq) \
	_MAKE_NAMEDTUPLE_GETTERS(seq) \
	_MAKE_NAMEDTUPLE_NAMES(seq)

// Variadic wrapper that converts arguments to sequence format
#define MAKE_NAMEDTUPLE_T(...) \
	MAKE_NAMEDTUPLE(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
