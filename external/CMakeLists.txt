add_subdirectory(googletest)

add_executable(mytest ../src/mytest.cpp)

target_include_directories(
	mytest PUBLIC
	${PROJECT_SOURCE_DIR}/external/googletest/googletest/include
)

target_link_libraries(
	mytest
	gtest_main
)

add_test(NAME mytest COMMAND mytest)