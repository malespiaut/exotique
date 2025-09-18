#include "exotique.h"

/* XXX: Screen size */

const i32 kScreenWidth = 480;
const i32 kScreenHeight = 480;
#define kScreenPixels (kScreenWidth * kScreenHeight)

typedef struct 


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
