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

#include "MySkyboxVShader.csh"
#include "MySkyboxPShader.csh"

#include "MySunPShader.csh"

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
SimpleMesh crystalMesh;
SimpleMesh skyboxMesh;
SimpleMesh skybox2Mesh;
SimpleMesh spaceShipMesh;
SimpleMesh sunMesh;
SimpleMesh earthMesh;
SimpleMesh jupiterMesh;
SimpleMesh moonMesh;

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

#pragma region CrystalMesh
CComPtr<ID3D11InputLayout> crystalMeshLayout;
CComPtr<ID3D11Buffer> crystalvBuff;
CComPtr<ID3D11Buffer> crystaliBuff;
CComPtr<ID3D11Texture2D> crystalTexture;
CComPtr<ID3D11ShaderResourceView> crystaltextureRV;
CComPtr<ID3D11VertexShader> crystalvShader;
CComPtr<ID3D11PixelShader> crystalpShader;
#pragma endregion

#pragma region SpaceShipMesh
CComPtr<ID3D11InputLayout> spaceShipMeshLayout;
CComPtr<ID3D11Buffer> spaceShipvBuff;
CComPtr<ID3D11Buffer> spaceShipiBuff;
CComPtr<ID3D11Texture2D> spaceShipTexture;
CComPtr<ID3D11ShaderResourceView> spaceShiptextureRV;
CComPtr<ID3D11VertexShader> spaceShipvShader;
CComPtr<ID3D11PixelShader> spaceShippShader;
#pragma endregion

#pragma region SunMesh
CComPtr<ID3D11InputLayout> sunMeshLayout;
CComPtr<ID3D11Buffer> sunvBuff;
CComPtr<ID3D11Buffer> suniBuff;
CComPtr<ID3D11Texture2D> sunTexture;
CComPtr<ID3D11ShaderResourceView> suntextureRV;
CComPtr<ID3D11VertexShader> sunvShader;
CComPtr<ID3D11PixelShader> sunpShader;
#pragma endregion

#pragma region EarthMesh
CComPtr<ID3D11InputLayout> earthMeshLayout;
CComPtr<ID3D11Buffer> earthvBuff;
CComPtr<ID3D11Buffer> earthiBuff;
CComPtr<ID3D11Texture2D> earthTexture;
CComPtr<ID3D11ShaderResourceView> earthtextureRV;
CComPtr<ID3D11VertexShader> earthvShader;
CComPtr<ID3D11PixelShader> earthpShader;
#pragma endregion

#pragma region JupiterMesh
CComPtr<ID3D11InputLayout> jupiterMeshLayout;
CComPtr<ID3D11Buffer> jupitervBuff;
CComPtr<ID3D11Buffer> jupiteriBuff;
CComPtr<ID3D11Texture2D> jupiterTexture;
CComPtr<ID3D11ShaderResourceView> jupitertextureRV;
CComPtr<ID3D11VertexShader> jupitervShader;
CComPtr<ID3D11PixelShader> jupiterpShader;
#pragma endregion

#pragma region MoonMesh
CComPtr<ID3D11InputLayout> moonMeshLayout;
CComPtr<ID3D11Buffer> moonvBuff;
CComPtr<ID3D11Buffer> mooniBuff;
CComPtr<ID3D11Texture2D> moonTexture;
CComPtr<ID3D11ShaderResourceView> moontextureRV;
CComPtr<ID3D11VertexShader> moonvShader;
CComPtr<ID3D11PixelShader> moonpShader;
#pragma endregion

#pragma region Skybox
CComPtr<ID3D11InputLayout> skyboxMeshLayout;
CComPtr<ID3D11Buffer> skyboxvBuff;
CComPtr<ID3D11Buffer> skyboxiBuff;
CComPtr<ID3D11Texture2D> skyboxTexture;
CComPtr<ID3D11ShaderResourceView> skyboxtextureRV;
CComPtr<ID3D11VertexShader> skyboxvShader;
CComPtr<ID3D11PixelShader> skyboxpShader;
CComPtr<ID3D11DepthStencilState> skyState;
#pragma endregion

#pragma region Skybox2
CComPtr<ID3D11InputLayout> skybox2MeshLayout;
CComPtr<ID3D11Buffer> skybox2vBuff;
CComPtr<ID3D11Buffer> skybox2iBuff;
CComPtr<ID3D11Texture2D> skybox2Texture;
CComPtr<ID3D11ShaderResourceView> skybox2textureRV;
CComPtr<ID3D11VertexShader> skybox2vShader;
CComPtr<ID3D11PixelShader> skybox2pShader;
CComPtr<ID3D11DepthStencilState> sky2State;
#pragma endregion

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
	XMFLOAT4 pointLightPos = XMFLOAT4(-8.0f, 2.0f, 0.0f, 0.0f);
	XMFLOAT4 pointLightColor = XMFLOAT4(2.0f, 2.0f, 0.0f, 1.0f);
	XMFLOAT4 pointLightRadius = XMFLOAT4(9.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 timer = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 spotLightPos = XMFLOAT4(8.0f, 2.0f, 0.0f, 0.0f);
	XMFLOAT4 spotLightColor = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);
	XMFLOAT4 spotLightDir = XMFLOAT4(0.0f, -0.5f, 0.0f, 1.0f);
	XMFLOAT4 spotLightOuterConeRatio = XMFLOAT4(0.4f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 spotLightInnerConeRatio = XMFLOAT4(0.5f, 0.0f, 0.0f, 0.0f);
};
ConstantBuffer myCBuff;
XMMATRIX view;
bool toReduceRadius = false;
float nearPlane = 0.1f;
float farPlane = 300.0f;
float FOV = 2.0f;

bool toSwitchScenes = false;

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
HWND window;

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

		RECT rect;
		GetClientRect(window, &rect);
		aspectRatio = (rect.right - rect.left) / float(rect.bottom - rect.top);

#pragma region ClearRenderTargetAndDepthBuffer
		// rendering here
		float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		myCon->ClearRenderTargetView(myRtv, color);

		myCon->ClearDepthStencilView(zBufferView, D3D11_CLEAR_DEPTH, 1, 0);
#pragma endregion

#pragma region ViewAndProjectionMatrices
		//view
		float translationValue = 0.0f;
		if (!toSwitchScenes)
			translationValue = 0.1f;
		if (toSwitchScenes)
			translationValue = 0.5f;
		float localX = 0.0f;
		float localZ = 0.0f;
		float globalY = 0.0f;
		float yRot = 0.0f;
		float xRot = 0.0f;
		if (GetAsyncKeyState(VK_LEFT))
			yRot -= 0.01f;
		if (GetAsyncKeyState(VK_RIGHT))
			yRot += 0.01f;
		if (GetAsyncKeyState(VK_DOWN))
			xRot += 0.01f;
		if (GetAsyncKeyState(VK_UP))
			xRot -= 0.01f;
		if (GetAsyncKeyState('W'))
			localZ += translationValue;
		if (GetAsyncKeyState('S'))
			localZ -= translationValue;
		if (GetAsyncKeyState('A'))
			localX -= translationValue;
		if (GetAsyncKeyState('D'))
			localX += translationValue;
		if (GetAsyncKeyState(VK_SPACE))
			globalY += translationValue;
		if (GetAsyncKeyState(VK_SHIFT))
			globalY -= translationValue;
		if (GetAsyncKeyState('U') && nearPlane < 50.0f)
			nearPlane += 0.1f;
		if (GetAsyncKeyState('J') && nearPlane > 0.15f)
			nearPlane -= 0.1f;
		if (GetAsyncKeyState('I') && farPlane < 1000.0f)
			farPlane += 1.0f;
		if (GetAsyncKeyState('K') && farPlane > 20.0f)
			farPlane -= 1.0f;
		if (GetAsyncKeyState('O') && FOV < 15.0f)
			FOV += 0.1f;
		if (GetAsyncKeyState('L') && FOV > 2.0f)
			FOV -= 0.1f;

		XMMATRIX translation = XMMatrixTranslation(localX, 0.0f, localZ);
		view = XMMatrixMultiply(translation, view);
		translation = XMMatrixTranslation(0.0f, globalY, 0.0f);
		view = XMMatrixMultiply(view, translation);

		XMVECTOR temp = view.r[3];
		view.r[3] = XMMatrixIdentity().r[3];
		translation = XMMatrixRotationY(yRot);
		view = XMMatrixMultiply(view, translation);
		view.r[3] = temp;

		translation = XMMatrixRotationX(xRot);
		view = XMMatrixMultiply(translation, view);

		XMMATRIX camera = DirectX::XMMatrixInverse(nullptr, view);
		XMStoreFloat4x4(&myCBuff.vMatrix, camera);
		//projection
		XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(3.14f / FOV, aspectRatio, nearPlane, farPlane);
		XMStoreFloat4x4(&myCBuff.pMatrix, projectionMatrix);
#pragma endregion

		// Setup the pipeline
		// output merger
		ID3D11RenderTargetView* tempRTV[] = { myRtv };
		myCon->OMSetRenderTargets(1, tempRTV, zBufferView);
		// rasterizer
		myCon->RSSetViewports(1, &myPort);

		if (GetAsyncKeyState('1'))
			toSwitchScenes = false;
		if (GetAsyncKeyState('2'))
			toSwitchScenes = true;

		if (!toSwitchScenes)
		{
#pragma region LightsAndTime
			myCBuff.directionalLightPos = XMFLOAT4(-0.577f, -0.577f, 0.577f, 1.0f);
			myCBuff.directionalLightColor = XMFLOAT4(0.75f, 0.75f, 0.94f, 1.0f);
			static float pointRadius = 9.0f;
			static float pointPosZ = 0.0f;
			static float spotPosZ = 0.0f;
			static float spotDirZ = 0.0f;
			myCBuff.pointLightPos = XMFLOAT4(-8.0f, 2.0f, pointPosZ, 0.0f);
			myCBuff.pointLightColor = XMFLOAT4(2.0f, 2.0f, 0.0f, 1.0f);
			myCBuff.pointLightRadius = XMFLOAT4(pointRadius, 0.0f, 0.0f, 0.0f);
			myCBuff.spotLightPos = XMFLOAT4(8.0f, 2.0f, spotPosZ, 0.0f);
			myCBuff.spotLightColor = XMFLOAT4(1.5f, 1.5f, 1.5f, 1.0f);
			myCBuff.spotLightDir = XMFLOAT4(0.0f, -0.5f, spotDirZ, 1.0f);
			static float rotate = 0; rotate += 0.0025f;
			static float timep = 0; timep += 0.0025f;
			XMVECTOR time = XMVectorSet(timep, 0.0f, 0.0f, 0.0f);
			XMStoreFloat4(&myCBuff.timer, time);
			if (pointRadius > 15.0f)
				toReduceRadius = true;
			else if (pointRadius < 3.0f)
				toReduceRadius = false;
			if (toReduceRadius)
			{
				pointRadius -= 0.01f;
				pointPosZ -= 0.01f;
				spotPosZ -= 0.025f;
				spotDirZ += 0.001f;
			}
			else if (!toReduceRadius)
			{
				pointRadius += 0.01f;
				pointPosZ += 0.01f;
				spotPosZ += 0.025f;
				spotDirZ -= 0.001f;
			}
#pragma endregion

#pragma region SetupForRenderingSkybox
			ID3D11ShaderResourceView* SkyboxViews[] = { skyboxtextureRV };
			myCon->PSSetShaderResources(0, 1, SkyboxViews);

			myCon->IASetInputLayout(skyboxMeshLayout);
			UINT skybox_strides[] = { sizeof(SimpleMesh) };
			UINT skybox_offsets[] = { 0 };
			ID3D11Buffer* skyboxVB[] = { skyboxvBuff };
			myCon->IASetVertexBuffers(0, 1, skyboxVB, skybox_strides, skybox_offsets);
			myCon->IASetIndexBuffer(skyboxiBuff, DXGI_FORMAT_R32_UINT, 0);
			myCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			myCon->VSSetShader(skyboxvShader, 0, 0);
			myCon->PSSetShader(skyboxpShader, 0, 0);

			XMMATRIX worldMatrix = XMMatrixTranslationFromVector(XMMatrixInverse(0, camera).r[3]);
			//worldMatrix = XMMatrixTranslation(0, 10, 0);
			XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

			// send it ot the CARD
			D3D11_MAPPED_SUBRESOURCE gpuBuffer;
			HRESULT	hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
			myCon->Unmap(cBuff, 0);

			ID3D11Buffer* constants[] = { cBuff };
			myCon->VSSetConstantBuffers(0, 1, constants);
			myCon->PSSetConstantBuffers(0, 1, constants);

			// draw it
			myCon->DrawIndexed(skyboxMesh.indicesList.size(), 0, 0);

			myCon->ClearDepthStencilView(zBufferView, D3D11_CLEAR_DEPTH, 1, 0);
#pragma endregion

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
			worldMatrix = XMMatrixIdentity();
			worldMatrix = XMMatrixTranslation(0, 5, 20);
			XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

			// Upload those matrices to the video card
			// Create and update a constant buffer (move variable from C++ to shaders)
			hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
			//memcpy(gpuBuffer.pData, &MyMatrices, sizeof(WVP));
			myCon->Unmap(cBuff, 0);
#pragma endregion

			// Apply matrix math in Vertex Shader _ check
			// connect constant buffer to pipeline _ check
			// remember by default HLSL matrices are COLUMN MAJOR

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

#pragma region SetupForRenderingCrystal
			ID3D11ShaderResourceView* crystalViews[] = { crystaltextureRV };
			myCon->PSSetShaderResources(0, 1, crystalViews);

			myCon->IASetInputLayout(crystalMeshLayout);
			UINT crystal_strides[] = { sizeof(SimpleMesh) };
			UINT crystal_offsets[] = { 0 };
			ID3D11Buffer* crystalVB[] = { crystalvBuff };
			myCon->IASetVertexBuffers(0, 1, crystalVB, crystal_strides, crystal_offsets);
			myCon->IASetIndexBuffer(crystaliBuff, DXGI_FORMAT_R32_UINT, 0);

			myCon->VSSetShader(crystalvShader, 0, 0);
			myCon->PSSetShader(crystalpShader, 0, 0);

			worldMatrix = XMMatrixTranslation(-15.0f, 3.0f, 4.0f);
			XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

			// send it ot the CARD
			hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
			myCon->Unmap(cBuff, 0);

			// draw it
			myCon->DrawIndexedInstanced(crystalMesh.indicesList.size(), 5, 0, 0, 0);
#pragma endregion
		}

		if (toSwitchScenes)
		{
#pragma region LightsAndTime
			myCBuff.directionalLightPos = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			myCBuff.directionalLightColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			myCBuff.pointLightPos = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			myCBuff.pointLightColor = XMFLOAT4(2.0f, 2.0f, 2.0f, 1.0f);
			myCBuff.pointLightRadius = XMFLOAT4(1000.0f, 0.0f, 0.0f, 0.0f);
			myCBuff.spotLightPos = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			myCBuff.spotLightColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			myCBuff.spotLightDir = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			static float timep = 0; timep += 0.0025f;
			XMVECTOR time = XMVectorSet(timep, 0.0f, 0.0f, 0.0f);
			XMStoreFloat4(&myCBuff.timer, time);
			if (myCBuff.pointLightRadius.x > 15.0f)
				toReduceRadius = true;
			else if (myCBuff.pointLightRadius.x < 3.0f)
				toReduceRadius = false;
#pragma endregion

#pragma region SetupForRenderingSkybox2
			ID3D11ShaderResourceView* Skybox2Views[] = { skybox2textureRV };
			myCon->PSSetShaderResources(0, 1, Skybox2Views);

			myCon->IASetInputLayout(skybox2MeshLayout);
			UINT skybox2_strides[] = { sizeof(SimpleMesh) };
			UINT skybox2_offsets[] = { 0 };
			ID3D11Buffer* skybox2VB[] = { skybox2vBuff };
			myCon->IASetVertexBuffers(0, 1, skybox2VB, skybox2_strides, skybox2_offsets);
			myCon->IASetIndexBuffer(skybox2iBuff, DXGI_FORMAT_R32_UINT, 0);
			myCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			myCon->VSSetShader(skybox2vShader, 0, 0);
			myCon->PSSetShader(skybox2pShader, 0, 0);

			XMMATRIX worldMatrix = XMMatrixTranslationFromVector(XMMatrixInverse(0, camera).r[3]);
			//worldMatrix = XMMatrixTranslation(0, 10, 0);
			XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

			// send it ot the CARD
			D3D11_MAPPED_SUBRESOURCE gpuBuffer;
			HRESULT	hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
			myCon->Unmap(cBuff, 0);

			ID3D11Buffer* constants[] = { cBuff };
			myCon->VSSetConstantBuffers(0, 1, constants);
			myCon->PSSetConstantBuffers(0, 1, constants);

			// draw it
			myCon->DrawIndexed(skybox2Mesh.indicesList.size(), 0, 0);

			myCon->ClearDepthStencilView(zBufferView, D3D11_CLEAR_DEPTH, 1, 0);
#pragma endregion

#pragma region SetupForRenderingSpaceShip
			static float shipRotate = 0; shipRotate -= 0.005f;
			ID3D11ShaderResourceView* spaceShipViews[] = { spaceShiptextureRV };
			myCon->PSSetShaderResources(0, 1, spaceShipViews);

			myCon->IASetInputLayout(spaceShipMeshLayout);
			UINT spaceShip_strides[] = { sizeof(SimpleMesh) };
			UINT spaceShip_offsets[] = { 0 };
			ID3D11Buffer* spaceShipVB[] = { spaceShipvBuff };
			myCon->IASetVertexBuffers(0, 1, spaceShipVB, spaceShip_strides, spaceShip_offsets);
			myCon->IASetIndexBuffer(spaceShipiBuff, DXGI_FORMAT_R32_UINT, 0);

			myCon->VSSetShader(spaceShipvShader, 0, 0);
			myCon->PSSetShader(spaceShippShader, 0, 0);

			worldMatrix = XMMatrixTranslation(70.0f, 0.0f, 0.0f);
			XMMATRIX tempRot = XMMatrixRotationY(shipRotate);
			worldMatrix = XMMatrixMultiply(worldMatrix, tempRot);
			XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

			// send it ot the CARD
			hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
			myCon->Unmap(cBuff, 0);

			// draw it
			myCon->DrawIndexed(spaceShipMesh.indicesList.size(), 0, 0);
#pragma endregion

#pragma region SetupForRenderingSun
			static float sunRotate = 0; sunRotate += 0.003f;
			ID3D11ShaderResourceView* sunViews[] = { suntextureRV };
			myCon->PSSetShaderResources(0, 1, sunViews);

			myCon->IASetInputLayout(sunMeshLayout);
			UINT sun_strides[] = { sizeof(SimpleMesh) };
			UINT sun_offsets[] = { 0 };
			ID3D11Buffer* sunVB[] = { sunvBuff };
			myCon->IASetVertexBuffers(0, 1, sunVB, sun_strides, sun_offsets);
			myCon->IASetIndexBuffer(suniBuff, DXGI_FORMAT_R32_UINT, 0);

			myCon->VSSetShader(sunvShader, 0, 0);
			myCon->PSSetShader(sunpShader, 0, 0);

			worldMatrix = XMMatrixIdentity();
			tempRot = XMMatrixRotationY(sunRotate);
			worldMatrix = XMMatrixMultiply(tempRot, worldMatrix);
			XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

			// send it ot the CARD
			hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
			myCon->Unmap(cBuff, 0);

			// draw it
			myCon->DrawIndexed(sunMesh.indicesList.size(), 0, 0);
#pragma endregion

#pragma region SetupForRenderingEarth
			static float earthOrbitRotate = 0; earthOrbitRotate += 0.001f;
			static float earthAxisRotate = 0; earthAxisRotate += 0.005f;
			ID3D11ShaderResourceView* earthViews[] = { earthtextureRV };
			myCon->PSSetShaderResources(0, 1, earthViews);

			myCon->IASetInputLayout(earthMeshLayout);
			UINT earth_strides[] = { sizeof(SimpleMesh) };
			UINT earth_offsets[] = { 0 };
			ID3D11Buffer* earthVB[] = { earthvBuff };
			myCon->IASetVertexBuffers(0, 1, earthVB, earth_strides, earth_offsets);
			myCon->IASetIndexBuffer(earthiBuff, DXGI_FORMAT_R32_UINT, 0);

			myCon->VSSetShader(earthvShader, 0, 0);
			myCon->PSSetShader(earthpShader, 0, 0);

			worldMatrix = XMMatrixTranslation(-120.0f, 0.0f, 0.0f);
			tempRot = XMMatrixRotationY(earthOrbitRotate);
			worldMatrix = XMMatrixMultiply(worldMatrix, tempRot);
			tempRot = XMMatrixRotationY(earthAxisRotate);
			worldMatrix = XMMatrixMultiply(tempRot, worldMatrix);
			XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

			// send it ot the CARD
			hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
			myCon->Unmap(cBuff, 0);

			// draw it
			myCon->DrawIndexed(earthMesh.indicesList.size(), 0, 0);
#pragma endregion

#pragma region SetupForRenderingJupiter
			static float jupiterOrbitRotate = 0; jupiterOrbitRotate += 0.0005f;
			static float jupiterAxisRotate = 0; jupiterAxisRotate += 0.0025f;
			ID3D11ShaderResourceView* jupiterViews[] = { jupitertextureRV };
			myCon->PSSetShaderResources(0, 1, jupiterViews);

			myCon->IASetInputLayout(jupiterMeshLayout);
			UINT jupiter_strides[] = { sizeof(SimpleMesh) };
			UINT jupiter_offsets[] = { 0 };
			ID3D11Buffer* jupiterVB[] = { jupitervBuff };
			myCon->IASetVertexBuffers(0, 1, jupiterVB, jupiter_strides, jupiter_offsets);
			myCon->IASetIndexBuffer(jupiteriBuff, DXGI_FORMAT_R32_UINT, 0);

			myCon->VSSetShader(jupitervShader, 0, 0);
			myCon->PSSetShader(jupiterpShader, 0, 0);

			worldMatrix = XMMatrixTranslation(200.0f, 0.0f, 0.0f);
			tempRot = XMMatrixRotationY(jupiterOrbitRotate);
			worldMatrix = XMMatrixMultiply(worldMatrix, tempRot);
			tempRot = XMMatrixRotationY(jupiterAxisRotate);
			worldMatrix = XMMatrixMultiply(tempRot, worldMatrix);
			XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

			// send it ot the CARD
			hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
			myCon->Unmap(cBuff, 0);

			// draw it
			myCon->DrawIndexed(jupiterMesh.indicesList.size(), 0, 0);
#pragma endregion

#pragma region SetupForRenderingMoon
			static float moonOrbitRotate = 0; moonOrbitRotate += 0.001f;
			static float moonAxisRotate = 0; moonAxisRotate += 0.005f;
			static float moonOrbit2Rotate = 0; moonOrbit2Rotate -= 0.01f;
			ID3D11ShaderResourceView* moonViews[] = { moontextureRV };
			myCon->PSSetShaderResources(0, 1, moonViews);

			myCon->IASetInputLayout(moonMeshLayout);
			UINT moon_strides[] = { sizeof(SimpleMesh) };
			UINT moon_offsets[] = { 0 };
			ID3D11Buffer* moonVB[] = { moonvBuff };
			myCon->IASetVertexBuffers(0, 1, moonVB, moon_strides, moon_offsets);
			myCon->IASetIndexBuffer(mooniBuff, DXGI_FORMAT_R32_UINT, 0);

			myCon->VSSetShader(moonvShader, 0, 0);
			myCon->PSSetShader(moonpShader, 0, 0);

			worldMatrix = XMMatrixTranslation(-120.0f, 0.0f, 0.0f);
			tempRot = XMMatrixRotationY(moonOrbitRotate);
			worldMatrix = XMMatrixMultiply(worldMatrix, tempRot);

			tempRot = XMMatrixRotationY(moonAxisRotate);
			worldMatrix = XMMatrixMultiply(tempRot, worldMatrix);

			XMMATRIX tempMat = XMMatrixTranslation(-30.0f, 0.0f, 0.0f);
			tempRot = XMMatrixRotationY(moonOrbit2Rotate);
			tempMat = XMMatrixMultiply(tempMat, tempRot);
			worldMatrix = XMMatrixMultiply(tempMat, worldMatrix);

			XMStoreFloat4x4(&myCBuff.wMatrix, worldMatrix);

			// send it ot the CARD
			hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((ConstantBuffer*)(gpuBuffer.pData)) = myCBuff;
			myCon->Unmap(cBuff, 0);

			// draw it
			myCon->DrawIndexed(moonMesh.indicesList.size(), 0, 0);
#pragma endregion
		}

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
	window = hWnd;
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
	view = XMMatrixTranslation(0, 10, -60);

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

#pragma region CrystalMesh
	LoadMesh("Assets/Crystal.mesh", crystalMesh);
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(SimpleMesh) * (crystalMesh.verticesList.size());
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	subData.pSysMem = crystalMesh.verticesList.data();

	hr = myDev->CreateBuffer(&bDesc, &subData, &crystalvBuff);
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * (crystalMesh.indicesList.size());
	subData.pSysMem = crystalMesh.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &crystaliBuff);

	// load our new mesh shader
	hr = myDev->CreateVertexShader(MyLoadVShader, sizeof(MyLoadVShader), nullptr, &crystalvShader);
	hr = myDev->CreatePixelShader(MyLoadPShader, sizeof(MyLoadPShader), nullptr, &crystalpShader);

	// make matching input layout for mesh vertex
	D3D11_INPUT_ELEMENT_DESC CrystalInputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDev->CreateInputLayout(CrystalInputDesc, 3, MyLoadVShader, sizeof(MyLoadVShader), &crystalMeshLayout);

	hr = CreateDDSTextureFromFile(myDev, L"Assets/icium.dds", (ID3D11Resource**)&crystalTexture, &crystaltextureRV);
#pragma endregion

#pragma region SkyBox
	skyboxMesh.verticesList = {
		{{ 1.0f, -1.0f,  1.0f } },
		{{ 1.0f,  1.0f,  1.0f } },
		{{ 1.0f,  1.0f, -1.0f } },

		{{ 1.0f, -1.0f, -1.0f } },
		{{-1.0f, -1.0f, -1.0f } },
		{{-1.0f,  1.0f, -1.0f } },

		{{-1.0f,  1.0f,  1.0f } },
		{{-1.0f, -1.0f,  1.0f } },
		{{-1.0f, -1.0f,  1.0f } },

		{{ 1.0f, -1.0f,  1.0f } },
		{{ 1.0f, -1.0f, -1.0f } },
		{{-1.0f, -1.0f, -1.0f } },

		{{ 1.0f,  1.0f, -1.0f } },
		{{ 1.0f,  1.0f,  1.0f } },
		{{-1.0f,  1.0f,  1.0f } },

		{{-1.0f,  1.0f, -1.0f } },
		{{-1.0f,  1.0f,  1.0f } },
		{{ 1.0f,  1.0f,  1.0f } },

		{{ 1.0f, -1.0f,  1.0f } },
		{{-1.0f, -1.0f,  1.0f } },
		{{ 1.0f, -1.0f, -1.0f } },

		{{ 1.0f,  1.0f, -1.0f } },
		{{-1.0f,  1.0f, -1.0f } },
		{{-1.0f, -1.0f, -1.0f } }

	};

	skyboxMesh.indicesList = {
		0,  1,  2,
		0,  2,  3,
		4,  5,  6,
		4,  6,  7,
		8,  9, 10,
		8, 10, 11,
		12, 13, 14,
		12, 14, 15,
		16, 17, 18,
		16, 18, 19,
		20, 21, 22,
		20, 22, 23
	};

	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(SimpleMesh) * (skyboxMesh.verticesList.size());
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	subData.pSysMem = skyboxMesh.verticesList.data();

	hr = myDev->CreateBuffer(&bDesc, &subData, &skyboxvBuff);
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * (skyboxMesh.indicesList.size());
	subData.pSysMem = skyboxMesh.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &skyboxiBuff);

	// load our new mesh shader
	hr = myDev->CreateVertexShader(MySkyboxVShader, sizeof(MySkyboxVShader), nullptr, &skyboxvShader);
	hr = myDev->CreatePixelShader(MySkyboxPShader, sizeof(MySkyboxPShader), nullptr, &skyboxpShader);

	// make matching input layout for mesh vertex
	D3D11_INPUT_ELEMENT_DESC skyboxInputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDev->CreateInputLayout(skyboxInputDesc, 3, MySkyboxVShader, sizeof(MySkyboxVShader), &skyboxMeshLayout);

	hr = CreateDDSTextureFromFile(myDev, L"Assets/SkyboxOcean.dds", (ID3D11Resource**)&skyboxTexture, &skyboxtextureRV);

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	myDev->CreateDepthStencilState(&dssDesc, &skyState);
#pragma endregion

#pragma region SkyBox2
	skybox2Mesh.verticesList = {
		{{ 1.0f, -1.0f,  1.0f } },
		{{ 1.0f,  1.0f,  1.0f } },
		{{ 1.0f,  1.0f, -1.0f } },

		{{ 1.0f, -1.0f, -1.0f } },
		{{-1.0f, -1.0f, -1.0f } },
		{{-1.0f,  1.0f, -1.0f } },

		{{-1.0f,  1.0f,  1.0f } },
		{{-1.0f, -1.0f,  1.0f } },
		{{-1.0f, -1.0f,  1.0f } },

		{{ 1.0f, -1.0f,  1.0f } },
		{{ 1.0f, -1.0f, -1.0f } },
		{{-1.0f, -1.0f, -1.0f } },

		{{ 1.0f,  1.0f, -1.0f } },
		{{ 1.0f,  1.0f,  1.0f } },
		{{-1.0f,  1.0f,  1.0f } },

		{{-1.0f,  1.0f, -1.0f } },
		{{-1.0f,  1.0f,  1.0f } },
		{{ 1.0f,  1.0f,  1.0f } },

		{{ 1.0f, -1.0f,  1.0f } },
		{{-1.0f, -1.0f,  1.0f } },
		{{ 1.0f, -1.0f, -1.0f } },

		{{ 1.0f,  1.0f, -1.0f } },
		{{-1.0f,  1.0f, -1.0f } },
		{{-1.0f, -1.0f, -1.0f } }

	};

	skybox2Mesh.indicesList = {
		0,  1,  2,
		0,  2,  3,
		4,  5,  6,
		4,  6,  7,
		8,  9, 10,
		8, 10, 11,
		12, 13, 14,
		12, 14, 15,
		16, 17, 18,
		16, 18, 19,
		20, 21, 22,
		20, 22, 23
	};

	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(SimpleMesh) * (skybox2Mesh.verticesList.size());
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	subData.pSysMem = skybox2Mesh.verticesList.data();

	hr = myDev->CreateBuffer(&bDesc, &subData, &skybox2vBuff);
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * (skybox2Mesh.indicesList.size());
	subData.pSysMem = skybox2Mesh.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &skybox2iBuff);

	// load our new mesh shader
	hr = myDev->CreateVertexShader(MySkyboxVShader, sizeof(MySkyboxVShader), nullptr, &skybox2vShader);
	hr = myDev->CreatePixelShader(MySkyboxPShader, sizeof(MySkyboxPShader), nullptr, &skybox2pShader);

	// make matching input layout for mesh vertex
	D3D11_INPUT_ELEMENT_DESC skybox2InputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDev->CreateInputLayout(skybox2InputDesc, 3, MySkyboxVShader, sizeof(MySkyboxVShader), &skybox2MeshLayout);

	hr = CreateDDSTextureFromFile(myDev, L"Assets/SpaceSkybox.dds", (ID3D11Resource**)&skybox2Texture, &skybox2textureRV);

	D3D11_DEPTH_STENCIL_DESC dss2Desc;
	ZeroMemory(&dss2Desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dss2Desc.DepthEnable = true;
	dss2Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dss2Desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	myDev->CreateDepthStencilState(&dss2Desc, &sky2State);
#pragma endregion

#pragma region SpaceShipMesh
	LoadMesh("Assets/spaceship.mesh", spaceShipMesh);
	for (int i = 0; i < spaceShipMesh.verticesList.size(); ++i)
	{
		spaceShipMesh.verticesList[i].Pos.x *= 3;
		spaceShipMesh.verticesList[i].Pos.y *= 3;
		spaceShipMesh.verticesList[i].Pos.z *= 3;
	}
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(SimpleMesh) * (spaceShipMesh.verticesList.size());
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	subData.pSysMem = spaceShipMesh.verticesList.data();

	hr = myDev->CreateBuffer(&bDesc, &subData, &spaceShipvBuff);
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * (spaceShipMesh.indicesList.size());
	subData.pSysMem = spaceShipMesh.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &spaceShipiBuff);

	// load our new mesh shader
	hr = myDev->CreateVertexShader(MyLoadVShader, sizeof(MyLoadVShader), nullptr, &spaceShipvShader);
	hr = myDev->CreatePixelShader(MyLoadPShader, sizeof(MyLoadPShader), nullptr, &spaceShippShader);

	// make matching input layout for mesh vertex
	D3D11_INPUT_ELEMENT_DESC SpaceShipInputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDev->CreateInputLayout(SpaceShipInputDesc, 3, MyLoadVShader, sizeof(MyLoadVShader), &spaceShipMeshLayout);

	hr = CreateDDSTextureFromFile(myDev, L"Assets/AlienSpeed_D.dds", (ID3D11Resource**)&spaceShipTexture, &spaceShiptextureRV);
#pragma endregion

#pragma region SunMesh
	LoadMesh("Assets/planet.mesh", sunMesh);
	for (int i = 0; i < sunMesh.verticesList.size(); ++i)
	{
		sunMesh.verticesList[i].Pos.x *= 50;
		sunMesh.verticesList[i].Pos.y *= 50;
		sunMesh.verticesList[i].Pos.z *= 50;
	}
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(SimpleMesh) * (sunMesh.verticesList.size());
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	subData.pSysMem = sunMesh.verticesList.data();

	hr = myDev->CreateBuffer(&bDesc, &subData, &sunvBuff);
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * (sunMesh.indicesList.size());
	subData.pSysMem = sunMesh.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &suniBuff);

	// load our new mesh shader
	hr = myDev->CreateVertexShader(MyLoadVShader, sizeof(MyLoadVShader), nullptr, &sunvShader);
	hr = myDev->CreatePixelShader(MySunPShader, sizeof(MySunPShader), nullptr, &sunpShader);

	// make matching input layout for mesh vertex
	D3D11_INPUT_ELEMENT_DESC SunInputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDev->CreateInputLayout(SunInputDesc, 3, MyLoadVShader, sizeof(MyLoadVShader), &sunMeshLayout);

	hr = CreateDDSTextureFromFile(myDev, L"Assets/SunTexture.dds", (ID3D11Resource**)&sunTexture, &suntextureRV);
#pragma endregion

#pragma region EarthMesh
	LoadMesh("Assets/planet.mesh", earthMesh);
	for (int i = 0; i < earthMesh.verticesList.size(); ++i)
	{
		earthMesh.verticesList[i].Pos.x *= 15;
		earthMesh.verticesList[i].Pos.y *= 15;
		earthMesh.verticesList[i].Pos.z *= 15;
	}
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(SimpleMesh) * (earthMesh.verticesList.size());
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	subData.pSysMem = earthMesh.verticesList.data();

	hr = myDev->CreateBuffer(&bDesc, &subData, &earthvBuff);
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * (earthMesh.indicesList.size());
	subData.pSysMem = earthMesh.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &earthiBuff);

	// load our new mesh shader
	hr = myDev->CreateVertexShader(MyLoadVShader, sizeof(MyLoadVShader), nullptr, &earthvShader);
	hr = myDev->CreatePixelShader(MyLoadPShader, sizeof(MyLoadPShader), nullptr, &earthpShader);

	// make matching input layout for mesh vertex
	D3D11_INPUT_ELEMENT_DESC EarthInputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDev->CreateInputLayout(EarthInputDesc, 3, MyLoadVShader, sizeof(MyLoadVShader), &earthMeshLayout);

	hr = CreateDDSTextureFromFile(myDev, L"Assets/EarthTexture.dds", (ID3D11Resource**)&earthTexture, &earthtextureRV);
#pragma endregion

#pragma region JupiterMesh
	LoadMesh("Assets/planet.mesh", jupiterMesh);
	for (int i = 0; i < jupiterMesh.verticesList.size(); ++i)
	{
		jupiterMesh.verticesList[i].Pos.x *= 25;
		jupiterMesh.verticesList[i].Pos.y *= 25;
		jupiterMesh.verticesList[i].Pos.z *= 25;
	}
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(SimpleMesh) * (jupiterMesh.verticesList.size());
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	subData.pSysMem = jupiterMesh.verticesList.data();

	hr = myDev->CreateBuffer(&bDesc, &subData, &jupitervBuff);
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * (jupiterMesh.indicesList.size());
	subData.pSysMem = jupiterMesh.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &jupiteriBuff);

	// load our new mesh shader
	hr = myDev->CreateVertexShader(MyLoadVShader, sizeof(MyLoadVShader), nullptr, &jupitervShader);
	hr = myDev->CreatePixelShader(MyLoadPShader, sizeof(MyLoadPShader), nullptr, &jupiterpShader);

	// make matching input layout for mesh vertex
	D3D11_INPUT_ELEMENT_DESC JupiterInputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDev->CreateInputLayout(JupiterInputDesc, 3, MyLoadVShader, sizeof(MyLoadVShader), &jupiterMeshLayout);

	hr = CreateDDSTextureFromFile(myDev, L"Assets/JupiterTexture.dds", (ID3D11Resource**)&jupiterTexture, &jupitertextureRV);
#pragma endregion

#pragma region MoonMesh
	LoadMesh("Assets/planet.mesh", moonMesh);
	for (int i = 0; i < moonMesh.verticesList.size(); ++i)
	{
		moonMesh.verticesList[i].Pos.x *= 5;
		moonMesh.verticesList[i].Pos.y *= 5;
		moonMesh.verticesList[i].Pos.z *= 5;
	}
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(SimpleMesh) * (moonMesh.verticesList.size());
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	subData.pSysMem = moonMesh.verticesList.data();

	hr = myDev->CreateBuffer(&bDesc, &subData, &moonvBuff);
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * (moonMesh.indicesList.size());
	subData.pSysMem = moonMesh.indicesList.data();
	hr = myDev->CreateBuffer(&bDesc, &subData, &mooniBuff);

	// load our new mesh shader
	hr = myDev->CreateVertexShader(MyLoadVShader, sizeof(MyLoadVShader), nullptr, &moonvShader);
	hr = myDev->CreatePixelShader(MyLoadPShader, sizeof(MyLoadPShader), nullptr, &moonpShader);

	// make matching input layout for mesh vertex
	D3D11_INPUT_ELEMENT_DESC MoonInputDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = myDev->CreateInputLayout(MoonInputDesc, 3, MyLoadVShader, sizeof(MyLoadVShader), &moonMeshLayout);

	hr = CreateDDSTextureFromFile(myDev, L"Assets/MoonTexture.dds", (ID3D11Resource**)&moonTexture, &moontextureRV);
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