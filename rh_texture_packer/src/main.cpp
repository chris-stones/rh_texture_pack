
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
#include <dirent.h>
#include <exception>

#include "args.h"
#include "hash.hpp"
#include "lz4.h"
#include "lz4hc.h"

#include "file_header.h"

typedef std::string ExtraContent;

// Check for collisions due to case-insensitive file systems, and same-name-different-file-extensions.
void CheckForCollisions( const std::vector<std::string> &allFiles, const char * res_root ) {

  std::vector<std::string>::const_iterator itor = allFiles.begin();
  std::vector<std::string>::const_iterator end  = allFiles.end();

  std::map<std::string, int> colmap;

  int collisions = 0;

  while(itor != end) {

    std::string gameresname = get_game_resource_name( *itor, "", res_root );

    int count = ++colmap[ gameresname ];

    if(count != 1) {

      printf("RESOURCE NAME COLLISION: %s\n", gameresname.c_str());
      collisions++;
    }

    itor++;
  }

  assert(collisions==0);
}

int main(int argc, char ** argv) {

  arguments args = read_args(argc,argv);

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

  CheckForCollisions( uniqueImages.GetAll(), args.resources );

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

    if(content.coord.z >= dst_images.size())
    	dst_images.resize(content.coord.z+1);

    if(!dst_images[content.coord.z]) {

      imgAllocImage(&dst_images[content.coord.z]);
      dst_images[content.coord.z]->format = IMG_FMT_FLOAT_RGBA; // error diffusion requires floating colour.
      dst_images[content.coord.z]->width = args.width;
      dst_images[content.coord.z]->height = args.height;
      imgAllocPixelBuffers(dst_images[content.coord.z]);
      layers++;
    }

    imgImage * src_image = NULL;

	if(args.debug)
		printf("%s\n", content.content.c_str());

    imgAllocAndRead(&src_image, content.content.c_str());

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

   if((layers < dst_images.size()) && dst_images[i]) {

     if(args.debug)
      imgWriteFileF(dst_images[i], "rh_tex_pack_out_layer%02d.png", i);

     // convert to output native pixel format
     imguCopyImage(native_image, dst_images[i]);

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

