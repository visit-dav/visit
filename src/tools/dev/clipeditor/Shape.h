// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ----------------------------------------------------------------------------
// File:  Shape.h
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
//
// Modifications:
//    Jeremy Meredith, Thu Sep 18 11:29:12 PDT 2003
//    Added Quad and Triangle shapes.
//
//    Jeremy Meredith, Thu Jun 24 10:38:05 PDT 2004
//    Added Voxel and Pixel shapes.
//
//    Jeremy Meredith, Tue Aug 29 16:13:43 EDT 2006
//    Added Line and Vertex shapes.
//
//    Jeremy Meredith, Mon Jul  9 15:22:06 EDT 2012
//    Added 5- thru 8-sided polygon shapes.
//
// ----------------------------------------------------------------------------

#ifndef SHAPE_H
#define SHAPE_H

#include <vector>
#include "Vector.h"
#include "Transforms.h"

struct DataSet;

enum ShapeType
{
    ST_HEX,
    ST_VOXEL,
    ST_WEDGE,
    ST_PYRAMID,
    ST_TET,
    ST_QUAD,
    ST_TRIANGLE,
    ST_PIXEL,
    ST_VERTEX,
    ST_LINE,
    ST_POINT,
    ST_POLY5,
    ST_POLY6,
    ST_POLY7,
    ST_POLY8
};

struct Shape
{
    ShapeType shapeType;

    int nverts;
    float xc[8];
    float yc[8];
    float zc[8];

    int nedges;
    typedef int edge_t[2];
    edge_t *edges;

    int ntris;
    typedef int tri_t[3];
    tri_t *tris;

    int nquads;
    typedef int quad_t[4];
    quad_t *quads;

    bool pointcase[8];

    bool edgepointsvalid[12];
    float exc[12];
    float eyc[12];
    float ezc[12];

    int splitCase;
    Shape *parentShape;
    char parentNodes[8];

    float xcent;
    float ycent;
    float zcent;
    bool valid;

    int color;

    // Rendering options
    static int  duplicateFacesRemoval;
    static bool lighting;
    static bool numbering;

    // Methods
    Shape() {}
    Shape(ShapeType, int sc, DataSet *ds);
    Shape(ShapeType st, Shape *parent, const char *nodes, DataSet *ds);
    Shape(ShapeType st, Shape *parent, int c, int n, const char *nodes, DataSet *ds);
    Shape(Shape *copy, int xformID, Shape *parent, DataSet *ds);
    void GeneratePolygonCoords();
    void DrawPolyData(Vector &up, Vector &right);
    void Init();
    void Invert();
    int  CheckCopyOf(Shape*);
    template <class T> void MakeCopyOf(Shape *s, T &xform);

    DataSet *dataset;
};

template <class T> void
Shape::MakeCopyOf(Shape *s, T &xform)
{
    for (int i=0; i<nverts; i++)
    {
        char c1 = s->parentNodes[i];
        char c2 = c1;
        if (c1 >= '0' && c1 <= '9')
        {
            c2 = xform.n[c1 - '0'] + '0';
        }
        else if (c1 >= 'a' && c1 <= 'l')
        {
            c2 = xform.e[c1 - 'a'];
        }
        parentNodes[i] = c2;
    }
    if (xform.f)
        Invert();
}


#endif
