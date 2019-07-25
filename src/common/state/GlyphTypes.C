// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              GlyphTypes.C                                 //
// ************************************************************************* //

#include <GlyphTypes.h>


static const char *GlyphType_strings[] = {
    "Box",
    "Axis",
    "Icosahedron",
    "Octahedron",
    "Tetrahedron",
    "SphereGeometry",
    "Point",
    "Sphere"
};

// ****************************************************************************
// Function: GlyphType_ToString
//
// Purpose:
//   Returns a string version of GlyphType.
//
// Programmer: Kathleen Biagas
// Creation:   December 12, 2016
//
// Modifications:
//
// ****************************************************************************
std::string
GlyphType_ToString(GlyphType m)
{
    int index = int(m);
    if(m < Box || m > Sphere) index = 6; // return Point if index is bad
    return GlyphType_strings[index];
}

std::string
GlyphType_ToString(int m)
{
    int index = m;
    if(index < Box || index > Sphere) index = 6; // return Point if index is bad
    return GlyphType_strings[index];
}

// ****************************************************************************
// Function: GlyphType_FromString
//
// Purpose:
//   Returns a GlyphType associated with a string value.
//
// Programmer: Kathleen Biagas
// Creation:   December 12, 2016
//
// Modifications:
//
// ****************************************************************************

bool
GlyphType_FromString(const std::string &s, GlyphType &m)
{
    m = Point;

    for(int i = Box; i <= Sphere; ++i)
    {
        if(s == GlyphType_strings[i])
        {
            m = GlyphType(i);
            return true;
        }
    }

    return false;
}

int
GlyphType_NumTypes(void)
{
    return 8;
}

