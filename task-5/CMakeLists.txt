cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME crypto)

ADD_SUBDIRECTORY(googletest)
enable_testing()

set(PROJECT_INCLUDES
    src/cryptography.hpp
    src/serialize_sfinae.hpp
    src/trie.hpp
)

set(PROJECT_SOURCES
    src/cryptography_test.cpp
    src/main.cpp
)

add_executable(${PROJECT_NAME}
    # src/main.cpp
    ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME} gtest)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)