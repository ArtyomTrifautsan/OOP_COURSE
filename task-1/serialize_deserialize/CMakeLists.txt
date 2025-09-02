cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME serialize_deserialize_lib)
project(${PROJECT_NAME})

set(PROJECT_INCLUDES
    includes/serialize.hpp
)

set(PROJECT_SOURCES
    src/serialize.cpp
)

set(ALL_SOURCES
    ${PROJECT_INCLUDES}
    ${PROJECT_SOURCES}
)

add_library(${PROJECT_NAME} STATIC ${ALL_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC includes)
target_include_directories(${PROJECT_NAME} PRIVATE src)
