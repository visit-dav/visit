/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
