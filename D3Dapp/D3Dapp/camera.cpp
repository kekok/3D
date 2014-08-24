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