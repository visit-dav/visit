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
  \file    SystemInfo.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    July 2008
*/

#include "SystemInfo.h"

#ifdef _WIN32
  #include <windows.h>
#else
  #if defined(macintosh) || (defined(__MACH__) && defined(__APPLE__))
    #include <sys/sysctl.h>
  #else
    #include <sys/sysinfo.h>
  #endif
#endif


SystemInfo::SystemInfo() :
  m_iUseMaxCPUMem(0),
  m_iUseMaxGPUMem(0)

{
  m_iProgrammBitWith = sizeof(void*)*8;
  m_iNumberOfCPUs = ComputeNumCPUs();
  m_iCPUMemSize   = ComputeCPUMemSize();
  m_iGPUMemSize   = ComputeGPUMemory();
}

unsigned int SystemInfo::ComputeNumCPUs() {
  #ifdef _WIN32
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo); 
    return siSysInfo.dwNumberOfProcessors;
  #else
    #if defined(macintosh) || (defined(__MACH__) && defined(__APPLE__))
      return 0;
    #else
      return 0;
    #endif
  #endif
}

UINT64 SystemInfo::ComputeCPUMemSize() {
  #ifdef _WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    GlobalMemoryStatusEx (&statex);
    return statex.ullTotalPhys;
  #else
    #if defined(macintosh) || (defined(__MACH__) && defined(__APPLE__))
  	  UINT64 phys = 0;
  	  int mib[2] = { CTL_HW, HW_PHYSMEM };    
  	  size_t len = sizeof(phys);	
  	  if (sysctl(mib, 2, &phys, &len, NULL, 0) != 0) return 0;  	
  	  return phys;
    #else
      struct sysinfo si;
      if(sysinfo(&si) != 0) return 0;
      return UINT64(si.totalram); // * UINT64(si*mem_unit);
    #endif
  #endif
}

#ifdef _WIN32
  #define INITGUID
  #include <windows.h>
  #include <string.h>
  #include <stdio.h>
  #include <assert.h>
  #include <d3d9.h>
  #include <multimon.h>

  #pragma comment( lib, "d3d9.lib" )

  HRESULT GetVideoMemoryViaDirectDraw( HMONITOR hMonitor, DWORD* pdwAvailableVidMem );
  HRESULT GetVideoMemoryViaDXGI( HMONITOR hMonitor, SIZE_T* pDedicatedVideoMemory, SIZE_T* pDedicatedSystemMemory, SIZE_T* pSharedSystemMemory );

  #ifndef SAFE_RELEASE
    #define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
  #endif

  UINT64 SystemInfo::ComputeGPUMemory( )
  {
    IDirect3D9* pD3D9 = NULL;
    pD3D9 = Direct3DCreate9( D3D_SDK_VERSION );
    if( pD3D9 )
    {
        UINT dwAdapterCount = pD3D9->GetAdapterCount();
        if (dwAdapterCount > 0) {
            UINT iAdapter = 0;

            HMONITOR hMonitor = pD3D9->GetAdapterMonitor( iAdapter );

            SIZE_T DedicatedVideoMemory;
            SIZE_T DedicatedSystemMemory;
            SIZE_T SharedSystemMemory;
            if( SUCCEEDED( GetVideoMemoryViaDXGI( hMonitor, &DedicatedVideoMemory, &DedicatedSystemMemory, &SharedSystemMemory ) ) )
            {
              SAFE_RELEASE( pD3D9 );
              return UINT64(DedicatedVideoMemory);
            } else {
              DWORD dwAvailableVidMem;
              if( SUCCEEDED( GetVideoMemoryViaDirectDraw( hMonitor, &dwAvailableVidMem ) ) ) {
                SAFE_RELEASE( pD3D9 );
                return UINT64(DedicatedVideoMemory);
              } else {
                SAFE_RELEASE( pD3D9 );
                return 0;
              }
            }
        }

        SAFE_RELEASE( pD3D9 );
        return 0;
    }
    else
    {
        return 0;
    }
  }

#else
  UINT64 SystemInfo::ComputeGPUMemory( )
  {
    #if defined(macintosh) || (defined(__MACH__) && defined(__APPLE__))
      return 0;
    #else
      return 0;
    #endif
  }
#endif
