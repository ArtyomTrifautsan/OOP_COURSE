cmake_minimum_required(VERSION 3.12)

set(PROJECT_NAME example_of_use)
project(${PROJECT_NAME})

set(PROJECT_INCLUDES
)

set(PROJECT_SOURCES
)

set(ALL_SOURCES
    ${PROJECT_INCLUDES}
    ${PROJECT_SOURCES}
)
                    
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${ALL_SOURCES}
)

target_link_libraries(${PROJECT_NAME} #==ДОПИСАТЬ НАЗВАНИЕ ЗАДАЧИ==)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
