
#ifndef _RH_TEXTURE_INTERNAL_H
#define _RH_TEXTURE_INTERNAL_H

#define GL_GLEXT_PROTOTYPES 1

#ifdef TARGET_GLES2
  #include <GLES2/gl2.h>
  #include <GLES2/gl2ext.h>
#else
  #include <GL/gl.h>
  #include <GL/glext.h>
#endif

#ifndef GL_TEXTURE_2D_ARRAY_EXT
#define GL_TEXTURE_2D_ARRAY_EXT 0x8C1A
#endif
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT  0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif

#ifdef TARGET_GLES

  #define GL_COMPRESSED_TEX_IMAGE_3D 		glCompressedTexImage3DOES
  #define GL_TEX_IMAGE_3D 			glTexImage3DOES
  #define GL_TEX_SUMBIMAGE_3D 			glTexSubImage3DOES
  #define GL_COMPRESSED_TEX_SUBIMAGE_3D		glCompressedTexSubImage3DOES
  
#else

  #define GL_COMPRESSED_TEX_IMAGE_3D 		glCompressedTexImage3D
  #define GL_TEX_IMAGE_3D 			glTexImage3D
  #define GL_TEX_SUMBIMAGE_3D 			glTexSubImage3D
  #define GL_COMPRESSED_TEX_SUBIMAGE_3D 	glCompressedTexSubImage3D

#endif

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <libimg.h>

#include "lz4.h"

/******************************************************************************
 * structures taken from rh_texture_packer
 */

struct rhtpak_hdr {

  char magic[16]; 		// "rockhopper.tpak"
  unsigned int version; 	// impl version
  unsigned int flags;		// flags
  unsigned int format;		// format if constant
  unsigned int w;		// width if constant
  unsigned int h;		// height if constant
  unsigned int channels; 	// channels if constant
  unsigned int depth;		// number of textures
  unsigned int resources;	// number of resources ( sprites )
  unsigned int seed;		// hash seed
  unsigned int text_data_ptr;	// pointer to rhtpak_hdr_tex_data. ( array length is rhtpak_hdr::depth )
  unsigned int hash_data_ptr;	// pointer to rhtpak_hdr_hash ( array length is rhtpak_hdr::resources )
};

struct rhtpak_hdr_tex_data {

  unsigned int x;		// x offset of this frame.
  unsigned int y;		// y offset of this frame.
  unsigned int w;		// width of this layer
  unsigned int h;		// height of this layer
  unsigned int flags;		// flags
  unsigned int format;		// format
  unsigned int channels;

  struct {
    unsigned int file_offset;		// file offset of pixel data
    unsigned int file_length;		// file length of pixel data
    unsigned int uncompressed_size;	// Uncompressed size of pixel data ( if compressed )

  } channel[4]; // [0] = packed rgb(a) or planar Y, [1]= null or planar Cb, [2]=null or planar Cr, [3]=null or planar Alpha
};

struct rhtpak_hdr_hash {

  unsigned int hash;  	/* resource name hash */
  unsigned int flags; 	/* flags */
  unsigned int w;     	/* orig resource width in pixels */
  unsigned int h;     	/* orig resource height in pixels */
  unsigned int i;	/* texture array index */
  struct {
    float s;	      	/* s */
    float t;	      	/* t */
    float p;		/* p */
  } tex_coords[4]; // top-left, bot-left, top-right, bot-right.
};

struct _texpak_type {

  GLuint * textures;
  GLenum target;
  int textures_length;
  unsigned int seed;
  struct rhtpak_hdr_hash * hash;
  int hash_length;
};

#include "rh_texture_loader.h"


#endif /*** _RH_TEXTURE_INTERNAL_H ***/



