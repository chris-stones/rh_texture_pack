

#include "rh_texture_internal.h"

static int __report_gl_err(const char * file, const char * func, int line) {

	GLenum e;
	int ecount = 0;

	while( (e = glGetError()) != GL_NO_ERROR ) {

		ecount++;
		LOGE("%s:%s:%d gl error %d", file, func, line, e);
	}

	return ecount;
}
#define GL_ERROR() __report_gl_err(__FILE__,__FUNCTION__,__LINE__)

/* code stolen from http://www.opengl.org/archives/resources/features/OGLextensions/ */
static int IsExtensionSupported(const char *extension)
{
  const GLubyte *extensions = NULL;
  const GLubyte *start;
  GLubyte *where, *terminator;

  /* Extension names should not have spaces. */
  where = (GLubyte *) strchr(extension, ' ');
  if (where || *extension == '\0')
    return 0;
  extensions = glGetString(GL_EXTENSIONS);
  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
  start = extensions;
  for (;;) {
    where = (GLubyte *) strstr((const char *) start, extension);
    if (!where)
      break;
    terminator = where + strlen(extension);
    if (where == start || *(where - 1) == ' ')
      if (*terminator == ' ' || *terminator == '\0')
        return 1;
    start = terminator;
  }
  return 0;
}

static GLenum get_gl_compression_enum(int libimg_format) {

	if( (libimg_format & IMG_FMT_COMPONENT_DXT1) == IMG_FMT_COMPONENT_DXT1 )
		return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

	if( (libimg_format & IMG_FMT_COMPONENT_DXT3) == IMG_FMT_COMPONENT_DXT3 )
		return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;

	if( (libimg_format & IMG_FMT_COMPONENT_DXT5) == IMG_FMT_COMPONENT_DXT5 )
		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

	return -1;
}

static GLint get_uncompressed_internal_format(int libimg_format) {

	if( (libimg_format & IMG_FMT_RGB24) == IMG_FMT_RGB24 )
		return GL_RGB;

	if( (libimg_format & IMG_FMT_RGBA32) == IMG_FMT_RGBA32 )
		return GL_RGBA;

	if( (libimg_format & IMG_FMT_RGBA16) == IMG_FMT_RGBA16 )
		return GL_RGBA;

	// other supported formats are GL_ALPHA, GL_LUMINANCE and GL_LUMINANCE_ALPHA

	return -1;
}

static GLint get_uncompressed_datatype(int libimg_format) {

	int mask_4444 = (IMG_FMT_COMPONENT_PACKED16 );

	if( ( libimg_format & mask_4444) == mask_4444 )
		GL_UNSIGNED_SHORT_4_4_4_4;

	return GL_UNSIGNED_BYTE;
};

static int allocate_texture_array_memory(const struct rhtpak_hdr *header, GLenum target) {

	if( header->format & IMG_FMT_COMPONENT_COMPRESSED) {

		GLenum format = get_gl_compression_enum( header->format );

		GLsizei imageSize = ((header->w+3)/4) * ((header->h+3)/4) * header->depth;

		if( header->format & IMG_FMT_COMPONENT_DXT1 )
			imageSize *= 8;
		else
			imageSize *= 16;


		GL_COMPRESSED_TEX_IMAGE_3D(
			target, 		// TARGET
			0, 			// LEVEL
			format,			// INTERNAL FORMAT
			header->w,		// WIDTH
			header->h, 		// HEIGHT
			header->depth,		// DEPTH
			0, 			// BORDER
			imageSize,		// IMAGE SIZE
			NULL 			// DATA
		);


		return 0;

	} else {

		GLint internal_format = get_uncompressed_internal_format( header->format );
		GLint data_type       = get_uncompressed_datatype( header->format );

		GL_TEX_IMAGE_3D(
			target,			// TARGET
			0,			// LEVEL
			internal_format,	// INTERNAL FORMAT,
			header->w,		// WIDTH
			header->h, 		// HEIGHT
			header->depth,		// DEPTH
			0, 			// BORDER
			internal_format,	// FORMAT
			data_type,	// TYPE
			NULL			// PIXELS
		);

		return 0;
	}

	return -1;
}

static int allocate_texture_memory(const struct rhtpak_hdr *header, GLenum target) {

	if( header->format & IMG_FMT_COMPONENT_COMPRESSED) {

		GLenum format = get_gl_compression_enum( header->format );

		GLsizei imageSize = ((header->w+3)/4) * ((header->h+3)/4) * header->depth;

		if( header->format & IMG_FMT_COMPONENT_DXT1 )
			imageSize *= 8;
		else
			imageSize *= 16;

		glCompressedTexImage2D(
			target, 	// TARGET
			0, 		// LEVEL
			format,		// INTERNAL FORMAT
			header->w,	// WIDTH
			header->h, 	// HEIGHT
			0, 		// BORDER
			imageSize,	// IMAGE SIZE
			NULL 		// DATA
		);

		return 0;

	} else {

		GLint internal_format = get_uncompressed_internal_format( header->format );
		GLint data_type       = get_uncompressed_datatype( header->format );

		glTexImage2D(
			target,			// TARGET
			0,			// LEVEL
			internal_format,	// INTERNAL FORMAT,
			header->w,		// WIDTH
			header->h, 		// HEIGHT
			0, 			// BORDER
			internal_format,	// FORMAT
			data_type,	// TYPE
			NULL			// PIXELS
		);

		return 0;
	}

	return -1;
}

static int load_texture_array_data( const struct rhtpak_hdr_tex_data *tex_data, int i, const void * data, int data_len, GLenum target) {

	if( tex_data->format & IMG_FMT_COMPONENT_COMPRESSED) {

		GLenum format = get_gl_compression_enum( tex_data[i].format );

		GL_COMPRESSED_TEX_SUBIMAGE_3D(
				target,		// TARGET
				0,		// LEVEL
				0,		// X OFFSET
				0,		// Y OFFSET
				i,		// Z OFFSET
				tex_data[i].w,	// WIDTH
				tex_data[i].h,	// HEIGHT
				1,		// DEPTH
				format,		// FORMAT
				data_len,	// SIZE
				data		// DATA
		);

		return 0;

	} else {

		GLint internal_format = get_uncompressed_internal_format( tex_data[i].format );
		GLint data_type       = get_uncompressed_datatype( tex_data[i].format );

		GL_TEX_SUMBIMAGE_3D(
				target,			// TARGET
				0,			// LEVEL
				0,			// X OFFSET
				0,			// Y OFFSET
				i,			// Z OFFSET
				tex_data[i].w,		// WIDTH
				tex_data[i].h,		// HEIGHT
				1,			// DEPTH
				internal_format,	// FORMAT
				data_type,	// TYPE
				data			// DATA
		);

		return 0;
	}


	return -1;
}

static int load_texture_data( const struct rhtpak_hdr_tex_data *tex_data, int i, const void * data, int data_len, GLenum target) {

	if( tex_data->format & IMG_FMT_COMPONENT_COMPRESSED) {

		GLenum format = get_gl_compression_enum( tex_data[i].format );

		glCompressedTexSubImage2D(
				target,			// TARGET
				0,			// LEVEL
				0,			// X OFFSET
				0,			// Y OFFSET
				tex_data[i].w,		// WIDTH
				tex_data[i].h,		// HEIGHT
				format,			// FORMAT
				data_len,		// SIZE
				data			// DATA
		);

		return 0;

	} else {

		GLint internal_format = get_uncompressed_internal_format( tex_data[i].format );
		GLint data_type       = get_uncompressed_datatype( tex_data[i].format );

		glTexSubImage2D(
				target,			// TARGET
				0,			// LEVEL
				0,			// X OFFSET
				0,			// Y OFFSET
				tex_data[i].w,		// WIDTH
				tex_data[i].h,		// HEIGHT
				internal_format,	// FORMAT
				data_type,	// TYPE
				data			// DATA
		);

		return 0;
	}


	return -1;
}

static int _setup_access_functionptrs(rh_texpak_handle  _loader) {

#ifdef __ANDROID__
	if(loader->flags & RH_RAWPAK_ANDROID_APK) {

		_setup_file_android(&_loader->file);
		return 0;
	}
#endif

	_setup_file_filesystem(&_loader->file);
	return 0;
}

int rh_texpak_open (const char * gfx_file, rh_texpak_handle * loader_out, int flags) {

  struct _texpak_type * loader = NULL;

  if(!(loader = (struct _texpak_type *)calloc(1, sizeof(struct _texpak_type) ))) {

	  LOGE("%s - cant alloc struct\n", __FUNCTION__);
	  goto err;
  }

  loader->flags = flags;

  _setup_access_functionptrs(loader);

  RHF_GETMGR(loader->file);

  if(RHF_OPEN(loader->file, gfx_file) != 0) {

	  LOGE("%s - cant open %s\n", __FUNCTION__, gfx_file );
	  goto err;
  }

  if( RHF_READ(loader->file, &loader->header, sizeof loader->header) != sizeof loader->header ) {

	  LOGE("%s - cant read header %s\n", __FUNCTION__, gfx_file );
	  goto err;
  }

  loader->hash_length = loader->header.resources;
  loader->seed = loader->header.seed;

  if(RHF_SEEK(loader->file, loader->header.hash_data_ptr , SEEK_SET) != 0) {
	LOGE("%s - seek error %s\n", __FUNCTION__, gfx_file);
    goto err;
  }

  if(!(loader->hash = (struct rhtpak_hdr_hash *)malloc(loader->hash_length * sizeof(struct rhtpak_hdr_hash)))) {

	LOGE("%s - cant allocate rhtpak_hdr_hash %s\n", __FUNCTION__, gfx_file );
    goto err;
  }

  if( RHF_READ(loader->file, loader->hash, loader->header.resources * sizeof(struct rhtpak_hdr_hash)) != loader->header.resources * sizeof(struct rhtpak_hdr_hash)) {

    LOGE("%s - cant read rhtpak_hdr_hash %s\n", __FUNCTION__, gfx_file );
    goto err;
  }

  *loader_out = loader;

  return 0;

err:

  if(loader) {
    free(loader->hash);
    RHF_CLOSE(loader->file);
    free(loader);
  }

  return -1;
}

int rh_texpak_load ( rh_texpak_handle loader ) {

	unsigned int uncompressed_buffer_size = 0;
	void * uncompressed_buffer = 0;
	unsigned int compressed_buffer_size = 0;
	void * compressed_buffer = NULL;

	struct rhtpak_hdr_tex_data *tex_data = NULL;

	GLenum compressed_tex_format = -1;
	int i;

	if(RHF_SEEK(loader->file, loader->header.text_data_ptr, SEEK_SET) < 0) {
	  LOGE("%s - seek error\n", __FUNCTION__);
	  goto err;
	}

	if(!(tex_data = (struct rhtpak_hdr_tex_data *)malloc(loader->header.depth * sizeof(struct rhtpak_hdr_tex_data)))) {

		LOGE("%s - cant allocate rhtpak_hdr_tex_data\n", __FUNCTION__ );
		goto err;
	}

	if( RHF_READ(loader->file, tex_data, loader->header.depth * sizeof(struct rhtpak_hdr_tex_data)) != loader->header.depth * sizeof(struct rhtpak_hdr_tex_data)) {

		LOGE("%s - cant read rhtpak_hdr_tex_data\n", __FUNCTION__ );
		goto err;
	}

	compressed_tex_format = get_gl_compression_enum( loader->header.format );

	loader->target = GL_TEXTURE_2D;

	if((loader->header.depth > 1) && (loader->flags & RH_TEXPAK_ENABLE_TEXTURE_ARRAY) && IsExtensionSupported("GL_EXT_texture_array"))
		loader->target = GL_TEXTURE_2D_ARRAY_EXT;

	if(loader->target == GL_TEXTURE_2D_ARRAY_EXT) {
		loader->textures_length = 1;
		LOGI("using GL_TEXTURE_2D_ARRAY_EXT\n");
	} else {
		loader->textures_length = loader->header.depth;
		LOGI("using GL_TEXTURE_2D\n");
	}

	loader->textures = (GLuint*)calloc(1, sizeof(GLuint) * loader->textures_length );
	glGenTextures(loader->textures_length, loader->textures);
	glActiveTexture(GL_TEXTURE0);
	{
		int i;
		for(i=0;i<loader->textures_length;i++) {
			glBindTexture( loader->target, loader->textures[i]);
			glTexParameteri( loader->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri( loader->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri( loader->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri( loader->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			if(loader->target == GL_TEXTURE_2D_ARRAY_EXT) {
				if( allocate_texture_array_memory( &loader->header, loader->target ) != 0) {
				  LOGE("%s - cannot allocate texture array memory.\n", __FUNCTION__);
				  goto err;
				}
			} else {
				if(allocate_texture_memory( &loader->header, loader->target ) != 0) {
				  LOGE("%s - cannot allocate texture memory.\n", __FUNCTION__);
				  goto err;
				}
			}
		}
	}

	for(i=0;i<loader->header.depth;i++) {

	  if(RHF_SEEK(loader->file, tex_data[i].channel[0].file_offset, SEEK_SET) < 0) {
	    LOGE("%s - seek error\n", __FUNCTION__);
	    goto err;
	  }

		if( tex_data[i].channel[0].file_length > compressed_buffer_size) {

			compressed_buffer_size = 0;
			free( compressed_buffer );
			if((compressed_buffer = malloc(tex_data[i].channel[0].file_length)))
				compressed_buffer_size = tex_data[i].channel[0].file_length;
		}

		if( tex_data[i].channel[0].uncompressed_size > uncompressed_buffer_size) {

			uncompressed_buffer_size = 0;
			free( uncompressed_buffer );
			if((uncompressed_buffer = malloc(tex_data[i].channel[0].uncompressed_size)))
				uncompressed_buffer_size = tex_data[i].channel[0].uncompressed_size;
		}

		if(uncompressed_buffer && compressed_buffer) {

			int lz4err = 0;

			unsigned int csize  = tex_data[i].channel[0].file_length;
			unsigned int ucsize = tex_data[i].channel[0].uncompressed_size;

			if( RHF_READ(loader->file, compressed_buffer, csize) != csize) {

				LOGE("%s - can't read compressed_buffer (%d bytes)\n", __FUNCTION__, csize );
				goto err;
			}

			if( (lz4err = LZ4_uncompress( (const char *)compressed_buffer, (char*)uncompressed_buffer, ucsize ) ) < 0 ) {

				LOGE("%s - can't decompress buffer\n", __FUNCTION__ );
				LOGE("  LZ4_uncompress(%p,%p,%d) == %d\n", compressed_buffer, uncompressed_buffer, csize, lz4err);
				goto err;
			}

			if(loader->target == GL_TEXTURE_2D_ARRAY_EXT) {
				if(load_texture_array_data( tex_data, i, uncompressed_buffer, ucsize, loader->target  ) != 0) {

					LOGE("%s - can't load texture array\n", __FUNCTION__ );
					goto err;
				}
			}
			else {

				glBindTexture( loader->target, loader->textures[i]);

				if(load_texture_data( tex_data, i, uncompressed_buffer, ucsize, loader->target  ) != 0) {
					LOGE("%s - can't load texture data\n", __FUNCTION__ );
					goto err;
				}
			}
		}
		else {
		  LOGE("%s - can't allocate buffers 0%p, %p\n", __FUNCTION__, uncompressed_buffer, compressed_buffer );
		  goto err;
		}
	}

	free(tex_data);
	tex_data = NULL;
	free(uncompressed_buffer);
	uncompressed_buffer = NULL;
	free(compressed_buffer);
	compressed_buffer = NULL;
	RHF_CLOSE(loader->file);

	if(GL_ERROR() == 0) {
	  return 0;
	}
	else {
	  LOGE("%s - GL Error\n", __FUNCTION__ );
	}

err:
	LOGE("%s exiting with error\n", __FUNCTION__ );

	GL_ERROR();

	if(loader) {

	  RHF_CLOSE(loader->file);

	  if(loader->textures)
		  glDeleteTextures(loader->textures_length, loader->textures);
	  free(loader->textures);
	  loader->textures = NULL;
	}

	free(tex_data);
	free(uncompressed_buffer);
	free(compressed_buffer);

	return -1;
}

// closes the pak.
// returns the number of still open references.
int rh_texpak_forceclose(rh_texpak_handle loader) {

	int err = 0;
	if(loader) {
		err = loader->refcount;
		free(loader->hash);
		if(loader->textures)
			glDeleteTextures(loader->textures_length, loader->textures);
		free(loader->textures);
		RHF_CLOSE(loader->file);
		free(loader);
	}
	return err;
}

// If the reference count is zero, closes the exture pak and returns zero.
// If the reference count is NOT zero, returns the number of still open references WITHOUT closing the pak.
int rh_texpak_close(rh_texpak_handle loader) {

	int err = 0;

	if(loader)
		if(!(err = loader->refcount))
			rh_texpak_forceclose(loader);

	return err;
}

static unsigned int hash( const char* _s, unsigned int seed)
{
    const char * s = _s;
    unsigned int hash = seed;
    int len = strlen(s);
    int i;

    while(len) {
    	len--;
    	if(s[len]=='.')
    		break;
    }

    for(i=0;i<len;i++)
    {
    	char c = *s++;

    	switch(c) {
    	case '/':
    	case '\\':
    		c = '.';
    		break;
    	default:
    		c = tolower(c);
    		break;
    	}

    	// THANKS PAUL LARSON.
        hash = hash * 101 + c;
    }

    return hash;
}

static int compare_hash(const void * key, const void * memb) {

	const unsigned int    * k = (const unsigned int *)key;
	const struct rhtpak_hdr_hash * m = (const struct rhtpak_hdr_hash*)memb;

	if( *k < m->hash )
		return -1;
	if( *k > m->hash )
		return 1;
	return 0;
}

int rh_texpak_release(rh_texpak_idx idx) {

	if(idx) {
		idx->pak->refcount--;
		free(idx);
		return 0;
	}
	return -1;
}

int rh_texpak_get(rh_texpak_handle loader, const char * name, rh_texpak_idx * idx) {

  if(loader && idx && name) {

	unsigned int key = hash( name, loader->seed );

	struct rhtpak_hdr_hash * res = (struct rhtpak_hdr_hash *)bsearch(&key, loader->hash, loader->hash_length, sizeof(struct rhtpak_hdr_hash ), &compare_hash);

	if(res) {

		rh_texpak_idx i = (rh_texpak_idx)(res);
		rh_texpak_idx b = (rh_texpak_idx)(loader->hash);

		if((*idx = calloc(1, sizeof(struct _texpak_idx_type)))) {

		 (*idx)->index = (i-b)/sizeof(struct rhtpak_hdr_hash );
		 (*idx)->pak = loader;

		 loader->refcount++;

		  return 0;
		}
	}
  }
  return -1;
}

int rh_texpak_get_texture(rh_texpak_idx idx, GLuint *tex) {

  *tex = idx->pak->textures[ idx->pak->hash[idx->index].i ];

  return 0;
}

int rh_texpak_get_size(rh_texpak_idx idx, unsigned int *w, unsigned int *h) {

  if(w) *w = idx->pak->hash[idx->index].orig_w;
  if(h) *h = idx->pak->hash[idx->index].orig_h;

  return 0;
}

int rh_texpak_get_depthi(rh_texpak_idx idx, unsigned int *i) {

  *i = idx->pak->hash[idx->index].i;
  return 0;
}

int rh_texpak_get_depthf(rh_texpak_idx idx, GLfloat *f) {

  *f = idx->pak->hash[idx->index].tex_coords[0].p;
  return 0;
}

int rh_texpak_get_coords(rh_texpak_idx idx, int dim, int stride ,GLfloat *coords) {

  int c;
  int d;

  float * s = &(idx->pak->hash[idx->index].tex_coords[0].s);

  // same format? just memcpy it.
  if(stride==3 && d==3) {
	memcpy(coords,s, 3*3*sizeof(float));
	return 0;
  }

  // dim will be 2 or 3, let the compiler know so that it cam optimise the h3ll out of it.
  if(dim==2) {
	for(c=0;c<4;c++)
      for(d=0;d<2;d++)
        coords[c*stride+d] = s[c*3+d];
	 return 0;
  }
  if(dim==3) {
	for(c=0;c<4;c++)
      for(d=0;d<3;d++)
        coords[c*stride+d] = s[c*3+d];
	 return 0;
  }
  // wont happen!
  for(c=0;c<4;c++)
    for(d=0;d<dim;d++)
      coords[c*stride+d] = s[c*3+d];

  return 0;
}

int rh_texpak_get_textures(rh_texpak_handle loader, int *texcount) {

  *texcount = loader->textures_length;
  return 0;
}

int rh_texpak_get_textarget(rh_texpak_handle loader, GLenum *target) {

  *target = loader->target;
  return 0;
}


