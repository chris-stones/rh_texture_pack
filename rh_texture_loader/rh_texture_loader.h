
#ifndef __RH_TEXTURE_LOADER_H_
#define __RH_TEXTURE_LOADER_H_

#ifdef __cplusplus
extern "C" {
#endif

struct _texpak_type;
typedef struct _texpak_type * rh_texpak_handle;

typedef size_t rh_texpak_idx;

int rh_texpak_open (const char * gfx_file, rh_texpak_handle * loader);
int rh_texpak_load ( rh_texpak_handle loader ); // requires opengl context.
int rh_texpak_lookup(rh_texpak_handle loader, const char * name, rh_texpak_idx * idx);
int rh_texpak_close(rh_texpak_handle loader);

int rh_texpak_get_texture(rh_texpak_handle loader, rh_texpak_idx idx, GLuint *tex);
int rh_texpak_get_size(rh_texpak_handle loader, rh_texpak_idx idx, unsigned int *w, unsigned int *h);
int rh_texpak_get_depthi(rh_texpak_handle loader, rh_texpak_idx idx, unsigned int *i);
int rh_texpak_get_depthf(rh_texpak_handle loader, rh_texpak_idx idx, GLfloat *f);
int rh_texpak_get_coords(rh_texpak_handle loader, rh_texpak_idx idx, int dim, int stride ,GLfloat *coords);
int rh_texpak_get_textures(rh_texpak_handle loader, int *texcount);
int rh_texpak_get_textarget(rh_texpak_handle loader, GLenum *target);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif /*** __RH_TEXTURE_LOADER_H_ ***/

