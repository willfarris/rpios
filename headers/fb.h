#pragma once

#include "types.h"

#define DISPLAY_WIDTH 1280
#define DISPLAY_HEIGHT 720

struct FrameBuffer
{
    u32 width;
    u32 height;
    u32 pitch;
    u32 isrgb;
    u32 cursor_x;
    u32 cursor_y;
    u32 bg;
    u8 lock;
    u8 * ptr;
};

i32 fbinit(int, int);
void fbclear(u32 color);
void fbrainbow();
void fbsetres(u64, u64);

extern struct FrameBuffer fb;