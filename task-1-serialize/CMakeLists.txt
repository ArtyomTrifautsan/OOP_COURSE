cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME serialize)

set(SFINAE_V tests_sfinae)
set(CONCEPTS_V tests_concepts)

ADD_SUBDIRECTORY(googletest)
enable_testing()

set(PROJECT_SOURCES
    src/serialize.hpp
    src/my_vector.hpp
    src/test_simple_types.cpp
    src/test_string.cpp
    src/test_vector.cpp
    src/test_associative_containers.cpp
    src/test_my_vector.cpp
    src/test_forward_list.cpp
)

add_executable(${SFINAE_V}
    src/main.cpp
    src/serialize_sfinae.hpp
    ${PROJECT_SOURCES}
)
target_link_libraries(${SFINAE_V} gtest)
set_target_properties(${SFINAE_V} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
target_compile_features(${SFINAE_V} PUBLIC cxx_std_23)

add_executable(${CONCEPTS_V}
    src/main.cpp
    src/serialize_concepts.hpp
    ${PROJECT_SOURCES}
)
target_compile_definitions(${CONCEPTS_V} PRIVATE USE_CONCEPTS)
target_link_libraries(${CONCEPTS_V} gtest)
set_target_properties(${CONCEPTS_V} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
target_compile_features(${CONCEPTS_V} PUBLIC cxx_std_23)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT serialize)