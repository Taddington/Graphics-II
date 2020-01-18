// D3D11Base.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "D3D11Base.h"

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "DDSTextureLoader.h"

#include <DirectXMath.h>
using namespace DirectX;

#include "MyVShader.csh"
#include "MyPShader.csh"

#include "MyMeshVShader.csh" // don't add a .csh to you project!
#include "MyMeshPShader.csh"

#include "MyLoadVShader.csh"
#include "MyLoadPShader.csh"

// pre-made mesh
#include "Assets/StoneHenge.h"
#include <Windows.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <atlbase.h>
using namespace std;

// for init
CComPtr<ID3D11Device> myDev;
CComPtr<IDXGISwapChain> mySwap;
CComPtr<ID3D11DeviceContext> myCon;

// for drawing
CComPtr<ID3D11RenderTargetView> myRtv;
D3D11_VIEWPORT myPort;
float aspectRatio = 1;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};

struct SimpleMesh
{
	vector<SimpleVertex> verticesList;
	vector<int> indicesList;
};
SimpleMesh d20Mesh;

struct MyVertex
{
	float xyzw[4];
	float rgba[4];
};

struct BaseMesh
{
	vector<MyVertex> vertList;
	vector<int> indexList;
};
BaseMesh procedure;
int numIndices = 0;
unsigned int numVerts = 0;

#pragma region ProceduralD3D11Interfaces
CComPtr<ID3D11Buffer> vBuff;
CComPtr<ID3D11Buffer> iBuff;
CComPtr<ID3D11InputLayout> vLayout;
CComPtr<ID3D11VertexShader> vShader; // HLSL
CComPtr<ID3D11PixelShader> pShader; // HLSL
#pragma endregion

#pragma region StoneHengeD3D11Interfaces
// mesh data
CComPtr<ID3D11Buffer> vBuffMesh; // vertex buffer
CComPtr<ID3D11Buffer> iBuffMesh; // index buffer

// mesh vertex shader
CComPtr<ID3D11VertexShader> vMeshShader; // HLSL
CComPtr<ID3D11InputLayout> vMeshLayout;
CComPtr<ID3D11PixelShader> pMeshShader; // HLSL

// texture varialbles
CComPtr<ID3D11Texture2D> mTexture;
CComPtr<ID3D11ShaderResourceView> textureRV;
#pragma endregion

CComPtr<ID3D11InputLayout> d20MeshLayout;
CComPtr<ID3D11Buffer> d20vBuff;
CComPtr<ID3D11Buffer> d20iBuff;
CComPtr<ID3D11Texture2D> d20Texture;
CComPtr<ID3D11ShaderResourceView> d20textureRV;
CComPtr<ID3D11VertexShader> d20vShader;
CComPtr<ID3D11PixelShader> d20pShader;


CComPtr<ID3D11Buffer> cBuff; // shader vars
// X buffer for depth sorting
CComPtr<ID3D11Texture2D> zBuffer;
CComPtr<ID3D11DepthStencilView> zBufferView;

// Math stuff
struct ConstantBuffer
{
	XMFLOAT4X4 wMatrix; // storage type
	XMFLOAT4X4 vMatrix;
	XMFLOAT4X4 pMatrix;
	XMFLOAT4 directionalLightPos = XMFLOAT4(-0.577f, -0.577f, 0.577f, 1.0f);
	XMFLOAT4 directionalLightColor = XMFLOAT4(0.75f, 0.75f, 0.94f, 1.0f);
	XMFLOAT4 pointLightPos = XMFLOAT4(-8.0f, 2.0f, 3.0f, 0.0f);
	XMFLOAT4 pointLightColor = XMFLOAT4(2.0f, 2.0f, 0.0f, 1.0f);
	XMFLOAT4 pointLightRadius = XMFLOAT4(3.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 timer = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 spotLightPos = XMFLOAT4(8.0f, 2.0f, -5.0f, 0.0f);
	XMFLOAT4 spotLightColor = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);
	XMFLOAT4 spotLightDir = XMFLOAT4(0.0f, -0.5f, 0.35f, 1.0f);
	XMFLOAT4 spotLightOuterConeRatio = XMFLOAT4(0.4f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 spotLightInnerConeRatio = XMFLOAT4(0.5f, 0.0f, 0.0f, 0.0f);
};
ConstantBuffer myCBuff;
XMMATRIX view = XMMatrixLookAtLH({ 1, 5, -10 }, { 0, 0, 0 }, { 0, 1, 0 });
bool toReduceRadius = false;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void LoadMesh(const char* meshFileName, SimpleMesh& mesh);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
#pragma region Win32
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_D3D11BASE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_D3D11BASE));

	MSG msg;
#pragma endregion

	// Main message loop:
	while (true)//(GetMessage(&msg, nullptr, 0, 0)) waits for message (not good)
	{
		PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
			break;

#pragma region LightsAndTime
		static float rotate = 0; rotate += 0.0025f;
		static float timep = 0; timep += 0.0025f;
		XMVECTOR time = XMVectorSet(timep, 0.0f, 0.0f, 0.0f);
		XMStoreFloat4(&myCBuff.timer, time);
		if (myCBuff.pointLightRadius.x > 15.0f)
			toReduceRadius = true;
		else if (myCBuff.pointLightRadius.x < 3.0f)
			toReduceRadius = false;
		if (toReduceRadius)
			myCBuff.pointLightRadius.x -= 0.01f;
		else if (!toReduceRadius)
			myCBuff.pointLightRadius.x += 0.01f;
#pragma endregion

#pragma region ClearRenderTargetAndDepthBuffer
		// rendering here
		float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		myCon->ClearRenderTargetView(myRtv, color);

		myCon->ClearDepthStencilView(zBufferView, D3D11_CLEAR_DEPTH, 1, 0);
#pragma endregion

#pragma region ViewAndProjectionMatrices
		//view
		if (GetAsyncKeyState('W'))
			view = XMMatrixMultiply(view, XMMatrixTranslation(0, 0, -0.1f));
		if (GetAsyncKeyState('S'))
			view = XMMatrixMultiply(view, XMMatrixTranslation(0, 0, 0.1f));
		if (GetAsyncKeyState('A'))
			view = XMMatrixMultiply(view, XMMatrixTranslation(0.1f, 0, 0));
		if (GetAsyncKeyState('D'))
			view = XMMatrixMultiply(view, XMMatrixTranslation(-0.1f, 0, 0));
		if (GetAsyncKeyState(' '))
			view = XMMatrixMultiply(XMMatrixTranslation(0, -0.1f, 0), view);
		if (GetAsyncKeyState(VK_SHIFT))
			view = XMMatrixMultiply(XMMatrixTranslation(0, 0.1f, 0), view);
		XMStoreFloat4x4(&myCBuff.vMatrix, view);
		//projection
		XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(3.14f / 2.0f, aspectRatio, 0.1f, 1000);
		XMStoreFloat4x4(&myCBuff.pMatrix, projectionMatrix);
#pragma endregion

		// Setup the pipeline
		// output merger
		ID3D11RenderTargetView* tempRTV[] = { myRtv };
		myCon->OMSetRenderTargets(1, tempRTV, zBufferView);
		// rasterizer
		myCon->RSSetViewports(1, &myPort);

#pragma region SetupForRenderingProceduralMesh
		// Input Assembler
		myCon->IASetInputLayout(vLayout);
		UINT strides[] = { sizeof(BaseMesh) };
		UINT offsets[] = { 0 };
		ID3D11Buffer* tempVB[] = { vBuff };
		myCon->IASetVertexBuffers(0, 1, tempVB, strides, offsets);
		myCon->IASetIndexBuffer(iBuff, DXGI_FORMAT_R32_UINT, 0);
		myCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Vertex Shader Stage
		myCon->VSSetShader(vShader, 0, 0);
		// Pixel Shader Stage
		myCon->PSSetShader(pShader, 0, 0);

		// make a world matrix for each object
		XMMATRIX worldMatrix = XMMatrixIdentity();
		worldMatrix = XMMatrixTranslation(0, 5, 10);
		XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

		// Upload those matrices to the video card
		// Create and update a constant buffer (move variable from C++ to shaders)
		D3D11_MAPPED_SUBRESOURCE gpuBuffer;
		HRESULT hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
		//memcpy(gpuBuffer.pData, &MyMatrices, sizeof(WVP));
		myCon->Unmap(cBuff, 0);
#pragma endregion

		// Apply matrix math in Vertex Shader _ check
		// connect constant buffer to pipeline _ check
		// remember by default HLSL matrices are COLUMN MAJOR
		ID3D11Buffer* constants[] = { cBuff };
		myCon->VSSetConstantBuffers(0, 1, constants);
		myCon->PSSetConstantBuffers(0, 1, constants);

		// Draw?
		myCon->DrawIndexed(procedure.indexList.size(), 0, 0);


		// immediate context
		ID3D11ShaderResourceView* texViews[] = { textureRV };
		myCon->PSSetShaderResources(0, 1, texViews);
		// get a more complex pre-made mesh (FBX, OBJ, custom header) _ check
		// load it onto the card (vertex buffer, index buffer) _ check
		// make sure our shaders can process it _ check?
		// place it somewhere else in the environment ???

#pragma region SetupForRenderingStoneHengeMesh
		// set pipeline
		myCon->IASetInputLayout(vMeshLayout);
		UINT mesh_strides[] = { sizeof(_OBJ_VERT_) };
		UINT mesh_offsets[] = { 0 };
		ID3D11Buffer* meshVB[] = { vBuffMesh };
		myCon->IASetVertexBuffers(0, 1, meshVB, mesh_strides, mesh_offsets);
		myCon->IASetIndexBuffer(iBuffMesh, DXGI_FORMAT_R32_UINT, 0);

		myCon->VSSetShader(vMeshShader, 0, 0);
		myCon->PSSetShader(pMeshShader, 0, 0);

		// modify world matrix before drawing next thing
		worldMatrix = XMMatrixIdentity();
		XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

		// send it ot the CARD
		hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
		myCon->Unmap(cBuff, 0);

		// draw it
		myCon->DrawIndexed(2532, 0, 0);
#pragma endregion

#pragma region SetupForRenderingD20
		ID3D11ShaderResourceView* d20Views[] = { d20textureRV };
		myCon->PSSetShaderResources(0, 1, d20Views);

		myCon->IASetInputLayout(d20MeshLayout);
		UINT d20_strides[] = { sizeof(SimpleMesh) };
		UINT d20_offsets[] = { 0 };
		ID3D11Buffer* d20VB[] = { d20vBuff };
		myCon->IASetVertexBuffers(0, 1, d20VB, d20_strides, d20_offsets);
		myCon->IASetIndexBuffer(d20iBuff, DXGI_FORMAT_R32_UINT, 0);

		myCon->VSSetShader(d20vShader, 0, 0);
		myCon->PSSetShader(d20pShader, 0, 0);

		worldMatrix = XMMatrixTranslation(-5.0f, 2.0f, 4.0f);
		XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

		// send it ot the CARD
		hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
		myCon->Unmap(cBuff, 0);

		// draw it
		myCon->DrawIndexed(d20Mesh.indicesList.size(), 0, 0);
#pragma endregion

		mySwap->Present(0, 0);
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_D3D11BASE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_D3D11BASE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
#pragma region Win32
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	RECT myWinR;
	GetClientRect(hWnd, &myWinR);
#pragma endregion

#pragma region DeviceSwapChainAndViewport
	// attach d3d11 to our window
	D3D_FEATURE_LEVEL dx11 = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swap;
	ZeroMemory(&swap, sizeof(DXGI_SWAP_CHAIN_DESC));
	swap.BufferCount = 1;
	swap.OutputWindow = hWnd;
	swap.Windowed = true;
	swap.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap.BufferDesc.Width = myWinR.right - myWinR.left;
	swap.BufferDesc.Height = myWinR.bottom - myWinR.top;
	swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap.SampleDesc.Count = 1;

	aspectRatio = swap.BufferDesc.Width / float(swap.BufferDesc.Height);

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &dx11, 1, D3D11_SDK_VERSION, &swap, &mySwap, &myDev, NULL, &myCon);

	ID3D11Resource* backbuffer;
	hr = mySwap->GetBuffer(0, __uuidof(backbuffer), (void**)&backbuffer);
	hr = myDev->CreateRenderTargetView(backbuffer, NULL, &myRtv);

	backbuffer->Release();

	myPort.Width = swap.BufferDesc.Width;
	myPort.Height = swap.BufferDesc.Height;
	myPort.TopLeftX = myPort.TopLeftY = 0;
	myPort.MinDepth = 0;
	myPort.MaxDepth = 1;
#pragma endregion

#pragma region ProceduralMesh
	int gridHeight = 10;
	int gridWidth = 20;
	for (int i = 0; i < gridHeight; ++i)
	{
		for (int j = 0; j < gridWidth; ++j)
		{
			procedure.vertList.push_back({ {j * 0.5f, i * 0.5f, 0.0f, 1.0f}, {j / static_cast<float>(gridWidth), i / static_cast<float>(gridHeight), 0.5f, 1.0f} });
			if (i < gridHeight - 1 && j < gridWidth - 1)
			{
				int currentIndex = (i * gridWidth) + j;
				int upperIndex = currentIndex + gridWidth;
				procedure.indexList.push_back(currentIndex);
				procedure.indexList.push_back(upperIndex);
				procedure.indexList.push_back(currentIndex + 1);

				procedure.indexList.push_back(upperIndex);
				procedure.indexList.push_back(upperIndex + 1);
				procedure.indexList.push_back(currentIndex + 1);
			}
		}
	}

	// load it on the card
	D3D11_BUFFER_DESC bDesc;
	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&bDesc, sizeof(bDesc));
	ZeroMemory(&subData, sizeof(subData));

	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(BaseMesh) * (procedure.vertList.size());
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_IMMUTABLE;

	subData.pSysMem = procedure.vertList.data();
	subData.SysMemPitch = 0;
	subData.SysMemSlicePitch = 0;

	hr = myDev->CreateBuffer(&bDesc, &subData, &vBuff);
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * (procedure.indexList.size());
	subData.pSysMem = procedure.indexList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &iBuff);

	// write and compile & load our shaders

	hr = myDev->CreateVertexShader(MyVShader, sizeof(MyVShader), nullptr, &vShader);
	hr = myDev->CreatePixelShader(MyPShader, sizeof(MyPShader), nullptr, &pShader);

	// describe the vertex to D3D11
	D3D11_INPUT_ELEMENT_DESC ieDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDev->CreateInputLayout(ieDesc, 2, MyVShader, sizeof(MyVShader), &vLayout);
#pragma endregion

#pragma region ConstantBuffer
	// create constant buffer
	ZeroMemory(&bDesc, sizeof(bDesc));

	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bDesc.ByteWidth = sizeof(ConstantBuffer);
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DYNAMIC;

	hr = myDev->CreateBuffer(&bDesc, nullptr, &cBuff);
#pragma endregion

#pragma region StoneHengeMesh
	// load our complex mesh onto the card
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(StoneHenge_data);
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_IMMUTABLE;

	subData.pSysMem = StoneHenge_data;

	hr = myDev->CreateBuffer(&bDesc, &subData, &vBuffMesh); // vertex buffer
	// index buffer mesh
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(StoneHenge_indicies);
	subData.pSysMem = StoneHenge_indicies;
	hr = myDev->CreateBuffer(&bDesc, &subData, &iBuffMesh);

	// load our new mesh shader
	hr = myDev->CreateVertexShader(MyMeshVShader, sizeof(MyMeshVShader), nullptr, &vMeshShader);
	hr = myDev->CreatePixelShader(MyMeshPShader, sizeof(MyMeshPShader), nullptr, &pMeshShader);

	// make matching input layout for mesh vertex
	D3D11_INPUT_ELEMENT_DESC meshInputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDev->CreateInputLayout(meshInputDesc, 3, MyMeshVShader, sizeof(MyMeshVShader), &vMeshLayout);

	hr = CreateDDSTextureFromFile(myDev, L"Assets/StoneHenge.dds", (ID3D11Resource**)&mTexture, &textureRV);
#pragma endregion

#pragma region d20Mesh
	LoadMesh("Assets/d20.mesh", d20Mesh);
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(SimpleMesh) * (d20Mesh.verticesList.size());
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	subData.pSysMem = d20Mesh.verticesList.data();

	hr = myDev->CreateBuffer(&bDesc, &subData, &d20vBuff);
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * (d20Mesh.indicesList.size());
	subData.pSysMem = d20Mesh.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &d20iBuff);

	// load our new mesh shader
	hr = myDev->CreateVertexShader(MyLoadVShader, sizeof(MyLoadVShader), nullptr, &d20vShader);
	hr = myDev->CreatePixelShader(MyLoadPShader, sizeof(MyLoadPShader), nullptr, &d20pShader);

	// make matching input layout for mesh vertex
	D3D11_INPUT_ELEMENT_DESC d20InputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDev->CreateInputLayout(d20InputDesc, 3, MyLoadVShader, sizeof(MyLoadVShader), &d20MeshLayout);

	hr = CreateDDSTextureFromFile(myDev, L"Assets/d20texture.dds", (ID3D11Resource**)&d20Texture, &d20textureRV);
#pragma endregion

#pragma region DepthBuffer(ZBuffer)
	// create Z buffer & view
	D3D11_TEXTURE2D_DESC zDesc;
	ZeroMemory(&zDesc, sizeof(zDesc));
	zDesc.ArraySize = 1;
	zDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	zDesc.Width = swap.BufferDesc.Width;
	zDesc.Height = swap.BufferDesc.Height;
	zDesc.Format = DXGI_FORMAT_D32_FLOAT;
	zDesc.Usage = D3D11_USAGE_DEFAULT;
	zDesc.MipLevels = 1;
	zDesc.SampleDesc.Count = 1;

	hr = myDev->CreateTexture2D(&zDesc, nullptr, &zBuffer);

	hr = myDev->CreateDepthStencilView(zBuffer, nullptr, &zBufferView);
#pragma endregion

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	//case WM_PAINT:
	//{
	//	PAINTSTRUCT ps;
	//	HDC hdc = BeginPaint(hWnd, &ps);
	//	// TODO: Add any drawing code that uses hdc here...
	//	EndPaint(hWnd, &ps);
	//}
	//break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void LoadMesh(const char* meshFileName, SimpleMesh& mesh)
{
	fstream file{ meshFileName, ios_base::in | ios_base::binary };

	assert(file.is_open());

	uint32_t player_index_count;
	file.read((char*)&player_index_count, sizeof(uint32_t));

	mesh.indicesList.resize(player_index_count);

	file.read((char*)mesh.indicesList.data(), sizeof(uint32_t) * player_index_count);

	uint32_t player_vertex_count;
	file.read((char*)&player_vertex_count, sizeof(uint32_t));

	mesh.verticesList.resize(player_vertex_count);

	file.read((char*)mesh.verticesList.data(), sizeof(SimpleVertex) * player_vertex_count);

	file.close();
}