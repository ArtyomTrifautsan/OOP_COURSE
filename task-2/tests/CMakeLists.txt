cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME filter_iterator_tests)
project(${PROJECT_NAME})

ADD_SUBDIRECTORY(googletest)
enable_testing()

set(PROJECT_INCLUDES
    src/filter_iterator.cpp
)

set(PROJECT_SOURCES
    src/iterator_test.cpp
    src/range_test.cpp
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME} gtest gmock pthread)
# target_link_libraries(${PROJECT_NAME} filter_iterator)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)