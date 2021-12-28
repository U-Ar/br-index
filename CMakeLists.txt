CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(br-index CXX)

FIND_PACKAGE(Git QUIET)
FIND_PACKAGE(Threads REQUIRED)

SET(SDSL_INCLUDE "~/sdsl/include") #SDSL headeres
SET(SDSL_LIB "~/sdsl/lib") #SDSL lib

INCLUDE_DIRECTORIES(${SDSL_INCLUDE}) 
LINK_DIRECTORIES(${SDSL_LIB}) 

SET(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS "-std=c++11")

SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-g -ggdb -Ofast -fstrict-aliasing -DNDEBUG -march=native")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -ggdb -Ofast -fstrict-aliasing -march=native")

# Git is installed and .git exists in the top directory
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    enable_testing()
    option(GOOGLETEST "Check Google Test during build" ON)
    # update submodule if option "-DGOOGLETEST=on" is designated
    if(GOOGLETEST)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/googletest/CMakeLists.txt")
message(FATAL_ERROR "The submodules were not downloaded! GOOGLETEST was turned off or failed. Please update submodules and try again.")
endif()

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

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(external)
ADD_SUBDIRECTORY(test)