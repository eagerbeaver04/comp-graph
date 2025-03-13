#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <windows.h>
#include <DirectXMath.h>
#include "DDSTextureLoader.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

struct SimpleVertex
{
    XMFLOAT3 pos;
    XMFLOAT2 tex;
};

struct SkyboxVertex
{
    XMFLOAT3 pos;
};

struct Vertices
{
	static SimpleVertex g_CubeVertices[];
    static SkyboxVertex g_SkyboxVertices[];
};

struct CubeData {
    XMFLOAT4 color;
    XMMATRIX modelMatrix;
    bool isTransparent;

    XMFLOAT3 GetPosition() const {
        XMFLOAT3 pos;
        XMStoreFloat3(&pos, modelMatrix.r[3]);
        return pos;
    }
};
SimpleVertex Vertices::g_CubeVertices[] = {
    { { -0.5f, -0.5f, 0.5f }, { 0.0f, 1.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },

    { { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } },

    { {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
    { { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f } },

    { {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f } },

    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
    { { -0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },

    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
    { { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f } },

    { {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f } },

    { {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } },

    { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
    { {  0.5f,  0.5f,  0.5f }, { 1.0f, 1.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f } },

    { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f } },
    { {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f } },
    { { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f } },

    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
    { {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },

    { { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
    { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
    { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
};

SkyboxVertex Vertices::g_SkyboxVertices[] =
{
    { { -1.0f, -1.0f, -1.0f } },
    { { -1.0f,  1.0f, -1.0f } },
    { {  1.0f,  1.0f, -1.0f } },
    { { -1.0f, -1.0f, -1.0f } },
    { {  1.0f,  1.0f, -1.0f } },
    { {  1.0f, -1.0f, -1.0f } },

    { {  1.0f, -1.0f,  1.0f } },
    { {  1.0f,  1.0f,  1.0f } },
    { { -1.0f,  1.0f,  1.0f } },
    { {  1.0f, -1.0f,  1.0f } },
    { { -1.0f,  1.0f,  1.0f } },
    { { -1.0f, -1.0f,  1.0f } },

    { { -1.0f, -1.0f,  1.0f } },
    { { -1.0f,  1.0f,  1.0f } },
    { { -1.0f,  1.0f, -1.0f } },
    { { -1.0f, -1.0f,  1.0f } },
    { { -1.0f,  1.0f, -1.0f } },
    { { -1.0f, -1.0f, -1.0f } },

    { {  1.0f, -1.0f, -1.0f } },
    { {  1.0f,  1.0f, -1.0f } },
    { {  1.0f,  1.0f,  1.0f } },
    { {  1.0f, -1.0f, -1.0f } },
    { {  1.0f,  1.0f,  1.0f } },
    { {  1.0f, -1.0f,  1.0f } },

    { { -1.0f,  1.0f, -1.0f } },
    { { -1.0f,  1.0f,  1.0f } },
    { {  1.0f,  1.0f,  1.0f } },
    { { -1.0f,  1.0f, -1.0f } },
    { {  1.0f,  1.0f,  1.0f } },
    { {  1.0f,  1.0f, -1.0f } },

    { { -1.0f, -1.0f,  1.0f } },
    { { -1.0f, -1.0f, -1.0f } },
    { {  1.0f, -1.0f, -1.0f } },
    { { -1.0f, -1.0f,  1.0f } },
    { {  1.0f, -1.0f, -1.0f } },
    { {  1.0f, -1.0f,  1.0f } },
};
