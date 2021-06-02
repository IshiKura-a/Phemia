cmake_minimum_required(VERSION 3.19.0)
project(Phemia)

set (CMAKE_CXX_STANDARD 14)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
find_package(LLVM REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")


bison_target(parser "${CMAKE_CURRENT_SOURCE_DIR}/semantic/parser.y" "${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp")
flex_target(lexer "${CMAKE_CURRENT_SOURCE_DIR}/lexical/lexer.l" "${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp")

ADD_FLEX_BISON_DEPENDENCY(lexer parser)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/lexical)
include_directories(${PROJECT_SOURCE_DIR}/semantic)
include_directories(${PROJECT_SOURCE_DIR}/llvm)
include_directories(${PROJECT_SOURCE_DIR}/util)

add_executable(Phemia ${BISON_parser_OUTPUTS} ${FLEX_lexer_OUTPUTS} main.cpp)

llvm_map_components_to_libnames(llvm_libs analysis core executionengine instcombine object orcjit runtimedyld scalaropts support native irreader mcjit)

target_link_libraries(Phemia ${llvm_libs})
