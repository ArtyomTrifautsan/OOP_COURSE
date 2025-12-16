cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME crypto)

ADD_SUBDIRECTORY(googletest)
enable_testing()

set(PROJECT_INCLUDES
    src/block_cipher.hpp
    src/mode.hpp
    src/coder.hpp
)

set(PROJECT_SOURCES
    src/xor_cipher.cpp
    src/ecb.cpp
    src/cbc.cpp
    src/tests.cpp
    src/main.cpp
)

add_executable(${PROJECT_NAME}
    # src/main.cpp
    ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME} gtest)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)