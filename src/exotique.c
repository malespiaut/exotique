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
#pragma GCC diagnostic pop

#include "exotique.h"

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

// XXX: Screen constants

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
  eKey_select,
  eKey_start,
  eKey_a,
  eKey_b,
  eKey_x,
  eKey_y,
  eKey_l1,
  eKey_r1,
  eKey_l2,
  eKey_r2,
  eKey_l3,
  eKey_r3,
  eKey__COUNT
}; /* There are 16 buttons. That hopefully should be way more than enough! */
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
  SDL_Scancode key_map[16]; // 16 buttons
  uint8_t key_states[16];   // 16 buttons
  SDL_GameController* controllers[4];
  ExotiqueInterface ei;
};

// XXX: Global data structure

static GameManager g_game_manager = {0};

// XXX: game.c mandatory functions declarations

void game_init(ExotiqueInterface* ei);
void game_update(ExotiqueInterface* ei);
void game_draw(ExotiqueInterface* ei);

// XXX: Functions declarations

static void exotique_panic(GameManager* gm);

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

static bool
key_new_get(GameManager* gm, int32_t key)
{
  return ((gm->key_states[key] == eKeyState_pressed) || (gm->key_states[key] == eKeyState_held));
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

      case SDL_CONTROLLERDEVICEADDED:
        for (int i = 0; i < 4; ++i)
        {
          if (!gm->controllers[i])
          {
            gm->controllers[i] = SDL_GameControllerOpen(event.cdevice.which);
            break;
          }
        }
        break;

      case SDL_CONTROLLERDEVICEREMOVED:
        {
          SDL_GameController* controller = SDL_GameControllerFromInstanceID(event.cdevice.which);
          for (int i = 0; i < 4; ++i)
          {
            if (gm->controllers[i] == controller)
            {
              SDL_GameControllerClose(controller);
              gm->controllers[i] = NULL;
              break;
            }
          }
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
        ei->mouse_click = 1;
        break;

      case SDL_MOUSEBUTTONUP:
        ei->mouse_click = 0;
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

  for (size_t i = 0; i < eKey__COUNT; ++i)
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
exotique_init(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  gm->name = "🌴 Exotique v0.8β - SDL2 (26/05/17)";

  sm->screen = malloc((unsigned long)kScreenPixels * sizeof(uint8_t));
  sm->screen_rgba = malloc((unsigned long)kScreenPixels * sizeof(uint32_t));

  gm->key_map[eKey_up] = SDL_SCANCODE_UP;
  gm->key_map[eKey_down] = SDL_SCANCODE_DOWN;
  gm->key_map[eKey_left] = SDL_SCANCODE_LEFT;
  gm->key_map[eKey_right] = SDL_SCANCODE_RIGHT;
  gm->key_map[eKey_select] = SDL_SCANCODE_SPACE;
  gm->key_map[eKey_start] = SDL_SCANCODE_RETURN;
  gm->key_map[eKey_a] = SDL_SCANCODE_A;
  gm->key_map[eKey_b] = SDL_SCANCODE_B;
  gm->key_map[eKey_x] = SDL_SCANCODE_X;
  gm->key_map[eKey_y] = SDL_SCANCODE_Y;
  gm->key_map[eKey_l1] = SDL_SCANCODE_1;
  gm->key_map[eKey_r1] = SDL_SCANCODE_6;
  gm->key_map[eKey_l2] = SDL_SCANCODE_2;
  gm->key_map[eKey_r2] = SDL_SCANCODE_5;
  gm->key_map[eKey_l3] = SDL_SCANCODE_3;
  gm->key_map[eKey_r3] = SDL_SCANCODE_4;

  ei->screen = gm->screen_manager.screen;
  ei->palette = gm->screen_manager.palette;

  memset(&ei->mouse, 0, sizeof(ei->mouse));
  memset(&ei->input, 0, sizeof(ei->input));
  memset(gm->controllers, 0, sizeof(gm->controllers));
}

static void
exotique_unload(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  for (int i = 0; i < 4; ++i)
  {
    if (gm->controllers[i])
    {
      SDL_GameControllerClose(gm->controllers[i]);
      gm->controllers[i] = NULL;
    }
  }

  free(sm->screen);
  free(sm->screen_rgba);
}

static void
exotique_update(GameManager* gm, ExotiqueInterface* ei)
{
  ei->ticks = SDL_GetTicks64();

  memset(&ei->mouse, 0, sizeof(ei->mouse));
  SDL_GetMouseState(&ei->mouse.x, &ei->mouse.y);
  // SDL_GetGlobalMouseState(&gm->input_manager.mouse_x, &gm->input_manager.mouse_y);

  memset(&ei->input, 0, sizeof(ei->input));
  exotique_events(gm);

  // Keyboard updates input[0]
  if (key_new_get(gm, eKey_up))
  {
    ei->input[0].up = 1;
  }
  if (key_new_get(gm, eKey_down))
  {
    ei->input[0].down = 1;
  }
  if (key_new_get(gm, eKey_left))
  {
    ei->input[0].left = 1;
  }
  if (key_new_get(gm, eKey_right))
  {
    ei->input[0].right = 1;
  }
  if (key_new_get(gm, eKey_select))
  {
    ei->input[0].select = 1;
  }
  if (key_new_get(gm, eKey_start))
  {
    ei->input[0].start = 1;
  }
  if (key_new_get(gm, eKey_a))
  {
    ei->input[0].a = 1;
  }
  if (key_new_get(gm, eKey_b))
  {
    ei->input[0].b = 1;
  }
  if (key_new_get(gm, eKey_x))
  {
    ei->input[0].x = 1;
  }
  if (key_new_get(gm, eKey_y))
  {
    ei->input[0].y = 1;
  }
  if (key_new_get(gm, eKey_l1))
  {
    ei->input[0].l1 = 1;
  }
  if (key_new_get(gm, eKey_r1))
  {
    ei->input[0].r1 = 1;
  }
  if (key_new_get(gm, eKey_l2))
  {
    ei->input[0].l2 = 1;
  }
  if (key_new_get(gm, eKey_r2))
  {
    ei->input[0].r2 = 1;
  }
  if (key_new_get(gm, eKey_l3))
  {
    ei->input[0].l3 = 1;
  }
  if (key_new_get(gm, eKey_r3))
  {
    ei->input[0].r3 = 1;
  }

  // Gamepads update input[0..3]
  for (int i = 0; i < 4; ++i)
  {
    SDL_GameController* pad = gm->controllers[i];
    if (!pad)
    {
      continue;
    }

    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP))
    {
      ei->input[i].up = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
    {
      ei->input[i].down = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
    {
      ei->input[i].left = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
    {
      ei->input[i].right = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_BACK))
    {
      ei->input[i].select = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_START))
    {
      ei->input[i].start = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_A))
    {
      ei->input[i].a = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_B))
    {
      ei->input[i].b = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_X))
    {
      ei->input[i].x = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_Y))
    {
      ei->input[i].y = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
    {
      ei->input[i].l1 = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
    {
      ei->input[i].r1 = 1;
    }
    if (SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 16384)
    {
      ei->input[i].l2 = 1;
    }
    if (SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16384)
    {
      ei->input[i].r2 = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_LEFTSTICK))
    {
      ei->input[i].l3 = 1;
    }
    if (SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_RIGHTSTICK))
    {
      ei->input[i].r3 = 1;
    }

    ei->input[i].joystick.x = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX);
    ei->input[i].joystick.y = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY);
  }
}

// XXX: SDL functions

static void
sdl2_init(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
  SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "SDL2 initialization");
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER))
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
sdl2_unload(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  SDL_DestroyTexture(sm->texture);
  SDL_DestroyRenderer(sm->renderer);
  SDL_DestroyWindow(sm->window);
}

static void
exotique_cleanup(GameManager* gm)
{
  sdl2_unload(gm);
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
