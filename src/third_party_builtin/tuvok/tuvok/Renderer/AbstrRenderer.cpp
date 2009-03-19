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
  \file    AbstrRenderer.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    August 2008
*/

#include "AbstrRenderer.h"
#include <Controller/MasterController.h>
#include <algorithm>

using namespace std;

AbstrRenderer::AbstrRenderer(MasterController* pMasterController, bool bUseOnlyPowerOfTwo) : 
  m_pMasterController(pMasterController),
  m_bPerformRedraw(true), 
  m_eRenderMode(RM_1DTRANS),
  m_eViewMode(VM_SINGLE),
  m_eBlendPrecision(BP_32BIT),
  m_bUseLigthing(true),
  m_pDataset(NULL),
  m_p1DTrans(NULL),
  m_p2DTrans(NULL),
  m_fSampleRateModifier(1.0f),
  m_fIsovalue(0.5f),
  m_vIsoColor(0.5,0.5,0.5),
  m_vTextColor(1,1,1,1),
  m_bRenderGlobalBBox(false),
  m_bRenderLocalBBox(false),
  m_vWinSize(0,0),
  m_iLogoPos(3),
  m_strLogoFilename(""),
  m_iMinFramerate(10),
  m_iStartDelay(1000),
  m_iMinLODForCurrentView(0),
  m_iTimeSliceMSecs(100),
  m_iIntraFrameCounter(0),
  m_iFrameCounter(0),
  m_iCheckCounter(0),
  m_iMaxLODIndex(0),
  m_iCurrentLODOffset(0),
  m_bClearFramebuffer(true),
  m_iCurrentLOD(0),
  m_iBricksRenderedInThisSubFrame(0),
  m_bLODDisabled(false),
  m_bDoClearView(false),
  m_fCVIsovalue(0.8f),
  m_vCVColor(1,0,0),
  m_fCVSize(5.5f),
  m_fCVContextScale(1.0f),
  m_fCVBorderScale(60.0f),
  m_vCVPos(0.5f, 0.5f),
  m_bPerformReCompose(false),
  m_bUseOnlyPowerOfTwo(bUseOnlyPowerOfTwo),
  m_bAvoidSeperateCompositing(true)
{
  m_vBackgroundColors[0] = FLOATVECTOR3(0,0,0);
  m_vBackgroundColors[1] = FLOATVECTOR3(0,0,0);

  m_e2x2WindowMode[0] = WM_3D;
  m_e2x2WindowMode[1] = WM_CORONAL;
  m_e2x2WindowMode[2] = WM_AXIAL;
  m_e2x2WindowMode[3] = WM_SAGITTAL;

  m_eFullWindowMode   = WM_3D;

  m_piSlice[0]     = 0.0f;
  m_piSlice[1]     = 0.0f;
  m_piSlice[2]     = 0.0f;

  m_bFlipView[0]   = VECTOR2<bool>(false, false);
  m_bFlipView[1]   = VECTOR2<bool>(false, false);
  m_bFlipView[2]   = VECTOR2<bool>(false, false);

  m_bUseMIP[0]   = false;
  m_bUseMIP[1]   = false;
  m_bUseMIP[2]   = false;

  m_bRedrawMask[0] = true;
  m_bRedrawMask[1] = true;
  m_bRedrawMask[2] = true;
  m_bRedrawMask[3] = true;
}

bool AbstrRenderer::Initialize() {
  return true; // not too much can go wrong here :-)
}

bool AbstrRenderer::LoadDataset(const string& strFilename) {
  if (m_pMasterController == NULL) return false;

  if (m_pMasterController->IOMan() == NULL) {
    m_pMasterController->DebugOut()->Error("AbstrRenderer::LoadDataset","Cannont load dataset because m_pMasterController->IOMan() == NULL");
    return false;
  }

  m_pDataset = m_pMasterController->IOMan()->LoadDataset(strFilename,this);

  if (m_pDataset == NULL) {
    m_pMasterController->DebugOut()->Error("AbstrRenderer::LoadDataset","IOMan call to load dataset failed");
    return false;
  }

  m_pMasterController->DebugOut()->Message("AbstrRenderer::LoadDataset","Load successful, initializing renderer!");

  // find the maximum lod index
  std::vector<UINT64> vSmallestLOD = m_pDataset->GetInfo()->GetLODLevelCountND();
  for (size_t i = 0;i<vSmallestLOD.size();i++) vSmallestLOD[i] -= 1;
  UINT64 iMaxSmallestLOD = 0;
  for (size_t i = 0;i<vSmallestLOD.size();i++) if (iMaxSmallestLOD < vSmallestLOD[i]) iMaxSmallestLOD = vSmallestLOD[i];  
  m_iMaxLODIndex = iMaxSmallestLOD;

  m_piSlice[size_t(WM_SAGITTAL)] = m_pDataset->GetInfo()->GetDomainSize()[0]/2;
  m_piSlice[size_t(WM_CORONAL)]  = m_pDataset->GetInfo()->GetDomainSize()[1]/2;
  m_piSlice[size_t(WM_AXIAL)]    = m_pDataset->GetInfo()->GetDomainSize()[2]/2;

  return true;
}

AbstrRenderer::~AbstrRenderer() {
  m_pMasterController->MemMan()->FreeDataset(m_pDataset, this);
  m_pMasterController->MemMan()->Free1DTrans(m_p1DTrans, this);
  m_pMasterController->MemMan()->Free2DTrans(m_p2DTrans, this);

}

void AbstrRenderer::SetRendermode(ERenderMode eRenderMode) 
{
  if (m_eRenderMode != eRenderMode) {
    m_eRenderMode = eRenderMode; 
    ScheduleCompleteRedraw();
  }  
}

void AbstrRenderer::SetViewmode(EViewMode eViewMode)
{
  if (m_eViewMode != eViewMode) {
    m_eViewMode = eViewMode; 
    ScheduleCompleteRedraw();
  }  
}

void AbstrRenderer::Set2x2Windowmode(ERenderArea eArea, EWindowMode eWindowMode)
{
  /// \todo make sure every view is only assigned to one subwindow
  if (m_e2x2WindowMode[size_t(eArea)] != eWindowMode) {
    m_e2x2WindowMode[size_t(eArea)] = eWindowMode; 
    ScheduleWindowRedraw(eWindowMode);
  }  
}

void AbstrRenderer::SetFullWindowmode(EWindowMode eWindowMode) {
  if (m_eFullWindowMode != eWindowMode) {
    m_eFullWindowMode = eWindowMode; 
    ScheduleCompleteRedraw();
  }  
}

void AbstrRenderer::SetUseLigthing(bool bUseLigthing) {
  if (m_bUseLigthing != bUseLigthing) {
    m_bUseLigthing = bUseLigthing; 
    ScheduleWindowRedraw(WM_3D);
  }
}

void AbstrRenderer::SetBlendPrecision(EBlendPrecision eBlendPrecision) {
  if (m_eBlendPrecision != eBlendPrecision) {
    m_eBlendPrecision = eBlendPrecision;
    ScheduleWindowRedraw(WM_3D);
  }
}


void AbstrRenderer::Changed1DTrans() {
  if (m_eRenderMode != RM_1DTRANS) {
    m_pMasterController->DebugOut()->Message("AbstrRenderer::Changed1DTrans","not using the 1D transferfunction at the moment, ignoring message");
  } else {
    m_pMasterController->DebugOut()->Message("AbstrRenderer::Changed1DTrans","complete redraw scheduled");
    ScheduleCompleteRedraw();
  }
}

void AbstrRenderer::Changed2DTrans() {
  if (m_eRenderMode != RM_2DTRANS) {
    m_pMasterController->DebugOut()->Message("AbstrRenderer::Changed2DTrans","not using the 2D transferfunction at the moment, ignoring message");
  } else {
    m_pMasterController->DebugOut()->Message("AbstrRenderer::Changed2DTrans","complete redraw scheduled");
    ScheduleCompleteRedraw();
  }
}


void AbstrRenderer::SetSampleRateModifier(float fSampleRateModifier) {
  if(m_fSampleRateModifier != fSampleRateModifier) {
    m_fSampleRateModifier = fSampleRateModifier;
    ScheduleWindowRedraw(WM_3D);
  }
}

void AbstrRenderer::SetIsoValue(float fIsovalue) {
  if(m_fIsovalue != fIsovalue) {
    m_fIsovalue = fIsovalue;
    ScheduleWindowRedraw(WM_3D);
  }
}

bool AbstrRenderer::CheckForRedraw() {
  if (m_vCurrentBrickList.size() > m_iBricksRenderedInThisSubFrame || m_iCurrentLODOffset > m_iMinLODForCurrentView) {
    if (m_iCheckCounter == 0)  {
      m_pMasterController->DebugOut()->Message("GLRenderer::CheckForRedraw","Still drawing...");
      return true;
    } else m_iCheckCounter--;
  }
  return m_bPerformRedraw || m_bPerformReCompose;
}

AbstrRenderer::EWindowMode AbstrRenderer::GetWindowUnderCursor(FLOATVECTOR2 vPos) {
  switch (m_eViewMode) {
    case VM_SINGLE   : return m_eFullWindowMode;
    case VM_TWOBYTWO : {
                          if (vPos.y < 0.5f) {
                            if (vPos.x < 0.5f) {
                                return m_e2x2WindowMode[0];
                            } else {
                                return m_e2x2WindowMode[1];
                            }
                          } else {
                            if (vPos.x < 0.5f) {
                                return m_e2x2WindowMode[2];
                            } else {
                                return m_e2x2WindowMode[3];
                            }
                          }
                       }
    default          : return WM_INVALID;
  }
}

FLOATVECTOR2 AbstrRenderer::GetLocalCursorPos(FLOATVECTOR2 vPos) {
  switch (m_eViewMode) {
    case VM_SINGLE   : return vPos;
    case VM_TWOBYTWO : {
                          if (vPos.y < 0.5f) {
                            if (vPos.x < 0.5f) {
                                return vPos*2.0f;
                            } else {
                                return FLOATVECTOR2(vPos.x-0.5f,vPos.y)*2.0f;
                            }
                          } else {
                            if (vPos.x < 0.5f) {
                                return FLOATVECTOR2(vPos.x,vPos.y-0.5f)*2.0f;
                            } else {
                                return FLOATVECTOR2(vPos.x-0.5f,vPos.y-0.5f)*2.0f;
                            }
                          }
                       }
    default          : return vPos;
  }
}

void AbstrRenderer::Resize(const UINTVECTOR2& vWinSize) {
  m_vWinSize = vWinSize;
  ScheduleCompleteRedraw();
}

void AbstrRenderer::SetRotation(const FLOATMATRIX4& mRotation) {
  m_mRotation = mRotation;
  ScheduleWindowRedraw(WM_3D);
}

void AbstrRenderer::SetTranslation(const FLOATMATRIX4& mTranslation) {
  m_mTranslation = mTranslation;
  ScheduleWindowRedraw(WM_3D);
}

void AbstrRenderer::SetSliceDepth(EWindowMode eWindow, UINT64 iSliceDepth) {
  if (eWindow < WM_3D) {
    m_piSlice[size_t(eWindow)] = iSliceDepth;
    ScheduleWindowRedraw(eWindow);
  }
}

UINT64 AbstrRenderer::GetSliceDepth(EWindowMode eWindow) {
  if (eWindow < WM_3D)
    return m_piSlice[size_t(eWindow)];
  else
    return 0;
}

void AbstrRenderer::SetGlobalBBox(bool bRenderBBox) {
  m_bRenderGlobalBBox = bRenderBBox;
  ScheduleWindowRedraw(WM_3D);
}

void AbstrRenderer::SetLocalBBox(bool bRenderBBox) {
  m_bRenderLocalBBox = bRenderBBox;
  ScheduleWindowRedraw(WM_3D);
}

void AbstrRenderer::ScheduleCompleteRedraw() {
  m_bPerformRedraw   = true;
  m_iCheckCounter    = m_iStartDelay;

  m_bRedrawMask[0] = true;
  m_bRedrawMask[1] = true;
  m_bRedrawMask[2] = true;
  m_bRedrawMask[3] = true;
}


void AbstrRenderer::ScheduleWindowRedraw(EWindowMode eWindow) {
  m_bPerformRedraw      = true;
  m_iCheckCounter       = m_iStartDelay;
  m_bRedrawMask[size_t(eWindow)] = true;
}

void AbstrRenderer::ScheduleRecompose() {
  if (!m_bAvoidSeperateCompositing && 
    m_vCurrentBrickList.size() == m_iBricksRenderedInThisSubFrame && m_iCurrentLODOffset > m_iMinLODForCurrentView) { // make sure we finished the current frame
    m_bPerformReCompose = true;
    m_bRedrawMask[WM_3D]  = true;
  } else {
    ScheduleWindowRedraw(WM_3D);
  }
}

void AbstrRenderer::ComputeMinLODForCurrentView() {
  UINTVECTOR3  viVoxelCount = UINTVECTOR3(m_pDataset->GetInfo()->GetDomainSize());
  FLOATVECTOR3 vfExtend     = (FLOATVECTOR3(viVoxelCount) / viVoxelCount.maxVal()) * FLOATVECTOR3(m_pDataset->GetInfo()->GetScale() / m_pDataset->GetInfo()->GetScale().maxVal() );

  // TODO consider real extend not center

  FLOATVECTOR3 vfCenter(0,0,0);
  m_iMinLODForCurrentView = max(0, min<int>(m_pDataset->GetInfo()->GetLODLevelCount()-1,m_FrustumCullingLOD.GetLODLevel(vfCenter,vfExtend,viVoxelCount)));
} 


vector<Brick> AbstrRenderer::BuildFrameBrickList() {
  vector<Brick> vBrickList;

  UINT64VECTOR3 vOverlap = m_pDataset->GetInfo()->GetBrickOverlapSize();
  UINT64VECTOR3 vBrickDimension = m_pDataset->GetInfo()->GetBrickCount(m_iCurrentLOD);
  UINT64VECTOR3 vDomainSize = m_pDataset->GetInfo()->GetDomainSize(m_iCurrentLOD);
  UINT64 iMaxDomainSize = vDomainSize.maxVal();
  FLOATVECTOR3 vScale(m_pDataset->GetInfo()->GetScale().x, 
                      m_pDataset->GetInfo()->GetScale().y, 
                      m_pDataset->GetInfo()->GetScale().z);

  vScale /= vScale.maxVal();

  FLOATVECTOR3 vDomainExtend = FLOATVECTOR3(vScale.x*vDomainSize.x, vScale.y*vDomainSize.y, vScale.z*vDomainSize.z) / iMaxDomainSize;


  FLOATVECTOR3 vBrickCorner;

  for (UINT64 z = 0;z<vBrickDimension.z;z++) {
    Brick b;
    for (UINT64 y = 0;y<vBrickDimension.y;y++) {
      for (UINT64 x = 0;x<vBrickDimension.x;x++) {
        
        UINT64VECTOR3 vSize = m_pDataset->GetInfo()->GetBrickSize(m_iCurrentLOD, UINT64VECTOR3(x,y,z));
        b = Brick(x,y,z, (unsigned int)(vSize.x), (unsigned int)(vSize.y), (unsigned int)(vSize.z));


        FLOATVECTOR3 vEffectiveSize = m_pDataset->GetInfo()->GetEffectiveBrickSize(m_iCurrentLOD, UINT64VECTOR3(x,y,z));


        b.vExtension.x = float(vEffectiveSize.x/float(iMaxDomainSize) * vScale.x);
        b.vExtension.y = float(vEffectiveSize.y/float(iMaxDomainSize) * vScale.y);
        b.vExtension.z = float(vEffectiveSize.z/float(iMaxDomainSize) * vScale.z);
        
        // compute center of the brick
        b.vCenter = (vBrickCorner + b.vExtension/2.0f)-vDomainExtend*0.5f;

        vBrickCorner.x += b.vExtension.x;


        // if the brick is inside the frustum continue processing
        if (m_FrustumCullingLOD.IsVisible(b.vCenter, b.vExtension)) {


          bool bContainsData;

          switch (m_eRenderMode) {
            case RM_1DTRANS    :  bContainsData = m_pDataset->GetInfo()->ContainsData(m_iCurrentLOD, UINT64VECTOR3(x,y,z), double(m_p1DTrans->GetNonZeroLimits().x), double(m_p1DTrans->GetNonZeroLimits().y)); break;
            case RM_2DTRANS    :  bContainsData = m_pDataset->GetInfo()->ContainsData(m_iCurrentLOD, UINT64VECTOR3(x,y,z), double(m_p2DTrans->GetNonZeroLimits().x), double(m_p2DTrans->GetNonZeroLimits().y),double(m_p2DTrans->GetNonZeroLimits().z), double(m_p2DTrans->GetNonZeroLimits().w)); break;
            case RM_ISOSURFACE :  bContainsData = m_pDataset->GetInfo()->ContainsData(m_iCurrentLOD, UINT64VECTOR3(x,y,z), m_fIsovalue*m_p1DTrans->GetSize(), m_fIsovalue*m_p1DTrans->GetSize()); break;
            default            :  bContainsData = false; break;
          }

          // if the brick is visible under the current transfer function continue processing
          if (bContainsData) {

            // compute texture coordinates
            if (m_bUseOnlyPowerOfTwo) {
              UINTVECTOR3 vRealVoxelCount(MathTools::NextPow2(b.vVoxelCount.x),
                                          MathTools::NextPow2(b.vVoxelCount.y),
                                          MathTools::NextPow2(b.vVoxelCount.z));
              b.vTexcoordsMin = FLOATVECTOR3((x == 0) ? 0.5f/vRealVoxelCount.x : vOverlap.x*0.5f/vRealVoxelCount.x,
                                             (y == 0) ? 0.5f/vRealVoxelCount.y : vOverlap.y*0.5f/vRealVoxelCount.y,
                                             (z == 0) ? 0.5f/vRealVoxelCount.z : vOverlap.z*0.5f/vRealVoxelCount.z);
              b.vTexcoordsMax = FLOATVECTOR3((x == vBrickDimension.x-1) ? 1.0f-0.5f/vRealVoxelCount.x : 1.0f-vOverlap.x*0.5f/vRealVoxelCount.x,
                                             (y == vBrickDimension.y-1) ? 1.0f-0.5f/vRealVoxelCount.y : 1.0f-vOverlap.y*0.5f/vRealVoxelCount.y,
                                             (z == vBrickDimension.z-1) ? 1.0f-0.5f/vRealVoxelCount.z : 1.0f-vOverlap.z*0.5f/vRealVoxelCount.z);

              b.vTexcoordsMax -= FLOATVECTOR3(vRealVoxelCount - b.vVoxelCount) / FLOATVECTOR3(vRealVoxelCount);
            } else {
              // compute texture coordinates
              b.vTexcoordsMin = FLOATVECTOR3((x == 0) ? 0.5f/b.vVoxelCount.x : vOverlap.x*0.5f/b.vVoxelCount.x,
                                             (y == 0) ? 0.5f/b.vVoxelCount.y : vOverlap.y*0.5f/b.vVoxelCount.y,
                                             (z == 0) ? 0.5f/b.vVoxelCount.z : vOverlap.z*0.5f/b.vVoxelCount.z);
              
              // for padded volume adjust texcoords
              b.vTexcoordsMax = FLOATVECTOR3((x == vBrickDimension.x-1) ? 1.0f-0.5f/b.vVoxelCount.x : 1.0f-vOverlap.x*0.5f/b.vVoxelCount.x,
                                             (y == vBrickDimension.y-1) ? 1.0f-0.5f/b.vVoxelCount.y : 1.0f-vOverlap.y*0.5f/b.vVoxelCount.y,
                                             (z == vBrickDimension.z-1) ? 1.0f-0.5f/b.vVoxelCount.z : 1.0f-vOverlap.z*0.5f/b.vVoxelCount.z);

            }


            /// compute minimum distance to brick corners (offset slightly to the center to resolve ambiguities) 
            b.fDistance = numeric_limits<float>::max();
            float fEpsilon = 0.4999f;
            FLOATVECTOR3 vEpsilonEdges[8] = {b.vCenter+FLOATVECTOR3(-b.vExtension.x, -b.vExtension.y, -b.vExtension.z)* fEpsilon, 
                                             b.vCenter+FLOATVECTOR3(-b.vExtension.x, -b.vExtension.y, +b.vExtension.z)* fEpsilon, 
                                             b.vCenter+FLOATVECTOR3(-b.vExtension.x, +b.vExtension.y, -b.vExtension.z)* fEpsilon, 
                                             b.vCenter+FLOATVECTOR3(-b.vExtension.x, +b.vExtension.y, +b.vExtension.z)* fEpsilon, 
                                             b.vCenter+FLOATVECTOR3(+b.vExtension.x, -b.vExtension.y, -b.vExtension.z)* fEpsilon, 
                                             b.vCenter+FLOATVECTOR3(+b.vExtension.x, -b.vExtension.y, +b.vExtension.z)* fEpsilon, 
                                             b.vCenter+FLOATVECTOR3(+b.vExtension.x, +b.vExtension.y, -b.vExtension.z)* fEpsilon, 
                                             b.vCenter+FLOATVECTOR3(+b.vExtension.x, +b.vExtension.y, +b.vExtension.z)* fEpsilon};
      
            for (size_t i = 0;i<8;i++) {
              b.fDistance = min(b.fDistance,(FLOATVECTOR4(vEpsilonEdges[i],1.0f)*m_matModelView).xyz().length());
            }

            // add the brick to the list of active bricks
            vBrickList.push_back(b);
          }
        }
      }

      vBrickCorner.x  = 0;
      vBrickCorner.y += b.vExtension.y;
    }
    vBrickCorner.y = 0;
    vBrickCorner.z += b.vExtension.z;
  }

  // depth sort bricks
  sort(vBrickList.begin(), vBrickList.end());

  return vBrickList;
}


void AbstrRenderer::Plan3DFrame() {
  if (m_bPerformRedraw) {
    // compute modelviewmatrix and pass it to the culling object
    m_matModelView = m_mRotation*m_mTranslation;
    m_FrustumCullingLOD.SetViewMatrix(m_matModelView);
    m_FrustumCullingLOD.Update();

    ComputeMinLODForCurrentView();
	  if (!m_bLODDisabled) {
		  m_iCurrentLODOffset = m_iMaxLODIndex+1;
	  } else {
		  m_iCurrentLODOffset = m_iMinLODForCurrentView+1;
	  }
  }

  // plan if the frame is to be redrawn
  // or if we have completed the last subframe but not the entire frame
  if (m_bPerformRedraw || 
     (m_vCurrentBrickList.size() == m_iBricksRenderedInThisSubFrame && m_iCurrentLODOffset > m_iMinLODForCurrentView)) {

    // compute current LOD level
    m_iCurrentLODOffset--;
    m_iCurrentLOD = std::min<UINT64>(m_iCurrentLODOffset,m_pDataset->GetInfo()->GetLODLevelCount()-1);
    UINT64VECTOR3 vBrickCount = m_pDataset->GetInfo()->GetBrickCount(m_iCurrentLOD);

    // build new brick todo-list
    m_vCurrentBrickList = BuildFrameBrickList();
    m_iBricksRenderedInThisSubFrame = 0;
  }

  if (m_bPerformRedraw) {
    // update frame states
    m_iIntraFrameCounter = 0;
    m_iFrameCounter = m_pMasterController->MemMan()->UpdateFrameCounter();
  }
}

void AbstrRenderer::SetCV(bool bEnable) {
  if (!SupportsClearView()) return;

  if (m_bDoClearView != bEnable) {
    m_bDoClearView = bEnable; 
    if (m_eRenderMode == RM_ISOSURFACE) 
      ScheduleWindowRedraw(WM_3D);
  }
}

void AbstrRenderer::SetIsosufaceColor(const FLOATVECTOR3& vColor) {
  m_vIsoColor = vColor; 
  if (m_eRenderMode == RM_ISOSURFACE) 
    ScheduleRecompose();
}

void AbstrRenderer::SetCVIsoValue(float fIsovalue) {
  if (m_fCVIsovalue != fIsovalue) {
    m_fCVIsovalue = fIsovalue;
    if (m_bDoClearView && m_eRenderMode == RM_ISOSURFACE) ScheduleWindowRedraw(WM_3D);
  }
}

void AbstrRenderer::SetCVColor(const FLOATVECTOR3& vColor) {
  if (m_vCVColor != vColor) {
    m_vCVColor = vColor; 
    if (m_bDoClearView && m_eRenderMode == RM_ISOSURFACE) 
      ScheduleRecompose();
  }
}

void AbstrRenderer::SetCVSize(float fSize) {
  if (m_fCVSize != fSize) {
    m_fCVSize = fSize;
    if (m_bDoClearView && m_eRenderMode == RM_ISOSURFACE) 
      ScheduleRecompose();
  }
}

void AbstrRenderer::SetCVContextScale(float fScale) {
  if (m_fCVContextScale != fScale) {
    m_fCVContextScale = fScale;
    if (m_bDoClearView && m_eRenderMode == RM_ISOSURFACE) 
      ScheduleRecompose();
  }
}

void AbstrRenderer::SetCVBorderScale(float fScale) {
  if (m_fCVBorderScale != fScale) {
    m_fCVBorderScale = fScale; 
    if (m_bDoClearView && m_eRenderMode == RM_ISOSURFACE) 
      ScheduleRecompose();
  }
}

void AbstrRenderer::SetCVFocusPos(FLOATVECTOR2 vPos) {
  vPos.y = 1.0f-vPos.y;
  if (m_vCVPos!= vPos) {
    m_vCVPos = vPos; 
    if (m_bDoClearView && m_eRenderMode == RM_ISOSURFACE) 
      ScheduleRecompose();
  }
}

void AbstrRenderer::SetLogoParams(string strLogoFilename, int iLogoPos) {
  m_strLogoFilename = strLogoFilename;
  m_iLogoPos        = iLogoPos;
}

void AbstrRenderer::Set2DFlipMode(EWindowMode eWindow, bool bFlipX, bool bFlipY) {
  // flipping is only possible for 2D views
  if (eWindow > WM_SAGITTAL) return;
  m_bFlipView[size_t(eWindow)] = VECTOR2<bool>(bFlipX, bFlipY);
  ScheduleWindowRedraw(eWindow);
}

void AbstrRenderer::Get2DFlipMode(EWindowMode eWindow, bool& bFlipX, bool& bFlipY) {
  // flipping is only possible for 2D views
  if (eWindow > WM_SAGITTAL) return;
  bFlipX = m_bFlipView[size_t(eWindow)].x;
  bFlipY = m_bFlipView[size_t(eWindow)].y;
}

void AbstrRenderer::GetUseMIP(EWindowMode eWindow, bool& bUseMIP) {
  // MIP is only possible for 2D views
  if (eWindow > WM_SAGITTAL) return;
  bUseMIP = m_bUseMIP[size_t(eWindow)];
}

void AbstrRenderer::SetUseMIP(EWindowMode eWindow, bool bUseMIP) {
  // MIP is only possible for 2D views
  if (eWindow > WM_SAGITTAL) return;
  m_bUseMIP[size_t(eWindow)] = bUseMIP;
  ScheduleWindowRedraw(eWindow);
}
