CMAKE_MINIMUM_REQUIRED(VERSION 3.11)

PROJECT(br-index CXX)

FIND_PACKAGE(Git QUIET)

SET(SDSL_INCLUDE "~/sdsl/include") #SDSL headeres
SET(SDSL_LIB "~/sdsl/lib") #SDSL lib

INCLUDE_DIRECTORIES(${SDSL_INCLUDE}) 
LINK_DIRECTORIES(${SDSL_LIB}) 

SET(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-g -ggdb -Ofast -fstrict-aliasing -DNDEBUG -march=native")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -ggdb -Ofast -fstrict-aliasing -march=native")

ADD_SUBDIRECTORY(test)
#ADD_SUBDIRECTORY(external)
INCLUDE_DIRECTORIES(src)

ADD_EXECUTABLE(bri-build src/bri-build.cpp)
TARGET_LINK_LIBRARIES(bri-build sdsl)
TARGET_LINK_LIBRARIES(bri-build divsufsort)
TARGET_LINK_LIBRARIES(bri-build divsufsort64)

ADD_EXECUTABLE(bri-count src/bri-count.cpp)
TARGET_LINK_LIBRARIES(bri-count sdsl)
TARGET_LINK_LIBRARIES(bri-count divsufsort)
TARGET_LINK_LIBRARIES(bri-count divsufsort64)

ADD_EXECUTABLE(bri-locate src/bri-locate.cpp)
TARGET_LINK_LIBRARIES(bri-locate sdsl)
TARGET_LINK_LIBRARIES(bri-locate divsufsort)
TARGET_LINK_LIBRARIES(bri-locate divsufsort64)

ADD_EXECUTABLE(bri-space src/bri-space.cpp)
TARGET_LINK_LIBRARIES(bri-space sdsl)
TARGET_LINK_LIBRARIES(bri-space divsufsort)
TARGET_LINK_LIBRARIES(bri-space divsufsort64)


enable_testing()

add_executable(run_tests test/test_all.cpp 
	test/sparse_sd_vector_test.cpp 
	test/my_test.cpp
)
target_link_libraries(run_tests PRIVATE sdsl divsufsort divsufsort64)

target_include_directories(run_tests PRIVATE ${PROJECT_SOURCE_DIR}/external/iutest/include)
