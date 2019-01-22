/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

