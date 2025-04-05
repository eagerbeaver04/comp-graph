#include "DDSTextureLoader.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include <DirectXMath.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <windows.h>

using namespace DirectX;

const int MAX_CUBES_GPU = 20;

struct CullingData {
  UINT numShapes;
  UINT padding[3];
  XMFLOAT4 bbMin[MAX_CUBES_GPU];
  XMFLOAT4 bbMax[MAX_CUBES_GPU];
};

struct Plane {
  float a, b, c, d;
};

struct SimpleVertex {
  float x, y, z;
  float nx, ny, nz;
  float u, v;
};
struct SceneCBData {
  XMFLOAT4X4 viewProjectionMatrix;
  XMFLOAT4 planes[6];
};

SimpleVertex g_CubeVertices[] = {

    {-0.5f, -0.5f, 0.5f, 0, 0, 1, 0.0f, 1.0f},
    {0.5f, -0.5f, 0.5f, 0, 0, 1, 1.0f, 1.0f},
    {0.5f, 0.5f, 0.5f, 0, 0, 1, 1.0f, 0.0f},
    {-0.5f, -0.5f, 0.5f, 0, 0, 1, 0.0f, 1.0f},
    {0.5f, 0.5f, 0.5f, 0, 0, 1, 1.0f, 0.0f},
    {-0.5f, 0.5f, 0.5f, 0, 0, 1, 0.0f, 0.0f},

    {0.5f, -0.5f, -0.5f, 0, 0, -1, 0.0f, 1.0f},
    {-0.5f, -0.5f, -0.5f, 0, 0, -1, 1.0f, 1.0f},
    {-0.5f, 0.5f, -0.5f, 0, 0, -1, 1.0f, 0.0f},
    {0.5f, -0.5f, -0.5f, 0, 0, -1, 0.0f, 1.0f},
    {-0.5f, 0.5f, -0.5f, 0, 0, -1, 1.0f, 0.0f},
    {0.5f, 0.5f, -0.5f, 0, 0, -1, 0.0f, 0.0f},

    {-0.5f, -0.5f, -0.5f, -1, 0, 0, 0.0f, 1.0f},
    {-0.5f, -0.5f, 0.5f, -1, 0, 0, 1.0f, 1.0f},
    {-0.5f, 0.5f, 0.5f, -1, 0, 0, 1.0f, 0.0f},
    {-0.5f, -0.5f, -0.5f, -1, 0, 0, 0.0f, 1.0f},
    {-0.5f, 0.5f, 0.5f, -1, 0, 0, 1.0f, 0.0f},
    {-0.5f, 0.5f, -0.5f, -1, 0, 0, 0.0f, 0.0f},

    {0.5f, -0.5f, 0.5f, 1, 0, 0, 0.0f, 1.0f},
    {0.5f, -0.5f, -0.5f, 1, 0, 0, 1.0f, 1.0f},
    {0.5f, 0.5f, -0.5f, 1, 0, 0, 1.0f, 0.0f},
    {0.5f, -0.5f, 0.5f, 1, 0, 0, 0.0f, 1.0f},
    {0.5f, 0.5f, -0.5f, 1, 0, 0, 1.0f, 0.0f},
    {0.5f, 0.5f, 0.5f, 1, 0, 0, 0.0f, 0.0f},

    {-0.5f, 0.5f, 0.5f, 0, 1, 0, 0.0f, 1.0f},
    {0.5f, 0.5f, 0.5f, 0, 1, 0, 1.0f, 1.0f},
    {0.5f, 0.5f, -0.5f, 0, 1, 0, 1.0f, 0.0f},
    {-0.5f, 0.5f, 0.5f, 0, 1, 0, 0.0f, 1.0f},
    {0.5f, 0.5f, -0.5f, 0, 1, 0, 1.0f, 0.0f},
    {-0.5f, 0.5f, -0.5f, 0, 1, 0, 0.0f, 0.0f},

    {-0.5f, -0.5f, -0.5f, 0, -1, 0, 0.0f, 1.0f},
    {0.5f, -0.5f, -0.5f, 0, -1, 0, 1.0f, 1.0f},
    {0.5f, -0.5f, 0.5f, 0, -1, 0, 1.0f, 0.0f},
    {-0.5f, -0.5f, -0.5f, 0, -1, 0, 0.0f, 1.0f},
    {0.5f, -0.5f, 0.5f, 0, -1, 0, 1.0f, 0.0f},
    {-0.5f, -0.5f, 0.5f, 0, -1, 0, 0.0f, 0.0f},
};

struct FullScreenVertex {
  float x, y, z, w;
  float u, v;
};

FullScreenVertex g_FullScreenTriangle[3] = {{-1.0f, -1.0f, 0, 1, 0.0f, 1.0f},
                                            {-1.0f, 3.0f, 0, 1, 0.0f, -1.0f},
                                            {3.0f, -1.0f, 0, 1, 2.0f, 1.0f}};

struct SkyboxVertex {
  float x, y, z;
};

SkyboxVertex g_SkyboxVertices[] = {
    {-1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},  {1.0f, 1.0f, -1.0f},
    {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, -1.0f},   {1.0f, -1.0f, -1.0f},
    {1.0f, -1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},    {-1.0f, 1.0f, 1.0f},
    {1.0f, -1.0f, 1.0f},   {-1.0f, 1.0f, 1.0f},   {-1.0f, -1.0f, 1.0f},
    {-1.0f, -1.0f, 1.0f},  {-1.0f, 1.0f, 1.0f},   {-1.0f, 1.0f, -1.0f},
    {-1.0f, -1.0f, 1.0f},  {-1.0f, 1.0f, -1.0f},  {-1.0f, -1.0f, -1.0f},
    {1.0f, -1.0f, -1.0f},  {1.0f, 1.0f, -1.0f},   {1.0f, 1.0f, 1.0f},
    {1.0f, -1.0f, -1.0f},  {1.0f, 1.0f, 1.0f},    {1.0f, -1.0f, 1.0f},
    {-1.0f, 1.0f, -1.0f},  {-1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},
    {-1.0f, 1.0f, -1.0f},  {1.0f, 1.0f, 1.0f},    {1.0f, 1.0f, -1.0f},
    {-1.0f, -1.0f, 1.0f},  {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, -1.0f},  {1.0f, -1.0f, 1.0f},
};

struct LightBufferType {
  XMFLOAT4 light0Pos;
  XMFLOAT4 light0Color;
  XMFLOAT4 light1Pos;
  XMFLOAT4 light1Color;
  XMFLOAT4 ambient;
};