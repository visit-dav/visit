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
  \file    DynamicDX.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    January 2009
*/

#if defined(_WIN32) && defined(USE_DIRECTX)

#include "DynamicDX.h"
#include <string>
#include <sstream>

using namespace std;

DynamicDX::LPCREATEDXGIFACTORY DynamicDX::CreateDXGIFactory                     = NULL;
DynamicDX::LPD3D10CREATEDEVICE DynamicDX::D3D10CreateDevice                     = NULL;
DynamicDX::LPD3DX10CREATEEFFECTFROMFILEW DynamicDX::D3DX10CreateEffectFromFileW = NULL;
DynamicDX::LPD3DX10CREATEEFFECTFROMFILEA DynamicDX::D3DX10CreateEffectFromFileA = NULL;

int DynamicDX::m_iD3DX10Version           = 0;
bool DynamicDX::m_bDynamicDXIsInitialized = false;
HINSTANCE DynamicDX::m_hD3D10             = NULL;
HINSTANCE DynamicDX::m_hDXGI              = NULL;
HINSTANCE DynamicDX::m_hD3DX10            = NULL;

bool DynamicDX::InitializeDX() {
  if (m_bDynamicDXIsInitialized) return true;

  m_hD3D10 = LoadLibrary( L"d3d10.dll" );
  m_hDXGI  = LoadLibrary( L"dxgi.dll" );

#ifdef _DEBUG
  wstring wstrD3DX10DLL_prefix = L"d3dx10d_";
#else
  wstring wstrD3DX10DLL_prefix = L"d3dx10_";
#endif

  // try a few different D3DX10 DLLs
  for (int i = 42; i>=33; i--) {
    wstringstream wstrD3DX10DLL;
    wstrD3DX10DLL << wstrD3DX10DLL_prefix << i << ".dll";
    m_iD3DX10Version = i;
    m_hD3DX10 = LoadLibrary( wstrD3DX10DLL.str().c_str() );
    if (m_hD3DX10) break;
  }

  if( !m_hD3D10 || !m_hDXGI || !m_hD3DX10 ) return false;

  // DXGI calls
  CreateDXGIFactory = ( LPCREATEDXGIFACTORY )GetProcAddress( m_hDXGI, "CreateDXGIFactory" );
  if (!CreateDXGIFactory) return false;
  // D3D10 calls
  D3D10CreateDevice = ( LPD3D10CREATEDEVICE )GetProcAddress( m_hD3D10, "D3D10CreateDevice" );
  if (!D3D10CreateDevice) return false;
  // D3DX10 calls
  D3DX10CreateEffectFromFileW = ( LPD3DX10CREATEEFFECTFROMFILEW )GetProcAddress( m_hD3DX10, "D3DX10CreateEffectFromFileW" );
  if (!D3DX10CreateEffectFromFileW) return false;
  D3DX10CreateEffectFromFileA = ( LPD3DX10CREATEEFFECTFROMFILEA )GetProcAddress( m_hD3DX10, "D3DX10CreateEffectFromFileA" );
  if (!D3DX10CreateEffectFromFileA) return false;
  
  m_bDynamicDXIsInitialized = true;
  
  return true;
}


void DynamicDX::CleanupDX( ) {
  if (m_bDynamicDXIsInitialized) return;

  FreeLibrary( m_hD3D10 );
  FreeLibrary( m_hDXGI );
  FreeLibrary( m_hD3DX10 );
}

#endif // _WIN32 && USE_DIRECTX