// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_POLYGON_TO_TRIANGLES_TESSELATOR_H
#define AVT_POLYGON_TO_TRIANGLES_TESSELATOR_H

#include <vector>
#include "database_exports.h"


// ****************************************************************************
//  Class:  avtPolygonToTrianglesTesselator
//
//  Purpose: Uses tess2 to create triangles from polygon contours.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Tue Oct  9 12:51:23 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

class VertexManager;
class vtkPoints;
class vtkPolyData;
struct TESSalloc;
struct TESStesselator;

//
// Note on Cyrus' spelling quandry: tesselator vs tessellator
// tess2 uses "tesselator" - for "Tessellate",  two 'l's
// is correct, but I not sure about "tesselator" - so I stuck with one 'l'
// for class names.
//


class DATABASE_API avtPolygonToTrianglesTesselator
{
  public:
             avtPolygonToTrianglesTesselator(vtkPoints *);
    virtual ~avtPolygonToTrianglesTesselator();

    // set normal use for tessellation
    void    SetNormal(double x, double y, double z);
    void    SetNormal(const double *vals);

    // define a contour
    void    BeginContour();
    void    AddContourVertex(double *vals);
    void    AddContourVertex(double x, double y, double z);
    void    EndContour();

    // access to vertex point ids
    int     GetVertexId(double *vals);

    // exec tessellation
    int     Tessellate();

    // this variant adds the generated tris to vtkPolyData
    int     Tessellate(vtkPolyData *pd);

    // these provide access results of last Tessellate call
    int     GetNumberOfTriangles() const;
    void    GetTriangleIndices(int i, int &a, int &b, int &c) const;

  private:
    // normal vector used for tessellation
    double               tessNorm[3];

    // holds verts for current contour
    std::vector<double>  verts;

    // bookkeeping for tess2 memory allocator
    int                  tessMemAllocated;

    // tess2 objs
    TESSalloc           *tessMemAllocator;
    TESStesselator      *tessObj;

    // used to take care of dup verts
    VertexManager       *vertexManager;

    // ref to external vtkPoints object
    vtkPoints           *xPoints;
};



#endif

