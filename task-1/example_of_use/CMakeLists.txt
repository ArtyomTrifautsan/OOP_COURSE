cmake_minimum_required(VERSION 3.12)

set(SFINAE_E example_sfinae)
set(CONCEPTS_E example_concepts)

set(PROJECT_SOURCES
    src/main.cpp
    src/serialize.hpp
)

add_executable(${SFINAE_E}
    ${PROJECT_SOURCES}
    src/serialize_sfinae.hpp
)
set_target_properties(${SFINAE_E} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
target_compile_features(${SFINAE_E} PUBLIC cxx_std_23)

add_executable(${CONCEPTS_E}
    ${PROJECT_SOURCES}
    src/serialize_concepts.hpp
)
target_compile_definitions(${CONCEPTS_E} PRIVATE USE_CONCEPTS)
set_target_properties(${CONCEPTS_E} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
target_compile_features(${CONCEPTS_E} PUBLIC cxx_std_23)
