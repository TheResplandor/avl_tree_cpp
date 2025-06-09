cmake_minimum_required(VERSION 3.10.0)
get_filename_component(PROJECT_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -O0)
endif()

set(APP_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${APP_ROOT_DIR})

add_executable(${PROJECT_NAME} test.cpp AVL_tree.hpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG _DEBUG>)
