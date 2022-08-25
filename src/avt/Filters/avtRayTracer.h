// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtRayTracer.h                            //
// ************************************************************************* //

#ifndef AVT_RAY_TRACER_H
#define AVT_RAY_TRACER_H

#include <filters_exports.h>

#include <avtRayTracerBase.h>


// ****************************************************************************
//  Class: avtRayTracer
//
//  Purpose:
//      Performs ray tracing, taking in a dataset as a source and has an
//      image as an output.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jan  8 16:52:26 PST 2001
//    Added "Get" functions.
//
//    Hank Childs, Sat Feb  3 20:37:01 PST 2001
//    Removed pixelizer and added mechanism to change background color.
//
//    Hank Childs, Tue Feb 13 15:15:50 PST 2001
//    Added ability to insert an opaque image into the rendering.
//
//    Brad Whitlock, Wed Dec 5 11:13:18 PDT 2001
//    Added gradient backgrounds.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Sun Dec  4 18:00:55 PST 2005
//    Add method that estimates number of stages.
//
//    Hank Childs, Mon Jan 16 11:11:47 PST 2006
//    Add support for kernel based sampling.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Wed Dec 24 14:17:03 PST 2008
//    Add method TightenClippingPlanes.
//
//    Pascal Grosset, Fri Sep 20 2013
//    Added ray casting slivr & trilinear interpolation
//
//    Kathleen Biagas, Wed Aug 17, 2022
//    Use override keyword.
//
// ****************************************************************************

class AVTFILTERS_API avtRayTracer : public avtRayTracerBase
{
  public:
                          avtRayTracer();
    virtual              ~avtRayTracer();

    const char           *GetType(void) override { return "avtRayTracer"; }
    const char           *GetDescription(void) override { return "Ray tracing"; }

    void                  SetBackgroundMode(int mode) override;
    void                  SetGradientBackgroundColors(const double [3],
                                                      const double [3]) override;
  protected:
    int                   backgroundMode;
    double                gradBG1[3];
    double                gradBG2[3];

    void                  Execute(void) override;
};


#endif


