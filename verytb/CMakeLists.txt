cmake_minimum_required (VERSION 3.16)
project(verytb)

# setup
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
find_package(spdlog REQUIRED)
find_package(GTest REQUIRED)

# find all necessary files
file(GLOB_RECURSE LIB_FILES "*.cpp")
list(FILTER LIB_FILES EXCLUDE REGEX ".*\\.mtest\\.cpp$") # test with main, shall link gtest
list(FILTER LIB_FILES EXCLUDE REGEX ".*\\.test\\.cpp$") # test without main, shall link gtest_main
list(FILTER LIB_FILES EXCLUDE REGEX ".*\\.main\\.cpp$")
file(GLOB_RECURSE MTEST_FILES "*.mtest.cpp")
file(GLOB_RECURSE TEST_FILES "*.test.cpp")
file(GLOB_RECURSE MAIN_FILES "*.main.cpp")
include_directories(${CMAKE_SOURCE_DIR})

# build the library
add_library(verytb STATIC ${LIB_FILES})
target_link_libraries(verytb PUBLIC spdlog::spdlog)
set(COMMON_LIBS verytb)

# build the tests
enable_testing()
foreach(TEST_FILE ${TEST_FILES})
	get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
	add_executable(${TEST_NAME}_test ${TEST_FILE})
	target_link_libraries(${TEST_NAME}_test GTest::gtest_main ${COMMON_LIBS})
	add_test(NAME ${TEST_NAME}_test COMMAND ${TEST_NAME}_test)
endforeach()
foreach(MTEST_FILE ${MTEST_FILES})
	get_filename_component(MTEST_NAME ${MTEST_FILE} NAME_WE)
	add_executable(${MTEST_NAME}_mtest ${MTEST_FILE})
	target_link_libraries(${MTEST_NAME}_mtest GTest::gtest ${COMMON_LIBS})
	add_test(NAME ${MTEST_NAME}_mtest COMMAND ${MTEST_NAME}_mtest)
endforeach()

# build the main
foreach(MAIN_FILE ${MAIN_FILES})
	get_filename_component(MAIN_NAME ${MAIN_FILE} NAME_WE)
	add_executable(${MAIN_NAME} ${MAIN_FILE})
	target_link_libraries(${TEST_NAME} ${COMMON_LIBS})
endforeach()
