//#include <iostream>
#include <string.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdio.h>

#include "FBAccess.h"
namespace FBAccess{

  extern "C" void setPixel(struct fb framebuffer, int posX, int posY, uint16_t color){
  
    int offset = (posY * framebuffer.fb_width + posX);
  
    ((uint16_t *)framebuffer.fbdata)[offset] = color;
  
  }

  extern "C" struct fb initFramebuffer(char* fbdev){
    // This is currently black magic to me
    struct fb framebuffer;
    framebuffer.fbfd = open(fbdev, O_RDWR);
    if (framebuffer.fbfd >= 0){
      struct fb_var_screeninfo vinfo;
      ioctl(framebuffer.fbfd, FBIOGET_VSCREENINFO, &vinfo);
      framebuffer.fb_width = vinfo.xres;
      framebuffer.fb_height = vinfo.yres;
      framebuffer.fb_bpp = vinfo.bits_per_pixel;
      framebuffer.fb_bytes = framebuffer.fb_bpp / 8;
      framebuffer.fb_data_size = framebuffer.fb_width * framebuffer.fb_height * framebuffer.fb_bytes;
      framebuffer.fbdata = mmap(0,framebuffer.fb_data_size, PROT_READ | PROT_WRITE, MAP_SHARED, framebuffer.fbfd, (off_t)0);
      return framebuffer;
    }
    _exit(1);
  }

  extern "C" void drawFrame(struct fb framebuffer, uint16_t *frame){
    //printf("Drawing frame\n");
    memcpy(framebuffer.fbdata, frame, framebuffer.fb_data_size);
  }
}
