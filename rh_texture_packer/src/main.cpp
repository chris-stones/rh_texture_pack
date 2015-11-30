
#include "binpack2d.hpp"
#include "Pack.hpp"
#include "CreateSpriteMap.hpp"
#include "FindContent.hpp"
#include "Output.hpp"
#include <libimgutil.h>

#include<map>
#include<string>
#include<vector>
#include<cstdlib>
#include<algorithm>

#include <sys/types.h>
//#include <dirent.h>
#include <exception>

#include "args.h"
#include "hash.hpp"
#include "lz4.h"
#include "lz4hc.h"

#include "file_header.h"

#include "opaque_stencil.hpp"

typedef Path::Image ExtraContent;

int main(int argc, char ** argv) {

  arguments args = read_args(argc,argv);

  if(!(args.format & IMG_FMT_COMPONENT_ALPHA))
	  create_opaque_stencil(args.resources);

  BinPack2D::CanvasArray<ExtraContent> canvasArray =
    BinPack2D::UniformCanvasArrayBuilder<ExtraContent>(args.width,args.height,args.depth).Build();

  BinPack2D::ContentAccumulator<ExtraContent> inputContent;
  BinPack2D::ContentAccumulator<ExtraContent> outputContent;

  // Recursively scan path for images.
  Path::Directory dir(args);

  // a record unique, and alias images.
  UniqueImages uniqueImages(args);

  // pack images found above into our bin
  Pack(dir, inputContent, uniqueImages, args.pad );

  inputContent.Sort( );

  // try to pack content into canvas.
  bool success = canvasArray.Place( inputContent );

  if(!success) {
    printf("OUT OF SPACE! Cannot place all content!\n");
    return -1;
  }

  canvasArray.CollectContent( outputContent );

  BinPack2D::Content<ExtraContent>::Vector::iterator itor = outputContent.Get().begin();
  BinPack2D::Content<ExtraContent>::Vector::iterator end  = outputContent.Get().end();

  int layers = 0;

  std::vector<imgImage*> dst_images;

  while(itor != end) {

    const BinPack2D::Content<ExtraContent> &content = *itor;

    if(content.coord.z >= (int)dst_images.size())
    	dst_images.resize(content.coord.z+1);

    if(!dst_images[content.coord.z]) {

      imgAllocImage(&dst_images[content.coord.z]);
      dst_images[content.coord.z]->format = IMG_FMT_FLOAT_RGBA; // error diffusion requires floating colour.
      dst_images[content.coord.z]->width = args.width;
      dst_images[content.coord.z]->height = args.height;
      imgAllocPixelBuffers(dst_images[content.coord.z]);
      layers++;

      if(args.debug)
    	  printf("adding a layer %d\n", layers);
    }

    imgImage * src_image = NULL;

	if(args.debug)
		printf("%s\n", content.content.GetFileName().c_str());

    imgAllocAndRead(&src_image, content.content.GetFileName().c_str());

    if(args.pad) {

      imgImage * padded_src_image;
      imguPad(&padded_src_image, src_image, args.pad, args.pad, args.pad, args.pad);
      imgFreeAll(src_image);
      src_image = padded_src_image;
    }

    if(content.rotated)
    {
      // BinPacker decided to rotate this image!
      imgImage * rotatedImage = NULL;
      imguRotateCW( &rotatedImage, src_image );
      imgFreeAll(src_image);
      src_image = rotatedImage;
    }

    if(content.content.GetSourceDataType() == SourceData::Alpha) {

    	// We are only interested in the alpha channel!

    	// Make sure source image is in good-old RGBA32 format.
    	if(src_image->format != IMG_FMT_RGBA32) {
    		imgImage * rgba32_image = NULL;
    		imgAllocImage(&rgba32_image);
    		rgba32_image->format = IMG_FMT_RGBA32;
    		rgba32_image->width  = src_image->width;
    		rgba32_image->height = src_image->height;
    		imgAllocPixelBuffers(rgba32_image);
    		imgFreeAll(src_image);
    		src_image = rgba32_image;
    	}

    	// Copy Alpha channel to Red, Green and Blue. Set Alpha to opaque.
    	unsigned char * rgba_data = static_cast<unsigned char *>(src_image->data.channel[0]);
    	for(unsigned int offset=0;offset<(unsigned int)src_image->linearsize[0];offset+=4) {
    		rgba_data[offset+0] =
    		rgba_data[offset+1] =
    		rgba_data[offset+2] =
    		rgba_data[offset+3] ;
    		rgba_data[offset+3] = 0xFF;
    	}
    }

    imguCopyRect(dst_images[content.coord.z], src_image, content.coord.x, content.coord.y, 0, 0, src_image->width, src_image->height);

	if(args.edk != ERR_DIFFUSE_KERNEL_NONE) {

		imguErrorDiffuseArea( dst_images[content.coord.z],
			content.coord.x, content.coord.y,
			src_image->width, src_image->height,
			args.edk_precision, args.edk );
	}

    imgFreeAll(src_image);

    itor++;
  }

  std::string output_filename = args.output_file;

  struct imgImage * native_image;
  imgAllocImage(&native_image);
  native_image->width = args.width;
  native_image->height = args.height;
  native_image->format = (imgFormat)args.format;

  imgAllocPixelBuffers(native_image);

  unsigned int seed = 0;

  std::map< unsigned int, rhtpak_hdr_hash > spriteMap =
    CreateSpriteMap(outputContent, uniqueImages.GetAliasMap(), args, &seed);

  Output outputFile( args, args.output_file, spriteMap.size(), args.width, args.height, layers, native_image->format );

  for(int i=0;i<layers;i++) {

   if((i < (int)dst_images.size()) && dst_images[i]) {

     if(args.debug) {

    	 imgImage *pngImage = NULL;
    	 imgAllocImage(&pngImage);
    	 pngImage->format = IMG_FMT_RGBA32;
    	 pngImage->width = dst_images[i]->width;
    	 pngImage->height = dst_images[i]->height;
    	 imgAllocPixelBuffers(pngImage);
    	 imguCopyImage(pngImage, dst_images[i]);
    	 printf("WRITING rh_tex_pack_out_layer%02d.png\n", i);
    	 imgWriteFileF(pngImage, "rh_tex_pack_out_layer%02d.png", i);
    	 imgFreeAll(pngImage);
     }

     // convert to output native pixel format
     printf("Creating layer %d...\n", i);


     copy_quality_t quality;
     switch(args.quality) {
     default:
     case 0:
       quality = COPY_QUALITY_HIGHEST;
       break;
     case 1:
       quality = COPY_QUALITY_MEDIUM;
       break;
     case 2:
       quality = COPY_QUALITY_LOWEST;
       break;
     }

     // TODO: split into multiple-threads for compressed textures.
     imguCopyImage3(native_image, dst_images[i], ERR_DIFFUSE_KERNEL_DEFAULT,quality);

     imgFreeAll(dst_images[i]);
     dst_images[i] = NULL;

     // write it!
     outputFile.WriteLayer( native_image );
   }
  }

  outputFile.WriteHashMap( seed, spriteMap );

  imgFreeAll(native_image);

  return 0;
}

