

#pragma once


#include<string>
#include<algorithm>
#include<cstdlib>
#include<map>
#include<assert.h>
#include<string.h>

#include "config.h"
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

template<typename _T> std::map< unsigned int, rhtpak_hdr_hash > CreateSpriteMap(

    const BinPack2D::ContentAccumulator<_T> &outputContent,
    const std::map<std::string,std::vector<std::string> > &aliasMap,
    const arguments & args,
    unsigned int * seed_out)
{
    std::map< unsigned int, rhtpak_hdr_hash > rmap;

    unsigned int seed = 0x69;

    for(;;) {

        rmap.clear();

		if(args.debug)
			printf("clearing rmap\n");

        bool hash_collision = false;

        typename BinPack2D::Content<_T>::Vector::const_iterator unique_itor = outputContent.Get().begin();
        typename BinPack2D::Content<_T>::Vector::const_iterator unique_end  = outputContent.Get().end();

        while( unique_itor != unique_end ) {

            const BinPack2D::Content<_T> &content = *unique_itor;

            std::string hashString = get_game_resource_name(content.content, args.resources);

            unsigned int hashval = hash( hashString.c_str() , seed );

            if( rmap.find( hashval ) != rmap.end() ) {
                hash_collision = true;
                break;
            }

            float cw = static_cast<float>( content.size.w-args.pad*2) / static_cast<float>(args.width);
            float ch = static_cast<float>( content.size.h-args.pad*2) / static_cast<float>(args.height);
            float cx = static_cast<float>( content.coord.x+args.pad) / static_cast<float>(args.width);
            float cy = static_cast<float>( content.coord.y+args.pad) / static_cast<float>(args.height);
            float cz = static_cast<float>( content.coord.z);

            rhtpak_hdr_hash &res = rmap[hashval];
            res.hash = hashval;
            res.w = content.size.w-args.pad*2;
            res.h = content.size.h-args.pad*2;
            res.flags = content.rotated ? 1 : 0;
            res.i = content.coord.z;

            res.tex_coords[0].s = cx   ;
            res.tex_coords[0].t = cy; // tl
            res.tex_coords[1].s = cx   ;
            res.tex_coords[1].t = cy+ch;    // bl
            res.tex_coords[2].s = cx+cw;
            res.tex_coords[2].t = cy; // tr
            res.tex_coords[3].s = cx+cw;
            res.tex_coords[3].t = cy+ch;    // br

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

            if(args.debug) {
				printf("UNIQUE HASH [0x%08x] \"%s\" {i:%2d, w:%4d, h:%4d, [0]{%08f,%08f}, [2]{%08f,%08f}}\n",
					hashval,
					hashString.c_str(),
					res.i,
					res.w, res.h,
					res.tex_coords[0].s, res.tex_coords[0].t,
					res.tex_coords[2].s, res.tex_coords[2].t );
			}

            ++unique_itor;
        }

        // now link up the aliases!
        if(!hash_collision) {

            std::map<std::string,std::vector<std::string> >::const_iterator alias_itor = aliasMap.begin();
            std::map<std::string,std::vector<std::string> >::const_iterator alias_end  = aliasMap.end();

            while(alias_itor != alias_end) {

                for(std::vector<std::string>::const_iterator	alias_fn = alias_itor->second.begin();
                        alias_fn != alias_itor->second.end();
                        alias_fn++)
                {
                    std::string alias_s = get_game_resource_name(*alias_fn, args.resources);
                    int alias_hashval = hash( alias_s.c_str() , seed );

                    if( rmap.find( alias_hashval ) != rmap.end() ) {
                        hash_collision = true;
                        alias_itor = alias_end;
                        break;
                    }

                    // no collisions... map alias.
                    std::string orig_s = get_game_resource_name(alias_itor->first, args.resources);
					int orig_hashval = hash( orig_s.c_str(), seed );
                    rmap[alias_hashval] = rmap[orig_hashval];
					rmap[alias_hashval].hash = alias_hashval;

					if(args.debug) {
						printf("ALIAS  HASH [0x%08x]=>[0x%08x] \"%s\"=>\"%s\" {i:%2d, w:%4d, h:%4d, [0]{%08f,%08f}, [2]{%08f,%08f}}\n",
							alias_hashval,   orig_hashval,
							alias_s.c_str(), orig_s.c_str(),
							rmap[alias_hashval].i,
							rmap[alias_hashval].w, rmap[alias_hashval].h,
							rmap[alias_hashval].tex_coords[0].s, rmap[alias_hashval].tex_coords[0].t,
							rmap[alias_hashval].tex_coords[2].s, rmap[alias_hashval].tex_coords[2].t );
					}
                }

                alias_itor++;
            }
        }

        if(!hash_collision)
            break;
        ++seed;
    }

    if(seed_out)
        *seed_out = seed;

    return rmap;
}


