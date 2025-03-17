#include <string>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "vertices.h"
#include <vector>
#include <algorithm>

using namespace DirectX;

ID3D11Device *g_pd3dDevice = nullptr;
ID3D11DeviceContext *g_pImmediateContext = nullptr;
IDXGISwapChain *g_pSwapChain = nullptr;
ID3D11RenderTargetView *g_pRenderTargetView = nullptr;
ID3D11DepthStencilView *g_pDepthStencilView = nullptr;
ID3D11SamplerState* g_pSamplerLinear = nullptr;

ID3D11VertexShader *g_pCubeVS = nullptr;
ID3D11PixelShader *g_pCubePS = nullptr;
ID3D11InputLayout *g_pCubeInputLayout = nullptr;
ID3D11Buffer *g_pCubeVertexBuffer = nullptr;
ID3D11Buffer *g_pCubeModelBuffer = nullptr;
ID3D11Buffer *g_pCubeVPBuffer = nullptr;
ID3D11ShaderResourceView *g_pCubeTextureRV = nullptr;

ID3D11VertexShader *g_pSkyboxVS = nullptr;
ID3D11PixelShader *g_pSkyboxPS = nullptr;
ID3D11InputLayout *g_pSkyboxInputLayout = nullptr;
ID3D11Buffer *g_pSkyboxVertexBuffer = nullptr;
ID3D11Buffer *g_pSkyboxVPBuffer = nullptr;
ID3D11ShaderResourceView *g_pSkyboxTextureRV = nullptr;

ID3D11VertexShader* g_pColorCubeVS = nullptr;
ID3D11PixelShader* g_pColorCubePS = nullptr;
ID3D11InputLayout* g_pColorCubeInputLayout = nullptr;
ID3D11Buffer* g_pColorCubeModelBuffer = nullptr;
ID3D11Buffer* g_pColorCubeColorBuffer = nullptr;
ID3D11BlendState* g_pBlendState = nullptr;
ID3D11DepthStencilState* g_pTransparentDepthState = nullptr;
ID3D11DepthStencilState* g_pTransparentDepthStencilState = nullptr;
ID3D11BlendState* g_pTransparentBlendState = nullptr;

float g_CubeAngle = 0.0f;
float g_CameraAngle = 0.0f;
bool g_MouseDragging = false;
POINT g_LastMousePos = {0, 0};
float g_CameraAzimuth = 0.0f;
float g_CameraElevation = 0.0f;

struct Light {
	XMFLOAT4 Position;
	XMFLOAT4 Color;
	XMFLOAT4 Attenuation;
};

ID3D11Buffer* g_pLightBuffer = nullptr;
ID3D11Buffer* g_pCameraBuffer = nullptr;
XMFLOAT3 g_CameraPos;

BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitDevice(HWND hWnd);
HRESULT InitGraphics();
void CleanupDevice();
void Render();

const std::wstring windowTitle = L"Mikhail Markov";
const std::wstring windowClass = L"MikhailMarkovClass";

struct CubeData {
	XMFLOAT4 color;
	XMMATRIX modelMatrix;
	bool isTransparent;
	bool isTextured;
};

struct modelData {
	XMMATRIX model;
	XMMATRIX normalMatrix;
} ;

std::vector<CubeData> g_Cubes = {
	{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMMatrixTranslation(0.0f, 0.0f, 0.0f), false, true},
	{ XMFLOAT4(1.0f, 0.0f, 0.0f, 0.5f), XMMatrixIdentity(), true, false },
	{ XMFLOAT4(0.0f, 1.0f, 0.0f, 0.5f), XMMatrixIdentity(), true, false },
	{ XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMMatrixIdentity(), false, false }
};

Light g_Lights[2] = { 
	{ XMFLOAT4(2.0f, 2.0f, 2.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.1f, 0.01f, 0.0f) },
	{ XMFLOAT4(-2.0f, 2.0f, -2.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f), XMFLOAT4(1.0f, 0.1f, 0.01f, 0.0f) } };

std::vector<CubeData*> g_TransparentObjects;
std::vector<CubeData*> g_NonTransparentObjects;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hbrBackground = nullptr;
	wcex.lpszClassName = windowClass.c_str();
	RegisterClassExW(&wcex);

	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	HWND hWnd = FindWindow(windowClass.c_str(), windowTitle.c_str());
	if (!hWnd)
		return FALSE;

	if (FAILED(InitDevice(hWnd)))
		return FALSE;

	if (FAILED(InitGraphics()))
	{
		CleanupDevice();
		return FALSE;
	}

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}
	CleanupDevice();
	return (int)msg.wParam;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd = CreateWindowW(windowClass.c_str(), windowTitle.c_str(), WS_OVERLAPPEDWINDOW,
							  CW_USEDEFAULT, 0, 800, 600,
							  nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
		return FALSE;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}

HRESULT InitDevice(HWND hWnd)
{
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
	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
											   createDeviceFlags, featureLevelArray, 1,
											   D3D11_SDK_VERSION, &sd, &g_pSwapChain,
											   &g_pd3dDevice, &featureLevel, &g_pImmediateContext);
	if (FAILED(hr))
		return hr;

	ID3D11Texture2D *pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackBuffer);
	if (FAILED(hr))
		return hr;
	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
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
	{
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);
		if (FAILED(hr)) return hr;
	}
	hr = g_pd3dDevice->CreateDepthStencilView(pDepthStencil, nullptr, &g_pDepthStencilView);
	pDepthStencil->Release();
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	D3D11_VIEWPORT vp = {};
	vp.Width = static_cast<FLOAT>(width);
	vp.Height = static_cast<FLOAT>(height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	return S_OK;
}

HRESULT CompileShaderFromFile(WCHAR *szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob **ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob *pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
							dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char *>(pErrorBlob->GetBufferPointer()));
			MessageBoxA(nullptr, (char *)pErrorBlob->GetBufferPointer(), "Shader Compile Error", MB_OK);
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob)
		pErrorBlob->Release();

	return S_OK;
}

HRESULT InitGraphics()
{
	HRESULT hr = S_OK;
	ID3DBlob *pBlob = nullptr;

	hr = CompileShaderFromFile(const_cast<wchar_t *>(L"lab6.fx"), "VS", "vs_4_0", &pBlob);
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
										  nullptr, &g_pCubeVS);
	if (FAILED(hr))
	{
		pBlob->Release();
		return hr;
	}
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = g_pd3dDevice->CreateInputLayout(layoutDesc, 3, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &g_pCubeInputLayout);
	pBlob->Release();
	if (FAILED(hr))
		return hr;

	hr = CompileShaderFromFile(const_cast<wchar_t *>(L"lab6.fx"), "PS", "ps_4_0", &pBlob);
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
										 nullptr, &g_pCubePS);
	pBlob->Release();
	if (FAILED(hr))
		return hr;

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 36;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = Vertices::g_CubeVertices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &initData, &g_pCubeVertexBuffer);
	if (FAILED(hr))
		return hr;

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = 2 * sizeof(XMMATRIX);
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

	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"cube.dds", nullptr, &g_pCubeTextureRV);
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
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
	if (FAILED(hr))
		return hr;

	hr = CompileShaderFromFile(const_cast<wchar_t *>(L"skybox.fx"), "SkyboxVS", "vs_4_0", &pBlob);
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
										  nullptr, &g_pSkyboxVS);
	if (FAILED(hr))
	{
		pBlob->Release();
		return hr;
	}

	D3D11_INPUT_ELEMENT_DESC skyboxLayout[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
	hr = g_pd3dDevice->CreateInputLayout(skyboxLayout, 1, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &g_pSkyboxInputLayout);
	pBlob->Release();
	if (FAILED(hr))
		return hr;

	hr = CompileShaderFromFile(const_cast<wchar_t *>(L"skybox.fx"), "SkyboxPS", "ps_4_0", &pBlob);
	if (FAILED(hr))
		return hr;
	hr = g_pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
										 nullptr, &g_pSkyboxPS);
	pBlob->Release();
	if (FAILED(hr))
		return hr;

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SkyboxVertex) * 36;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	initData.pSysMem = Vertices::g_SkyboxVertices;
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

	hr = CreateDDSTextureFromFile(g_pd3dDevice, L"skybox.dds", nullptr, &g_pSkyboxTextureRV);
	if (FAILED(hr))
		return hr;

	pBlob = nullptr;

	hr = CompileShaderFromFile(const_cast<wchar_t*>(L"transparent.fx"), "VS", "vs_4_0", &pBlob);
	if (FAILED(hr)) return hr;
	hr = g_pd3dDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &g_pColorCubeVS);
	if (FAILED(hr)) { pBlob->Release(); return hr; }

	D3D11_INPUT_ELEMENT_DESC layoutColorDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = g_pd3dDevice->CreateInputLayout(layoutColorDesc, 2, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &g_pColorCubeInputLayout);
	pBlob->Release();
	if (FAILED(hr)) 
		return hr;

	hr = CompileShaderFromFile(const_cast<wchar_t*>(L"transparent.fx"), "PS", "ps_4_0", &pBlob);
	if (FAILED(hr)) return hr;
	hr = g_pd3dDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &g_pColorCubePS);
	pBlob->Release();
	if (FAILED(hr)) 
		return hr;

	bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = 2 * sizeof(XMMATRIX);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pColorCubeModelBuffer);
	if (FAILED(hr)) return hr;

	bd.ByteWidth = sizeof(XMFLOAT4);
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pColorCubeColorBuffer);
	if (FAILED(hr)) return hr;

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = g_pd3dDevice->CreateBlendState(&blendDesc, &g_pTransparentBlendState);
	if (FAILED(hr))
		return hr;

	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = TRUE;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	hr = g_pd3dDevice->CreateDepthStencilState(&depthDesc, &g_pTransparentDepthState);
	if (FAILED(hr))
		return hr;

	g_Cubes[1].modelMatrix = XMMatrixTranslation(-2.0f, 0.0f, 0.0f);
	g_Cubes[2].modelMatrix = XMMatrixTranslation(2.0f, 0.0f, 0.0f);
	g_Cubes[3].modelMatrix = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

	D3D11_BUFFER_DESC lightBufferDesc = {};
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(Light) * 2;
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = g_pd3dDevice->CreateBuffer(&lightBufferDesc, nullptr, &g_pLightBuffer);

	D3D11_BUFFER_DESC cameraBufferDesc = {};
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = g_pd3dDevice->CreateBuffer(&cameraBufferDesc, nullptr, &g_pCameraBuffer);

	return S_OK;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
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
		if (g_MouseDragging)
		{
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

void CleanupDevice()
{
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
	if (g_pSamplerLinear)
		g_pSamplerLinear->Release();
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
	if (g_pColorCubeModelBuffer) g_pColorCubeModelBuffer->Release();
	if (g_pColorCubeColorBuffer) g_pColorCubeColorBuffer->Release();
	if (g_pColorCubeInputLayout) g_pColorCubeInputLayout->Release();
	if (g_pColorCubeVS) g_pColorCubeVS->Release();
	if (g_pColorCubePS) g_pColorCubePS->Release();
	if (g_pBlendState) g_pBlendState->Release();
	if (g_pTransparentDepthState) g_pTransparentDepthState->Release();
	if (g_pTransparentDepthStencilState) g_pTransparentDepthStencilState->Release();
	if (g_pTransparentBlendState) g_pTransparentBlendState->Release();
	if (g_pLightBuffer) g_pLightBuffer->Release();
	if (g_pCameraBuffer) g_pCameraBuffer->Release();
}

void SkyboxRender()
{
	UINT stride = sizeof(SkyboxVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pSkyboxVertexBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pSkyboxInputLayout);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pImmediateContext->VSSetShader(g_pSkyboxVS, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pSkyboxPS, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pSkyboxVPBuffer);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pSkyboxTextureRV);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
	g_pImmediateContext->Draw(36, 0);
}

XMVECTOR UpdateCamera(XMMATRIX& view, XMMATRIX& proj)
{
	RECT rc;
	GetClientRect(FindWindow(windowClass.c_str(), windowTitle.c_str()), &rc);
	float aspect = static_cast<float>(rc.right - rc.left) / (rc.bottom - rc.top);
	proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 0.1f, 100.0f);

	static const float radius = 7.0f;
	float camX = radius * sinf(g_CameraAzimuth) * cosf(g_CameraElevation);
	float camY = radius * sinf(g_CameraElevation);
	float camZ = radius * cosf(g_CameraAzimuth) * cosf(g_CameraElevation);
	XMVECTOR eyePos = XMVectorSet(camX, camY, camZ, 0.0f);
	XMVECTOR focusPoint = XMVectorZero();
	XMVECTOR upDir = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	view = XMMatrixLookAtLH(eyePos, focusPoint, upDir);
	return XMVectorSet(camX, camY, camZ, 0.0f);
}

void SetupSkyboxStates()
{
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	ID3D11DepthStencilState* pDSStateSkybox = nullptr;
	g_pd3dDevice->CreateDepthStencilState(&dsDesc, &pDSStateSkybox);
	g_pImmediateContext->OMSetDepthStencilState(pDSStateSkybox, 0);
	pDSStateSkybox->Release();

	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.FrontCounterClockwise = false;
	ID3D11RasterizerState* pSkyboxRS = nullptr;
	if (SUCCEEDED(g_pd3dDevice->CreateRasterizerState(&rsDesc, &pSkyboxRS)))
	{
		g_pImmediateContext->RSSetState(pSkyboxRS);
		pSkyboxRS->Release();
	}
}

modelData prepareModelData(CubeData* cube)
{
	XMMATRIX modelMatrix = cube->modelMatrix;

	XMMATRIX invTransposeModel = XMMatrixInverse(nullptr, modelMatrix);
	XMMATRIX normalT = XMMatrixTranspose(invTransposeModel);

	XMMATRIX modelT = XMMatrixTranspose(modelMatrix);

	return { modelT,normalT };
}

void PrepareTextureCube(CubeData* cube)
{
	g_pImmediateContext->IASetInputLayout(g_pCubeInputLayout);
	g_pImmediateContext->VSSetShader(g_pCubeVS, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pCubePS, nullptr, 0);
	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pCubeTextureRV);
	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);

	modelData data = prepareModelData(cube);
	g_pImmediateContext->UpdateSubresource(g_pCubeModelBuffer, 0, nullptr, &data, 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCubeModelBuffer);    
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCubeVPBuffer);       
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightBuffer);        
	g_pImmediateContext->PSSetConstantBuffers(3, 1, &g_pCameraBuffer);       
}

void PrepareColorCube(CubeData* cube)
{
	g_pImmediateContext->IASetInputLayout(g_pColorCubeInputLayout);
	g_pImmediateContext->VSSetShader(g_pColorCubeVS, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pColorCubePS, nullptr, 0);

	modelData data = prepareModelData(cube);
	g_pImmediateContext->UpdateSubresource(g_pColorCubeModelBuffer, 0, nullptr, &data, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pColorCubeColorBuffer, 0, nullptr, &(cube->color), 0, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pColorCubeModelBuffer); 
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCubeVPBuffer);         
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pColorCubeColorBuffer); 
	g_pImmediateContext->PSSetConstantBuffers(3, 1, &g_pLightBuffer);          
	g_pImmediateContext->PSSetConstantBuffers(4, 1, &g_pCameraBuffer);         
}

void DrawCube()
{
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pCubeVertexBuffer, &stride, &offset);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pCubeVPBuffer);
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_pImmediateContext->Draw(36, 0);
}

template<typename F>
void RenderCube(CubeData* cube, F&& postPrepareFunc)
{
	if (cube->isTextured) 
		PrepareTextureCube(cube);
	else 
		PrepareColorCube(cube);
	postPrepareFunc();
	DrawCube();
}

void SortTransparentObjects(XMVECTOR cameraPosition, std::vector<CubeData*>& transparentCubes) {
	std::sort(transparentCubes.begin(), transparentCubes.end(),
		[cameraPosition](const auto& a, const auto& b) {
			return XMVectorGetX(XMVector3LengthSq(a->modelMatrix.r[3] - cameraPosition)) > 
				XMVectorGetX(XMVector3LengthSq(b->modelMatrix.r[3] - cameraPosition));
		});

}

void RenderCubes(const XMMATRIX& view, const XMMATRIX& proj, XMVECTOR cameraPos) {
	XMMATRIX vp = XMMatrixTranspose(view * proj);

	D3D11_MAPPED_SUBRESOURCE mapped;
	if (SUCCEEDED(g_pImmediateContext->Map(g_pCubeVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
		memcpy(mapped.pData, &vp, sizeof(XMMATRIX));
		g_pImmediateContext->Unmap(g_pCubeVPBuffer, 0);
	}
	static float cubeAngle = 0.0f;
	cubeAngle += 0.005f;

	const float zAmplitude = 3.0f;
	const float zFrequency = 0.8f;

	float z = zAmplitude * sinf(cubeAngle * zFrequency);

	g_Cubes[0].modelMatrix = XMMatrixTranslation(0.0f, 0.0f, z) *
		XMMatrixRotationZ(cubeAngle * 2.5f);

	g_TransparentObjects.clear();
	g_NonTransparentObjects.clear();

	for (auto& cube : g_Cubes)
		if (cube.isTransparent)
			g_TransparentObjects.push_back(&cube);
		else
			g_NonTransparentObjects.push_back(&cube);
	
	SortTransparentObjects(cameraPos, g_TransparentObjects);

	for (auto& cube : g_NonTransparentObjects) {
		RenderCube(cube, []() {g_pImmediateContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		g_pImmediateContext->OMSetDepthStencilState(nullptr, 0);});
	}
	for (auto& cube : g_TransparentObjects) 
	{
		RenderCube(cube, []() {
			float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			g_pImmediateContext->OMSetBlendState(g_pTransparentBlendState, blendFactor, 0xffffffff);
			g_pImmediateContext->OMSetDepthStencilState(g_pTransparentDepthState, 0);});
	}
}


void Render() {
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	XMMATRIX view, proj;
	XMVECTOR camPos = UpdateCamera(view, proj);
	XMStoreFloat3(&g_CameraPos, camPos);

	D3D11_MAPPED_SUBRESOURCE mapped;
	if (SUCCEEDED(g_pImmediateContext->Map(g_pCameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
		memcpy(mapped.pData, &g_CameraPos, sizeof(XMFLOAT3));
		g_pImmediateContext->Unmap(g_pCameraBuffer, 0);
	}


	XMMATRIX viewSkybox = view;
	viewSkybox.r[3] = XMVectorSet(0, 0, 0, 1);
	XMMATRIX vpSkybox = XMMatrixTranspose(viewSkybox * proj);

	if (SUCCEEDED(g_pImmediateContext->Map(g_pSkyboxVPBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
		memcpy(mapped.pData, &vpSkybox, sizeof(XMMATRIX));
		g_pImmediateContext->Unmap(g_pSkyboxVPBuffer, 0);
	}

	if (SUCCEEDED(g_pImmediateContext->Map(g_pLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
		memcpy(mapped.pData, g_Lights, sizeof(Light) * 2);
		g_pImmediateContext->Unmap(g_pLightBuffer, 0);
	}

	SetupSkyboxStates();
	SkyboxRender();
	g_pImmediateContext->RSSetState(nullptr);

	RenderCubes(view, proj, camPos);

	g_pSwapChain->Present(1, 0);
}
