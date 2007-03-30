// ************************************************************************* //
//                                 avtViewCurve.h                            //
// ************************************************************************* //

#ifndef AVT_VIEW_CURVE_H
#define AVT_VIEW_CURVE_H
#include <pipeline_exports.h>

struct avtViewInfo;

// ****************************************************************************
//  Class: avtViewCurve
//
//  Purpose:
//    Contains the information for a Curve view.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 30, 2002 
//
// ****************************************************************************

struct PIPELINE_API avtViewCurve
{
    double   viewport[4];
    double   window[4];
    double   yScale; 

  public:
                    avtViewCurve();
    avtViewCurve     & operator=(const avtViewCurve &);
    bool            operator==(const avtViewCurve &);
    void            SetToDefault(void);
    void            SetViewFromViewInfo(const avtViewInfo &);
    void            SetViewInfoFromView(avtViewInfo &) const;
    void            SetViewportFromView(double *, const int, const int) const;
};


#endif

