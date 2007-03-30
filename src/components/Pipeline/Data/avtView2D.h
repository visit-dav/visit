// ************************************************************************* //
//                                 avtView2D.h                               //
// ************************************************************************* //

#ifndef AVT_VIEW_2D_H
#define AVT_VIEW_2D_H
#include <pipeline_exports.h>

struct avtViewInfo;

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
// ****************************************************************************

struct PIPELINE_API avtView2D
{
    double   viewport[4];
    double   window[4];
    double   axisScaleFactor;
    int      axisScaleType; // 0 == x, 1 == y

  public:
                    avtView2D();
    avtView2D     & operator=(const avtView2D &);
    bool            operator==(const avtView2D &);
    void            SetToDefault(void);
    void            SetViewFromViewInfo(const avtViewInfo &);
    void            SetViewInfoFromView(avtViewInfo &) const;
    void            SetViewportFromView(double *, const int, const int) const;

  protected:
    void            GetValidWindow(double *) const;
};


#endif

