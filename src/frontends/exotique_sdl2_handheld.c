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
//#include <SDL_image.h>
#pragma GCC diagnostic pop

// XXX: Screen constants

extern const int kScreenWidth;
extern const int kScreenHeight;
#define kScreenPixels (kScreenWidth * kScreenHeight)

// XXX: Data structures

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
  SDL_GameController* game_controller;
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
  unsigned up : 1;
  unsigned down : 1;
  unsigned left : 1;
  unsigned right : 1;
  unsigned select : 1;
  unsigned start : 1;
  unsigned a : 1;
  unsigned b : 1;
  unsigned x : 1;
  unsigned y : 1;
  unsigned l1 : 1;
  unsigned r1 : 1;
  unsigned l2 : 1;
  unsigned r2 : 1;
  unsigned l3 : 1;
  unsigned r3 : 1;
  vec2i_t joystick;
};

typedef struct ExotiqueInterface ExotiqueInterface;
struct ExotiqueInterface
{
  uint8_t* screen;   //[kScreenWidth * kScreenHeight];
  uint32_t* palette; //[256];

  vec2i_t mouse;
  PlayerInput input[4];

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
}

static void
exotique_load(GameManager* gm, ExotiqueInterface* ei)
{
  ScreenManager* sm = &gm->screen_manager;

  gm->name = "🌴 Exotique v0.6β - SDL2 (25/08/13)";

  sm->screen = malloc((unsigned long)kScreenPixels * sizeof(uint8_t));
  sm->screen_rgba = malloc((unsigned long)kScreenPixels * sizeof(uint32_t));

  ei->screen = gm->screen_manager.screen;
  ei->palette = gm->screen_manager.palette;

  memset(&ei->mouse, 0, sizeof(ei->mouse));
  memset(&ei->input, 0, sizeof(ei->input));
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
  // SDL_GetGlobalMouseState(&gm->input_manager.mouse_x, &gm->input_manager.mouse_y);

  memset(&ei->input, 0, sizeof(ei->input));
  exotique_events(gm);

  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_DPAD_UP))
  {
    ei->input[0].up = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
  {
    ei->input[0].down = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
  {
    ei->input[0].left = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
  {
    ei->input[0].right = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_BACK))
  {
    ei->input[0].select = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_START))
  {
    ei->input[0].start = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_A))
  {
    ei->input[0].a = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_B))
  {
    ei->input[0].b = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_X))
  {
    ei->input[0].x = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_Y))
  {
    ei->input[0].y = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))
  {
    ei->input[0].l1 = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER))
  {
    ei->input[0].r1 = 1;
  }
  /*
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_l2))
  {
    ei->input[0].l2 = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_r2))
  {
    ei->input[0].r2 = 1;
  }
  */
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_LEFTSTICK))
  {
    ei->input[0].l3 = 1;
  }
  if (SDL_GameControllerGetButton(gm->game_controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK))
  {
    ei->input[0].r3 = 1;
  }
  }


// XXX: SDL functions

static void
sdl_load(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
  SDL_LogInfo(SDL_LOG_CATEGORY_SYSTEM, "SDL2 initialization");
  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER))
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Couldn't initialize the SDL library: %s", SDL_GetError());
    exotique_panic(gm);
  }

  if (!(sm->window = SDL_CreateWindow(gm->name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, kScreenWidth, kScreenHeight, SDL_WINDOW_SHOWN)))
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

  if (!(gm->game_controller = SDL_GameControllerOpen(0)))
  {
    SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Couldn't open a game controller for use: %s", SDL_GetError());
  }
}

static void
sdl_unload(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;

  SDL_GameControllerClose(gm->game_controller);
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
