//
// Created by consti10 on 15.09.22.
//

#ifndef HELLO_DRMPRIME__COLOR_HELPER_H_
#define HELLO_DRMPRIME__COLOR_HELPER_H_

#include <cstring>
#include <cstdint>

static uint32_t create_pixel_rgba(uint8_t r,uint8_t g,uint8_t b,uint8_t a){
#ifdef __windows__
    return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
#else
    uint8_t rgba[4];
    rgba[0]=r;
    rgba[1]=g;
    rgba[2]=b;
    rgba[3]=a;
    uint32_t ret;
    memcpy(&ret,rgba,4);
    return ret;
#endif


}

// --------------------------------------------------- from drm-howto ---------------------------------------------------
// returns fully red,green,blue color alternating and wrapping around
static uint32_t createColor(const int idx,const uint8_t alpha=50){
  uint8_t r,g,b;
  int colorIdx= idx % 3;
  if(colorIdx==0){
	r=255;
	g=0;
	b=0;
  }else if(colorIdx==1){
	r=0;
	g=255;
	b=0;
  }else{
	r=0;
	g=0;
	b=255;
  }
  //const uint32_t rgb=(r << 16) | (g << 8) | b;
  return create_pixel_rgba(r,g,b,alpha);
}

static inline void memset32_loop(uint32_t* dest,const uint32_t value,int num){
  for ( ; num ; dest+=1, num-=1) {
	*dest=value;
  }
}

// fill a RGB(A) frame buffer with a specific color, taking stride into account
static void fillFrame(uint8_t* dest,const int width,const int height,const int stride,const uint32_t rgb){
  if(stride==width*4){
	memset32_loop((uint32_t*)dest,rgb,height*width);
  }else{
	//std::cout<<stride<<" "<<width<<"\n";
	for (int j = 0; j < height; ++j) {
	  const int offsetStride=stride * j;
	  uint32_t* lineStart=(uint32_t*)&dest[offsetStride];
	  memset32_loop(lineStart,rgb,width);
	}
  }
}


#endif //HELLO_DRMPRIME__COLOR_HELPER_H_
