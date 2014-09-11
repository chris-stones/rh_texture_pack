
#ifndef _RH_TEXTURE_INTERNAL_H
#define _RH_TEXTURE_INTERNAL_H

#include "rh_file.h"

#ifdef __ANDROID__
  #ifndef RH_TARGET_API_GLES2
    #define RH_TARGET_API_GLES2
  #endif
  #ifndef RH_TARGET_OS_ANDROID
    #define RH_TARGET_OS_ANDROID
  #endif
#endif

#ifdef __ANDROID__
	#include <android/log.h>
	#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
	#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
	#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-activity", __VA_ARGS__))
#else
	#define LOGI(...) ((void)printf(__VA_ARGS__))
	#define LOGW(...) ((void)printf(__VA_ARGS__))
	#define LOGE(...) ((void)printf(__VA_ARGS__))
#endif

#define GL_GLEXT_PROTOTYPES 1

#ifdef RH_TARGET_API_GLES2
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
#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES 0x8D64
#endif


#ifdef RH_TARGET_API_GLES2

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
  unsigned int orig_w;     	/* orig resource width in pixels */
  unsigned int orig_h;     	/* orig resource height in pixels */
  unsigned int i;	/* texture array index */
  struct {
    float s;	      	/* s */
    float t;	      	/* t */
    float p;		/* p */
  } tex_coords[4]; 	// top-left, bot-left, top-right, bot-right.
};

struct _texpak_type {

  struct rhtpak_hdr header;

  rh_file_t file;

  GLuint * textures;
  GLenum target;
  int textures_length;
  unsigned int seed;
  struct rhtpak_hdr_hash * hash;
  int hash_length;
  int flags;
  int refcount;
};

struct _texpak_idx_type {

	struct _texpak_type * pak;
	size_t index;
};

#include "rh_texture_loader.h"
#include <libimg.h> // for image format macros only... no need to distribute .so/.dll

#endif /*** _RH_TEXTURE_INTERNAL_H ***/



