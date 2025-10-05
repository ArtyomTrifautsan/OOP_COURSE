cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME filter_iterator)
project(${PROJECT_NAME})

set(PROJECT_INCLUDES
    includes/filter_iterator.hpp
)

set(PROJECT_SOURCES
    src/main.cpp
)

set(ALL_SOURCES
    ${PROJECT_INCLUDES}
    ${PROJECT_SOURCES}
)

add_library(${PROJECT_NAME} STATIC ${ALL_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC includes)
target_include_directories(${PROJECT_NAME} PRIVATE src)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)