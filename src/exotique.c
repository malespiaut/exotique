#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <SDL.h>
#include <SDL_image.h>
#pragma GCC diagnostic pop

// XXX: Screen size

extern const int kScreenWidth;
extern const int kScreenHeight;
#define kScreenPixels (kScreenWidth * kScreenHeight)

// XXX: Data structures

enum eKey
{
  eKey_up,
  eKey_down,
  eKey_left,
  eKey_right,
  eKey_shoot,
  eKey_cancel,
  eKey_pause,
  eKey_count
};
typedef enum eKey eKey;

enum eKeyState
{
  eKeyState_off = 0x0,       // 0b00
  eKeyState_up = 0x1,        // 0b01
  eKeyState_pressed = 0x2,   // 0b10
  eKeyState_held = 0x3,      // 0b11
  eKeyState_active_bit = 0x2 // 0b10
};
typedef enum eKeyState eKeyState;

enum eButton
{
  bUp,
  bDown,
  bLeft,
  bRight,
  bSelect,
  bStart,
  bA,
  bB,
  bX,
  bY,
  bL1,
  bR1,
  bL2,
  bR2,
  bL3,
  bR3,
  bMouseLeftClick,
  bMouseRightClick
};
typedef enum eButton eButton;

typedef struct ScreenManager ScreenManager;
struct ScreenManager
{
  SDL_Renderer* renderer;
  SDL_Texture* texture;
  SDL_Window* window;
  bool fullscreen;
  uint8_t* screen;
  uint32_t* screen_rgba;
  uint32_t palette[256];
};

typedef struct GameManager GameManager;
struct GameManager
{
  const char* name;
  ScreenManager screen_manager;
  bool exit;
  SDL_Scancode key_map[7]; // 7 = eKey_count
  uint8_t key_states[7];   // 7 = eKey_count
};

typedef struct vec2i_s vec2i_t;
struct vec2i_s
{
  int32_t x;
  int32_t y;
};

typedef struct PlayerInput PlayerInput;
struct PlayerInput
{
  uint32_t buttons;
  vec2i_t joystick;
};

typedef struct ExotiqueInterface ExotiqueInterface;
struct ExotiqueInterface
{
  uint8_t* screen;   //[kScreenWidth * kScreenHeight];
  uint32_t* palette; //[256];

  vec2i_t mouse;
  PlayerInput player[4];

  uint64_t ticks;
};

// XXX: Global data structure

static GameManager g_game_manager = {0};
ExotiqueInterface g_exotique_interface = {0};

// XXX: game.c mandatory functions declarations

void game_load(ExotiqueInterface* ei);
void game_update(ExotiqueInterface* ei);
void game_draw(ExotiqueInterface* ei);

// XXX: Functions declarations

static void exotique_panic(GameManager* gm);

// XXX: Input functions

static inline void
bit_set(uint32_t* data, uint8_t n)
{
  if (n < 32) // Ensure n is within 0-31
  {
    *data |= ((uint32_t)1 << n);
  }
}

static void
key_state_update(uint8_t* state, bool is_down)
{
  switch (*state) // look at prev state
  {
    case eKeyState_held:
    case eKeyState_pressed:
      *state = is_down ? eKeyState_held : eKeyState_up;
      break;
    case eKeyState_off:
    case eKeyState_up:
    default:
      *state = is_down ? eKeyState_pressed : eKeyState_off;
      break;
  }
}

static bool
key_new_get(GameManager* gm, int32_t key)
{
  return gm->key_states[key] == eKeyState_pressed;
}

// XXX: Game functions

static void
exotique_draw(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  for (int32_t i = 0; i < kScreenPixels; ++i)
  {
    sm->screen_rgba[i] = sm->palette[sm->screen[i]];
  }
  if (SDL_UpdateTexture(sm->texture, nullptr, sm->screen_rgba, (int)(sizeof(uint32_t) * (unsigned long)kScreenWidth)))
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "Couldn't update the given texture rectangle with new pixel data: %s", SDL_GetError());
    exotique_panic(gm);
  }

  // Rendering the final texture to screen
  if (SDL_RenderClear(sm->renderer))
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "Couldn't clear the current rendering target with the drawing color: %s", SDL_GetError());
    exotique_panic(gm);
  }
  if (SDL_RenderCopy(sm->renderer, sm->texture, NULL, NULL))
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "Couldn't copy a portion of the texture to the current rendering target: %s", SDL_GetError());
    exotique_panic(gm);
  }

  SDL_RenderPresent(sm->renderer);
}

static void
exotique_events(GameManager* gm)
{
  SDL_Event event = {0};

  SDL_PumpEvents();

  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT:
        gm->exit = true;
        break;

      case SDL_KEYDOWN:
        if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
        {
          gm->exit = true;
          break;
        }
        break;
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_CLOSE)
        {
          gm->exit = true;
          break;
        }
        else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        {
          if (!gm->screen_manager.fullscreen)
          {
            if (SDL_ENABLE == SDL_ShowCursor(SDL_ENABLE))
            {
              SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Couldn't toggle whether or not the cursor is shown: %s", SDL_GetError());
            }
          }
        }
        else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
        {
          if (!gm->screen_manager.fullscreen)
          {
            if (SDL_DISABLE == SDL_ShowCursor(SDL_DISABLE))
            {
              SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Couldn't toggle whether or not the cursor is shown: %s", SDL_GetError());
            }
          }
        }
        break;

      default:
        break;
    }
  }

  int numkeys = 0;
  const uint8_t* keystate = SDL_GetKeyboardState(&numkeys);
  if (!keystate)
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_INPUT, "Couldn't get a snapshot of the current state of the keyboard: %s", SDL_GetError());
    exotique_panic(gm);
  }

  for (size_t i = 0; i < eKey_count; ++i)
  {
    const int scancode = (int)gm->key_map[i];

    bool is_down = false;

    if (scancode && scancode < numkeys)
    {
      is_down |= 0 != keystate[scancode];
    }

    key_state_update(&gm->key_states[i], is_down);
  }
}

static void
exotique_load(GameManager* gm, ExotiqueInterface* ei)
{
  ScreenManager* sm = &gm->screen_manager;

  gm->name = "🌴 Exotique v0.5β - SDL2 (25/05/09)";

  sm->screen = malloc((unsigned long)kScreenPixels * sizeof(uint8_t));
  sm->screen_rgba = malloc((unsigned long)kScreenPixels * sizeof(uint32_t));

  gm->key_map[eKey_up] = SDL_SCANCODE_UP;
  gm->key_map[eKey_down] = SDL_SCANCODE_DOWN;
  gm->key_map[eKey_left] = SDL_SCANCODE_LEFT;
  gm->key_map[eKey_right] = SDL_SCANCODE_RIGHT;
  gm->key_map[eKey_shoot] = SDL_SCANCODE_RETURN;

  ei->screen = gm->screen_manager.screen;
  ei->palette = gm->screen_manager.palette;

  memset(&ei->mouse, 0, sizeof(ei->mouse));
  memset(&ei->player, 0, sizeof(ei->player));
}

static void
exotique_unload(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  free(sm->screen);
  free(sm->screen_rgba);
}

static void
exotique_update(GameManager* gm, ExotiqueInterface* ei)
{
  ei->ticks = SDL_GetTicks64();

  memset(&ei->mouse, 0, sizeof(ei->mouse));
  SDL_GetMouseState(&ei->mouse.x, &ei->mouse.y);
  //SDL_GetGlobalMouseState(&gm->input_manager.mouse_x, &gm->input_manager.mouse_y);

  memset(&ei->player, 0, sizeof(ei->player));
  exotique_events(gm);
  if (key_new_get(gm, eKey_up))
  {
    bit_set(&ei->player[0].buttons, bUp);
  }
  if (key_new_get(gm, eKey_down))
  {
    bit_set(&ei->player[0].buttons, bDown);
  }
  if (key_new_get(gm, eKey_left))
  {
    bit_set(&ei->player[0].buttons, bLeft);
  }
  if (key_new_get(gm, eKey_right))
  {
    bit_set(&ei->player[0].buttons, bRight);
  }
}

// XXX: SDL functions

static void
sdl_load(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
  SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "SDL2 initialization");
  if (SDL_Init(SDL_INIT_VIDEO))
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Couldn't initialize the SDL library: %s", SDL_GetError());
    exotique_panic(gm);
  }

  if (!(sm->window = SDL_CreateWindow(gm->name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, kScreenWidth, kScreenHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_UTILITY)))
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Couldn't create a window with the specified position, dimensions, and flags: %s", SDL_GetError());
    exotique_panic(gm);
  }

  if (!(sm->renderer = SDL_CreateRenderer(sm->window, -1, SDL_RENDERER_PRESENTVSYNC)))
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "Couldn't create a 2D rendering context for a window: %s", SDL_GetError());
    exotique_panic(gm);
  }

  if (!(sm->texture = SDL_CreateTexture(sm->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, kScreenWidth, kScreenHeight)))
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "Couldn't create a texture for a rendering context: %s", SDL_GetError());
    exotique_panic(gm);
  }

  // These calls are optional.
  // They only serves to make the pixels square and clean when resizing the window.
  if (SDL_RenderSetLogicalSize(sm->renderer, kScreenWidth, kScreenHeight))
  {
    SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Couldn't create a texture for a rendering context: %s", SDL_GetError());
  }
  if (SDL_RenderSetIntegerScale(sm->renderer, SDL_TRUE))
  {
    SDL_LogWarn(SDL_LOG_CATEGORY_RENDER, "Couldn't set whether to force integer scales for resolution-independent rendering: %s", SDL_GetError());
  }
}

static void
sdl_unload(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  SDL_DestroyTexture(sm->texture);
  SDL_DestroyRenderer(sm->renderer);
  SDL_DestroyWindow(sm->window);
}

static void
exotique_cleanup(GameManager* gm)
{
  sdl_unload(gm);
  exotique_unload(gm);
  SDL_Quit();
}

static void
exotique_panic(GameManager* gm)
{
  exotique_cleanup(gm);
  exit(EXIT_FAILURE);
}

// XXX: Main function

int
main(const int argc, const char* argv[])
{
  (void)argc;
  (void)argv;

  exotique_load(&g_game_manager, &g_exotique_interface);
  game_load(&g_exotique_interface);
  sdl_load(&g_game_manager);

  // Main loop
  while (!g_game_manager.exit)
  {
    exotique_update(&g_game_manager, &g_exotique_interface);
    game_update(&g_exotique_interface);
    game_draw(&g_exotique_interface);
    exotique_draw(&g_game_manager);
  }

  exotique_cleanup(&g_game_manager);
  return EXIT_SUCCESS;
}
