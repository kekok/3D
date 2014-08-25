#include "StdAfx.h"
#include <d3dx9.h>

class camera
{
public:
	camera(bool trans);
	void SetViewMatrix( D3DXVECTOR3 vEyePt1,D3DXVECTOR3 vLookatPt1,D3DXVECTOR3 vUpVec1);
	void SetProjMatrix(float fFOV, float fAspect, float fNear, float fFar);
	void apply(LPDIRECT3DDEVICE9  pDevice);
	void ProcessKey(float fElapsedTime);
	void Update(float fElapsedTime);
	LRESULT camera::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	D3DXMATRIXA16 matView,matProj;
	D3DXVECTOR3	vEyePt,vLookatPt,vUpVec,Delta;
	bool IsTrans;
	bool IsRot;
	POINT		LastPoint;
	float		CameraYawAngle;		
	float		CameraPitchAngle;
};