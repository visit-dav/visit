// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtRayFunction.h                             //
// ************************************************************************* //

#ifndef AVT_RAY_FUNCTION_H
#define AVT_RAY_FUNCTION_H

#include <pipeline_exports.h>

#include <avtCellTypes.h>


class     avtGradients;
class     avtLightingModel;
class     avtRay;


// ****************************************************************************
//  Class: avtRayFunction
//
//  Purpose:
//      This is the base type for any ray function.  A ray function is a
//      routine that takes a series of sample points along a ray (avtRay) and
//      a lighting model and determines what the shading for the pixel should
//      be.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Feb  3 20:14:19 PST 2001
//    Made the output of GetRayValue be a pixel instead of a value.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Sep 11 14:59:30 PDT 2006
//    Add method SetPrimaryIndex.  Also add methods for needing pixel
//    indices.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Hank Childs, Sun Aug 31 08:04:42 PDT 2008
//    Remove infrastructure for gradients.  This is now done a different way.
//    Add support for lighting using gradients.
//
// ****************************************************************************

class PIPELINE_API avtRayFunction
{
  public:
                         avtRayFunction(avtLightingModel *);
    virtual             ~avtRayFunction();

    void                 SetPrimaryVariableIndex(int vi)
                                    { primaryVariableIndex = vi; };

    virtual void         GetRayValue(const avtRay *,
                                     unsigned char rgb[3], double) = 0;
    virtual bool         CanContributeToPicture(int,
                                          const double (*)[AVT_VARIABLE_LIMIT]);
    virtual bool         NeedPixelIndices(void) { return false; };

    void                 SetPixelIndex(int i, int j)
                             { pixelIndexI = i; pixelIndexJ = j; };

    void                 SetGradientVariableIndex(int gvi);

    virtual int          GetOpacityVariableIndex() const { return -1; }
    virtual int          GetWeightVariableIndex() const { return -1; }

  protected:
    avtLightingModel    *lighting;
    int                  gradientVariableIndex;
    int                  primaryVariableIndex;
    int                  pixelIndexI, pixelIndexJ;

    inline int           IndexOfDepth(const double &, const int &);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtRayFunction(const avtRayFunction &) {;};
    avtRayFunction      &operator=(const avtRayFunction &) { return *this; };
};


// ****************************************************************************
//  Method: avtRayFunction::IndexOfDepth
//
//  Purpose:
//      Determines the index of a depth in the z-buffer.  Assumes 0. is the
//      near plane, 1. is the far plane.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
// ****************************************************************************

inline int
avtRayFunction::IndexOfDepth(const double &depth, const int &numSamples)
{
    int rv = (int) (depth*numSamples);
    if (rv >= numSamples)
    {
        rv = numSamples-1;
    }
    else if (rv < 0)
    {
        rv = 0;
    }

    return rv;
}


#endif


