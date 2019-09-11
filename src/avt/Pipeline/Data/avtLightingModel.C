// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************ //
//                           avtLightingModel.h                             //
// ************************************************************************ //

#include <avtLightingModel.h>


// ****************************************************************************
//  Method: avtLightingModel constructor
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2008
//
// ****************************************************************************

avtLightingModel::avtLightingModel()
{
    gradientVariableIndex = -1;
    view_direction[0] = 0.;
    view_direction[1] = 0.;
    view_direction[2] = 1.;
    view_up[0] = 0.;
    view_up[1] = 1.;
    view_up[2] = 0.;
    view_right[0] = 1.;
    view_right[1] = 0.;
    view_right[2] = 0.;
    doSpecular = false;
    specularCoeff = 0.;
    specularPower = 0.;
}


// ****************************************************************************
//  Method: avtLightingModel denstructor
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2008
//
// ****************************************************************************

avtLightingModel::~avtLightingModel()
{
    // Needed on Windows
}


