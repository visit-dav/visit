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
    void            SetViewFromViewInfo(const avtViewInfo &, int *);
    void            SetViewInfoFromView(avtViewInfo &, int *) const;

    double          GetScaleFactor(int *) const;

    void            SetFromViewCurveAttributes(const ViewCurveAttributes *);
    void            SetToViewCurveAttributes(ViewCurveAttributes *) const;

  protected:
    void            GetValidDomainRange(double *, double *) const;
};


#endif

