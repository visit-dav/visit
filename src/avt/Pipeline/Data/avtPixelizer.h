// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtPixelizer.h                                //
// ************************************************************************* //

#ifndef AVT_PIXELIZER_H
#define AVT_PIXELIZER_H
#include <pipeline_exports.h>


// ****************************************************************************
//  Class: avtPixelizer
//
//  Purpose:
//      Takes a value and an intensity and maps it to a color.  The derived
//      type of pixelizer should relate to the output of a ray function.
//      For example, if the ray function is avtCompositeRF, then the pixelizer
//      that should be used with it should be avtCompositePixelizer.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtPixelizer
{
  public:
    virtual           ~avtPixelizer();

    virtual void       GetColor(double value, double intensity,
                                unsigned char rgb[3]) = 0;
};


#endif


