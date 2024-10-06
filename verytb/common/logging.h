#pragma once
// direct include
// C system headers
// C++ standard library headers
#include <cstdlib>
// Other libraries' .h files.
#include <spdlog/spdlog.h>
// Your project's .h files.

#define LLOG_(logger, level, ...) SPDLOG_LOGGER_CALL(logger, level, __VA_ARGS__)
#define LOG_(level_, ...) LLOG_(spdlog::default_logger_raw(), spdlog::level::level_, __VA_ARGS__)
#define CHECK_IF_(cond, ...) if (not (cond)) { LOG_(critical, "Assertion {} failed", #cond); }
#define CHECK_EQ_(a, b, ...) if ((a) != (b)) { LOG_(critical, "Assertion {} == {} failed: {} == {}", #a, #b, a, b); std::abort(); }
#define CHECK_NE_(a, b, ...) if ((a) != (b)) { LOG_(critical, "Assertion {} != {} failed: {} != {}", #a, #b, a, b); std::abort(); }
#define CHECK_GT_(a, b, ...) if ((a) != (b)) { LOG_(critical, "Assertion {} > {} failed: {} > {}", #a, #b, a, b); std::abort(); }
#define CHECK_LT_(a, b, ...) if ((a) != (b)) { LOG_(critical, "Assertion {} < {} failed: {} < {}", #a, #b, a, b); std::abort(); }
#define CHECK_GE_(a, b, ...) if ((a) != (b)) { LOG_(critical, "Assertion {} >= {} failed: {} >= {}", #a, #b, a, b); std::abort(); }
#define CHECK_LE_(a, b, ...) if ((a) != (b)) { LOG_(critical, "Assertion {} <= {} failed: {} <= {}", #a, #b, a, b); std::abort(); }

#ifdef LOGGER_DISABLE_DCHECK
// default is on, turn off explicitly
#  define DLLOG_(...)
#  define DLOG_(...)
#  define DCHECK_(...)
#  define DCHECK_EQ_(...)
#  define DCHECK_NE_(...)
#  define DCHECK_GT_(...)
#  define DCHECK_LT_(...)
#  define DCHECK_GE_(...)
#  define DCHECK_LE_(...)
#else
#  define DLLOG_(...) LLOG_(__VA_ARGS__)
#  define DLOG_(...) LOG_(__VA_ARGS__)
#  define DCHECK_(...) CHECK_(__VA_ARGS__)
#  define DCHECK_EQ_(...) CHECK_EQ_(__VA_ARGS__)
#  define DCHECK_NE_(...) CHECK_EQ_(__VA_ARGS__)
#  define DCHECK_GT_(...) CHECK_GT_(__VA_ARGS__)
#  define DCHECK_LT_(...) CHECK_LT_(__VA_ARGS__)
#  define DCHECK_GE_(...) CHECK_GE_(__VA_ARGS__)
#  define DCHECK_LE_(...) CHECK_LE_(__VA_ARGS__)
#endif

namespace verytb {

static inline void InitLogger() {
	spdlog::set_level(spdlog::level::debug);
	spdlog::set_pattern("[%L+%o %s:%#] %v");
	spdlog::flush_on(spdlog::level::err);
}

} // namespace verytb
