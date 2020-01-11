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

// pre-made mesh
#include "Assets/StoneHenge.h"

// for init
ID3D11Device* myDev;
IDXGISwapChain* mySwap;
ID3D11DeviceContext* myCon;

// for drawing
ID3D11RenderTargetView* myRtv;
D3D11_VIEWPORT myPort;
float aspectRatio = 1;

struct MyVertex
{
	float xyzw[4];
	float rgba[4];
};
unsigned int numVerts = 0;

ID3D11Buffer* vBuff;
ID3D11InputLayout* vLayout;
ID3D11VertexShader* vShader; // HLSL
ID3D11PixelShader* pShader; // HLSL

ID3D11Buffer* cBuff; // shader vars

// mesh data
ID3D11Buffer* vBuffMesh; // vertex buffer
ID3D11Buffer* iBuffMesh; // index buffer

// mesh vertex shader
ID3D11VertexShader* vMeshShader; // HLSL
ID3D11InputLayout* vMeshLayout;
ID3D11PixelShader* pMeshShader; // HLSL

// X buffer for depth sorting
ID3D11Texture2D* zBuffer;
ID3D11DepthStencilView* zBufferView;

// texture varialbles
ID3D11Texture2D* mTexture;
ID3D11ShaderResourceView* textureRV;

// Math stuff
struct ConstantBuffer
{
	XMFLOAT4X4 wMatrix; // storage type
	XMFLOAT4X4 vMatrix;
	XMFLOAT4X4 pMatrix;
	XMFLOAT4 directionalLightPos;
	XMFLOAT4 directionalLightColor;
	XMFLOAT4 pointLightPos;
	XMFLOAT4 pointLightColor;
	XMFLOAT4 pointLightRadius = XMFLOAT4(1500.0f, 0.0f, 0.0f, 0.0f);
};
ConstantBuffer myCBuff;
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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
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

		// rendering here
		float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		myCBuff.directionalLightPos = XMFLOAT4(0.577f, 0.577f, -0.577f, 1.0f);
		myCBuff.directionalLightColor = XMFLOAT4(0.75f, 0.75f, 0.94f, 1.0f);
		myCBuff.pointLightPos = XMFLOAT4(-1.0f, 0.5f, -1.0f, 1.0f);
		myCBuff.pointLightColor = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
		if (myCBuff.pointLightRadius.x > 5000.0f)
			toReduceRadius = true;
		else if (myCBuff.pointLightRadius.x < 10.0f)
			toReduceRadius = false;
		if (toReduceRadius)
			myCBuff.pointLightRadius.x -= 5.0f;
		else if (!toReduceRadius)
			myCBuff.pointLightRadius.x += 5.0f;
		myCon->ClearRenderTargetView(myRtv, color);

		myCon->ClearDepthStencilView(zBufferView, D3D11_CLEAR_DEPTH, 1, 0);

		// Setup the pipeline

		// output merger
		ID3D11RenderTargetView* tempRTV[] = { myRtv };
		myCon->OMSetRenderTargets(1, tempRTV, zBufferView);
		// rasterizer
		myCon->RSSetViewports(1, &myPort);
		// Input Assembler
		myCon->IASetInputLayout(vLayout);
		UINT strides[] = { sizeof(MyVertex) };
		UINT offsets[] = { 0 };
		ID3D11Buffer* tempVB[] = { vBuff };
		myCon->IASetVertexBuffers(0, 1, tempVB, strides, offsets);
		myCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// Vertex Shader Stage
		myCon->VSSetShader(vShader, 0, 0);
		// Pixel Shader Stage
		myCon->PSSetShader(pShader, 0, 0);

		// Try and Make your triangle 3D

		// make into a pyramid (more verts)

		// make a world, view & projection matrix
		static float rot = 0; rot += 0.01f;
		XMMATRIX temp = XMMatrixIdentity();
		temp = XMMatrixTranslation(3, 2, -5);
		XMMATRIX temp2 = XMMatrixRotationY(rot);
		temp = XMMatrixMultiply(temp2, temp);
		XMStoreFloat4x4(&myCBuff.wMatrix, temp);
		// view
		temp = XMMatrixLookAtLH({ 1, 5, -10 }, { 0, 0, 0 }, { 0, 1, 0 });
		XMStoreFloat4x4(&myCBuff.vMatrix, temp);
		// projection
		temp = XMMatrixPerspectiveFovLH(3.14f / 2.0f, aspectRatio, 0.1f, 1000);
		XMStoreFloat4x4(&myCBuff.pMatrix, temp);

		// Upload those matrices to the video card
		// Create and update a constant buffer (move variable from C++ to shaders)
		D3D11_MAPPED_SUBRESOURCE gpuBuffer;
		HRESULT hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
		//memcpy(gpuBuffer.pData, &MyMatrices, sizeof(WVP));
		myCon->Unmap(cBuff, 0);


		// Apply matrix math in Vertex Shader _ check
		// connect constant buffer to pipeline _ check
		// remember by default HLSL matrices are COLUMN MAJOR
		ID3D11Buffer* constants[] = { cBuff };
		myCon->VSSetConstantBuffers(0, 1, constants);
		myCon->PSSetConstantBuffers(0, 1, constants);

		// Draw?
		myCon->Draw(numVerts, 0);

		// immediate context
		ID3D11ShaderResourceView* texViews[] { textureRV };
		myCon->PSSetShaderResources(0, 1, texViews);
		// get a more complex pre-made mesh (FBX, OBJ, custom header) _ check
		// load it onto the card (vertex buffer, index buffer) _ check
		// make sure our shaders can process it _ check?
		// place it somewhere else in the environment ???

		// set pipeline
		UINT mesh_strides[] = { sizeof(_OBJ_VERT_) };
		UINT mesh_offsets[] = { 0 };
		ID3D11Buffer* meshVB[] = { vBuffMesh };
		myCon->IASetVertexBuffers(0, 1, meshVB, mesh_strides, mesh_offsets);
		myCon->IASetIndexBuffer(iBuffMesh, DXGI_FORMAT_R32_UINT, 0);
		myCon->VSSetShader(vMeshShader, 0, 0);
		myCon->PSSetShader(pMeshShader, 0, 0);
		myCon->IASetInputLayout(vMeshLayout);

		// modify world matrix before drawing next thing
		temp = XMMatrixIdentity();
		XMStoreFloat4x4(&myCBuff.wMatrix, temp);
		// send it ot the CARD
		hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
		myCon->Unmap(cBuff, 0);

		// draw it
		myCon->DrawIndexed(2532, 0, 0);


		mySwap->Present(0, 0);
	}

	// release all out D3D11 interfaces
	myRtv->Release();
	vBuff->Release();
	myCon->Release();
	mySwap->Release();
	vShader->Release();
	pShader->Release();
	vLayout->Release();
	myDev->Release();
	cBuff->Release();
	iBuffMesh->Release();
	vBuffMesh->Release();
	vMeshShader->Release();
	vMeshLayout->Release();
	zBuffer->Release();
	zBufferView->Release();
	pMeshShader->Release();
	textureRV->Release();
	mTexture->Release();


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

	MyVertex tri[] = // NDC Normalized Device Coordinates
	{ // xyzw, rgba
		// front
		{ {   0.0f,   1.0f,   0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
		{ {  0.25f, -0.25f, -0.25f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f} },
		{ { -0.25f, -0.25f, -0.25f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f} },
		// right
		{ {   0.0f,   1.0f,   0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
		{ {  0.25f, -0.25f,  0.25f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f} },
		{ {  0.25f, -0.25f, -0.25f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f} },
		// back
		{ {   0.0f,   1.0f,  0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
		{ { -0.25f, -0.25f, 0.25f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f} },
		{ {  0.25f, -0.25f, 0.25f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f} },
		// left
		{ {   0.0f,   1.0f,   0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} },
		{ { -0.25f, -0.25f, -0.25f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f} },
		{ { -0.25f, -0.25f,  0.25f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f} }
	};
	numVerts = ARRAYSIZE(tri);

	// load it on the card
	D3D11_BUFFER_DESC bDesc;
	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&bDesc, sizeof(bDesc));
	ZeroMemory(&subData, sizeof(subData));

	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(MyVertex) * ARRAYSIZE(tri);
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_IMMUTABLE;

	subData.pSysMem = tri;

	hr = myDev->CreateBuffer(&bDesc, &subData, &vBuff);

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

	// create constant buffer
	ZeroMemory(&bDesc, sizeof(bDesc));

	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bDesc.ByteWidth = sizeof(ConstantBuffer);
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DYNAMIC;

	hr = myDev->CreateBuffer(&bDesc, nullptr, &cBuff);

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
