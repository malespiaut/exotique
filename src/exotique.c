#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wswitch-default"
#include <SDL.h>
#include <SDL_image.h>
#pragma GCC diagnostic pop

// XXX: Defines

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884f // pi
#endif

#ifndef M_PI_2
#define M_PI_2 1.570796326794896619231321691639751442f // pi/2
#endif

#ifndef M_3PI_2
#define M_3PI_2 4.712388980384689857693965074919254326f // (3*pi)/2
#endif

#ifndef M_2PI
#define M_2PI 6.283185307179586476925286766559005768f // 2*pi
#endif

// XXX: This magical number is used to calculate tank sprite rotation
// It has been discovered intuitively and empiricaly.

#ifndef M_360_2PI
#define M_360_2PI 57.295779513082320876798154814105170336f // 360/(2*pi)
#endif

#define kDegreeToRadian (M_PI / 180.0f)

#ifndef kScreenWidth 
#define kScreenWidth 320
#endif
#ifndef kScreenHeight
#define kScreenHeight 240
#endif
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

typedef struct InputManager InputManager;
struct InputManager
{
  uint16_t buttons[4];
  int16_t joy_x[4];
  int16_t joy_y[4];
  int32_t mouse_x;
  int32_t mouse_y;
};

typedef struct ScreenManager ScreenManager;
struct ScreenManager
{
  SDL_Renderer* renderer;
  SDL_Texture* texture;
  SDL_Window* window;
  bool fullscreen;
  uint8_t* screen;
  uint32_t* screen_rgba;
  // uint8_t screen[kScreenPixels];
  // uint32_t screen_rgba[kScreenPixels];
  uint32_t palette[256];
};

typedef struct GameManager GameManager;
struct GameManager
{
  const char* name;
  ScreenManager screen_manager;
  InputManager input_manager;
  bool exit;
  uint64_t ticks;
  SDL_Scancode key_map[7]; // 7 = eKey_count
  uint8_t key_states[7];   // 7 = eKey_count
};

typedef struct ExotiqueInterface ExotiqueInterface;
struct ExotiqueInterface
{
  uint8_t* screen;   //[kScreenWidth * kScreenHeight];
  uint32_t* palette; //[256];

  int32_t* mouse_x;
  int32_t* mouse_y;

  uint16_t* player1_buttons;
  int16_t* player1_joy_x;
  int16_t* player1_joy_y;

  uint16_t* player2_buttons;
  int16_t* player2_joy_x;
  int16_t* player2_joy_y;

  uint16_t* player3_buttons;
  int16_t* player3_joy_x;
  int16_t* player3_joy_y;

  uint16_t* player4_buttons;
  int16_t* player4_joy_x;
  int16_t* player4_joy_y;
};

// XXX: Global data structure

GameManager g_game_manager = {0};
ExotiqueInterface g_exotique_interface = {0};

// XXX: game.c mandatory functions declarations

void game_load(ExotiqueInterface* ei);
void game_update(ExotiqueInterface* ei);
void game_draw(ExotiqueInterface* ei);

// XXX: Debug functions

static void
debug_printf(const char* fmt, const char* fn, const char* fmt2, ...)
{
  va_list ap;

// #pragma clang diagnostic push
#pragma GCC diagnostic push
// #pragma clang diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
  printf(fmt, fn);
// #pragma clang diagnostic pop
#pragma GCC diagnostic pop

  va_start(ap, fmt2);
  vprintf(fmt2, ap);
  va_end(ap);

  puts("\033[0m");
}

inline static void
sdl_panic_check(const bool condition, const char* function)
{
  if (condition)
  {
    debug_printf("\033[0;31m[PANIC] SDL_%s(): ", function, "%s", SDL_GetError());
    exit(EXIT_FAILURE);
  }
}

// XXX: Input functions

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

// XXX: Game functions

static void
exotique_draw(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  for (int32_t i = 0; i < kScreenPixels; ++i)
  {
    sm->screen_rgba[i] = sm->palette[sm->screen[i]];
  }
  sdl_panic_check(SDL_UpdateTexture(sm->texture, nullptr, sm->screen_rgba, sizeof(uint32_t) * kScreenWidth), "UpdateTexture");

  // XXX: Rendering the game to a texture
  // sdl_panic_check(SDL_SetRenderTarget(sm->renderer, sm->texture), "SetRenderTarget");
  // sdl_panic_check(SDL_RenderClear(sm->renderer), "RenderClear");

  // XXX: Rendering the final texture to screen
  // sdl_panic_check(SDL_SetRenderTarget(sm->renderer, NULL), "SetRenderTarget");
  sdl_panic_check(SDL_RenderClear(sm->renderer), "RenderClear");
  sdl_panic_check(SDL_RenderCopy(sm->renderer, sm->texture, NULL, NULL), "RenderCopy");

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

      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_CLOSE)
        {
          gm->exit = true;
          break;
        }
        else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        {
          if (gm->screen_manager.fullscreen)
          {
            const int sdl_showcursor_result = SDL_ShowCursor(1);
            sdl_panic_check(sdl_showcursor_result < 0, "ShowCursor");
          }
        }
        else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
        {
          if (gm->screen_manager.fullscreen)
          {
            const int sdl_showcursor_result = SDL_ShowCursor(0);
            sdl_panic_check(sdl_showcursor_result < 0, "ShowCursor");
          }
        }
        break;

      default:
        break;
    }
  }

  int numkeys = 0;
  const uint8_t* keystate = SDL_GetKeyboardState(&numkeys);
  sdl_panic_check(!keystate, "GetKeyboardState");

  for (size_t i = 0; i < eKey_count; ++i)
  {
    const int scancode = gm->key_map[i];

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

  gm->name = "🌴 Exotique - Preview Developer Version 0.1 - 24/09/18";

  sm->screen = malloc(kScreenWidth * kScreenHeight * sizeof(uint8_t));
  sm->screen_rgba = malloc(kScreenWidth * kScreenHeight * sizeof(uint32_t));

  ei->screen = gm->screen_manager.screen;
  ei->palette = gm->screen_manager.palette;

  ei->mouse_x = &gm->input_manager.mouse_x;
  ei->mouse_y = &gm->input_manager.mouse_y;

  ei->player1_buttons = &gm->input_manager.buttons[0];
  ei->player1_joy_x = &gm->input_manager.joy_x[0];
  ei->player1_joy_y = &gm->input_manager.joy_y[0];

  ei->player2_buttons = &gm->input_manager.buttons[1];
  ei->player2_joy_x = &gm->input_manager.joy_x[1];
  ei->player2_joy_y = &gm->input_manager.joy_y[1];

  ei->player3_buttons = &gm->input_manager.buttons[2];
  ei->player3_joy_x = &gm->input_manager.joy_x[2];
  ei->player3_joy_y = &gm->input_manager.joy_y[2];

  ei->player4_buttons = &gm->input_manager.buttons[3];
  ei->player4_joy_x = &gm->input_manager.joy_x[3];
  ei->player4_joy_y = &gm->input_manager.joy_y[3];

  //memset(&ei->input, 0, sizeof(ei->input));
}

static void
exotique_unload(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  free(sm->screen);
  free(sm->screen_rgba);
}

static void
exotique_update(GameManager* gm)
{
  gm->ticks = SDL_GetTicks64();

  SDL_GetMouseState(&gm->input_manager.mouse_x, &gm->input_manager.mouse_y);
  //SDL_GetGlobalMouseState(&gm->input_manager.mouse_x, &gm->input_manager.mouse_y);

  exotique_events(gm);
}

// XXX: SDL functions

static void
sdl_load(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  puts("[INFO]: SDL2 initialization");
  sdl_panic_check(SDL_Init(SDL_INIT_VIDEO), "Init");

  sm->window = SDL_CreateWindow(gm->name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, kScreenWidth, kScreenHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_UTILITY);
  sdl_panic_check(!sm->window, "CreateWindow");

  sm->renderer = SDL_CreateRenderer(sm->window, -1, /*0*/SDL_RENDERER_PRESENTVSYNC);
  sdl_panic_check(!sm->renderer, "CreateRenderer");

  sm->texture = SDL_CreateTexture(sm->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, kScreenWidth, kScreenHeight);
  sdl_panic_check(!sm->texture, "CreateTexture");

  // These calls are optional.
  // They only serves to make the pixels square and clean when resizing the window.
  sdl_panic_check(SDL_RenderSetLogicalSize(sm->renderer, kScreenWidth, kScreenHeight), "RenderSetLogicalSize");
  sdl_panic_check(SDL_RenderSetIntegerScale(sm->renderer, SDL_TRUE), "RenderSetIntegerScale");
}

static void
sdl_unload(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  SDL_DestroyTexture(sm->texture);
  SDL_DestroyRenderer(sm->renderer);
  SDL_DestroyWindow(sm->window);
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

  // XXX: Main loop
  while (!g_game_manager.exit)
  {
    exotique_update(&g_game_manager);
    game_update(&g_exotique_interface);
    game_draw(&g_exotique_interface);
    exotique_draw(&g_game_manager);
  }

  sdl_unload(&g_game_manager);
  exotique_unload(&g_game_manager);
  SDL_Quit();

  return EXIT_SUCCESS;
}
