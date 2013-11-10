

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

template<typename _T> std::map< unsigned int, rhtpak_hdr_hash > CreateSpriteMap(

    const BinPack2D::ContentAccumulator<_T> &outputContent,
    const std::map<std::string,std::vector<std::string> > &aliasMap,
    const char * resource_root,
    int w,
    int h,
    int pad,
    unsigned int * seed_out)
{
    std::map< unsigned int, rhtpak_hdr_hash > rmap;

    unsigned int seed = 0x69;

    for(;;) {

        rmap.clear();

        bool hash_collision = false;

        typename BinPack2D::Content<_T>::Vector::const_iterator unique_itor = outputContent.Get().begin();
        typename BinPack2D::Content<_T>::Vector::const_iterator unique_end  = outputContent.Get().end();

        while( unique_itor != unique_end ) {

            const BinPack2D::Content<_T> &content = *unique_itor;

            std::string hashString = get_game_resource_name(content.content, resource_root);

            unsigned int hashval = hash( hashString.c_str() , seed );

            if( rmap.find( hashval ) != rmap.end() ) {
                hash_collision = true;
                break;
            }

            float cw = static_cast<float>( content.size.w-pad*2) / static_cast<float>(w);
            float ch = static_cast<float>( content.size.h-pad*2) / static_cast<float>(h);
            float cx = static_cast<float>( content.coord.x+pad) / static_cast<float>(w);
            float cy = static_cast<float>( content.coord.y+pad) / static_cast<float>(h);
            float cz = static_cast<float>( content.coord.z);

            rhtpak_hdr_hash &res = rmap[hashval];
            res.hash = hashval;
            res.w = content.size.w-pad*2;
            res.h = content.size.h-pad*2;
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
                    std::string s = get_game_resource_name(*alias_fn, resource_root);
                    int hashval = hash( s.c_str() , seed );

                    if( rmap.find( hashval ) != rmap.end() ) {
                        hash_collision = true;
                        alias_itor = alias_end;
                        break;
                    }

                    // no collisions... map alias.
                    s = get_game_resource_name(alias_itor->first, resource_root);
                    rmap[hashval] = rmap[hash( s.c_str(), seed )];
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


