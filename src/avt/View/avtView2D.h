// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtView2D.h                               //
// ************************************************************************* //

#ifndef AVT_VIEW_2D_H
#define AVT_VIEW_2D_H
#include <view_exports.h>
#include <enumtypes.h>

struct avtViewInfo;
class View2DAttributes;

// ****************************************************************************
//  Class: avtView2D
//
//  Purpose:
//    Contains the information for a 2D view.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu May 15 09:46:46 PDT 2003 
//    Added members axisScaleFactor and axisScaleType.
// 
//    Kathleen Bonnell, Wed Jul 16 16:46:02 PDT 2003 
//    Added methods ScaleWindow and ReverseScaleWindow.
// 
//    Eric Brugger, Wed Aug 20 09:35:51 PDT 2003
//    I replaced SetFromViewAttributes with SetFromView2DAttributes and
//    SetToViewAttributes with SetToView2DAttributes.
//
//    Eric Brugger, Wed Oct  8 16:43:28 PDT 2003
//    I replaced axisScaleFactor and axisScaleType with fullFrame.  I
//    deleted SetViewFromViewInfo.  I added window size arguments to
//    SetViewFromViewInfo.  I renamed SetViewportFromView to GetActualViewport.
//    I added GetScaleFactor.  I deleted ScaleWindow and ReverseScaleWindow.
//
//    Eric Brugger, Tue Nov 18 08:17:28 PST 2003
//    Replaced GetValidWindow with CheckAndCorrectWindow.
//
//    Mark C. Miller, Tue Mar 14 17:49:26 PST 2006
//    Added stuff to support auto full frame
//
//    Kathleen Bonnell, Thu Mar 29 11:04:17 PDT 2007 
//    Added xScale, yScale, havePerformedLogX, havePerformedLogY.
//
//    Eric Brugger, Thu Oct 27 09:27:30 PDT 2011
//    Added windowValid to support adding a multi resolution display
//    capability for AMR data.
//
//    Eric Brugger, Thu Jan 23 16:22:54 PST 2014
//    Added GetCompositeProjectionTransformMatrix and CalculateExtentsAndArea.
//
// ****************************************************************************

struct AVTVIEW_API avtView2D
{
    double   viewport[4];
    double   window[4];
    bool     fullFrame;

    bool     windowValid;

    int      fullFrameActivationMode;
    float    fullFrameAutoThreshold;

    ScaleMode xScale;
    ScaleMode yScale;

    bool     havePerformedLogX;
    bool     havePerformedLogY;

  public:
                    avtView2D();
    avtView2D     & operator=(const avtView2D &);
    bool            operator==(const avtView2D &);

    // alternative to == that ignores autoff state
    bool            EqualViews(const avtView2D &);

    void            SetToDefault(void);
    void            SetViewInfoFromView(avtViewInfo &, int *);

    void            GetActualViewport(double *, const int, const int);
    double          GetScaleFactor(int *);

    void            SetFromView2DAttributes(const View2DAttributes *);
    void            SetToView2DAttributes(View2DAttributes *) const;

    void            GetCompositeProjectionTransformMatrix(double *, double);

    static void     CalculateExtentsAndArea(double *, double &, double *);

  protected:
    void            CheckAndCorrectWindow();
};


#endif

