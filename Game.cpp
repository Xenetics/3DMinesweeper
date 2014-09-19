//***************************************************************************************
// 
//
//
// 
//		
//      
//
//***************************************************************************************

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "xnacollision.h"
#include "RenderStates.h"
#include <string.h>
#include <ostream>
#include <sstream>
#include <cstdlib>
#include <algorithm>

#include "fmod.hpp"
#include "fmod_errors.h"

#define EPSILON 0.00001

struct Cube
{
	XMVECTOR pos;
	UINT texture = 0;
	XNA::AxisAlignedBox mMeshBox;
	bool isMenu = false;
	XMFLOAT4X4 localWorld;
};

class CrateApp : public D3DApp
{
public:
	std::vector<Cube*> cubes;


	CrateApp(HINSTANCE hInstance);
	~CrateApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnMouseWheelMove(WPARAM btnState,int fwKeys, int zDelta, int x, int y);
	void MakeLevel(UINT width, UINT length, UINT height);
	void Pick(int sx, int sy);

private:
	void BuildGeometryBuffers();

	void InitFMOD();
	void UpdateSound();

private:
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3D11ShaderResourceView* mDiffuseMapSRV;
	ID3D11ShaderResourceView* mDiffuseMapSRV2;
	ID3D11ShaderResourceView* mDiffuseMapSRV3[120];
	ID3D11ShaderResourceView* mDiffuseMapSRV4;
	ID3D11ShaderResourceView* mDiffuseMapSRV5;
	ID3D11ShaderResourceView* mDiffuseMapSRVMenuButtons[8];
	enum menuButtons {LOGO,PLAY,EASY,MEDIUM,HARD,EXIT,SOUND,MUSIC};
	menuButtons button;

	LPCTSTR num;

	// Lighting variables
	DirectionalLight mDirLights[2];
	PointLight mPointLights[2];

	// Material variables
	Material mBoxMat;

	XMFLOAT4X4 mTexTransform;
	XMFLOAT4X4 mBoxWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mBoxVertexOffset;
	UINT mBoxIndexOffset;
	UINT mBoxIndexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
	int timer = 0;
	int whichIMG = 0;

	// enum for difficulties
	enum difficulty
	{
		easy, medium, hard
	};

	//FMOD stuff
	FMOD::System *system;
	FMOD_RESULT result;
	FMOD::Sound      *sound1, *sound2, *sound3, *music;
	FMOD::Channel    *channel1, *channel2, *channel3, *musicChannel;
	int               key;
	unsigned int      version;

public:
	// Define transformations from local spaces to world space.
	XMFLOAT4X4 mMeshWorld;
	Camera mCam;
	UINT mPickedTriangle;

	// Keep system memory copies of the Mesh geometry for picking.
	std::vector<Vertex::Basic32> mMeshVertices;
	std::vector<UINT> mMeshIndices;
	
	Material mMeshMat;
	Material mPickedTriangleMat;

	// Define transformations from local spaces to world space.
	UINT mMeshIndexCount;

	UINT SetCubeTexture(UINT cube, UINT x, UINT y, UINT z, UINT width, UINT height, UINT length); 
	std::vector<UINT> currentCubeLayer;
	std::vector<UINT> previousCubeLayer;
	BOOL hasDiamond = false;

	UINT levelWidth = 5;
	UINT levelLength = 5;
	UINT levelHeight = 5;
	bool AreSame(float a, float b); //checks if floats are the same to 5 decimal places
	void CreateMenu();
	bool menu = true;
	void CleanLevel(); //cleans the level data before loading new level
	void InitTextures();
};

void ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	CrateApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}
 

CrateApp::CrateApp(HINSTANCE hInstance)
: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mDiffuseMapSRV(0), mDiffuseMapSRV2(0), mDiffuseMapSRV4(0), mDiffuseMapSRV5(0), mEyePosW(0.0f, 0.0f, 0.0f),
mTheta(1.3f*MathHelper::Pi), mPhi(0.4f*MathHelper::Pi), mRadius(2.5f), mCam(), mMeshIndexCount(0), mPickedTriangle(-1)
{
	mDiffuseMapSRV3[0] = 0;
	mMainWndCaption = L"3D Minesweeper";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mBoxWorld, I);
	XMStoreFloat4x4(&mTexTransform, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	mDirLights[0].Ambient  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Diffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);
 
	mDirLights[1].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Diffuse  = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	mPointLights[0].Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mPointLights[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mPointLights[0].Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mPointLights[0].Att = XMFLOAT3(0.4f, 0.2f, 0.0f);
	mPointLights[0].Position = XMFLOAT3(0.0f, 1.0f, 15.0f);
	mPointLights[0].Range = 20.0f;

	mBoxMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	mPickedTriangleMat.Ambient = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	mPickedTriangleMat.Diffuse = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
	mPickedTriangleMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
	
	//MakeLevel(levelWidth, levelLength, levelHeight); //makes the cube of blocks.
	CreateMenu();

	//-------------------
	

	//XMMATRIX MeshScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	//XMMATRIX MeshOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	//XMStoreFloat4x4(&mMeshWorld, XMMatrixMultiply(MeshScale, MeshOffset));
	//XMStoreFloat4x4(&mMeshWorld, XMMatrixTranslation(levelWidth*0.5f, levelLength * 0.5f, levelHeight * 0.5f));
	mCam.SetPosition(0.0f, 0.0f, -15.0f);

	
}

CrateApp::~CrateApp()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mDiffuseMapSRV);
	ReleaseCOM(mDiffuseMapSRV2);
	ReleaseCOM(mDiffuseMapSRV4);
	ReleaseCOM(mDiffuseMapSRV5);
	for (int i = 0; i < 120; i++)
	{
		ReleaseCOM(mDiffuseMapSRV3[i]);
	}
	

	Effects::DestroyAll();
	InputLayouts::DestroyAll();

	//Clean up FMOD
	result = music->release();
	ERRCHECK(result);
	result = system->close();
	ERRCHECK(result);
	result = system->release();
	ERRCHECK(result);
}

void CrateApp::InitFMOD()
{
	result = FMOD::System_Create(&system);
	ERRCHECK(result);

	result = system->getVersion(&version);
	ERRCHECK(result);

	if (version < FMOD_VERSION)
	{
		printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
		return;
	}

	int numdrivers = 0;
	char name[256];
	FMOD_CAPS caps;
	FMOD_SPEAKERMODE speakermode;

	result = system->getNumDrivers(&numdrivers);
	ERRCHECK(result);
	if (numdrivers == 0)
	{
		result = system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
		ERRCHECK(result);
	}
	else
	{
		result - system->getDriverCaps(0, &caps, 0, &speakermode);
		ERRCHECK(result);
		//set the user selected speaker mode.
		result = system->setSpeakerMode(speakermode);
		ERRCHECK(result);
		if (caps & FMOD_CAPS_HARDWARE_EMULATED)
		{
			/*
			The user has the 'Acceleration' slider set to off! This is really bad
			for latency! You might want to warn the user about this.
			*/
			result = system->setDSPBufferSize(1024, 10);
			ERRCHECK(result);
		}
		result = system->getDriverInfo(0, name, 256, 0);
		ERRCHECK(result);
		if (strstr(name, "SigmaTel"))
		{
			/*
			Sigmatel sound devices crackle for some reason if the format is PCM 16bit.
			PCM floating point output seems to solve it.
			*/
			result = system->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0,
				FMOD_DSP_RESAMPLER_LINEAR);
			ERRCHECK(result);
		}
	}
	result = system->init(100, FMOD_INIT_NORMAL, 0);
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)
	{
		/*
		Ok, the speaker mode selected isn't supported by this soundcard. Switch it
		back to stereo...
		*/
		result = system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
		ERRCHECK(result);
		/*
		... and re-init.
		*/
		result = system->init(100, FMOD_INIT_NORMAL, 0);
	}
	ERRCHECK(result);

	result = system->createStream("testMusic.mp3", FMOD_HARDWARE | FMOD_LOOP_NORMAL | FMOD_2D, 0, &music);

	ERRCHECK(result);

	result = system->playSound(FMOD_CHANNEL_FREE, music, false, &musicChannel);
	ERRCHECK(result);

	//set volume on the channel
	result = musicChannel->setVolume(0.05f);
	ERRCHECK(result);

	channel1 = 0;
	channel2 = 0;
	channel3 = 0;
}

void CrateApp::InitTextures()
{
	//Menu Textures
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/test.png", 0, 0, &mDiffuseMapSRVMenuButtons[0], 0)); //LOGO
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/test.png", 0, 0, &mDiffuseMapSRVMenuButtons[1], 0)); //PLAY
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/test.png", 0, 0, &mDiffuseMapSRVMenuButtons[2], 0)); //EASY
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/test.png", 0, 0, &mDiffuseMapSRVMenuButtons[3], 0)); //MEDIUM
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/test.png", 0, 0, &mDiffuseMapSRVMenuButtons[4], 0)); //HARD
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/test.png", 0, 0, &mDiffuseMapSRVMenuButtons[5], 0)); //EXIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/test.png", 0, 0, &mDiffuseMapSRVMenuButtons[6], 0)); //SOUND
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/test.png", 0, 0, &mDiffuseMapSRVMenuButtons[7], 0)); //MUSIC

	//Game Textures
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"Textures/MetalBox.jpg", 0, 0, &mDiffuseMapSRV, 0));

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"Textures/rock.dds", 0, 0, &mDiffuseMapSRV2, 0));

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"Textures/diamond.png", 0, 0, &mDiffuseMapSRV4, 0));

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"Textures/sand.jpg", 0, 0, &mDiffuseMapSRV5, 0));
}

bool CrateApp::Init()
{
	if(!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);

	InitFMOD();
	InitTextures();

	std::wstring filename = L"Textures/FireAnim/Fire";
	for (int i = 1; i < 121; i++)
	{
		std::wstringstream os;
		std::wstring intString;
		if (i < 10)
		{
			os << filename << L"00" << i << L".bmp";
		}
		else if (i < 100)
		{
			os << filename << L"0" << i << L".bmp";
		}
		else
		{
			os << filename << i << L".bmp";
		}
		intString = os.str();
		num = (LPCTSTR)intString.c_str();
		HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
			num, 0, 0, &mDiffuseMapSRV3[i-1], 0));
	}

 
	BuildGeometryBuffers();

	return true;
}

void CrateApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void CrateApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	//
	// Control the camera.
	//
	if (!menu) //disables camera control when viewing the menu
	{
		if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000)
		{
			//mCam.Walk(10.0f*dt);
			mCam.OrbitVertical(1 * dt);
		}
		if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			//mCam.Walk(-10.0f*dt);
			mCam.OrbitVertical(-1 * dt);
		}
		if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			//mCam.Strafe(-10.0f*dt);
			mCam.OrbitHorizontal(-1 * dt);
		}
		if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			//mCam.Strafe(10.0f*dt);
			mCam.OrbitHorizontal(1 * dt);
		}

		if (GetAsyncKeyState('Q') & 0x8000)
		{
			mCam.Walk(10.0f*dt);
		}
		if (GetAsyncKeyState('E') & 0x8000)
		{
			mCam.Walk(-10.0f*dt);
		}
	}

	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) //exits the game
	{
		PostQuitMessage(0);
	}
}

void CrateApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
	UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;
 
	//XMMATRIX view  = XMLoadFloat4x4(&mView);
	//XMMATRIX proj  = XMLoadFloat4x4(&mProj);
	//XMMATRIX viewProj = view*proj;

	mCam.UpdateViewMatrix();

	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();

	// Set per frame constants.
	if (!menu)
	{
		mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
		mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

		mDirLights[1].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		mDirLights[1].Diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
		mDirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
		mDirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);
		Effects::BasicFX->SetDirLights(mDirLights);
	}
	else
	{
		mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
		mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
		mDirLights[0].Direction = XMFLOAT3(0.0f, 0.0f, 0.7f);

		mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		mDirLights[1].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		mDirLights[1].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
		mDirLights[1].Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Effects::BasicFX->SetDirLights(mDirLights);
	}
	//Effects::BasicFX->SetEyePosW(mEyePosW);
	Effects::BasicFX->SetEyePosW(mCam.GetPosition());
 
	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexTech;

    D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		//animate fire
		timer++;
		if (timer > 30)
		{
			timer = 0;
			whichIMG++;
			if (whichIMG > 119)
			{
				whichIMG = 0;
			}
		}
		if (GetAsyncKeyState('1') & 0x8000)
			md3dImmediateContext->RSSetState(RenderStates::WireframeRS);

		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		// Draw the box.
		if (!menu)
		{
			for (int i = 0; i < cubes.size(); i++)
			{

				XMMATRIX world = /*XMLoadFloat4x4(&mBoxWorld);*/XMMatrixTranslationFromVector(cubes[i]->pos);
				XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
				XMMATRIX worldViewProj = world*view*proj;

				Effects::BasicFX->SetWorld(world);
				Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
				Effects::BasicFX->SetWorldViewProj(worldViewProj);
				Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
				Effects::BasicFX->SetMaterial(mBoxMat);
				//Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV);
				//Effects::BasicFX->SetDiffuseMap2(mDiffuseMapSRV2);
				switch (cubes[i]->texture) //show texture of cube
				{
				case 0:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV); //grass
					break;
				case 1:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV2); //stone
					break;
				case 2:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV3[whichIMG]); //fire
					break;
				case 3:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV4); //diamond
					break;
				case 4:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV5); //sand
					break;
				}

				activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
				md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);
				// Restore default
				md3dImmediateContext->RSSetState(0);
				if (mPickedTriangle != -1)
				{
					// Change depth test from < to <= so that if we draw the same triangle twice, it will still pass
					// the depth test.  This is because we redraw the picked triangle with a different material
					// to highlight it.  

					md3dImmediateContext->OMSetDepthStencilState(RenderStates::LessEqualDSS, 0);

					Effects::BasicFX->SetMaterial(mPickedTriangleMat);
					activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
					md3dImmediateContext->DrawIndexed(3, 3 * mPickedTriangle, 0);

					// restore default
					md3dImmediateContext->OMSetDepthStencilState(0, 0);
				}
			}
		}
		else
		{
			for (int i = 0; i < cubes.size(); i++)
			{

				XMMATRIX world = XMLoadFloat4x4(&cubes[i]->localWorld)/* * XMMatrixTranslationFromVector(cubes[i]->pos)*/;
				XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
				XMMATRIX worldViewProj = world*view*proj;

				Effects::BasicFX->SetWorld(world);
				Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
				Effects::BasicFX->SetWorldViewProj(worldViewProj);
				Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
				Effects::BasicFX->SetMaterial(mBoxMat);
				//Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV);
				//Effects::BasicFX->SetDiffuseMap2(mDiffuseMapSRV2);
				switch (cubes[i]->texture) //show texture of cube
				{
				case LOGO:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[0]);
					break;
				case PLAY:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[1]);
					break;
				case EASY:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[2]);
					break;
				case MEDIUM:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[3]);
					break;
				case HARD:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[4]);
					break;
				case EXIT:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[5]);
					break;
				case SOUND:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[6]);
					break;
				case MUSIC:
					Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[7]);
					break;
				}

				activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
				md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);
				// Restore default
				md3dImmediateContext->RSSetState(0);
				if (mPickedTriangle != -1)
				{
					// Change depth test from < to <= so that if we draw the same triangle twice, it will still pass
					// the depth test.  This is because we redraw the picked triangle with a different material
					// to highlight it.  

					md3dImmediateContext->OMSetDepthStencilState(RenderStates::LessEqualDSS, 0);

					Effects::BasicFX->SetMaterial(mPickedTriangleMat);
					activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
					md3dImmediateContext->DrawIndexed(3, 3 * mPickedTriangle, 0);

					// restore default
					md3dImmediateContext->OMSetDepthStencilState(0, 0);
				}
			}
		}
    }

	HR(mSwapChain->Present(0, 0));
}

void CrateApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	/*if ((btnState & MK_LBUTTON) != 0)
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;

		SetCapture(mhMainWnd);
	}*/
	if ((btnState & MK_LBUTTON) != 0)
	{
		Pick(x, y);
	}
	if (btnState == 16)
	{
		cubes.clear();
		hasDiamond = false;
		MakeLevel(levelWidth, levelLength, levelHeight);
	}
}

void CrateApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CrateApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	/*if ((btnState & MK_RBUTTON) != 0 && !menu)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;*/
}

void CrateApp::OnMouseWheelMove(WPARAM btnState,int fwKeys, int zDelta, int x, int y)
{
	if (!menu)
	{
		XMVECTOR distance = XMVector3Length(mCam.GetPositionXM() - XMVectorZero());
		float length = XMVectorGetByIndex(distance, 1);

		if (zDelta > 1 && length > 5) //mouse wheel up
		{
			mCam.Walk(1.0f);
		}
		else if (zDelta < 1 && length < 30) //mouse wheel down
		{
			mCam.Walk(-1.0f);
		}
	}
}

void CrateApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset      = 0;
	// Cache the index count of each object.
	mBoxIndexCount      = box.Indices.size();
	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset      = 0;
	UINT totalVertexCount = box.Vertices.size();
	UINT totalIndexCount = mBoxIndexCount;
	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//
	std::vector<Vertex::Basic32> vertices(totalVertexCount);
	UINT k = 0;
	for(size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos    = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex    = box.Vertices[i].TexC;
	}
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));
	//
	// Pack the indices of all the meshes into one index buffer.
	//
	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
	
}

void CrateApp::MakeLevel(UINT width, UINT length, UINT height)
{
	//UINT arraySize = width*length;
	//UINT blockData[arraySize];
	int x = 0; //width
	int y = 0; //length
	int z = 0; //height
	int tex = 0; //which texture max 4
	for (int i = 0; i < width*length*height; i++)
	{
		if (x == width)
		{
			z++;
			x = 0;
		}
		if (z == length)
		{
			y++;
			z = 0;
			x = 0;
		}
		Cube* c = new Cube;

		
		//c->texture = SetCubeTexture(i, x, y, z, width, length, height);
		if (c->texture == 5)
		{
			delete(c);
		}
		else
		{
			c->pos = XMVectorSet(x, y, z, 1);
			XMStoreFloat3(&c->mMeshBox.Center, c->pos);
			XMVECTOR halfSize = XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
			XMStoreFloat3(&c->mMeshBox.Extents, halfSize);

			CrateApp::cubes.push_back(c);
		}
		x++;
	}
	cubes[rand() % cubes.size()]->texture = 3;
	hasDiamond = true;
}

UINT CrateApp::SetCubeTexture(UINT cube, UINT x, UINT y, UINT z, UINT width, UINT height, UINT length)
{

	if (y == 0)
	{
		currentCubeLayer.push_back(0);
		return 1;
	}
	int random = rand()%6;
	switch (random)
	{
	case 0:
		return 0; //grass
	case 1:
		return 1; //stone
	case 2:
		return 2; //fire
	case 3:
		return 4; //sand
	default:
		return 5; // delete cube
	}
}

void CrateApp::Pick(int sx, int sy)
{
	XMMATRIX P = mCam.Proj();

	// Compute picking ray in view space.
	float vx = (+2.0f*sx / mClientWidth - 1.0f) / P(0, 0);
	float vy = (-2.0f*sy / mClientHeight + 1.0f) / P(1, 1);

	// Ray definition in view space.
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

	// Tranform ray to local space of Mesh.
	XMMATRIX V = mCam.View();
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

	//XMMATRIX W = XMMatrixTranslationFromVector(cubes[i]->pos);
	//XMMATRIX W = XMLoadFloat4x4(&mMeshWorld);
	//XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX toLocal = invView;// XMMatrixMultiply(invView, invWorld);

	rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
	rayDir = XMVector3TransformNormal(rayDir, toLocal);

	// If we hit the bounding box of the Mesh, then we might have picked a Mesh triangle,
	// so do the ray/triangle tests.
	//
	// If we did not hit the bounding box, then it is impossible that we hit 
	// the Mesh, so do not waste effort doing ray/triangle tests.

	// Assume we have not picked anything yet, so init to -1.
	mPickedTriangle = -1;
	float tmin = 0.0f;
	std::vector<float> cubesHit;
	std::vector<int> vectorPlace;

	for (int i = 0; i < CrateApp::cubes.size(); i++)
	{
		// Make the ray direction unit length for the intersection tests.
		rayDir = XMVector3Normalize(rayDir);
		if (XNA::IntersectRayAxisAlignedBox(rayOrigin, rayDir, &CrateApp::cubes[i]->mMeshBox /*&mMeshBox*/, &tmin))
		{
			XMVECTOR temp = XMVector3Length(cubes[i]->pos)/* - XMVector3Length(mCam.GetPositionXM())*/;
			cubesHit.push_back(XMVectorGetX(temp));
			vectorPlace.push_back(i);
//TODO check which box's position is closest to ray origion through magnitude of vector.
			
			//delete(cubes[i]);
			//CrateApp::cubes.erase(cubes.begin() + i);
			//break;
		}

	}
	
	if (!cubesHit.empty())
	{
		std::sort(cubesHit.begin(), cubesHit.end());
		
		for (int j = 0; j < vectorPlace.size(); j++)
		{
			if (AreSame(XMVectorGetX(XMVector3Length(cubes[vectorPlace[j]]->pos)),cubesHit[0]))
			{
				if (IsMenu)
				{
					CleanLevel();
					MakeLevel(3, 3, 3);
					menu = false;
				}
				else
				{
					if (cubes[vectorPlace[j]]->texture == 3)
					{
						MessageBox(0, L"You Found The Diamond!\n Press OK to play again.", L"Click Counter", MB_OK);
						cubes.clear();
						hasDiamond = false;
						MakeLevel(levelWidth, levelLength, levelHeight);
						break;
					}
					else if (cubes[vectorPlace[j]]->texture == 2)
					{
						//play sound!!
						cubes.clear();
						hasDiamond = false;
						MakeLevel(levelWidth, levelLength, levelHeight);
						break;
					}
					delete(cubes[vectorPlace[j]]);
					cubes.erase(cubes.begin() + vectorPlace[j]);
					break;
				}
			}
		}
		
		cubesHit.clear();
		vectorPlace.clear();
	}
	
}

bool CrateApp::AreSame(float a, float b)
{
	return fabs(a - b) < EPSILON;
}

void CrateApp::CreateMenu()
{
	// LOGO
	Cube * logoButton = new Cube; //creates new block
	logoButton->pos = XMVectorSet(0, 6, 5, 1); //set the position in world space for the cube
	XMMATRIX logoBoxScale = XMMatrixScaling(20.0f, 2.0f, 1.0f); //set the scale of the button
	XMStoreFloat4x4(&logoButton->localWorld, XMMatrixMultiply(logoBoxScale, XMMatrixTranslationFromVector(logoButton->pos)));
	XMStoreFloat3(&logoButton->mMeshBox.Center, logoButton->pos); //sets the center of the mesh box for click detection
	XMVECTOR logoHalfSize = XMVectorSet(10.0f, 1.0f, 0.5f, 1.0f); // sets the size of the bounding box from the center of the object
	XMStoreFloat3(&logoButton->mMeshBox.Extents, logoHalfSize);
	logoButton->texture = LOGO; //sets the texture of button; 
	logoButton->isMenu = true; //tells the game this is a menu block, not a game block. (wont be destroyed when clicked)
	CrateApp::cubes.push_back(logoButton); //adds the play button to the array of cubes to draw

	//PLAY BUTTON
	Cube * playButton = new Cube; //creates new block
	playButton->pos = XMVectorSet(0, -1, 5, 1); //set the position in world space for the cube
	XMMATRIX boxScale = XMMatrixScaling(10.0f, 2.0f, 1.0f); //set the scale of the button
	XMStoreFloat4x4(&playButton->localWorld, XMMatrixMultiply(boxScale, XMMatrixTranslationFromVector(playButton->pos)));
	XMStoreFloat3(&playButton->mMeshBox.Center, playButton->pos); //sets the center of the mesh box for click detection
	XMVECTOR halfSize = XMVectorSet(2.5f, 1.0f, 0.5f, 1.0f); // sets the size of the bounding box from the center of the object
	XMStoreFloat3(&playButton->mMeshBox.Extents, halfSize);
	playButton->texture = PLAY; //sets the texture of button; 
	playButton->isMenu = true; //tells the game this is a menu block, not a game block. (wont be destroyed when clicked)
	CrateApp::cubes.push_back(playButton); //adds the play button to the array of cubes to draw

	// EASY BUTTON
	Cube * easyButton = new Cube;
	easyButton->pos = XMVectorSet(-7, 3, 5, 1);
	XMMATRIX eboxScale = XMMatrixScaling(6.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&easyButton->localWorld, XMMatrixMultiply(eboxScale, XMMatrixTranslationFromVector(easyButton->pos)));
	XMStoreFloat3(&easyButton->mMeshBox.Center, easyButton->pos);
	XMVECTOR ehalfSize = XMVectorSet(1.5f, 1.5f, 1.5f, 1.5f);
	XMStoreFloat3(&easyButton->mMeshBox.Extents, ehalfSize);
	easyButton->texture = EASY;
	easyButton->isMenu = true;
	CrateApp::cubes.push_back(easyButton);

	//MEDIUM BUTTON
	Cube * midButton = new Cube;
	midButton->pos = XMVectorSet(0, 3, 5, 1);
	XMMATRIX midboxScale = XMMatrixScaling(6.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&midButton->localWorld, XMMatrixMultiply(midboxScale, XMMatrixTranslationFromVector(midButton->pos)));
	XMStoreFloat3(&midButton->mMeshBox.Center, midButton->pos);
	XMVECTOR midHalfSize = XMVectorSet(1.5f, 1.5f, 1.5f, 1.5f);
	XMStoreFloat3(&midButton->mMeshBox.Extents, midHalfSize);
	midButton->texture = MEDIUM;
	midButton->isMenu = true;
	CrateApp::cubes.push_back(midButton);

	//HARD BUTTON
	Cube * hardButton = new Cube;
	hardButton->pos = XMVectorSet(7, 3, 5, 1);
	XMMATRIX hBoxScale = XMMatrixScaling(6.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&hardButton->localWorld, XMMatrixMultiply(hBoxScale, XMMatrixTranslationFromVector(hardButton->pos)));
	XMStoreFloat3(&hardButton->mMeshBox.Center, hardButton->pos);
	XMVECTOR hardHalfSize = XMVectorSet(1.5f, 1.5f, 1.5f, 1.5f);
	XMStoreFloat3(&hardButton->mMeshBox.Extents, hardHalfSize);
	hardButton->texture = HARD;
	hardButton->isMenu = true;
	CrateApp::cubes.push_back(hardButton);

	//EXIT BUTTON
	Cube * exitButton = new Cube;
	exitButton->pos = XMVectorSet(9, -7, 5, 1);
	XMMATRIX exBoxScale = XMMatrixScaling(2.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&exitButton->localWorld, XMMatrixMultiply(exBoxScale, XMMatrixTranslationFromVector(exitButton->pos)));
	XMStoreFloat3(&exitButton->mMeshBox.Center, exitButton->pos);
	XMVECTOR exitHalfSize = XMVectorSet(1.5f, 1.5f, 1.5f, 1.5f);
	XMStoreFloat3(&exitButton->mMeshBox.Extents, exitHalfSize);
	exitButton->texture = EXIT;
	exitButton->isMenu = true;
	CrateApp::cubes.push_back(exitButton);

	//SOUND TOGGLE
	Cube * soundButton = new Cube;
	soundButton->pos = XMVectorSet(-8, -5, 5, 1);
	XMMATRIX sBoxScale = XMMatrixScaling(4.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&soundButton->localWorld, XMMatrixMultiply(sBoxScale, XMMatrixTranslationFromVector(soundButton->pos)));
	XMStoreFloat3(&soundButton->mMeshBox.Center, soundButton->pos);
	XMVECTOR sHalfSize = XMVectorSet(1.5f, 1.5f, 1.5f, 1.5f);
	XMStoreFloat3(&soundButton->mMeshBox.Extents, sHalfSize);
	soundButton->texture = SOUND;
	soundButton->isMenu = true;
	CrateApp::cubes.push_back(soundButton);

	//MUSIC TOGGLE
	Cube * musicButton = new Cube;
	musicButton->pos = XMVectorSet(-8, -7, 5, 1);
	XMMATRIX musicBoxScale = XMMatrixScaling(4.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&musicButton->localWorld, XMMatrixMultiply(musicBoxScale, XMMatrixTranslationFromVector(musicButton->pos)));
	XMStoreFloat3(&musicButton->mMeshBox.Center, musicButton->pos);
	XMVECTOR musicHalfSize = XMVectorSet(1.5f, 1.5f, 1.5f, 1.5f);
	XMStoreFloat3(&musicButton->mMeshBox.Extents, musicHalfSize);
	musicButton->texture = MUSIC;
	musicButton->isMenu = true;
	CrateApp::cubes.push_back(musicButton);
}

void CrateApp::CleanLevel()
{
	cubes.clear();
}


 
