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

// ************************************************************************* //
//                                avtCellTypes.h                             //
// ************************************************************************* //

#ifndef AVT_CELL_TYPES_H
#define AVT_CELL_TYPES_H


#define AVT_VARIABLE_LIMIT 10


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Tue Nov 13 15:18:18 PST 2001
//    Allowed for cells to have multiple variables.
//
//    Hank Childs, Tue Jan 24 16:25:58 PST 2006
//    Added avtPoint.
//
//    Timo Bremer, Thu Sep 13 13:53:31 PDT 2007
//    Added avtHexahedron2[07].
//
// ****************************************************************************

typedef struct
{
    float  pts[8][3];
    float  val[8][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtHexahedron;


typedef struct
{
    float  pts[20][3];
    float  val[20][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtHexahedron20;

typedef struct
{
    float  pts[27][3];
    float  val[27][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtHexahedron27;


typedef struct
{
    float  pts[4][3];
    float  val[4][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtTetrahedron;


//
// The four vertices that form the base are 0, 1, 2, 3 and the top vertex is
// vertex 4.
//
typedef struct
{
    float  pts[5][3];
    float  val[5][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtPyramid;


//
// Vertices 0, 1, 2 form one side of the wedge and 3, 4, and 5 form the other.
// (This is stored as two triangles, not as a quad with two more points)
//
typedef struct
{
    float  pts[6][3];
    float  val[6][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtWedge;


typedef struct
{
    float bbox[6];
    float val[AVT_VARIABLE_LIMIT];
    int   nVars;
}  avtPoint;


#endif


