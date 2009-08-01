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
  \file    GLRaycaster.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/

#include "GLInclude.h"
#include "GLRaycaster.h"
#include "GLFBOTex.h"

#include <Controller/Controller.h>
#include "../GPUMemMan/GPUMemMan.h"
#include "../../Basics/Plane.h"
#include "GLSLProgram.h"
#include "GLTexture1D.h"
#include "GLTexture2D.h"

using namespace std;

GLRaycaster::GLRaycaster(MasterController* pMasterController, bool bUseOnlyPowerOfTwo, bool bDownSampleTo8Bits, bool bDisableBorder, bool bNoRCClipplanes) :
  GLRenderer(pMasterController,bUseOnlyPowerOfTwo, bDownSampleTo8Bits, bDisableBorder),
  m_pFBORayEntry(NULL),
  m_pProgramRenderFrontFaces(NULL),
  m_pProgramIso2(NULL),
  m_bNoRCClipplanes(bNoRCClipplanes)
{
}

GLRaycaster::~GLRaycaster() {
}


void GLRaycaster::Cleanup() {
  GLRenderer::Cleanup();

  if (m_pFBORayEntry){m_pMasterController->MemMan()->FreeFBO(m_pFBORayEntry); m_pFBORayEntry = NULL;}
  if (m_pProgramRenderFrontFaces){m_pMasterController->MemMan()->FreeGLSLProgram(m_pProgramRenderFrontFaces); m_pProgramRenderFrontFaces = NULL;}
  if (m_pProgramIso2) {m_pMasterController->MemMan()->FreeGLSLProgram(m_pProgramIso2); m_pProgramIso2 = NULL;}
}

void GLRaycaster::CreateOffscreenBuffers() {
  GLRenderer::CreateOffscreenBuffers();
  if (m_pFBORayEntry){m_pMasterController->MemMan()->FreeFBO(m_pFBORayEntry); m_pFBORayEntry = NULL;}
  if (m_vWinSize.area() > 0) {
    m_pFBORayEntry = m_pMasterController->MemMan()->GetFBO(GL_NEAREST, GL_NEAREST, GL_CLAMP, m_vWinSize.x, m_vWinSize.y, GL_RGBA16F_ARB, 2*4, false);
  }
}

bool GLRaycaster::Initialize() {
  if (!GLRenderer::Initialize()) {
    T_ERROR("Error in parent call -> aborting");
    return false;
  }

  glShadeModel(GL_SMOOTH);

  string shaderNames[7];
  if (m_bNoRCClipplanes) {
   shaderNames[0] = "GLRaycaster-1D-FS-NC.glsl";
   shaderNames[1] = "GLRaycaster-1D-light-FS-NC.glsl";
   shaderNames[2] = "GLRaycaster-2D-FS-NC.glsl";
   shaderNames[3] = "GLRaycaster-2D-light-FS-NC.glsl";
   shaderNames[4] = "GLRaycaster-Color-FS-NC.glsl";
   shaderNames[5] = "GLRaycaster-ISO-CV-FS-NC.glsl";
   shaderNames[6] = "GLRaycaster-ISO-FS-NC.glsl";
  } else {
   shaderNames[0] = "GLRaycaster-1D-FS.glsl";
   shaderNames[1] = "GLRaycaster-1D-light-FS.glsl";
   shaderNames[2] = "GLRaycaster-2D-FS.glsl";
   shaderNames[3] = "GLRaycaster-2D-light-FS.glsl";
   shaderNames[4] = "GLRaycaster-Color-FS.glsl";
   shaderNames[5] = "GLRaycaster-ISO-CV-FS.glsl";
   shaderNames[6] = "GLRaycaster-ISO-FS.glsl";
  }


  if (!LoadAndVerifyShader("GLRaycaster-VS.glsl", "GLRaycaster-frontfaces-FS.glsl", m_vShaderSearchDirs, &(m_pProgramRenderFrontFaces)) ||
      !LoadAndVerifyShader("GLRaycaster-VS.glsl", shaderNames[0],   m_vShaderSearchDirs, &(m_pProgram1DTrans[0])) ||
      !LoadAndVerifyShader("GLRaycaster-VS.glsl", shaderNames[1],   m_vShaderSearchDirs, &(m_pProgram1DTrans[1])) ||
      !LoadAndVerifyShader("GLRaycaster-VS.glsl", shaderNames[2],   m_vShaderSearchDirs, &(m_pProgram2DTrans[0])) ||
      !LoadAndVerifyShader("GLRaycaster-VS.glsl", shaderNames[3],   m_vShaderSearchDirs, &(m_pProgram2DTrans[1])) ||
      !LoadAndVerifyShader("GLRaycaster-VS.glsl", shaderNames[6],   m_vShaderSearchDirs, &(m_pProgramIso)) ||
      !LoadAndVerifyShader("GLRaycaster-VS.glsl", shaderNames[4],   m_vShaderSearchDirs, &(m_pProgramColor)) ||
      !LoadAndVerifyShader("GLRaycaster-VS.glsl", shaderNames[5],   m_vShaderSearchDirs, &(m_pProgramIso2)) ||
      !LoadAndVerifyShader("GLRaycaster-VS.glsl", "GLRaycaster-MIP-Rot-FS.glsl",    m_vShaderSearchDirs, &(m_pProgramHQMIPRot))) {

      Cleanup();

      T_ERROR("Error loading a shader.");
      return false;
  } else {

    m_pProgram1DTrans[0]->Enable();
    m_pProgram1DTrans[0]->SetUniformVector("texVolume",0);
    m_pProgram1DTrans[0]->SetUniformVector("texTrans1D",1);
    m_pProgram1DTrans[0]->SetUniformVector("texRayExitPos",2);
    m_pProgram1DTrans[0]->Disable();

    m_pProgram1DTrans[1]->Enable();
    m_pProgram1DTrans[1]->SetUniformVector("texVolume",0);
    m_pProgram1DTrans[1]->SetUniformVector("texTrans1D",1);
    m_pProgram1DTrans[1]->SetUniformVector("texRayExitPos",2);
    m_pProgram1DTrans[1]->SetUniformVector("vLightAmbient",0.2f,0.2f,0.2f);
    m_pProgram1DTrans[1]->SetUniformVector("vLightDiffuse",1.0f,1.0f,1.0f);
    m_pProgram1DTrans[1]->SetUniformVector("vLightSpecular",1.0f,1.0f,1.0f);
    m_pProgram1DTrans[1]->SetUniformVector("vLightDir",0.0f,0.0f,-1.0f);
    m_pProgram1DTrans[1]->Disable();

    m_pProgram2DTrans[0]->Enable();
    m_pProgram2DTrans[0]->SetUniformVector("texVolume",0);
    m_pProgram2DTrans[0]->SetUniformVector("texTrans2D",1);
    m_pProgram2DTrans[0]->SetUniformVector("texRayExitPos",2);
    m_pProgram2DTrans[0]->Disable();

    m_pProgram2DTrans[1]->Enable();
    m_pProgram2DTrans[1]->SetUniformVector("texVolume",0);
    m_pProgram2DTrans[1]->SetUniformVector("texTrans2D",1);
    m_pProgram2DTrans[1]->SetUniformVector("texRayExitPos",2);
    m_pProgram2DTrans[1]->SetUniformVector("vLightAmbient",0.2f,0.2f,0.2f);
    m_pProgram2DTrans[1]->SetUniformVector("vLightDiffuse",1.0f,1.0f,1.0f);
    m_pProgram2DTrans[1]->SetUniformVector("vLightSpecular",1.0f,1.0f,1.0f);
    m_pProgram2DTrans[1]->SetUniformVector("vLightDir",0.0f,0.0f,-1.0f);
    m_pProgram2DTrans[1]->Disable();

    FLOATVECTOR2 vParams = m_FrustumCullingLOD.GetDepthScaleParams();

    m_pProgramIso->Enable();
    m_pProgramIso->SetUniformVector("texVolume",0);
    m_pProgramIso->SetUniformVector("texRayExitPos",2);
    m_pProgramIso->SetUniformVector("vProjParam",vParams.x, vParams.y);
    m_pProgramIso->SetUniformVector("iTileID", 1); // just to make sure it is never 0
    m_pProgramIso->Disable();

    m_pProgramColor->Enable();
    m_pProgramColor->SetUniformVector("texVolume",0);
    m_pProgramColor->SetUniformVector("texRayExitPos",2);
    m_pProgramColor->SetUniformVector("vProjParam",vParams.x, vParams.y);
    m_pProgramColor->Disable();

    m_pProgramHQMIPRot->Enable();
    m_pProgramHQMIPRot->SetUniformVector("texVolume",0);
    m_pProgramHQMIPRot->SetUniformVector("texRayExitPos",2);
    m_pProgramHQMIPRot->Disable();

    m_pProgramIso2->Enable();
    m_pProgramIso2->SetUniformVector("texVolume",0);
    m_pProgramIso2->SetUniformVector("texRayExitPos",2);
    m_pProgramIso2->SetUniformVector("texLastHit",4);
    m_pProgramIso2->SetUniformVector("texLastHitPos",5);
    m_pProgramIso2->Disable();

    /// We always clip against the plane in the shader, so initialize the plane
    /// to be way out in left field, ensuring nothing will be clipped.
    ClipPlaneToShader(ExtendedPlane(PLANE<float>(0,0,1,-100000)),0,true);

  }

  return true;
}

void GLRaycaster::SetBrickDepShaderVars(const Brick& currentBrick, size_t iCurrentBrick) {
  FLOATVECTOR3 vVoxelSizeTexSpace = 1.0f/FLOATVECTOR3(currentBrick.vVoxelCount);
  float fRayStep = (currentBrick.vExtension*vVoxelSizeTexSpace * 0.5f * 1.0f/m_fSampleRateModifier).minVal();
  float fStepScale = 1.0f/m_fSampleRateModifier * (FLOATVECTOR3(m_pDataset->GetInfo().GetDomainSize())/FLOATVECTOR3(m_pDataset->GetInfo().GetDomainSize(m_iCurrentLOD))).maxVal();

  switch (m_eRenderMode) {
    case RM_1DTRANS    :  {
                            m_pProgram1DTrans[m_bUseLighting ? 1 : 0]->SetUniformVector("fStepScale", fStepScale);
                            m_pProgram1DTrans[m_bUseLighting ? 1 : 0]->SetUniformVector("fRayStepsize", fRayStep);
                            if (m_bUseLighting)
                                m_pProgram1DTrans[1]->SetUniformVector("vVoxelStepsize", vVoxelSizeTexSpace.x, vVoxelSizeTexSpace.y, vVoxelSizeTexSpace.z);
                            break;
                          }
    case RM_2DTRANS    :  {
                            m_pProgram2DTrans[m_bUseLighting ? 1 : 0]->SetUniformVector("fStepScale", fStepScale);
                            m_pProgram2DTrans[m_bUseLighting ? 1 : 0]->SetUniformVector("vVoxelStepsize", vVoxelSizeTexSpace.x, vVoxelSizeTexSpace.y, vVoxelSizeTexSpace.z);
                            m_pProgram2DTrans[m_bUseLighting ? 1 : 0]->SetUniformVector("fRayStepsize", fRayStep);
                            break;
                          }
    case RM_ISOSURFACE : {
                            GLSLProgram* shader = (m_pDataset->GetInfo().GetComponentCount() == 1) ? m_pProgramIso : m_pProgramColor;
                            if (m_bDoClearView) {
                              m_pProgramIso2->Enable();
                              m_pProgramIso2->SetUniformVector("vVoxelStepsize", vVoxelSizeTexSpace.x, vVoxelSizeTexSpace.y, vVoxelSizeTexSpace.z);
                              m_pProgramIso2->SetUniformVector("fRayStepsize", fRayStep);
                              m_pProgramIso2->SetUniformVector("iTileID", int(iCurrentBrick));
                              m_pProgramIso2->Disable();
                              shader->Enable();
                              shader->SetUniformVector("iTileID", int(iCurrentBrick));
                            }
                            shader->SetUniformVector("vVoxelStepsize", vVoxelSizeTexSpace.x, vVoxelSizeTexSpace.y, vVoxelSizeTexSpace.z);
                            shader->SetUniformVector("fRayStepsize", fRayStep);

                            break;
                          }
    case RM_INVALID    :  T_ERROR("Invalid rendermode set"); break;
  }

}

void GLRaycaster::RenderBox(const FLOATVECTOR3& vCenter, const FLOATVECTOR3& vExtend, const FLOATVECTOR3& vMinCoords, const FLOATVECTOR3& vMaxCoords, bool bCullBack, int iStereoID) const  {
  if (bCullBack) {
    glCullFace(GL_BACK);
  } else {
    glCullFace(GL_FRONT);
  }

  FLOATVECTOR3 vMinPoint, vMaxPoint;
  vMinPoint = (vCenter - vExtend/2.0);
  vMaxPoint = (vCenter + vExtend/2.0);

  // \todo compute this only once per brick
  FLOATMATRIX4 m = ComputeEyeToTextureMatrix(FLOATVECTOR3(vMaxPoint.x, vMaxPoint.y, vMaxPoint.z),
                                             FLOATVECTOR3(vMaxCoords.x, vMaxCoords.y, vMaxCoords.z),
                                             FLOATVECTOR3(vMinPoint.x, vMinPoint.y, vMinPoint.z),
                                             FLOATVECTOR3(vMinCoords.x, vMinCoords.y, vMinCoords.z),
                                             iStereoID);

  m.setTextureMatrix();

  glBegin(GL_QUADS);
    // BACK
    glVertex3f(vMaxPoint.x, vMinPoint.y, vMinPoint.z);
    glVertex3f(vMinPoint.x, vMinPoint.y, vMinPoint.z);
    glVertex3f(vMinPoint.x, vMaxPoint.y, vMinPoint.z);
    glVertex3f(vMaxPoint.x, vMaxPoint.y, vMinPoint.z);
    // FRONT
    glVertex3f(vMaxPoint.x, vMaxPoint.y, vMaxPoint.z);
    glVertex3f(vMinPoint.x, vMaxPoint.y, vMaxPoint.z);
    glVertex3f(vMinPoint.x, vMinPoint.y, vMaxPoint.z);
    glVertex3f(vMaxPoint.x, vMinPoint.y, vMaxPoint.z);
    // LEFT
    glVertex3f(vMinPoint.x, vMaxPoint.y, vMinPoint.z);
    glVertex3f(vMinPoint.x, vMinPoint.y, vMinPoint.z);
    glVertex3f(vMinPoint.x, vMinPoint.y, vMaxPoint.z);
    glVertex3f(vMinPoint.x, vMaxPoint.y, vMaxPoint.z);
    // RIGHT
    glVertex3f(vMaxPoint.x, vMaxPoint.y, vMaxPoint.z);
    glVertex3f(vMaxPoint.x, vMinPoint.y, vMaxPoint.z);
    glVertex3f(vMaxPoint.x, vMinPoint.y, vMinPoint.z);
    glVertex3f(vMaxPoint.x, vMaxPoint.y, vMinPoint.z);
    // BOTTOM
    glVertex3f(vMaxPoint.x, vMinPoint.y, vMaxPoint.z);
    glVertex3f(vMinPoint.x, vMinPoint.y, vMaxPoint.z);
    glVertex3f(vMinPoint.x, vMinPoint.y, vMinPoint.z);
    glVertex3f(vMaxPoint.x, vMinPoint.y, vMinPoint.z);
    // TOP
    glVertex3f(vMaxPoint.x, vMaxPoint.y, vMinPoint.z);
    glVertex3f(vMinPoint.x, vMaxPoint.y, vMinPoint.z);
    glVertex3f(vMinPoint.x, vMaxPoint.y, vMaxPoint.z);
    glVertex3f(vMaxPoint.x, vMaxPoint.y, vMaxPoint.z);
  glEnd();
}


/// Set the clip plane input variable in the shader.
void GLRaycaster::ClipPlaneToShader(const ExtendedPlane& clipPlane, int iStereoID, bool bForce) {
  if (m_bNoRCClipplanes) return;

  vector<GLSLProgram*> vCurrentShader;

  if (bForce) {
    vCurrentShader.push_back(m_pProgram1DTrans[0]);
    vCurrentShader.push_back(m_pProgram1DTrans[1]);
    vCurrentShader.push_back(m_pProgram2DTrans[0]);
    vCurrentShader.push_back(m_pProgram2DTrans[1]);
    vCurrentShader.push_back(m_pProgramIso);
    vCurrentShader.push_back(m_pProgramColor);
    vCurrentShader.push_back(m_pProgramIso2);
  } else {
    switch (m_eRenderMode) {
      case RM_1DTRANS    :  vCurrentShader.push_back(m_pProgram1DTrans[m_bUseLighting ? 1 : 0]);
                            break;
      case RM_2DTRANS    :  vCurrentShader.push_back(m_pProgram2DTrans[m_bUseLighting ? 1 : 0]);
                            break;
      case RM_ISOSURFACE :  if (m_pDataset->GetInfo().GetComponentCount() == 1)
                              vCurrentShader.push_back(m_pProgramIso);
                            else
                              vCurrentShader.push_back(m_pProgramColor);
                            if (m_bDoClearView) vCurrentShader.push_back(m_pProgramIso2);
                            break;
      default    :          T_ERROR("Invalid rendermode set");
                            break;
    }
  }

  if (bForce || m_bClipPlaneOn) {
    ExtendedPlane plane(clipPlane);

    plane.Transform(m_mView[iStereoID]);
    for (size_t i = 0;i<vCurrentShader.size();i++) {
      vCurrentShader[i]->Enable();
      vCurrentShader[i]->SetUniformVector("vClipPlane", plane.x(), plane.y(),
                                                        plane.z(), plane.d());
      vCurrentShader[i]->Disable();
    }
  }
}


void GLRaycaster::Render3DPreLoop() {
  switch (m_eRenderMode) {
    case RM_1DTRANS    :  m_p1DTransTex->Bind(1);
                          glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                          break;
    case RM_2DTRANS    :  m_p2DTransTex->Bind(1);
                          glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                          break;
    case RM_ISOSURFACE :  break;
    default    :          T_ERROR("Invalid rendermode set");
                          break;
  }


  /*
  if (m_iBricksRenderedInThisSubFrame == 0) {
    m_TargetBinder.Bind(m_pFBORayEntry);

    // render nearplane into buffer
    float fNear = m_FrustumCullingLOD.GetNearPlane() + 0.01f;

    FLOATMATRIX4 mInvModelView = m_matModelView[0].inverse();

    FLOATVECTOR4 vMin(-1, -1, fNear, 1);
    FLOATVECTOR4 vMax( 1,  1, fNear, 1);

    vMin = vMin * mInvModelView;
    vMax = vMax * mInvModelView;

    m_pProgramRenderFrontFaces->Enable();
    glBegin(GL_QUADS);
      glVertex4f(  vMax.x,  vMax.y, vMax.z, vMax.w);
      glVertex4f(  vMin.x, vMax.y, vMax.z, vMax.w);
      glVertex4f(  vMin.x, vMin.y, vMax.z, vMax.w);
      glVertex4f(  vMax.x, vMin.y, vMax.z, vMax.w);
    glEnd();
    m_pProgramRenderFrontFaces->Disable();

    m_TargetBinder.Bind(m_pFBO3DImageCurrent);
  }*/

  glEnable(GL_CULL_FACE);
}

void GLRaycaster::Render3DInLoop(size_t iCurrentBrick, int iStereoID) {
  const Brick& b = (iStereoID == 0) ? m_vCurrentBrickList[iCurrentBrick] : m_vLeftEyeBrickList[iCurrentBrick];

  glDisable(GL_BLEND);
  glDepthMask(GL_FALSE);

  m_matModelView[iStereoID].setModelview();
  m_mProjection[iStereoID].setProjection();

  if (m_bClipPlaneOn) ClipPlaneToShader(m_ClipPlane, iStereoID);

  // write frontfaces (ray entry points)
  m_TargetBinder.Bind(m_pFBORayEntry);
  m_pProgramRenderFrontFaces->Enable();
  RenderBox(b.vCenter, b.vExtension,
            b.vTexcoordsMin, b.vTexcoordsMax,
            false, iStereoID);
  m_pProgramRenderFrontFaces->Disable();

  if (m_eRenderMode == RM_ISOSURFACE) {
    glDepthMask(GL_TRUE);
    m_TargetBinder.Bind(m_pFBOIsoHit[iStereoID], 0, m_pFBOIsoHit[iStereoID], 1);

    if (m_iBricksRenderedInThisSubFrame == 0) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLSLProgram* shader = (m_pDataset->GetInfo().GetComponentCount() == 1) ? m_pProgramIso : m_pProgramColor;
    shader->Enable();
    SetBrickDepShaderVars(b, iCurrentBrick);
    m_pFBORayEntry->Read(2);
    RenderBox(b.vCenter, b.vExtension,
              b.vTexcoordsMin, b.vTexcoordsMax,
              true, iStereoID);
    m_pFBORayEntry->FinishRead();
    shader->Disable();

    if (m_bDoClearView) {
      m_TargetBinder.Bind(m_pFBOCVHit[iStereoID], 0, m_pFBOCVHit[iStereoID], 1);

      if (m_iBricksRenderedInThisSubFrame == 0) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      m_pProgramIso2->Enable();
      m_pFBORayEntry->Read(2);
      m_pFBOIsoHit[iStereoID]->Read(4, 0);
      m_pFBOIsoHit[iStereoID]->Read(5, 1);
      RenderBox(b.vCenter, b.vExtension,
                b.vTexcoordsMin, b.vTexcoordsMax,
                true, iStereoID);
      m_pFBOIsoHit[iStereoID]->FinishRead(1);
      m_pFBOIsoHit[iStereoID]->FinishRead(0);
      m_pFBORayEntry->FinishRead();
      m_pProgramIso2->Disable();
    }

  } else {
    m_TargetBinder.Bind(m_pFBO3DImageCurrent[iStereoID]);

    // do the raycasting
    switch (m_eRenderMode) {
      case RM_1DTRANS    :  m_pProgram1DTrans[m_bUseLighting ? 1 : 0]->Enable();
                            break;
      case RM_2DTRANS    :  m_pProgram2DTrans[m_bUseLighting ? 1 : 0]->Enable();
                            break;
      default            :  T_ERROR("Invalid rendermode set");
                            break;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);

    SetBrickDepShaderVars(b, iCurrentBrick);

    m_pFBORayEntry->Read(2);
    RenderBox(b.vCenter, b.vExtension,
              b.vTexcoordsMin, b.vTexcoordsMax,
              true, iStereoID);
    m_pFBORayEntry->FinishRead();

    switch (m_eRenderMode) {
      case RM_1DTRANS    :  m_pProgram1DTrans[m_bUseLighting ? 1 : 0]->Disable();
                            break;
      case RM_2DTRANS    :  m_pProgram2DTrans[m_bUseLighting ? 1 : 0]->Disable();
                            break;
      default            :  T_ERROR("Invalid rendermode set");
                            break;
    }
    glDisable(GL_BLEND);

  }
  m_TargetBinder.Unbind();
}

void GLRaycaster::Render3DPostLoop() {
  GLRenderer::Render3DPostLoop();

  glDisable(GL_CULL_FACE);
  glDepthMask(GL_TRUE);
  glEnable(GL_BLEND);
}

void GLRaycaster::RenderHQMIPPreLoop(EWindowMode eDirection) {
  GLRenderer::RenderHQMIPPreLoop(eDirection);
  m_pProgramHQMIPRot->Enable();
  m_pProgramHQMIPRot->SetUniformVector("vScreensize",float(m_vWinSize.x), float(m_vWinSize.y));
  m_pProgramHQMIPRot->Disable();
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  if (m_bOrthoView)
    m_matModelView[0] = m_maMIPRotation;
  else
    m_matModelView[0] = m_maMIPRotation * m_mView[0];

  m_matModelView[0].setModelview();
}

void GLRaycaster::RenderHQMIPInLoop(const Brick& b) {
  glDisable(GL_BLEND);

  // write frontfaces (ray entry points)
  m_TargetBinder.Bind(m_pFBORayEntry);

  m_pProgramRenderFrontFaces->Enable();
  RenderBox(b.vCenter, b.vExtension, b.vTexcoordsMin, b.vTexcoordsMax, false, 0);
  m_pProgramRenderFrontFaces->Disable();

  m_TargetBinder.Bind(m_pFBO3DImageCurrent[1]);  // for MIP rendering "abuse" left-eye buffer for the itermediate results
  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_MAX);
  glEnable(GL_BLEND);

  m_pProgramHQMIPRot->Enable();

  FLOATVECTOR3 vVoxelSizeTexSpace = 1.0f/FLOATVECTOR3(b.vVoxelCount);
  float fRayStep = (b.vExtension*vVoxelSizeTexSpace * 0.5f * 1.0f/m_fSampleRateModifier).minVal();
  m_pProgramHQMIPRot->SetUniformVector("fRayStepsize", fRayStep);

  m_pFBORayEntry->Read(2);
  RenderBox(b.vCenter, b.vExtension,b.vTexcoordsMin, b.vTexcoordsMax, true, 0);
  m_pFBORayEntry->FinishRead();
  m_pProgramHQMIPRot->Disable();
}

void GLRaycaster::RenderHQMIPPostLoop() {
  GLRenderer::RenderHQMIPPostLoop();
  glDisable(GL_CULL_FACE);
  glDepthMask(GL_TRUE);
}

void GLRaycaster::StartFrame() {
  GLRenderer::StartFrame();

  FLOATVECTOR2 vfWinSize = FLOATVECTOR2(m_vWinSize);

  switch (m_eRenderMode) {
    case RM_1DTRANS    :  m_pProgram1DTrans[0]->Enable();
                          m_pProgram1DTrans[0]->SetUniformVector("vScreensize",vfWinSize.x, vfWinSize.y);
                          m_pProgram1DTrans[0]->Disable();

                          m_pProgram1DTrans[1]->Enable();
                          m_pProgram1DTrans[1]->SetUniformVector("vScreensize",vfWinSize.x, vfWinSize.y);
                          m_pProgram1DTrans[1]->Disable();
                          break;
    case RM_2DTRANS    :  m_pProgram2DTrans[0]->Enable();
                          m_pProgram2DTrans[0]->SetUniformVector("vScreensize",vfWinSize.x, vfWinSize.y);
                          m_pProgram2DTrans[0]->Disable();

                          m_pProgram2DTrans[1]->Enable();
                          m_pProgram2DTrans[1]->SetUniformVector("vScreensize",vfWinSize.x, vfWinSize.y);
                          m_pProgram2DTrans[1]->Disable();
                          break;
    case RM_ISOSURFACE :  {
                            if (m_bDoClearView) {
                              m_pProgramIso2->Enable();
                              m_pProgramIso2->SetUniformVector("vScreensize",vfWinSize.x, vfWinSize.y);
                              m_pProgramIso2->Disable();
                            }
                            GLSLProgram* shader = (m_pDataset->GetInfo().GetComponentCount() == 1) ? m_pProgramIso : m_pProgramColor;
                            shader->Enable();
                            shader->SetUniformVector("vScreensize",vfWinSize.x, vfWinSize.y);
                            shader->Disable();
                          }
                          break;
    default    :          T_ERROR("Invalid rendermode set");
                          break;
  }
}

void GLRaycaster::SetDataDepShaderVars() {
  GLRenderer::SetDataDepShaderVars();
  if (m_eRenderMode == RM_ISOSURFACE && m_bDoClearView) {
    m_pProgramIso2->Enable();
    m_pProgramIso2->SetUniformVector("fIsoval",m_fScaledCVIsovalue);
    m_pProgramIso2->Disable();
  }
}


FLOATMATRIX4 GLRaycaster::ComputeEyeToTextureMatrix(FLOATVECTOR3 p1, FLOATVECTOR3 t1,
                                                    FLOATVECTOR3 p2, FLOATVECTOR3 t2,
                                                    int iStereoID) const {
  FLOATMATRIX4 m;

  FLOATMATRIX4 mInvModelView = m_matModelView[iStereoID].inverse();

  FLOATVECTOR3 vTrans1 = -p1;
  FLOATVECTOR3 vScale  = (t2-t1) / (p2-p1);
  FLOATVECTOR3 vTrans2 =  t1;

  FLOATMATRIX4 mTrans1;
  FLOATMATRIX4 mScale;
  FLOATMATRIX4 mTrans2;

  mTrans1.Translation(vTrans1.x,vTrans1.y,vTrans1.z);
  mScale.Scaling(vScale.x,vScale.y,vScale.z);
  mTrans2.Translation(vTrans2.x,vTrans2.y,vTrans2.z);

  m = mInvModelView * mTrans1 * mScale * mTrans2;

  return m;
}


void GLRaycaster::DisableClipPlane() {
  AbstrRenderer::DisableClipPlane();
  /// We always clip against the plane in the shader, so initialize the plane
  /// to be way out in left field, ensuring nothing will be clipped.
  ClipPlaneToShader(ExtendedPlane(PLANE<float>(0,0,1,-100000)),0,true);
}
