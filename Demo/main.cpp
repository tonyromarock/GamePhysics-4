//--------------------------------------------------------------------------------------
// File: main.cpp
//
// The main file containing the entry point main().
//--------------------------------------------------------------------------------------

#include <sstream>
#include <iomanip>
#include <random>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

//DirectX includes
#include <DirectXMath.h>
using namespace DirectX;
using std::cout;

// Effect framework includes
#include <d3dx11effect.h>

// DXUT includes
#include <DXUT.h>
#include <DXUTcamera.h>

// DirectXTK includes
#include "Effects.h"
#include "VertexTypes.h"
#include "PrimitiveBatch.h"
#include "GeometricPrimitive.h"
#include "ScreenGrab.h"

// AntTweakBar includes
#include "AntTweakBar.h"

// Internal includes
#include "util/util.h"
#include "util/FFmpeg.h"

// Own Includes (Peter)
#include "Point.h"
#include "Box.h"
#include "collisionDetect.h"
#include <math.h> // for PI

#define TEMPLATE_DEMO
//#define MASS_SPRING_SYSTEM

// DXUT camera
// NOTE: CModelViewerCamera does not only manage the standard view transformation/camera position 
//       (CModelViewerCamera::GetViewMatrix()), but also allows for model rotation
//       (CModelViewerCamera::GetWorldMatrix()). 
//       Look out for CModelViewerCamera::SetButtonMasks(...).
CModelViewerCamera g_camera;

// Effect corresponding to "effect.fx"
ID3DX11Effect* g_pEffect = nullptr;
ID3D11Device* g_pPd3Device = nullptr;
// Main tweak bar
TwBar* g_pTweakBar;

// DirectXTK effects, input layouts and primitive batches for different vertex types
// (for drawing multicolored & unlit primitives)
BasicEffect*                          g_pEffectPositionColor          = nullptr;
ID3D11InputLayout*                    g_pInputLayoutPositionColor     = nullptr;
PrimitiveBatch<VertexPositionColor>*  g_pPrimitiveBatchPositionColor  = nullptr;

// DirectXTK effect, input layout and primitive batch for position/normal vertices
// (for drawing unicolor & oriented & lit primitives)
BasicEffect*                          g_pEffectPositionNormal         = nullptr;
ID3D11InputLayout*                    g_pInputLayoutPositionNormal    = nullptr;
PrimitiveBatch<VertexPositionNormal>* g_pPrimitiveBatchPositionNormal = nullptr;

BasicEffect*                               g_pEffectPositionNormalColor         = nullptr;
ID3D11InputLayout*                         g_pInputLayoutPositionNormalColor    = nullptr;
PrimitiveBatch<VertexPositionNormalColor>* g_pPrimitiveBatchPositionNormalColor = nullptr;

// DirectXTK simple geometric primitives
std::unique_ptr<GeometricPrimitive> g_pSphere;
std::unique_ptr<GeometricPrimitive> g_pTeapot;

// Movable object management
XMINT2   g_viMouseDelta = XMINT2(0,0);
XMFLOAT3 g_vfMovableObjectPos = XMFLOAT3(0,0,0);
XMFLOAT3 g_vfRotate = XMFLOAT3(0, 0, 0);

// TweakAntBar GUI variables

int g_iTestCase = 1;
int g_iPreTestCase = -1;
bool  g_bSimulateByStep = false;
bool  g_bIsSpaceReleased = true;
#ifdef TEMPLATE_DEMO
int   g_iNumSpheres    = 100;
float g_fSphereSize    = 0.05f;
bool  g_bDrawTeapot    = true;
bool  g_bDrawTriangle  = true;
bool  g_bDrawSpheres = true;
// self added variables
bool	g_bDrawBoxes = true;
bool	g_bApplyGravity = false;
float	h_timeStep = 0.1f;
float	g_fGravity = -9.18f;
float	g_fMass = 2.0f;
float	g_fCConst = 0.1f;	// constant for collision handling
#endif
//#ifdef MASS_SPRING_SYSTEM
//#endif

// Physics objects

std::vector<Box*> boxes;

// added functions

void DrawBoxes(ID3D11DeviceContext* pd3dImmediateContext);
void nextStep(float timeStep);
Box* addBox(float x, float y, float z, XMVECTOR pos, float mass, bool fixed, XMVECTOR orientation);
void RigidBodyInit(int mode);
void PhysicValuesInit();
void printVector(XMVECTOR vec);
void RigidCollInit();



// Video recorder
FFmpeg* g_pFFmpegVideoRecorder = nullptr;

// Create TweakBar and add required buttons and variables
void InitTweakBar(ID3D11Device* pd3dDevice)
{
    g_pTweakBar = TwNewBar("TweakBar");
	TwDefine(" TweakBar color='0 128 128' alpha=128 ");

	TwType TW_TYPE_TESTCASE = TwDefineEnumFromString("Test Scene", "BasicTest,Setup1,Setup2,Setup3,Demo1,Demo2,Demo3,Demo4");
	TwAddVarRW(g_pTweakBar, "Test Scene", TW_TYPE_TESTCASE, &g_iTestCase, "");
	// HINT: For buttons you can directly pass the callback function as a lambda expression.
	TwAddButton(g_pTweakBar, "Reset Scene", [](void *){g_iPreTestCase = -1; }, nullptr, "");
	TwAddButton(g_pTweakBar, "Reset Camera", [](void *){g_camera.Reset(); }, nullptr, "");
	// Run mode, step by step, control by space key
	TwAddVarRW(g_pTweakBar, "RunStep(space)", TW_TYPE_BOOLCPP, &g_bSimulateByStep, "");
	
#ifdef TEMPLATE_DEMO
	switch (g_iTestCase)
	{
	case 0:
		TwAddVarRW(g_pTweakBar, "Draw Spheres", TW_TYPE_BOOLCPP, &g_bDrawSpheres, "");
		TwAddVarRW(g_pTweakBar, "Num Spheres", TW_TYPE_INT32, &g_iNumSpheres, "min=1");
		TwAddVarRW(g_pTweakBar, "Sphere Size", TW_TYPE_FLOAT, &g_fSphereSize, "min=0.01 step=0.01");
		break;
	case 1:
		TwAddVarRW(g_pTweakBar, "Draw Teapot",   TW_TYPE_BOOLCPP, &g_bDrawTeapot, "");
		break;
	case 2:
		TwAddVarRW(g_pTweakBar, "Draw Triangle", TW_TYPE_BOOLCPP, &g_bDrawTriangle, "");
		break;
	case 4:
		TwAddVarRW(g_pTweakBar, "Draw Box", TW_TYPE_BOOLCPP, &g_bDrawBoxes, "");
		break;
	case 5:
		TwAddVarRW(g_pTweakBar, "Draw Box", TW_TYPE_BOOLCPP, &g_bDrawBoxes, "");
		TwAddVarRW(g_pTweakBar, "Add Gravity", TW_TYPE_BOOLCPP, &g_bApplyGravity, "");
		break;
	case 6:
		TwAddVarRW(g_pTweakBar, "Draw Box", TW_TYPE_BOOLCPP, &g_bDrawBoxes, "");
		break;
	default:
		break;
	}
#endif

//#ifdef MASS_SPRING_SYSTEM
//#endif
}

// Draw the Boxes for the Rigid Body Simulation

void DrawBoxes(ID3D11DeviceContext* pd3dImmediateContext)
{
	pd3dImmediateContext->IASetInputLayout(g_pInputLayoutPositionColor);
	
	// index buffer
	int index[12][3] = { { 0, 2, 1 }, { 1, 2, 3 }, { 4, 5, 6 }, { 6, 5, 7 }, { 0, 1, 4 }, { 4, 1, 5 }, { 6, 7, 2 }, { 2, 7, 3 }, { 0, 4, 2 }, { 2, 4, 6 }, { 3, 7, 1 }, { 1, 7, 5 } };
	
	for (auto box: boxes) {
		g_pEffectPositionColor->SetWorld(XMMatrixRotationQuaternion(box->orientation) * box->transform);
		g_pEffectPositionColor->Apply(pd3dImmediateContext);

		for (auto vertice : index)
		{
			g_pPrimitiveBatchPositionColor->Begin();

			g_pPrimitiveBatchPositionColor->DrawTriangle(VertexPositionColor(box->corners[vertice[0]], Colors::Red),
				VertexPositionColor(box->corners[vertice[1]], Colors::Green),
				VertexPositionColor(box->corners[vertice[2]], Colors::Yellow));
			g_pPrimitiveBatchPositionColor->End();
		}

	}
}

// Draw the edges of the bounding box [-0.5;0.5]� rotated with the cameras model tranformation.
// (Drawn as line primitives using a DirectXTK primitive batch)
void DrawBoundingBox(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/color effect
    g_pEffectPositionColor->SetWorld(g_camera.GetWorldMatrix());
    
    g_pEffectPositionColor->Apply(pd3dImmediateContext);
    pd3dImmediateContext->IASetInputLayout(g_pInputLayoutPositionColor);

    // Draw
    g_pPrimitiveBatchPositionColor->Begin();
    
    // Lines in x direction (red color)
    for (int i=0; i<4; i++)
    {
        g_pPrimitiveBatchPositionColor->DrawLine(
            VertexPositionColor(XMVectorSet(-0.5f, (float)(i%2)-0.5f, (float)(i/2)-0.5f, 1), Colors::Red),
            VertexPositionColor(XMVectorSet( 0.5f, (float)(i%2)-0.5f, (float)(i/2)-0.5f, 1), Colors::Red)
        );
    }

    // Lines in y direction
    for (int i=0; i<4; i++)
    {
        g_pPrimitiveBatchPositionColor->DrawLine(
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f, -0.5f, (float)(i/2)-0.5f, 1), Colors::Green),
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f,  0.5f, (float)(i/2)-0.5f, 1), Colors::Green)
        );
    }

    // Lines in z direction
    for (int i=0; i<4; i++)
    {
        g_pPrimitiveBatchPositionColor->DrawLine(
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f, (float)(i/2)-0.5f, -0.5f, 1), Colors::Blue),
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f, (float)(i/2)-0.5f,  0.5f, 1), Colors::Blue)
        );
    }

    g_pPrimitiveBatchPositionColor->End();
}

// Draw a large, square plane at y=-1 with a checkerboard pattern
// (Drawn as multiple quads, i.e. triangle strips, using a DirectXTK primitive batch)
void DrawFloor(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/normal/color effect
    g_pEffectPositionNormalColor->SetWorld(XMMatrixIdentity());
    g_pEffectPositionNormalColor->SetEmissiveColor(Colors::Black);
    g_pEffectPositionNormalColor->SetDiffuseColor(0.8f * Colors::White);
    g_pEffectPositionNormalColor->SetSpecularColor(0.4f * Colors::White);
    g_pEffectPositionNormalColor->SetSpecularPower(1000);
    
    g_pEffectPositionNormalColor->Apply(pd3dImmediateContext);
    pd3dImmediateContext->IASetInputLayout(g_pInputLayoutPositionNormalColor);

    // Draw 4*n*n quads spanning x = [-n;n], y = -1, z = [-n;n]
    const float n = 4;
    XMVECTOR normal      = XMVectorSet(0, 1,0,0);
    XMVECTOR planecenter = XMVectorSet(0,-1,0,0);

    g_pPrimitiveBatchPositionNormalColor->Begin();
    for (float z = -n; z < n; z++)
    {
        for (float x = -n; x < n; x++)
        {
            // Quad vertex positions
            XMVECTOR pos[] = { XMVectorSet(x  , -1, z+1, 0),
                               XMVectorSet(x+1, -1, z+1, 0),
                               XMVectorSet(x+1, -1, z  , 0),
                               XMVectorSet(x  , -1, z  , 0) };

            // Color checkerboard pattern (white & gray)
            XMVECTOR color = ((int(z + x) % 2) == 0) ? XMVectorSet(1,1,1,1) : XMVectorSet(0.6f,0.6f,0.6f,1);

            // Color attenuation based on distance to plane center
            float attenuation[] = {
                1.0f - XMVectorGetX(XMVector3Length(pos[0] - planecenter)) / n,
                1.0f - XMVectorGetX(XMVector3Length(pos[1] - planecenter)) / n,
                1.0f - XMVectorGetX(XMVector3Length(pos[2] - planecenter)) / n,
                1.0f - XMVectorGetX(XMVector3Length(pos[3] - planecenter)) / n };

            g_pPrimitiveBatchPositionNormalColor->DrawQuad(
                VertexPositionNormalColor(pos[0], normal, attenuation[0] * color),
                VertexPositionNormalColor(pos[1], normal, attenuation[1] * color),
                VertexPositionNormalColor(pos[2], normal, attenuation[2] * color),
                VertexPositionNormalColor(pos[3], normal, attenuation[3] * color)
            );
        }
    }
    g_pPrimitiveBatchPositionNormalColor->End();    
}

#ifdef TEMPLATE_DEMO
// Draw several objects randomly positioned in [-0.5f;0.5]�  using DirectXTK geometric primitives.
void DrawSomeRandomObjects(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/normal effect (constant variables)
    g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
    g_pEffectPositionNormal->SetSpecularColor(0.4f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(100);
      
    std::mt19937 eng;
    std::uniform_real_distribution<float> randCol( 0.0f, 1.0f);
    std::uniform_real_distribution<float> randPos(-0.5f, 0.5f);

    for (int i=0; i<g_iNumSpheres; i++)
    {
        // Setup position/normal effect (per object variables)
        g_pEffectPositionNormal->SetDiffuseColor(0.6f * XMColorHSVToRGB(XMVectorSet(randCol(eng), 1, 1, 0)));
        XMMATRIX scale    = XMMatrixScaling(g_fSphereSize, g_fSphereSize, g_fSphereSize);
        XMMATRIX trans    = XMMatrixTranslation(randPos(eng),randPos(eng),randPos(eng));
        g_pEffectPositionNormal->SetWorld(scale * trans * g_camera.GetWorldMatrix());

        // Draw
        // NOTE: The following generates one draw call per object, so performance will be bad for n>>1000 or so
        g_pSphere->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
    }
}

// Draw a teapot at the position g_vfMovableObjectPos.
void DrawMovableTeapot(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/normal effect (constant variables)
    g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
    g_pEffectPositionNormal->SetDiffuseColor(0.6f * Colors::Cornsilk);
    g_pEffectPositionNormal->SetSpecularColor(0.4f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(100);

    XMMATRIX scale    = XMMatrixScaling(0.5f, 0.5f, 0.5f);    
    XMMATRIX trans    = XMMatrixTranslation(g_vfMovableObjectPos.x, g_vfMovableObjectPos.y, g_vfMovableObjectPos.z);
	XMMATRIX rotations = XMMatrixRotationRollPitchYaw(g_vfRotate.x, g_vfRotate.y, g_vfRotate.z);
	g_pEffectPositionNormal->SetWorld(rotations * scale * trans);

    // Draw
    g_pTeapot->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
}

// Draw a simple triangle using custom shaders (g_pEffect)
void DrawTriangle(ID3D11DeviceContext* pd3dImmediateContext)
{
	XMMATRIX world = g_camera.GetWorldMatrix();
	XMMATRIX view  = g_camera.GetViewMatrix();
	XMMATRIX proj  = g_camera.GetProjMatrix();
	XMFLOAT4X4 mViewProj;
	XMStoreFloat4x4(&mViewProj, world * view * proj);
	g_pEffect->GetVariableByName("g_worldViewProj")->AsMatrix()->SetMatrix((float*)mViewProj.m);
	g_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->Apply(0, pd3dImmediateContext);
    
	pd3dImmediateContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	pd3dImmediateContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
	pd3dImmediateContext->IASetInputLayout(nullptr);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dImmediateContext->Draw(3, 0);
}
#endif

//#ifdef MASS_SPRING_SYSTEM
//void DrawMassSpringSystem(ID3D11DeviceContext* pd3dImmediateContext)
//#endif
// ============================================================
// DXUT Callbacks
// ============================================================


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependent on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();;

    std::wcout << L"Device: " << DXUTGetDeviceStats() << std::endl;
    
    // Load custom effect from "effect.fxo" (compiled "effect.fx")
	std::wstring effectPath = GetExePath() + L"effect.fxo";
	if(FAILED(hr = D3DX11CreateEffectFromFile(effectPath.c_str(), 0, pd3dDevice, &g_pEffect)))
	{
        std::wcout << L"Failed creating effect with error code " << int(hr) << std::endl;
		return hr;
	}

    // Init AntTweakBar GUI
	TwInit(TW_DIRECT3D11, pd3dDevice);
    InitTweakBar(pd3dDevice);

    // Create DirectXTK geometric primitives for later usage
    g_pSphere = GeometricPrimitive::CreateGeoSphere(pd3dImmediateContext, 2.0f, 2, false);
    g_pTeapot = GeometricPrimitive::CreateTeapot(pd3dImmediateContext, 1.5f, 8, false);

    // Create effect, input layout and primitive batch for position/color vertices (DirectXTK)
    {
        // Effect
        g_pEffectPositionColor = new BasicEffect(pd3dDevice);
        g_pEffectPositionColor->SetVertexColorEnabled(true); // triggers usage of position/color vertices

        // Input layout
        void const* shaderByteCode;
        size_t byteCodeLength;
        g_pEffectPositionColor->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
        
        pd3dDevice->CreateInputLayout(VertexPositionColor::InputElements,
                                      VertexPositionColor::InputElementCount,
                                      shaderByteCode, byteCodeLength,
                                      &g_pInputLayoutPositionColor);

        // Primitive batch
        g_pPrimitiveBatchPositionColor = new PrimitiveBatch<VertexPositionColor>(pd3dImmediateContext);
    }

    // Create effect, input layout and primitive batch for position/normal vertices (DirectXTK)
    {
        // Effect
        g_pEffectPositionNormal = new BasicEffect(pd3dDevice);
        g_pEffectPositionNormal->EnableDefaultLighting(); // triggers usage of position/normal vertices
        g_pEffectPositionNormal->SetPerPixelLighting(true);

        // Input layout
        void const* shaderByteCode;
        size_t byteCodeLength;
        g_pEffectPositionNormal->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        pd3dDevice->CreateInputLayout(VertexPositionNormal::InputElements,
                                      VertexPositionNormal::InputElementCount,
                                      shaderByteCode, byteCodeLength,
                                      &g_pInputLayoutPositionNormal);

        // Primitive batch
        g_pPrimitiveBatchPositionNormal = new PrimitiveBatch<VertexPositionNormal>(pd3dImmediateContext);
    }

    // Create effect, input layout and primitive batch for position/normal/color vertices (DirectXTK)
    {
        // Effect
        g_pEffectPositionNormalColor = new BasicEffect(pd3dDevice);
        g_pEffectPositionNormalColor->SetPerPixelLighting(true);
        g_pEffectPositionNormalColor->EnableDefaultLighting();     // triggers usage of position/normal/color vertices
        g_pEffectPositionNormalColor->SetVertexColorEnabled(true); // triggers usage of position/normal/color vertices

        // Input layout
        void const* shaderByteCode;
        size_t byteCodeLength;
        g_pEffectPositionNormalColor->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        pd3dDevice->CreateInputLayout(VertexPositionNormalColor::InputElements,
                                      VertexPositionNormalColor::InputElementCount,
                                      shaderByteCode, byteCodeLength,
                                      &g_pInputLayoutPositionNormalColor);

        // Primitive batch
        g_pPrimitiveBatchPositionNormalColor = new PrimitiveBatch<VertexPositionNormalColor>(pd3dImmediateContext);
    }

	g_pPd3Device = pd3dDevice;
	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	SAFE_RELEASE(g_pEffect);
	
    TwDeleteBar(g_pTweakBar);
    g_pTweakBar = nullptr;
	TwTerminate();

    g_pSphere.reset();
    g_pTeapot.reset();

	for each (auto box in boxes) { delete box; }
    
    SAFE_DELETE (g_pPrimitiveBatchPositionColor);
    SAFE_RELEASE(g_pInputLayoutPositionColor);
    SAFE_DELETE (g_pEffectPositionColor);

    SAFE_DELETE (g_pPrimitiveBatchPositionNormal);
    SAFE_RELEASE(g_pInputLayoutPositionNormal);
    SAFE_DELETE (g_pEffectPositionNormal);

    SAFE_DELETE (g_pPrimitiveBatchPositionNormalColor);
    SAFE_RELEASE(g_pInputLayoutPositionNormalColor);
    SAFE_DELETE (g_pEffectPositionNormalColor);
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    // Update camera parameters
	int width = pBackBufferSurfaceDesc->Width;
	int height = pBackBufferSurfaceDesc->Height;
	g_camera.SetWindow(width, height);
	g_camera.SetProjParams(XM_PI / 4.0f, float(width) / float(height), 0.1f, 100.0f);

    // Inform AntTweakBar about back buffer resolution change
  	TwWindowSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    HRESULT hr;

	if(bKeyDown)
	{
		switch(nChar)
		{
            // RETURN: toggle fullscreen
			case VK_RETURN :
			{
				if(bAltDown) DXUTToggleFullScreen();
				break;
			}
            // F8: Take screenshot
			case VK_F8:
			{
                // Save current render target as png
                static int nr = 0;
				std::wstringstream ss;
				ss << L"Screenshot" << std::setfill(L'0') << std::setw(4) << nr++ << L".png";

                ID3D11Resource* pTex2D = nullptr;
                DXUTGetD3D11RenderTargetView()->GetResource(&pTex2D);
                SaveWICTextureToFile(DXUTGetD3D11DeviceContext(), pTex2D, GUID_ContainerFormatPng, ss.str().c_str());
                SAFE_RELEASE(pTex2D);

                std::wcout << L"Screenshot written to " << ss.str() << std::endl;
				break;
			}
            // F10: Toggle video recording
            case VK_F10:
            {
                if (!g_pFFmpegVideoRecorder) {
                    g_pFFmpegVideoRecorder = new FFmpeg(25, 21, FFmpeg::MODE_INTERPOLATE);
                    V(g_pFFmpegVideoRecorder->StartRecording(DXUTGetD3D11Device(), DXUTGetD3D11RenderTargetView(), "output.avi"));
                } else {
                    g_pFFmpegVideoRecorder->StopRecording();
                    SAFE_DELETE(g_pFFmpegVideoRecorder);
                }
            }			    
		}
	}
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                       bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                       int xPos, int yPos, void* pUserContext )
{
	switch (g_iTestCase)
	{
	case 1:
		// Track mouse movement if left mouse key is pressed
		{
			static int xPosSave = 0, yPosSave = 0;

			if (bLeftButtonDown)
			{
				// Accumulate deltas in g_viMouseDelta
				g_viMouseDelta.x += xPos - xPosSave;
				g_viMouseDelta.y += yPos - yPosSave;
			}

			xPosSave = xPos;
			yPosSave = yPos;
		}
		break;
	default:
		break;
	}
   
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Send message to AntTweakbar first
    if (TwEventWin(hWnd, uMsg, wParam, lParam))
    {
        *pbNoFurtherProcessing = true;
        return 0;
    }

    // If message not processed yet, send to camera
	if(g_camera.HandleMessages(hWnd,uMsg,wParam,lParam))
    {
        *pbNoFurtherProcessing = true;
		return 0;
    }

	return 0;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double dTime, float fElapsedTime, void* pUserContext)
{
	UpdateWindowTitle(L"Demo");

	static float time_counter = 0;

	// Move camera
	g_camera.FrameMove(fElapsedTime);

	// Update effects with new view + proj transformations
	g_pEffectPositionColor->SetView(g_camera.GetViewMatrix());
	g_pEffectPositionColor->SetProjection(g_camera.GetProjMatrix());

	g_pEffectPositionNormal->SetView(g_camera.GetViewMatrix());
	g_pEffectPositionNormal->SetProjection(g_camera.GetProjMatrix());

	g_pEffectPositionNormalColor->SetView(g_camera.GetViewMatrix());
	g_pEffectPositionNormalColor->SetProjection(g_camera.GetProjMatrix());

#ifdef TEMPLATE_DEMO

	if (g_iPreTestCase != g_iTestCase){// test case changed
		// clear old setup and build up new setup
		TwDeleteBar(g_pTweakBar);
		g_pTweakBar = nullptr;
		InitTweakBar(g_pPd3Device);
		switch (g_iTestCase)
		{
		case 0:
			cout << "Basic Test!\n";
			g_bDrawSpheres = true;
			g_iNumSpheres = 100;
			g_fSphereSize = 0.05f;
			break;
		case 1:
			cout << "Test1!\n";
			g_bDrawTeapot = true;			
			g_vfMovableObjectPos = XMFLOAT3(0, 0, 0);
			g_vfRotate = XMFLOAT3(0, 0, 0);
			break;
		case 2:
			cout << "Test2!\n";
			g_bDrawTriangle = true;
			break;
		case 4:
			cout << "Demo1\n";
			g_bDrawBoxes = true;
			RigidBodyInit(4);
			break;
		case 5:
			cout << "Demo2\n";
			g_bDrawBoxes = true;
			RigidBodyInit(5);
			break;
		case 6:
			cout << "Demo3\n";
			g_bDrawBoxes = true;
			RigidCollInit();
			break;
		default:
			cout << "Empty Test!\n";
			break;
		}
		g_iPreTestCase = g_iTestCase;
	}
	if (g_bSimulateByStep && DXUTIsKeyDown(VK_SPACE)){
		g_bIsSpaceReleased = false;
	}
	if (g_bSimulateByStep && !g_bIsSpaceReleased)
		if (DXUTIsKeyDown(VK_SPACE))
			return;
	if (g_bSimulateByStep && g_bIsSpaceReleased)
		return;
	// update current setup for each frame
	switch (g_iTestCase)
	{// handling different cases
	case 1:
		// Apply accumulated mouse deltas to g_vfMovableObjectPos (move along cameras view plane)
		if (g_viMouseDelta.x != 0 || g_viMouseDelta.y != 0)
		{
			// Calcuate camera directions in world space
			XMMATRIX viewInv = XMMatrixInverse(nullptr, g_camera.GetViewMatrix());
			XMVECTOR camRightWorld = XMVector3TransformNormal(g_XMIdentityR0, viewInv);
			XMVECTOR camUpWorld = XMVector3TransformNormal(g_XMIdentityR1, viewInv);

			// Add accumulated mouse deltas to movable object pos
			XMVECTOR vMovableObjectPos = XMLoadFloat3(&g_vfMovableObjectPos);

			float speedScale = 0.001f;
			vMovableObjectPos = XMVectorAdd(vMovableObjectPos, speedScale * (float)g_viMouseDelta.x * camRightWorld);
			vMovableObjectPos = XMVectorAdd(vMovableObjectPos, -speedScale * (float)g_viMouseDelta.y * camUpWorld);

			XMStoreFloat3(&g_vfMovableObjectPos, vMovableObjectPos);

			// Reset accumulated mouse deltas
			g_viMouseDelta = XMINT2(0, 0);
		}
		// rotate the teapot
		g_vfRotate.x += 0.005f;
		if (g_vfRotate.x > 2 * M_PI) g_vfRotate.x -= 2 * M_PI;
		g_vfRotate.y += 0.005f;
		if (g_vfRotate.y > 2 * M_PI) g_vfRotate.y -= 2 * M_PI;
		g_vfRotate.z += 0.005f;
		if (g_vfRotate.z > 2 * M_PI) g_vfRotate.z -= 2 * M_PI;

		break;
	case 4:
	case 5:
	case 6:
		time_counter += fElapsedTime;
		if (time_counter > h_timeStep)
		{
			nextStep(h_timeStep);
			time_counter -= h_timeStep;
		}
		break;
	default:
		break;
	}
	if (g_bSimulateByStep)
		g_bIsSpaceReleased = true;
	
#endif
//#ifdef MASS_SPRING_SYSTEM
//#endif
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
                                  double fTime, float fElapsedTime, void* pUserContext )
{
    HRESULT hr;

	// Clear render target and depth stencil
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    // Draw floor
    DrawFloor(pd3dImmediateContext);

    // Draw axis box
    DrawBoundingBox(pd3dImmediateContext);


#ifdef TEMPLATE_DEMO
	switch (g_iTestCase)
	{
	case 0:
		// Draw speheres
		if (g_bDrawSpheres) DrawSomeRandomObjects(pd3dImmediateContext);
		break;
	case 1:
		// Draw movable teapot
		if (g_bDrawTeapot) DrawMovableTeapot(pd3dImmediateContext);
		break;
	case 2:
		// Draw simple triangle
		if (g_bDrawTriangle) DrawTriangle(pd3dImmediateContext);
		break;
	case 4:
	case 5:
	case 6:
		// Draw a box
		if (g_bDrawBoxes)	{
			DrawBoxes(pd3dImmediateContext);
			/*for each (auto box in boxes)
			{
				cout << XMVectorGetByIndex(box->position,0) <<
					", " << XMVectorGetByIndex(box->position, 1) <<
					", " << XMVectorGetByIndex(box->position, 2) << "\n";
			}*/
		}
		break;
	default:
		break;
	}
#endif

//#ifdef MASS_SPRING_SYSTEM
//#endif

    // Draw GUI
    TwDraw();

    if (g_pFFmpegVideoRecorder) 
    {
        V(g_pFFmpegVideoRecorder->AddFrame(pd3dImmediateContext, DXUTGetD3D11RenderTargetView()));
    }
}

// Physics objects functions

Box* addBox(float x, float y, float z, XMVECTOR pos, float mass, bool fixed, XMVECTOR orientation)
{
	Box* b = new Box(x, y, z, pos, mass, fixed, orientation);
	boxes.push_back(b);
	return b;
}

void nextStep(float timeStep)
{
	// compare with slide 14 of "lecture06_rb3d"
	for each(auto box in boxes)
	{
		if (box->centerOfMass.fixed) { continue; }
		if (g_iTestCase == 5 && g_bApplyGravity)
		{
			box->centerOfMass.addGravity(timeStep);
			box->torqueAccumulator += box->centerOfMass.getTotalForce();
		}


		// Euler step
		box->position += box->velocity * timeStep;
		box->transform = XMMatrixTranslationFromVector(box->position);
		box->velocity += (box->torqueAccumulator / box->centerOfMass.mass) * timeStep;

		// Quaternion
		box->orientation = XMQuaternionNormalize(box->orientation + 
			XMQuaternionMultiply((timeStep / 2.0f) * XMVectorSetW(box->angularVelocity, 0.0f), box->orientation));

		box->angularMomentum += box->torqueAccumulator * timeStep;

		XMMATRIX rotation = XMMatrixRotationQuaternion(box->orientation);
		XMMATRIX inertiaTensorInverse = rotation * box->intertiaTensorInverse * XMMatrixTranspose(rotation);

		box->angularVelocity = XMVector3Transform(box->angularMomentum, inertiaTensorInverse);

		box->clearTorque();
		box->centerOfMass.clearForces();

	}

	for each (auto box1 in boxes) 
	{
		for each (auto box2 in boxes) 
		{
			if (box1 == box2) { continue; }

			CollisionInfo info = checkCollision(XMMatrixRotationQuaternion(box1->orientation) * box1->transform,
				XMMatrixRotationQuaternion(box2->orientation)* box2->transform);

			if (info.isValid) 
			{
				if (XMVectorGetByIndex(XMVector3Dot(box1->velocity - box2->velocity, -info.normalWorld), 0) > 0){ continue; }

				// top and bottom half of the fraction
				float j_up = 0.f;
				float j_down = 0.f;

				j_up = -(1.0f + g_fCConst) * XMVectorGetByIndex(XMVector3Dot(box1->velocity - box2->velocity, -info.normalWorld), 0);
				j_down =
					box1->massInverse +
					box2->massInverse +
					XMVectorGetByIndex(XMVector3Dot(
					XMVector3Cross(XMVector3Transform(XMVector3Cross(box1->position, info.normalWorld),box1->intertiaTensorInverse), box1->position)
					+ XMVector3Cross(XMVector3Transform(XMVector3Cross(box2->position, info.normalWorld), box2->intertiaTensorInverse), box2->position)
					,info.normalWorld
					), 0);

				float j = j_up / j_down;

				box1->velocity = box1->velocity - (j * info.normalWorld * box1->massInverse);
				box2->velocity = box2->velocity + (j * info.normalWorld * box2->massInverse);

				box1->angularMomentum = box1->angularMomentum - XMVector3Cross(box1->position, (j * info.normalWorld));
				box2->angularMomentum = box2->angularMomentum + XMVector3Cross(box2->position, (j * info.normalWorld));

			}


		}
	}
}

void RigidBodyInit(int mode)
{
	for each (auto box in boxes)
	{
		delete box;
	}

	boxes.clear();

	Box* boxA;
	XMVECTOR boxA_cm_pos;

	switch (mode)
	{
	case 4:
		h_timeStep = 2.0f;
		
		g_bApplyGravity = false;

		// notice: rotate around z axis by 90
		boxA_cm_pos = XMVectorSet(0.f, 0.f, 0.f, 0.f);
		boxA = addBox(1.f, 0.6f, 0.5f, boxA_cm_pos, 2.f, false, XMVectorSet(0.f, 0.f, M_PI_2, 0.f));
		
		boxA->addTorque(XMVectorSet(1.f, 1.f, 0.f, 0.f), XMVectorSet(0.3f, 0.5f, 0.25f, 0.f));

		nextStep(h_timeStep);

		cout << "After one timestep (2.0)\n";
		cout << "Linear Velocity: "; printVector(boxA->velocity);
		cout << "Angular Velocity: "; printVector(boxA->angularVelocity);
		cout << "World Space of (-0.3, -0.5, -0.25): ";														// v_i = (x_i+1 - x_i)/2
		printVector( (boxA->position - (boxA->corners[0] - boxA->position) - boxA->corners[0]) * (0.5f));	// corners[0] (-,-,-)
		break;
	case 5:
		h_timeStep = 0.1f;

		// notice: rotate around z axis by 90
		boxA_cm_pos = XMVectorSet(0.f, 0.f, 0.f, 0.f);
		boxA = addBox(1.f, 0.6f, 0.5f, boxA_cm_pos, 2.f, false, XMVectorSet(0.f, 0.f, M_PI_2, 0.f));

		boxA->addTorque(XMVectorSet(1.f, 1.f, 0.f, 0.f), XMVectorSet(0.3f, 0.5f, 0.25f, 0.f));
		break;
	}
	
}

// for Demo3 a collision
void RigidCollInit() 
{
	for each (auto box in boxes)
	{
		delete box;
	}

	boxes.clear();

	h_timeStep = 0.1f;
	g_bApplyGravity = false;
	Box* boxA; Box* boxB;

	boxA = addBox(1.f, 0.6f, 0.5f, XMVectorSet(0.f,0.f,0.,0.f), 2.f, false, XMVectorSet(0.f, 0.f, 0.f, 0.f));
	boxB = addBox(1.f, 0.6f, 0.5f, XMVectorSet(7.0f, 0.f, 0.f, 0.f), 2.f, false, XMVectorSet(M_PI/3.f,M_PI / 3.0f, M_PI_2, 0.f));

	boxA->addVelocity(0.5f, 0.f, 0.f);
	boxB->addVelocity(-2.0f, 0.f, 0.f);

}

void PhysicValuesInit()
{
	Point::f_Mass = &g_fMass;
	Point::f_Gravity = &g_fGravity;
}

void printVector(XMVECTOR vec) 
{
	cout << "(" << XMVectorGetByIndex(vec, 0) << "| " << XMVectorGetByIndex(vec, 1) << "| " << XMVectorGetByIndex(vec, 2) << ")\n";
}

//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
#if defined(DEBUG) | defined(_DEBUG)
	// Enable run-time memory check for debug builds.
	// (on program exit, memory leaks are printed to Visual Studio's Output console)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

#ifdef _DEBUG
	std::wcout << L"---- DEBUG BUILD ----\n\n";
#endif

	// Set general DXUT callbacks
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackMouse( OnMouse, true );
	DXUTSetCallbackKeyboard( OnKeyboard );

	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

	// Set the D3D11 DXUT callbacks
	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    // Init camera
 	XMFLOAT3 eye(0.0f, 0.0f, -2.0f);
	XMFLOAT3 lookAt(0.0f, 0.0f, 0.0f);
	g_camera.SetViewParams(XMLoadFloat3(&eye), XMLoadFloat3(&lookAt));
    g_camera.SetButtonMasks(MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_RIGHT_BUTTON);

    // Init DXUT and create device
	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	//DXUTSetIsInGammaCorrectMode( false ); // true by default (SRGB backbuffer), disable to force a RGB backbuffer
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTCreateWindow( L"Demo" );
	DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 1280, 960 );

	// Init Physics Values (point mass and gravity constant)
	PhysicValuesInit();
    
	DXUTMainLoop(); // Enter into the DXUT render loop

	DXUTShutdown(); // Shuts down DXUT (includes calls to OnD3D11ReleasingSwapChain() and OnD3D11DestroyDevice())
	
	return DXUTGetExitCode();
}
