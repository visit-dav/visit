// ************************************************************************* //
//                                 avtView3D.h                               //
// ************************************************************************* //

#ifndef AVT_VIEW_3D_H
#define AVT_VIEW_3D_H
#include <pipeline_exports.h>

struct avtViewInfo;

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
// ****************************************************************************

struct PIPELINE_API avtView3D
{
    double   normal[3];
    double   focus[3];
    double   viewUp[3];
    double   viewAngle;
    double   parallelScale;
    double   nearPlane;
    double   farPlane;
    bool     perspective;

  public:
                    avtView3D();
    avtView3D     & operator=(const avtView3D &);
    bool            operator==(const avtView3D &);
    void            SetToDefault(void);
    void            SetViewInfoFromView(avtViewInfo &) const;
};


#endif

