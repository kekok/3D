//--------------------------------------------------------------------------------------
// File: Tutorial09.cpp
//
// Mesh loading through DXUT
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTmisc.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "DXUTcamera.h"
#define DEG2RAD( a ) ( a * D3DX_PI / 180.f )


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
ID3D10Effect*                       g_pEffect = NULL;
ID3D10InputLayout*                  g_pVertexLayout = NULL;
ID3D10EffectTechnique*              g_pTechnique = NULL;
ID3D10EffectTechnique*      g_pTechniqueRenderLight = NULL;
CDXUTSDKMesh                        g_Mesh;
ID3D10EffectShaderResourceVariable* g_ptxDiffuseVariable = NULL;
ID3D10EffectMatrixVariable*         g_pWorldVariable = NULL;
ID3D10EffectMatrixVariable*         g_pViewVariable = NULL;
ID3D10EffectMatrixVariable*         g_pProjectionVariable = NULL;
ID3D10EffectVectorVariable* g_pLightDirVariable = NULL;
ID3D10EffectVectorVariable* g_pLightColorVariable = NULL;
ID3D10EffectVectorVariable* g_pOutputColorVariable = NULL;
D3DXMATRIX                          g_World;
D3DXMATRIX                          g_View;
D3DXMATRIX                          g_Projection;
CModelViewerCamera                  g_Camera;               // A model viewing camera
D3D10_DRIVER_TYPE           g_driverType = D3D10_DRIVER_TYPE_NULL;


//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D10DeviceAcceptable( UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType,
                                       DXGI_FORMAT BufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBufferSurfaceDesc,
                                      void* pUserContext );
HRESULT CALLBACK OnD3D10ResizedSwapChain( ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D10ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D10DestroyDevice( void* pUserContext );
void CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );

LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D10) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackD3D10DeviceAcceptable( IsD3D10DeviceAcceptable );
    DXUTSetCallbackD3D10DeviceCreated( OnD3D10CreateDevice );
    DXUTSetCallbackD3D10SwapChainResized( OnD3D10ResizedSwapChain );
    DXUTSetCallbackD3D10SwapChainReleasing( OnD3D10ReleasingSwapChain );
    DXUTSetCallbackD3D10DeviceDestroyed( OnD3D10DestroyDevice );
    DXUTSetCallbackD3D10FrameRender( OnD3D10FrameRender );

    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Tutorial09" );
    DXUTCreateDevice( true, 640, 480 );
    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Reject any D3D10 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D10DeviceAcceptable( UINT Adapter, UINT Output, D3D10_DRIVER_TYPE DeviceType,
                                       DXGI_FORMAT BufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10CreateDevice( ID3D10Device* pd3dDevice, const DXGI_SURFACE_DESC* pBufferSurfaceDesc,
                                      void* pUserContext )
{
    HRESULT hr;

    // Find the D3DX effect file
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"Tutorial09.fx" ) );
    DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3D10_SHADER_DEBUG;
    #endif
    V_RETURN( D3DX10CreateEffectFromFile( str, NULL, NULL, "fx_4_0", dwShaderFlags, 0, pd3dDevice, NULL,
                                              NULL, &g_pEffect, NULL, NULL ) );

    // Obtain the technique
    g_pTechnique = g_pEffect->GetTechniqueByName( "Render" );
	  g_pTechniqueRenderLight = g_pEffect->GetTechniqueByName( "RenderLight" );
    g_ptxDiffuseVariable = g_pEffect->GetVariableByName( "g_txDiffuse" )->AsShaderResource();
    g_pWorldVariable = g_pEffect->GetVariableByName( "World" )->AsMatrix();
    g_pViewVariable = g_pEffect->GetVariableByName( "View" )->AsMatrix();
    g_pProjectionVariable = g_pEffect->GetVariableByName( "Projection" )->AsMatrix();
	   g_pLightDirVariable = g_pEffect->GetVariableByName( "vLightDir" )->AsVector();
    g_pLightColorVariable = g_pEffect->GetVariableByName( "vLightColor" )->AsVector();
    g_pOutputColorVariable = g_pEffect->GetVariableByName( "vOutputColor" )->AsVector();

    // Define the input layout
    const D3D10_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = sizeof( layout ) / sizeof( layout[0] );

    // Create the input layout
    D3D10_PASS_DESC PassDesc;
    g_pTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    V_RETURN( pd3dDevice->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
                                             PassDesc.IAInputSignatureSize, &g_pVertexLayout ) );

    // Set the input layout
    pd3dDevice->IASetInputLayout( g_pVertexLayout );

    // Load the mesh
    V_RETURN( g_Mesh.Create( pd3dDevice, L"Tiny\\tiny.sdkmesh", true ) );

    // Initialize the world matrices
    D3DXMatrixIdentity( &g_World );

   D3DXVECTOR3 Eye( 0.0f, 0.0f, -800.0f );
    D3DXVECTOR3 At( 0.0f, 0.0f, 0.0f );
    g_Camera.SetViewParams( &Eye, &At );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D10 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D10ResizedSwapChain( ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBufferSurfaceDesc, void* pUserContext )
{
    // Setup the projection parameters again
   float fAspectRatio = static_cast<float>( pBufferSurfaceDesc->Width ) /
        static_cast<float>( pBufferSurfaceDesc->Height );
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 5000.0f );
    g_Camera.SetWindow( pBufferSurfaceDesc->Width, pBufferSurfaceDesc->Height );
    g_Camera.SetButtonMasks( MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D10 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10FrameRender( ID3D10Device* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    //
    // Clear the back buffer
    //
	  static float t = 0.0f;
    if( g_driverType == D3D10_DRIVER_TYPE_REFERENCE )
    {
        t += ( float )D3DX_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();
        if( dwTimeStart == 0 )
            dwTimeStart = dwTimeCur;
        t = ( dwTimeCur - dwTimeStart ) / 1000.0f;
    }

    // Rotate cube around the origin
    D3DXMatrixRotationY( &g_World, t );

    // Setup our lighting parameters
    D3DXVECTOR4 vLightDirs[2] =
    {
        D3DXVECTOR4( -0.577f, 0.577f, -0.577f, 1.0f ),
        D3DXVECTOR4( 0.0f, 0.0f, -1.0f, 1.0f ),
    };
    D3DXVECTOR4 vLightColors[2] =
    {
        D3DXVECTOR4( 0.5f, 0.5f, 0.5f, 1.0f ),
        D3DXVECTOR4( 0.5f, 0.0f, 0.0f, 1.0f )
    };

    //rotate the second light around the origin
    D3DXMATRIX mRotate;
    D3DXVECTOR4 vOutDir;
    D3DXMatrixRotationY( &mRotate, -2.0f * t );
    D3DXVec3Transform( &vLightDirs[1], ( D3DXVECTOR3* )&vLightDirs[1], &mRotate );

    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    ID3D10RenderTargetView* pRTV = DXUTGetD3D10RenderTargetView();
    pd3dDevice->ClearRenderTargetView( pRTV, ClearColor );

    //
    // Clear the depth stencil
    //
    ID3D10DepthStencilView* pDSV = DXUTGetD3D10DepthStencilView();
    pd3dDevice->ClearDepthStencilView( pDSV, D3D10_CLEAR_DEPTH, 1.0, 0 );

    //
    // Update variables that change once per frame
    //
     g_pProjectionVariable->SetMatrix( ( float* )g_Camera.GetProjMatrix() );
    g_pViewVariable->SetMatrix( ( float* )g_Camera.GetViewMatrix() );
    g_pWorldVariable->SetMatrix( ( float* )&g_World );
    //
    // Set the Vertex Layout
    //
    pd3dDevice->IASetInputLayout( g_pVertexLayout );

    //
    // Render the mesh
    //
    UINT Strides[1];
    UINT Offsets[1];
    ID3D10Buffer* pVB[1];
    pVB[0] = g_Mesh.GetVB10( 0, 0 );
    Strides[0] = ( UINT )g_Mesh.GetVertexStride( 0, 0 );
    Offsets[0] = 0;
    pd3dDevice->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
    pd3dDevice->IASetIndexBuffer( g_Mesh.GetIB10( 0 ), g_Mesh.GetIBFormat10( 0 ), 0 );

    D3D10_TECHNIQUE_DESC techDesc;
    g_pTechnique->GetDesc( &techDesc );
    SDKMESH_SUBSET* pSubset = NULL;
    ID3D10ShaderResourceView* pDiffuseRV = NULL;
    D3D10_PRIMITIVE_TOPOLOGY PrimType;
	  g_pLightDirVariable->SetFloatVectorArray( ( float* )vLightDirs, 0, 2 );
    g_pLightColorVariable->SetFloatVectorArray( ( float* )vLightColors, 0, 2 );

    for( UINT p = 0; p < techDesc.Passes; ++p )
    {
        for( UINT subset = 0; subset < g_Mesh.GetNumSubsets( 0 ); ++subset )
        {
            pSubset = g_Mesh.GetSubset( 0, subset );

            PrimType = g_Mesh.GetPrimitiveType10( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
            pd3dDevice->IASetPrimitiveTopology( PrimType );

            pDiffuseRV = g_Mesh.GetMaterial( pSubset->MaterialID )->pDiffuseRV10;
            g_ptxDiffuseVariable->SetResource( pDiffuseRV );

            g_pTechnique->GetPassByIndex( p )->Apply( 0 );
            pd3dDevice->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
        }
    }
	 for( int m = 0; m < 2; m++ )
    {
        D3DXMATRIX mLight;
        D3DXMATRIX mLightScale;
        D3DXVECTOR3 vLightPos = vLightDirs[m] * 5.0f;
        D3DXMatrixTranslation( &mLight, vLightPos.x, vLightPos.y, vLightPos.z );
        D3DXMatrixScaling( &mLightScale, 0.2f, 0.2f, 0.2f );
        mLight = mLightScale * mLight;

        // Update the world variable to reflect the current light
        g_pWorldVariable->SetMatrix( ( float* )&mLight );
        g_pOutputColorVariable->SetFloatVector( ( float* )&vLightColors[m] );

        g_pTechniqueRenderLight->GetDesc( &techDesc );
        for( UINT p = 0; p < techDesc.Passes; ++p )
        {
            g_pTechniqueRenderLight->GetPassByIndex( p )->Apply( 0 );
            pd3dDevice->DrawIndexed( 36, 0, 0 );
        }

    }
    //the mesh class also had a render method that allows rendering the mesh with the most common options
    //g_Mesh.Render( pd3dDevice, g_pTechnique, g_ptxDiffuseVariable );
}


//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10ReleasingSwapChain( void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Release D3D10 resources created in OnD3D10CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D10DestroyDevice( void* pUserContext )
{
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_RELEASE( g_pVertexLayout );
    SAFE_RELEASE( g_pEffect );
    g_Mesh.Destroy();
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Rotate cube around the origin
   g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
	 g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1: // Change as needed                
                break;
        }
    }
}

