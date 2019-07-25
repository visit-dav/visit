// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtView3D.h                               //
// ************************************************************************* //

#ifndef AVT_VIEW_3D_H
#define AVT_VIEW_3D_H
#include <view_exports.h>

struct avtViewInfo;
class View3DAttributes;

// ****************************************************************************
//  Class: avtView3D
//
//  Purpose:
//    Contains the information for a 3D view.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
//  Modifications:
//    Eric Brugger, Fri Mar 29 15:09:35 PST 2002
//    Remove the method SetViewFromViewInfo.
//
//    Eric Brugger, Fri Jun  6 15:20:43 PDT 2003
//    I added image pan and image zoom.
//
//    Eric Brugger, Wed Aug 20 09:38:17 PDT 2003
//    I replaced SetFromViewAttributes with SetFromView3DAttributes and
//    SetToViewAttributes with SetToView3DAttributes.
//
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
//    Eric Brugger, Mon Feb  9 15:59:15 PST 2004
//    Added centerOfRotationSet and centerOfRotation.
//
//    Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//    Support 3D axis scaling (3D equivalent of full-frame mode).
//
//    Jeremy Meredith, Mon Aug  2 14:23:08 EDT 2010
//    Add shear for oblique projection support.
//
//    Eric Brugger, Thu Jan  2 15:04:00 PST 2014
//    I added GetFrustum to add support for 3d multi resolution data
//    selections.
//
//    Eric Brugger, Wed Jan  8 16:44:28 PST 2014
//    I added windowValid to support adding a multi resolution display
//    capability.
//
//    Eric Brugger, Thu Jan 23 16:24:37 PST 2014
//    I deleted GetFrustum. I added GetCompositeProjectionTransformMatrix
//    and CalculateExtentsAndArea.
//
// ****************************************************************************

struct AVTVIEW_API avtView3D
{
    double   normal[3];
    double   focus[3];
    double   viewUp[3];
    double   viewAngle;
    double   parallelScale;
    double   nearPlane;
    double   farPlane;
    double   imagePan[2];
    double   imageZoom;
    double   eyeAngle;
    bool     perspective;
    bool     centerOfRotationSet;
    double   centerOfRotation[3];
    bool     axis3DScaleFlag;
    double   axis3DScales[3];
    double   shear[3];
    bool     windowValid;

  public:
                    avtView3D();
    avtView3D     & operator=(const avtView3D &);
    bool            operator==(const avtView3D &);
    void            SetToDefault(void);
    void            SetViewInfoFromView(avtViewInfo &) const;

    void            SetFromView3DAttributes(const View3DAttributes *);
    void            SetToView3DAttributes(View3DAttributes *) const;

    void            GetCompositeProjectionTransformMatrix(double *, double)
                        const;

    static void     CalculateExtentsAndArea(double *, double &, double *);
};


#endif

