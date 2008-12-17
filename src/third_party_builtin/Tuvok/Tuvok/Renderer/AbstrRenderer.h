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
  \file    AbstrRenderer.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.0
  \date    August 2008
*/


#pragma once

#ifndef ABSTRRENDERER_H
#define ABSTRRENDERER_H

#include <string>

#include "../StdDefines.h"
#include "../IO/VolumeDataset.h"
#include "../IO/TransferFunction1D.h"
#include "../IO/TransferFunction2D.h"
#include "../Renderer/CullingLOD.h"

class Brick {
public:
  Brick() :
    vCenter(0,0,0),
    vExtension(0,0,0), 
    vCoords(0,0,0) 
  {
  }

  Brick(unsigned int x, unsigned int y, unsigned int z, unsigned int iSizeX, unsigned int iSizeY, unsigned int iSizeZ) :
    vCenter(0,0,0),
    vExtension(0,0,0), 
    vVoxelCount(iSizeX, iSizeY, iSizeZ),
    vCoords(x,y,z) 
  {
  }

  FLOATVECTOR3 vCenter;
  FLOATVECTOR3 vTexcoordsMin;
  FLOATVECTOR3 vTexcoordsMax;
  FLOATVECTOR3 vExtension;
  UINTVECTOR3 vVoxelCount;
  UINTVECTOR3 vCoords;
  float fDistance;
};

inline bool operator < (const Brick& left, const Brick& right) {
	if  (left.fDistance < right.fDistance) return true;
  return false;
}

class MasterController;

/** \class AbstrRenderer
 * Base for all renderers. */
class AbstrRenderer {
  public:

    enum ERenderArea {
      RA_TOPLEFT = 0,
      RA_TOPRIGHT,
      RA_LOWERLEFT,
      RA_LOWERRIGHT,
      RA_FULLSCREEN,
      RA_INVALID
    };

    enum ERenderMode {
      RM_1DTRANS = 0,  /**< one dimensional transfer function */
      RM_2DTRANS,      /**< two dimensional transfer function */
      RM_ISOSURFACE,   /**< render isosurfaces                */
      RM_INVALID
    };
    ERenderMode GetRendermode() {return m_eRenderMode;}
    virtual void SetRendermode(ERenderMode eRenderMode);

    enum EViewMode {
      VM_SINGLE = 0,  /**< a single large image */
      VM_TWOBYTWO,    /**< four small images */
      VM_INVALID
    };
    EViewMode GetViewmode() {return m_eViewMode;}
    virtual void SetViewmode(EViewMode eViewMode);

    enum EWindowMode {
      WM_CORONAL = 0,
      WM_AXIAL,   
      WM_SAGITTAL,   
      WM_3D,
      WM_INVALID
    };
    EWindowMode Get2x2Windowmode(ERenderArea eArea) {return m_e2x2WindowMode[size_t(eArea)];}
    virtual void Set2x2Windowmode(ERenderArea eArea, EWindowMode eWindowMode);
    EWindowMode GetFullWindowmode() {return m_eFullWindowMode;}
    virtual void SetFullWindowmode(EWindowMode eWindowMode);
    EWindowMode GetWindowUnderCursor(FLOATVECTOR2 vPos);
    FLOATVECTOR2 GetLocalCursorPos(FLOATVECTOR2 vPos);
    
    enum EBlendPrecision {
      BP_8BIT = 0,      
      BP_16BIT,
      BP_32BIT,
      BP_INVALID
    };
    EBlendPrecision GetBlendPrecision() {return m_eBlendPrecision;}
    virtual void SetBlendPrecision(EBlendPrecision eBlendPrecision);

    bool GetUseLigthing() {return m_bUseLigthing;}
    virtual void SetUseLigthing(bool bUseLigthing);

    /** Default settings: 1D transfer function, one by three view, white text, black BG.
     * @param pMasterController message router */
    AbstrRenderer(MasterController* pMasterController, bool bUseOnlyPowerOfTwo);
    /** Deallocates dataset and transfer functions. */
    virtual ~AbstrRenderer();
    /** Sends a message to the master to ask for a dataset to be loaded.
     * The dataset is converted to UVF if it is not one already.
     * @param strFilename path to a file */
    virtual bool LoadDataset(const std::string& strFilename);
    /** Query whether or not we should redraw the next frame, else we should
     * reuse what is already rendered or cintinue with the current frame if it
     * is not complete yet. */
    virtual bool CheckForRedraw();

    virtual void Paint() = 0;

    virtual bool Initialize();

    VolumeDataset*      GetDataSet() {return m_pDataset;}
    TransferFunction1D* Get1DTrans() {return m_p1DTrans;}
    TransferFunction2D* Get2DTrans() {return m_p2DTrans;}

    /** Force a redraw if we're currently using a one dimensional TF. */ 
    virtual void Changed1DTrans();
    /** Force a redraw if we're currently using a two dimensional TF. */ 
    virtual void Changed2DTrans();

    /** Sets up a gradient background which fades vertically.
     * @param vColors[0] is the color at the bottom;
     * @param vColors[1] is the color at the top. */
    virtual bool SetBackgroundColors(FLOATVECTOR3 vColors[2]) {
      if (vColors[0] != m_vBackgroundColors[0] || vColors[1] != m_vBackgroundColors[1]) {
        m_vBackgroundColors[0]=vColors[0];
        m_vBackgroundColors[1]=vColors[1];
        ScheduleCompleteRedraw();
        return true;
      } return false;
    }

    virtual bool SetTextColor(FLOATVECTOR4 vColor) {
      if (vColor != m_vTextColor) {
        m_vTextColor=vColor;ScheduleCompleteRedraw();
        return true;
      } return false;
    }

    FLOATVECTOR3 GetBackgroundColor(int i) const {return m_vBackgroundColors[i];}
    FLOATVECTOR4 GetTextColor() const {return m_vTextColor;}

    virtual void SetSampleRateModifier(float fSampleRateModifier);
    float GetSampleRateModifier() {return m_fSampleRateModifier;}

    virtual void SetIsoValue(float fIsovalue);
    float GetIsoValue() {return m_fIsovalue;}

    virtual void SetIsosufaceColor(const FLOATVECTOR3& vColor);
    virtual FLOATVECTOR3 GetIsosufaceColor()  const {return m_vIsoColor;}


    /** Change the size of the render window.  Any previous image is
     * destroyed, causing a full redraw on the next render.
     * \param vWinSize  new width and height of the view window */
    virtual void Resize(const UINTVECTOR2& vWinSize);
    
    virtual void SetRotation(const FLOATMATRIX4& mRotation);
    virtual void SetTranslation(const FLOATMATRIX4& mTranslation);
    virtual void SetSliceDepth(EWindowMode eWindow, UINT64 fSliceDepth);
    virtual UINT64 GetSliceDepth(EWindowMode eWindow);

    void SetClearFramebuffer(bool bClearFramebuffer) {m_bClearFramebuffer = bClearFramebuffer;}
    bool GetClearFramebuffer() {return m_bClearFramebuffer;}
    void SetGlobalBBox(bool bRenderBBox);
    bool GetGlobalBBox() {return m_bRenderGlobalBBox;}
    void SetLocalBBox(bool bRenderBBox);
    bool GetLocalBBox() {return m_bRenderLocalBBox;}
  
    virtual void SetLogoParams(std::string strLogoFilename, int iLogoPos);
    void Set2DFlipMode(EWindowMode eWindow, bool bFlipX, bool bFlipY);
    void Get2DFlipMode(EWindowMode eWindow, bool& bFlipX, bool& bFlipY);
    void GetUseMIP(EWindowMode eWindow, bool& bUseMIP);
    void SetUseMIP(EWindowMode eWindow, bool bUseMIP);

    // scheduling routines
    UINT64 GetCurrentSubFrameCount() {return 1+m_iMaxLODIndex-m_iMinLODForCurrentView;}
    unsigned int GetWorkingSubFrame() {return 1+m_iMaxLODIndex-m_iCurrentLOD;}

    unsigned int GetCurrentBrickCount() {return (unsigned int)(m_vCurrentBrickList.size());}
    unsigned int GetWorkingBrick() {return m_iBricksRenderedInThisSubFrame;}

    unsigned int GetFrameProgress() {return (unsigned int)(100.0f * float(GetWorkingSubFrame()) / float(GetCurrentSubFrameCount()));}
    unsigned int GetSubFrameProgress() {return (m_vCurrentBrickList.size() == 0) ? 100 : (unsigned int)(100.0f * m_iBricksRenderedInThisSubFrame / float(m_vCurrentBrickList.size()));}
    
    void SetTimeSlice(unsigned int iMSecs) {m_iTimeSliceMSecs = iMSecs;}
    void SetPerfMeasures(unsigned int iMinFramerate, unsigned int iStartDelay) {m_iMinFramerate = iMinFramerate; m_iStartDelay = iStartDelay;}
    void SetRescaleFactors(const DOUBLEVECTOR3& vfRescale) {m_pDataset->GetInfo()->SetRescaleFactors(vfRescale); ScheduleCompleteRedraw();}
    DOUBLEVECTOR3 GetRescaleFactors() {return m_pDataset->GetInfo()->GetRescaleFactors();}

	  void DisableLOD(bool bLODDisabled) {m_bLODDisabled = bLODDisabled;}

    // ClearView
    virtual bool SupportsClearView() {return false;}
    virtual void SetCV(bool bEnable);
    virtual bool GetCV() const {return m_bDoClearView;}
    virtual void SetCVIsoValue(float fIsovalue);
    virtual float GetCVIsoValue() const {return m_fCVIsovalue;}
    virtual void SetCVColor(const FLOATVECTOR3& vColor);
    virtual FLOATVECTOR3 GetCVColor() const {return m_vCVColor;}
    virtual void SetCVSize(float fSize);
    virtual float GetCVSize() const {return m_fCVSize;}
    virtual void SetCVContextScale(float fScale);
    virtual float GetCVContextScale() const {return m_fCVContextScale;}
    virtual void SetCVBorderScale(float fScale);
    virtual float GetCVBorderScale() const {return m_fCVBorderScale;}
    virtual void SetCVFocusPos(FLOATVECTOR2 vPos);
    virtual FLOATVECTOR2 GetCVFocusPos() const {return FLOATVECTOR2(m_vCVPos.x, 1.0f-m_vCVPos.y);}

    virtual void ScheduleCompleteRedraw();
    virtual void ScheduleWindowRedraw(EWindowMode eWindow);

    void SetAvoidSeperateCompositing(bool bAvoidSeperateCompositing) {m_bAvoidSeperateCompositing = bAvoidSeperateCompositing;}
    bool GetAvoidSeperateCompositing() const {return m_bAvoidSeperateCompositing;}

  protected:
    MasterController*   m_pMasterController;
    bool                m_bPerformRedraw;
    bool                m_bRedrawMask[4];
    ERenderMode         m_eRenderMode;
    EViewMode           m_eViewMode;
    EWindowMode         m_e2x2WindowMode[4];
    VECTOR2<bool>       m_bFlipView[3];
    bool                m_bUseMIP[3];
    EWindowMode         m_eFullWindowMode;
    UINT64              m_piSlice[3];
    EBlendPrecision     m_eBlendPrecision;
    bool                m_bUseLigthing;
    VolumeDataset*      m_pDataset;
    TransferFunction1D* m_p1DTrans;
    TransferFunction2D* m_p2DTrans;
    float               m_fSampleRateModifier;
    float               m_fIsovalue;
    FLOATVECTOR3        m_vIsoColor;
    FLOATVECTOR3        m_vBackgroundColors[2];
    FLOATVECTOR4        m_vTextColor;
    FLOATMATRIX4        m_mRotation;
    FLOATMATRIX4        m_mTranslation;
    bool                m_bRenderGlobalBBox;
    bool                m_bRenderLocalBBox;
    UINTVECTOR2         m_vWinSize;
    int                 m_iLogoPos;
    std::string         m_strLogoFilename;

    unsigned int        m_iMinFramerate;
    unsigned int        m_iStartDelay;
    UINT64              m_iMinLODForCurrentView;
    unsigned int        m_iTimeSliceMSecs;

    UINT64              m_iIntraFrameCounter;
    UINT64              m_iFrameCounter;
    unsigned int        m_iCheckCounter;
    UINT64              m_iMaxLODIndex;
    UINT64              m_iCurrentLODOffset;
    CullingLOD          m_FrustumCullingLOD;
    bool                m_bClearFramebuffer;
    UINT64              m_iCurrentLOD;
    UINT64              m_iBricksRenderedInThisSubFrame;
    std::vector<Brick>  m_vCurrentBrickList;
  	bool				        m_bLODDisabled;

    bool                m_bDoClearView;
    float               m_fCVIsovalue;
    FLOATVECTOR3        m_vCVColor;
    float               m_fCVSize;
    float               m_fCVContextScale;
    float               m_fCVBorderScale;
    FLOATVECTOR2        m_vCVPos;
    bool                m_bPerformReCompose;

    // compatibility settings
    bool                m_bUseOnlyPowerOfTwo;
    bool                m_bAvoidSeperateCompositing;

    virtual void ScheduleRecompose();
    void ComputeMinLODForCurrentView();

    FLOATMATRIX4        m_matModelView;

    void                Plan3DFrame();
    std::vector<Brick>  BuildFrameBrickList();

    virtual void ClearDepthBuffer() = 0;
    virtual void ClearColorBuffer() = 0;
};

#endif // ABSTRRENDERER_H
