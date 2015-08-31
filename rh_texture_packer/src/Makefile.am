bin_PROGRAMS = rh_texture_packer

rh_texture_packer_SOURCES =  CreateSpriteMap.hpp
rh_texture_packer_SOURCES += file_header.h
rh_texture_packer_SOURCES += FindContent.hpp
rh_texture_packer_SOURCES += hash.cpp hash.hpp
rh_texture_packer_SOURCES += lz4.c lz4.h lz4hc.c lz4hc.h
rh_texture_packer_SOURCES += main.cpp
rh_texture_packer_SOURCES += Output.hpp
rh_texture_packer_SOURCES += Pack.hpp
rh_texture_packer_SOURCES += UniqueImages.hpp
rh_texture_packer_SOURCES += args.h
rh_texture_packer_SOURCES += args.c

rh_texture_packer_LDADD = $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB)
AM_CXXFLAGS = $(BOOST_CPPFLAGS)
AM_LDFLAGS = $(BOOST_LDFLAGS)

