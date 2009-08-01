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
  \file    DynamicDX.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    January 2009
*/

#pragma once

#if defined(_WIN32) && defined(USE_DIRECTX)

#ifndef DYNAMICDX_H
#define DYNAMICDX_H

#include "StdDefines.h"

#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>

//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif


#pragma warning( disable : 4995 ) // disable deprecated warning 
#include <string>
#pragma warning( default : 4995 ) 

enum DX10FormatDataType
{
    FDT_TYPELESS,
    FDT_UNORM,
    FDT_UNORM_SRGB,
    FDT_SNORM,
    FDT_UINT,
    FDT_SINT,
    FDT_FLOAT,
    FDT_SHAREDEXP,
};

class DX10Format
{
public:
  DX10FormatDataType  m_DataType;
  unsigned int        m_iChannelCount;
  unsigned int        m_iByteSize;
	std::wstring		m_strDesc;

    DX10Format() : m_DataType(FDT_TYPELESS), m_iChannelCount(0), m_iByteSize(0), m_strDesc(L"Invalid")
    {
    }

	DX10Format(DX10FormatDataType DataType, int iChannelCount, int iByteSize, std::wstring strDesc) :
        m_DataType(DataType), m_iChannelCount(iChannelCount), m_iByteSize(iByteSize), m_strDesc(strDesc)
    {
    }
};

static DX10Format g_dx10Format[] =
{
  DX10Format(FDT_TYPELESS, 0,  0, L"DXGI_FORMAT_UNKNOWN"), // 	= 0,
  DX10Format(FDT_TYPELESS, 4, 16, L"DXGI_FORMAT_R32G32B32A32_TYPELESS"), // 	= 1,
	DX10Format(FDT_FLOAT   , 4, 16, L"DXGI_FORMAT_R32G32B32A32_FLOAT"), // 	= 2,
	DX10Format(FDT_UINT    , 4, 16, L"DXGI_FORMAT_R32G32B32A32_UINT"), // 	= 3,
	DX10Format(FDT_SINT    , 4, 16, L"DXGI_FORMAT_R32G32B32A32_SINT"), // 	= 4,

	DX10Format(FDT_TYPELESS, 3, 12, L"DXGI_FORMAT_R32G32B32_TYPELESS"), // 	= 5,
	DX10Format(FDT_FLOAT   , 3, 12, L"DXGI_FORMAT_R32G32B32_FLOAT"), // 	= 6,
	DX10Format(FDT_UINT    , 3, 12, L"DXGI_FORMAT_R32G32B32_UINT"), // 	= 7,
	DX10Format(FDT_SINT    , 3, 12, L"DXGI_FORMAT_R32G32B32_SINT"), // 	= 8,

	DX10Format(FDT_TYPELESS, 4,  8, L"DXGI_FORMAT_R16G16B16A16_TYPELESS"), // 	= 9,
	DX10Format(FDT_FLOAT   , 4,  8, L"DXGI_FORMAT_R16G16B16A16_FLOAT"), // 	= 10,
	DX10Format(FDT_UNORM   , 4,  8, L"DXGI_FORMAT_R16G16B16A16_UNORM"), // 	= 11,
	DX10Format(FDT_UINT    , 4,  8, L"DXGI_FORMAT_R16G16B16A16_UINT"), // 	= 12,
	DX10Format(FDT_SNORM   , 4,  8, L"DXGI_FORMAT_R16G16B16A16_SNORM"), // 	= 13,
	DX10Format(FDT_SINT    , 4,  8, L"DXGI_FORMAT_R16G16B16A16_SINT"), // 	= 14,

	DX10Format(FDT_TYPELESS, 2,  8, L"DXGI_FORMAT_R32G32_TYPELESS"), // 	= 15,
	DX10Format(FDT_FLOAT   , 2,  8, L"DXGI_FORMAT_R32G32_FLOAT"), // 	= 16,
	DX10Format(FDT_UINT    , 2,  8, L"DXGI_FORMAT_R32G32_UINT"), // 	= 17,
	DX10Format(FDT_SINT    , 2,  8, L"DXGI_FORMAT_R32G32_SINT"), // 	= 18,

	DX10Format(FDT_TYPELESS, 3,  8, L"DXGI_FORMAT_R32G8X24_TYPELESS"), // 	= 19,
	DX10Format(FDT_FLOAT   , 1,  4, L"DXGI_FORMAT_D32_FLOAT_S8X24_UINT"), // 	= 20,
	DX10Format(FDT_FLOAT   , 1,  4, L"DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS"), // 	= 21,
	DX10Format(FDT_TYPELESS, 2,  4, L"DXGI_FORMAT_X32_TYPELESS_G8X24_UINT"), // 	= 22,

	DX10Format(FDT_TYPELESS, 4,  4, L"DXGI_FORMAT_R10G10B10A2_TYPELESS"), // 	= 23,
	DX10Format(FDT_UNORM   , 4,  4, L"DXGI_FORMAT_R10G10B10A2_UNORM"), // 	= 24,
	DX10Format(FDT_UINT    , 4,  4, L"DXGI_FORMAT_R10G10B10A2_UINT"), // 	= 25,

	DX10Format(FDT_FLOAT   , 3,  4, L"DXGI_FORMAT_R11G11B10_FLOAT"), // 	= 26,

	DX10Format(FDT_TYPELESS  , 4,  4, L"DXGI_FORMAT_R8G8B8A8_TYPELESS"), // 	= 27,
	DX10Format(FDT_UNORM     , 4,  4, L"DXGI_FORMAT_R8G8B8A8_UNORM"), // 	= 28,
	DX10Format(FDT_UNORM_SRGB, 4,  4, L"DXGI_FORMAT_R8G8B8A8_UNORM_SRGB"), // 	= 29,
	DX10Format(FDT_UINT      , 4,  4, L"DXGI_FORMAT_R8G8B8A8_UINT"), // 	= 30,
	DX10Format(FDT_SNORM     , 4,  4, L"DXGI_FORMAT_R8G8B8A8_SNORM"), // 	= 31,
	DX10Format(FDT_SINT      , 4,  4, L"DXGI_FORMAT_R8G8B8A8_SINT"), // 	= 32,

	DX10Format(FDT_TYPELESS, 2,  8, L"DXGI_FORMAT_R16G16_TYPELESS"), // 	= 33,
	DX10Format(FDT_FLOAT   , 2,  8, L"DXGI_FORMAT_R16G16_FLOAT"), // 	= 34,
	DX10Format(FDT_UNORM   , 2,  8, L"DXGI_FORMAT_R16G16_UNORM"), // 	= 35,
	DX10Format(FDT_UINT    , 2,  8, L"DXGI_FORMAT_R16G16_UINT"), // 	= 36,
	DX10Format(FDT_SNORM   , 2,  8, L"DXGI_FORMAT_R16G16_SNORM"), // 	= 37,
	DX10Format(FDT_SINT    , 2,  8, L"DXGI_FORMAT_R16G16_SINT"), // 	= 38,

	DX10Format(FDT_TYPELESS, 1,  4, L"DXGI_FORMAT_R32_TYPELESS"), // 	= 39,
	DX10Format(FDT_FLOAT   , 1,  4, L"DXGI_FORMAT_D32_FLOAT"), // 	= 40,
	DX10Format(FDT_FLOAT   , 1,  4, L"DXGI_FORMAT_R32_FLOAT"), // 	= 41,
	DX10Format(FDT_UINT    , 1,  4, L"DXGI_FORMAT_R32_UINT"), // 	= 42,
	DX10Format(FDT_SINT    , 1,  4, L"DXGI_FORMAT_R32_SINT"), // 	= 43,

	DX10Format(FDT_TYPELESS, 2,  4, L"DXGI_FORMAT_R24G8_TYPELESS"), // 	= 44,
	DX10Format(FDT_UNORM   , 2,  4, L"DXGI_FORMAT_D24_UNORM_S8_UINT"), // 	= 45,
	DX10Format(FDT_UNORM   , 2,  4, L"DXGI_FORMAT_R24_UNORM_X8_TYPELESS"), // 	= 46,
	DX10Format(FDT_TYPELESS, 2,  4, L"DXGI_FORMAT_X24_TYPELESS_G8_UINT"), // 	= 47,

	DX10Format(FDT_TYPELESS, 2,  2, L"DXGI_FORMAT_R8G8_TYPELESS"), // 	= 48,
	DX10Format(FDT_UNORM   , 2,  2, L"DXGI_FORMAT_R8G8_UNORM"), // 	= 49,
	DX10Format(FDT_UINT    , 2,  2, L"DXGI_FORMAT_R8G8_UINT"), // 	= 50,
	DX10Format(FDT_SNORM   , 2,  2, L"DXGI_FORMAT_R8G8_SNORM"), // 	= 51,
	DX10Format(FDT_SINT    , 2,  2, L"DXGI_FORMAT_R8G8_SINT"), // 	= 52,

  DX10Format(FDT_TYPELESS, 1,  2, L"DXGI_FORMAT_R16_TYPELESS"), // 	= 53,
	DX10Format(FDT_FLOAT   , 1,  2, L"DXGI_FORMAT_R16_FLOAT"), // 	= 54,
	DX10Format(FDT_UNORM   , 1,  2, L"DXGI_FORMAT_D16_UNORM"), // 	= 55,
	DX10Format(FDT_UNORM   , 1,  2, L"DXGI_FORMAT_R16_UNORM"), // 	= 56,
	DX10Format(FDT_UINT    , 1,  2, L"DXGI_FORMAT_R16_UINT"), // 	= 57,
	DX10Format(FDT_SNORM   , 1,  2, L"DXGI_FORMAT_R16_SNORM"), // 	= 58,
	DX10Format(FDT_SINT    , 1,  2, L"DXGI_FORMAT_R16_SINT"), // 	= 59,

	DX10Format(FDT_TYPELESS, 1,  1, L"DXGI_FORMAT_R8_TYPELESS"), // 	= 60,
	DX10Format(FDT_UNORM   , 1,  1, L"DXGI_FORMAT_R8_UNORM"), // 	= 61,
	DX10Format(FDT_UINT    , 1,  1, L"DXGI_FORMAT_R8_UINT"), // 	= 62,
	DX10Format(FDT_SNORM   , 1,  1, L"DXGI_FORMAT_R8_SNORM"), // 	= 63,
	DX10Format(FDT_SINT    , 1,  1, L"DXGI_FORMAT_R8_SINT"), // 	= 64,
	DX10Format(FDT_UNORM   , 1,  1, L"DXGI_FORMAT_A8_UNORM"), // 	= 65,
	DX10Format(FDT_UNORM   , 1,  1, L"DXGI_FORMAT_R1_UNORM"), // 	= 66,

	DX10Format(FDT_SHAREDEXP, 4,  4, L"DXGI_FORMAT_R9G9B9E5_SHAREDEXP"), // 	= 67,
	DX10Format(FDT_UNORM    , 4,  4, L"DXGI_FORMAT_R8G8_B8G8_UNORM"), // 	= 68,
	DX10Format(FDT_UNORM    , 4,  4, L"DXGI_FORMAT_G8R8_G8B8_UNORM"), // 	= 69,

	DX10Format(FDT_TYPELESS  , 0,  0, L"DXGI_FORMAT_BC1_TYPELESS"), // 	= 70,
	DX10Format(FDT_UNORM     , 0,  0, L"DXGI_FORMAT_BC1_UNORM"), // 	= 71,
	DX10Format(FDT_UNORM_SRGB, 0,  0, L"DXGI_FORMAT_BC1_UNORM_SRGB"), // 	= 72,
	DX10Format(FDT_TYPELESS  , 0,  0, L"DXGI_FORMAT_BC2_TYPELESS"), // 	= 73,
	DX10Format(FDT_UNORM     , 0,  0, L"DXGI_FORMAT_BC2_UNORM"), // 	= 74,
	DX10Format(FDT_UNORM_SRGB, 0,  0, L"DXGI_FORMAT_BC2_UNORM_SRGB"), // 	= 75,
	DX10Format(FDT_TYPELESS  , 0,  0, L"DXGI_FORMAT_BC3_TYPELESS"), // 	= 76,
	DX10Format(FDT_UNORM     , 0,  0, L"DXGI_FORMAT_BC3_UNORM"), // 	= 77,
	DX10Format(FDT_UNORM_SRGB, 0,  0, L"DXGI_FORMAT_BC3_UNORM_SRGB"), // 	= 78,
	DX10Format(FDT_TYPELESS  , 0,  0, L"DXGI_FORMAT_BC4_TYPELESS"), // 	= 79,
	DX10Format(FDT_UNORM     , 0,  0, L"DXGI_FORMAT_BC4_UNORM"), // 	= 80,
	DX10Format(FDT_SNORM     , 0,  0, L"DXGI_FORMAT_BC4_SNORM"), // 	= 81,
	DX10Format(FDT_TYPELESS  , 0,  0, L"DXGI_FORMAT_BC5_TYPELESS"), // 	= 82,
	DX10Format(FDT_UNORM     , 0,  0, L"DXGI_FORMAT_BC5_UNORM"), // 	= 83,
	DX10Format(FDT_SNORM     , 0,  0, L"DXGI_FORMAT_BC5_SNORM"), // 	= 84,

	DX10Format(FDT_UNORM   , 3,  2, L"DXGI_FORMAT_B5G6R5_UNORM"), // 	= 85,
	DX10Format(FDT_UNORM   , 4,  2, L"DXGI_FORMAT_B5G5R5A1_UNORM"), // 	= 86,
	DX10Format(FDT_UNORM   , 4,  4, L"DXGI_FORMAT_B8G8R8A8_UNORM"), // 	= 87,
	DX10Format(FDT_UNORM   , 4,  4, L"DXGI_FORMAT_B8G8R8X8_UNORM"), // 	= 88,
};


class DynamicDX {
public:
  static bool InitializeDX();
  static void CleanupDX();
  static bool IsInitialized() {return m_bDynamicDXIsInitialized;}

  // DXGI calls
  typedef HRESULT ( WINAPI* LPCREATEDXGIFACTORY )( REFIID, void** );
  static LPCREATEDXGIFACTORY CreateDXGIFactory;

  // D3D10 calls
  typedef HRESULT ( WINAPI* LPD3D10CREATEDEVICE )( IDXGIAdapter*, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, ID3D10Device** );
  static LPD3D10CREATEDEVICE D3D10CreateDevice;

  // D3DX10 calls
  typedef HRESULT (WINAPI *LPD3DX10CREATEEFFECTFROMFILEW)(LPCWSTR pFileName, CONST D3D10_SHADER_MACRO *pDefines, 
      ID3D10Include *pInclude, LPCSTR pProfile, UINT HLSLFlags, UINT FXFlags, ID3D10Device *pDevice, 
      ID3D10EffectPool *pEffectPool, ID3DX10ThreadPump* pPump, ID3D10Effect **ppEffect, ID3D10Blob **ppErrors, HRESULT* pHResult);
  static LPD3DX10CREATEEFFECTFROMFILEW D3DX10CreateEffectFromFileW;

  typedef HRESULT (WINAPI *LPD3DX10CREATEEFFECTFROMFILEA)(LPCSTR pFileName, CONST D3D10_SHADER_MACRO *pDefines, 
    ID3D10Include *pInclude, LPCSTR pProfile, UINT HLSLFlags, UINT FXFlags, ID3D10Device *pDevice, 
    ID3D10EffectPool *pEffectPool, ID3DX10ThreadPump* pPump, ID3D10Effect **ppEffect, ID3D10Blob **ppErrors, HRESULT* pHResult);
  static LPD3DX10CREATEEFFECTFROMFILEA D3DX10CreateEffectFromFileA;

  static int GetD3DX10Version() {return m_iD3DX10Version;}

private:
  static int        m_iD3DX10Version;
  static bool       m_bDynamicDXIsInitialized;
  static HINSTANCE  m_hD3D10;
  static HINSTANCE  m_hDXGI;
  static HINSTANCE  m_hD3DX10;

};

#endif // DYNAMICDX_H

#endif // _WIN32 && USE_DIRECTX