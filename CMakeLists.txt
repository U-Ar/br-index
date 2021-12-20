cmake_minimum_required(VERSION 3.0)

project(test_cmake CXX)

# download GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)

# turn off nonnecessary cache of Google Test
#set(BUILD_GMOCK CACHE BOOL OFF "" FORCE)
#set(INSTALL_GTEST CACHE BOOL OFF "" FORCE)

# prevent from overwriting settings of compiler and linker in super project (only in Windows)
if(WIN32)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

# make GoogleTest available
FetchContent_MakeAvailable(googletest)

enable_testing()

# define unit tests
add_executable(mytest mytest.cpp)
target_link_libraries(mytest gtest_main)
add_test(NAME mytest COMMAND mytest)

# register unit tests to CTest
include(GoogleTest)
gtest_discover_tests(mytest)


