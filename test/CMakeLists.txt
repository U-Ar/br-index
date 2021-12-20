SET(bri_tests "")

FILE(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp)

INCLUDE_DIRECTORIES(
	${SDSL_INCLUDE}
	${PROJECT_SOURCE_DIR}/external/googletest/googletest/include
)

LINK_DIRECTORIES(${SDSL_LIB})

LINK_LIBRARIES(sdsl divsufsort divsufsort64 gtest gtest_main)

SET(tmp_dir ${CMAKE_CURRENT_LIST_DIR}/tmp)

FOREACH(test_source ${test_sources})
	GET_FILENAME_COMPONENT(test_name_we ${test_source} NAME_WE)
	
	STRING(REGEX REPLACE "_" "-" test_name_hy ${test_name_we})

	ADD_EXECUTABLE(${test_name_we} EXCLUDE_FROM_ALL ${test_name_we}.cpp)

	ADD_CUSTOM_TARGET(${test_name_hy}
		COMMAND $<TARGET_FILE:${test_name_we}> ${tmp_dir}
		DEPENDS test/${test_name_we} ${test_name_we}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "Execute ${test_name_hy}."
	)
	LIST(APPEND bri_tests ${test_name_hy})
ENDFOREACH(test_source)

ADD_CUSTOM_TARGET(test-bri DEPENDS ${bri_tests})