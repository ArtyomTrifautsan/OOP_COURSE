cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME filter_iterator_project)

cmake_minimum_required(VERSION 3.12)

set(SFINAE_V tests_sfinae)
set(CONCEPTS_V tests_concepts)

ADD_SUBDIRECTORY(googletest)
enable_testing()

set(PROJECT_SOURCES
    src/filter_iterator.hpp
    src/filter_test.cpp
    src/main.cpp
)

add_executable(${SFINAE_V}
    src/filter_iterator_sfinae.hpp
    ${PROJECT_SOURCES}
)
target_link_libraries(${SFINAE_V} gtest)
set_target_properties(${SFINAE_V} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
target_compile_features(${SFINAE_V} PUBLIC cxx_std_17)

add_executable(${CONCEPTS_V}
    src/filter_iterator_concepts.hpp
    ${PROJECT_SOURCES}
)
target_compile_definitions(${CONCEPTS_V} PRIVATE USE_CONCEPTS)
target_link_libraries(${CONCEPTS_V} gtest)
set_target_properties(${CONCEPTS_V} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
target_compile_features(${CONCEPTS_V} PUBLIC cxx_std_20)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT filter_iterator)