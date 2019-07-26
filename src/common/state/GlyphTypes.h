// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              GlyphTypes.h                                 //
// ************************************************************************* //

#ifndef GLYPH_TYPES_H
#define GLYPH_TYPES_H

#include <state_exports.h>
#include <string>

enum GlyphType
{
   Box              = 0,
   Axis,           /* 1 */
   Icosahedron,    /* 2 */
   Octahedron,     /* 3 */
   Tetrahedron,    /* 4 */
   SphereGeometry, /* 5 */
   Point,          /* 6 */
   Sphere          /* 7 */
};


STATE_API std::string GlyphType_ToString(GlyphType m);
STATE_API std::string GlyphType_ToString(int m);
STATE_API bool        GlyphType_FromString(const std::string &s, GlyphType &m);
STATE_API int         GlyphType_NumTypes(void);

#endif
