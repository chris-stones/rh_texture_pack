
#pragma once

#include <boost/filesystem.hpp>
#include<libimgutil.h>

namespace fs = boost::filesystem;

namespace AlphaType {
typedef enum {

    Opaque,
    Mask,
    Gradient,

} alpha_enum_t;

const char * ToString( alpha_enum_t type ) {
    switch(type) {
    case Opaque:
        return "AlphaType::Opaque";
    case Mask:
        return "AlphaType::Mask";
    case Gradient:
        return "AlphaType::Gradient";
    default :
        return "AlphaType::UNKNOWN";
    }
}
}

namespace Path {

class Image {

    int w;
    int h;
    std::string path;
    AlphaType::alpha_enum_t alphaType;

public:

    class OpenImageException : public std::exception {
    public:
        const char * what() const throw() {
            return "OpenImageException";
        }
    };

    typedef std::vector<Image> Vector;

    Image( const fs::path &full_path ) {

        struct imgImage * image = NULL;

        const char * fn = full_path.native().c_str();

        if( imgAllocAndStat(&image, fn) != IMG_OKAY ) {

            throw OpenImageException();
        }

        w = image->width;
        h = image->height;
        path = full_path.native();
        alphaType = AlphaType::Gradient; // todo!

        imgFreeAll(image);
    }

    int GetWidth() const {
        return w;
    }
    int GetHeight() const {
        return h;
    }

    const std::string &GetFileName() const {
        return path;
    }

    bool operator < ( const Image & that ) const {

        if( this->w != that.w )
            return this->w < that.w;

        if( this->h != that.h )
            return this->h < that.h;

        if( this->alphaType != that.alphaType )
            return this->alphaType < that.alphaType;

        return this->path < that.path;
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

    Directory(const fs::path &full_path) {

        Construct( full_path );
    }

    const Image::Vector &GetImages( ) 		const {
        return images;
    }
    const Vector        &GetDirectories() 	const {
        return subDirs;
    }

private:

    void Construct(const fs::path &full_path) {

        if(!fs::exists(full_path))
            throw OpenDirException();

        if(!fs::is_directory(full_path))
            throw OpenDirException();

        fs::directory_iterator end;
        for( fs::directory_iterator itor = fs::directory_iterator( full_path ); itor != end; itor++) {

            if(fs::is_directory( itor->status() )) {

                subDirs.push_back( Directory( itor->path() ));

            } else if( fs::is_regular_file( itor->status() )) {

                try {

                    images.push_back( Image( itor->path() ));

                } catch(...) {

                    // TODO: distinguish between unsuppoted image format, and attempts to open text files!
                }
            }
        }
    }
};
}

