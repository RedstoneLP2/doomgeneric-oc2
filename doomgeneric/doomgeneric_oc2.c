//doomgeneric for oc2

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>


#include <stdbool.h>
#include "FBAccess.h"

#define KEYQUEUE_SIZE 16

uint16_t ConvertPixel(uint32_t);
static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;
fb FrameBuffer;
uint16_t* FakeFrameBuffer;
int startTimeMilis;
int keyb_fd;

static const char *const evval[3] = {
    "RELEASED",
    "PRESSED ",
    "REPEATED"
};

static unsigned char convertToDoomKey(unsigned int key)
{
	switch (key)
	{
    case KEY_ENTER:
		key = 13;
		break;
    case KEY_ESC:
		key = KEY_ESCAPE;
		break;
    case KEY_LEFT:
		key = KEY_LEFTARROW;
		break;
    case KEY_RIGHT:
		key = KEY_RIGHTARROW;
		break;
    case KEY_UP:
		key = KEY_UPARROW;
		break;
    case KEY_DOWN:
		key = KEY_DOWNARROW;
		break;
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
		key = KEY_FIRE;
		break;
    case KEY_SPACE:
		key = KEY_USE;
		break;
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:
		key = KEY_RSHIFT;
		break;
	default:
		key = tolower(key);
		break;
	}

	return key;
}

static void addKeyToQueue(int pressed, unsigned int keyCode){
	unsigned char key = convertToDoomKey(keyCode);


  unsigned short keyData = (pressed << 8) | key;

  s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
  s_KeyQueueWriteIndex++;
  s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

static void handleKeyInput(){
  struct input_event ev;
  ssize_t n;
  
  n = read(keyb_fd, &ev, sizeof ev);
  if (n != sizeof ev) {
      errno = EIO;
  }
  if (ev.type == EV_KEY) {
    //printf("keyinput: %d\n", ev.code);
      if (ev.value == 0 || ev.value == 1) {
          addKeyToQueue(ev.value, ev.code);
      }
  }       
}


void DG_Init(){
    //printf("initiating\n\n\n\n\n\n\n");
    char* fbdev="/dev/fb0";
    FrameBuffer = initFramebuffer(fbdev);

    FakeFrameBuffer = (uint16_t*) malloc(FrameBuffer.fb_width * FrameBuffer.fb_height * sizeof(uint16_t));
    memset(FakeFrameBuffer, 0xffff00, FrameBuffer.fb_width * FrameBuffer.fb_height * sizeof(uint16_t));

    struct timeval tp;
    gettimeofday(&tp, NULL);
    startTimeMilis = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    //printf("setting up keyboard\n");
    const char *dev = "/dev/input/event0";
    keyb_fd = open(dev, O_RDONLY);
    if (keyb_fd == -1) {
        //fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void DrawFrame16Bit(){
      for (int y = 0 ; y < FrameBuffer.fb_height ; y++)
    {
        for (int x = 0; x < FrameBuffer.fb_width; x++) {
        if (DOOMGENERIC_RESX < x|| DOOMGENERIC_RESY < y) {
            FakeFrameBuffer[x+FrameBuffer.fb_width*y] = 0x0000;
        }
        else {
            FakeFrameBuffer[x+DOOMGENERIC_RESX*y] = ConvertPixel(DG_ScreenBuffer[x+DOOMGENERIC_RESX*y]);
        }
    }
    }

    drawFrame(FrameBuffer,FakeFrameBuffer);
    
}

uint16_t ConvertPixel(uint32_t Pixel){
  uint8_t *PixelPointer = (uint8_t*)&Pixel;
  uint16_t PixelOut;
  PixelOut = ((PixelPointer[2] & 0b11111000) << 8) | ((PixelPointer[1] & 0b11111100) << 3) | (PixelPointer[0] >> 3);
  return PixelOut;
}

void DG_DrawFrame()
{
  DrawFrame16Bit();
  //printf("Rendered frame\n");
  handleKeyInput();
  //printf("handled input\n");
}

void DG_SleepMs(uint32_t ms)
{
  usleep(ms);
}

uint32_t DG_GetTicksMs()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    
    return (tp.tv_sec * 1000 + tp.tv_usec / 1000) - startTimeMilis;
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{

  if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex){
    //key queue is empty
    return 0;
  }else{
    unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
    s_KeyQueueReadIndex++;
    s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

    *pressed = keyData >> 8;
    *doomKey = keyData & 0xFF;

    return 1;
  }

  return 0;
}

void DG_SetWindowTitle(const char* title)
{

}