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
#include <GL/glut.h>
#include <GL/gl.h>
#pragma GCC diagnostic pop

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

/* XXX: Screen and sound constants */

extern const int kScreenWidth;
extern const int kScreenHeight;
#define kScreenPixels (kScreenWidth * kScreenHeight)

#define kSoundSampleRate 44100
#define kSoundSamples 2048

/* XXX: Data structures */

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
  eKeyState_off = 0x0,       /* 0b00 */
  eKeyState_up = 0x1,        /* 0b01 */
  eKeyState_pressed = 0x2,   /* 0b10 */
  eKeyState_held = 0x3,      /* 0b11 */
  eKeyState_active_bit = 0x2 /* 0b10 */
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
  GLuint texture_id;
  bool fullscreen;
  uint8_t* screen;
  uint32_t* screen_rgba;
  uint32_t palette[256];
  int window_id;
};

typedef struct SoundManager SoundManager;
struct SoundManager
{
  ma_device device;
  ma_device_config device_config;
  int16_t* sound_buffer;
  int16_t* internal_buffer;
  uint32_t buffer_pos;
  bool initialized;
};

typedef struct GameManager GameManager;
struct GameManager
{
  const char* name;
  ScreenManager screen_manager;
  SoundManager sound_manager;
  bool exit;
  int glut_key_map[7]; /* 7 = eKey_count */
  uint8_t key_states[7];   /* 7 = eKey_count */
  int mouse_x;
  int mouse_y;
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
  uint8_t* screen;   /* [kScreenWidth * kScreenHeight] */
  uint32_t* palette; /* [256] */
  int16_t* sound;    /* [kSoundSamples] */

  vec2i_t mouse;
  PlayerInput player[4];

  uint64_t ticks;
  uint32_t sound_sample_rate;
  uint32_t sound_samples;
};

/* XXX: Global data structure */

static GameManager g_game_manager = {0};
ExotiqueInterface g_exotique_interface = {0};

/* XXX: game.c mandatory functions declarations */

void game_load(ExotiqueInterface* ei);
void game_update(ExotiqueInterface* ei);
void game_draw(ExotiqueInterface* ei);

/* XXX: Functions declarations */

static void exotique_panic(GameManager* gm) __attribute__((unused));
static void sound_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

/* XXX: Input functions */

static void
bit_set(uint32_t* data, uint8_t n)
{
  if (n < 32) /* Ensure n is within 0-31 */
  {
    *data |= ((uint32_t)1 << n);
  }
}

static void
key_state_update(uint8_t* state, bool is_down)
{
  switch (*state) /* look at prev state */
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

/* XXX: Game functions */

static void
exotique_draw(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;
  int32_t i;

  /* Convert palette indexed pixels to RGBA */
  for (i = 0; i < kScreenPixels; ++i)
  {
    sm->screen_rgba[i] = sm->palette[sm->screen[i]];
  }

  /* Clear the screen */
  glClear(GL_COLOR_BUFFER_BIT);
  
  /* Setup OpenGL state for 2D rendering */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, kScreenWidth, kScreenHeight, 0, -1, 1);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  
  /* Bind and update texture */
  glBindTexture(GL_TEXTURE_2D, sm->texture_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kScreenWidth, kScreenHeight, 0, 
               GL_RGBA, GL_UNSIGNED_BYTE, sm->screen_rgba);
  
  /* Draw textured quad */
  glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f((float)kScreenWidth, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f((float)kScreenWidth, (float)kScreenHeight);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, (float)kScreenHeight);
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
  
  glutSwapBuffers();
}

static void
exotique_update_input(GameManager* gm, ExotiqueInterface* ei)
{
  memset(&ei->mouse, 0, sizeof(ei->mouse));
  ei->mouse.x = gm->mouse_x;
  ei->mouse.y = gm->mouse_y;

  memset(&ei->player, 0, sizeof(ei->player));
  
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

static void
sound_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
  SoundManager* sound_manager = (SoundManager*)pDevice->pUserData;
  int16_t* output = (int16_t*)pOutput;
  uint32_t i;
  
  (void)pInput; /* Unused parameter */
  
  for (i = 0; i < frameCount; ++i)
  {
    int16_t sample = 0;
    if (sound_manager->buffer_pos < kSoundSamples)
    {
      sample = sound_manager->internal_buffer[sound_manager->buffer_pos];
      sound_manager->buffer_pos++;
    }
    output[i] = sample;
  }
}

static void
exotique_load(GameManager* gm, ExotiqueInterface* ei)
{
  ScreenManager* sm = &gm->screen_manager;
  SoundManager* sound_mgr = &gm->sound_manager;

  gm->name = "🌴 Exotique v0.5β - OpenGL1.1/FreeGLUT (25/05/09)";

  sm->screen = malloc((unsigned long)kScreenPixels * sizeof(uint8_t));
  sm->screen_rgba = malloc((unsigned long)kScreenPixels * sizeof(uint32_t));
  
  sound_mgr->sound_buffer = malloc(kSoundSamples * sizeof(int16_t));
  sound_mgr->internal_buffer = malloc(kSoundSamples * sizeof(int16_t));
  memset(sound_mgr->sound_buffer, 0, kSoundSamples * sizeof(int16_t));
  memset(sound_mgr->internal_buffer, 0, kSoundSamples * sizeof(int16_t));
  sound_mgr->buffer_pos = 0;
  sound_mgr->initialized = false;

  gm->glut_key_map[eKey_up] = GLUT_KEY_UP;
  gm->glut_key_map[eKey_down] = GLUT_KEY_DOWN;
  gm->glut_key_map[eKey_left] = GLUT_KEY_LEFT;
  gm->glut_key_map[eKey_right] = GLUT_KEY_RIGHT;
  gm->glut_key_map[eKey_shoot] = 13; /* Enter key */
  gm->glut_key_map[eKey_cancel] = 27; /* Escape key */
  gm->glut_key_map[eKey_pause] = 32; /* Space key */

  ei->screen = gm->screen_manager.screen;
  ei->palette = gm->screen_manager.palette;
  ei->sound = sound_mgr->sound_buffer;
  ei->sound_sample_rate = kSoundSampleRate;
  ei->sound_samples = kSoundSamples;

  memset(&ei->mouse, 0, sizeof(ei->mouse));
  memset(&ei->player, 0, sizeof(ei->player));
}

static void
exotique_unload(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;
  SoundManager* sound_mgr = &gm->sound_manager;

  if (sound_mgr->initialized)
  {
    ma_device_uninit(&sound_mgr->device);
  }
  
  free(sm->screen);
  free(sm->screen_rgba);
  free(sound_mgr->sound_buffer);
  free(sound_mgr->internal_buffer);
}

static void
exotique_update(GameManager* gm, ExotiqueInterface* ei)
{
  SoundManager* sound_mgr = &gm->sound_manager;
  
  ei->ticks = (uint64_t)(glutGet(GLUT_ELAPSED_TIME));
  
  exotique_update_input(gm, ei);
  
  memcpy(sound_mgr->internal_buffer, ei->sound, kSoundSamples * sizeof(int16_t));
  sound_mgr->buffer_pos = 0;
}

/* XXX: GLUT callback functions */

static void
glut_display_callback(void)
{
  if (!g_game_manager.exit)
  {
    exotique_update(&g_game_manager, &g_exotique_interface);
    game_update(&g_exotique_interface);
    game_draw(&g_exotique_interface);
    exotique_draw(&g_game_manager);
  }
}

static void
glut_idle_callback(void)
{
  glutPostRedisplay();
}

static void
glut_keyboard_callback(unsigned char key, int x, int y)
{
  GameManager* gm = &g_game_manager;
  size_t i;
  
  (void)x; (void)y; /* Unused parameters */
  
  if (key == 27) /* Escape */
  {
    gm->exit = true;
    return;
  }
  
  /* Update key states for regular keys */
  for (i = 0; i < eKey_count; ++i)
  {
    if (gm->glut_key_map[i] == (int)key)
    {
      key_state_update(&gm->key_states[i], true);
      break;
    }
  }
}

static void
glut_keyboard_up_callback(unsigned char key, int x, int y)
{
  GameManager* gm = &g_game_manager;
  size_t i;
  
  (void)x; (void)y; /* Unused parameters */
  
  /* Update key states for regular keys */
  for (i = 0; i < eKey_count; ++i)
  {
    if (gm->glut_key_map[i] == (int)key)
    {
      key_state_update(&gm->key_states[i], false);
      break;
    }
  }
}

static void
glut_special_callback(int key, int x, int y)
{
  GameManager* gm = &g_game_manager;
  size_t i;
  
  (void)x; (void)y; /* Unused parameters */
  
  /* Update key states for special keys */
  for (i = 0; i < eKey_count; ++i)
  {
    if (gm->glut_key_map[i] == key)
    {
      key_state_update(&gm->key_states[i], true);
      break;
    }
  }
}

static void
glut_special_up_callback(int key, int x, int y)
{
  GameManager* gm = &g_game_manager;
  size_t i;
  
  (void)x; (void)y; /* Unused parameters */
  
  /* Update key states for special keys */
  for (i = 0; i < eKey_count; ++i)
  {
    if (gm->glut_key_map[i] == key)
    {
      key_state_update(&gm->key_states[i], false);
      break;
    }
  }
}

static void
glut_mouse_callback(int button, int state, int x, int y)
{
  GameManager* gm = &g_game_manager;
  
  (void)button; (void)state; /* Unused parameters for now */
  
  gm->mouse_x = x;
  gm->mouse_y = y;
}

static void
glut_motion_callback(int x, int y)
{
  GameManager* gm = &g_game_manager;
  
  gm->mouse_x = x;
  gm->mouse_y = y;
}

static void
glut_passive_motion_callback(int x, int y)
{
  GameManager* gm = &g_game_manager;
  
  gm->mouse_x = x;
  gm->mouse_y = y;
}

/* static void
glut_close_callback(void)
{
  g_game_manager.exit = true;
} */

/* XXX: OpenGL and audio setup functions */

static void
opengl_load(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;
  int argc = 1;
  char* argv[] = {"exotique_ogl1", NULL};

  /* Initialize GLUT */
  glutInit(&argc, argv);
  
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(kScreenWidth, kScreenHeight);
  sm->window_id = glutCreateWindow(gm->name);
  
  /* Set up callbacks */
  glutDisplayFunc(glut_display_callback);
  glutIdleFunc(glut_idle_callback);
  glutKeyboardFunc(glut_keyboard_callback);
  glutKeyboardUpFunc(glut_keyboard_up_callback);
  glutSpecialFunc(glut_special_callback);
  glutSpecialUpFunc(glut_special_up_callback);
  glutMouseFunc(glut_mouse_callback);
  glutMotionFunc(glut_motion_callback);
  glutPassiveMotionFunc(glut_passive_motion_callback);
  /* glutCloseFunc(glut_close_callback); */ /* Not available in all GLUT versions */
  
  /* Setup OpenGL state */
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glDisable(GL_DEPTH_TEST);
  
  /* Generate texture for screen buffer */
  glGenTextures(1, &sm->texture_id);
  glBindTexture(GL_TEXTURE_2D, sm->texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

static void
audio_load(GameManager* gm)
{
  SoundManager* sound_mgr = &gm->sound_manager;
  
  sound_mgr->device_config = ma_device_config_init(ma_device_type_playback);
  sound_mgr->device_config.playback.format = ma_format_s16;
  sound_mgr->device_config.playback.channels = 1;
  sound_mgr->device_config.sampleRate = kSoundSampleRate;
  sound_mgr->device_config.dataCallback = sound_callback;
  sound_mgr->device_config.pUserData = sound_mgr;
  
  if (ma_device_init(NULL, &sound_mgr->device_config, &sound_mgr->device) != MA_SUCCESS)
  {
    printf("Warning: Failed to initialize audio device\n");
    sound_mgr->initialized = false;
    return;
  }
  
  if (ma_device_start(&sound_mgr->device) != MA_SUCCESS)
  {
    printf("Warning: Failed to start audio device\n");
    ma_device_uninit(&sound_mgr->device);
    sound_mgr->initialized = false;
    return;
  }
  
  sound_mgr->initialized = true;
}

static void
opengl_unload(GameManager* gm)
{
  ScreenManager* sm = &gm->screen_manager;
  
  glDeleteTextures(1, &sm->texture_id);
  glutDestroyWindow(sm->window_id);
}

static void
exotique_cleanup(GameManager* gm)
{
  opengl_unload(gm);
  exotique_unload(gm);
}

static void
exotique_panic(GameManager* gm)
{
  printf("Exotique panic - cleaning up and exiting\n");
  exotique_cleanup(gm);
  exit(EXIT_FAILURE);
}

/* XXX: Main function */

int
main(const int argc, const char* argv[])
{
  (void)argc;
  (void)argv;

  exotique_load(&g_game_manager, &g_exotique_interface);
  game_load(&g_exotique_interface);
  opengl_load(&g_game_manager);
  audio_load(&g_game_manager);

  /* Main loop */
  glutMainLoop();

  exotique_cleanup(&g_game_manager);
  return EXIT_SUCCESS;
}