////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: 박창현 Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <array>

#include "CSphere.h"
#include "CWall.h"
#include "CLight.h"
#include "CTopWall.h"
#include "CBottomWall.h"
#include "CRightWall.h"
#include "CLeftWall.h"
#include "CFloor.h"
#include "CHole.h"
#include "CHandSphere.h"
#include "Status.h"
#include "Player.h"
#include "DisplayGameStatus.h"
#include "FoulManager.h"
#include "TurnManager.h"
#include "d3dUtility.h"
#include "d3dfont.h"

using std::array;

IDirect3DDevice9* Device = NULL;

// window size
const int Width = 1024;
const int Height = 768;

// ball setting
const float BALL_SET_RATIO = 1.82f;
const float COMMON_RADIUS = 0.14f;


const float spherePos[16][2] = {
	//hand ball
	{-2.7f, 0},

	//solid
	{1.5f, 0}, 
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 4), (COMMON_RADIUS * 2 + 0.02f)}, 
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO)), -(COMMON_RADIUS + 0.01f)}, 
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 4), -(COMMON_RADIUS * 2 + 0.02f)}, 
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 3), (COMMON_RADIUS * 3 + 0.03f)},   
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 2), (COMMON_RADIUS * 2 + 0.02f)},
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 3), -(COMMON_RADIUS + 0.01f)},

	//black ball
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 2), 0}, 

	//striple ball
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 3), -(COMMON_RADIUS * 3 + 0.03f)},
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 3), (COMMON_RADIUS + 0.01f)}, 
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 4), -(COMMON_RADIUS * 4 + 0.04f)}, 
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO)), COMMON_RADIUS + 0.01f},
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 4), 0},
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 4), (COMMON_RADIUS * 4 + 0.04f)},
	{(1.5f + (COMMON_RADIUS * BALL_SET_RATIO) * 2), -(COMMON_RADIUS * 2 + 0.02f)} 
};

const float holePos[6][2] = {
	{-4.45f,-2.95f},{0.05f,-3.05f},{4.5f,-2.95f},
	{-4.45f,2.95f},{0.05f,3.05f},{4.5f,2.95f}
};



// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

#define M_RADIUS 0.21   // ball radius
#define PI 3.14159265
#define M_HEIGHT 0.01
#define DECREASE_RATE 0.9982


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CFloor	g_legoPlane;
CSphere	g_target_blueball("blue");
CLight	g_light;
CHole g_hole[6];


array<CWall*, 4> g_legowall = {
	new CTopWall(0.0f, 0.12f, 3.06f, d3d::DARKRED),
	new CBottomWall(0.0f, 0.12f, -3.06f, d3d::DARKRED),
	new CRightWall(4.56f, 0.12f, 0.0f, d3d::DARKRED),
	new CLeftWall(-4.56f, 0.12f, 0.0f, d3d::DARKRED),
};

array<CSphere*, 16> g_sphere = {
	new CHandSphere("0"), new CSphere("1"), new CSphere("2"), new CSphere("3"),
	new CSphere("4"), new CSphere("5"), new CSphere("6"), new CSphere("7"),
	new CSphere("8"), new CSphere("9"), new CSphere("10"), new CSphere("11"),
	new CSphere("12"), new CSphere("13"), new CSphere("14"), new CSphere("15")
};

double g_camera_pos[3] = { 0.0, 5.0, -8.0 };

Player players[2] = { Player(1), Player(2) };
vector<Player*> playerVec = { &players[0], &players[1] };
Status status(playerVec);

TurnManager turnManager(status.getPlayerIdList());
FoulManager foulManager;
DisplayGameStatus displayGameStatus(Width, Height, players);
HWND window;


// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
void destroyAllLegoBlock(void)
{
}

// initialization
bool Setup()
{
	int i;

	D3DXMatrixIdentity(&g_mWorld);
	D3DXMatrixIdentity(&g_mView);
	D3DXMatrixIdentity(&g_mProj);

	if (false == displayGameStatus.create("Times New Roman", 16, Device)) return false;

	for (i = 0; i < 6; i++)
	{
		if (false == g_hole[i].create(Device)) return false;
		g_hole[i].setPosition(holePos[i][0], -0.23f, holePos[i][1]);
	}

	// create plane and set the position
	if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f);

	// create walls and set the position. note that there are four walls
	if (false == g_legowall[0]->create(Device, -1, -1, 9.57, 0.3f, 0.30f, d3d::DARKRED)) return false;
	g_legowall[0]->setPosition(0.0f, 0.12f, 3.15f);
	if (false == g_legowall[1]->create(Device, -1, -1, 9.57, 0.3f, 0.30f, d3d::DARKRED)) return false;
	g_legowall[1]->setPosition(0.0f, 0.12f, -3.15f);
	if (false == g_legowall[2]->create(Device, -1, -1, 0.30f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[2]->setPosition(4.65f, 0.12f, 0.0f);
	if (false == g_legowall[3]->create(Device, -1, -1, 0.30f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[3]->setPosition(-4.65f, 0.12f, 0.0f);

	// create sixteen balls and set the position
	for (i = 0; i < 16; i++) {
		if (false == g_sphere[i]->create(Device)) return false;
		g_sphere[i]->setPosition(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
		g_sphere[i]->setPower(0, 0);
	}

	// create blue ball for set direction
	if (false == g_target_blueball.create(Device)) return false;
	g_target_blueball.setPosition(.0f, (float)M_RADIUS, .0f);

	// light setting 
	D3DLIGHT9 lit;
	::ZeroMemory(&lit, sizeof(lit));
	lit.Type = D3DLIGHT_POINT;
	lit.Diffuse = d3d::WHITE;
	lit.Specular = d3d::WHITE * 1.0f;
	lit.Ambient = d3d::WHITE * 1.0f;
	lit.Position = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
	lit.Range = 100.0f;
	lit.Attenuation0 = 0.0f;
	lit.Attenuation1 = 0.9f;
	lit.Attenuation2 = 0.0f;

	float radius = 0.1f;
	if (false == g_light.create(Device, lit, radius)) return false;

	// Position and aim the camera.
	D3DXVECTOR3 pos(0.0f, 5.0f, -8.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);

	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4, (float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

	// Set render states.
	Device->SetRenderState(D3DRS_LIGHTING, TRUE);
	Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	g_light.setLight(Device, g_mWorld);
	return true;
}

void Cleanup(void)
{
	g_legoPlane.destroy();
	for (int i = 0; i < 4; i++) {
		g_legowall[i]->destroy();
	}
	destroyAllLegoBlock();
	g_light.destroy();
	displayGameStatus.destory();
}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta)
{
	int i = 0;
	int j = 0;

	if (Device)
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();

		displayGameStatus.update();

		// update the position of each ball. during update, check whether each ball hit by walls.
		for (i = 0; i < 16; i++) {
			g_sphere[i]->ballUpdate(timeDelta);
			for (j = 0; j < 4; j++) { 
				g_legowall[j]->hitBy(*g_sphere[i]); 
			}
		}

		for (i = 0; i < 16; i++) {
			for (j = 0; j < 16; j++) {
				if (i >= j) {
					continue;
				}
				g_sphere[i]->hitBy(*g_sphere[j]);
			}
		}

		for (i = 0; i < 6; i++) {
			for (j = 0; j < 16; j++) {
				if (g_hole[i].hasIntersected(*g_sphere[j]) && status.getTurnPlayer()->getBallType() == BallType::NONE &&
					g_sphere[j]->getBallType() != BallType::EIGHT && g_sphere[j]->getBallType() != BallType::HAND) {
					BallType nowBallType = g_sphere[j]->getBallType();
					status.getTurnPlayer()->setBallType(nowBallType);
					status.getNotTurnPlayer()->setBallType((nowBallType == BallType::STRIPE) ? BallType::SOLID : BallType::STRIPE);
				}
				g_hole[i].hitBy(*g_sphere[j]);
			}
		}

		// draw plane, walls, and spheres
		g_legoPlane.draw(Device, g_mWorld);
		for (i = 0; i < 4; i++) {
			g_legowall[i]->draw(Device, g_mWorld);
		}
		for (i = 0; i < 16; i++) {
			g_sphere[i]->draw(Device, g_mWorld);
		}
		for (i = 0; i < 6; i++) {
			g_hole[i].draw(Device, g_mWorld);
		}
			

		g_target_blueball.draw(Device, g_mWorld);
		//g_light.draw(Device); // 효과는 주되, 화면상에서 가려줌

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture(0, NULL);
	}

	foulManager.checkFoul();

	if (!turnManager.processTurn(g_sphere)) {
		if (foulManager.isLose()) {
			status.setWinnerPlayer(status.getNotTurnPlayer()->getPlayerId());
		}

		if (status.getGameEndStatus()) {
			string msg = "Player " + std::to_string(status.getWinnerPlayer()) + " 승리";
			MessageBox(nullptr, msg.c_str(), nullptr, 0);
			::DestroyWindow(window);
			return true;
		}
	}
	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false;
	static bool isReset = true;
	static int old_x = 0;
	static int old_y = 0;
	static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE;

	switch (msg) {
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_ESCAPE:
			::DestroyWindow(hwnd);
			break;
		case VK_RETURN:
			if (NULL != Device) {
				wire = !wire;
				Device->SetRenderState(D3DRS_FILLMODE,
					(wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
			}
			break;
		case VK_SPACE:
			if (!status.getTurnProgressStatus()) {
				D3DXVECTOR3 targetpos = g_target_blueball.getPosition();
				D3DXVECTOR3	whitepos = g_sphere[0]->getPosition();
				double theta = acos(sqrt(pow(targetpos.x - whitepos.x, 2)) / sqrt(pow(targetpos.x - whitepos.x, 2) +
					pow(targetpos.z - whitepos.z, 2)));		// 기본 1 사분면
				if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x >= 0) { 
					theta = -theta; 
				} //4 사분면
				if (targetpos.z - whitepos.z >= 0 && targetpos.x - whitepos.x <= 0) { 
					theta = PI - theta; 
				} //2 사분면
				if (targetpos.z - whitepos.z <= 0 && targetpos.x - whitepos.x <= 0) { 
					theta = PI + theta; 
				} // 3 사분면
				double distance = sqrt(pow(targetpos.x - whitepos.x, 2) + pow(targetpos.z - whitepos.z, 2));
				g_sphere[0]->setPower(distance * cos(theta), distance * sin(theta));
				turnManager.processTriggerOn();
			}
			
			break;

		}
		break;
	}

	case WM_MOUSEMOVE:
	{
		int new_x = LOWORD(lParam);
		int new_y = HIWORD(lParam);
		float dx;
		float dy;

		if (LOWORD(wParam) & MK_LBUTTON) {
			

			if (isReset) {
				isReset = false;
			}
			else {
				D3DXVECTOR3 vDist;
				D3DXVECTOR3 vTrans;
				D3DXMATRIX mTrans;
				D3DXMATRIX mX;
				D3DXMATRIX mY;

				switch (move) {
				case WORLD_MOVE:
					dx = (old_x - new_x) * 0.01f;
					dy = (old_y - new_y) * 0.01f;
					D3DXMatrixRotationY(&mX, dx);
					D3DXMatrixRotationX(&mY, dy);
					g_mWorld = g_mWorld * mX * mY;

					break;
				}
			}

			old_x = new_x;
			old_y = new_y;

		}
		else {
			isReset = true;

			if (LOWORD(wParam) & MK_RBUTTON) {
				dx = (old_x - new_x);// * 0.01f;
				dy = (old_y - new_y);// * 0.01f;

				D3DXVECTOR3 coord3d = g_target_blueball.getPosition();
				g_target_blueball.setPosition(coord3d.x + dx * (-0.007f), coord3d.y, coord3d.z + dy * 0.007f);
			}
			old_x = new_x;
			old_y = new_y;

			move = WORLD_MOVE;
		}
		break;
	}
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	srand(static_cast<unsigned int>(time(NULL)));

	if (!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	Device->Release();

	return 0;
}