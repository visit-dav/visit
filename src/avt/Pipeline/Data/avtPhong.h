// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                avtPhong.h                                 //
// ************************************************************************* //

#ifndef AVT_PHONG_H
#define AVT_PHONG_H
#include <pipeline_exports.h>


#include <avtLightingModel.h>


// ****************************************************************************
//  Method: avtPhong
//
//  Purpose:
//      Performs phong shading with assumption about light position, camera
//      location, projection.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Sun Aug 31 10:28:47 PDT 2008
//    Actually enabled this (after almost 8 years!!) and made several changes
//    to make it work.
//
//    Jeremy Meredith, Mon Jan  4 17:12:16 EST 2010
//    Added ability to reduce amount of lighting for low-gradient-mag areas.
//    Default values to constructor leave the old behavior (i.e. gradmax=0).
//
//    Alister Maguire, Fri Mar 20 16:02:16 PDT 2020
//    Added inv_lightingPower for use in AddLightingHeadlight.
//
// ****************************************************************************

class PIPELINE_API avtPhong : public avtLightingModel
{
  public:
                           avtPhong(double gradMax=0.,double lightingPower=1.);
    virtual               ~avtPhong();

    virtual void           AddLighting(int, const avtRay *, unsigned char *)
                             const;

    virtual void           AddLightingHeadlight(int, const avtRay *,
                                                unsigned char *,
                                                double alpha,
                                                double matProperties[4]) const;
  private:
    double                 gradMax, inv_gradMax, lightingPower;
    double                 inv_lightingPower;
};


#endif


