#ifdef __cplusplus
namespace FBAccess{
   
#endif
 #include <stdint.h>
struct fb{
int fb_width, fb_height, fb_bpp, fb_bytes, fb_data_size, fbfd;
void *fbdata;
};

#ifdef __cplusplus
extern "C"{
#endif

typedef struct fb fb;
struct fb initFramebuffer(char*);
void setPixel(struct fb, int, int, uint16_t);
void drawFrame(struct fb, uint16_t*);

#ifdef __cplusplus
}
}
#endif