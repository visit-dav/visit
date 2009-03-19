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
  \file    DXRenderer.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.0
  \date    August 2008
*/


#pragma once

#if defined(_WIN32) && defined(USE_DIRECTX)

#ifndef DXRENDERER_H
#define DXRENDERER_H

#include <string>

#include "../../StdTuvokDefines.h"
#include "../GPUMemMan/GPUMemMan.h"
#include "../AbstrRenderer.h"
#include "DXInclude.h"

class MasterController;

class DXRenderer : public AbstrRenderer {
  public:
    DXRenderer(MasterController* pMasterController, bool bUseOnlyPowerOfTwo, bool bDownSampleTo8Bits, bool bDisableBorder);
    virtual ~DXRenderer();
    virtual bool Initialize();
    virtual void Changed1DTrans();
    virtual void Changed2DTrans();

    /** Set the bit depth mode of the offscreen buffer used for blending.  Causes a full redraw. */
    virtual void SetBlendPrecision(EBlendPrecision eBlendPrecision);

    /** Deallocates GPU memory allocated during the rendering process. */
    virtual void Cleanup();

    /** Paint the image */
    virtual void Paint();

    /** Sends a message to the master to ask for a dataset to be loaded.
     * The dataset is converted to UVF if it is not one already.
     * @param strFilename path to a file */
    virtual bool LoadDataset(const std::string& strFilename);

    /** Change the size of the FBO we render to.  Any previous image is
     * destroyed, causing a full redraw on the next render.
     * \param vWinSize  new width and height of the view window */
    virtual void Resize(const UINTVECTOR2& vWinSize);

    virtual void SetLogoParams(std::string strLogoFilename, int iLogoPos);

    void RenderSlice(EWindowMode eDirection, UINT64 iSliceIndex,
                     FLOATVECTOR3 vMinCoords, FLOATVECTOR3 vMaxCoords,
                     UINT64VECTOR3 vDomainSize, DOUBLEVECTOR3 vAspectRatio,
                     DOUBLEVECTOR2 vWinAspectRatio);

    void SetWinID(HWND winId) {m_hWnd = winId;}
  protected:
    HWND                    m_hWnd;
    HINSTANCE               m_hInst;
    D3D10_DRIVER_TYPE       m_driverType;
    ID3D10Device*           m_pd3dDevice;
    IDXGISwapChain*         m_pSwapChain;
    ID3D10RenderTargetView* m_pRenderTargetView;
    IDXGIFactory*           m_pDXGIFactory;

    bool OnCreateDevice();
    bool OnResizedSwapChain();
    void OnReleasingSwapChain();
    void OnDestroyDevice();


    void SetRenderTargetArea(ERenderArea eREnderArea);
    void SetRenderTargetAreaScissor(ERenderArea eREnderArea);
    void SetViewPort(UINTVECTOR2 viLowerLeft, UINTVECTOR2 viUpperRight);

    bool Render2DView(ERenderArea eREnderArea, EWindowMode eDirection, UINT64 iSliceIndex);
    void RenderBBox(const FLOATVECTOR4 vColor = FLOATVECTOR4(1,0,0,1));
    void RenderBBox(const FLOATVECTOR4 vColor, const FLOATVECTOR3& vCenter, const FLOATVECTOR3& vExtend);
    void NewFrameClear(ERenderArea eREnderArea);
    bool Execute3DFrame(ERenderArea eREnderArea);
    void RerenderPreviousResult(bool bTransferToFramebuffer);
    void DrawLogo();
    void DrawBackGradient();

    virtual void SetDataDepShaderVars();

    virtual void Render3DView();
    virtual void Render3DPreLoop() {};
    virtual void Render3DInLoop(size_t iCurentBrick, int iStereoID) = 0;
    virtual void Render3DPostLoop() {}
    virtual void ComposeSurfaceImage(int iStereoID);
    virtual void Recompose3DView(ERenderArea eArea);

    virtual void RenderHQMIPPreLoop(EWindowMode eDirection);
    virtual void RenderHQMIPInLoop(const Brick& b) = 0;
    virtual void RenderHQMIPPostLoop() {}

    virtual void CreateOffscreenBuffers();
    virtual bool LoadAndVerifyShader(std::string strVSFile, std::string strFSFile, const std::vector<std::string>& strDirs, GLSLProgram** pShaderProgram);
    virtual bool LoadAndVerifyShader(std::string strVSFile, std::string strFSFile, GLSLProgram** pShaderProgram, bool bSearchSubdirs);

    void BBoxPreRender();
    void BBoxPostRender();
    virtual void ClearDepthBuffer();
    virtual void ClearColorBuffer();

    virtual void StartFrame();
    virtual void EndFrame(bool bNewDataToShow);
  private:
    void SetBrickDepShaderVarsSlice(const UINTVECTOR3& vVoxelCount);
    void RenderSeperatingLines();
    void RenderCoordArrows();

};
#endif // DXRENDERER_H

#endif // _WIN32 && USE_DIRECTX