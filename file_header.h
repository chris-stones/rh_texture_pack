
#pragma once

struct rhtpak_hdr {
  
  char magic[16]; 		// "rockhopper.tpak"
  unsigned int version; 	// impl version
  unsigned int flags;		// flags
  unsigned int format;		// format if constant
  unsigned int w;		// width if constant
  unsigned int h;		// height if constant
  unsigned int depth;		// number of textures
  unsigned int resources;	// number of resources ( sprites )
  unsigned int text_data_ptr;	// pointer to rhtpak_hdr_tex_data. ( array length is rhtpak_hdr::depth )
  unsigned int hash_data_ptr;	// pointer to rhtpak_hdr_hash ( array length is rhtpak_hdr::resources )
};

struct rhtpak_hdr_tex_data {
  
  unsigned int x;			// x offset of this frame.
  unsigned int y;			// y offset of this frame.
  unsigned int w;			// width of this layer
  unsigned int h;			// height of this layer
  unsigned int flags;			// flags ( pixel data is lz4 compressed )
  unsigned int format;			// format
  unsigned int channels;
  
  struct {
    unsigned int file_offset;		// file offset of pixel data
    unsigned int file_length;		// file length of pixel data
    unsigned int uncompressed_size;	// uncompresseed size of pixel data ( if compressed )
    
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

