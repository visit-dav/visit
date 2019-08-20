// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtPlaneSelection.C                         //
// ************************************************************************* //

#include <float.h>

#include <avtPlaneSelection.h>


// ****************************************************************************
//  Method: avtPlaneSelection constructor
//
//  Programmer: Hank Childs 
//  Creation:   January 5, 2009 
//
// ****************************************************************************

avtPlaneSelection::avtPlaneSelection()
{
    normal[0] = 1.;
    normal[1] = 0.;
    normal[2] = 0.;
    origin[0] = 0.;
    origin[1] = 0.;
    origin[2] = 0.;
}

// ****************************************************************************
//  Method: avtPlaneSelection::SetNormal
//
//  Programmer: Hank Childs 
//  Creation:   January 5, 2009 
//
// ****************************************************************************

void
avtPlaneSelection::SetNormal(const double *_normal)
{
    for (int i = 0; i < 3; i++)
        normal[i] = _normal[i];
}


// ****************************************************************************
//  Method: avtPlaneSelection::SetOrigin
//
//  Programmer: Hank Childs 
//  Creation:   January 5, 2009 
//
// ****************************************************************************

void
avtPlaneSelection::SetOrigin(const double *_origin)
{
    for (int i = 0; i < 3; i++)
        origin[i] = _origin[i];
}


// ****************************************************************************
//  Method: GetNormal
//
//  Programmer: Hank Childs 
//  Creation:   January 5, 2009 
//
// ****************************************************************************

void
avtPlaneSelection::GetNormal(double *_normal) const
{
    for (int i = 0; i < 3; i++)
        _normal[i] = normal[i];
}

// ****************************************************************************
//  Method: GetOrigin
//
//  Programmer: Hank Childs 
//  Creation:   January 5, 2009 
//
// ****************************************************************************

void
avtPlaneSelection::GetOrigin(double *_origin) const
{
    for (int i = 0; i < 3; i++)
        _origin[i] = origin[i];
}


// ****************************************************************************
//  Method: avtPlaneSelection::DescriptionString
//
//  Purpose:
//      Creates a string (used as a key for caching) that describes this
//      selection.
//
//  Programmmer: Hank Childs
//  Creation:    December 20, 2011
//
// ****************************************************************************

std::string
avtPlaneSelection::DescriptionString(void)
{
    char str[1024];
    snprintf(str, sizeof(str), "avtPlaneSelection:%f_%f_%f_%f_%f_%f",
                                normal[0], normal[1], normal[2],
                                origin[0], origin[1], origin[2]);
    std::string s2 = str;
    return s2;
}


