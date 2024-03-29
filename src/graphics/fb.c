#include "types.h"
#include "mbox.h"
#include "fb.h"
#include "utils.h"
#include "printf.h"

i32 fbinit(int pw, int ph) {
    mbox[0] = 35*4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003;      //set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = pw;           //FrameBufferInfo.width
    mbox[6] = ph;           //FrameBufferInfo.height

    mbox[7] = 0x48004;      //set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = pw;          //FrameBufferInfo.virtual_width
    mbox[11] = ph;          //FrameBufferInfo.virtual_height

    mbox[12] = 0x48009;     //set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0;           //FrameBufferInfo.x_offset
    mbox[16] = 0;           //FrameBufferInfo.y.offset

    mbox[17] = 0x48005;     //set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32;          //FrameBufferInfo.depth

    mbox[21] = 0x48006;     //set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 0;           //RGB, not BGR preferably

    mbox[25] = 0x40001;     //get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096;        //FrameBufferInfo.pointer
    mbox[29] = 0;           //FrameBufferInfo.size

    mbox[30] = 0x40008;     //get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0;           //FrameBufferInfo.pitch

    mbox[34] = MBOX_TAG_LAST;

    //this might not return exactly what we asked for, could be
    //the closest supported resolution instead
    if(mbox_call(MBoxChannelPROP) && mbox[20]==32 && mbox[28]!=0) {
        mbox[28]&=0x3FFFFFFF;   //convert GPU address to ARM address
        fb.width  = mbox[5];          //get actual physical width
        fb.height = mbox[6];         //get actual physical height
        fb.pitch  = mbox[33];         //get number of bytes per line
        fb.isrgb  = mbox[24];         //get the actual channel order
        fb.lock   = 0;
        fb.cursor_x = 0;
        fb.cursor_y = 128;
        fb.ptr    = (void*)((unsigned long)mbox[28]);
        
        draw_pi_logo_at_pos(0, 0);
        fbputs("\n\nRaspberry Pi 3\n\n\rImage built "__TIME__" on "__DATE__"\n\n");

        return 0;
    }
    return -1;
}

void fbclear(u32 color) {
    if(!fb.ptr) {
        return;
    }
    // Invert R and B channels if in RGB mode
    if(fb.isrgb)
    {
        u32 r = color & 0xFF0000;
        u32 g = color & 0x00FF00;
        u32 b = color & 0x0000FF;
        color = (r >> 16) | g | (b << 16);
    }
    fb.bg = color;
    fb.cursor_x = 0;
    fb.cursor_y = 0;
    
    u64 * cur_pixel = (u64 *) fb.ptr;
    u64 long_color = fb.bg | ((u64) fb.bg << 32);
    for(int y=0; y<fb.height;++y)
    {
        for(int x=0;x < fb.width;++x)
        {
            *cur_pixel = long_color;
            cur_pixel++;
        }
        cur_pixel += fb.pitch - fb.width*4;
    }
}

void fbsetres(u64 w, u64 h) {
    mbox[0] = 17*4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x00048003;
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = w;
    mbox[6] = h;

    mbox[7] = 0x40008;     //get pitch
    mbox[8] = 4;
    mbox[9] = 4;
    mbox[10] = 0;           //FrameBufferInfo.pitch

    mbox[11] = 0x40001;     //get framebuffer, gets alignment on request
    mbox[12] = 8;
    mbox[13] = 8;
    mbox[14] = 4096;        //FrameBufferInfo.pointer
    mbox[15] = 0;           //FrameBufferInfo.size

    mbox[16] = MBOX_TAG_LAST;

    mbox_call(MBoxChannelPROP);
    if(1) {
        fb.width  = mbox[5];          //get actual physical width
        fb.height = mbox[6];         //get actual physical height
        fb.pitch = mbox[10];
        fb.ptr = (void*)((unsigned long)mbox[14]);
    }
}
