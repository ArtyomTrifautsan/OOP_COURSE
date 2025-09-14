cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME serialize_tests)
project(${PROJECT_NAME})

ADD_SUBDIRECTORY(googletest)
enable_testing()

set(PROJECT_SOURCES
    src/test_simple_types.cpp
    src/test_string.cpp
    src/test_vector.cpp
    src/test_associative_containers.cpp
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME} gtest gmock pthread)
target_link_libraries(${PROJECT_NAME} serialize_lib)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
