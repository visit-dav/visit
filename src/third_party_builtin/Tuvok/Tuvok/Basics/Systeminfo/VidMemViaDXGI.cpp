//----------------------------------------------------------------------------
// File: VidMemViaDXGI.cpp
//
//
// DXGI is only available on Windows Vista or later. This method returns the 
// amount of dedicated video memory, the amount of dedicated system memory, 
// and the amount of shared system memory. DXGI is more reflective of the true 
// system configuration than the previous 4 methods. 
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define INITGUID
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <dxgi.h>


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

typedef HRESULT ( WINAPI* LPCREATEDXGIFACTORY )( REFIID, void** );


HRESULT GetVideoMemoryViaDXGI( HMONITOR hMonitor,
                               SIZE_T* pDedicatedVideoMemory,
                               SIZE_T* pDedicatedSystemMemory,
                               SIZE_T* pSharedSystemMemory )
{
    HRESULT hr;
    bool bGotMemory = false;
    *pDedicatedVideoMemory = 0;
    *pDedicatedSystemMemory = 0;
    *pSharedSystemMemory = 0;

    HINSTANCE hDXGI = LoadLibrary( L"dxgi.dll" );
    if( hDXGI )
    {
        LPCREATEDXGIFACTORY pCreateDXGIFactory = NULL;
        IDXGIFactory* pDXGIFactory = NULL;

        pCreateDXGIFactory = ( LPCREATEDXGIFACTORY )GetProcAddress( hDXGI, "CreateDXGIFactory" );
        pCreateDXGIFactory( __uuidof( IDXGIFactory ), ( LPVOID* )&pDXGIFactory );

        for( int index = 0; ; ++index )
        {
            bool bFoundMatchingAdapter = false;
            IDXGIAdapter* pAdapter = NULL;
            hr = pDXGIFactory->EnumAdapters( index, &pAdapter );
            if( FAILED( hr ) ) // DXGIERR_NOT_FOUND is expected when the end of the list is hit
                break;

            for( int iOutput = 0; ; ++iOutput )
            {
                IDXGIOutput* pOutput = NULL;
                hr = pAdapter->EnumOutputs( iOutput, &pOutput );
                if( FAILED( hr ) ) // DXGIERR_NOT_FOUND is expected when the end of the list is hit
                    break;

                DXGI_OUTPUT_DESC outputDesc;
                ZeroMemory( &outputDesc, sizeof( DXGI_OUTPUT_DESC ) );
                if( SUCCEEDED( pOutput->GetDesc( &outputDesc ) ) )
                {
                    if( hMonitor == outputDesc.Monitor )
                        bFoundMatchingAdapter = true;

                }
                SAFE_RELEASE( pOutput );
            }

            if( bFoundMatchingAdapter )
            {
                DXGI_ADAPTER_DESC desc;
                ZeroMemory( &desc, sizeof( DXGI_ADAPTER_DESC ) );
                if( SUCCEEDED( pAdapter->GetDesc( &desc ) ) )
                {
                    bGotMemory = true;
                    *pDedicatedVideoMemory = desc.DedicatedVideoMemory;
                    *pDedicatedSystemMemory = desc.DedicatedSystemMemory;
                    *pSharedSystemMemory = desc.SharedSystemMemory;
                }
                break;
            }
        }

        FreeLibrary( hDXGI );
    }

    if( bGotMemory )
        return S_OK;
    else
        return E_FAIL;
}

