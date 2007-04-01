// ************************************************************************* //
//                                 avtViewCurve.h                            //
// ************************************************************************* //

#ifndef AVT_VIEW_CURVE_H
#define AVT_VIEW_CURVE_H
#include <pipeline_exports.h>

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
// ****************************************************************************

struct PIPELINE_API avtViewCurve
{
    double   viewport[4];
    double   domain[2];
    double   range[2];

  public:
                    avtViewCurve();
    avtViewCurve  & operator=(const avtViewCurve &);
    bool            operator==(const avtViewCurve &);
    void            SetToDefault(void);
    void            SetViewInfoFromView(avtViewInfo &, int *);

    void            GetViewport(double *) const;
    double          GetScaleFactor(int *);

    void            SetFromViewCurveAttributes(const ViewCurveAttributes *);
    void            SetToViewCurveAttributes(ViewCurveAttributes *) const;

  protected:
    void            CheckAndCorrectDomainRange();
};


#endif

