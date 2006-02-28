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
// ****************************************************************************

typedef struct
{
    float  pts[8][3];
    float  val[8][AVT_VARIABLE_LIMIT];
    int    nVars;
}  avtHexahedron;


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


