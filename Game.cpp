/***************************************************************************************\
 GENERAL TODO:
	Need way to go back to main menu from game
	timer

 
 
 
 TODO SHAD: 
	-add new menu button texture images in the initTextures function.


 
	
      
 NOTES FOR ALEX:

	
\***************************************************************************************/

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "Sky.h"
#include "xnacollision.h"
#include "RenderStates.h"
#include "FileWriter.h"
#include <string.h>
#include <ostream>
#include <sstream>
#include <cstdlib>
#include <algorithm>

#include "fmod.hpp"
#include "fmod_errors.h"

//DEFINES
#define EPSILON 0.00001
//level sizes must be even numbers and less then 12
#define SML_LVL_SIZE 4
#define MED_LVL_SIZE 6
#define LRG_LVL_SIZE 10

#define SML_NUM_MINES 2
#define MED_NUM_MINES 28
#define LRG_NUM_MINES 100

#define SEED 2

struct Cube
{
	XMVECTOR pos;
	XMVECTOR originPos;
	XMVECTOR scale;
	bool flagged = false;
	enum cubeTextures {EMPTY,GRAY,MINE,ONE,TWO,THREE,FOUR,FIVE,SIX,FLAG};
	cubeTextures texture = GRAY;

	UINT menuTexture;
	XNA::AxisAlignedBox mMeshBox;
	bool isMenu = false;
	XMFLOAT4X4 localWorld;
	UINT uniqueID;
	float distanceFromCam;
};

class Game : public D3DApp
{
public:
	std::vector<Cube*> cubes;

	Game(HINSTANCE hInstance);
	~Game();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnMouseWheelMove(WPARAM btnState,int fwKeys, int zDelta, int x, int y);
	void MakeLevel(UINT width, UINT length, UINT height);
	void Pick(int sx, int sy, int button);
	void MenuLighting();
	void GameLighting();

private:
	void BuildGeometryBuffers();

	void InitFMOD();
	void UpdateSound();

private:
	float funcTimer = 10.0f;
	
	FileWriter highscoreFile;

	float smlHScore = 155.56467;
	float midHScore = 152.67979;
	float lrgHScore = 157.78789;

private:
	Sky* mSky;

	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3D11ShaderResourceView* mDiffuseMapSRVBoxTypes[8];
	//ID3D11ShaderResourceView* mDiffuseMapSRV2;
	//ID3D11ShaderResourceView* mDiffuseMapSRV3[120];
	//ID3D11ShaderResourceView* mDiffuseMapSRV4;
	//ID3D11ShaderResourceView* mDiffuseMapSRV5;
	ID3D11ShaderResourceView* mDiffuseMapSRVMenuButtons[25];
	enum menuButtons {LOGOb,PLAYb,EASYb,EASYbOn,MEDIUMb,MEDIUMbOn,HARDb,HARDbOn,EXITb,SOUNDb,SOUNDbOff,MUSICb,MUSICbOff,HIGHSCOREb,HSDIGITb0,HSDIGITb1,HSDIGITb2,HSDIGITb3,HSDIGITb4,HSDIGITb5,HSDIGITb6,HSDIGITb7,HSDIGITb8,HSDIGITb9,HSDOTb};
	menuButtons button;

	LPCTSTR num;

	// Lighting variables
	DirectionalLight mDirLights[3];

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
	float timer = 0;
	bool timerOn = false;
	int whichIMG = 0;

	// enum for difficulties
	enum difficulty
	{
		NONE, EASY, MEDIUM, HARD
	};
	// Curreny difficulty state
	difficulty diffState = NONE;

	//FMOD stuff
	FMOD::System *system;
	FMOD_RESULT result;
	FMOD::Sound      *sound1, *sound2, *sound3, *music;
	FMOD::Channel    *channel1, *channel2, *channel3, *channel4, *musicChannel;
	int               key;
	unsigned int      version;
	bool musicIsPlaying = true;
	bool soundIsPlaying = true;

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

	UINT levelWidth = 1;
	UINT levelLength = 1;
	UINT levelHeight = 1;
	bool AreSame(float a, float b); //checks if floats are the same to 5 decimal places
	bool AreSameVec(XMVECTOR a, XMVECTOR b);
	void CreateMenu();
	bool menu = true;
	void CleanLevel(); //cleans the level data before loading new level
	void InitTextures();
	void SetUpLevelData(int mines);
	bool isLevelSet = false;
	int CheckBlockSides(int placeInArray);
	std::vector<int> cubesChecked;
	
	XMVECTOR curPos;
	XMVECTOR PushBack = XMVectorSet(1.0f, 1.0f, 1.1f, 1.0f); // used for moving the buttons on press
	void IndentDiff(int index);

	static bool SortByVector(const Cube* lhs, const Cube* rhs) { return lhs->distanceFromCam < rhs->distanceFromCam; }
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

	Game theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}
 

Game::Game(HINSTANCE hInstance)
: D3DApp(hInstance), mSky(0), mBoxVB(0), mBoxIB(0), mEyePosW(0.0f, 0.0f, 0.0f), mTheta(1.3f*MathHelper::Pi), mPhi(0.4f*MathHelper::Pi), mRadius(2.5f), mCam(), mMeshIndexCount(0), mPickedTriangle(-1),
highscoreFile("Highscores.hst")
{
	//mDiffuseMapSRV3[0] = 0;
	mMainWndCaption = L"3D Minesweeper";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mBoxWorld, I);
	XMStoreFloat4x4(&mTexTransform, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	mBoxMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	mBoxMat.Reflect	 = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	mPickedTriangleMat.Ambient	= XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mPickedTriangleMat.Diffuse	= XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mPickedTriangleMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	mPickedTriangleMat.Reflect	= XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	
	srand(SEED);//time(NULL));
	//MakeLevel(5, 5, 5); //makes the cube of blocks.
	

	//-------------------
	
	
	//XMMATRIX MeshScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	//XMMATRIX MeshOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	//XMStoreFloat4x4(&mMeshWorld, XMMatrixMultiply(MeshScale, MeshOffset));
	//XMStoreFloat4x4(&mMeshWorld, XMMatrixTranslation(levelWidth*0.5f, levelLength * 0.5f, levelHeight * 0.5f));
	mCam.SetPosition(0.0f, 0.0f, -15.0f);

	int timer = 0;

	std::vector<std::string> smlScore = highscoreFile.ReadData("smlScore");
	int jam = 0;
	
}

Game::~Game()
{
	SafeDelete(mSky);

	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	//ReleaseCOM(mDiffuseMapSRV);
	//ReleaseCOM(mDiffuseMapSRV2);
	//ReleaseCOM(mDiffuseMapSRV4);
	//ReleaseCOM(mDiffuseMapSRV5);
	/*for (int i = 0; i < 120; i++)
	{
		ReleaseCOM(mDiffuseMapSRV3[i]);
	}*/
	

	Effects::DestroyAll();
	InputLayouts::DestroyAll();

	//Clean up FMOD
	result = sound1->release();
	ERRCHECK(result);

	result = sound2->release();
	ERRCHECK(result);

	result = sound3->release();
	ERRCHECK(result);

	result = music->release();
	ERRCHECK(result);

	result = system->close();
	ERRCHECK(result);

	result = system->release();
	ERRCHECK(result);
	
}

void Game::InitFMOD()
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

	result = system->createSound("flag.mp3", FMOD_HARDWARE, 0, &sound1);
	ERRCHECK(result);

	result = system->createSound("victory.mp3", FMOD_HARDWARE, 0, &sound2);
	ERRCHECK(result);

	result = system->createSound("boom.mp3", FMOD_HARDWARE, 0, &sound3);
	ERRCHECK(result);

	result = system->createStream("music.mp3", FMOD_HARDWARE | FMOD_LOOP_NORMAL | FMOD_2D, 0, &music);
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

void Game::InitTextures()
{
	// Skybox
	mSky = new Sky(md3dDevice, L"Textures/underwater.dds", 5000.0f);

	//Menu Textures
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/logo2.png", 0, 0, &mDiffuseMapSRVMenuButtons[0], 0)); //LOGO
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/play.png", 0, 0, &mDiffuseMapSRVMenuButtons[1], 0)); //PLAY
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/easypicked.png", 0, 0, &mDiffuseMapSRVMenuButtons[2], 0)); //EASY
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/easyselected.png", 0, 0, &mDiffuseMapSRVMenuButtons[3], 0)); //EASY
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/mediumnot.png", 0, 0, &mDiffuseMapSRVMenuButtons[4], 0)); //MEDIUM
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/mediumselected.png", 0, 0, &mDiffuseMapSRVMenuButtons[5], 0)); //MEDIUM
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/hardnot.png", 0, 0, &mDiffuseMapSRVMenuButtons[6], 0)); //HARD
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/hardselected.png", 0, 0, &mDiffuseMapSRVMenuButtons[7], 0)); //HARD
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/exit.png", 0, 0, &mDiffuseMapSRVMenuButtons[8], 0)); //EXIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/soundfxon.png", 0, 0, &mDiffuseMapSRVMenuButtons[9], 0)); //SOUND
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/soundfxoff.png", 0, 0, &mDiffuseMapSRVMenuButtons[10], 0)); //SOUND
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/musicon.png", 0, 0, &mDiffuseMapSRVMenuButtons[11], 0)); //MUSIC
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/musicoff.png", 0, 0, &mDiffuseMapSRVMenuButtons[12], 0)); //MUSIC
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/highscore.png", 0, 0, &mDiffuseMapSRVMenuButtons[13], 0)); //HIGHSCORE
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/HS#0.png", 0, 0, &mDiffuseMapSRVMenuButtons[14], 0)); //HIGHSCORE DIGIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/HS#1.png", 0, 0, &mDiffuseMapSRVMenuButtons[15], 0)); //HIGHSCORE DIGIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/HS#2.png", 0, 0, &mDiffuseMapSRVMenuButtons[16], 0)); //HIGHSCORE DIGIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/HS#3.png", 0, 0, &mDiffuseMapSRVMenuButtons[17], 0)); //HIGHSCORE DIGIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/HS#4.png", 0, 0, &mDiffuseMapSRVMenuButtons[18], 0)); //HIGHSCORE DIGIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/HS#5.png", 0, 0, &mDiffuseMapSRVMenuButtons[19], 0)); //HIGHSCORE DIGIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/HS#6.png", 0, 0, &mDiffuseMapSRVMenuButtons[20], 0)); //HIGHSCORE DIGIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/HS#7.png", 0, 0, &mDiffuseMapSRVMenuButtons[21], 0)); //HIGHSCORE DIGIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/HS#8.png", 0, 0, &mDiffuseMapSRVMenuButtons[22], 0)); //HIGHSCORE DIGIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/HS#9.png", 0, 0, &mDiffuseMapSRVMenuButtons[23], 0)); //HIGHSCORE DIGIT
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/game pics/HSDOT.png", 0, 0, &mDiffuseMapSRVMenuButtons[24], 0)); //HIGHSCORE DIGIT


	//Game Textures
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/MetalBox.jpg", 0, 0, &mDiffuseMapSRVBoxTypes[0], 0));
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/diamond.png", 0, 0, &mDiffuseMapSRVBoxTypes[1], 0));
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/one.png", 0, 0, &mDiffuseMapSRVBoxTypes[2], 0));
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/two.png", 0, 0, &mDiffuseMapSRVBoxTypes[3], 0));
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/three.png", 0, 0, &mDiffuseMapSRVBoxTypes[4], 0));
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/four.png", 0, 0, &mDiffuseMapSRVBoxTypes[5], 0));
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/five.png", 0, 0, &mDiffuseMapSRVBoxTypes[6], 0));
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/flag.png", 0, 0, &mDiffuseMapSRVBoxTypes[7], 0));
}

bool Game::Init()
{
	if(!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);

	InitFMOD();
	InitTextures();
	CreateMenu();

	/*std::wstring filename = L"Textures/FireAnim/Fire";
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
		HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,num, 0, 0, &mDiffuseMapSRV3[i-1], 0));
	}*/
	BuildGeometryBuffers();
	return true;
}

void Game::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 6.0f, 1000.0f);
}

void Game::UpdateScene(float dt)
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
			mCam.OrbitVertical(2 * dt);
			//mCam.Pitch(2 * dt);
		}
		if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			//mCam.Walk(-10.0f*dt);
			mCam.OrbitVertical(-2 * dt);
			//mCam.Pitch(-2 * dt);
		}
		if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			//mCam.Strafe(-10.0f*dt);
			mCam.OrbitHorizontal(-2 * dt);
			//mCam.RotateY(-2*dt);
		}
		if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			//mCam.Strafe(10.0f*dt);
			mCam.OrbitHorizontal(2 * dt);
			//mCam.RotateY(2 * dt);
		}
		//Zoom controls
		if (GetAsyncKeyState('Q') & 0x8000)
		{
			XMVECTOR distance = XMVector3Length(mCam.GetPositionXM() - XMVectorZero());
			float length = XMVectorGetByIndex(distance, 1);
			if (length > 5) 
			{
				mCam.Walk(10*dt);
			}
		}
		if (GetAsyncKeyState('E') & 0x8000)
		{
			XMVECTOR distance = XMVector3Length(mCam.GetPositionXM() - XMVectorZero());
			float length = XMVectorGetByIndex(distance, 1);
			if (length < 30)
			{
				mCam.Walk(-10*dt);
			}
		}
	}
	//mCam.SetPosition(mCam.GetLook().x*1, mCam.GetLook().y*1, mCam.GetLook().z*-18);
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) //exits the game
	{
		PostQuitMessage(0);
	}

	//game timer
	if (timerOn)
	{
		timer += dt;
	}


}

void Game::DrawScene()
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
		GameLighting();
	}
	else
	{
		MenuLighting();
	}
	//Effects::BasicFX->SetEyePosW(mEyePosW);
	Effects::BasicFX->SetEyePosW(mCam.GetPosition());
	//Set Cubemap
	Effects::BasicFX->SetCubeMap(mSky->CubeMapSRV());

	ID3DX11EffectTechnique* activeTexTech = Effects::BasicFX->Light3TexAlphaClipTech;//Effects::BasicFX->Light3TexTech;

    D3DX11_TECHNIQUE_DESC techDesc;
	activeTexTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		if (GetAsyncKeyState('1') & 0x8000)md3dImmediateContext->RSSetState(RenderStates::WireframeRS);

		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		// Draw the box.
		if (!menu)
		{
			for (int i = 0; i < cubes.size(); i++)
			{
				if (cubes[i] != NULL)
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
					if (cubes[i]->flagged == true)
					{
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVBoxTypes[7]); //flag
					}
					else
					{
						switch (cubes[i]->texture) //show texture of cube
						{
						case Cube::GRAY:
							Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVBoxTypes[0]); //metel box
							break;
						case Cube::MINE:
							Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVBoxTypes[0]); //diamond
							break;
						case Cube::ONE:
							Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVBoxTypes[2]); //one
							break;
						case Cube::TWO:
							Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVBoxTypes[3]); //two
							break;
						case Cube::THREE:
							Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVBoxTypes[4]); //three
							break;
						case Cube::FOUR:
							Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVBoxTypes[5]); //four
							break;
						case Cube::FIVE:
							Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVBoxTypes[6]); //five
							break;
						case Cube::FLAG:
							Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVBoxTypes[7]); //flag
							break;
							/*case 0:
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
								break;*/
						}
					}
					
					activeTexTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
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
						activeTexTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
						md3dImmediateContext->DrawIndexed(3, 3 * mPickedTriangle, 0);

						// restore default
						md3dImmediateContext->OMSetDepthStencilState(0, 0);
					}
				}
			}
		}
		else //IF MENU
		{
			for (int i = 0; i < cubes.size(); i++)
			{
				if (cubes[i] != NULL)
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
					switch (cubes[i]->menuTexture) //show texture of cube
					{
					case LOGOb:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[0]);
						break;
					case PLAYb:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[1]);
						break;
					case EASYb:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[2]);
						break;
					case EASYbOn:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[3]);
						break;
					case MEDIUMb:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[4]);
						break;
					case MEDIUMbOn:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[5]);
						break;
					case HARDb:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[6]);
						break;
					case HARDbOn:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[7]);
						break;
					case EXITb:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[8]);
						break;
					case SOUNDb:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[9]);
						break;
					case SOUNDbOff:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[10]);
						break;
					case MUSICb:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[11]);
						break;
					case MUSICbOff:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[12]);
						break;
					case HIGHSCOREb:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[13]);
						break;
					case HSDIGITb0:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[14]);
						break;
					case HSDIGITb1:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[15]);
						break;
					case HSDIGITb2:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[16]);
						break;
					case HSDIGITb3:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[17]);
						break;
					case HSDIGITb4:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[18]);
						break;
					case HSDIGITb5:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[19]);
						break;
					case HSDIGITb6:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[20]);
						break;
					case HSDIGITb7:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[21]);
						break;
					case HSDIGITb8:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[22]);
						break;
					case HSDIGITb9:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[23]);
						break;
					case HSDOTb:
						Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRVMenuButtons[24]);
						break;
					}
					activeTexTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
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
						activeTexTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
						md3dImmediateContext->DrawIndexed(3, 3 * mPickedTriangle, 0);

						// restore default
						md3dImmediateContext->OMSetDepthStencilState(0, 0);
					}
				}
			}
		}
		//Draw Cubemap
		mSky->Draw(md3dImmediateContext, mCam);
    }

	HR(mSwapChain->Present(0, 0));
}

void Game::OnMouseDown(WPARAM btnState, int x, int y)
{
	/*if ((btnState & MK_LBUTTON) != 0)
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;

		SetCapture(mhMainWnd);
	}*/
	if ((btnState & MK_LBUTTON) != 0)
	{
		Pick(x, y, MK_LBUTTON);
	}
	if ((btnState & MK_RBUTTON) != 0)
	{
		Pick(x, y, MK_RBUTTON);
	}

}

void Game::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Game::OnMouseMove(WPARAM btnState, int x, int y)
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

void Game::OnMouseWheelMove(WPARAM btnState,int fwKeys, int zDelta, int x, int y)
{
	if (!menu)
	{
		XMVECTOR distance = XMVector3Length(mCam.GetPositionXM() - XMVectorZero());
		float length = XMVectorGetByIndex(distance, 1);

		if (zDelta > 1 && length > 5) //mouse wheel up
		{
			mCam.Walk(0.5f);
		}
		else if (zDelta < 1 && length < 30) //mouse wheel down
		{
			mCam.Walk(-0.5f);
		}
	}
}

void Game::BuildGeometryBuffers()
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

void Game::MakeLevel(UINT width, UINT length, UINT height)
{
	CleanLevel();
	levelWidth = width;
	levelLength = length;
	levelHeight = height;
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
		/*if (c->texture == Cube::EMPTY)
		{
			delete(c);
		}
		else
		{*/
			c->texture = Cube::GRAY;
			c->pos = XMVectorSet(x-(width*0.5f)+0.5f, y-(height*0.5f)+0.5f, z-(length*0.5f)+0.5f, 1);
			c->uniqueID = i;
			XMStoreFloat3(&c->mMeshBox.Center, c->pos);
			XMVECTOR halfSize = XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
			XMStoreFloat3(&c->mMeshBox.Extents, halfSize);
			cubes.push_back(c);
		//}
		x++;
	}
	//cubes[rand() % cubes.size()]->texture = Cube::MINE;
	//hasDiamond = true;
}

UINT Game::SetCubeTexture(UINT cube, UINT x, UINT y, UINT z, UINT width, UINT height, UINT length)
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

bool Game::AreSame(float a, float b)
{
	return fabs(a - b) < EPSILON;
}

bool Game::AreSameVec(XMVECTOR a, XMVECTOR b)
{
	if (XMVectorGetIntX(a) - XMVectorGetIntX(b) < EPSILON &&
		XMVectorGetIntY(a) - XMVectorGetIntY(b) < EPSILON &&
		XMVectorGetIntZ(a) - XMVectorGetIntZ(b) < EPSILON)
	{
		return true;
	}
	else
		return false;
}

void Game::Pick(int sx, int sy, int button)
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

	mPickedTriangle = -1;// Assume we have not picked anything yet, so init to -1.
	float tmin = 0.0f;
	std::vector<Cube*> cubesTouched;

	// Make the ray direction unit length for the intersection tests.
	rayDir = XMVector3Normalize(rayDir);
	for (int i = 0; i < cubes.size(); i++)
	{
		if (cubes[i] != NULL)
		{
			// Make the ray direction unit length for the intersection tests.
			rayDir = XMVector3Normalize(rayDir);
			if (XNA::IntersectRayAxisAlignedBox(rayOrigin, rayDir, &Game::cubes[i]->mMeshBox, &tmin))
			{
				XMVECTOR temp = XMVector3Length(mCam.GetPositionXM() - cubes[i]->pos);
				cubes[i]->distanceFromCam = XMVectorGetIntX(temp);
				cubesTouched.push_back(cubes[i]);
			}
		}
	}
	
	if (menu)
	{
		if (!cubesTouched.empty())
		{
			for (int i = 0; i < cubesTouched.size(); i++)
			{
				if (AreSameVec(XMVector3Length(mCam.GetPositionXM() - cubesTouched[i]->pos), XMVector3Length(mCam.GetPositionXM() - cubesTouched[i]->pos)))
				{
					switch (cubesTouched[i]->menuTexture)
					{
					case LOGOb:

						break;
					case PLAYb:
						switch (diffState)
						{
						case EASY:
							MakeLevel(SML_LVL_SIZE, SML_LVL_SIZE, SML_LVL_SIZE);
							menu = false;
							break;
						case MEDIUM:
							MakeLevel(MED_LVL_SIZE, MED_LVL_SIZE, MED_LVL_SIZE);
							menu = false;
							break;
						case HARD:
							MakeLevel(LRG_LVL_SIZE, LRG_LVL_SIZE, LRG_LVL_SIZE);
							menu = false;
							break;
						case NONE:
							MessageBox(0, L"Please Choose Your Difficulty and Try Again", L"Error", MB_OK);
							break;
						}

						break;
					case EASYb:
						diffState = EASY;
						IndentDiff(2);
						cubes[2]->menuTexture = EASYbOn;
						break;
					case MEDIUMb:
						diffState = MEDIUM;
						IndentDiff(3);
						cubes[3]->menuTexture = MEDIUMbOn;
						break;
					case HARDb:
						diffState = HARD;
						IndentDiff(4);
						cubes[4]->menuTexture = HARDbOn;
						break;
					case EXITb:
						PostQuitMessage(0);
						break;
					case SOUNDb:
						soundIsPlaying = !soundIsPlaying;
						channel1->setMute(soundIsPlaying);
						channel2->setMute(soundIsPlaying);
						channel3->setMute(soundIsPlaying);
						channel4->setMute(soundIsPlaying);
						break;
					case MUSICb:
						musicIsPlaying = !musicIsPlaying;
						musicChannel->setMute(musicIsPlaying);
						break;
					}
				}
			}
		}
	}
	else //in game
	{
		if (!cubesTouched.empty())
		{
			std::sort(cubesTouched.begin(), cubesTouched.end(), SortByVector);
			int size = cubes.size();
			int place = cubesTouched[0]->uniqueID;
			//turn in the timer
			timerOn = true;

			if (button == MK_LBUTTON && !cubes[place]->flagged)
			{
				switch (cubes[place]->texture)
				{
				case Cube::GRAY:
					switch (levelHeight)
					{
					case SML_LVL_SIZE:
						SetUpLevelData(SML_NUM_MINES);
						break;
					case MED_LVL_SIZE:
						SetUpLevelData(MED_NUM_MINES);
						break;
					case LRG_LVL_SIZE:
						SetUpLevelData(LRG_NUM_MINES);
						break;
					}
					CheckBlockSides(place);
					cubesChecked.clear();
					//delete(cubes[place]);
					//cubes[place] = NULL;
					//cubes.erase(cubes.begin() + place);
					break;
				case Cube::MINE:
					result = system->playSound(FMOD_CHANNEL_FREE, sound3, false, &channel4);
					ERRCHECK(result);
					CleanLevel();
					MakeLevel(levelWidth, levelHeight, levelLength);
					break;
				}
			}
			else if (button == MK_RBUTTON)//swap is flagged state
			{
				cubes[place]->flagged = !cubes[place]->flagged;
				result = system->playSound(FMOD_CHANNEL_FREE, sound1, false, &channel2);
				ERRCHECK(result);
			}
		}
		//win check
		int minesFlagged = 0;
		int grays = 0;
		for (int i = 0; i < cubes.size(); i++)
		{
			if (cubes[i] != NULL)
			{
				if (cubes[i]->texture == Cube::GRAY)
				{
					grays++;
				}
				if (cubes[i]->flagged == true && cubes[i]->texture == Cube::MINE)
				{

					minesFlagged++;
				}
			}
		}
		switch (levelHeight)
		{
		case SML_LVL_SIZE:
			if (minesFlagged == SML_NUM_MINES && grays == 0)
			{
				result = system->playSound(FMOD_CHANNEL_FREE, sound2, false, &channel3);
				ERRCHECK(result);
				std::wstringstream out;
				out << L"You Win in ";
				out << timer;
				out << "! The game will no reset.";
				MessageBox(0, out.str().c_str(), L"Congratulations", MB_OK);
				
				CleanLevel();
				timer = 0;
				timerOn = false;
				MakeLevel(levelWidth, levelHeight, levelLength);
				
			}
			break;
		case MED_LVL_SIZE:
			if (minesFlagged == MED_NUM_MINES && grays == 0)
			{
				result = system->playSound(FMOD_CHANNEL_FREE, sound2, false, &channel3);
				ERRCHECK(result);
				std::wstringstream out;
				out << L"You Win in ";
				out << timer;
				out << "! The game will no reset.";
				MessageBox(0, out.str().c_str(), L"Congratulations", MB_OK);
				
				CleanLevel();
				timer = 0;
				timerOn = false;
				MakeLevel(levelWidth, levelHeight, levelLength);

			}
			break;
		case LRG_LVL_SIZE:
			if (minesFlagged == LRG_NUM_MINES && grays == 0)
			{
				result = system->playSound(FMOD_CHANNEL_FREE, sound2, false, &channel3);
				ERRCHECK(result);
				std::wstringstream out;
				out << L"You Win in ";
				out << timer;
				out << "! The game will no reset.";
				MessageBox(0, out.str().c_str(), L"Congratulations", MB_OK);
				
				CleanLevel();
				timer = 0;
				timerOn = false;
				MakeLevel(levelWidth, levelHeight, levelLength);
			}
			break;
		}

	}
}

void Game::CreateMenu()
{
	// LOGO
	Cube * logoButton = new Cube; //creates new block
	logoButton->pos = XMVectorSet(0, 6, 5, 1); //set the position in world space for the cube
	logoButton->originPos = XMVectorSet(0, 6, 5, 1); //set its origin pos for button presses
	logoButton->scale = XMVectorSet(20.0f, 2.0f, 1.0f, 1.0f); //set the scale of the button
	XMStoreFloat4x4(&logoButton->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(logoButton->scale), XMMatrixTranslationFromVector(logoButton->pos)));
	XMStoreFloat3(&logoButton->mMeshBox.Center, logoButton->pos); //sets the center of the mesh box for click detection
	XMVECTOR logoHalfSize = XMVectorSet(10.0f, 1.0f, 0.5f, 1.0f); // sets the size of the bounding box from the center of the object
	XMStoreFloat3(&logoButton->mMeshBox.Extents, logoHalfSize);
	logoButton->menuTexture = LOGOb; //sets the texture of button; 
	logoButton->isMenu = true; //tells the game this is a menu block, not a game block. (wont be destroyed when clicked)
	Game::cubes.push_back(logoButton); //adds the play button to the array of cubes to draw

	//PLAY BUTTON
	Cube * playButton = new Cube;
	playButton->pos = XMVectorSet(0, -6, 5, 1);
	playButton->originPos = XMVectorSet(0, -6, 5, 1);
	playButton->scale = XMVectorSet(10.0f, 2.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&playButton->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(playButton->scale), XMMatrixTranslationFromVector(playButton->pos)));
	XMStoreFloat3(&playButton->mMeshBox.Center, playButton->pos);
	XMVECTOR halfSize = XMVectorSet(5.0f, 1.0f, 0.5f, 1.0f);
	XMStoreFloat3(&playButton->mMeshBox.Extents, halfSize);
	playButton->menuTexture = PLAYb; 
	playButton->isMenu = true;
	Game::cubes.push_back(playButton); 

	//highScore
	Cube * highScore = new Cube;
	highScore->pos = XMVectorSet(0, 2, 5, 1);
	highScore->originPos = XMVectorSet(0, 2, 5, 1);
	highScore->scale = XMVectorSet(10.0f, 0.5f, 1.0f, 1.0f);
	XMStoreFloat4x4(&highScore->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(highScore->scale), XMMatrixTranslationFromVector(highScore->pos)));
	XMStoreFloat3(&highScore->mMeshBox.Center, highScore->pos);
	XMVECTOR HShalfSize = XMVectorSet(5.0f, 0.25f, 0.5f, 1.0f);
	XMStoreFloat3(&highScore->mMeshBox.Extents, HShalfSize);
	highScore->menuTexture = HIGHSCOREb;
	highScore->isMenu = true;
	Game::cubes.push_back(highScore);

	//SINGLE DIGIT SCORE BOX
	for (int i = 0; i < 5; i++)
	{
		std::stringstream toDraw;
		toDraw << smlHScore;
		int numToDraw = toDraw.str()[i] + 14 - 48;
		if (toDraw.str()[i] == 46)
			numToDraw = HSDOTb;

		Cube * scoreDigit = new Cube;
		scoreDigit->pos = XMVectorSet(-10+i*1.1, -1, 5, 1);
		scoreDigit->originPos = XMVectorSet(0, -1, 5, 1);
		scoreDigit->scale = XMVectorSet(1.0f, 1.2f, 1.0f, 1.0f);
		XMStoreFloat4x4(&scoreDigit->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(scoreDigit->scale), XMMatrixTranslationFromVector(scoreDigit->pos)));
		XMStoreFloat3(&scoreDigit->mMeshBox.Center, scoreDigit->pos);
		XMVECTOR SDhalfSize = XMVectorSet(5.0f, 1.0f, 0.5f, 1.0f);
		XMStoreFloat3(&scoreDigit->mMeshBox.Extents, SDhalfSize);
		scoreDigit->menuTexture = numToDraw;
		scoreDigit->isMenu = true;
		Game::cubes.push_back(scoreDigit);
	}

	//SINGLE DIGIT SCORE BOX
	for (int i = 0; i < 5; i++)
	{
		std::stringstream toDraw;
		toDraw << midHScore;//change this to the float you want to draw
		int numToDraw = toDraw.str()[i] + 14 - 48;
		if (toDraw.str()[i] == 46)
		numToDraw = HSDOTb;

		Cube * scoreDigit = new Cube;
		scoreDigit->pos = XMVectorSet(-2 + i*1.1, -1, 5, 1);
		scoreDigit->originPos = XMVectorSet(0, -1, 5, 1);
		scoreDigit->scale = XMVectorSet(1.0f, 1.2f, 1.0f, 1.0f);
		XMStoreFloat4x4(&scoreDigit->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(scoreDigit->scale), XMMatrixTranslationFromVector(scoreDigit->pos)));
		XMStoreFloat3(&scoreDigit->mMeshBox.Center, scoreDigit->pos);
		XMVECTOR SDhalfSize = XMVectorSet(5.0f, 1.0f, 0.5f, 1.0f);
		XMStoreFloat3(&scoreDigit->mMeshBox.Extents, SDhalfSize);
		scoreDigit->menuTexture = numToDraw;
		scoreDigit->isMenu = true;
		Game::cubes.push_back(scoreDigit);
	}

	//SINGLE DIGIT SCORE BOX
	for (int i = 0; i < 5; i++)
	{
		std::stringstream toDraw;
		toDraw << lrgHScore;//change this to the float you want to draw
		int numToDraw = toDraw.str()[i] + 14 - 48;
		if (toDraw.str()[i] == 46)
		numToDraw = HSDOTb;

		Cube * scoreDigit = new Cube;
		scoreDigit->pos = XMVectorSet(5 + i*1.1, -1, 5, 1);
		scoreDigit->originPos = XMVectorSet(0, -1, 5, 1);
		scoreDigit->scale = XMVectorSet(1.0f, 1.2f, 1.0f, 1.0f);
		XMStoreFloat4x4(&scoreDigit->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(scoreDigit->scale), XMMatrixTranslationFromVector(scoreDigit->pos)));
		XMStoreFloat3(&scoreDigit->mMeshBox.Center, scoreDigit->pos);
		XMVECTOR SDhalfSize = XMVectorSet(5.0f, 1.0f, 0.5f, 1.0f);
		XMStoreFloat3(&scoreDigit->mMeshBox.Extents, SDhalfSize);
		scoreDigit->menuTexture = numToDraw;
		scoreDigit->isMenu = true;
		Game::cubes.push_back(scoreDigit);
	}
	
	// EASY BUTTON
	Cube * easyButton = new Cube;
	easyButton->pos = XMVectorSet(-7, 3, 5, 1);
	easyButton->originPos = XMVectorSet(-7, 3, 5, 1);
	easyButton->scale = XMVectorSet(6.0f, 1.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&easyButton->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(easyButton->scale), XMMatrixTranslationFromVector(easyButton->pos)));
	XMStoreFloat3(&easyButton->mMeshBox.Center, easyButton->pos);
	XMVECTOR ehalfSize = XMVectorSet(3.0f, 0.5f, 0.5f, 1.5f);
	XMStoreFloat3(&easyButton->mMeshBox.Extents, ehalfSize);
	easyButton->menuTexture = EASYb;
	easyButton->isMenu = true;
	Game::cubes.push_back(easyButton);

	//MEDIUM BUTTON
	Cube * midButton = new Cube;
	midButton->pos = XMVectorSet(0, 3, 5, 1);
	midButton->originPos = XMVectorSet(0, 3, 5, 1);
	midButton->scale = XMVectorSet(6.0f, 1.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&midButton->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(midButton->scale), XMMatrixTranslationFromVector(midButton->pos)));
	XMStoreFloat3(&midButton->mMeshBox.Center, midButton->pos);
	XMVECTOR midHalfSize = XMVectorSet(3.0f, 0.5f, 0.5f, 1.5f);
	XMStoreFloat3(&midButton->mMeshBox.Extents, midHalfSize);
	midButton->menuTexture = MEDIUMb;
	midButton->isMenu = true;
	Game::cubes.push_back(midButton);

	//HARD BUTTON
	Cube * hardButton = new Cube;
	hardButton->pos = XMVectorSet(7, 3, 5, 1);
	hardButton->originPos = XMVectorSet(7, 3, 5, 1);
	hardButton->scale = XMVectorSet(6.0f, 1.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&hardButton->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(hardButton->scale), XMMatrixTranslationFromVector(hardButton->pos)));
	XMStoreFloat3(&hardButton->mMeshBox.Center, hardButton->pos);
	XMVECTOR hardHalfSize = XMVectorSet(3.0f, 0.5f, 0.5f, 1.0f);
	XMStoreFloat3(&hardButton->mMeshBox.Extents, hardHalfSize);
	hardButton->menuTexture = HARDb;
	hardButton->isMenu = true;
	Game::cubes.push_back(hardButton);

	//EXIT BUTTON
	Cube * exitButton = new Cube;
	exitButton->pos = XMVectorSet(9, -7, 5, 1);
	exitButton->originPos = XMVectorSet(9, -7, 5, 1);
	exitButton->scale = XMVectorSet(2.0f, 1.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&exitButton->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(exitButton->scale), XMMatrixTranslationFromVector(exitButton->pos)));
	XMStoreFloat3(&exitButton->mMeshBox.Center, exitButton->pos);
	XMVECTOR exitHalfSize = XMVectorSet(1.0f, 0.5f, 0.5f, 1.0f);
	XMStoreFloat3(&exitButton->mMeshBox.Extents, exitHalfSize);
	exitButton->menuTexture = EXITb;
	exitButton->isMenu = true;
	Game::cubes.push_back(exitButton);

	//SOUND TOGGLE
	Cube * soundButton = new Cube;
	soundButton->pos = XMVectorSet(-8, -5, 5, 1);
	soundButton->originPos = XMVectorSet(-8, -5, 5, 1);
	soundButton->scale = XMVectorSet(4.0f, 1.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&soundButton->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(soundButton->scale), XMMatrixTranslationFromVector(soundButton->pos)));
	XMStoreFloat3(&soundButton->mMeshBox.Center, soundButton->pos);
	XMVECTOR sHalfSize = XMVectorSet(2.0f, 0.5f, 0.5f, 1.0f);
	XMStoreFloat3(&soundButton->mMeshBox.Extents, sHalfSize);
	soundButton->menuTexture = SOUNDb;
	soundButton->isMenu = true;
	Game::cubes.push_back(soundButton);

	//MUSIC TOGGLE
	Cube * musicButton = new Cube;
	musicButton->pos = XMVectorSet(-8, -7, 5, 1);
	musicButton->originPos = XMVectorSet(-8, -7, 5, 1);
	musicButton->scale = XMVectorSet(4.0f, 1.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&musicButton->localWorld, XMMatrixMultiply(XMMatrixScalingFromVector(musicButton->scale), XMMatrixTranslationFromVector(musicButton->pos)));
	XMStoreFloat3(&musicButton->mMeshBox.Center, musicButton->pos);
	XMVECTOR musicHalfSize = XMVectorSet(2.0f, 0.5f, 0.5f, 1.0f);
	XMStoreFloat3(&musicButton->mMeshBox.Extents, musicHalfSize);
	musicButton->menuTexture = MUSICb;
	musicButton->isMenu = true;
	Game::cubes.push_back(musicButton);
}

void Game::CleanLevel()
{

	cubes.clear();
	isLevelSet = false;
}

void Game::MenuLighting()
{
	mDirLights[0].Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	mDirLights[0].Direction = XMFLOAT3(0.0f, 0.0f, 0.7f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[1].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[1].Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	
	Effects::BasicFX->SetDirLights(mDirLights);
}

void Game::GameLighting()
{
	mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	mDirLights[0].Direction = XMFLOAT3(0.307f, -0.307f, 0.0f);

	mDirLights[1].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.307f, 0.0f, 0.307f);

	mDirLights[2].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	mDirLights[2].Direction = XMFLOAT3(-0.307f, 0.307f, -0.307f);

	Effects::BasicFX->SetDirLights(mDirLights);
}

void Game::IndentDiff(int index)
{
	if (!AreSameVec(cubes[index]->pos, cubes[index]->originPos * PushBack))
	{
		for (int i = 2; i < 5; ++i)
		{
			cubes[i]->pos = cubes[i]->originPos;
			XMStoreFloat4x4(&cubes[i]->localWorld, XMMatrixScalingFromVector(cubes[i]->scale) * XMMatrixTranslationFromVector(cubes[i]->pos));
		}
		curPos = cubes[index]->pos;
		cubes[index]->pos = curPos * PushBack;
		XMStoreFloat4x4(&cubes[index]->localWorld, XMMatrixScalingFromVector(cubes[index]->scale) * XMMatrixTranslationFromVector(cubes[index]->pos));
	}
	cubes[2]->menuTexture = EASYb;
	cubes[3]->menuTexture = MEDIUMb;
	cubes[4]->menuTexture = HARDb;
}

void Game::SetUpLevelData(int mines)
{
	if (!isLevelSet)
	{
		for (int i = 0; i < mines; i++) //changes blocks values into mines for the number passes in (mines)
		{
			int whichBlock = rand() % cubes.size();
			while (cubes[whichBlock]->texture == Cube::MINE)
			{
				whichBlock = rand() % cubes.size(); //picks a random number between 0 and size of cube array
			}
			cubes[whichBlock]->texture = Cube::MINE;
		}
		isLevelSet = true;
	}
}


int Game::CheckBlockSides(int placeInArray)
{
	/* draw thingy
	while (funcTimer > 0)
	{
		DrawScene();
		funcTimer -= FUNC_TIME_LENGTH;
	}
	funcTimer = 10;
	*/



	//check if the block is not really a block
	if ( placeInArray < cubes.size() && cubes[placeInArray] == NULL || cubes[placeInArray]->texture == Cube::MINE)
	{
		return 0;
	}

	//check if it has beed checked before
	for (int i = 0; i < cubesChecked.size(); i++)
	{
		if (cubesChecked[i] == placeInArray)
		{
			return 0;
		}
	}

	cubesChecked.push_back(cubes[placeInArray]->uniqueID);
	int numOfMinesTouching = 0;
	
	/*if (placeInArray == 184)
	{
		int u = 0;
	}*/


	//make counter for number of mines
	//int q = (36 / 35) + 1;
	int layerArea = levelWidth * levelLength;
	int cubeVolume = levelWidth * levelLength * levelHeight;
	int layerCurr = (placeInArray / layerArea) + 1;
	int layerOther;
	int temp;

	//set up some vars for runnnig the function again
	bool runLeft = false;
	bool runRight = false;
	bool runAbove = false;
	bool runBelow = false;
	bool runForward = false;
	bool runBack = false;

	//get the different blocks places 
	int left = placeInArray - 1;
	int right = placeInArray + 1;
	int forward = placeInArray + levelWidth;
	int back = placeInArray - levelWidth;
	int above = placeInArray + layerArea;
	int below = placeInArray - layerArea;

	//lambda to check stuff for removel
	auto checkRemoveBlock = [&](int blockPlace) 
	{

		//check if the block is not really a block
		if (cubes.size() > blockPlace && !cubes[blockPlace] == NULL)
		{
			//check if it has beed checked before
			for (int i = 0; i < cubesChecked.size(); i++)
			{
				if (!cubesChecked[i] == blockPlace)
				{
					cubesChecked.push_back(cubes[blockPlace]->uniqueID);
				}
			}
		}		
	};

	//check left
	if (left >= 0 &&
		placeInArray % levelWidth != 0 &&
		cubes[left] != NULL)
	{
		if (cubes[left]->texture == Cube::MINE)
		{
			numOfMinesTouching++;
			//make all the blocks aroiund the block we are checking marked as checked(this should mostly work
			//checkRemoveBlock(right);
			//checkRemoveBlock(forward);
			//checkRemoveBlock(back);
			//checkRemoveBlock(above);
			//checkRemoveBlock(below);

		}
		else if(cubes[left]->texture == Cube::GRAY)
		{
			runLeft = true;
		}
	}

	//check right
	if (right < cubeVolume &&
		placeInArray % levelWidth != levelWidth - 1 &&
		cubes[right] != NULL)
	{
		if (cubes[right]->texture == Cube::MINE)
		{
			numOfMinesTouching++;
			//return 0;
			//checkRemoveBlock(left);
			//checkRemoveBlock(forward);
			//checkRemoveBlock(back);
			//checkRemoveBlock(above);
			//checkRemoveBlock(below);
		}
		else if (cubes[right]->texture == Cube::GRAY)
		{
			runRight = true;
		}
	}

	//check forward
	layerOther = (forward / layerArea) + 1;
	//temp = placeInArray - (layerArea * layer);
	if (forward < cubeVolume - levelWidth &&
		layerCurr == layerOther &&
		//temp <= -((layerArea / levelWidth)) + 1 && //old andrew stuff
		cubes[forward] != NULL) //is not in top row)
	{
		if (cubes[forward]->texture == Cube::MINE)
		{
			numOfMinesTouching++;
			//return 0;
			//checkRemoveBlock(right);
			//checkRemoveBlock(left);
			//checkRemoveBlock(back);
			//checkRemoveBlock(above);
			//checkRemoveBlock(below);
		}
		else if (cubes[forward]->texture == Cube::GRAY)
		{
			runForward = true;
		}
	}

	//check backward
	layerOther = (back / layerArea) + 1; //Horizontal layer (xz Plane)
	temp = placeInArray - (layerArea * layerOther);//(placeInArray - (layerArea * layer - 1)) - layerArea; //gets a number from -(levelWidth*levelHeight) to 0 which represents which spot in the layer the block is in
	if (back >= 0 &&
		layerCurr == layerOther &&
		temp >= levelWidth && //temp >= -(layerArea)+levelWidth && //Old andrew stuff
		cubes[back] != NULL) //this if is never true
	{
		if (cubes[back]->texture == Cube::MINE)
		{
			
			numOfMinesTouching++;
			//return 0; 
			//checkRemoveBlock(right);
			//checkRemoveBlock(forward);
			//checkRemoveBlock(left);
			//checkRemoveBlock(above);
			//checkRemoveBlock(below);
		}
		else if (cubes[back]->texture == Cube::GRAY)
		{
			runBack = true;
		}
	}

	//check above
	//layer = (above / layerArea) + 1;
	if (above < cubeVolume && above > layerArea - 2 && // -2 or just -1 ?
		cubes[above] != NULL)
	{
		if (cubes[above]->texture == Cube::MINE)
		{
			numOfMinesTouching++;
			//return 0;
			//checkRemoveBlock(right);
			//checkRemoveBlock(forward);
			//checkRemoveBlock(back);
			//checkRemoveBlock(left);
			//checkRemoveBlock(below);
		}
		else if (cubes[above]->texture == Cube::GRAY)
		{
			runAbove = true;
		}
	}

	//check below
	//layer = (above / layerArea) + 1;
	if (below >= 0 && below < (cubeVolume - layerArea) &&
		cubes[below] != NULL)
	{
		if (cubes[below]->texture == Cube::MINE)
		{
			numOfMinesTouching++;
			//return 0;
			//checkRemoveBlock(right);
			//checkRemoveBlock(forward);
			//checkRemoveBlock(back);
			//checkRemoveBlock(above);
			//checkRemoveBlock(left);
		}
		else if (cubes[below]->texture == Cube::GRAY)
		{

			runBelow = true;
		}
	}

	//removes bounding box if the cube is going to represent a number
	if (numOfMinesTouching > 0 && numOfMinesTouching < 6)
	{
		XMStoreFloat3(&cubes[placeInArray]->mMeshBox.Extents, XMVectorZero());
	}
	//sets the texture of cube depending on how many mines its touching or deletes it.
	switch (numOfMinesTouching)
	{
	case 1:
		cubes[placeInArray]->texture = Cube::ONE;
		break;
	case 2:
		cubes[placeInArray]->texture = Cube::TWO;
		break;
	case 3:
		cubes[placeInArray]->texture = Cube::THREE;
		break;
	case 4:
		cubes[placeInArray]->texture = Cube::FOUR;
		break;
	case 5:
		cubes[placeInArray]->texture = Cube::FIVE;
		break;
	default:
		delete(cubes[placeInArray]);
		cubes[placeInArray] = NULL;
		break;

	}

	if (numOfMinesTouching == 0)
	{
		if (runRight)
		{
			CheckBlockSides(right);
		}
		if (runLeft)
		{
			CheckBlockSides(left);
		}
		if (runForward)
		{
			CheckBlockSides(forward);
		}
		if (runBack)
		{
			CheckBlockSides(back);
		}
		if (runAbove)
		{
			CheckBlockSides(above);
		}
		if (runBelow)
		{
			CheckBlockSides(below);
		}
	}
	else
	{
		DrawScene();
	}

	return 0;
}
