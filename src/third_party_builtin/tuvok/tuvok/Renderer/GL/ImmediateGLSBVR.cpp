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
  \file    ImmediateGLSBVR.cpp
  \author  Tom Fogal
           SCI Institute
           University of Utah
*/

#include "GLInclude.h"
#include "ImmediateGLSBVR.h"
#include "Controller/Controller.h"
#include "Basics/Vectors.h"
#include "../GPUMemMan/GPUMemMan.h"
#include <cstring>

ImmediateGLSBVR::ImmediateGLSBVR(bool bUseOnlyPowerOfTwo, bool bDownSample,
                                 bool bDisableBorder) :
  GLSBVR(&(Controller::Instance()), bUseOnlyPowerOfTwo, bDownSample,
         bDisableBorder)
{
}
void ImmediateGLSBVR::Paint()
{
  SetBlendPrecision(BP_8BIT);
  CreateOffscreenBuffers();
  Render();
}

// we want to reuse as much code as possible, so we create a `brick' which is
// really just the whole domain, and then we add it to the internal brick list.
// at that point, we can tell Tuvok to `render all bricks' -- i.e. our scene.
void ImmediateGLSBVR::Render()
{
  SetClearFramebuffer(true);
  StartFrame();
  SetRenderTargetArea(RA_FULLSCREEN);

  // the `current LOD' and vector3 arguments are unused for this case, because
  // we know we'll have a CoreVolumeInfo instead of the default
  // VolumeDatasetInfo.
  UINT64VECTOR3 sz = m_pDataset->GetInfo()->GetBrickSize(m_iCurrentLOD,
                                                         UINT64VECTOR3(0,0,0));

  UINT64VECTOR3 vOverlap = m_pDataset->GetInfo()->GetBrickOverlapSize();
  UINT64VECTOR3 vBrickDimension = m_pDataset->GetInfo()->GetBrickCount(0);
  UINT64VECTOR3 vDomainSize = m_pDataset->GetInfo()->GetDomainSize(0);
  FLOATVECTOR3 vScale(m_pDataset->GetInfo()->GetScale().x,
                      m_pDataset->GetInfo()->GetScale().y,
                      m_pDataset->GetInfo()->GetScale().z);
  FLOATVECTOR3 vDomainExtend = vScale * FLOATVECTOR3(vDomainSize);
  float downscale = vDomainExtend.maxVal();
  vDomainExtend /= downscale;
  const FLOATVECTOR3 vBrickCorner = FLOATVECTOR3(0.f,0.f,0.f);

  Brick b = Brick(0,0,0, static_cast<UINT32>(sz.x),
                         static_cast<UINT32>(sz.y),
                         static_cast<UINT32>(sz.z));

  FLOATVECTOR3 vEffectiveSize = m_pDataset->GetInfo()->GetEffectiveBrickSize(m_iCurrentLOD,
                                                  UINT64VECTOR3(0,0,0));
  b.vExtension = (vEffectiveSize * vScale) / downscale;
  b.vCenter = (vBrickCorner + b.vExtension/2.0f) - vDomainExtend*.5f;

  UINTVECTOR3 vRealVoxelCount(MathTools::NextPow2(b.vVoxelCount.x),
                              MathTools::NextPow2(b.vVoxelCount.y),
                              MathTools::NextPow2(b.vVoxelCount.z));
  b.vTexcoordsMin = FLOATVECTOR3(
    0.5f/vRealVoxelCount.x,
    0.5f/vRealVoxelCount.y,
    0.5f/vRealVoxelCount.z
  );
  b.vTexcoordsMax = FLOATVECTOR3(1.0f-0.5f/vRealVoxelCount.x,
                                 1.0f-0.5f/vRealVoxelCount.y,
                                 1.0f-0.5f/vRealVoxelCount.z);
  b.vTexcoordsMax -= FLOATVECTOR3(vRealVoxelCount - b.vVoxelCount) /
                     FLOATVECTOR3(vRealVoxelCount);

  {
    SetDataDepShaderVars();
    Render3DPreLoop();
    // convert 3D variables to the more general ND scheme used in the memory
    // manager, e.i. convert 3-vectors to stl vectors
    std::vector<UINT64> vLOD; vLOD.push_back(m_iCurrentLOD);
    std::vector<UINT64> vBrick;
    vBrick.push_back(b.vCoords.x);
    vBrick.push_back(b.vCoords.y);
    vBrick.push_back(b.vCoords.z);

    // get the 3D texture from the memory manager
    GPUMemMan &mm = *(Controller::Instance().MemMan());
    GLTexture3D* t = mm.Get3DTexture(m_pDataset, vLOD, vBrick, true, false,
                                     false, 0,0);
    if(t) t->Bind(0);

    {
      m_SBVRGeogen.SetBrickData(b.vExtension, b.vVoxelCount, b.vTexcoordsMin,
                                b.vTexcoordsMax);
  FLOATMATRIX4 maBricktTrans;
  maBricktTrans.Translation(b.vCenter.x, b.vCenter.y, b.vCenter.z);
  FLOATMATRIX4 maBricktModelView = maBricktTrans * m_matModelView[0];
  m_mProjection[0].setProjection();
  maBricktModelView.setModelview();

      //m_SBVRGeogen.SetWorld(m_mRotation * m_mTranslation);
      m_SBVRGeogen.SetWorld(maBricktTrans * m_mRotation * m_mTranslation);
      m_SBVRGeogen.SetView(m_mView[0], true);

      m_TargetBinder.Bind(m_pFBO3DImageCurrent[0]);
        glDepthMask(GL_FALSE);
        SetBrickDepShaderVars(b);
        RenderProxyGeometry();
        glDepthMask(GL_TRUE);
      m_TargetBinder.Unbind();
    }
    mm.Release3DTexture(t);
    Render3DPostLoop();
  }

  EndFrame(true);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
}

void
ImmediateGLSBVR::Hack(float fov, float zNear, float zFar,
                      float eye[3], float ref[3], float vup[3])
{
    this->fov = fov;
    this->z_near = zNear;
    this->z_far = zFar;
    std::memcpy(this->eye, eye, sizeof(float)*3);
    std::memcpy(this->ref, ref, sizeof(float)*3);
    std::memcpy(this->vup, vup, sizeof(float)*3);
}

void ImmediateGLSBVR::SetViewPort(UINTVECTOR2 viLowerLeft,
                                  UINTVECTOR2 viUpperRight)
                                 
{
#if 1
    UINTVECTOR2 viSize = viUpperRight-viLowerLeft;
    float aspect =(float)viSize.x/(float)viSize.y;

#if 0
    glViewport(viLowerLeft.x,viLowerLeft.y,viSize.x,viSize.y);
#endif
    MESSAGE("Aspect ratio of %ux%u window: %f", viSize.x, viSize.y, aspect);

    // view matrix
    m_mView[0].BuildLookAt(FLOATVECTOR3(this->eye), FLOATVECTOR3(this->ref),
                           FLOATVECTOR3(this->vup));
    MESSAGE("The eye(%5.3f,%5.3f,%5.3f) is looking at (%5.3f,%5.3f,%5.3f); "
            " up is (%5.3f,%5.3f,%5.3f).",
            this->eye[0], this->eye[1], this->eye[2],
            this->ref[0], this->ref[1], this->ref[2],
            this->vup[0], this->vup[1], this->vup[2]);

    // projection matrix
    m_mProjection[0].Perspective(this->fov, aspect, this->z_near, this->z_far);
    m_mProjection[0].setProjection();
    MESSAGE("fov:   %5.3f", this->fov);
    MESSAGE("znear: %5.3f", this->z_near);
    MESSAGE("zfar:  %5.3f", this->z_far);

    // forward the projection matrix to the culling object
    m_FrustumCullingLOD.SetProjectionMatrix(m_mProjection[0]);
    m_FrustumCullingLOD.SetScreenParams(this->fov, aspect, this->z_near,
                                        this->z_far, viSize.y);
#endif
}
