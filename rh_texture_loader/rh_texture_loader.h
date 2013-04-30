
#pragma once

struct gfx_loader_type;
typedef struct gfx_loader_type * gfx_loader_handle;

struct gfx_sprite {

  GLuint texture;
  int	pwidth;
  int	pheight;

  struct {

    GLfloat s;
    GLfloat t;
    GLfloat q;

  } tcoords[4];
};

int gfx_loader_open (const char * gfx_file, gfx_loader_handle * loader);
int gfx_loader_close(gfx_loader_handle loader);
int gfx_loader_getsprite(gfx_loader_handle loader, const char * name, struct gfx_sprite *sprite);
