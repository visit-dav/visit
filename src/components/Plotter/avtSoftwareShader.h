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
// ****************************************************************************

class avtSoftwareShader
{
  public:
    static bool  GetLightDirection(const LightAttributes &, const avtView3D &,
                                   double *);
    static void  AddShadows(avtImage_p, avtImage_p, avtView3D &, avtView3D &,
                            double, double);
};

#endif


