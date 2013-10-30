
#ifndef __RH_TEXTURE_LOADER_H_
#define __RH_TEXTURE_LOADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define RH_TEXPAK_FILESYSTEM	0x01
#define RH_TEXPAK_ANDROID_APK	0x02
#define RH_TEXPAK_ENABLE_TEXTURE_ARRAY (1<<31) // enable the use of texture arrays ( if supported by hardware, see GL_EXT_texture_array )

#ifdef __ANDROID__
#define RH_TEXPAK_APP RH_TEXPAK_ANDROID_APK
#endif
#ifndef RH_TEXPAK_APP
#define RH_TEXPAK_APP RH_TEXPAK_FILESYSTEM
#endif

struct _texpak_type;
typedef struct _texpak_type * rh_texpak_handle;

struct _texpak_idx_type;
typedef struct _texpak_idx_type * rh_texpak_idx;

int rh_texpak_open 		(const char * gfx_file, rh_texpak_handle * loader, int flags);
int rh_texpak_load 		(rh_texpak_handle loader ); // requires opengl context.
int rh_texpak_close		(rh_texpak_handle loader);  // close IF refcount is zero. returns refcount.
int rh_texpak_forceclose(rh_texpak_handle loader);  // close and return refcount.

int rh_texpak_get		(rh_texpak_handle loader, const char * name, rh_texpak_idx * idx);
int rh_texpak_release	(rh_texpak_idx idx);


int rh_texpak_get_textures(rh_texpak_handle loader, int *texcount);
int rh_texpak_get_textarget(rh_texpak_handle loader, GLenum *target);

int rh_texpak_get_texture(rh_texpak_idx idx, GLuint *tex);
int rh_texpak_get_size(rh_texpak_idx idx, unsigned int *w, unsigned int *h);
int rh_texpak_get_depthi(rh_texpak_idx idx, unsigned int *i);
int rh_texpak_get_depthf(rh_texpak_idx idx, GLfloat *f);
int rh_texpak_get_coords(rh_texpak_idx idx, int dim, int stride ,GLfloat *coords);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif /*** __RH_TEXTURE_LOADER_H_ ***/

