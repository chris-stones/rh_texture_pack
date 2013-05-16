
cmake_minimum_required(VERSION 2.8)

project(rh_texture_packer)

add_executable(rh_texture_packer main.cpp lz4hc.c lz4.c hash.c config.cpp)

target_link_libraries (rh_texture_packer imgutil binpack boost_filesystem)

install(TARGETS rh_texture_packer RUNTIME DESTINATION bin)

add_subdirectory(rh_texture_loader)

