// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtRayFunction.C                            //
// ************************************************************************* //

#include <avtRayFunction.h>

#include <avtLightingModel.h>
#include <avtRay.h>


// ****************************************************************************
//  Method: avtRayFunction constructor
//
//  Arguments:
//      l       The lighting model this object should use for shading.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Sep 11 14:59:30 PDT 2006
//    Initialize primaryVariableIndex and pixelIndices.
//
// ****************************************************************************

avtRayFunction::avtRayFunction(avtLightingModel *l)
{
    lighting = l;
    primaryVariableIndex = 0;
    pixelIndexI = 0;
    pixelIndexJ = 0;
}


// ****************************************************************************
//  Method: avtRayFunction destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtRayFunction::~avtRayFunction()
{
    ;
}


// ****************************************************************************
//  Method: avtRayFunction::CanContributeToPicture
//
//  Purpose:
//      Sampling is one of the costliest portions of doing volume rendering.
//      This is an opportunity to the "cheat" and ask the ray function if it
//      thinks that it will needs this cell to make the final picture.  If the
//      answer is no, the cell is not sampled.
//
//  Arguments:
//      <unused>   The number of vertices for the cell.
//      <unused>   The variables at each vertex.
//
//  Returns:       true if the cell can contribute to the picture, false
//                 otherwise.
//
//  Notes:         Derived types should redefine this if appropriate.
//
//  Programmer:    Hank Childs
//  Creation:      December 7, 2001
//
// ****************************************************************************

bool
avtRayFunction::CanContributeToPicture(int,const double (*)[AVT_VARIABLE_LIMIT])
{
    return true;
}


// ****************************************************************************
//  Method: avtRayFunction::SetGradientVariableIndex
//
//  Purpose:
//      Tells the ray function the index of the gradient variable.  This index
//      corresponds to the x-coordinate and the following two indices are for
//      y and z.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2008
//
// ****************************************************************************

void
avtRayFunction::SetGradientVariableIndex(int gvi)
{
    lighting->SetGradientVariableIndex(gvi);
    gradientVariableIndex = gvi;
}


