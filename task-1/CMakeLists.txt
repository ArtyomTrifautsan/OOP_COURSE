cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME serialize_deserialize)

add_subdirectory(serialize_deserialize)
add_subdirectory(tests)
# add_subdirectory(example_of_use)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT maze)