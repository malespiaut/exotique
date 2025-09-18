#include "exotique.h"

/* XXX: Screen size */

const i32 kScreenWidth = 320;
const i32 kScreenHeight = 240;
#define kScreenPixels (kScreenWidth * kScreenHeight)

i32 xcube = 0;
i32 ycube = 0;

/* XXX: Structures and types */

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

static void
rectangle_fill_draw(ExotiqueInterface* ei, i32 x1, i32 y1, i32 x2, i32 y2, u8 color)
{

  if (y1 > y2)
  {
    i32 temp = y1;
    y1 = y2;
    y2 = temp;
  }
  
  if (x1 > x2)
  {
    i32 temp = x1;
    x1 = x2;
    x2 = temp;
  }

  /* Clamp within screen bounds */

  if (x1 < 0)
  {
    x1 = 0;
  }
  if (x2 >= kScreenWidth)
  {
    x2 = kScreenWidth - 1;
  }

  if (y1 < 0)
  {
    y1 = 0;
  }
  if (y2 >= kScreenHeight)
  {
    y2 = kScreenHeight - 1;
  }
  

  /* Calculate the width and height of the rectangle */

  {
   i32 y1_offset = y1 * kScreenWidth + x1;
  i32 y2_offset = y2 * kScreenWidth + x1;
  i32 width = x2 - x1 + 1;
      i32 i = y1_offset;
      for (; i <= y2_offset ; i+= kScreenWidth)
      {
        i32 j = 0;
        for (; j < width; ++j){

        ei->screen[j + i] = color;
        /*ei->screen[j * kScreenWidth + i] = color;*/
        }
      }
  }
}

static void
button_draw(ExotiqueInterface* ei, i32 x, i32 y, i32 is_active)
{
  if (is_active)
  {
    rectangle_fill_draw(ei, x, y, x + 10, y + 10, eColorBrightSun);
  }
  else
  {
    rectangle_fill_draw(ei, x, y, x + 10, y + 10, eColorTorchRed);
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
  /* Setting up the color palette */
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
  if (ei->input[0].up)
  {
    --ycube;
  }
  if (ei->input[0].down)
  {
    ++ycube;
  }
  if (ei->input[0].left)
  {
    --xcube;
  }
  if (ei->input[0].right)
  {
    ++xcube;
  }
}

void
game_draw(ExotiqueInterface* ei)
{
  memzero(ei->screen, (u64)kScreenPixels);

  /* Demonstrate flexible font rendering with different sizes */
  rectangle_fill_draw(ei, xcube, ycube, xcube+10, ycube+10, eColorRipePlum);
  button_draw(ei, 122, 84, ei->input[0].up);
  button_draw(ei, 122, 106, ei->input[0].down);
  button_draw(ei, 111, 95, ei->input[0].left);
  button_draw(ei, 133, 95, ei->input[0].right);

  button_draw(ei, 144, 73, ei->input[0].select);
  button_draw(ei, 166, 73, ei->input[0].start);

  button_draw(ei, 199, 95, ei->input[0].a);
  button_draw(ei, 188, 106, ei->input[0].b);
  button_draw(ei, 188, 84, ei->input[0].x);
  button_draw(ei, 177, 95, ei->input[0].y);

  button_draw(ei, 111, 62, ei->input[0].l1);
  button_draw(ei, 199, 62, ei->input[0].r1);
  button_draw(ei, 133, 62, ei->input[0].l2);
  button_draw(ei, 177, 62, ei->input[0].r2);
  button_draw(ei, 122, 128, ei->input[0].l3);
  button_draw(ei, 188, 128, ei->input[0].r3);
}
