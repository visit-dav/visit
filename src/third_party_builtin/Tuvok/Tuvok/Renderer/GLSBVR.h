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
  \file    GLSBVR.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.0
  \date    August 2008
*/


#pragma once

#ifndef GLSBVR_H
#define GLSBVR_H

#include "../Renderer/GLRenderer.h"
#include "../Renderer/SBVRGeogen.h"

/** \class GLSBVR
 * Slice-based GPU volume renderer.
 *
 * GLSBVR is a slice based volume renderer which uses GLSL. */
class GLSBVR : public GLRenderer {
  public:
    /** Constructs a VRer with immediate redraw, and
     * wireframe mode off.
     * \param pMasterController message routing object */
    GLSBVR(MasterController* pMasterController, bool bUseOnlyPowerOfTwo);
    virtual ~GLSBVR();

    /** Loads GLSL vertex and fragment shaders. */
    virtual bool Initialize();

    /** Set the oversampling ratio (e.g. 2 means twice the slices as needed).  Causes a full redraw. */
    virtual void SetSampleRateModifier(float fSampleRateModifier);

    virtual void SetDataDepShaderVars();

    /** Sends a message to the master to ask for a dataset to be loaded.
     * The dataset is converted to UVF if it is not one already.
     * @param strFilename path to a file */
    virtual bool LoadDataset(const std::string& strFilename);

    virtual bool SupportsClearView() {return !m_bAvoidSeperateCompositing;}

  protected:
    SBVRGeogen    m_SBVRGeogen;
    GLSLProgram*  m_pProgramIsoNoCompose;

    void SetBrickDepShaderVars(size_t iCurrentBrick);

    virtual void Render3DPreLoop();
    virtual void Render3DInLoop(size_t iCurrentBrick);
    virtual void Render3DPostLoop();
    void RenderProxyGeometry();
    virtual void Cleanup();

    virtual void ComposeSurfaceImage();

};

#endif // GLSBVR_H
