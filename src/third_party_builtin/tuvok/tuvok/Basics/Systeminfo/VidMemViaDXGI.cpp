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
#if defined(_WIN32) && defined(USE_DIRECTX)

#define INITGUID
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <dxgi.h>

#include "../DynamicDX.h"

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

    if( DynamicDX::IsInitialized() )
    {
        IDXGIFactory* pDXGIFactory = NULL;
        DynamicDX::CreateDXGIFactory( __uuidof( IDXGIFactory ), ( LPVOID* )&pDXGIFactory );

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
    }

    if( bGotMemory )
        return S_OK;
    else
        return E_FAIL;
}


#endif