enable_testing()

add_executable(run_tests test_all.cpp sparse_sd_vector_test.cpp my_test.cpp)
target_link_libraries(run_tests PRIVATE sdsl divsufsort divsufsort64)

target_include_directories(run_tests PRIVATE ${PROJECT_SOURCE_DIR}/external/iutest/include)

