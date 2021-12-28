INCLUDE(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG 703bd9caab50b139428cea1aaff9974ebee5742e
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
	sparse_sd_vector_test
	sparse_sd_vector_test.cpp
)
target_link_libraries(
	sparse_sd_vector_test
	gtest_main
)

add_executable(
	my_test
	my_test.cpp
)
target_link_libraries(
	my_test
	gtest_main
)


include(GoogleTest)
gtest_discover_tests(sparse_sd_vector_test)
gtest_discover_tests(my_test)


#SET(bri_tests "")

#FILE(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp)

#INCLUDE_DIRECTORIES(
#	${SDSL_INCLUDE}
#	${PROJECT_SOURCE_DIR}/external/googletest/googletest/include
#)

#LINK_DIRECTORIES(${SDSL_LIB})

#LINK_LIBRARIES(sdsl divsufsort divsufsort64 gtest gtest_main)

#SET(tmp_dir ${CMAKE_CURRENT_LIST_DIR}/tmp)

#FOREACH(test_source ${test_sources})
#	GET_FILENAME_COMPONENT(test_name_we ${test_source} NAME_WE)
	
#	STRING(REGEX REPLACE "_" "-" test_name_hy ${test_name_we})

#	ADD_EXECUTABLE(${test_name_we} EXCLUDE_FROM_ALL ${test_name_we}.cpp)

#	ADD_CUSTOM_TARGET(${test_name_hy}
#		COMMAND $<TARGET_FILE:${test_name_we}> ${tmp_dir}
#		DEPENDS test/${test_name_we} ${test_name_we}
#		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#	)
#	LIST(APPEND bri_tests ${test_name_hy})
#ENDFOREACH(test_source)

#ADD_CUSTOM_TARGET(test-bri DEPENDS ${bri_tests})