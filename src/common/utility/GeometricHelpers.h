// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef GEOMETRIC_HELPERS_H
#define GEOMETRIC_HELPERS_H
#include <utility_exports.h>

// ****************************************************************************
//  File:  GeometricHelpers
//
//  Purpose:
//    So far, this is just a nice place to store the geometry for
//    a geodesic sphere tesselated as a few different levels, since
//    calculating that on the fly isn't cheap.  Furthermore, I'm adding
//    an expression and operator which must agree on the same geometry,
//    including the same ordering, so it must be some place common....
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 19, 2009
//
// ****************************************************************************

#define geodesic_sphere_tess0_npts 12
#define geodesic_sphere_tess1_npts 42
#define geodesic_sphere_tess2_npts 162
#define geodesic_sphere_tess0_ntris 20
#define geodesic_sphere_tess1_ntris 80
#define geodesic_sphere_tess2_ntris 320
extern UTILITY_API double geodesic_sphere_tess0_points[12][3];
extern UTILITY_API double geodesic_sphere_tess1_points[42][3];
extern UTILITY_API double geodesic_sphere_tess2_points[162][3];
extern UTILITY_API int    geodesic_sphere_tess0_tris[20][3];
extern UTILITY_API int    geodesic_sphere_tess1_tris[80][3];
extern UTILITY_API int    geodesic_sphere_tess2_tris[320][3];

// default
#define geodesic_sphere_npts      geodesic_sphere_tess1_npts
#define geodesic_sphere_ntris     geodesic_sphere_tess1_ntris
#define geodesic_sphere_points    geodesic_sphere_tess1_points
#define geodesic_sphere_tris      geodesic_sphere_tess1_tris

#endif
