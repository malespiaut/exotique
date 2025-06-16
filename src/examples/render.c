/*
render.c
Benedict Henshaw, 2024
benedicthenshaw.com
cc render.c -lm -lSDL2
*/

#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int vert_count = 190;
int face_count = 282;
extern float vertices[];
extern int indices[];
extern uint32_t colours[];

typedef struct
{
  float x, y, z, w;
} vec_t;

typedef struct
{
  float m[4][4];
} mat_t;

struct
{
  uint32_t* pixels;
  float* depth;
  int width, height;
  mat_t model, view, proj;
  vec_t camera;
} render;

/* clang-format off */
float vertices[] = {
  0.090f,  1.910f,  -0.070f, 0.100f,  1.800f,  -0.070f, 0.000f,  1.950f,
  -0.030f, 0.000f,  1.950f,  0.070f,  -0.090f, 1.910f,  -0.070f, 0.000f,
  1.900f,  -0.110f, 0.000f,  1.770f,  -0.120f, -0.100f, 1.800f,  -0.070f,
  -0.090f, 1.910f,  0.060f,  -0.100f, 1.800f,  0.070f,  0.000f,  1.900f,
  0.100f,  0.000f,  1.780f,  0.110f,  0.000f,  1.700f,  -0.080f, 0.000f,
  1.670f,  0.080f,  -0.080f, 1.710f,  -0.000f, -0.320f, 0.850f,  -0.040f,
  -0.320f, 0.850f,  0.090f,  -0.390f, 0.880f,  -0.030f, -0.300f, 0.970f,
  -0.060f, -0.300f, 0.940f,  0.070f,  -0.380f, 0.890f,  0.090f,  -0.250f,
  1.540f,  -0.080f, -0.240f, 1.540f,  0.060f,  0.000f,  1.620f,  -0.100f,
  -0.360f, 1.000f,  -0.030f, -0.290f, 0.990f,  0.070f,  -0.360f, 1.010f,
  0.070f,  -0.240f, 1.190f,  -0.090f, -0.300f, 1.250f,  -0.090f, -0.070f,
  1.660f,  -0.000f, -0.220f, 1.210f,  0.030f,  -0.230f, 1.240f,  -0.080f,
  -0.290f, 1.260f,  0.010f,  -0.290f, 1.290f,  -0.080f, -0.170f, 1.620f,
  -0.080f, -0.130f, 1.620f,  0.070f,  0.000f,  1.450f,  0.160f,  0.000f,
  1.467f,  -0.143f, -0.170f, 1.450f,  -0.110f, -0.150f, 1.460f,  0.110f,
  0.000f,  1.220f,  0.150f,  0.000f,  1.220f,  -0.090f, -0.140f, 1.230f,
  -0.050f, -0.130f, 1.230f,  0.110f,  -0.190f, 0.940f,  -0.080f, 0.000f,
  0.930f,  0.140f,  0.000f,  1.050f,  -0.140f, 0.000f,  1.040f,  0.150f,
  -0.185f, 1.060f,  -0.100f, -0.175f, 1.070f,  0.130f,  -0.210f, 0.450f,
  -0.070f, -0.060f, 0.450f,  -0.070f, -0.190f, 0.550f,  0.110f,  -0.060f,
  0.550f,  0.110f,  -0.200f, 0.040f,  0.210f,  -0.140f, 0.040f,  0.210f,
  -0.130f, 0.000f,  0.100f,  -0.200f, 0.110f,  -0.050f, -0.230f, 0.100f,
  0.080f,  -0.130f, 0.110f,  0.050f,  -0.140f, 0.110f,  -0.050f, -0.050f,
  0.590f,  -0.020f, -0.050f, 0.610f,  0.130f,  -0.180f, 0.610f,  0.130f,
  -0.200f, 0.590f,  -0.020f, 0.090f,  1.910f,  0.060f,  0.100f,  1.800f,
  0.060f,  0.000f,  1.680f,  0.110f,  -0.070f, 1.700f,  0.096f,  0.000f,
  1.620f,  0.090f,  0.000f,  0.930f,  -0.100f, -0.230f, 0.000f,  0.080f,
  -0.220f, -0.000f, -0.070f, -0.130f, -0.000f, -0.050f, -0.120f, 0.000f,
  0.230f,  -0.230f, 0.000f,  0.230f,  -0.220f, 0.100f,  -0.080f, -0.210f,
  0.110f,  0.050f,  -0.110f, 0.100f,  -0.080f, -0.100f, 0.100f,  0.080f,
  0.080f,  1.710f,  -0.000f, 0.000f,  1.180f,  -0.140f, 0.000f,  1.180f,
  0.160f,  -0.160f, 1.180f,  0.120f,  0.320f,  0.850f,  -0.040f, 0.320f,
  0.850f,  0.090f,  0.390f,  0.880f,  -0.030f, 0.300f,  0.970f,  -0.060f,
  0.300f,  0.940f,  0.070f,  0.380f,  0.890f,  0.090f,  -0.350f, 1.000f,
  -0.020f, -0.350f, 1.010f,  0.040f,  -0.310f, 0.940f,  -0.040f, -0.310f,
  0.980f,  0.050f,  -0.340f, 1.040f,  -0.020f, -0.340f, 1.050f,  0.040f,
  -0.300f, 1.000f,  -0.040f, -0.300f, 1.020f,  0.050f,  0.250f,  1.540f,
  -0.080f, 0.240f,  1.540f,  0.060f,  -0.170f, 1.180f,  -0.060f, 0.360f,
  1.000f,  -0.030f, 0.290f,  0.990f,  0.070f,  0.360f,  1.010f,  0.070f,
  0.240f,  1.190f,  -0.090f, 0.300f,  1.250f,  -0.090f, 0.070f,  1.660f,
  -0.000f, 0.220f,  1.210f,  0.030f,  0.230f,  1.240f,  -0.080f, 0.290f,
  1.260f,  0.010f,  0.290f,  1.290f,  -0.080f, 0.170f,  1.620f,  -0.080f,
  0.130f,  1.620f,  0.070f,  0.170f,  1.450f,  -0.110f, 0.150f,  1.460f,
  0.110f,  0.140f,  1.230f,  -0.050f, 0.130f,  1.230f,  0.110f,  0.190f,
  0.940f,  -0.080f, 0.185f,  1.060f,  -0.100f, 0.175f,  1.070f,  0.130f,
  -0.075f, 1.817f,  0.079f,  -0.043f, 1.802f,  0.102f,  -0.043f, 1.830f,
  0.102f,  -0.012f, 1.817f,  0.105f,  0.210f,  0.450f,  -0.070f, 0.060f,
  0.450f,  -0.070f, 0.190f,  0.550f,  0.110f,  0.060f,  0.550f,  0.110f,
  0.200f,  0.040f,  0.210f,  0.140f,  0.040f,  0.210f,  0.130f,  0.000f,
  0.100f,  0.200f,  0.110f,  -0.050f, 0.230f,  0.100f,  0.080f,  0.130f,
  0.110f,  0.050f,  0.140f,  0.110f,  -0.050f, 0.050f,  0.590f,  -0.020f,
  0.050f,  0.610f,  0.130f,  0.180f,  0.610f,  0.130f,  0.200f,  0.590f,
  -0.020f, 0.070f,  1.700f,  0.100f,  0.230f,  0.000f,  0.080f,  0.220f,
  -0.000f, -0.070f, 0.130f,  -0.000f, -0.050f, 0.120f,  0.000f,  0.230f,
  0.000f,  1.847f,  0.103f,  0.220f,  0.100f,  -0.080f, 0.230f,  0.000f,
  0.230f,  0.000f,  1.762f,  0.108f,  -0.021f, 1.773f,  0.108f,  0.000f,
  1.769f,  0.145f,  0.210f,  0.110f,  0.050f,  0.110f,  0.100f,  -0.080f,
  0.100f,  0.100f,  0.080f,  0.160f,  1.180f,  0.120f,  0.350f,  1.000f,
  -0.020f, 0.350f,  1.010f,  0.040f,  0.310f,  0.940f,  -0.040f, 0.310f,
  0.980f,  0.050f,  0.340f,  1.040f,  -0.020f, 0.340f,  1.050f,  0.040f,
  0.300f,  1.000f,  -0.040f, 0.300f,  1.020f,  0.050f,  0.170f,  1.180f,
  -0.060f, 0.043f,  1.830f,  0.102f,  0.012f,  1.817f,  0.105f,  0.043f,
  1.802f,  0.102f,  0.021f,  1.773f,  0.108f,  0.075f,  1.817f,  0.079f,
  -0.075f, 1.830f,  0.082f,  -0.044f, 1.838f,  0.097f,  -0.044f, 1.845f,
  0.095f,  -0.015f, 1.835f,  0.104f,  0.044f,  1.838f,  0.097f,  0.015f,
  1.835f,  0.104f,  0.044f,  1.845f,  0.095f,  0.075f,  1.830f,  0.082f,
  -0.035f, 1.735f,  0.106f,  -0.000f, 1.721f,  0.116f,  0.035f,  1.735f,
  0.106f,  0.000f,  1.721f,  0.116f,  0.000f,  1.749f,  0.116f,  0.000f,
  1.721f,  0.116f,  -0.057f, 1.816f,  0.097f,  -0.044f, 1.803f,  0.104f,
  -0.044f, 1.829f,  0.105f,  -0.030f, 1.815f,  0.104f,  0.030f,  1.815f,
  0.104f,  0.044f,  1.829f,  0.105f,  0.044f,  1.803f,  0.104f,  0.057f,
  1.816f,  0.097f,
};

int indices[] = {
  5,   1,   6,   65,  2,   3,   99,  110, 98,  130, 146, 143, 101, 104, 105,
  136, 135, 127, 105, 108, 110, 150, 141, 131, 152, 125, 151, 66,  10,  11,
  12,  1,   80,  137, 45,  136, 160, 154, 156, 98,  111, 99,  155, 86,  154,
  10,  65,  3,   106, 112, 111, 5,   2,   0,   109, 101, 105, 117, 118, 119,
  108, 104, 107, 80,  66,  139, 86,  85,  84,  86,  156, 154, 13,  139, 67,
  132, 124, 126, 106, 12,  80,  138, 137, 126, 128, 140, 150, 117, 137, 138,
  135, 117, 138, 151, 124, 145, 45,  135, 136, 111, 23,  106, 114, 108, 107,
  45,  119, 47,  157, 89,  155, 112, 106, 69,  89,  88,  85,  119, 40,  47,
  129, 150, 133, 140, 142, 141, 111, 98,  113, 118, 70,  46,  142, 133, 134,
  131, 142, 134, 99,  112, 114, 139, 11,  67,  113, 23,  111, 130, 129, 133,
  110, 109, 105, 143, 128, 129, 118, 41,  115, 1,   65,  66,  107, 103, 109,
  113, 81,  37,  106, 13,  69,  114, 109, 99,  99,  111, 112, 138, 125, 135,
  153, 36,  82,  113, 110, 108, 162, 114, 153, 152, 126, 127, 127, 137, 136,
  158, 155, 154, 119, 115, 116, 138, 126, 124, 104, 102, 107, 10,  9,   11,
  7,   5,   6,   8,   2,   4,   5,   4,   2,   10,  3,   8,   8,   7,   9,
  7,   12,  14,  14,  9,   7,   68,  11,  9,   13,  68,  14,  38,  81,  100,
  83,  36,  39,  39,  100, 83,  29,  12,  23,  13,  29,  69,  61,  45,  62,
  63,  45,  49,  44,  61,  64,  63,  44,  64,  92,  17,  90,  19,  15,  92,
  20,  16,  19,  17,  16,  20,  93,  19,  92,  17,  91,  90,  20,  93,  91,
  38,  33,  21,  39,  32,  30,  34,  38,  21,  22,  33,  32,  22,  39,  35,
  39,  69,  35,  38,  23,  37,  34,  29,  23,  35,  69,  29,  29,  34,  35,
  27,  25,  18,  32,  24,  26,  27,  24,  28,  30,  26,  25,  31,  30,  27,
  33,  28,  32,  31,  28,  33,  39,  31,  38,  40,  49,  47,  48,  41,  46,
  49,  42,  48,  70,  48,  46,  44,  49,  48,  45,  47,  49,  61,  53,  51,
  62,  53,  61,  64,  50,  52,  64,  52,  63,  51,  79,  78,  58,  50,  76,
  64,  51,  50,  50,  78,  76,  52,  79,  53,  63,  53,  62,  73,  71,  72,
  56,  75,  71,  77,  72,  71,  59,  73,  60,  55,  56,  59,  71,  54,  77,
  54,  74,  55,  77,  55,  59,  73,  57,  60,  135, 125, 127, 114, 69,  36,
  21,  22,  34,  22,  35,  34,  93,  96,  97,  91,  97,  95,  90,  96,  92,
  91,  94,  90,  88,  84,  85,  157, 156, 88,  155, 161, 157, 157, 160, 156,
  121, 122, 120, 165, 167, 163, 166, 144, 149, 149, 148, 147, 148, 149, 144,
  149, 147, 166, 121, 123, 122, 165, 163, 164, 169, 170, 168, 172, 175, 174,
  169, 171, 170, 172, 174, 173, 180, 179, 178, 177, 180, 176, 183, 184, 182,
  188, 189, 187, 183, 185, 184, 188, 187, 186, 5,   0,   1,   65,  0,   2,
  99,  109, 110, 130, 140, 146, 101, 87,  104, 105, 104, 108, 150, 140, 141,
  152, 127, 125, 66,  65,  10,  12,  6,   1,   137, 119, 45,  160, 158, 154,
  155, 89,  86,  109, 103, 101, 80,  1,   66,  86,  89,  85,  86,  84,  156,
  13,  80,  139, 132, 145, 124, 106, 23,  12,  128, 146, 140, 117, 119, 137,
  135, 70,  117, 151, 125, 124, 45,  70,  135, 114, 113, 108, 157, 88,  89,
  119, 116, 40,  129, 128, 150, 140, 130, 142, 118, 117, 70,  142, 130, 133,
  131, 141, 142, 139, 66,  11,  113, 37,  23,  130, 143, 129, 143, 146, 128,
  118, 46,  41,  1,   0,   65,  107, 102, 103, 113, 162, 81,  106, 80,  13,
  114, 107, 109, 138, 124, 125, 153, 114, 36,  113, 98,  110, 162, 113, 114,
  152, 132, 126, 127, 126, 137, 158, 159, 155, 119, 118, 115, 104, 87,  102,
  10,  8,   9,   7,   4,   5,   8,   3,   2,   8,   4,   7,   7,   6,   12,
  14,  68,  9,   68,  67,  11,  13,  67,  68,  38,  37,  81,  83,  82,  36,
  39,  38,  100, 29,  14,  12,  13,  14,  29,  61,  70,  45,  63,  62,  45,
  44,  70,  61,  63,  49,  44,  92,  15,  17,  19,  16,  15,  17,  15,  16,
  17,  20,  91,  20,  19,  93,  38,  31,  33,  39,  22,  32,  22,  21,  33,
  39,  36,  69,  38,  34,  23,  27,  30,  25,  32,  28,  24,  27,  18,  24,
  30,  32,  26,  31,  27,  28,  39,  30,  31,  40,  43,  49,  48,  42,  41,
  49,  43,  42,  70,  44,  48,  51,  53,  79,  58,  52,  50,  64,  61,  51,
  50,  51,  78,  52,  58,  79,  63,  52,  53,  73,  56,  71,  56,  74,  75,
  77,  57,  72,  59,  56,  73,  55,  74,  56,  71,  75,  54,  54,  75,  74,
  77,  54,  55,  73,  72,  57,  114, 112, 69,  93,  92,  96,  91,  93,  97,
  90,  94,  96,  91,  95,  94,  88,  156, 84,  155, 159, 161, 157, 161, 160,
  180, 181, 179, 177, 181, 180,
};

uint32_t colours[] = {
  0xEDAA80, 0xEDAA80, 0xffffff, 0x212121, 0xffffff, 0x484848, 0xffffff,
  0x212121, 0x484848, 0xEDAA80, 0xEDAA80, 0x484848, 0xEDAA80, 0xffffff,
  0xEDAA80, 0xEDAA80, 0xffffff, 0xEDAA80, 0xffffff, 0x484848, 0xffffff,
  0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0x484848, 0xEDAA80, 0x484848,
  0x212121, 0x484848, 0x484848, 0x484848, 0x484848, 0xffffff, 0xffffff,
  0x484848, 0xEDAA80, 0xffffff, 0xEDAA80, 0x484848, 0x212121, 0x212121,
  0xffffff, 0x484848, 0x212121, 0x212121, 0xffffff, 0xEDAA80, 0xffffff,
  0x212121, 0xffffff, 0x212121, 0x484848, 0xEDAA80, 0xffffff, 0xffffff,
  0xEDAA80, 0xffffff, 0xffffff, 0x484848, 0xffffff, 0xffffff, 0xffffff,
  0x484848, 0x484848, 0xEDAA80, 0x484848, 0x484848, 0xffffff, 0xEDAA80,
  0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80,
  0xEDAA80, 0xEDAA80, 0xffffff, 0xffffff, 0xffffff, 0xEDAA80, 0xEDAA80,
  0x484848, 0x484848, 0x484848, 0x484848, 0xEDAA80, 0xEDAA80, 0xEDAA80,
  0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xffffff, 0xffffff, 0xffffff,
  0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff,
  0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff,
  0xffffff, 0x484848, 0x484848, 0x484848, 0x484848, 0x484848, 0x484848,
  0x484848, 0x484848, 0x484848, 0x484848, 0x484848, 0x484848, 0x484848,
  0x484848, 0x484848, 0x484848, 0x212121, 0x212121, 0x212121, 0x212121,
  0x212121, 0x212121, 0x212121, 0x212121, 0x212121, 0x484848, 0xffffff,
  0xffffff, 0xffffff, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80,
  0xEDAA80, 0xEDAA80, 0xEDAA80, 0xFEF7F1, 0xFEF7F1, 0xE3924B, 0xE3924B,
  0xE3924B, 0xE3924B, 0xFEF7F1, 0xFEF7F1, 0x502F12, 0x502F12, 0x502F12,
  0x502F12, 0xF7616D, 0xF7616D, 0x1A0F0A, 0x1A0F0A, 0x1A0F0A, 0x1A0F0A,
  0xEDAA80, 0xEDAA80, 0xffffff, 0x212121, 0xffffff, 0xffffff, 0x212121,
  0x484848, 0xEDAA80, 0xEDAA80, 0x484848, 0xEDAA80, 0xEDAA80, 0xffffff,
  0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0x484848, 0xEDAA80, 0x212121,
  0x484848, 0x484848, 0x484848, 0x484848, 0xffffff, 0xEDAA80, 0x484848,
  0x212121, 0x212121, 0x484848, 0x212121, 0x212121, 0xEDAA80, 0xffffff,
  0x212121, 0x212121, 0x484848, 0xEDAA80, 0xffffff, 0xffffff, 0xEDAA80,
  0xffffff, 0x484848, 0xffffff, 0xffffff, 0xffffff, 0x484848, 0x484848,
  0xEDAA80, 0x484848, 0xffffff, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80,
  0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xffffff, 0xffffff, 0xffffff,
  0xEDAA80, 0xEDAA80, 0x484848, 0x484848, 0x484848, 0x484848, 0xEDAA80,
  0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xffffff, 0xffffff, 0xffffff,
  0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff,
  0xffffff, 0x484848, 0x484848, 0x484848, 0x484848, 0x484848, 0x484848,
  0x484848, 0x484848, 0x484848, 0x484848, 0x212121, 0x212121, 0x212121,
  0x212121, 0x212121, 0x212121, 0x212121, 0x212121, 0x212121, 0xffffff,
  0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80, 0xEDAA80,
  0xF7616D, 0xF7616D,
};
/* clang-format on */

vec_t
vec_add(vec_t a, vec_t b)
{
  return (vec_t){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
vec_t
vec_sub(vec_t a, vec_t b)
{
  return (vec_t){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}
vec_t
vec_div(vec_t a, float b)
{
  return (vec_t){a.x / b, a.y / b, a.z / b, a.w};
}
vec_t
vec_mul(vec_t a, float b)
{
  return (vec_t){a.x * b, a.y * b, a.z * b, a.w * b};
}
float
vec_dot(vec_t a, vec_t b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
vec_t
vec_cross(vec_t a, vec_t b)
{
  return (vec_t){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, 1};
}
float
vec_length(vec_t v)
{
  return sqrtf(vec_dot(v, v));
}
vec_t
vec_norm(vec_t v)
{
  float l = vec_length(v);
  return (vec_t){v.x / l, v.y / l, v.z / l, 1};
}

mat_t
mat_ident()
{
  return (mat_t){{[0][0] = 1, [1][1] = 1, [2][2] = 1, [3][3] = 1}};
}
mat_t
mat_rot_x(float a)
{
  return (mat_t){{[0][0] = 1, [1][1] = cosf(a), [1][2] = sinf(a), [2][1] = -sinf(a), [2][2] = cosf(a), [3][3] = 1}};
}
mat_t
mat_rot_y(float a)
{
  return (mat_t){{[0][0] = cosf(a), [0][2] = sinf(a), [2][0] = -sinf(a), [1][1] = 1, [2][2] = cosf(a), [3][3] = 1}};
}
mat_t
mat_rot_z(float a)
{
  return (mat_t){{[0][0] = cosf(a), [0][1] = sinf(a), [1][0] = -sinf(a), [1][1] = cosf(a), [2][2] = 1, [3][3] = 1}};
}
mat_t
mat_trans(float x, float y, float z)
{
  return (mat_t){{[0][0] = 1, [1][1] = 1, [2][2] = 1, [3][3] = 1, [3][0] = x, [3][1] = y, [3][2] = z}};
}
mat_t
mat_scale(float x, float y, float z)
{
  return (mat_t){{[0][0] = x, [1][1] = y, [2][2] = z, [3][3] = 1}};
}

mat_t
mat_proj(float fov, float aspect)
{
  fov = 1.0f / tanf(fov * 0.5f / 180.0f * 3.1415926536f);
  float near = 0.1, far = 10.0;
  float depth = 1.0 / (far - near);
  return (mat_t){{
    [0][0] = aspect * fov,
    [1][1] = fov,
    [2][2] = (far + near) * depth,
    [3][2] = -2.0 * far * near * depth,
    [2][3] = 1.0,
  }};
}

mat_t
mat_point_at(vec_t from, vec_t to, vec_t up)
{
  vec_t forward = vec_sub(to, from);
  forward = vec_norm(forward);
  vec_t a = vec_mul(forward, vec_dot(up, forward));
  up = vec_norm(vec_sub(up, a));
  vec_t right = vec_cross(up, forward);
  return (mat_t){{
    [0][0] = right.x,
    [0][1] = right.y,
    [0][2] = right.z,
    [0][3] = 0,
    [1][0] = up.x,
    [1][1] = up.y,
    [1][2] = up.z,
    [1][3] = 0,
    [2][0] = forward.x,
    [2][1] = forward.y,
    [2][2] = forward.z,
    [2][3] = 0,
    [3][0] = from.x,
    [3][1] = from.y,
    [3][2] = from.z,
    [3][3] = 1,
  }};
}

mat_t
mat_qinv(mat_t m)
{
  mat_t o = (mat_t){{
    [0][0] = m.m[0][0],
    [0][1] = m.m[1][0],
    [0][2] = m.m[2][0],
    [0][3] = 0,
    [1][0] = m.m[0][1],
    [1][1] = m.m[1][1],
    [1][2] = m.m[2][1],
    [1][3] = 0,
    [2][0] = m.m[0][2],
    [2][1] = m.m[1][2],
    [2][2] = m.m[2][2],
    [2][3] = 0,
  }};
  o.m[3][0] = -(m.m[3][0] * o.m[0][0] + m.m[3][1] * o.m[1][0] + m.m[3][2] * o.m[2][0]);
  o.m[3][1] = -(m.m[3][0] * o.m[0][1] + m.m[3][1] * o.m[1][1] + m.m[3][2] * o.m[2][1]);
  o.m[3][2] = -(m.m[3][0] * o.m[0][2] + m.m[3][1] * o.m[1][2] + m.m[3][2] * o.m[2][2]);
  o.m[3][3] = 1;
  return o;
}

mat_t
mat_mul(mat_t a, mat_t b)
{
  return (mat_t){{
    [0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0] + a.m[0][3] * b.m[3][0],
    [0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1] + a.m[0][3] * b.m[3][1],
    [0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2] + a.m[0][3] * b.m[3][2],
    [0][3] = a.m[0][0] * b.m[0][3] + a.m[0][1] * b.m[1][3] + a.m[0][2] * b.m[2][3] + a.m[0][3] * b.m[3][3],
    [1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0] + a.m[1][3] * b.m[3][0],
    [1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1] + a.m[1][3] * b.m[3][1],
    [1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2] + a.m[1][3] * b.m[3][2],
    [1][3] = a.m[1][0] * b.m[0][3] + a.m[1][1] * b.m[1][3] + a.m[1][2] * b.m[2][3] + a.m[1][3] * b.m[3][3],
    [2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0] + a.m[2][3] * b.m[3][0],
    [2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1] + a.m[2][3] * b.m[3][1],
    [2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2] + a.m[2][3] * b.m[3][2],
    [2][3] = a.m[2][0] * b.m[0][3] + a.m[2][1] * b.m[1][3] + a.m[2][2] * b.m[2][3] + a.m[2][3] * b.m[3][3],
    [3][0] = a.m[3][0] * b.m[0][0] + a.m[3][1] * b.m[1][0] + a.m[3][2] * b.m[2][0] + a.m[3][3] * b.m[3][0],
    [3][1] = a.m[3][0] * b.m[0][1] + a.m[3][1] * b.m[1][1] + a.m[3][2] * b.m[2][1] + a.m[3][3] * b.m[3][1],
    [3][2] = a.m[3][0] * b.m[0][2] + a.m[3][1] * b.m[1][2] + a.m[3][2] * b.m[2][2] + a.m[3][3] * b.m[3][2],
    [3][3] = a.m[3][0] * b.m[0][3] + a.m[3][1] * b.m[1][3] + a.m[3][2] * b.m[2][3] + a.m[3][3] * b.m[3][3],
  }};
}

vec_t
mat_mul_vec(mat_t m, vec_t v)
{
  return (vec_t){
    v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + v.w * m.m[3][0],
    v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + v.w * m.m[3][1],
    v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + v.w * m.m[3][2],
    v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + v.w * m.m[3][3],
  };
}

void
reset()
{
  render.model = mat_ident();
}
void
rotate_x(float angle)
{
  render.model = mat_mul(render.model, mat_rot_x(angle));
}
void
rotate_y(float angle)
{
  render.model = mat_mul(render.model, mat_rot_y(angle));
}
void
rotate_z(float angle)
{
  render.model = mat_mul(render.model, mat_rot_z(angle));
}
void
translate(float x, float y, float z)
{
  render.model = mat_mul(render.model, mat_trans(x, y, z));
}
void
scale(float x, float y, float z)
{
  render.model = mat_mul(render.model, mat_scale(x, y, z));
}
void
set_fov(float fov_in_degrees)
{
  render.proj = mat_proj(fov_in_degrees, render.height / (float)render.width);
}

void
set_camera(float x, float y, float z, float yaw, float pitch, float roll)
{
  vec_t up = {0, 1, 0, 1};
  vec_t forward = {0, 0, 1, 1};
  up = mat_mul_vec(mat_rot_z(roll), up);
  forward = mat_mul_vec(mat_rot_x(pitch), forward);
  forward = mat_mul_vec(mat_rot_y(yaw), forward);
  forward = vec_add(render.camera, forward);
  render.view = mat_qinv(mat_point_at(render.camera, forward, up));
}

void
look_at(float x, float y, float z, float tx, float ty, float tz)
{
  vec_t from = {x, y, z, 1};
  vec_t to = {tx, ty, tz, 1};
  vec_t up = {0, 1, 0, 1};
  render.view = mat_qinv(mat_point_at(from, to, up));
}

int
to_screen(float x, float y, float z, int* sx, int* sy, float* sz)
{
  vec_t p = {x, y, z, 1};
  p = mat_mul_vec(render.model, p);
  p = mat_mul_vec(render.view, p);
  p = mat_mul_vec(render.proj, p);
  if (p.w <= 0)
    return 0; /* behind camera */
  p.x = (1.0f + (p.x / p.w)) * (render.width / 2.0f);
  p.y = (1.0f - (p.y / p.w)) * (render.height / 2.0f);
  /* could still be off screen, may need clipping */
  *sx = p.x;
  *sy = p.y;
  if (sz)
    *sz = p.z / p.w;
  return *sx >= 0 && *sx < render.width && *sy >= 0 && *sy < render.height;
}

vec_t
intersect_plane(vec_t pos, vec_t norm, vec_t start, vec_t end)
{
  float delta = -vec_dot(norm, pos);
  float ad = vec_dot(start, norm);
  float bd = vec_dot(end, norm);
  float t = (-delta - ad) / (bd - ad);
  vec_t start_to_end = vec_sub(end, start);
  vec_t segment = vec_mul(start_to_end, t);
  return vec_add(start, segment);
}

int
clip_against_plane(vec_t pos, vec_t norm, vec_t* triangle, vec_t* clipped)
{
  int i[3];
  int ni = 0;
  int o[3];
  int no = 0;
  float delta = vec_dot(norm, pos);
  float d0 = norm.x * triangle[0].x + norm.y * triangle[0].y + norm.z * triangle[0].z - delta;
  float d1 = norm.x * triangle[1].x + norm.y * triangle[1].y + norm.z * triangle[1].z - delta;
  float d2 = norm.x * triangle[2].x + norm.y * triangle[2].y + norm.z * triangle[2].z - delta;
  if (d0 >= 0)
    i[ni++] = 0;
  else
    o[no++] = 0;
  if (d1 >= 0)
    i[ni++] = 1;
  else
    o[no++] = 1;
  if (d2 >= 0)
    i[ni++] = 2;
  else
    o[no++] = 2;
  if (ni == 3)
  {
    clipped[0] = triangle[0];
    clipped[1] = triangle[1];
    clipped[2] = triangle[2];
    return 1;
  }
  else if (ni == 1)
  {
    clipped[0] = triangle[i[0]];
    clipped[1] = intersect_plane(pos, norm, triangle[i[0]], triangle[o[0]]);
    clipped[2] = intersect_plane(pos, norm, triangle[i[0]], triangle[o[1]]);
    return 1;
  }
  else if (ni == 2)
  {
    clipped[0] = triangle[i[0]];
    clipped[1] = triangle[i[1]];
    clipped[2] = intersect_plane(pos, norm, triangle[i[0]], triangle[o[0]]);
    clipped[3] = triangle[i[1]];
    clipped[4] = clipped[2];
    clipped[5] = intersect_plane(pos, norm, triangle[i[1]], triangle[o[0]]);
    return 2;
  }
  return 0;
}

void
rasterise_triangle(float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz, uint32_t c)
{
  int y = 0;
  float t = 0;
  if (ay > by)
  {
    t = ax;
    ax = bx;
    bx = t;
    t = ay;
    ay = by;
    by = t;
    t = az;
    az = bz;
    bz = t;
  }
  if (ay > cy)
  {
    t = ax;
    ax = cx;
    cx = t;
    t = ay;
    ay = cy;
    cy = t;
    t = az;
    az = cz;
    cz = t;
  }
  if (by > cy)
  {
    t = bx;
    bx = cx;
    cx = t;
    t = by;
    by = cy;
    cy = t;
    t = bz;
    bz = cz;
    cz = t;
  }
  float long_x = ax, long_step_x = (cx - ax) / (cy - ay);
  float long_z = az, long_step_z = (cz - az) / (cy - ay);
  float short_x, short_step_x, short_z, short_step_z;
#define INNER                                                        \
  {                                                                  \
    float left_x = long_x, right_x = short_x;                        \
    float left_z = long_z, right_z = short_z;                        \
    if (left_x > right_x)                                            \
    {                                                                \
      left_x = short_x;                                              \
      right_x = long_x;                                              \
      left_z = short_z;                                              \
      right_z = long_z;                                              \
    }                                                                \
    float step = (right_z - left_z) / (right_x - left_x);            \
    int stride_offset = y * render.width;                            \
    uint32_t* pixel = render.pixels + ((int)left_x + stride_offset); \
    uint32_t* end = render.pixels + ((int)right_x + stride_offset);  \
    float* depth = render.depth + ((int)left_x + stride_offset);     \
    float d = left_z;                                                \
    while (pixel < end)                                              \
    {                                                                \
      if (d < *depth)                                                \
      {                                                              \
        *depth = d;                                                  \
        *pixel = c;                                                  \
      }                                                              \
      d += step;                                                     \
      ++pixel;                                                       \
      ++depth;                                                       \
    }                                                                \
    long_x += long_step_x;                                           \
    long_z += long_step_z;                                           \
    short_x += short_step_x;                                         \
    short_z += short_step_z;                                         \
  }
  short_x = ax, short_step_x = (bx - ax) / (by - ay);
  short_z = az, short_step_z = (bz - az) / (by - ay);
  for (y = (int)ay; y < (int)by; ++y)
    INNER
  short_x = bx, short_step_x = (cx - bx) / (cy - by);
  short_z = bz, short_step_z = (cz - bz) / (cy - by);
  for (y = (int)by; y < (int)cy; ++y)
    INNER
#undef INNER
}

void
draw_model(int vert_count, int face_count, float* vertices, int* indices, uint32_t* colours)
{
  int vi;
  int fi;
  int i;
  static vec_t v[512];
  mat_t mv = mat_mul(render.model, render.view);
  for (vi = 0; vi < vert_count; ++vi)
  {
    float va = vertices[vi * 3 + 0], vb = vertices[vi * 3 + 1], vc = vertices[vi * 3 + 2];
    v[vi] = mat_mul_vec(mv, (vec_t){va, vb, vc, 1});
  }
  for (fi = 0; fi < face_count; ++fi)
  {
    int ia = indices[fi * 3 + 0], ib = indices[fi * 3 + 1], ic = indices[fi * 3 + 2];
    vec_t queue[16 * 3] = {
      {v[ia].x, v[ia].y, v[ia].z, 1},
      {v[ib].x, v[ib].y, v[ib].z, 1},
      {v[ic].x, v[ic].y, v[ic].z, 1}};
    vec_t normal = vec_cross(vec_sub(queue[1], queue[0]), vec_sub(queue[2], queue[0]));
    bool facing_away = vec_dot(normal, queue[0]) >= 0;
    if (facing_away)
      continue;
    int queue_count = clip_against_plane((vec_t){0, 0, 0.01, 1}, (vec_t){0, 0, 1, 1}, queue, queue);
    if (queue_count == 0)
      continue;
    vec_t* t = queue;
    for (i = 0; i < queue_count; ++i)
    {
      t[0] = mat_mul_vec(render.proj, t[0]);
      t[1] = mat_mul_vec(render.proj, t[1]);
      t[2] = mat_mul_vec(render.proj, t[2]);
      t[0] = vec_div(t[0], t[0].w);
      t[1] = vec_div(t[1], t[1].w);
      t[2] = vec_div(t[2], t[2].w);
      float w = render.width / 2, h = render.height / 2;
      t[0].x = (1.0f + t[0].x) * w, t[0].y = (1.0f - t[0].y) * h;
      t[1].x = (1.0f + t[1].x) * w, t[1].y = (1.0f - t[1].y) * h;
      t[2].x = (1.0f + t[2].x) * w, t[2].y = (1.0f - t[2].y) * h;
      t += 3;
    }
    float w = render.width + 0.5f, h = render.height + 0.5f;
    vec_t planes[8] = {
      /* position, normal */
      {0, .5, 0, 1},
      {0, 1, 0, 1}, /* top */
      {0, h, 0, 1},
      {0, -1, 0, 1}, /* bottom */
      {.5, 0, 0, 1},
      {1, 0, 0, 1}, /* left */
      {w, 0, 0, 1},
      {-1, 0, 0, 1}, /* right */
    };
    int triangles_to_clip = queue_count;
    for (i = 0; i < 8; i += 2)
    {
      while (triangles_to_clip > 0)
      {
        --queue_count;
        --triangles_to_clip;
        vec_t next[3] = {queue[0], queue[1], queue[2]};
        memmove(queue, queue + 3, sizeof(vec_t) * 3 * queue_count);
        queue_count += clip_against_plane(planes[i], planes[i + 1], next, queue + queue_count * 3);
      }
      triangles_to_clip = queue_count;
    }
    t = queue;
    for (i = 0; i < queue_count; ++i)
    {
      rasterise_triangle(t[0].x, t[0].y, t[0].z, t[1].x, t[1].y, t[1].z, t[2].x, t[2].y, t[2].z, colours[fi]);
      t += 3;
    }
  }
}

int
main(int argument_count, char** arguments)
{
  render.width = 400;
  render.height = 600;
  SDL_Window* window = SDL_CreateWindow("ben's renderer", 10, 10, render.width, render.height, 0);
  int buffer_size = render.width * render.height * 4;
  render.pixels = SDL_GetWindowSurface(window)->pixels;
  render.depth = malloc(buffer_size);
  render.model = mat_ident();
  render.proj = mat_proj(30.0f, render.height / (float)render.width);

  while (1)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
        return 0;
    }
    memset(render.pixels, 0, buffer_size);
    memset(render.depth, 0x7f, buffer_size);
    float t = SDL_GetTicks() * 0.001f;
    look_at(3, 2 + sinf(t), 3, 0, 1, 0);
    render.model = mat_rot_y(t);
    draw_model(vert_count, face_count, vertices, indices, colours);

    SDL_UpdateWindowSurface(window);
    SDL_Delay(10);
  }

  return 0;
}
