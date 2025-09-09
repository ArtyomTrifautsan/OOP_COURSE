cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME example_of_use)
project(${PROJECT_NAME})

set(PROJECT_SOURCES
    src/main.cpp
)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME} serialize_lib)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
