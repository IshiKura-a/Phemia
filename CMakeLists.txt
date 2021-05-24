cmake_minimum_required(VERSION 3.20.2)
project(Phemia)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

bison_target(parser "${CMAKE_CURRENT_SOURCE_DIR}/semantic/parser.y" "${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp")
flex_target(lexer "${CMAKE_CURRENT_SOURCE_DIR}/lexical/lexer.l" "${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp")

ADD_FLEX_BISON_DEPENDENCY(lexer parser)
include_directories(${PROJECT_SOURCE_DIR}/lexical)
include_directories(${PROJECT_SOURCE_DIR}/semantic)

add_executable(Phemia ${BISON_parser_OUTPUTS} ${FLEX_lexer_OUTPUTS})