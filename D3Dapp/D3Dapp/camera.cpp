#include "StdAfx.h"
#include"camera.h"
camera::camera(bool trans)
{
	IsTrans = trans;
}
void camera::SetViewMatrix( D3DXVECTOR3 vEyePt1,D3DXVECTOR3 vLookatPt1,D3DXVECTOR3 vUpVec1)
{
	vEyePt = vEyePt1;
	vLookatPt = vLookatPt1;
	vUpVec = vUpVec1;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	D3DXMATRIX tmp;
	D3DXMatrixInverse( &tmp, NULL, &matView );
	D3DXVECTOR3* pZBasis = (D3DXVECTOR3*) &tmp._31;
	CameraYawAngle   = atan2f( pZBasis->x, pZBasis->z );
	float fLen = sqrtf(pZBasis->z*pZBasis->z + pZBasis->x*pZBasis->x);
	CameraPitchAngle = -atan2f( pZBasis->y, fLen );
}
void camera::SetProjMatrix(float fFOV, float fAspect, float fNear, float fFar)
{
	D3DXMatrixPerspectiveFovLH( &matProj, fFOV, fAspect, fNear, fFar );
}
void camera::apply(LPDIRECT3DDEVICE9  pDevice)
{
	if( pDevice )
	{
		pDevice->SetTransform(D3DTS_VIEW, &matView);
		pDevice->SetTransform(D3DTS_PROJECTION, &matProj);
	}
}
void  camera::ProcessKey(float fElapsedTime)
{
	Delta = D3DXVECTOR3(0.f, 0.f, 0.f);
	float fVelocity = 20 * fElapsedTime;

	if( GetKeyState('W') & 0x8000 )
	{
		IsTrans = true;
		Delta.z += fVelocity;
	}
	if( GetKeyState('S') & 0x8000 )
	{
		IsTrans = true;
		Delta.z -= fVelocity;
	}
	if( GetKeyState('A') & 0x8000 )
	{
		IsTrans = true;
		Delta.x -= fVelocity;
	}
	if( GetKeyState('D') & 0x8000 )
	{
		IsTrans = true;
		Delta.x += fVelocity;
	}
	if( GetKeyState(VK_HOME) & 0x8000 )
	{
		IsTrans = true;
		Delta.y += fVelocity;
	}
	if( GetKeyState(VK_END) & 0x8000 )
	{
		IsTrans = true;
		Delta.y -= fVelocity;
	}
}
void camera::Update(float fElapsedTime)
{
	POINT CurrentPos = {0, 0};
	POINT DeltaPos = {0, 0};

	if(IsRot)
	{
		
		GetCursorPos(&CurrentPos);
		DeltaPos.x = CurrentPos.x - LastPoint.x;
		DeltaPos.y = CurrentPos.y - LastPoint.y;
		LastPoint = CurrentPos;

		float fYaw = DeltaPos.x*0.01f;
		float fPitch = DeltaPos.y*0.01f;


		CameraYawAngle   += fYaw;
		CameraPitchAngle += fPitch;

	}

	 ProcessKey(fElapsedTime);
	D3DXVECTOR3 head = D3DXVECTOR3(0,0,1);
		if(IsTrans)
	{
	
		
		 vEyePt += Delta;
		 vLookatPt =  vEyePt +head;
	}
 // IsTrans = false;
	


	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
}
LRESULT camera::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//m_vDelta = D3DXVECTOR3(0.f, 0.f, 0.f);
	// setcapture getcursorPos
	switch( msg )
	{	
	case WM_LBUTTONDOWN:
		{
			IsRot = true;
			SetCapture(hWnd);
			GetCursorPos(&LastPoint);
		}
		break;
	case WM_LBUTTONUP:
		{
			IsRot = false;
			ReleaseCapture();
		}
		break;
	}

	return 0;
}