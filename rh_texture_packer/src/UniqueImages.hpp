
#pragma once

#include<vector>
#include<map>
#include<string>
#include<set>
#include<exception>
#include<libimg.h>
#include<libimgutil.h>

#include "args.h"

class LibImg {

    imgImage * img;

    mutable bool hashed;
    mutable int  hashVal;

public:

    class LibImgOpenException : public std::exception {
    public:
        const char * what() const throw() {
            return "LibImgOpenException";
        }
    };

    LibImg(const std::string &fn)
        :	img(NULL),
            hashed(false),
            hashVal(0)
    {
        if( imgAllocAndRead(&img, fn.c_str()) != 0)
            throw LibImgOpenException();
    }

    virtual ~LibImg() {

        imgFreeAll(img);
    }

    int Hash32() const {

        if(!hashed) {
            hashVal = imguBinaryHash32(img);
            hashed = true;
        }

        return hashVal;
    }

    bool operator == (const LibImg &that) const {

        return imguBinaryCompare(this->img, that.img) == 0;
    }
};

class UniqueImages
{
	// Type of the hash of image data.
	typedef int                                                             data_hash_t;

	// Pair of data source ( colour/alpha/both ) and data hash;
	typedef std::pair<SourceData::source_data_enum_t,   data_hash_t>        source_data_hash_t;

	// Vector of images.
	typedef std::vector<Path::Image>                                        image_vector_t;

	// Map of resource name aliases.
	typedef std::map<Path::Image, std::vector<Path::Image> >                alias_map_t;

	// image hash to image vector map. ( copes with hash collisions )
	typedef std::map<source_data_hash_t, image_vector_t>                    hash_image_map_t;

	// list of unique game resource names.
	typedef std::set<std::string>                                           string_set_t;

	image_vector_t     uniqueImages;
	hash_image_map_t   hashImageMap;
	alias_map_t        alias;
	string_set_t       resourceNames;

    arguments args;

public:

    class UniqueImageException : public std::exception {
	public:
		const char * what() const throw() {
			return "UniqueImageException";
		}
	};

    UniqueImages(const arguments &args)
		:	args(args)
    {
    }

    UniqueImages & operator += (const Path::Image &source) {

    	LibImg newImg(source.GetFileName());

    	// Existing images matching new images data-hash.
    	image_vector_t & vector =
    		hashImageMap[
				source_data_hash_t(
					source.GetSourceDataType(), newImg.Hash32())];

    	// Test if image is identical, or just has a hash collision.
    	for(const auto & i : vector) {

    		if(LibImg(i.GetFileName()) == newImg) {

    			// New image is a duplicate of an existing image!
    			if(args.debug)
    				printf("Image %s is a duplicate of %s.\n", source.GetFileName().c_str(), i.GetFileName().c_str());

    			alias[i].push_back(source);

    			return *this;
    		}
    	}

    	if(args.debug)
    		printf("Image %s is unique.\n", source.GetFileName().c_str());

    	vector.push_back(source);
    	uniqueImages.push_back(source);

    	if(resourceNames.count(source.GetResourceName()))
    	{
    		printf("RESOURCE NAME COLLISION %s\n", source.GetResourceName());
    		printf("  This resource appears multiple times.\n");
    		printf("  Possibly with different file extensions\n.");
    		printf("  Possibly with different cases on a case sensitive file systems.\n");
    		printf("  REMOVE OR RENAME IT.\n");
    		throw UniqueImageException();
    	}
    	resourceNames.insert(source.GetResourceName());

    	return *this;
    }

    const image_vector_t & GetUnique() const {

    	return uniqueImages;
    }

    const alias_map_t GetAliasMap() const {

    	return alias;
    }
};

