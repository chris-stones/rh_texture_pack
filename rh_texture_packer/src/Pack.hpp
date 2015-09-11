
#pragma once

#include "binpack2d.hpp"

#include "FindContent.hpp"
#include "UniqueImages.hpp"

static int AccumulateUnique(UniqueImages & uniqueImages, const Path::Directory & dir) {

  for( Path::Image::Vector::const_iterator itor = dir.GetImages().begin(); itor != dir.GetImages().end(); itor++ )
    uniqueImages += *itor;

  for( Path::Directory::Vector::const_iterator itor = dir.GetDirectories().begin(); itor != dir.GetDirectories().end(); itor++ )
    AccumulateUnique( uniqueImages, *itor );

  return 0;
}

template<typename _T> static void Pack( 	const Path::Directory & dir,
					BinPack2D::ContentAccumulator<_T> &contentAccumulator,
					UniqueImages &uniqueImages,
					int pad ) {

  AccumulateUnique(uniqueImages, dir);

  const std::vector<Path::Image> &unique = uniqueImages.GetUnique();

  for(std::vector<Path::Image>::const_iterator itor = unique.begin(); itor != unique.end(); itor++) {

    BinPack2D::Size size(itor->GetWidth()+2*pad, itor->GetHeight()+2*pad );

    contentAccumulator += BinPack2D::Content<_T>( *itor, BinPack2D::Coord(), size, false );

  }
}

