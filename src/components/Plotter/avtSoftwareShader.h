// ************************************************************************* //
//                               avtSoftwareShader                           //
// ************************************************************************* //

#ifndef AVT_SOFTWARE_SHADER_H
#define AVT_SOFTWARE_SHADER_H

#include <avtImage.h>


class  LightAttributes;

class  avtView3D;


// ****************************************************************************
//  Class: avtSoftwareShader
//
//  Purpose:
//      Does shadows in software.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2004
//
//  Modifications:
//    Jeremy Meredith, Fri Oct 29 16:49:43 PDT 2004
//    Added FindLightView.  Removed the "aspect" argument from AddShadows
//    because we can just calculate it based on the image passed in.
//
// ****************************************************************************

class avtSoftwareShader
{
  public:
    static bool  GetLightDirection(const LightAttributes &, const avtView3D &,
                                   double *);
    static void  AddShadows(avtImage_p, avtImage_p, avtView3D &, avtView3D &,
                            double);
    static avtView3D  FindLightView(avtImage_p,avtView3D &,double*,double);
};

#endif


