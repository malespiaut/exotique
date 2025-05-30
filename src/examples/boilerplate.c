#include "exotique.h"

/* XXX: Screen size */

const i32 kScreenWidth = 320;
const i32 kScreenHeight = 200;
#define kScreenPixels (kScreenWidth * kScreenHeight)

/* XXX: Exotique interface functions */

void
game_load(ExotiqueInterface* ei)
{
  /* Setup the color palette here */
  (void)ei;
}

void
game_update(ExotiqueInterface* ei)
{
  (void)ei;
}

void
game_draw(ExotiqueInterface* ei)
{
  (void)ei;
}
