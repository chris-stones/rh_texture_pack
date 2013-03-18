

#pragma once


#include<string>
#include<algorithm>
#include<cstdlib>
#include<map>
#include<assert.h>
#include<string.h>

#include "hash.h"
#include "file_header.h"  


static int tohashable(int c) {
 
  switch(c) {
    default:
      return tolower(c);
    case '/':
    case '\\':
      return '.';
  }
}

static std::string get_game_resource_name(std::string hashString, const char * resource_root) {
 
  int dot = hashString.find_last_of(".");
  assert(dot != std::string::npos );
  hashString = hashString.substr(0, dot);
  std::transform(hashString.begin(), hashString.end(), hashString.begin(), tohashable);
  hashString = std::string( hashString.begin()+strlen(resource_root), hashString.end() );
  if(hashString[0] == '.')
    hashString = std::string( hashString.begin()+1, hashString.end() );
  return hashString;
}

template<typename _T> std::map< unsigned int, rhtpak_hdr_hash > CreateSpriteMap(const BinPack2D::ContentAccumulator<_T> &outputContent, const char * resource_root, int w, int h, int pad) {
  
  // TODO: take padding into account when generating tex-coords!!!
  
    typename BinPack2D::Content<_T>::Vector::const_iterator itor = outputContent.Get().begin();
    typename BinPack2D::Content<_T>::Vector::const_iterator end  = outputContent.Get().end();
    
    std::map< unsigned int, rhtpak_hdr_hash > rmap;
    
    unsigned int seed = 0x69;
    
    for(;;) {
      
      rmap.clear();
      
      bool hash_collision = false;
    
      while( itor != end ) {
	
	const BinPack2D::Content<_T> &content = *itor;
      
	std::string hashString = get_game_resource_name(content.content, resource_root);
	
	unsigned int hashval = hash( hashString.c_str() , seed );
	
	printf("hashing %s = %d\n", hashString.c_str(), hashval);
	
	if( rmap.find( hashval ) != rmap.end() ) {
	 hash_collision = true;
	 printf("HASH COLLISION WITH SEED %d, trying %d...\n", seed, seed+1); 
	 break;
	}

	float cw = static_cast<float>( content.size.w) / static_cast<float>(w);
	float ch = static_cast<float>( content.size.h) / static_cast<float>(h);
	float cx = static_cast<float>( content.coord.x) / static_cast<float>(w);
	float cy = 1.0f - ((static_cast<float>( content.coord.y) / static_cast<float>(h)) + ch);
	float cz = static_cast<float>( content.coord.z);
	
	rhtpak_hdr_hash &res = rmap[hashval];
	res.hash = hashval;
	res.w = content.size.w;
	res.h = content.size.h;
	res.flags = content.rotated ? 1 : 0;
	res.i = content.coord.z;
	
	res.tex_coords[0].s = cx   ; res.tex_coords[0].t = cy+ch; // tl
	res.tex_coords[1].s = cx   ; res.tex_coords[1].t = cy;    // bl
	res.tex_coords[2].s = cx+cw; res.tex_coords[2].t = cy+ch; // tr
	res.tex_coords[3].s = cx+cw; res.tex_coords[3].t = cy;    // br
	
	res.tex_coords[0].p = 
	res.tex_coords[1].p = 
	res.tex_coords[2].p = 
	res.tex_coords[3].p = cz;
	  
	if(content.rotated) {
	  
	  const int TOP_LEFT = 0;
	  const int BOT_LEFT = 1;
	  const int TOP_RIGHT = 2;
	  const int BOT_RIGHT = 3;

	  rhtpak_hdr_hash rres = res;
	  rres.tex_coords[TOP_LEFT] = res.tex_coords[TOP_RIGHT];
	  rres.tex_coords[BOT_LEFT] = res.tex_coords[TOP_LEFT];
	  rres.tex_coords[TOP_RIGHT] = res.tex_coords[BOT_RIGHT];
	  rres.tex_coords[BOT_RIGHT] = res.tex_coords[BOT_LEFT];
	  rres.w = res.h;
	  rres.h = res.w;
	  
	  res = rres;
	}
	
	++itor;
      }
      
      if(!hash_collision)
	break;
      ++seed;
    }
    
    return rmap; 
}

