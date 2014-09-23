rh_texture_pack
======

A Texture atlas / texture loading tool:
Packer program arranges textures into an array of atlases.
Supports dithering when downsampling,
A variety of texture formats, including DXT1,3,5, ETC1.
File is compressed using LZ4-hc.

Loader program supports loading textures on OpenGL, OpenGLES.
On Android, textures can be loaded from the filesystem, or from an Asset.
The library handles all texture creation, and texture coordinates.

I plan on adding DirectX support at some point...

