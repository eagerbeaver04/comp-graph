#include "data_structures.h"
#include <string>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

const std::wstring windowTitle = L"Mikhail Markov";
const std::wstring windowClass = L"MikhailMarkovClass";
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

ID3D11Buffer *g_pCullingParamsBuffer = nullptr;
ID3D11Device *g_pd3dDevice = nullptr;
ID3D11DeviceContext *g_pImmediateContext = nullptr;
IDXGISwapChain *g_pSwapChain = nullptr;
ID3D11RenderTargetView *g_pRenderTargetView = nullptr;
ID3D11DepthStencilView *g_pDepthStencilView = nullptr;
ID3D11VertexShader *g_pTransparentVS = nullptr;
ID3D11VertexShader *g_pCubeVS = nullptr;
ID3D11PixelShader *g_pCubePS = nullptr;
ID3D11InputLayout *g_pCubeInputLayout = nullptr;
ID3D11Buffer *g_pCubeVertexBuffer = nullptr;
ID3D11Buffer *g_pCubeModelBuffer = nullptr;
ID3D11Buffer *g_pCubeVPBuffer = nullptr;
ID3D11ShaderResourceView *g_pCubeTextureRV = nullptr;
ID3D11ComputeShader *g_pCullingCS = nullptr;
ID3D11Buffer *g_pSceneCBBuffer = nullptr;
ID3D11Buffer *g_pIndirectArgsBuffer = nullptr;
ID3D11Buffer *g_pObjectsIdsBuffer = nullptr;
ID3D11UnorderedAccessView *g_pIndirectArgsUAV = nullptr;
ID3D11UnorderedAccessView *g_pObjectsIdsUAV = nullptr;

ID3D11ShaderResourceView *g_pCubeNormalMapRV = nullptr;
ID3D11SamplerState *g_pSamplerLiinner = nullptr;
ID3D11Buffer *g_pIndirectArgsStagingBuffer = nullptr;
ID3D11VertexShader *g_pSkyboxVS = nullptr;
ID3D11PixelShader *g_pSkyboxPS = nullptr;
ID3D11InputLayout *g_pSkyboxInputLayout = nullptr;
ID3D11Buffer *g_pSkyboxVertexBuffer = nullptr;
ID3D11Buffer *g_pSkyboxVPBuffer = nullptr;
ID3D11ShaderResourceView *g_pSkyboxTextureRV = nullptr;

ID3D11PixelShader *g_pTransparentPS = nullptr;
ID3D11Buffer *g_pTransparentBuffer = nullptr;

ID3D11Buffer *g_pLightBuffer = nullptr;

ID3D11BlendState *g_pAlphaBlendState = nullptr;
ID3D11DepthStencilState *g_pDSStateTrans = nullptr;

bool g_EnablePostProcessFilter = false;

ID3D11Texture2D *g_pPostProcessTex = nullptr;
ID3D11RenderTargetView *g_pPostProcessRTV = nullptr;
ID3D11ShaderResourceView *g_pPostProcessSRV = nullptr;
ID3D11VertexShader *g_pPostProcessVS = nullptr;
ID3D11PixelShader *g_pPostProcessPS = nullptr;
ID3D11Buffer *g_pFullScreenVB = nullptr;
ID3D11InputLayout *g_pFullScreenLayout = nullptr;

bool g_EnableCPUCulling = false;
bool g_EnableGPUCulling = false;
int g_visibleCubesGPU = 0;
float g_CubeAngle = 0.0f;
float g_CameraAngle = 0.0f;
bool g_MouseDragging = false;
POINT g_LastMousePos = {0, 0};
float g_CameraAzimuth = 0.0f;
float g_CameraElevation = 0.0f;
int g_totalInstances = 0;
int g_visibleCubesСPU = 0;

void ExtractFrustumPlanes(const XMMATRIX &M, Plane planes[6]) {
  planes[0].a = M.r[0].m128_f32[3] + M.r[0].m128_f32[0];
  planes[0].b = M.r[1].m128_f32[3] + M.r[1].m128_f32[0];
  planes[0].c = M.r[2].m128_f32[3] + M.r[2].m128_f32[0];
  planes[0].d = M.r[3].m128_f32[3] + M.r[3].m128_f32[0];

  planes[1].a = M.r[0].m128_f32[3] - M.r[0].m128_f32[0];
  planes[1].b = M.r[1].m128_f32[3] - M.r[1].m128_f32[0];
  planes[1].c = M.r[2].m128_f32[3] - M.r[2].m128_f32[0];
  planes[1].d = M.r[3].m128_f32[3] - M.r[3].m128_f32[0];

  planes[2].a = M.r[0].m128_f32[3] - M.r[0].m128_f32[1];
  planes[2].b = M.r[1].m128_f32[3] - M.r[1].m128_f32[1];
  planes[2].c = M.r[2].m128_f32[3] - M.r[2].m128_f32[1];
  planes[2].d = M.r[3].m128_f32[3] - M.r[3].m128_f32[1];

  planes[3].a = M.r[0].m128_f32[3] + M.r[0].m128_f32[1];
  planes[3].b = M.r[1].m128_f32[3] + M.r[1].m128_f32[1];
  planes[3].c = M.r[2].m128_f32[3] + M.r[2].m128_f32[1];
  planes[3].d = M.r[3].m128_f32[3] + M.r[3].m128_f32[1];

  planes[4].a = M.r[0].m128_f32[2];
  planes[4].b = M.r[1].m128_f32[2];
  planes[4].c = M.r[2].m128_f32[2];
  planes[4].d = M.r[3].m128_f32[2];

  planes[5].a = M.r[0].m128_f32[3] - M.r[0].m128_f32[2];
  planes[5].b = M.r[1].m128_f32[3] - M.r[1].m128_f32[2];
  planes[5].c = M.r[2].m128_f32[3] - M.r[2].m128_f32[2];
  planes[5].d = M.r[3].m128_f32[3] - M.r[3].m128_f32[2];
}

bool IsSphereInFrustum(const Plane planes[6], const XMVECTOR &center,
                       float radius) {
  for (int i = 0; i < 6; i++) {
    float distance =
        XMVectorGetX(XMVector3Dot(
            center, XMVectorSet(planes[i].a, planes[i].b, planes[i].c, 0.0f))) +
        planes[i].d;
    if (distance < -radius)
      return false;
  }
  return true;
}

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3D(HWND hWnd);
HRESULT InitGraphics();
void CleanupD3D();
void Render();

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
  std::srand(static_cast<unsigned>(std::time(nullptr)));
  MyRegisterClass(hInstance);
  if (!InitInstance(hInstance, nCmdShow))
    return FALSE;

  HWND hWnd = FindWindow(windowClass.c_str(), windowTitle.c_str());
  if (!hWnd)
    return FALSE;

  if (FAILED(InitD3D(hWnd)))
    return FALSE;

  if (FAILED(InitGraphics())) {
    CleanupD3D();
    return FALSE;
  }

  MSG msg = {};
  while (msg.message != WM_QUIT) {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      Render();
    }
  }
  CleanupD3D();
  return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
  WNDCLASSEXW wcex = {};
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.hInstance = hInstance;
  wcex.hbrBackground = nullptr;
  wcex.lpszClassName = windowClass.c_str();
  return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
  HWND hWnd = CreateWindowW(windowClass.c_str(), windowTitle.c_str(),
                            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 800, 600,
                            nullptr, nullptr, hInstance, nullptr);
  if (!hWnd)
    return FALSE;
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);
  return TRUE;
}

HRESULT CompileShaderFromFile(WCHAR *szFileName, LPCSTR szEntryPoint,
                              LPCSTR szShaderModel, ID3DBlob **ppBlobOut) {
  HRESULT hr = S_OK;

  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  dwShaderFlags |= D3DCOMPILE_DEBUG;
  dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ID3DBlob *pErrorBlob = nullptr;
  hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint,
                          szShaderModel, dwShaderFlags, 0, ppBlobOut,
                          &pErrorBlob);
  if (FAILED(hr)) {
    if (pErrorBlob) {
      OutputDebugStringA(
          reinterpret_cast<const char *>(pErrorBlob->GetBufferPointer()));
      MessageBoxA(nullptr, (char *)pErrorBlob->GetBufferPointer(),
                  "Shader Compile Error", MB_OK);
      pErrorBlob->Release();
    }
    return hr;
  }
  if (pErrorBlob)
    pErrorBlob->Release();

  return S_OK;
}

HRESULT InitD3D(HWND hWnd) {
  RECT rc;
  GetClientRect(hWnd, &rc);
  UINT width = rc.right - rc.left;
  UINT height = rc.bottom - rc.top;

  DXGI_SWAP_CHAIN_DESC sd = {};
  sd.BufferCount = 2;
  sd.BufferDesc.Width = width;
  sd.BufferDesc.Height = height;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

  UINT createDeviceFlags = 0;
#if defined(_DEBUG)
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[1] = {D3D_FEATURE_LEVEL_11_0};
  HRESULT hr = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
      featureLevelArray, 1, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
      &g_pd3dDevice, &featureLevel, &g_pImmediateContext);
  if (FAILED(hr))
    return hr;

  ID3D11Texture2D *pBackBuffer = nullptr;
  hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                               (LPVOID *)&pBackBuffer);
  if (FAILED(hr))
    return hr;
  hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
                                            &g_pRenderTargetView);
  pBackBuffer->Release();
  if (FAILED(hr))
    return hr;

  D3D11_TEXTURE2D_DESC descDepth = {};
  descDepth.Width = width;
  descDepth.Height = height;
  descDepth.MipLevels = 1;
  descDepth.ArraySize = 1;
  descDepth.Format = DXGI_FORMAT_D32_FLOAT;
  descDepth.SampleDesc.Count = 1;
  descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  ID3D11Texture2D *pDepthStencil = nullptr;
  hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);
  if (FAILED(hr))
    return hr;
  hr = g_pd3dDevice->CreateDepthStencilView(pDepthStencil, nullptr,
                                            &g_pDepthStencilView);
  pDepthStencil->Release();
  if (FAILED(hr))
    return hr;

  g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView,
                                          g_pDepthStencilView);

  D3D11_VIEWPORT vp = {};
  vp.Width = static_cast<FLOAT>(width);
  vp.Height = static_cast<FLOAT>(height);
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  g_pImmediateContext->RSSetViewports(1, &vp);

  width = rc.right - rc.left;
  height = rc.bottom - rc.top;

  D3D11_TEXTURE2D_DESC ppDesc = {};
  ppDesc.Width = width;
  ppDesc.Height = height;
  ppDesc.MipLevels = 1;
  ppDesc.ArraySize = 1;
  ppDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  ppDesc.SampleDesc.Count = 1;
  ppDesc.Usage = D3D11_USAGE_DEFAULT;
  ppDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

  hr = g_pd3dDevice->CreateTexture2D(&ppDesc, nullptr, &g_pPostProcessTex);
  if (FAILED(hr))
    return hr;

  hr = g_pd3dDevice->CreateRenderTargetView(g_pPostProcessTex, nullptr,
                                            &g_pPostProcessRTV);
  if (FAILED(hr))
    return hr;

  hr = g_pd3dDevice->CreateShaderResourceView(g_pPostProcessTex, nullptr,
                                              &g_pPostProcessSRV);
  if (FAILED(hr))
    return hr;

  return S_OK;
}

void InitImGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();

  ImGui::StyleColorsDark();

  ImGui_ImplWin32_Init(FindWindow(windowClass.c_str(), windowTitle.c_str()));

  ImGui_ImplDX11_Init(g_pd3dDevice, g_pImmediateContext);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    return true;

  switch (message) {
  case WM_LBUTTONDOWN:
    g_MouseDragging = true;
    g_LastMousePos.x = LOWORD(lParam);
    g_LastMousePos.y = HIWORD(lParam);
    SetCapture(hWnd);
    break;
  case WM_LBUTTONUP:
    g_MouseDragging = false;
    ReleaseCapture();
    break;
  case WM_MOUSEMOVE:
    if (g_MouseDragging) {
      int x = LOWORD(lParam);
      int y = HIWORD(lParam);
      int dx = x - g_LastMousePos.x;
      int dy = y - g_LastMousePos.y;
      g_CameraAzimuth += dx * 0.005f;
      g_CameraElevation += dy * 0.005f;
      if (g_CameraElevation > XM_PIDIV2 - 0.01f)
        g_CameraElevation = XM_PIDIV2 - 0.01f;
      if (g_CameraElevation < -XM_PIDIV2 + 0.01f)
        g_CameraElevation = -XM_PIDIV2 + 0.01f;
      g_LastMousePos.x = x;
      g_LastMousePos.y = y;
    }
    break;
  case WM_KEYDOWN:
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

HRESULT InitGraphics() {
  HRESULT hr = S_OK;
  ID3DBlob *pBlob = nullptr;
  ID3DBlob *pBlobVS = nullptr;
  ID3DBlob *pBlobPS = nullptr;

  hr = CompileShaderFromFile(const_cast<wchar_t *>(L"lab8.fx"), "VS", "vs_5_0",
                             &pBlob);
  if FAILED (hr)
    return hr;
  hr = g_pd3dDevice->CreateVertexShader(
      pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &g_pCubeVS);
  if (FAILED(hr)) {
    pBlob->Release();
    return hr;
  }
  D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 6,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
  };
  hr = g_pd3dDevice->CreateInputLayout(layoutDesc, 3, pBlob->GetBufferPointer(),
                                       pBlob->GetBufferSize(),
                                       &g_pCubeInputLayout);
  pBlob->Release();
  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(const_cast<wchar_t *>(L"lab8.fx"), "PS", "ps_5_0",
                             &pBlob);
  if (FAILED(hr))
    return hr;
  hr = g_pd3dDevice->CreatePixelShader(
      pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &g_pCubePS);
  pBlob->Release();
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC bd = {};
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(SimpleVertex) * ARRAYSIZE(g_CubeVertices);
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = g_CubeVertices;
  hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pCubeVertexBuffer);
  if (FAILED(hr))
    return hr;

  bd.Usage = D3D11_USAGE_DEFAULT;
  const int numInstances = 21;
  bd.ByteWidth = sizeof(XMMATRIX) * numInstances;
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;
  hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pCubeModelBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC vpBufferDesc = {};
  vpBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  vpBufferDesc.ByteWidth = sizeof(XMMATRIX);
  vpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  vpBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  hr = g_pd3dDevice->CreateBuffer(&vpBufferDesc, nullptr, &g_pCubeVPBuffer);
  if (FAILED(hr))
    return hr;

  hr = CreateDDSTextureFromFile(g_pd3dDevice, L"cube.dds", nullptr,
                                &g_pCubeTextureRV);
  if (FAILED(hr))
    return hr;

  hr = CreateDDSTextureFromFile(g_pd3dDevice, L"cube_normal.dds", nullptr,
                                &g_pCubeNormalMapRV);
  if (FAILED(hr))
    return hr;

  D3D11_SAMPLER_DESC sampDesc = {};
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLiinner);
  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(const_cast<wchar_t *>(L"skybox.fx"), "VS",
                             "vs_5_0", &pBlob);
  if (FAILED(hr))
    return hr;
  hr = g_pd3dDevice->CreateVertexShader(
      pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &g_pSkyboxVS);
  if (FAILED(hr)) {
    pBlob->Release();
    return hr;
  }
  D3D11_INPUT_ELEMENT_DESC skyboxLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
  };
  hr = g_pd3dDevice->CreateInputLayout(
      skyboxLayout, 1, pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
      &g_pSkyboxInputLayout);
  pBlob->Release();
  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(const_cast<wchar_t *>(L"skybox.fx"), "PS",
                             "ps_5_0", &pBlob);
  if (FAILED(hr))
    return hr;
  hr = g_pd3dDevice->CreatePixelShader(
      pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &g_pSkyboxPS);
  pBlob->Release();
  if (FAILED(hr))
    return hr;

  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(SkyboxVertex) * ARRAYSIZE(g_SkyboxVertices);
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  initData.pSysMem = g_SkyboxVertices;
  hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pSkyboxVertexBuffer);
  if (FAILED(hr))
    return hr;

  vpBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
  vpBufferDesc.ByteWidth = sizeof(XMMATRIX);
  vpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  vpBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  hr = g_pd3dDevice->CreateBuffer(&vpBufferDesc, nullptr, &g_pSkyboxVPBuffer);
  if (FAILED(hr))
    return hr;

  hr = CreateDDSTextureFromFile(g_pd3dDevice, L"skybox.dds", nullptr,
                                &g_pSkyboxTextureRV);
  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(const_cast<wchar_t *>(L"transparent.fx"), "PS",
                             "ps_5_0", &pBlob);
  if (FAILED(hr))
    return hr;
  hr = g_pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(),
                                       pBlob->GetBufferSize(), nullptr,
                                       &g_pTransparentPS);
  pBlob->Release();
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC tbDesc = {};
  tbDesc.Usage = D3D11_USAGE_DEFAULT;
  tbDesc.ByteWidth = sizeof(XMFLOAT4);
  tbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  tbDesc.CPUAccessFlags = 0;
  hr = g_pd3dDevice->CreateBuffer(&tbDesc, nullptr, &g_pTransparentBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC lbDesc = {};
  lbDesc.Usage = D3D11_USAGE_DEFAULT;
  lbDesc.ByteWidth = sizeof(LightBufferType);
  lbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  lbDesc.CPUAccessFlags = 0;
  hr = g_pd3dDevice->CreateBuffer(&lbDesc, nullptr, &g_pLightBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_BLEND_DESC blendDesc = {};
  blendDesc.RenderTarget[0].BlendEnable = TRUE;
  blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blendDesc.RenderTarget[0].RenderTargetWriteMask =
      D3D11_COLOR_WRITE_ENABLE_ALL;
  hr = g_pd3dDevice->CreateBlendState(&blendDesc, &g_pAlphaBlendState);
  if (FAILED(hr))
    return hr;

  D3D11_DEPTH_STENCIL_DESC dsDescTrans = {};
  dsDescTrans.DepthEnable = true;
  dsDescTrans.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  dsDescTrans.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
  hr = g_pd3dDevice->CreateDepthStencilState(&dsDescTrans, &g_pDSStateTrans);
  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(const_cast<wchar_t *>(L"transparent.fx"), "VS",
                             "vs_5_0", &pBlob);
  if (FAILED(hr))
    return hr;
  hr = g_pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(),
                                        pBlob->GetBufferSize(), nullptr,
                                        &g_pTransparentVS);
  pBlob->Release();
  if (FAILED(hr))
    return hr;

  ID3DBlob *pBlobVSPost = nullptr;
  ID3DBlob *pBlobPSPost = nullptr;

  hr = CompileShaderFromFile(const_cast<wchar_t *>(L"post_processing.fx"), "VS",
                             "vs_5_0", &pBlobVSPost);
  if (FAILED(hr))
    return hr;
  hr = g_pd3dDevice->CreateVertexShader(pBlobVSPost->GetBufferPointer(),
                                        pBlobVSPost->GetBufferSize(), nullptr,
                                        &g_pPostProcessVS);
  if (FAILED(hr)) {
    pBlobVSPost->Release();
    return hr;
  }

  D3D11_INPUT_ELEMENT_DESC layoutDesc_[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 4,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  hr = g_pd3dDevice->CreateInputLayout(
      layoutDesc_, _countof(layoutDesc_), pBlobVSPost->GetBufferPointer(),
      pBlobVSPost->GetBufferSize(), &g_pFullScreenLayout);
  if (FAILED(hr)) {
    pBlobVSPost->Release();
    return hr;
  }

  pBlobVSPost->Release();
  pBlobVSPost = nullptr;

  hr = CompileShaderFromFile(const_cast<wchar_t *>(L"post_processing.fx"), "PS",
                             "ps_5_0", &pBlobPSPost);
  if (FAILED(hr))
    return hr;

  hr = g_pd3dDevice->CreatePixelShader(pBlobPSPost->GetBufferPointer(),
                                       pBlobPSPost->GetBufferSize(), nullptr,
                                       &g_pPostProcessPS);

  pBlobPSPost->Release();
  pBlobPSPost = nullptr;

  if (FAILED(hr))
    return hr;
  bd = {};
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(FullScreenVertex) * 3;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;

  initData = {};
  initData.pSysMem = g_FullScreenTriangle;

  hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pFullScreenVB);
  if (FAILED(hr))
    return hr;

  ID3DBlob *pBlobCS = nullptr;
  hr = CompileShaderFromFile(const_cast<wchar_t *>(L"gpu_culling.hlsl"), "main",
                             "cs_5_0", &pBlobCS);
  if (FAILED(hr))
    return hr;

  hr = g_pd3dDevice->CreateComputeShader(pBlobCS->GetBufferPointer(),
                                         pBlobCS->GetBufferSize(), nullptr,
                                         &g_pCullingCS);
  pBlobCS->Release();
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC bufDesc = {};
  bufDesc.Usage = D3D11_USAGE_DEFAULT;
  bufDesc.ByteWidth = sizeof(UINT) * 4;
  bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
  bufDesc.CPUAccessFlags = 0;
  bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  bufDesc.StructureByteStride = sizeof(UINT);

  hr = g_pd3dDevice->CreateBuffer(&bufDesc, nullptr, &g_pIndirectArgsBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
  uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
  uavDesc.Buffer.FirstElement = 0;
  uavDesc.Buffer.NumElements = 4;
  uavDesc.Format = DXGI_FORMAT_UNKNOWN;

  hr = g_pd3dDevice->CreateUnorderedAccessView(g_pIndirectArgsBuffer, &uavDesc,
                                               &g_pIndirectArgsUAV);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC cullingParamsDesc = {};
  cullingParamsDesc.Usage = D3D11_USAGE_DEFAULT;
  cullingParamsDesc.ByteWidth = sizeof(CullingData);
  cullingParamsDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cullingParamsDesc.CPUAccessFlags = 0;
  cullingParamsDesc.MiscFlags = 0;

  hr = g_pd3dDevice->CreateBuffer(&cullingParamsDesc, nullptr,
                                  &g_pCullingParamsBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC stagingDesc = {};
  stagingDesc.Usage = D3D11_USAGE_STAGING;
  stagingDesc.ByteWidth = sizeof(UINT) * 4;
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  stagingDesc.BindFlags = 0;
  stagingDesc.MiscFlags = 0;
  hr = g_pd3dDevice->CreateBuffer(&stagingDesc, nullptr,
                                  &g_pIndirectArgsStagingBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC sceneDesc = {};
  sceneDesc.Usage = D3D11_USAGE_DEFAULT;
  sceneDesc.ByteWidth = sizeof(SceneCBData);
  sceneDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  sceneDesc.CPUAccessFlags = 0;
  sceneDesc.MiscFlags = 0;

  hr = g_pd3dDevice->CreateBuffer(&sceneDesc, nullptr, &g_pSceneCBBuffer);
  if (FAILED(hr))
    return hr;

  InitImGui();

  return S_OK;
}

void CleanupD3D() {

  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  if (g_pImmediateContext)
    g_pImmediateContext->ClearState();

  if (g_pCubeVertexBuffer)
    g_pCubeVertexBuffer->Release();
  if (g_pCubeInputLayout)
    g_pCubeInputLayout->Release();
  if (g_pCubeVS)
    g_pCubeVS->Release();
  if (g_pCubePS)
    g_pCubePS->Release();
  if (g_pCubeModelBuffer)
    g_pCubeModelBuffer->Release();
  if (g_pCubeVPBuffer)
    g_pCubeVPBuffer->Release();
  if (g_pCubeTextureRV)
    g_pCubeTextureRV->Release();

  if (g_pCubeNormalMapRV)
    g_pCubeNormalMapRV->Release();
  if (g_pSamplerLiinner)
    g_pSamplerLiinner->Release();
  if (g_pRenderTargetView)
    g_pRenderTargetView->Release();
  if (g_pDepthStencilView)
    g_pDepthStencilView->Release();
  if (g_pSwapChain)
    g_pSwapChain->Release();
  if (g_pImmediateContext)
    g_pImmediateContext->Release();
  if (g_pd3dDevice)
    g_pd3dDevice->Release();

  if (g_pSkyboxVertexBuffer)
    g_pSkyboxVertexBuffer->Release();
  if (g_pSkyboxInputLayout)
    g_pSkyboxInputLayout->Release();
  if (g_pSkyboxVS)
    g_pSkyboxVS->Release();
  if (g_pSkyboxPS)
    g_pSkyboxPS->Release();
  if (g_pSkyboxVPBuffer)
    g_pSkyboxVPBuffer->Release();
  if (g_pSkyboxTextureRV)
    g_pSkyboxTextureRV->Release();

  if (g_pTransparentPS)
    g_pTransparentPS->Release();
  if (g_pTransparentBuffer)
    g_pTransparentBuffer->Release();
  if (g_pAlphaBlendState)
    g_pAlphaBlendState->Release();
  if (g_pDSStateTrans)
    g_pDSStateTrans->Release();

  if (g_pLightBuffer)
    g_pLightBuffer->Release();
  if (g_pTransparentVS)
    g_pTransparentVS->Release();

  if (g_pPostProcessTex)
    g_pPostProcessTex->Release();
  if (g_pPostProcessRTV)
    g_pPostProcessRTV->Release();
  if (g_pPostProcessSRV)
    g_pPostProcessSRV->Release();
  if (g_pPostProcessVS)
    g_pPostProcessVS->Release();
  if (g_pPostProcessPS)
    g_pPostProcessPS->Release();

  if (g_pFullScreenVB)
    g_pFullScreenVB->Release();
  if (g_pFullScreenLayout)
    g_pFullScreenLayout->Release();
  if (g_pCullingCS)
    g_pCullingCS->Release();
  if (g_pCullingParamsBuffer)
    g_pCullingParamsBuffer->Release();
  if (g_pIndirectArgsBuffer)
    g_pIndirectArgsBuffer->Release();
  if (g_pIndirectArgsUAV)
    g_pIndirectArgsUAV->Release();
  if (g_pObjectsIdsBuffer)
    g_pObjectsIdsBuffer->Release();
  if (g_pObjectsIdsUAV)
    g_pObjectsIdsUAV->Release();
  if (g_pIndirectArgsStagingBuffer)
    g_pIndirectArgsStagingBuffer->Release();
  if (g_pSceneCBBuffer)
    g_pSceneCBBuffer->Release();
}

void RenderCubes() {
  if (!g_EnablePostProcessFilter) {
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView,
                                            g_pDepthStencilView);
    float ClearColor[4] = {0.2f, 0.2f, 0.4f, 1.0f};
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView,
                                               D3D11_CLEAR_DEPTH, 1.0f, 0);
  }

  RECT rc;
  GetClientRect(FindWindow(windowClass.c_str(), windowTitle.c_str()), &rc);
  float aspect = static_cast<float>(rc.right - rc.left) / (rc.bottom - rc.top);
  XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 100.0f);

  float camRadius = 6.0f;
  float camX = camRadius * sinf(g_CameraAzimuth) * cosf(g_CameraElevation);
  float camY = camRadius * sinf(g_CameraElevation);
  float camZ = camRadius * cosf(g_CameraAzimuth) * cosf(g_CameraElevation);
  XMVECTOR eyePos = XMVectorSet(camX, camY, camZ, 0.0f);
  XMVECTOR focusPoint = XMVectorZero();
  XMVECTOR upDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  XMMATRIX view = XMMatrixLookAtLH(eyePos, focusPoint, upDir);

  XMMATRIX viewSkybox = view;
  viewSkybox.r[3] = XMVectorSet(0, 0, 0, 1);
  XMMATRIX vpSkybox = XMMatrixTranspose(viewSkybox * proj);

  D3D11_MAPPED_SUBRESOURCE mappedResource;
  if (SUCCEEDED(g_pImmediateContext->Map(
          g_pSkyboxVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
    memcpy(mappedResource.pData, &vpSkybox, sizeof(XMMATRIX));
    g_pImmediateContext->Unmap(g_pSkyboxVPBuffer, 0);
  }

  D3D11_DEPTH_STENCIL_DESC dsDesc = {};
  dsDesc.DepthEnable = true;
  dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
  ID3D11DepthStencilState *pDSStateSkybox = nullptr;
  g_pd3dDevice->CreateDepthStencilState(&dsDesc, &pDSStateSkybox);
  g_pImmediateContext->OMSetDepthStencilState(pDSStateSkybox, 0);

  D3D11_RASTERIZER_DESC rsDesc = {};
  rsDesc.FillMode = D3D11_FILL_SOLID;
  rsDesc.CullMode = D3D11_CULL_FRONT;
  rsDesc.FrontCounterClockwise = false;
  ID3D11RasterizerState *pSkyboxRS = nullptr;
  if (SUCCEEDED(g_pd3dDevice->CreateRasterizerState(&rsDesc, &pSkyboxRS))) {
    g_pImmediateContext->RSSetState(pSkyboxRS);
  }

  UINT stride = sizeof(SkyboxVertex);
  UINT offset = 0;
  g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pSkyboxVertexBuffer, &stride,
                                          &offset);
  g_pImmediateContext->IASetInputLayout(g_pSkyboxInputLayout);
  g_pImmediateContext->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  g_pImmediateContext->VSSetShader(g_pSkyboxVS, nullptr, 0);
  g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pSkyboxVPBuffer);
  g_pImmediateContext->PSSetShader(g_pSkyboxPS, nullptr, 0);
  g_pImmediateContext->PSSetShaderResources(0, 1, &g_pSkyboxTextureRV);
  g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLiinner);
  g_pImmediateContext->Draw(ARRAYSIZE(g_SkyboxVertices), 0);

  pDSStateSkybox->Release();
  if (pSkyboxRS) {
    pSkyboxRS->Release();
    g_pImmediateContext->RSSetState(nullptr);
  }
  g_pImmediateContext->OMSetDepthStencilState(nullptr, 0);

  g_CubeAngle += 0.005f;

  XMMATRIX vpCube = XMMatrixTranspose(view * proj);
  if (SUCCEEDED(g_pImmediateContext->Map(
          g_pCubeVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
    memcpy(mappedResource.pData, &vpCube, sizeof(XMMATRIX));
    g_pImmediateContext->Unmap(g_pCubeVPBuffer, 0);
  }

  const int innerCubesCount = 10;
  const int outerCubesCount = 10;
  const int totalInstances = innerCubesCount + outerCubesCount;
  g_totalInstances = totalInstances;

  XMMATRIX instanceMatrices[totalInstances];

  float innerOrbitRadius = 3.0f;
  float innerStep = XM_2PI / 10.0f;
  for (int i = 0; i < innerCubesCount; i++) {
    float offsetAngle = (i - 1) * innerStep;
    float orbitAngle = g_CubeAngle + offsetAngle;
    XMMATRIX translation =
        XMMatrixTranslation(innerOrbitRadius * cosf(orbitAngle), 0.0f,
                            innerOrbitRadius * sinf(orbitAngle));
    instanceMatrices[i] = translation;
  }

  float outerOrbitRadius = 15.0f;
  float outerStep = XM_2PI / outerCubesCount;
  for (int i = innerCubesCount; i < totalInstances; i++) {
    float offsetAngle = (i - innerCubesCount) * outerStep;
    float orbitAngle = g_CubeAngle + offsetAngle;
    XMMATRIX translation =
        XMMatrixTranslation(outerOrbitRadius * cosf(orbitAngle), 0.0f,
                            outerOrbitRadius * sinf(orbitAngle));
    instanceMatrices[i] = translation;
  }

  XMMATRIX matVP = view * proj;
  Plane frustumPlanes[6];
  ExtractFrustumPlanes(matVP, frustumPlanes);
  for (int i = 0; i < 6; i++) {
    float len = sqrtf(frustumPlanes[i].a * frustumPlanes[i].a +
                      frustumPlanes[i].b * frustumPlanes[i].b +
                      frustumPlanes[i].c * frustumPlanes[i].c);
    frustumPlanes[i].a /= len;
    frustumPlanes[i].b /= len;
    frustumPlanes[i].c /= len;
    frustumPlanes[i].d /= len;
  }

  if (g_EnableGPUCulling) {
    CullingData cullingData = {};
    cullingData.numShapes = totalInstances;
    for (int i = 0; i < totalInstances; i++) {
      XMVECTOR center = instanceMatrices[i].r[3];
      XMVECTOR offset = XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f);
      XMVECTOR minV = XMVectorSubtract(center, offset);
      XMVECTOR maxV = XMVectorAdd(center, offset);
      XMStoreFloat4(&cullingData.bbMin[i], minV);
      XMStoreFloat4(&cullingData.bbMax[i], maxV);
    }
    g_pImmediateContext->UpdateSubresource(g_pCullingParamsBuffer, 0, nullptr,
                                           &cullingData, 0, 0);

    SceneCBData sceneData;
    XMStoreFloat4x4(&sceneData.viewProjectionMatrix,
                    XMMatrixTranspose(view * proj));
    for (int i = 0; i < 6; i++) {
      sceneData.planes[i] = XMFLOAT4(frustumPlanes[i].a, frustumPlanes[i].b,
                                     frustumPlanes[i].c, frustumPlanes[i].d);
    }

    g_pImmediateContext->UpdateSubresource(g_pSceneCBBuffer, 0, nullptr,
                                           &sceneData, 0, 0);
    g_pImmediateContext->CSSetConstantBuffers(1, 1, &g_pSceneCBBuffer);

    UINT clearVals[4] = {0, 0, 0, 0};
    g_pImmediateContext->ClearUnorderedAccessViewUint(g_pIndirectArgsUAV,
                                                      clearVals);

    g_pImmediateContext->CSSetShader(g_pCullingCS, nullptr, 0);
    g_pImmediateContext->CSSetConstantBuffers(0, 1, &g_pCullingParamsBuffer);
    g_pImmediateContext->CSSetUnorderedAccessViews(0, 1, &g_pIndirectArgsUAV,
                                                   nullptr);
    g_pImmediateContext->CSSetUnorderedAccessViews(1, 1, &g_pObjectsIdsUAV,
                                                   nullptr);

    UINT numGroups = (totalInstances + 63) / 64;
    g_pImmediateContext->Dispatch(numGroups, 1, 1);

    ID3D11UnorderedAccessView *nullUAV[2] = {nullptr, nullptr};
    g_pImmediateContext->CSSetUnorderedAccessViews(0, 2, nullUAV, nullptr);
    g_pImmediateContext->CSSetShader(nullptr, nullptr, 0);

    g_pImmediateContext->CopyResource(g_pIndirectArgsStagingBuffer,
                                      g_pIndirectArgsBuffer);

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    HRESULT hr = g_pImmediateContext->Map(g_pIndirectArgsStagingBuffer, 0,
                                          D3D11_MAP_READ, 0, &mapped);
    UINT visibleCount = 0;
    if (SUCCEEDED(hr)) {
      UINT *pData = (UINT *)mapped.pData;
      visibleCount = pData[1];
      g_pImmediateContext->Unmap(g_pIndirectArgsStagingBuffer, 0);
    }
    g_visibleCubesGPU = visibleCount;
    g_visibleCubesСPU = totalInstances;
    XMMATRIX transposedMatrices[totalInstances];
    for (int i = 0; i < totalInstances; i++) {
      transposedMatrices[i] = XMMatrixTranspose(instanceMatrices[i]);
    }
    g_pImmediateContext->UpdateSubresource(g_pCubeModelBuffer, 0, nullptr,
                                           transposedMatrices, 0, 0);

    g_pImmediateContext->DrawInstanced(ARRAYSIZE(g_CubeVertices), visibleCount,
                                       0, 0);
  } else {
    XMMATRIX visibleInstances[totalInstances];
    int finalInstanceCount = 0;
    if (g_EnableCPUCulling) {
      for (int i = 0; i < totalInstances; i++) {
        XMVECTOR center =
            XMVectorSet(instanceMatrices[i].r[3].m128_f32[0],
                        instanceMatrices[i].r[3].m128_f32[1],
                        instanceMatrices[i].r[3].m128_f32[2], 1.0f);
        if (IsSphereInFrustum(frustumPlanes, center, 1.0f)) {
          visibleInstances[finalInstanceCount++] = instanceMatrices[i];
        }
      }
    } else {
      memcpy(visibleInstances, instanceMatrices, sizeof(instanceMatrices));
      finalInstanceCount = totalInstances;
    }
    g_visibleCubesСPU = finalInstanceCount;
    g_visibleCubesGPU = totalInstances;
    XMMATRIX finalMatricesT[totalInstances];
    for (int i = 0; i < finalInstanceCount; i++) {
      finalMatricesT[i] = XMMatrixTranspose(visibleInstances[i]);
    }
    g_pImmediateContext->UpdateSubresource(g_pCubeModelBuffer, 0, nullptr,
                                           finalMatricesT, 0, 0);
    g_pImmediateContext->DrawInstanced(ARRAYSIZE(g_CubeVertices),
                                       finalInstanceCount, 0, 0);
  }

  stride = sizeof(SimpleVertex);
  offset = 0;
  g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pCubeVertexBuffer, &stride,
                                          &offset);
  g_pImmediateContext->IASetInputLayout(g_pCubeInputLayout);
  g_pImmediateContext->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  g_pImmediateContext->VSSetShader(g_pCubeVS, nullptr, 0);
  g_pImmediateContext->PSSetShader(g_pCubePS, nullptr, 0);
  g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCubeModelBuffer);
  g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCubeVPBuffer);
  g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pLightBuffer);

  ID3D11ShaderResourceView *cubeSRVs[2] = {g_pCubeTextureRV,
                                           g_pCubeNormalMapRV};
  g_pImmediateContext->PSSetShaderResources(0, 2, cubeSRVs);
  g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLiinner);

  g_pImmediateContext->DrawInstanced(ARRAYSIZE(g_CubeVertices),
                                     g_visibleCubesСPU, 0, 0);

  LightBufferType lightData;
  lightData.light0Pos = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f);
  lightData.light0Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f);
  lightData.light1Pos = XMFLOAT4(-1.0f, 0.0f, 0.0, 0.0f);
  lightData.light1Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f);
  lightData.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
  g_pImmediateContext->UpdateSubresource(g_pLightBuffer, 0, nullptr, &lightData,
                                         0, 0);

  XMMATRIX lightScale = XMMatrixScaling(0.1f, 0.1f, 0.1f);

  XMMATRIX lightTranslate0 = XMMatrixTranslation(1.0f, 1.0f, 0.0f);
  XMMATRIX lightModel0 = XMMatrixTranspose(lightScale * lightTranslate0);
  g_pImmediateContext->UpdateSubresource(g_pCubeModelBuffer, 0, nullptr,
                                         &lightModel0, 0, 0);
  XMFLOAT4 lightColor0(1.0f, 1.0f, 0.0f, 1.0f);
  g_pImmediateContext->Draw(ARRAYSIZE(g_CubeVertices), 0);

  XMMATRIX lightTranslate1 = XMMatrixTranslation(-1.0f, -1.0f, 0.0f);
  XMMATRIX lightModel1 = XMMatrixTranspose(lightScale * lightTranslate1);
  g_pImmediateContext->UpdateSubresource(g_pCubeModelBuffer, 0, nullptr,
                                         &lightModel1, 0, 0);
  XMFLOAT4 lightColor1(1.0f, 1.0f, 1.0f, 1.0f);
  g_pImmediateContext->Draw(ARRAYSIZE(g_CubeVertices), 0);
}

void SetUpImGui() {
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(60, 40), ImGuiCond_FirstUseEver);

  ImGui::Begin("Options");
  ImGui::Checkbox("Grayscale Filter", &g_EnablePostProcessFilter);
  if (ImGui::Checkbox("Frustum Culling (CPU)", &g_EnableCPUCulling)) {
    if (g_EnableCPUCulling) {
      g_EnableGPUCulling = false;
    }
  }
  if (ImGui::Checkbox("Frustum Culling (GPU)", &g_EnableGPUCulling)) {
    if (g_EnableGPUCulling) {
      g_EnableCPUCulling = false;
    }
  }
  ImGui::Text("Total cubes: %d", g_totalInstances);
  ImGui::Text("Visible cubes (CPU): %d", g_visibleCubesСPU);
  ImGui::Text("Visible cubes (GPU): %d", g_visibleCubesGPU);
  ImGui::End();

  ImGui::Render();

  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Render() {
  static float ClearColor[4] = {0.2f, 0.2f, 0.2f, 1.0f};

  if (g_EnablePostProcessFilter) {
    g_pImmediateContext->OMSetRenderTargets(1, &g_pPostProcessRTV,
                                            g_pDepthStencilView);
    g_pImmediateContext->ClearRenderTargetView(g_pPostProcessRTV, ClearColor);
  } else {
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView,
                                            g_pDepthStencilView);
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
  }
  g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView,
                                             D3D11_CLEAR_DEPTH, 1.0f, 0);
  RenderCubes();
  if (g_EnablePostProcessFilter) {
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

    g_pImmediateContext->OMSetDepthStencilState(nullptr, 0);

    UINT stride = sizeof(FullScreenVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pFullScreenVB, &stride,
                                            &offset);
    g_pImmediateContext->IASetInputLayout(g_pFullScreenLayout);
    g_pImmediateContext->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    g_pImmediateContext->VSSetShader(g_pPostProcessVS, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pPostProcessPS, nullptr, 0);

    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pPostProcessSRV);
    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLiinner);

    g_pImmediateContext->Draw(3, 0);

    ID3D11ShaderResourceView *nullSRV[1] = {nullptr};
    g_pImmediateContext->PSSetShaderResources(0, 1, nullSRV);
  }

  SetUpImGui();

  g_pSwapChain->Present(1, 0);
}