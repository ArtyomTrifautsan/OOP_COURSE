cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME trie)
project(${PROJECT_NAME})

ADD_SUBDIRECTORY(googletest)
enable_testing()

set(PROJECT_INCLUDES
    trie/trie.hpp
)

set(PROJECT_SOURCES
    tests/test_trie_vertex.cpp
)

add_executable(${PROJECT_NAME}
    tests/main.cpp
    ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME} gtest)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)