
#include "opaque_stencil.hpp"
#include<libimg.h>
#include<string.h>

/***************
 * Opaque images can use this generated stencil when rendering
 * with alpha enabled on native pixel formats that don't
 * support alpha. (DXT1 / ETC1).
 */
bool create_opaque_stencil(const char * resources) {

	imgImage  * whiteBox = nullptr;
	bool success = false;
	if(imgAllocImage(&whiteBox) == 0) {
		whiteBox->format = IMG_FMT_RGB24;
		whiteBox->width =  16;
		whiteBox->height = 16;
		if(imgAllocPixelBuffers(whiteBox)==0) {

			memset(whiteBox->data.channel[0], 0xFF, whiteBox->linearsize[0]);
			if(imgWriteFileF(whiteBox,"%s/.___opaque_stencil___.png", resources) == 0)
				success = true;
		}
	}
	imgFreeAll(whiteBox);
	return success;
}

