
project(rh_texture_loader)

FILE(GLOB c_source_files *.c)

add_library(rh_texture_loader ${c_source_files})

#target_link_libraries (rh_texture_loader )

#install(TARGETS rh_texture_packer RUNTIME DESTINATION bin)


