
#pragma once

#include<vector>
#include<map>
#include<string>
#include<exception>
#include<libimg.h>
#include<libimgutil.h>

class LibImg {
  
  imgImage * img;
  
  mutable bool hashed;
  mutable int  hashVal;
  
public:
  
  class LibImgOpenException : public std::exception {public: const char * what() const throw() { return "LibImgOpenException"; } };
  
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
    
//    if(this->Hash32() != that.Hash32())
//      return false;
   
    return imguBinaryCompare(this->img, that.img) == 0;
  }
};

class UniqueImages
{
    typedef std::vector<std::string> 		StringVector;
    typedef std::map<int,StringVector> 		HashStringVectorMap;
    typedef std::map<std::string,StringVector>	StringStringVector;
    
    HashStringVectorMap 			uniqueHashStringVectorMap; 	// unique files ordered by hash.
    StringVector 				uniqueStringVector;		// unique files.
    StringVector				allStringVector;		// all files
    StringStringVector 				aliasMap;			// duplicate images.

public:
  
  UniqueImages()
  {
  }
  
  const std::vector<std::string> &GetUnique() 	const { return uniqueStringVector; 	}
  const std::vector<std::string> &GetAll() 	const { return allStringVector; 	}
  
  const std::map<std::string,std::vector<std::string> > &GetAliasMap() const { 
    
    return aliasMap; 
  }
  
  UniqueImages & operator += ( const std::string fn ) {
   
    LibImg addImg(fn);
    
    StringVector &sv = uniqueHashStringVectorMap[addImg.Hash32()];
    
    bool unique = true;
    for(StringVector::iterator itor = sv.begin(); unique && (itor != sv.end()); itor++)
      if(LibImg(*itor) == addImg) {
	printf("Image %s is a duplicate of %s.\n", fn.c_str(), itor->c_str());
	unique = false;
	
	aliasMap[*itor].push_back(fn);
      }
    
    if(unique) {
      sv.push_back(fn);
      uniqueStringVector.push_back(fn);
    }
    
    allStringVector.push_back(fn);
    
    return *this;
  }
};

