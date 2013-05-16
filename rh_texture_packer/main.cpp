

#include<libimgutil.h>
#include<binpack2d.hpp>

#include<map>
#include<string>
#include<vector>
#include<cstdlib>
#include<algorithm>

#include <sys/types.h>
#include <dirent.h>
#include <exception>

#include "config.h"
#include "hash.h"
#include "lz4.h"
#include "lz4hc.h"

#include "CreateSpriteMap.hpp"
#include "FindContent.hpp"
#include "Pack.hpp"
#include "Output.hpp"

#include "file_header.h"  

typedef std::string ExtraContent;

void CheckForCollisions( BinPack2D::ContentAccumulator<ExtraContent> contentAcc, const char * res_root ) {
  
  BinPack2D::Content<ExtraContent>::Vector::iterator itor = contentAcc.Get().begin();
  BinPack2D::Content<ExtraContent>::Vector::iterator end  = contentAcc.Get().end();
  
  std::map<std::string, int> colmap;
  
  int collisions = 0;
  
  while(itor != end) {
   
    std::string gameresname = get_game_resource_name( (*itor).content  ,res_root );
    
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
    
  // recursivly scan path for images.
  Path::Directory dir(args.resources);
  
  // pack images found above into our bin
  Pack(dir, inputContent, args.pad );
  
  CheckForCollisions( inputContent, args.resources );
  
  inputContent.Sort( );
  
  // try to pack content into convas.
  bool success = canvasArray.Place( inputContent );
  
  if(!success) {
    printf("OUT OF SPACE! Cannot place all content!\n");
    return -1;
  }
  
  canvasArray.CollectContent( outputContent );
  
  BinPack2D::Content<ExtraContent>::Vector::iterator itor = outputContent.Get().begin();
  BinPack2D::Content<ExtraContent>::Vector::iterator end  = outputContent.Get().end();
  
  int layers = 0;
  
  imgImage* dst_images[64];
  memset(dst_images, 0, sizeof dst_images);
  
  while(itor != end) {
   
    const BinPack2D::Content<ExtraContent> &content = *itor;
       
    if(!dst_images[content.coord.z]) {
     
      imgAllocImage(dst_images + content.coord.z);
      dst_images[content.coord.z]->format = IMG_FMT_RGBA32;
      dst_images[content.coord.z]->width = args.width;
      dst_images[content.coord.z]->height = args.height;
      imgAllocPixelBuffers(dst_images[content.coord.z]);
      layers++;
    }
    
    imgImage * src_image = NULL;
    
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
  
  Output outputFile( args.output_file, outputContent.Get().size(), args.width, args.height, layers, native_image->format );
  
  for(int i=0;i<64;i++) {
    
   if(dst_images[i]) {

     if(args.debug)
      imgWriteFileF(dst_images[i], "rh_tex_pack_out_layer%02d.png", i);
     
     // convert to output native pixel format
     imguCopyImage(native_image, dst_images[i]);
     
     // write it!
     outputFile.WriteLayer( native_image );
   }
  }
  
  unsigned int seed = 0;
  
  std::map< unsigned int, rhtpak_hdr_hash > spriteMap = 
    CreateSpriteMap(outputContent, args.resources, args.width, args.height, args.pad, &seed);
  
  outputFile.WriteHashMap( seed, spriteMap );
    
  imgFreeAll(native_image);
 
  return 0;
}

