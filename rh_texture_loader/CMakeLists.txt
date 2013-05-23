
cmake_minimum_required(VERSION 2.8)

project(rh_texture_loader)

FILE(GLOB c_source_files *.c)

add_library(rh_texture_loader SHARED ${c_source_files})
add_executable(test test.c)

target_link_libraries (rh_texture_loader GL)
target_link_libraries(test rh_texture_loader rh_window_glx)

install(TARGETS rh_texture_loader DESTINATION lib)
install(FILES rh_texture_loader.h DESTINATION include)


