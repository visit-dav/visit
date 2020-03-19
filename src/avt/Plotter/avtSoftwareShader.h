// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtSoftwareShader                           //
// ************************************************************************* //

#ifndef AVT_SOFTWARE_SHADER_H
#define AVT_SOFTWARE_SHADER_H
#include <plotter_exports.h>
#include <avtImage.h>


class  LightAttributes;

struct  avtView3D;


// ****************************************************************************
//  Class: avtSoftwareShader
//
//  Purpose:
//      Does shadows in software.  Also handles depth cueing.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2004
//
//  Modifications:
//    Jeremy Meredith, Fri Oct 29 16:49:43 PDT 2004
//    Added FindLightView.  Removed the "aspect" argument from AddShadows
//    because we can just calculate it based on the image passed in.
//
//    Jeremy Meredith, Wed Aug 29 13:11:37 EDT 2007
//    Added depth cueing.
//
//    Tom Fogal, Mon Jun 16 11:17:53 EDT 2008
//    Added some const qualifications.
//
//    Jeremy Meredith, Fri Apr 30 14:23:19 EDT 2010
//    Added automatic mode for depth cueing.
//
//    Burlen Loring, Thu Oct  8 11:52:26 PDT 2015
//    make cue color array passed in const
//
// ****************************************************************************

class PLOTTER_API avtSoftwareShader
{
  public:
    static bool  GetLightDirection(const LightAttributes &, const avtView3D &,
                                   double *);
    static void  AddShadows(avtImage_p, avtImage_p, const avtView3D &,
                            const avtView3D &, double);
    static void  AddDepthCueing(avtImage_p current_image,
                                const avtView3D &current_view,
                                bool autoExtents,
                                const double startPoint[3],
                                const double endPoint[3],
                                const unsigned char cuecolor[3]);
    static avtView3D  FindLightView(avtImage_p, const avtView3D &,
                                    const double*,double);
};

#endif


