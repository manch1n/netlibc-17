cmake_minimum_required(VERSION 3.10)

project(test)

aux_source_directory(./ SRCS)

include_directories(./)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-g -Wall -Werror -Wno-error=sign-compare)

add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} -lpthread)
