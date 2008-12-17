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
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    August 2008
*/


#include "GLSBVR.h"

#include <cmath>
#include <Basics/SysTools.h>
#include <Controller/MasterController.h>

using namespace std;

GLSBVR::GLSBVR(MasterController* pMasterController, bool bUseOnlyPowerOfTwo) :
  GLRenderer(pMasterController, bUseOnlyPowerOfTwo),
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
    m_pMasterController->DebugOut()->Error("GLSBVR::Initialize","Error in parent call -> aborting");
    return false;
  }

  glShadeModel(GL_SMOOTH);
  glDisable(GL_CULL_FACE);
  
  if (!LoadAndVerifyShader("../Tuvok/Shaders/GLSBVR-VS.glsl", "../Tuvok/Shaders/GLSBVR-1D-FS.glsl",       &(m_pProgram1DTrans[0])) ||
      !LoadAndVerifyShader("../Tuvok/Shaders/GLSBVR-VS.glsl", "../Tuvok/Shaders/GLSBVR-1D-light-FS.glsl", &(m_pProgram1DTrans[1])) ||
      !LoadAndVerifyShader("../Tuvok/Shaders/GLSBVR-VS.glsl", "../Tuvok/Shaders/GLSBVR-2D-FS.glsl",       &(m_pProgram2DTrans[0])) ||
      !LoadAndVerifyShader("../Tuvok/Shaders/GLSBVR-VS.glsl", "../Tuvok/Shaders/GLSBVR-2D-light-FS.glsl", &(m_pProgram2DTrans[1])) ||
      !LoadAndVerifyShader("../Tuvok/Shaders/GLSBVR-VS.glsl", "../Tuvok/Shaders/GLSBVR-ISO-FS.glsl",      &(m_pProgramIso)) ||
      !LoadAndVerifyShader("../Tuvok/Shaders/GLSBVR-VS.glsl", "../Tuvok/Shaders/GLSBVR-ISO-NC-FS.glsl",   &(m_pProgramIsoNoCompose))) {

      Cleanup();

      m_pMasterController->DebugOut()->Error("GLSBVR::Initialize","Error loading a shader.");
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

void GLSBVR::SetBrickDepShaderVars(size_t iCurrentBrick) {
  const Brick& currentBrick = m_vCurrentBrickList[iCurrentBrick];
  FLOATVECTOR3 vStep(1.0f/currentBrick.vVoxelCount.x, 1.0f/currentBrick.vVoxelCount.y, 1.0f/currentBrick.vVoxelCount.z);

  float fStepScale = m_SBVRGeogen.GetOpacityCorrection();

  switch (m_eRenderMode) {
    case RM_1DTRANS    :  {                    
                            m_pProgram1DTrans[m_bUseLigthing ? 1 : 0]->SetUniformVector("fStepScale", fStepScale);
                            if (m_bUseLigthing)
                                m_pProgram1DTrans[1]->SetUniformVector("vVoxelStepsize", vStep.x, vStep.y, vStep.z);
                            break;
                          }
    case RM_2DTRANS    :  {
                            m_pProgram2DTrans[m_bUseLigthing ? 1 : 0]->SetUniformVector("fStepScale", fStepScale);
                            m_pProgram2DTrans[m_bUseLigthing ? 1 : 0]->SetUniformVector("vVoxelStepsize", vStep.x, vStep.y, vStep.z);
                            break;
                          }
    case RM_ISOSURFACE : {
                            if (m_bAvoidSeperateCompositing)
                              m_pProgramIsoNoCompose->SetUniformVector("vVoxelStepsize", vStep.x, vStep.y, vStep.z);
                            else
                              m_pProgramIso->SetUniformVector("vVoxelStepsize", vStep.x, vStep.y, vStep.z);
                            break;
                          }
    case RM_INVALID    :  m_pMasterController->DebugOut()->Error("GLSBVR::SetBrickDepShaderVars","Invalid rendermode set"); break;
  }

}

void GLSBVR::Render3DPreLoop() {
  switch (m_eRenderMode) {
    case RM_1DTRANS    :  m_p1DTransTex->Bind(1); 
                          m_pProgram1DTrans[m_bUseLigthing ? 1 : 0]->Enable();
                          glEnable(GL_BLEND);
                          glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                          break;
    case RM_2DTRANS    :  m_p2DTransTex->Bind(1);
                          m_pProgram2DTrans[m_bUseLigthing ? 1 : 0]->Enable(); 
                          glEnable(GL_BLEND);
                          glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                          break;
    case RM_ISOSURFACE :  if (m_bAvoidSeperateCompositing) {
                            m_pProgramIsoNoCompose->Enable();
                            glEnable(GL_BLEND);
                            glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                          }
                          break;
    default    :  m_pMasterController->DebugOut()->Error("GLSBVR::Render3DView","Invalid rendermode set"); 
                          break;
  }

  m_SBVRGeogen.SetLODData( UINTVECTOR3(m_pDataset->GetInfo()->GetDomainSize(m_iCurrentLOD))  );
}

void GLSBVR::RenderProxyGeometry() {
    glBegin(GL_TRIANGLES);
      for (int i = int(m_SBVRGeogen.m_vSliceTriangles.size())-1;i>=0;i--) {
        glTexCoord3fv(m_SBVRGeogen.m_vSliceTriangles[i].m_vTex);
        glVertex3fv(m_SBVRGeogen.m_vSliceTriangles[i].m_vPos);
      }
    glEnd();
}

void GLSBVR::Render3DInLoop(size_t iCurrentBrick) {
  // setup the slice generator
  m_SBVRGeogen.SetBrickData(m_vCurrentBrickList[iCurrentBrick].vExtension, m_vCurrentBrickList[iCurrentBrick].vVoxelCount, 
                            m_vCurrentBrickList[iCurrentBrick].vTexcoordsMin, m_vCurrentBrickList[iCurrentBrick].vTexcoordsMax);
  FLOATMATRIX4 maBricktTrans; 
  maBricktTrans.Translation(m_vCurrentBrickList[iCurrentBrick].vCenter.x, m_vCurrentBrickList[iCurrentBrick].vCenter.y, m_vCurrentBrickList[iCurrentBrick].vCenter.z);
  FLOATMATRIX4 maBricktModelView = maBricktTrans * m_matModelView;
  maBricktModelView.setModelview();
  m_SBVRGeogen.SetTransformation(maBricktModelView, true);

  if (! m_bAvoidSeperateCompositing && m_eRenderMode == RM_ISOSURFACE) {
    // disable writing to the main offscreen buffer
    m_pFBO3DImageCurrent->FinishWrite();
  
    m_pFBOIsoHit->Write(GL_COLOR_ATTACHMENT0_EXT, 0);
    m_pFBOIsoHit->Write(GL_COLOR_ATTACHMENT1_EXT, 1);
    GLFBOTex::TwoDrawBuffers();

    if (m_iBricksRenderedInThisSubFrame == 0) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_pProgramIso->Enable();
    SetBrickDepShaderVars(iCurrentBrick);
    m_pProgramIso->SetUniformVector("fIsoval",m_fScaledIsovalue);
    RenderProxyGeometry();
    m_pProgramIso->Disable();

    GLFBOTex::NoDrawBuffer();
    m_pFBOIsoHit->FinishWrite(1);
    m_pFBOIsoHit->FinishWrite(0);

    if (m_bDoClearView) {
      m_pFBOCVHit->Write(GL_COLOR_ATTACHMENT0_EXT, 0);
      m_pFBOCVHit->Write(GL_COLOR_ATTACHMENT1_EXT, 1);
      GLFBOTex::TwoDrawBuffers();

      if (m_iBricksRenderedInThisSubFrame == 0) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      m_pProgramIso->Enable();
      m_pProgramIso->SetUniformVector("fIsoval",m_fScaledCVIsovalue);
      RenderProxyGeometry();
      m_pProgramIso->Disable();
      GLFBOTex::NoDrawBuffer();

      m_pFBOCVHit->FinishWrite(1);
      m_pFBOCVHit->FinishWrite(0);
    }

    m_pFBO3DImageCurrent->Write();
    GLFBOTex::OneDrawBuffer();
  } else {
    glDepthMask(GL_FALSE);
    SetBrickDepShaderVars(iCurrentBrick);
    RenderProxyGeometry();
	  glDepthMask(GL_TRUE);
  }
}


void GLSBVR::Render3DPostLoop() {
  GLRenderer::Render3DPostLoop(); 

  // disable the shader
  switch (m_eRenderMode) {
    case RM_1DTRANS    :  m_pProgram1DTrans[m_bUseLigthing ? 1 : 0]->Disable();
						              glDisable(GL_BLEND);
						              break;						  
    case RM_2DTRANS    :  m_pProgram2DTrans[m_bUseLigthing ? 1 : 0]->Disable(); 
						              glDisable(GL_BLEND);
					                break;
    case RM_ISOSURFACE :  if (m_bAvoidSeperateCompositing) {
                             m_pProgramIsoNoCompose->Disable(); 
                             glDisable(GL_BLEND);
                          }
                          break;
    case RM_INVALID    :  m_pMasterController->DebugOut()->Error("GLSBVR::Render3DView","Invalid rendermode set"); break;
  }
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

void GLSBVR::ComposeSurfaceImage() {
  if (!m_bAvoidSeperateCompositing) GLRenderer::ComposeSurfaceImage();
}
