cmake_minimum_required(VERSION 3.9)
project(golang-lexer)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(fmt)

set(SRC_LIST 
    src/Lexer.cpp
)

add_executable(lexer main.cpp ${SRC_LIST})

target_include_directories(lexer PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(lexer PRIVATE fmt::fmt)