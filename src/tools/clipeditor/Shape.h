// ----------------------------------------------------------------------------
// File:  Shape.h
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
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
    ST_WEDGE,
    ST_PYRAMID,
    ST_TET,
    ST_POINT
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
    void DrawPolyData(Vector &up, Vector &right);
    void Init();
    void Invert();
    int  CheckCopyOf(Shape*);
    void MakeCopyOf(Shape *s, HexTransform &xform);
    void MakeCopyOf(Shape *s, WedgeTransform &xform);
    void MakeCopyOf(Shape *s, PyramidTransform &xform);
    void MakeCopyOf(Shape *s, TetTransform &xform);

    DataSet *dataset;
};


#endif
