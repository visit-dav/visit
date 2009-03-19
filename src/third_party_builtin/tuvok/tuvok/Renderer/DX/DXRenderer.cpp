/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/**
  \file    DXRenderer.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/

#if defined(_WIN32) && defined(USE_DIRECTX)

#include <Basics/DynamicDX.h>


#include "DXRenderer.h"
#include <Controller/Controller.h>
#include <Basics/SysTools.h>
#include <ctime>

using namespace std;

DXRenderer::DXRenderer(MasterController* pMasterController, bool bUseOnlyPowerOfTwo, bool bDownSampleTo8Bits, bool bDisableBorder) :
  AbstrRenderer(pMasterController, bUseOnlyPowerOfTwo, bDownSampleTo8Bits, bDisableBorder),
  m_hWnd(NULL),
  m_hInst(NULL),
  m_driverType(D3D10_DRIVER_TYPE_NULL),
  m_pd3dDevice(NULL),
  m_pSwapChain(NULL),
  m_pRenderTargetView(NULL),
  m_pDXGIFactory(NULL)
{
}

DXRenderer::~DXRenderer() {
}

void DXRenderer::Cleanup() {
  OnReleasingSwapChain();
  OnDestroyDevice();
}

bool DXRenderer::Initialize() {
  // call the parent
  if (!AbstrRenderer::Initialize()) {
    T_ERROR("Error in parent call -> aborting");
    return false;
  }

  // next destroy the dx system we may have created already 
  if (m_pd3dDevice) OnDestroyDevice();
  
  // next initialize the DirectX subsystem
  HRESULT hr = S_OK;
  UINT createDeviceFlags = 0;
#ifdef _DEBUG
  createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif
  D3D10_DRIVER_TYPE driverTypes[] =
  {
      D3D10_DRIVER_TYPE_HARDWARE,
      D3D10_DRIVER_TYPE_REFERENCE,
  };
  UINT numDriverTypes = sizeof( driverTypes ) / sizeof( driverTypes[0] );
  m_pd3dDevice = NULL;
  for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
  {
    m_driverType = driverTypes[driverTypeIndex];
    hr = DynamicDX::D3D10CreateDevice( NULL,
                            m_driverType,
                            NULL,
                            createDeviceFlags,
                            D3D10_SDK_VERSION,
                            &m_pd3dDevice );
    if( SUCCEEDED( hr ) ) break;
  }
  if( FAILED( hr ) ) return false; 
  hr = DynamicDX::CreateDXGIFactory( IID_IDXGIFactory, ( void** )&m_pDXGIFactory );
  if( FAILED( hr ) ) return false;

  // finally initialize the renderer
  return OnCreateDevice();
}

void DXRenderer::Changed1DTrans() {
  AbstrRenderer::Changed1DTrans();


}

void DXRenderer::Changed2DTrans() {
  AbstrRenderer::Changed2DTrans();



}

void DXRenderer::Resize(const UINTVECTOR2& vWinSize) {
  // call the parent
  AbstrRenderer::Resize(vWinSize);

  // next destroy screen dependent data we may have created already 
  if (m_pSwapChain) OnReleasingSwapChain();

  // next create the DirectX swapchain subsystem
  HRESULT hr = S_OK;
  // get the dxgi device
  IDXGIDevice* pDXGIDevice = NULL;
  hr = m_pd3dDevice->QueryInterface( IID_IDXGIDevice, ( void** )&pDXGIDevice );
  if( FAILED( hr ) ) {
      // TODO report failiure
      return;
  }

  // create a swap chain
  DXGI_SWAP_CHAIN_DESC SwapChainDesc;
  ZeroMemory( &SwapChainDesc, sizeof( DXGI_SWAP_CHAIN_DESC ) );
  SwapChainDesc.BufferDesc.Width = vWinSize.x;
  SwapChainDesc.BufferDesc.Height = vWinSize.y;
  SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
  SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
  SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  SwapChainDesc.SampleDesc.Count = 1;
  SwapChainDesc.SampleDesc.Quality = 0;
  SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  SwapChainDesc.BufferCount = 3;
  SwapChainDesc.OutputWindow = m_hWnd;
  SwapChainDesc.Windowed = true;
  SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  SwapChainDesc.Flags = 0;
  hr = m_pDXGIFactory->CreateSwapChain( pDXGIDevice, &SwapChainDesc, &m_pSwapChain );
  pDXGIDevice->Release();
  pDXGIDevice = NULL;
  if( FAILED( hr ) ) {
      // TODO report failiure
      return;
  }

  // Create a render target view
  ID3D10Texture2D* pBackBuffer;
  hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), ( LPVOID* )&pBackBuffer );
  if( FAILED( hr ) ) {
      // TODO report failiure
      return;
  }

  hr = m_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView );
  pBackBuffer->Release();
  if( FAILED( hr ) ) {
      // TODO report failiure
      return;
  }

  m_pd3dDevice->OMSetRenderTargets( 1, &m_pRenderTargetView, NULL );

  // Setup the viewport
  D3D10_VIEWPORT vp;
  vp.Width = vWinSize.x;
  vp.Height = vWinSize.y;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  m_pd3dDevice->RSSetViewports( 1, &vp );

  // next initialize the renderer
  if (!OnResizedSwapChain()) {
      // TODO report failiure
      return;
  }
}

bool DXRenderer::OnCreateDevice() {
  return true;
}

bool DXRenderer::OnResizedSwapChain() {
  return true;
}

void DXRenderer::OnReleasingSwapChain() {

}

void DXRenderer::OnDestroyDevice() {

}

void DXRenderer::RenderSeperatingLines() {
}

void DXRenderer::ClearDepthBuffer() {
}

void DXRenderer::ClearColorBuffer() {
}

void DXRenderer::StartFrame() {
}

void DXRenderer::Paint() {
  AbstrRenderer::Paint();
  // TODO
}

#pragma warning(disable:4100) // disable "unused parameter" warning for now
void DXRenderer::EndFrame(bool bNewDataToShow) {
}

void DXRenderer::SetRenderTargetArea(ERenderArea eREnderArea) {
}

void DXRenderer::SetRenderTargetAreaScissor(ERenderArea eREnderArea) {
}

void DXRenderer::SetViewPort(UINTVECTOR2 viLowerLeft, UINTVECTOR2 viUpperRight) {
}

void DXRenderer::RenderSlice(EWindowMode eDirection, UINT64 iSliceIndex,
                             FLOATVECTOR3 vMinCoords, FLOATVECTOR3 vMaxCoords,
                             UINT64VECTOR3 vDomainSize, DOUBLEVECTOR3 vAspectRatio,
                             DOUBLEVECTOR2 vWinAspectRatio) {
}

bool DXRenderer::Render2DView(ERenderArea eREnderArea, EWindowMode eDirection, UINT64 iSliceIndex) {
  return true;
}

void DXRenderer::RenderHQMIPPreLoop(EWindowMode eDirection) {
}

void DXRenderer::RenderBBox(const FLOATVECTOR4 vColor) {
}

void DXRenderer::RenderBBox(const FLOATVECTOR4 vColor, const FLOATVECTOR3& vCenter, const FLOATVECTOR3& vExtend) {
}

void DXRenderer::NewFrameClear(ERenderArea eREnderArea) {
}

void DXRenderer::RenderCoordArrows() {
}

bool DXRenderer::Execute3DFrame(ERenderArea eREnderArea) {
  return true;
}

void DXRenderer::RerenderPreviousResult(bool bTransferToFramebuffer) {
}

void DXRenderer::DrawBackGradient() {
}

void DXRenderer::CreateOffscreenBuffers() {
}

void DXRenderer::SetBrickDepShaderVarsSlice(const UINTVECTOR3& vVoxelCount) {
}

void DXRenderer::SetDataDepShaderVars() {
}

void DXRenderer::SetBlendPrecision(EBlendPrecision eBlendPrecision) {
  if (eBlendPrecision != m_eBlendPrecision) {
    AbstrRenderer::SetBlendPrecision(eBlendPrecision);
    // TODO
  }
}

bool DXRenderer::LoadAndVerifyShader(string strVSFile, string strFSFile, const std::vector<std::string>& strDirs, GLSLProgram** pShaderProgram) {
  return true;
}

bool DXRenderer::LoadAndVerifyShader(string strVSFile, string strFSFile, GLSLProgram** pShaderProgram, bool bSearchSubdirs) {
  return true;
}

void DXRenderer::BBoxPreRender() {
}

void DXRenderer::BBoxPostRender() {
}

bool DXRenderer::LoadDataset(const string& strFilename) {
  if (AbstrRenderer::LoadDataset(strFilename)) {
    // TODO
    return true;
  } else return false;
}

void DXRenderer::Recompose3DView(ERenderArea eArea) {
}

void DXRenderer::Render3DView() {
}

void DXRenderer::SetLogoParams(std::string strLogoFilename, int iLogoPos) {
  AbstrRenderer::SetLogoParams(strLogoFilename, iLogoPos);
  // TODO
}

void DXRenderer::ComposeSurfaceImage(int iStereoID) {
}

#endif // _WIN32 && USE_DIRECTX
