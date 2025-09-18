#include "exotique.h"

/* XXX: Screen size */

const i32 kScreenWidth = 320;
const i32 kScreenHeight = 200;
#define kScreenPixels (kScreenWidth * kScreenHeight)

enum color_e
{
  eColorTransparent,
  eColorRipePlum,
  eColorDisco,
  eColorTorchRed,
  eColorCrusta,
  eColorBrightSun,
  eColorLima,
  eColorOceanGreen,
  eColorChambray,
  eColorBlack,
  eColorBlueRibbon,
  eColorDodgerBlue,
  eColorBrightTurquoise,
  eColorSeashellPeach,
  eColorFrenchGray,
  eColorBrownRust,
  eColorSoyaBean
};
typedef enum color_e color_t;

u8 gsprite[64] = {
  1,2,3,4,5,6,7,8,
  9,10,11,12,13,14,15,16,
  1,2,3,4,5,6,7,8,
  9,10,11,12,13,14,15,16,
  1,2,3,4,5,6,7,8,
  9,10,11,12,13,14,15,16,
  1,2,3,4,5,6,7,8,
  9,10,11,12,13,14,15,16
};
i32 scaling=0;

/*
 * ANSI C89 compliant sprite scaling using 16.16 fixed-point arithmetic.
 *
 * This function scales an 8x8 indexed sprite and draws it centered at (base_x, base_y)
 * on a kScreenWidth x kScreenHeight screen using nearest-neighbor interpolation.
 *
 * Fixed-point format: 16.16 (16 integer bits, 16 fractional bits)
 * Scale factor: 65536 = 1.0x
 *
 * All arithmetic is done in 32-bit fixed-point; no floating-point or library functions.
 */


void scale_sprite(u8* sprite, u8* screen, 
                  int base_x, int base_y, int scale_factor)
{
    /* Constants for screen dimensions */
    const int SCREEN_WIDTH = 320;
    const int SCREEN_HEIGHT = 200;
    const int SPRITE_SIZE = 8;
    
    /* Fixed-point constants (16.16 format) */
    const int FIXED_ONE = 65536;        /* 1.0 in 16.16 fixed-point */
    const int FIXED_HALF = 32768;       /* 0.5 in 16.16 fixed-point */
    
    /* Calculate the scaled sprite dimensions in pixels */
    /* scaled_size = 8 * scale_factor / FIXED_ONE */
    /* Using long to prevent overflow during multiplication */
    long temp = (long)SPRITE_SIZE * scale_factor;
    int scaled_size = (int)(temp >> 16);  /* Divide by 65536 using shift */
    
    /* Calculate top-left corner of the scaled sprite */
    /* Center the sprite at (base_x, base_y) */
    int half_size = scaled_size >> 1;
    int start_x = base_x - half_size;
    int start_y = base_y - half_size;
    
    
    /* Calculate the inverse scale for source coordinate mapping */
    /* inv_scale = FIXED_ONE / scale_factor */
    /* We use fixed-point division: (1.0 << 32) / scale_factor */
    /* This gives us the step size in source coordinates */
    /*long long*/ i64 dividend = ((/*long long*/i64)FIXED_ONE << 16);
    int inv_scale = (int)(dividend / scale_factor);
    
    /* Determine actual drawing bounds with clipping */
    int draw_start_x = (start_x < 0) ? 0 : start_x;
    int draw_start_y = (start_y < 0) ? 0 : start_y;
    int draw_end_x = (start_x + scaled_size > SCREEN_WIDTH) ? 
                     SCREEN_WIDTH : start_x + scaled_size;
    int draw_end_y = (start_y + scaled_size > SCREEN_HEIGHT) ? 
                     SCREEN_HEIGHT : start_y + scaled_size;
    
    /* Calculate initial source coordinates in fixed-point */
    /* When clipped, we need to offset into the source sprite */
    int src_start_x_fixed = 0;
    int src_start_y_fixed = 0;
    
    
    /* Main rendering loop */
    int screen_y, screen_x;
    int src_y_fixed = src_start_y_fixed;
    
    /* Early exit if scale is zero or negative */
    if (scale_factor <= 0) return;
    
    
    /* Early exit if sprite would be invisible (too small) */
    if (scaled_size <= 0) return;
    
    
    /* Early culling: completely off-screen */
    if (start_x >= SCREEN_WIDTH || start_y >= SCREEN_HEIGHT ||
        start_x + scaled_size <= 0 || start_y + scaled_size <= 0) {
        return;
    }

    if (start_x < 0) {
        /* Calculate how far into the source sprite we start */
        src_start_x_fixed = (-start_x) * inv_scale;
    }
    if (start_y < 0) {
        src_start_y_fixed = (-start_y) * inv_scale;
    }

    for (screen_y = draw_start_y; screen_y < draw_end_y; screen_y++) {
        int src_row_offset;
        int src_x_fixed;

        /* Calculate screen buffer offset for this row */
        int screen_row_offset = screen_y * SCREEN_WIDTH;
        
        /* Convert fixed-point to integer source Y coordinate */
        /* Using nearest neighbor: add 0.5 before truncating */
        int src_y = (src_y_fixed + FIXED_HALF) >> 16;
        
        /* Clamp source Y to valid range [0, 7] */
        if (src_y < 0) src_y = 0;
        if (src_y >= SPRITE_SIZE) src_y = SPRITE_SIZE - 1;
        
        /* Calculate source row offset */
        src_row_offset = src_y << 3;  /* src_y * 8 */
        
        /* Process pixels in this row */
        src_x_fixed = src_start_x_fixed;
        
        for (screen_x = draw_start_x; screen_x < draw_end_x; screen_x++) {
            /* Convert fixed-point to integer source X coordinate */
            int src_x = (src_x_fixed + FIXED_HALF) >> 16;
            
            /* Clamp source X to valid range [0, 7] */
            if (src_x < 0) src_x = 0;
            if (src_x >= SPRITE_SIZE) src_x = SPRITE_SIZE - 1;
            
            /* Read from source sprite and write to screen */
            /* Both arrays are guaranteed to be within bounds */
            screen[screen_row_offset + screen_x] = sprite[src_row_offset + src_x];
            
            /* Advance source X coordinate */
            src_x_fixed += inv_scale;
        }
        
        /* Advance source Y coordinate */
        src_y_fixed += inv_scale;
    }
}



static void
memzero(u8* dest, u64 len)
{
  while (len-- > 0)
  {
    *dest++ = 0;
  }
}

/* XXX: Exotique interface functions */

void
game_load(ExotiqueInterface* ei)
{
  /* Setup the color palette here */
  ei->palette[eColorTransparent] = 0x00000000;
  ei->palette[eColorRipePlum] = 0x430067ff;
  ei->palette[eColorDisco] = 0x94216aff;
  ei->palette[eColorTorchRed] = 0xff004dff;
  ei->palette[eColorCrusta] = 0xff8426ff;
  ei->palette[eColorBrightSun] = 0xffdd34ff;
  ei->palette[eColorLima] = 0x50e112ff;
  ei->palette[eColorOceanGreen] = 0x3fa66fff;
  ei->palette[eColorChambray] = 0x365987ff;
  ei->palette[eColorBlack] = 0x000000ff;
  ei->palette[eColorBlueRibbon] = 0x0033ffff;
  ei->palette[eColorDodgerBlue] = 0x29adffff;
  ei->palette[eColorBrightTurquoise] = 0x00ffccff;
  ei->palette[eColorSeashellPeach] = 0xfff1e8ff;
  ei->palette[eColorFrenchGray] = 0xc2c3c7ff;
  ei->palette[eColorBrownRust] = 0xab5236ff;
  ei->palette[eColorSoyaBean] = 0x5f574fff;
}

void
game_update(ExotiqueInterface* ei)
{
  (void)ei;
  scaling += 8192;
}

void
game_draw(ExotiqueInterface* ei)
{
  memzero(ei->screen, (u64)kScreenPixels);
  scale_sprite(gsprite, ei->screen, 160, 100, scaling);
}
