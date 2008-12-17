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
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    August 2008
*/


#include "GLRaycaster.h"

#include <cmath>
#include <Basics/SysTools.h>
#include <Controller/MasterController.h>
#include <ctime>

using namespace std;

GLRaycaster::GLRaycaster(MasterController* pMasterController, bool bUseOnlyPowerOfTwo) :
  GLRenderer(pMasterController,bUseOnlyPowerOfTwo),
  m_pFBORayEntry(NULL),
  m_pProgramRenderFrontFaces(NULL),
  m_pProgramIso2(NULL)
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
    m_pFBORayEntry = m_pMasterController->MemMan()->GetFBO(GL_NEAREST, GL_NEAREST, GL_CLAMP, m_vWinSize.x, m_vWinSize.y, GL_RGBA16F_ARB, 16*4, false);
  }
}

bool GLRaycaster::Initialize() {
  if (!GLRenderer::Initialize()) {
    m_pMasterController->DebugOut()->Error("GLRaycaster::Initialize","Error in parent call -> aborting");
    return false;
  }

  glShadeModel(GL_SMOOTH);
 
  if (!LoadAndVerifyShader("../Tuvok/Shaders/GLRaycaster-VS.glsl", "../Tuvok/Shaders/GLRaycaster-frontfaces-FS.glsl",&(m_pProgramRenderFrontFaces)) ||
      !LoadAndVerifyShader("../Tuvok/Shaders/GLRaycaster-VS.glsl", "../Tuvok/Shaders/GLRaycaster-1D-FS.glsl",        &(m_pProgram1DTrans[0])) ||
      !LoadAndVerifyShader("../Tuvok/Shaders/GLRaycaster-VS.glsl", "../Tuvok/Shaders/GLRaycaster-1D-light-FS.glsl",  &(m_pProgram1DTrans[1])) ||
      !LoadAndVerifyShader("../Tuvok/Shaders/GLRaycaster-VS.glsl", "../Tuvok/Shaders/GLRaycaster-2D-FS.glsl",        &(m_pProgram2DTrans[0])) ||
      !LoadAndVerifyShader("../Tuvok/Shaders/GLRaycaster-VS.glsl", "../Tuvok/Shaders/GLRaycaster-2D-light-FS.glsl",  &(m_pProgram2DTrans[1])) ||
      !LoadAndVerifyShader("../Tuvok/Shaders/GLRaycaster-VS.glsl", "../Tuvok/Shaders/GLRaycaster-ISO-FS.glsl",       &(m_pProgramIso)) || 
      !LoadAndVerifyShader("../Tuvok/Shaders/GLRaycaster-VS.glsl", "../Tuvok/Shaders/GLRaycaster-ISO-CV-FS.glsl",    &(m_pProgramIso2))) {

      Cleanup();

      m_pMasterController->DebugOut()->Error("GLRaycaster::Initialize","Error loading a shader.");
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
    m_pProgramIso->Disable();

    m_pProgramIso2->Enable();
    m_pProgramIso2->SetUniformVector("texVolume",0);
    m_pProgramIso2->SetUniformVector("texRayExitPos",2);
    m_pProgramIso2->SetUniformVector("texLastHit",4);
    m_pProgramIso2->SetUniformVector("texLastHitPos",5);
    m_pProgramIso2->Disable();    
  }

  return true;
}

void GLRaycaster::SetBrickDepShaderVars(size_t iCurrentBrick) {
  const Brick& currentBrick = m_vCurrentBrickList[iCurrentBrick];

  FLOATVECTOR3 vVoxelSizeTexSpace = 1.0f/FLOATVECTOR3(currentBrick.vVoxelCount);
  float fRayStep = (currentBrick.vExtension*vVoxelSizeTexSpace * 0.5f * 1.0f/m_fSampleRateModifier).minVal();
  float fStepScale = 1.0f/m_fSampleRateModifier * (FLOATVECTOR3(m_pDataset->GetInfo()->GetDomainSize())/FLOATVECTOR3(m_pDataset->GetInfo()->GetDomainSize(m_iCurrentLOD))).maxVal();

  switch (m_eRenderMode) {
    case RM_1DTRANS    :  {
                            m_pProgram1DTrans[m_bUseLigthing ? 1 : 0]->SetUniformVector("fStepScale", fStepScale);
                            m_pProgram1DTrans[m_bUseLigthing ? 1 : 0]->SetUniformVector("fRayStepsize", fRayStep);
                            if (m_bUseLigthing)
                                m_pProgram1DTrans[1]->SetUniformVector("vVoxelStepsize", vVoxelSizeTexSpace.x, vVoxelSizeTexSpace.y, vVoxelSizeTexSpace.z);
                            break;
                          }
    case RM_2DTRANS    :  {
                            m_pProgram2DTrans[m_bUseLigthing ? 1 : 0]->SetUniformVector("fStepScale", fStepScale);
                            m_pProgram2DTrans[m_bUseLigthing ? 1 : 0]->SetUniformVector("vVoxelStepsize", vVoxelSizeTexSpace.x, vVoxelSizeTexSpace.y, vVoxelSizeTexSpace.z);
                            m_pProgram2DTrans[m_bUseLigthing ? 1 : 0]->SetUniformVector("fRayStepsize", fRayStep);
                            break;
                          }
    case RM_ISOSURFACE : {
                            if (m_bDoClearView) {
                              m_pProgramIso2->Enable();
                              m_pProgramIso2->SetUniformVector("vVoxelStepsize", vVoxelSizeTexSpace.x, vVoxelSizeTexSpace.y, vVoxelSizeTexSpace.z);
                              m_pProgramIso2->SetUniformVector("fRayStepsize", fRayStep);
                              m_pProgramIso2->SetUniformVector("iTileID", int(iCurrentBrick));
                              m_pProgramIso2->Disable();
                              m_pProgramIso->Enable();
                            } 
                            m_pProgramIso->SetUniformVector("vVoxelStepsize", vVoxelSizeTexSpace.x, vVoxelSizeTexSpace.y, vVoxelSizeTexSpace.z);
                            m_pProgramIso->SetUniformVector("fRayStepsize", fRayStep);
                            m_pProgramIso->SetUniformVector("iTileID", int(iCurrentBrick));
                            break;
                          }
    case RM_INVALID    :  m_pMasterController->DebugOut()->Error("GLRaycaster::SetBrickDepShaderVars","Invalid rendermode set"); break;
  }

}

void GLRaycaster::RenderBox(const FLOATVECTOR3& vCenter, const FLOATVECTOR3& vExtend, const FLOATVECTOR3& vMinCoords, const FLOATVECTOR3& vMaxCoords, bool bCullBack) {
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
                                             FLOATVECTOR3(vMinCoords.x, vMinCoords.y, vMinCoords.z));


  m.setTextureMatrix();

  glBegin(GL_QUADS);        
    // BACK
    glVertex3f( vMaxPoint.x, vMinPoint.y, vMinPoint.z);
    glVertex3f( vMinPoint.x, vMinPoint.y, vMinPoint.z);
    glVertex3f( vMinPoint.x, vMaxPoint.y, vMinPoint.z);
    glVertex3f( vMaxPoint.x, vMaxPoint.y, vMinPoint.z);
    // FRONT
    glVertex3f( vMaxPoint.x, vMaxPoint.y, vMaxPoint.z);
    glVertex3f( vMinPoint.x, vMaxPoint.y, vMaxPoint.z);
    glVertex3f( vMinPoint.x, vMinPoint.y, vMaxPoint.z);
    glVertex3f( vMaxPoint.x, vMinPoint.y, vMaxPoint.z);
    // LEFT
    glVertex3f( vMinPoint.x, vMaxPoint.y, vMinPoint.z);
    glVertex3f( vMinPoint.x, vMinPoint.y, vMinPoint.z);
    glVertex3f( vMinPoint.x, vMinPoint.y, vMaxPoint.z);
    glVertex3f( vMinPoint.x, vMaxPoint.y, vMaxPoint.z);
    // RIGHT
    glVertex3f( vMaxPoint.x, vMaxPoint.y, vMaxPoint.z);
    glVertex3f( vMaxPoint.x, vMinPoint.y, vMaxPoint.z);
    glVertex3f( vMaxPoint.x, vMinPoint.y, vMinPoint.z);
    glVertex3f( vMaxPoint.x, vMaxPoint.y, vMinPoint.z);
    // BOTTOM
    glVertex3f( vMaxPoint.x, vMinPoint.y, vMaxPoint.z);
    glVertex3f( vMinPoint.x, vMinPoint.y, vMaxPoint.z);
    glVertex3f( vMinPoint.x, vMinPoint.y, vMinPoint.z);
    glVertex3f( vMaxPoint.x, vMinPoint.y, vMinPoint.z);
    // TOP
    glVertex3f( vMaxPoint.x, vMaxPoint.y, vMinPoint.z);
    glVertex3f( vMinPoint.x, vMaxPoint.y, vMinPoint.z);
    glVertex3f( vMinPoint.x, vMaxPoint.y, vMaxPoint.z);
    glVertex3f( vMaxPoint.x, vMaxPoint.y, vMaxPoint.z);
  glEnd();
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
    default    :          m_pMasterController->DebugOut()->Error("GLRaycaster::Render3DView","Invalid rendermode set"); 
                          break;
  }

  /*
  if (m_iBricksRenderedInThisSubFrame == 0) {
    m_pFBO3DImageCurrent->FinishWrite();
    m_pFBORayEntry->Write(GL_COLOR_ATTACHMENT0_EXT, 0);

    // render nearplane into buffer
    float fNear = m_FrustumCullingLOD.GetNearPlane() + 0.01f;

    FLOATMATRIX4 mInvModelView = m_matModelView.inverse();

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

    m_pFBORayEntry->FinishWrite(0);
    m_pFBO3DImageCurrent->Write();    
  }*/

  glEnable(GL_CULL_FACE);
}

void GLRaycaster::Render3DInLoop(size_t iCurrentBrick) {
  glDisable(GL_BLEND);
  glDepthMask(GL_FALSE);

  // disable writing to the main offscreen buffer
  m_pFBO3DImageCurrent->FinishWrite();

  // write frontfaces (ray entry points)
  m_pFBORayEntry->Write(GL_COLOR_ATTACHMENT0_EXT, 0);

  m_pProgramRenderFrontFaces->Enable();
  RenderBox(m_vCurrentBrickList[iCurrentBrick].vCenter, m_vCurrentBrickList[iCurrentBrick].vExtension, m_vCurrentBrickList[iCurrentBrick].vTexcoordsMin, m_vCurrentBrickList[iCurrentBrick].vTexcoordsMax, false);
  m_pProgramRenderFrontFaces->Disable();

  m_pFBORayEntry->FinishWrite(0);
 
  if (m_eRenderMode == RM_ISOSURFACE) { 
    glDepthMask(GL_TRUE);
    m_pFBOIsoHit->Write(GL_COLOR_ATTACHMENT0_EXT, 0);
    m_pFBOIsoHit->Write(GL_COLOR_ATTACHMENT1_EXT, 1);
    GLFBOTex::TwoDrawBuffers();

    if (m_iBricksRenderedInThisSubFrame == 0) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_pProgramIso->Enable();
    SetBrickDepShaderVars(iCurrentBrick);
    m_pFBORayEntry->Read(GL_TEXTURE2_ARB);
    RenderBox(m_vCurrentBrickList[iCurrentBrick].vCenter, m_vCurrentBrickList[iCurrentBrick].vExtension, m_vCurrentBrickList[iCurrentBrick].vTexcoordsMin, m_vCurrentBrickList[iCurrentBrick].vTexcoordsMax, true);
    m_pFBORayEntry->FinishRead();
    m_pProgramIso->Disable();

    GLFBOTex::NoDrawBuffer();

    m_pFBOIsoHit->FinishWrite(1);
    m_pFBOIsoHit->FinishWrite(0);

    if (m_bDoClearView) {
      m_pFBOCVHit->Write(GL_COLOR_ATTACHMENT0_EXT, 0);
      m_pFBOCVHit->Write(GL_COLOR_ATTACHMENT1_EXT, 1);
      GLFBOTex::TwoDrawBuffers();

      if (m_iBricksRenderedInThisSubFrame == 0) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      m_pProgramIso2->Enable();
      m_pFBORayEntry->Read(GL_TEXTURE2_ARB);
      m_pFBOIsoHit->Read(GL_TEXTURE4_ARB, 0);
      m_pFBOIsoHit->Read(GL_TEXTURE5_ARB, 1);
      RenderBox(m_vCurrentBrickList[iCurrentBrick].vCenter, m_vCurrentBrickList[iCurrentBrick].vExtension, m_vCurrentBrickList[iCurrentBrick].vTexcoordsMin, m_vCurrentBrickList[iCurrentBrick].vTexcoordsMax, true);
      m_pFBOIsoHit->FinishRead(1);
      m_pFBOIsoHit->FinishRead(0);
      m_pFBORayEntry->FinishRead();
      m_pProgramIso2->Disable();
      GLFBOTex::NoDrawBuffer();

      m_pFBOCVHit->FinishWrite(1);
      m_pFBOCVHit->FinishWrite(0);
    }

    m_pFBO3DImageCurrent->Write();
    GLFBOTex::OneDrawBuffer();

  } else {
    m_pFBO3DImageCurrent->Write();
    GLFBOTex::OneDrawBuffer();

    // do the raycasting
    switch (m_eRenderMode) {
      case RM_1DTRANS    :  m_pProgram1DTrans[m_bUseLigthing ? 1 : 0]->Enable();
                            glEnable(GL_BLEND);
                            glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                            break;
      case RM_2DTRANS    :  m_pProgram2DTrans[m_bUseLigthing ? 1 : 0]->Enable(); 
                            glEnable(GL_BLEND);
                            glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                            break;
      default            :  m_pMasterController->DebugOut()->Error("GLRaycaster::Render3DInLoop","Invalid rendermode set"); 
                            break;
    }

    SetBrickDepShaderVars(iCurrentBrick);

    m_pFBORayEntry->Read(GL_TEXTURE2_ARB);
    RenderBox(m_vCurrentBrickList[iCurrentBrick].vCenter, m_vCurrentBrickList[iCurrentBrick].vExtension, m_vCurrentBrickList[iCurrentBrick].vTexcoordsMin, m_vCurrentBrickList[iCurrentBrick].vTexcoordsMax, true);
    m_pFBORayEntry->FinishRead();

    switch (m_eRenderMode) {
      case RM_1DTRANS    :  m_pProgram1DTrans[m_bUseLigthing ? 1 : 0]->Disable();
                            glDisable(GL_BLEND);
                            break;
      case RM_2DTRANS    :  m_pProgram2DTrans[m_bUseLigthing ? 1 : 0]->Disable(); 
                            glDisable(GL_BLEND);
                            break;
      default            :  m_pMasterController->DebugOut()->Error("GLRaycaster::Render3DInLoop","Invalid rendermode set"); 
                            break;
    }
  }
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
    case RM_ISOSURFACE :  if (m_bDoClearView) {
                            m_pProgramIso2->Enable();
                            m_pProgramIso2->SetUniformVector("vScreensize",vfWinSize.x, vfWinSize.y);
                            m_pProgramIso2->Disable();
                          }
                          m_pProgramIso->Enable();
                          m_pProgramIso->SetUniformVector("vScreensize",vfWinSize.x, vfWinSize.y);
                          m_pProgramIso->Disable();
                          break;
    default    :          m_pMasterController->DebugOut()->Error("GLRaycaster::StartFrame","Invalid rendermode set"); 
                          break;
  }
}

void GLRaycaster::Render3DPostLoop() {
  GLRenderer::Render3DPostLoop(); 

  glDisable(GL_CULL_FACE);
  glDepthMask(GL_TRUE);
  glEnable(GL_BLEND);

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
                                                    FLOATVECTOR3 p2, FLOATVECTOR3 t2) {
  FLOATMATRIX4 m;

  FLOATMATRIX4 mInvModelView = m_matModelView.inverse();

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
