// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtViewCurve.h                            //
// ************************************************************************* //

#ifndef AVT_VIEW_CURVE_H
#define AVT_VIEW_CURVE_H
#include <view_exports.h>
#include <enumtypes.h>

struct avtViewInfo;
class ViewCurveAttributes;

// ****************************************************************************
//  Class: avtViewCurve
//
//  Purpose:
//    Contains the information for a Curve view.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 30, 2002 
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:40:27 PDT 2003
//    I removed yScale and replaced window with domain and range.  I
//    replaced SetFromViewAttributes with SetFromViewCurveAttributes and
//    SetToViewAttributes with SetToViewCurveAttributes.  I added window
//    size arguments to SetViewFromViewInfo and SetViewInfoFromView.
//
//    Eric Brugger, Fri Oct 10 12:42:27 PDT 2003
//    I deleted SetViewportFromView.  I added GetScaleFactor and
//    GetValidDomainRange.
//
//    Mark C. Miller, Tue Nov  4 14:04:02 PST 2003
//    Added GetViewport method
//
//    Eric Brugger, Tue Nov 18 09:20:54 PST 2003
//    I deleted SetViewFromViewInfo.  I replaced GetValidDomainRange
//    with CheckAndCorrectDomainRange.
//
//    Kathleen Bonnell, Thu Mar 22 20:12:04 PDT 2007 
//    Added domainScale, rangeScale. 
//
//    Kathleen Bonnell, Fri May 11 09:20:06 PDT 2007 
//    Added havePerformedLogDomain/Range.
//
//    Mark Blair, Mon Jul 16 17:16:29 PDT 2007
//    Added SetViewport.
//
//    Alister Maguire, Tue Jun  5 09:13:10 PDT 2018
//    Added a const version of GetScaleFactor, ValidDomainRange,
//    and ComputeScaleFactor. 
//
// ****************************************************************************

struct AVTVIEW_API avtViewCurve
{
    double    viewport[4];
    double    domain[2];
    double    range[2];
    ScaleMode domainScale;
    ScaleMode rangeScale;
    bool      havePerformedLogDomain;
    bool      havePerformedLogRange;

  public:
                    avtViewCurve();
    avtViewCurve  & operator=(const avtViewCurve &);
    bool            operator==(const avtViewCurve &);
    void            SetToDefault(void);
    void            SetViewInfoFromView(avtViewInfo &, int *);
    void            SetViewport(double *newViewport);

    void            GetViewport(double *) const;
    double          GetScaleFactor(int *);
    bool            GetScaleFactor(int *, double &) const;

    void            SetFromViewCurveAttributes(const ViewCurveAttributes *);
    void            SetToViewCurveAttributes(ViewCurveAttributes *) const;

  protected:
    void            CheckAndCorrectDomainRange();
    bool            ValidDomainRange() const;
    double          ComputeViewScale(int *) const;
};


#endif

