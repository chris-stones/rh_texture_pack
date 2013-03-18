
#pragma once

#include<binpack2d.hpp>

#include "FindContent.hpp"

template<typename _T> static int Pack( const Path::Directory & dir, BinPack2D::ContentAccumulator<_T> &contentAccumulator, int pad ) {
  
  for( Path::Image::Vector::const_iterator itor = dir.GetImages().begin(); itor != dir.GetImages().end(); itor++ ) {
    
    const Path::Image &image = *itor;
    
    BinPack2D::Size size(image.GetWidth()+2*pad, image.GetHeight()+2*pad );
    
    contentAccumulator += BinPack2D::Content<_T>( image.GetFileName(), BinPack2D::Coord(), size, false );
  }
  
  for( Path::Directory::Vector::const_iterator itor = dir.GetDirectories().begin(); itor != dir.GetDirectories().end(); itor++ ) {
   
    const Path::Directory &dir = *itor;
    
    Pack( dir, contentAccumulator, pad );
  }
 
  return 0;
}


