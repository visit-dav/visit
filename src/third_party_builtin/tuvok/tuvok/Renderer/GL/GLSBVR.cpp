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
  \file    GLSBVR.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/


#include "GLInclude.h"
#include "GLSBVR.h"

#include <cmath>
#include <Basics/SysTools.h>
#include <Controller/Controller.h>
#include "../GPUMemMan/GPUMemMan.h"
#include "GLDebug.h"

using namespace std;

GLSBVR::GLSBVR(MasterController* pMasterController, bool bUseOnlyPowerOfTwo, bool bDownSampleTo8Bits, bool bDisableBorder) :
  GLRenderer(pMasterController, bUseOnlyPowerOfTwo, bDownSampleTo8Bits, bDisableBorder),
  m_pProgramIsoNoCompose(NULL)
{
}

GLSBVR::~GLSBVR() {
}

void GLSBVR::Cleanup() {
  GLRenderer::Cleanup();
  if (m_pProgramIsoNoCompose) {m_pMasterController->MemMan()->FreeGLSLProgram(m_pProgramIsoNoCompose); m_pProgramIsoNoCompose =NULL;}
}

bool GLSBVR::Initialize() {
  if (!GLRenderer::Initialize()) {
    T_ERROR("Error in parent call -> aborting");
    return false;
  }

  if (!LoadAndVerifyShader("GLSBVR-VS.glsl", "GLSBVR-1D-FS.glsl",       m_vShaderSearchDirs, &(m_pProgram1DTrans[0])) ||
      !LoadAndVerifyShader("GLSBVR-VS.glsl", "GLSBVR-1D-light-FS.glsl", m_vShaderSearchDirs, &(m_pProgram1DTrans[1])) ||
      !LoadAndVerifyShader("GLSBVR-VS.glsl", "GLSBVR-2D-FS.glsl",       m_vShaderSearchDirs, &(m_pProgram2DTrans[0])) ||
      !LoadAndVerifyShader("GLSBVR-VS.glsl", "GLSBVR-2D-light-FS.glsl", m_vShaderSearchDirs, &(m_pProgram2DTrans[1])) ||
      !LoadAndVerifyShader("GLSBVR-VS.glsl", "GLSBVR-MIP-Rot-FS.glsl",  m_vShaderSearchDirs, &(m_pProgramHQMIPRot)) ||
      !LoadAndVerifyShader("GLSBVR-VS.glsl", "GLSBVR-ISO-FS.glsl",      m_vShaderSearchDirs, &(m_pProgramIso)) ||
      !LoadAndVerifyShader("GLSBVR-VS.glsl", "GLSBVR-ISO-NC-FS.glsl",   m_vShaderSearchDirs, &(m_pProgramIsoNoCompose))) {

      Cleanup();

      T_ERROR("Error loading a shader.");
      return false;
  } else {
    m_pProgram1DTrans[0]->Enable();
    m_pProgram1DTrans[0]->SetUniformVector("texVolume",0);
    m_pProgram1DTrans[0]->SetUniformVector("texTrans1D",1);
    m_pProgram1DTrans[0]->Disable();

    m_pProgram1DTrans[1]->Enable();
    m_pProgram1DTrans[1]->SetUniformVector("texVolume",0);
    m_pProgram1DTrans[1]->SetUniformVector("texTrans1D",1);
    m_pProgram1DTrans[1]->SetUniformVector("vLightAmbient",0.2f,0.2f,0.2f);
    m_pProgram1DTrans[1]->SetUniformVector("vLightDiffuse",1.0f,1.0f,1.0f);
    m_pProgram1DTrans[1]->SetUniformVector("vLightSpecular",1.0f,1.0f,1.0f);
    m_pProgram1DTrans[1]->SetUniformVector("vLightDir",0.0f,0.0f,-1.0f);
    m_pProgram1DTrans[1]->Disable();

    m_pProgram2DTrans[0]->Enable();
    m_pProgram2DTrans[0]->SetUniformVector("texVolume",0);
    m_pProgram2DTrans[0]->SetUniformVector("texTrans2D",1);
    m_pProgram2DTrans[0]->Disable();

    m_pProgram2DTrans[1]->Enable();
    m_pProgram2DTrans[1]->SetUniformVector("texVolume",0);
    m_pProgram2DTrans[1]->SetUniformVector("texTrans2D",1);
    m_pProgram2DTrans[1]->SetUniformVector("vLightAmbient",0.2f,0.2f,0.2f);
    m_pProgram2DTrans[1]->SetUniformVector("vLightDiffuse",1.0f,1.0f,1.0f);
    m_pProgram2DTrans[1]->SetUniformVector("vLightSpecular",1.0f,1.0f,1.0f);
    m_pProgram2DTrans[1]->SetUniformVector("vLightDir",0.0f,0.0f,-1.0f);
    m_pProgram2DTrans[1]->Disable();

    m_pProgramIso->Enable();
    m_pProgramIso->SetUniformVector("texVolume",0);
    m_pProgramIso->Disable();

    m_pProgramHQMIPRot->Enable();
    m_pProgramHQMIPRot->SetUniformVector("texVolume",0);
    m_pProgramHQMIPRot->Disable();

    m_pProgramIsoNoCompose->Enable();
    m_pProgramIsoNoCompose->SetUniformVector("texVolume",0);
    m_pProgramIsoNoCompose->SetUniformVector("vLightAmbient",0.2f,0.2f,0.2f);
    m_pProgramIsoNoCompose->SetUniformVector("vLightDiffuse",1.0f,1.0f,1.0f);
    m_pProgramIsoNoCompose->SetUniformVector("vLightSpecular",1.0f,1.0f,1.0f);
    m_pProgramIsoNoCompose->SetUniformVector("vLightDir",0.0f,0.0f,-1.0f);
    m_pProgramIsoNoCompose->Disable();
  }

  return true;
}

void GLSBVR::SetSampleRateModifier(float fSampleRateModifier) {
  GLRenderer::SetSampleRateModifier(fSampleRateModifier);
  m_SBVRGeogen.SetSamplingModifier(fSampleRateModifier);
}

void GLSBVR::SetDataDepShaderVars() {
  GLRenderer::SetDataDepShaderVars();

  if (m_eRenderMode == RM_ISOSURFACE && m_bAvoidSeperateCompositing) {
    m_pProgramIsoNoCompose->Enable();
    m_pProgramIsoNoCompose->SetUniformVector("fIsoval",m_fScaledIsovalue);
    // this is not really a data dependent var but as we only need to do it once per frame we may also do it here
    m_pProgramIsoNoCompose->SetUniformVector("vLightDiffuse",m_vIsoColor.x, m_vIsoColor.y, m_vIsoColor.z);
    m_pProgramIsoNoCompose->Disable();
  }
}

void GLSBVR::SetBrickDepShaderVars(const Brick& currentBrick) {
  FLOATVECTOR3 vStep(1.0f/currentBrick.vVoxelCount.x, 1.0f/currentBrick.vVoxelCount.y, 1.0f/currentBrick.vVoxelCount.z);

  float fStepScale = m_SBVRGeogen.GetOpacityCorrection();

  switch (m_eRenderMode) {
    case RM_1DTRANS    :  {
                            m_pProgram1DTrans[m_bUseLighting ? 1 : 0]->SetUniformVector("fStepScale", fStepScale);
                            if (m_bUseLighting)
                                m_pProgram1DTrans[1]->SetUniformVector("vVoxelStepsize", vStep.x, vStep.y, vStep.z);
                            break;
                          }
    case RM_2DTRANS    :  {
                            m_pProgram2DTrans[m_bUseLighting ? 1 : 0]->SetUniformVector("fStepScale", fStepScale);
                            m_pProgram2DTrans[m_bUseLighting ? 1 : 0]->SetUniformVector("vVoxelStepsize", vStep.x, vStep.y, vStep.z);
                            break;
                          }
    case RM_ISOSURFACE : {
                            if (m_bAvoidSeperateCompositing)
                              m_pProgramIsoNoCompose->SetUniformVector("vVoxelStepsize", vStep.x, vStep.y, vStep.z);
                            else
                              m_pProgramIso->SetUniformVector("vVoxelStepsize", vStep.x, vStep.y, vStep.z);
                            break;
                          }
    case RM_INVALID    :  T_ERROR("Invalid rendermode set"); break;
  }

}

void GLSBVR::EnableClipPlane() {
  if(!m_bClipPlaneOn) {
    AbstrRenderer::EnableClipPlane();
    m_SBVRGeogen.EnableClipPlane();
    PLANE<float> plane(m_ClipPlane.Plane());
    m_SBVRGeogen.SetClipPlane(plane);
  }
}

void GLSBVR::DisableClipPlane() {
  if(m_bClipPlaneOn) {
    AbstrRenderer::DisableClipPlane();
    m_SBVRGeogen.DisableClipPlane();
  }
}

void GLSBVR::Render3DPreLoop() {
  if(m_bClipPlaneOn) {
    m_SBVRGeogen.EnableClipPlane();
    PLANE<float> plane(m_ClipPlane.Plane());
    m_SBVRGeogen.SetClipPlane(plane);
  } else {
    m_SBVRGeogen.DisableClipPlane();
  }

  switch (m_eRenderMode) {
    case RM_1DTRANS    :  m_p1DTransTex->Bind(1);
                          m_pProgram1DTrans[m_bUseLighting ? 1 : 0]->Enable();
                          GL(glEnable(GL_BLEND));
                          GL(glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE));
                          break;
    case RM_2DTRANS    :  m_p2DTransTex->Bind(1);
                          m_pProgram2DTrans[m_bUseLighting ? 1 : 0]->Enable();
                          GL(glEnable(GL_BLEND));
                          GL(glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE));
                          break;
    case RM_ISOSURFACE :  if (m_bAvoidSeperateCompositing) {
                            m_pProgramIsoNoCompose->Enable();
                            glEnable(GL_BLEND);
                            glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                          } else {
                            glEnable(GL_DEPTH_TEST);
                          }
                          break;
    default    :  T_ERROR("Invalid rendermode set");
                          break;
  }

  m_SBVRGeogen.SetLODData( UINTVECTOR3(m_pDataset->GetInfo()->GetDomainSize(m_iCurrentLOD))  );
}

void GLSBVR::RenderProxyGeometry() {
  MESSAGE("Rendering %zu triangles.", m_SBVRGeogen.m_vSliceTriangles.size());
  glBegin(GL_TRIANGLES);
    for (int i = int(m_SBVRGeogen.m_vSliceTriangles.size())-1;i>=0;i--) {
      glTexCoord3f(m_SBVRGeogen.m_vSliceTriangles[i].m_vTex.x,
                   m_SBVRGeogen.m_vSliceTriangles[i].m_vTex.y,
                   m_SBVRGeogen.m_vSliceTriangles[i].m_vTex.z);
      glVertex3f(m_SBVRGeogen.m_vSliceTriangles[i].m_vPos.x,
                 m_SBVRGeogen.m_vSliceTriangles[i].m_vPos.y,
                 m_SBVRGeogen.m_vSliceTriangles[i].m_vPos.z);
#if 0
      MESSAGE("tri(tex): %3.3f,%3.3f,%3.3f (%3.3f,%3.3f,%3.3f)",
              m_SBVRGeogen.m_vSliceTriangles[i].m_vPos.x,
              m_SBVRGeogen.m_vSliceTriangles[i].m_vPos.y,
              m_SBVRGeogen.m_vSliceTriangles[i].m_vPos.z,
              m_SBVRGeogen.m_vSliceTriangles[i].m_vTex.x,
              m_SBVRGeogen.m_vSliceTriangles[i].m_vTex.y,
              m_SBVRGeogen.m_vSliceTriangles[i].m_vTex.z);
#endif
    }
  GL(glEnd());
}

void GLSBVR::Render3DInLoop(size_t iCurrentBrick, int iStereoID) {
  const Brick& b = (iStereoID == 0) ? m_vCurrentBrickList[iCurrentBrick] : m_vLeftEyeBrickList[iCurrentBrick];

  // setup the slice generator
  m_SBVRGeogen.SetBrickData(b.vExtension, b.vVoxelCount,
                            b.vTexcoordsMin, b.vTexcoordsMax);
  FLOATMATRIX4 maBricktTrans;
  maBricktTrans.Translation(b.vCenter.x, b.vCenter.y, b.vCenter.z);
  FLOATMATRIX4 maBricktModelView = maBricktTrans * m_matModelView[iStereoID];
  m_mProjection[iStereoID].setProjection();
  maBricktModelView.setModelview();

  m_SBVRGeogen.SetWorld(maBricktTrans * m_mRotation * m_mTranslation);
  m_SBVRGeogen.SetView(m_mView[iStereoID], true);

  if (! m_bAvoidSeperateCompositing && m_eRenderMode == RM_ISOSURFACE) {
    m_TargetBinder.Bind(m_pFBOIsoHit[iStereoID], 0, m_pFBOIsoHit[iStereoID], 1);

    if (m_iBricksRenderedInThisSubFrame == 0) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_pProgramIso->Enable();
    SetBrickDepShaderVars(b);
    m_pProgramIso->SetUniformVector("fIsoval",m_fScaledIsovalue);
    RenderProxyGeometry();
    m_pProgramIso->Disable();

    if (m_bDoClearView) {
      m_TargetBinder.Bind(m_pFBOCVHit[iStereoID], 0, m_pFBOCVHit[iStereoID], 1);

      if (m_iBricksRenderedInThisSubFrame == 0) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      m_pProgramIso->Enable();
      m_pProgramIso->SetUniformVector("fIsoval",m_fScaledCVIsovalue);
      RenderProxyGeometry();
      m_pProgramIso->Disable();
    }
  } else {
    m_TargetBinder.Bind(m_pFBO3DImageCurrent[iStereoID]);

    GL(glDepthMask(GL_FALSE));
    SetBrickDepShaderVars(b);
    RenderProxyGeometry();
    GL(glDepthMask(GL_TRUE));
  }
  m_TargetBinder.Unbind();
}


void GLSBVR::Render3DPostLoop() {
  GLRenderer::Render3DPostLoop();

  // disable the shader
  switch (m_eRenderMode) {
    case RM_1DTRANS    :  m_pProgram1DTrans[m_bUseLighting ? 1 : 0]->Disable();
                          GL(glDisable(GL_BLEND));
                          break;
    case RM_2DTRANS    :  m_pProgram2DTrans[m_bUseLighting ? 1 : 0]->Disable();
                          glDisable(GL_BLEND);
                          break;
    case RM_ISOSURFACE :  if (m_bAvoidSeperateCompositing) {
                             m_pProgramIsoNoCompose->Disable();
                             glDisable(GL_BLEND);
                          }
                          break;
    case RM_INVALID    :  T_ERROR("Invalid rendermode set"); break;
  }
}

void GLSBVR::RenderHQMIPPreLoop(EWindowMode eDirection) {
  GLRenderer::RenderHQMIPPreLoop(eDirection);
  m_pProgramHQMIPRot->Enable();

  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_MAX);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
}

void GLSBVR::RenderHQMIPInLoop(const Brick& b) {
  m_SBVRGeogen.SetBrickData(b.vExtension, b.vVoxelCount, b.vTexcoordsMin, b.vTexcoordsMax);
  FLOATMATRIX4 maBricktTrans;
  maBricktTrans.Translation(b.vCenter.x, b.vCenter.y, b.vCenter.z);
  if (m_bOrthoView)
    m_SBVRGeogen.SetView(FLOATMATRIX4());
  else
    m_SBVRGeogen.SetView(m_mView[0]);

  m_SBVRGeogen.SetWorld(maBricktTrans * m_maMIPRotation, true);

  RenderProxyGeometry();
}

void GLSBVR::RenderHQMIPPostLoop() {
  GLRenderer::RenderHQMIPPostLoop();
  m_pProgramHQMIPRot->Disable();
}


bool GLSBVR::LoadDataset(const string& strFilename) {
  if (GLRenderer::LoadDataset(strFilename)) {
    UINTVECTOR3    vSize = UINTVECTOR3(m_pDataset->GetInfo()->GetDomainSize());
    FLOATVECTOR3 vAspect = FLOATVECTOR3(m_pDataset->GetInfo()->GetScale());
    vAspect /= vAspect.maxVal();

    m_SBVRGeogen.SetVolumeData(vAspect, vSize);
    return true;
  } else return false;
}

void GLSBVR::ComposeSurfaceImage(int iStereoID) {
  if (!m_bAvoidSeperateCompositing) GLRenderer::ComposeSurfaceImage(iStereoID);
}
