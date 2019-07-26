// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtFlatLighting.h>

avtFlatLighting::avtFlatLighting()
{
    // Needed on Windows
}

avtFlatLighting::~avtFlatLighting()
{
    // Needed on Windows
}


// ****************************************************************************
//  Method: avtFlatLighting
//
//  Purpose:
//      Applies "flat" lighting, meaning that no lighting is applied.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2008
//
// ****************************************************************************

void
avtFlatLighting::AddLighting(int, const avtRay *, unsigned char *) const
{
    ; // No-op; leave color as it is.
}



void
avtFlatLighting::AddLightingHeadlight(int, const avtRay *, unsigned char *, double alpha, double matProperties[4]) const
{
    ; // No-op; leave color as it is.
}
