
#pragma once

#include <boost/filesystem.hpp>
#include <libimgutil.h>
#include "args.h"
#include "hash.hpp"

namespace fs = boost::filesystem;

namespace SourceData {

	typedef enum {
		Colour = (1<<0),
		Alpha  = (1<<1),
		Full   = Colour | Alpha,
	} source_data_enum_t;

	const char * ToString(source_data_enum_t source) {

		switch(source) {
		case Colour:
			return "SourceData::Colour";
		case Alpha:
			return "SourceData::Alpha";
		case Full:
			return "SourceData::Full";
		default:
			return "SourceData::UNKNOWN";
		}
	}
};

namespace Path {

class Image {

    int w;
    int h;
    std::string real_path;
    std::string hash_name;
    SourceData::source_data_enum_t source_data;

    Image(int w, int h, const std::string &real_path, const std::string &hash_name, const SourceData::source_data_enum_t &source_data)
    	:	w(w), h(h), real_path(real_path), hash_name(hash_name), source_data(source_data)
    {
    }

public:

    class OpenImageException : public std::exception {
    public:
        const char * what() const throw() {
            return "OpenImageException";
        }
    };

    typedef std::vector<Image> Vector;

    Image( const arguments & args, const fs::path &full_path ) {

        struct imgImage * image = NULL;

        const char * fn = full_path.native().c_str();

        if( imgAllocAndStat(&image, fn) != IMG_OKAY ) {

            throw OpenImageException();
        }

        w = image->width;
        h = image->height;
        real_path = full_path.native();
        hash_name = get_game_resource_name(real_path, "", args.resources);

        source_data = SourceData::Colour;

        if(image->format & IMG_FMT_COMPONENT_ALPHA) {

        	// source image has an alpha channel, but is it actually used?

        	// convert to RGBA32 if not already in that format.
        	if(image->format != IMG_FMT_RGBA32) {

        		imgImage * rgba32 = NULL;
        		imgAllocImage(&rgba32);
        		imgAllocPixelBuffers(image);
        		imgReadFile(image,fn);
        		rgba32->format = IMG_FMT_RGBA32;
        		rgba32->width = image->width;
        		rgba32->height = image->height;
        		imgAllocPixelBuffers(rgba32);
        		imguCopyImage(rgba32,image);
        		imgFreeAll(image);
        		image = rgba32;
        	}

        	{
        		// Test for any low-opacity pixels...
				unsigned char * rgba_data = static_cast<unsigned char *>(image->data.channel[0]);
				for(unsigned int offset=0;offset<image->linearsize[0];offset+=4) {
					if(rgba_data[offset+3] < 254) {
						// Got one.. looks like alpha channel is used.
						//  we can't strip it!
						source_data = SourceData::Full;
						break;
					}
				}
        	}

        	if( args.debug && ( source_data == SourceData::Colour ) )
        		printf("\'%s\' has an unused alpha channel, stripping it.\n", fn);
        }


        imgFreeAll(image);
    }

    int GetWidth() const {
        return w;
    }
    int GetHeight() const {
        return h;
    }
    SourceData::source_data_enum_t GetSourceDataType() const {

    	return source_data;
    }

    const std::string &GetFileName() const {
        return real_path;
    }

    const std::string &GetResourceName() const {

    	return hash_name;
    }

    bool operator < ( const Image & that ) const {

        if( this->w != that.w )
            return this->w < that.w;

        if( this->h != that.h )
            return this->h < that.h;

        if(this->source_data != that.source_data)
        	return this->source_data < that.source_data;

        return this->real_path < that.real_path;
    }

    bool HasAlpha() const {

    	return GetSourceDataType() & SourceData::Alpha ? true : false;
    }

    Image RemoveAlpha(const arguments &args) {

		source_data = SourceData::Colour;

		std::string alphaHashName = get_game_resource_name(real_path, "/A/", args.resources);
		return Image(w,h,real_path,alphaHashName,SourceData::Alpha);
	}

    bool IsAnAlphaMask() const {

    	return source_data == SourceData::Alpha;
    }
};

class Directory {

    std::vector<Image> 		images;
    std::vector<Directory>	subDirs;

public:

    typedef std::vector<Directory> Vector;

    class OpenDirException : public std::exception {
    public:
        const char * what() const throw() {
            return "OpenDirException";
        }
    };

    Directory(const arguments & args, const fs::path &full_path) {

        Construct(args, full_path );
    }

    Directory(const arguments & args) {

		Construct(args, args.resources );
	}

    const Image::Vector &GetImages( ) 		const {
        return images;
    }
    const Vector        &GetDirectories() 	const {
        return subDirs;
    }

private:

    void Construct(const arguments & args, const fs::path &full_path) {

        if(!fs::exists(full_path))
            throw OpenDirException();

        if(!fs::is_directory(full_path))
            throw OpenDirException();

        fs::directory_iterator end;
        for( fs::directory_iterator itor = fs::directory_iterator( full_path ); itor != end; itor++) {

            if(fs::is_directory( itor->status() )) {

                subDirs.push_back( Directory( args, itor->path() ));

            } else if( fs::is_regular_file( itor->status() )) {

                try {

                	Image image(args, itor->path());

                	if(image.HasAlpha() && !(args.format & IMG_FMT_COMPONENT_ALPHA))
                		images.push_back(image.RemoveAlpha(args));

                    images.push_back(image);

                } catch(...) {

                    // TODO: distinguish between unsupported image format, and attempts to open text files!
                }
            }
        }
    }
};
}

