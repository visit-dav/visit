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
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \version 1.0
  \date    August 2008
*/
#pragma once

#ifndef ABSTRRENDERER_H
#define ABSTRRENDERER_H

#include <string>

#include "../StdTuvokDefines.h"
#include "../Renderer/CullingLOD.h"
#include "../IO/Metadata.h"
#include "../IO/Dataset.h"
#include "../Basics/Plane.h"
#include "../Basics/GeometryGenerator.h"

class TransferFunction1D;
class TransferFunction2D;

class Brick {
public:
  Brick() :
    vCenter(0,0,0),
    vExtension(0,0,0),
    vCoords(0,0,0)
  {
  }

  Brick(UINT32 x, UINT32 y, UINT32 z,
        UINT32 iSizeX, UINT32 iSizeY, UINT32 iSizeZ) :
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

    enum ERendererType {
      RT_SBVR = 0,
      RT_RC,
      RT_INVALID
    };

    virtual ERendererType GetRendererType() {return RT_INVALID;}

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
      WM_SAGITTAL = 0,
      WM_AXIAL,
      WM_CORONAL,
      WM_3D,
      WM_INVALID
    };

    EWindowMode Get2x2Windowmode(ERenderArea eArea) const
        { return m_e2x2WindowMode[size_t(eArea)]; }
    virtual void Set2x2Windowmode(ERenderArea eArea, EWindowMode eWindowMode);
    EWindowMode GetFullWindowmode() const { return m_eFullWindowMode; }
    virtual void SetFullWindowmode(EWindowMode eWindowMode);
    EWindowMode GetWindowUnderCursor(FLOATVECTOR2 vPos) const;
    FLOATVECTOR2 GetLocalCursorPos(FLOATVECTOR2 vPos) const;

    enum EBlendPrecision {
      BP_8BIT = 0,
      BP_16BIT,
      BP_32BIT,
      BP_INVALID
    };
    EBlendPrecision GetBlendPrecision() {return m_eBlendPrecision;}
    virtual void SetBlendPrecision(EBlendPrecision eBlendPrecision);

    bool GetUseLighting() const { return m_bUseLighting; }
    virtual void SetUseLighting(bool bUseLighting);

    enum ScalingMethod {
      SMETH_BIT_WIDTH=0,    ///< scaled based on DS and TF bit width
      SMETH_BIAS_AND_SCALE  ///< bias + scale factors of DS calculated
    };
    /** Default settings: 1D transfer function, single-image view, white text,
     * black BG.
     * @param pMasterController message router
     * @param bUseOnlyPowerOfTwo force power of two textures, for systems that
     *                           do not support npot textures.
     * @param bDownSampleTo8Bits force 8bit textures, for systems that do not
     *                           support 16bit textures (or 16bit linear
     *                           interpolation).
     * @param bDisableBorder     don't use OpenGL borders for textures.
     * @param smeth              method to scale values into TF range */
    AbstrRenderer(MasterController* pMasterController, bool bUseOnlyPowerOfTwo,
                  bool bDownSampleTo8Bits, bool bDisableBorder,
                  enum ScalingMethod smeth = SMETH_BIT_WIDTH);
    /** Deallocates dataset and transfer functions. */
    virtual ~AbstrRenderer();
    /** Sends a message to the master to ask for a dataset to be loaded.
     * The dataset is converted to UVF if it is not one already.
     * @param strFilename path to a file */
    virtual bool LoadDataset(const std::string& strFilename);
    /** Query whether or not we should redraw the next frame, else we should
     * reuse what is already rendered or continue with the current frame if it
     * is not complete yet. */
    virtual bool CheckForRedraw();

    virtual void Paint() {
      // check if we are rendering a stereo frame
      m_bDoStereoRendering = m_bRequestStereoRendering &&
                             m_eViewMode == VM_SINGLE &&
                             m_eFullWindowMode == WM_3D;
    }

    virtual bool Initialize();

    /** Deallocates GPU memory allocated during the rendering process. */
    virtual void Cleanup() = 0;

    /// Sets the dataset from external source; only meant to be used by clients
    /// which don't want to use the LOD subsystem.
    void SetDataset(tuvok::Dataset *vds);

    tuvok::Dataset&       GetDataset()       { return *m_pDataset; }
    const tuvok::Dataset& GetDataset() const { return *m_pDataset; }

    TransferFunction1D* Get1DTrans() {return m_p1DTrans;}
    TransferFunction2D* Get2DTrans() {return m_p2DTrans;}
    virtual void Set1DTrans(const std::vector<unsigned char>& rgba);

    /** Notify renderer that 1D TF has changed.  In most cases, this will cause
     * the renderer to start anew. */
    virtual void Changed1DTrans();
    /** Notify renderer that 2D TF has changed.  In most cases, this will cause
     * the renderer to start anew. */
    virtual void Changed2DTrans();

    /** Sets up a gradient background which fades vertically.
     * @param vColors[0] is the color at the bottom;
     * @param vColors[1] is the color at the top. */
    virtual bool SetBackgroundColors(FLOATVECTOR3 vColors[2]) {
      if (vColors[0] != m_vBackgroundColors[0] ||
          vColors[1] != m_vBackgroundColors[1]) {
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

    FLOATVECTOR3 GetBackgroundColor(int i) const {
      return m_vBackgroundColors[i];
    }
    FLOATVECTOR4 GetTextColor() const {return m_vTextColor;}

    virtual void SetSampleRateModifier(float fSampleRateModifier);
    float GetSampleRateModifier() const { return m_fSampleRateModifier; }

    virtual void SetIsoValue(float fIsovalue);
    float GetIsoValue() const { return m_fIsovalue; }

    virtual void SetIsosufaceColor(const FLOATVECTOR3& vColor);
    virtual FLOATVECTOR3 GetIsosufaceColor() const { return m_vIsoColor; }

    virtual void SetOrthoView(bool bOrthoView);
    virtual bool GetOrthoView() const {return m_bOrthoView;}

    virtual void SetRenderCoordArrows(bool bRenderCoordArrows);
    virtual bool GetRenderCoordArrows() const {return m_bRenderCoordArrows;}
    
    virtual void Set2DPlanesIn3DView(bool bRenderPlanesIn3D);
    virtual bool Get2DPlanesIn3DView() const {return m_bRenderPlanesIn3D;}

    /** Change the size of the render window.  Any previous image is
     * destroyed, causing a full redraw on the next render.
     * \param vWinSize  new width and height of the view window */
    virtual void Resize(const UINTVECTOR2& vWinSize);

    virtual void SetRotation(const FLOATMATRIX4& mRotation);
    virtual void SetTranslation(const FLOATMATRIX4& mTranslation);
    void SetClipPlane(const ExtendedPlane& plane);
    virtual void EnableClipPlane();
    virtual void DisableClipPlane();
    virtual void ShowClipPlane(bool);
    virtual void ClipPlaneRelativeLock(bool);
    virtual bool CanDoClipPlane() {return true;}
    bool ClipPlaneEnabled() const { return m_bClipPlaneOn; }
    bool ClipPlaneShown() const   { return m_bClipPlaneDisplayed; }
    bool ClipPlaneLocked() const  { return m_bClipPlaneLocked; }

    /// slice parameter for slice views.
    virtual void SetSliceDepth(EWindowMode eWindow, UINT64 fSliceDepth);
    virtual UINT64 GetSliceDepth(EWindowMode eWindow);

    void SetClearFramebuffer(bool bClearFramebuffer) {
      m_bClearFramebuffer = bClearFramebuffer;
    }
    bool GetClearFramebuffer() {return m_bClearFramebuffer;}
    void SetGlobalBBox(bool bRenderBBox);
    bool GetGlobalBBox() {return m_bRenderGlobalBBox;}
    void SetLocalBBox(bool bRenderBBox);
    bool GetLocalBBox() {return m_bRenderLocalBBox;}

    virtual void SetLogoParams(std::string strLogoFilename, int iLogoPos);
    void Set2DFlipMode(EWindowMode eWindow, bool bFlipX, bool bFlipY);
    void Get2DFlipMode(EWindowMode eWindow, bool& bFlipX, bool& bFlipY) const;
    bool GetUseMIP(EWindowMode eWindow) const;
    void SetUseMIP(EWindowMode eWindow, bool bUseMIP);

    // scheduling routines
    UINT64 GetCurrentSubFrameCount() const
        { return 1+m_iMaxLODIndex-m_iMinLODForCurrentView; }
    UINT32 GetWorkingSubFrame() const
        { return 1+m_iMaxLODIndex-m_iCurrentLOD; }

    UINT32 GetCurrentBrickCount() const
        { return UINT32(m_vCurrentBrickList.size()); }
    UINT32 GetWorkingBrick() const
        { return m_iBricksRenderedInThisSubFrame; }

    UINT32 GetFrameProgress() const {
        return UINT32(100.0f * float(GetWorkingSubFrame()) /
                      float(GetCurrentSubFrameCount()));
    }
    UINT32 GetSubFrameProgress() const {
        return (m_vCurrentBrickList.empty()) ? 100 :
                UINT32(100.0f * m_iBricksRenderedInThisSubFrame /
                float(m_vCurrentBrickList.size()));
    }

    void SetTimeSlice(UINT32 iMSecs) {m_iTimeSliceMSecs = iMSecs;}
    void SetPerfMeasures(UINT32 iMinFramerate, UINT32 iStartDelay) {
      m_iMinFramerate = iMinFramerate; m_iStartDelay = iStartDelay;
    }
    void SetRescaleFactors(const DOUBLEVECTOR3& vfRescale) {
      m_pDataset->GetInfo().SetRescaleFactors(vfRescale); ScheduleCompleteRedraw();
    }
    DOUBLEVECTOR3 GetRescaleFactors() {
      return m_pDataset->GetInfo().GetRescaleFactors();
    }

    void SetCaptureMode(bool bCaptureMode) {m_bCaptureMode = bCaptureMode;}
    void SetMIPLOD(bool bMIPLOD) {m_bMIPLOD = bMIPLOD;}

    virtual void  SetStereo(bool bStereoRendering);
    virtual void  SetStereoEyeDist(float fStereoEyeDist);
    virtual void  SetStereoFocalLength(float fStereoFocalLength);
    virtual bool  GetStereo() {return m_bRequestStereoRendering;}
    virtual float GetStereoEyeDist() {return m_fStereoEyeDist;}
    virtual float GetStereoFocalLength() {return m_fStereoFocalLength;}

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
    virtual FLOATVECTOR2 GetCVFocusPos() const {
      return FLOATVECTOR2(m_vCVPos.x, 1.0f-m_vCVPos.y);
    }

    virtual void ScheduleCompleteRedraw();
    virtual void ScheduleWindowRedraw(EWindowMode eWindow);

    void SetAvoidSeperateCompositing(bool bAvoidSeperateCompositing) {
      m_bAvoidSeperateCompositing = bAvoidSeperateCompositing;
    }
    bool GetAvoidSeperateCompositing() const {
      return m_bAvoidSeperateCompositing;
    }

    void SetMIPRotationAngle(float fAngle) {
      m_fMIPRotationAngle = fAngle;
      m_bPerformRedraw = true;
    }

    /// Prepends the given directory to the list of paths Tuvok will
    /// try to find shaders in.
    void AddShaderPath(const std::string &path) {
      m_vShaderSearchDirs.insert(m_vShaderSearchDirs.begin(), path);
    }

    void SetViewParameters(float angle, float znear, float zfar,
                           const FLOATVECTOR3& eye,
                           const FLOATVECTOR3& ref,
                           const FLOATVECTOR3& vup) {
      m_vEye = eye;
      m_vAt = ref;
      m_vUp = vup;
      m_fFOV = angle;
      m_fZNear = znear;
      m_fZFar = zfar;
    }

    void SetScalingMethod(enum ScalingMethod sm) {
      this->m_TFScalingMethod = sm;
    }

  protected:
    MasterController*   m_pMasterController;
    bool                m_bPerformRedraw;
    float               m_fMsecPassed[2];
    bool                m_bRedrawMask[4];
    ERenderMode         m_eRenderMode;
    EViewMode           m_eViewMode;
    EWindowMode         m_e2x2WindowMode[4];
    VECTOR2<bool>       m_bFlipView[3];
    bool                m_bUseMIP[3];
    EWindowMode         m_eFullWindowMode;
    UINT64              m_piSlice[3];
    EBlendPrecision     m_eBlendPrecision;
    bool                m_bUseLighting;
    tuvok::Dataset*     m_pDataset;
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

    UINT32              m_iMinFramerate;
    UINT32              m_iStartDelay;
    UINT64              m_iMinLODForCurrentView;
    UINT32              m_iTimeSliceMSecs;

    UINT64              m_iIntraFrameCounter;
    UINT64              m_iFrameCounter;
    UINT32              m_iCheckCounter;
    UINT64              m_iMaxLODIndex;
    UINT64              m_iPerformanceBasedLODSkip;
    UINT64              m_iCurrentLODOffset;
    UINT64              m_iStartLODOffset;
    CullingLOD          m_FrustumCullingLOD;
    bool                m_bClearFramebuffer;
    UINT64              m_iCurrentLOD;
    UINT64              m_iBricksRenderedInThisSubFrame;
    std::vector<Brick>  m_vCurrentBrickList;
    std::vector<Brick>  m_vLeftEyeBrickList;
    bool                m_bCaptureMode;
    bool                m_bMIPLOD;
    float               m_fMIPRotationAngle;
    FLOATMATRIX4        m_maMIPRotation;
    bool                m_bOrthoView;
    bool                m_bRenderCoordArrows;
    bool                m_bRenderPlanesIn3D;

    bool                m_bDoClearView;
    float               m_fCVIsovalue;
    FLOATVECTOR3        m_vCVColor;
    float               m_fCVSize;
    float               m_fCVContextScale;
    float               m_fCVBorderScale;
    FLOATVECTOR2        m_vCVPos;
    bool                m_bPerformReCompose;
    bool                m_bRequestStereoRendering;
    bool                m_bDoStereoRendering;
    float               m_fStereoEyeDist;
    float               m_fStereoFocalLength;
    std::vector<Triangle> m_vArrowGeometry;

    // compatibility settings
    bool                m_bUseOnlyPowerOfTwo;
    bool                m_bDownSampleTo8Bits;
    bool                m_bDisableBorder;
    bool                m_bAvoidSeperateCompositing;
    enum ScalingMethod  m_TFScalingMethod;

    FLOATMATRIX4        m_mProjection[2];
    FLOATMATRIX4        m_mView[2];
    FLOATMATRIX4        m_matModelView[2];
    std::vector<std::string> m_vShaderSearchDirs;

    ExtendedPlane       m_ClipPlane;
    bool                m_bClipPlaneOn;
    bool                m_bClipPlaneDisplayed;
    bool                m_bClipPlaneLocked;

    /// view parameters.
    ///@{
    FLOATVECTOR3        m_vEye, m_vAt, m_vUp;
    float               m_fFOV;
    float               m_fZNear, m_fZFar;
    ///@}

    virtual void        ScheduleRecompose();
    void                ComputeMinLODForCurrentView();
    void                ComputeMaxLODForCurrentView();
    void                Plan3DFrame();
    void                PlanHQMIPFrame();
    std::vector<Brick>  BuildSubFrameBrickList(bool bUseResidencyAsDistanceCriterion=false);
    std::vector<Brick>  BuildLeftEyeSubFrameBrickList(const std::vector<Brick>& vRightEyeBrickList);
    virtual void        ClearDepthBuffer() = 0;
    virtual void        ClearColorBuffer() = 0;
};

#endif // ABSTRRENDERER_H
