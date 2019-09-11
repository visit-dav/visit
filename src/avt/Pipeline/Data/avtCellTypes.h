// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
    double  pts[8][3];
    double  val[8][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtHexahedron;


typedef struct
{
    double  pts[20][3];
    double  val[20][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtHexahedron20;

typedef struct
{
    double  pts[27][3];
    double  val[27][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtHexahedron27;


typedef struct
{
    double  pts[4][3];
    double  val[4][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtTetrahedron;


//
// The four vertices that form the base are 0, 1, 2, 3 and the top vertex is
// vertex 4.
//
typedef struct
{
    double  pts[5][3];
    double  val[5][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtPyramid;


//
// Vertices 0, 1, 2 form one side of the wedge and 3, 4, and 5 form the other.
// (This is stored as two triangles, not as a quad with two more points)
//
typedef struct
{
    double  pts[6][3];
    double  val[6][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtWedge;


typedef struct
{
    double bbox[6];
    double val[AVT_VARIABLE_LIMIT];
    int   nVars;
}  avtPoint;


#endif


