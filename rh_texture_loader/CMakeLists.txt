
cmake_minimum_required(VERSION 2.8)

project(rh_texture_loader)

add_definitions(-std=c++0x)

FILE(GLOB c_source_files *.c)

add_library(rh_texture_loader SHARED ${c_source_files})
add_executable(test_c test.c)
add_executable(test_cxx test.cpp)

target_link_libraries (rh_texture_loader GL)
target_link_libraries(test_c rh_texture_loader rh_window_glx)
target_link_libraries(test_cxx rh_texture_loader rh_window_glx)

install(TARGETS rh_texture_loader DESTINATION lib)
install(FILES rh_texture_loader.h rh_texture_loader.hpp DESTINATION include)


