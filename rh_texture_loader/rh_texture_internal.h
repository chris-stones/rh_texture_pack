
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


/***** IMG_FMT_* TAKEN FROM <libimg.h> DON'T MODIFY HERE!!! *****/

#define IMG_FMT_COMPONENT_RED			( 1<< 0)
#define IMG_FMT_COMPONENT_GREEN			( 1<< 1)
#define IMG_FMT_COMPONENT_BLUE			( 1<< 2)
#define IMG_FMT_COMPONENT_ALPHA			( 1<< 3)
#define IMG_FMT_COMPONENT_Y				( 1<< 4)
#define IMG_FMT_COMPONENT_CB			( 1<< 5)
#define IMG_FMT_COMPONENT_CR			( 1<< 6)
#define IMG_FMT_COMPONENT_PLANAR		( 1<< 7)
#define IMG_FMT_COMPONENT_420P			((1<< 8) | IMG_FMT_COMPONENT_PLANAR)
#define IMG_FMT_COMPONENT_PACKED		( 1<< 9)
#define IMG_FMT_COMPONENT_PACKED8		((1<<10) | IMG_FMT_COMPONENT_PACKED)
#define IMG_FMT_COMPONENT_PACKED15		((1<<11) | IMG_FMT_COMPONENT_PACKED)
#define IMG_FMT_COMPONENT_PACKED16		((1<<12) | IMG_FMT_COMPONENT_PACKED)
#define IMG_FMT_COMPONENT_PACKED24		((1<<13) | IMG_FMT_COMPONENT_PACKED)
#define IMG_FMT_COMPONENT_PACKED32		((1<<14) | IMG_FMT_COMPONENT_PACKED)
#define IMG_FMT_COMPONENT_PACKED48		((1<<15) | IMG_FMT_COMPONENT_PACKED)
#define IMG_FMT_COMPONENT_PACKED64		((1<<16) | IMG_FMT_COMPONENT_PACKED)
#define IMG_FMT_COMPONENT_RGBA			( 1<<17) // packed channel order
#define IMG_FMT_COMPONENT_ARGB			( 1<<18) // packed channel order
#define IMG_FMT_COMPONENT_BGRA			( 1<<19) // packed channel order
#define IMG_FMT_COMPONENT_ABGR			( 1<<20) // packed channel order
#define IMG_FMT_COMPONENT_YCBCRA		( 1<<21)
#define IMG_FMT_COMPONENT_YCRCBA		( 1<<22)
#define IMG_FMT_COMPONENT_GREY			( 1<<23)

#define IMG_FMT_COMPONENT_COMPRESSED		 (1<<24)
#define IMG_FMT_COMPONENT_DXT1			((1<<25) | IMG_FMT_COMPONENT_COMPRESSED)
#define IMG_FMT_COMPONENT_DXT3			((1<<26) | IMG_FMT_COMPONENT_COMPRESSED)
#define IMG_FMT_COMPONENT_DXT5			((1<<27) | IMG_FMT_COMPONENT_COMPRESSED)
#define IMG_FMT_COMPONENT_DXTn			(IMG_FMT_COMPONENT_DXT1 | IMG_FMT_COMPONENT_DXT3 | IMG_FMT_COMPONENT_DXT5)

#define IMG_FMT_COMPONENT_PMA			(1<<28)

#define IMG_FMT_COMPONENT_NULL			0

#define IMG_FMT_1(x) 				IMG_FMT_COMPONENT_ ## x
#define IMG_FMT_2(x1,x2) 			IMG_FMT_1(x1) | IMG_FMT_1(x2)
#define IMG_FMT_3(x1,x2,x3)			IMG_FMT_2(x1,x2) | IMG_FMT_1(x3)
#define IMG_FMT_4(x1,x2,x3,x4)			IMG_FMT_3(x1,x2,x3) | IMG_FMT_1(x4)
#define IMG_FMT_5(x1,x2,x3,x4,x5)		IMG_FMT_4(x1,x2,x3,x4) | IMG_FMT_1(x5)
#define IMG_FMT_6(x1,x2,x3,x4,x5,x6)		IMG_FMT_5(x1,x2,x3,x4,x5) | IMG_FMT_1(x6)
#define IMG_FMT_7(x1,x2,x3,x4,x5,x6,x7)		IMG_FMT_6(x1,x2,x3,x4,x5,x6) | IMG_FMT_1(x7)

#define IMG_FMT_COLS4(c1,c2,c3,c4) 	IMG_FMT_4(c1,c2,c3,c4)
#define IMG_FMT_COLS3(c1,c2,c3)		IMG_FMT_3(c1,c2,c3)

enum imgFormat {

	IMG_FMT_UNKNOWN		=	0,

	IMG_FMT_RGB16		= 	IMG_FMT_5(PACKED16,RGBA,RED,GREEN,BLUE),
	IMG_FMT_BGR16		= 	IMG_FMT_5(PACKED16,BGRA,RED,GREEN,BLUE),

	IMG_FMT_RGB15		= 	IMG_FMT_5(PACKED15,RGBA,RED,GREEN,BLUE),
	IMG_FMT_BGR15		= 	IMG_FMT_5(PACKED15,BGRA,RED,GREEN,BLUE),

	IMG_FMT_RGB24		=	IMG_FMT_5(PACKED24,RGBA,RED,GREEN,BLUE),
	IMG_FMT_BGR24		=	IMG_FMT_5(PACKED24,BGRA,RED,GREEN,BLUE),

	/*** uncompressed formats with alpha channels ***/
	IMG_FMT_RGBA32		=	IMG_FMT_6(PACKED32,RGBA,RED,GREEN,BLUE,ALPHA),
	IMG_FMT_BGRA32		=	IMG_FMT_6(PACKED32,BGRA,RED,GREEN,BLUE,ALPHA),
	IMG_FMT_ARGB32		=	IMG_FMT_6(PACKED32,ARGB,RED,GREEN,BLUE,ALPHA),
	IMG_FMT_ABGR32		=	IMG_FMT_6(PACKED32,ABGR,RED,GREEN,BLUE,ALPHA),
	IMG_FMT_RGBA64		=	IMG_FMT_6(PACKED64,RGBA,RED,GREEN,BLUE,ALPHA),
	IMG_FMT_BGRA64		=	IMG_FMT_6(PACKED64,BGRA,RED,GREEN,BLUE,ALPHA),
	IMG_FMT_ARGB64		=	IMG_FMT_6(PACKED64,ARGB,RED,GREEN,BLUE,ALPHA),
	IMG_FMT_ABGR64		=	IMG_FMT_6(PACKED64,ABGR,RED,GREEN,BLUE,ALPHA),
	IMG_FMT_GREYA16		=	IMG_FMT_3(PACKED16,GREY,ALPHA),
	IMG_FMT_GREYA32		=	IMG_FMT_3(PACKED32,GREY,ALPHA),
	IMG_FMT_YUVA420P	=	IMG_FMT_6(420P,YCBCRA,Y,CB,CR,ALPHA),

	/*** pre-multiplied alpha versions of above ***/
	IMG_FMT_RGBA32_PMA	=	IMG_FMT_7(PACKED32,RGBA,RED,GREEN,BLUE,ALPHA,PMA),
	IMG_FMT_BGRA32_PMA	=	IMG_FMT_7(PACKED32,BGRA,RED,GREEN,BLUE,ALPHA,PMA),
	IMG_FMT_ARGB32_PMA	=	IMG_FMT_7(PACKED32,ARGB,RED,GREEN,BLUE,ALPHA,PMA),
	IMG_FMT_ABGR32_PMA	=	IMG_FMT_7(PACKED32,ABGR,RED,GREEN,BLUE,ALPHA,PMA),
	IMG_FMT_RGBA64_PMA	=	IMG_FMT_7(PACKED64,RGBA,RED,GREEN,BLUE,ALPHA,PMA),
	IMG_FMT_BGRA64_PMA	=	IMG_FMT_7(PACKED64,BGRA,RED,GREEN,BLUE,ALPHA,PMA),
	IMG_FMT_ARGB64_PMA	=	IMG_FMT_7(PACKED64,ARGB,RED,GREEN,BLUE,ALPHA,PMA),
	IMG_FMT_ABGR64_PMA	=	IMG_FMT_7(PACKED64,ABGR,RED,GREEN,BLUE,ALPHA,PMA),
	IMG_FMT_GREYA16_PMA	=	IMG_FMT_4(PACKED16,GREY,ALPHA,PMA),
	IMG_FMT_GREYA32_PMA	=	IMG_FMT_4(PACKED32,GREY,ALPHA,PMA),
	IMG_FMT_YUVA420P_PMA	=	IMG_FMT_7(420P,YCBCRA,Y,CB,CR,ALPHA,PMA), //hmm.. how would this work!?


	IMG_FMT_RGBX32		=	IMG_FMT_5(PACKED32,RGBA,RED,GREEN,BLUE),
	IMG_FMT_BGRX32		=	IMG_FMT_5(PACKED32,BGRA,RED,GREEN,BLUE),
	IMG_FMT_XRGB32		=	IMG_FMT_5(PACKED32,ARGB,RED,GREEN,BLUE),
	IMG_FMT_XBGR32		=	IMG_FMT_5(PACKED32,ABGR,RED,GREEN,BLUE),
	IMG_FMT_RGB48		=	IMG_FMT_5(PACKED48,RGBA,RED,GREEN,BLUE),
	IMG_FMT_BGR48		=	IMG_FMT_5(PACKED48,BGRA,RED,GREEN,BLUE),
	IMG_FMT_RGBX64		=	IMG_FMT_5(PACKED64,RGBA,RED,GREEN,BLUE),
	IMG_FMT_BGRX64		=	IMG_FMT_5(PACKED64,BGRA,RED,GREEN,BLUE),
	IMG_FMT_XRGB64		=	IMG_FMT_5(PACKED64,ARGB,RED,GREEN,BLUE),
	IMG_FMT_XBGR64		=	IMG_FMT_5(PACKED64,ABGR,RED,GREEN,BLUE),

	IMG_FMT_GREY8		=	IMG_FMT_2(PACKED8,GREY),
	IMG_FMT_GREY16		=	IMG_FMT_2(PACKED16,GREY),

	IMG_FMT_YUV420P		=	IMG_FMT_5(420P,YCBCRA,Y,CB,CR),

	// compressed formats
	IMG_FMT_DXT1  		=	IMG_FMT_4(DXT1,RED,GREEN,BLUE),
	IMG_FMT_DXT3 		= 	IMG_FMT_5(DXT3,RED,GREEN,BLUE,ALPHA),
	IMG_FMT_DXT5 		= 	IMG_FMT_5(DXT5,RED,GREEN,BLUE,ALPHA),

	// compressed formats with pre-multiplied alpha
	IMG_FMT_DXT4 		= 	IMG_FMT_6(DXT5,RED,GREEN,BLUE,ALPHA,PMA),
	IMG_FMT_DXT2 		= 	IMG_FMT_6(DXT3,RED,GREEN,BLUE,ALPHA,PMA),
};


#endif /*** _RH_TEXTURE_INTERNAL_H ***/



