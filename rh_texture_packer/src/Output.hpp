

#pragma once

#include <exception>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <map>
#include <libimg.h>

#include "lz4hc.h"
#include "lz4.h"
#include "file_header.h"

class Output {

  FILE *file;

  rhtpak_hdr header;

  void Open(const char * fn) {

    file = fopen(fn, "wb");
    if(!file)
      throw OutputOpenException();

    memset(&header, 0, sizeof header);
    Write(header);

    header.text_data_ptr = ftell(file);

    rhtpak_hdr_tex_data dummy_tex_data;
    memset(&dummy_tex_data,0,sizeof dummy_tex_data);
    for(int i=0;i<layers;i++)
      Write(dummy_tex_data);

    header.hash_data_ptr = ftell(file);

    rhtpak_hdr_hash dummy_hash_data;
    for(int i=0;i<sprites;i++)
      Write(dummy_hash_data);

    header.depth = layers;
    header.resources = sprites;
    header.w = w;
    header.h = h;
    header.channels = imgGetChannels((imgFormat)format);
    header.format = format;
    header.flags = 0;
    header.version = 0;
    memcpy(&header.magic,"rockhopper.tpak",16);

    Seek(0, SEEK_SET);
    Write(header);
  }

  static const char * WhenceStr(int whence) {

    switch(whence) {
      default: return "???";
      case SEEK_SET: return "SEEK_SET";
      case SEEK_CUR: return "SEEK_CUR";
      case SEEK_END: return "SEEK_END";
    }
  }

  void Seek( long offset, int whence ) {

    if( fseek( file, offset, whence ) != 0 ) {

      throw OutputSeekException();
    }
  }

  void Write( const void * data, unsigned int size ) {

    if(fwrite(data,size,1,file) != 1)
      throw OutputWriteException();
  }

  template<typename _T> void Write( const _T & data ) {

    Write(&data,sizeof data);
  }

  void Close() {

    fclose(file);
  }

  const int layers;
  const int sprites;
  const int w;
  const int h;
  const int format;
  int next_layer;

  arguments args;

public:

  class OutputOpenException : public std::exception {public: const char * what() const throw() { return "OutputOpenException"; } };
  class OutputWriteException : public std::exception {public: const char * what() const throw() { return "OutputWriteException"; } };
  class OutputSeekException : public std::exception {public: const char * what() const throw() { return "OutputSeekException"; } };

  Output(const arguments &args, const std::string &fn, int sprites, int w, int h,int layers, int format)
    :	args(args),
		file(NULL),
		layers(layers),
		sprites(sprites),
		w(w),
		h(h),
		format(format),
		next_layer(0)
  {
    Open(fn.c_str());
  }

  virtual ~Output(void) throw() {

    Close();
  }

  void WriteLayer(const struct imgImage *img) {

    rhtpak_hdr_tex_data layer;
    memset(&layer, 0, sizeof layer);

    layer.x = 0;
    layer.y = 0;
    layer.w = w;
    layer.h = h;
    layer.flags = 0;
    layer.format = img->format;
    layer.channels = imgGetChannels(img->format);

    for(int channel = 0; channel < layer.channels; channel++ ) {

      int src_len = img->linearsize[channel];

//    void * cdata = malloc( LZ4_compressBound( src_len ) );
	  void * cdata = calloc( 1, LZ4_compressBound( src_len ) );

      unsigned int csize = LZ4_compressHC((const char *)(img->data.channel[channel]), (char *)(cdata), src_len );

      Seek( 0, SEEK_END );

      layer.channel[channel].file_offset = ftell(file);
      layer.channel[channel].file_length = csize;
      layer.channel[channel].uncompressed_size = src_len;

	  if(args.debug)
		printf("writing compressed data @ %8d (channel %d) %d -> %d\n", (int)ftell(file), channel, src_len, csize);

      Write( cdata, csize );
    }

    Seek(header.text_data_ptr + sizeof(rhtpak_hdr_tex_data) * next_layer, SEEK_SET);

    Write(layer);

    ++next_layer;
  }

  void WriteHashMap( unsigned int seed, const std::map< unsigned int, rhtpak_hdr_hash > & spriteMap ) {

      std::map< unsigned int, rhtpak_hdr_hash >::const_iterator itor = spriteMap.begin();
      std::map< unsigned int, rhtpak_hdr_hash >::const_iterator end = spriteMap.end();

      // UGLY - rewind, and write header AGAIN with seed
      Seek( 0, SEEK_SET );
      header.seed = seed;
      Write(header);
      /////////////////////

      Seek( header.hash_data_ptr, SEEK_SET);

	  int hashes = 0;

	  if(args.debug)
		printf("writing hashmap at 0x%08x (size=%d)\n", (int)ftell(file), (int)spriteMap.size());

      while(itor != end) {

		const rhtpak_hdr_hash &hash = itor->second;

		if(args.debug) {
			printf("@ 0x%08x: hash: 0x%08x\n", (int)ftell(file), hash.hash );
			printf("    %1.6f  %1.6f\n", hash.tex_coords[0].s, hash.tex_coords[0].t);
			printf("    %1.6f  %1.6f\n", hash.tex_coords[1].s, hash.tex_coords[1].t);
			printf("    %1.6f  %1.6f\n", hash.tex_coords[2].s, hash.tex_coords[2].t);
			printf("    %1.6f  %1.6f\n", hash.tex_coords[3].s, hash.tex_coords[3].t);
		}

		Write(hash);

		++hashes;

		itor++;
    }

    assert(hashes == sprites);
  }
};



