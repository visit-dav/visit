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
  \file    GLRenderer.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.0
  \date    August 2008
*/


#pragma once

#ifndef GLRENDERER_H
#define GLRENDERER_H

#include <string>

#include "../../StdTuvokDefines.h"
#include "../AbstrRenderer.h"
#include "GLTargetBinder.h"

class MasterController;
class GLTexture1D;
class GLTexture2D;
class GLSLProgram;

class GLRenderer : public AbstrRenderer {
  public:
    GLRenderer(MasterController* pMasterController, bool bUseOnlyPowerOfTwo, bool bDownSampleTo8Bits, bool bDisableBorder);
    virtual ~GLRenderer();
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

  protected:
    float           m_fScaledIsovalue;
    float           m_fScaledCVIsovalue;
    GLTargetBinder  m_TargetBinder;
    GLTexture1D*    m_p1DTransTex;
    GLTexture2D*    m_p2DTransTex;
    unsigned char*  m_p1DData;
    unsigned char*  m_p2DData;
    GLFBOTex*       m_pFBO3DImageLast;
    GLFBOTex*       m_pFBO3DImageCurrent[2];
    GLFBOTex*       m_pFBOIsoHit[2];
    GLFBOTex*       m_pFBOCVHit[2];
    int             m_iFilledBuffers;
    GLTexture2D*    m_pLogoTex;
    GLSLProgram*    m_pProgram1DTrans[2];
    GLSLProgram*    m_pProgram2DTrans[2];
    GLSLProgram*    m_pProgramIso;
    GLSLProgram*    m_pProgramHQMIPRot;

    void SetRenderTargetArea(ERenderArea eREnderArea);
    void SetRenderTargetAreaScissor(ERenderArea eREnderArea);
    void SetViewPort(UINTVECTOR2 viLowerLeft, UINTVECTOR2 viUpperRight);

    bool Render2DView(ERenderArea eREnderArea, EWindowMode eDirection, UINT64 iSliceIndex);
    void RenderBBox(const FLOATVECTOR4 vColor = FLOATVECTOR4(1,0,0,1));
    void RenderBBox(const FLOATVECTOR4 vColor, const FLOATVECTOR3& vCenter,
                    const FLOATVECTOR3& vExtend);
    void RenderClipPlane(size_t iStereoID);
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
    GLSLProgram*    m_pProgramTrans;
    GLSLProgram*    m_pProgram1DTransSlice;
    GLSLProgram*    m_pProgram2DTransSlice;
    GLSLProgram*    m_pProgramMIPSlice;
    GLSLProgram*    m_pProgramTransMIP;
    GLSLProgram*    m_pProgramIsoCompose;
    GLSLProgram*    m_pProgramCVCompose;
    GLSLProgram*    m_pProgramComposeAnaglyphs;

    void SetBrickDepShaderVarsSlice(const UINTVECTOR3& vVoxelCount);
    void RenderSeperatingLines();
    void RenderCoordArrows();
    void PreSubframe(ERenderArea);
    void PostSubframe();
};
#endif // GLRenderer_H
