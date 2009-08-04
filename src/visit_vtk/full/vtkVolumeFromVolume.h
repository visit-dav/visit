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
//                            vtkVolumeFromVolume.h                          //
// ************************************************************************* //

#ifndef VTK_VOLUME_FROM_VOLUME_H
#define VTK_VOLUME_FROM_VOLUME_H

#include <vtkDataSetFromVolume.h>
#include <vtkCellType.h>

#include <vector>


class vtkCellData;
class vtkPointData;
class vtkPolyData;
class vtkUnstructuredGrid;


// ****************************************************************************
//  Class: vtkVolumeFromVolume
//
//  Purpose:
//      This class is a data object.  It is much like vtkSurfaceFromVolume,
//      except instead of generating new surfaces (e.g. from contours or
//      slicing), it generates new volumes (e.g. from clipping or isovolumes).
//      It also extends it to allow points based on the unweighted
//      interpolation of up-to-eight other points, and it uses negative
//      indices for these "centroid" points.
//
//  Programmer: Jeremy Meredith
//  Creation:   August  7, 2003
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 16 18:01:29 PST 2004
//    Added 2D shape support.  This might seem weird, but there may be 
//    some 2D shapes in a volumetric setting.
//
//    Hank Childs, Thu Oct 21 07:23:55 PDT 2004
//    Added new data members shapes and nshapes.  Also added 
//    GetNumberOfPointsPerShape method to ShapeList.
//
//    Jeremy Meredith, Tue Aug 29 14:33:30 EDT 2006
//    Added support for lines and vertices.
//
// ****************************************************************************

class vtkVolumeFromVolume : public vtkDataSetFromVolume
{

class ShapeList
{
  public:
                   ShapeList(int size);
    virtual       ~ShapeList();
    virtual int    GetVTKType(void) const = 0;
    int            GetShapeSize(void) const { return shapeSize; };
    int            GetTotalNumberOfShapes(void) const;
    int            GetNumberOfLists(void) const;
    int            GetList(int, const int *&) const;
  protected:
    int          **list;
    int            currentList;
    int            currentShape;
    int            listSize;
    int            shapesPerList;
    int            shapeSize;
};

class HexList : public ShapeList
{
  public:
                   HexList();
    virtual       ~HexList();
    virtual int    GetVTKType(void) const { return VTK_HEXAHEDRON; };
    void           AddHex(int, int, int, int, int, int, int, int, int);
};

class WedgeList : public ShapeList
{
  public:
                   WedgeList();
    virtual       ~WedgeList();
    virtual int    GetVTKType(void) const { return VTK_WEDGE; };
    void           AddWedge(int, int, int, int, int, int, int);
};

class PyramidList : public ShapeList
{
  public:
                   PyramidList();
    virtual       ~PyramidList();
    virtual int    GetVTKType(void) const { return VTK_PYRAMID; };
    void           AddPyramid(int, int, int, int, int, int);
};

class TetList : public ShapeList
{
  public:
                   TetList();
    virtual       ~TetList();
    virtual int    GetVTKType(void) const { return VTK_TETRA; };
    void           AddTet(int, int, int, int, int);
};

class QuadList : public ShapeList
{
  public:
                   QuadList();
    virtual       ~QuadList();
    virtual int    GetVTKType(void) const { return VTK_QUAD; };
    void           AddQuad(int, int, int, int, int);
};

class TriList : public ShapeList
{
  public:
                   TriList();
    virtual       ~TriList();
    virtual int    GetVTKType(void) const { return VTK_TRIANGLE; };
    void           AddTri(int, int, int, int);
};

class LineList : public ShapeList
{
  public:
                   LineList();
    virtual       ~LineList();
    virtual int    GetVTKType(void) const { return VTK_LINE; };
    void           AddLine(int, int, int);
};

class VertexList : public ShapeList
{
  public:
                   VertexList();
    virtual       ~VertexList();
    virtual int    GetVTKType(void) const { return VTK_VERTEX; };
    void           AddVertex(int, int);
};

struct CentroidPointEntry
{
    int     nPts;
    int     ptIds[8];
};


class CentroidPointList
{
  public:
                   CentroidPointList();
    virtual       ~CentroidPointList();
 
    int            AddPoint(int, int*);
 
    int            GetTotalNumberOfPoints(void) const;
    int            GetNumberOfLists(void) const;
    int            GetList(int, const CentroidPointEntry *&) const;
 
  protected:
    CentroidPointEntry   **list;
    int                    currentList;
    int                    currentPoint;
    int                    listSize;
    int                    pointsPerList;
};

typedef struct
{
   bool   hasPtsList;
   float *pts_ptr;
   int   *dims;
   float *X;
   float *Y;
   float *Z;
} CommonPointsStructure;


  public:
                      vtkVolumeFromVolume(int nPts, int ptSizeGuess);
    virtual          ~vtkVolumeFromVolume() { ; };

    void              ConstructDataSet(vtkPointData *, vtkCellData *,
                                       vtkUnstructuredGrid *, float *);
    void              ConstructDataSet(vtkPointData *, vtkCellData *,
                                       vtkUnstructuredGrid *, int *, float *,
                                       float *,float *);

    int            AddCentroidPoint(int n, int *p)
                        { return -1 - centroid_list.AddPoint(n, p); }

    void           AddHex(int z, int v0, int v1, int v2, int v3,
                          int v4, int v5, int v6, int v7)
                        { hexes.AddHex(z, v0, v1, v2, v3, v4, v5, v6, v7); }
        
    void           AddWedge(int z,int v0,int v1,int v2,int v3,int v4,int v5)
                        { wedges.AddWedge(z, v0, v1, v2, v3, v4, v5); }
    void           AddPyramid(int z, int v0, int v1, int v2, int v3, int v4)
                        { pyramids.AddPyramid(z, v0, v1, v2, v3, v4); }
    void           AddTet(int z, int v0, int v1, int v2, int v3)
                        { tets.AddTet(z, v0, v1, v2, v3); }
    void           AddQuad(int z, int v0, int v1, int v2, int v3)
                        { quads.AddQuad(z, v0, v1, v2, v3); }
    void           AddTri(int z, int v0, int v1, int v2)
                        { tris.AddTri(z, v0, v1, v2); }
    void           AddLine(int z, int v0, int v1)
                        { lines.AddLine(z, v0, v1); }
    void           AddVertex(int z, int v0)
                        { vertices.AddVertex(z, v0); }

  protected:
    CentroidPointList  centroid_list;
    HexList            hexes;
    WedgeList          wedges;
    PyramidList        pyramids;
    TetList            tets;
    QuadList           quads;
    TriList            tris;
    LineList           lines;
    VertexList         vertices;

    ShapeList         *shapes[8];
    const int          nshapes;

    void               ConstructDataSet(vtkPointData *, vtkCellData *,
                                        vtkUnstructuredGrid *, 
                                        CommonPointsStructure &);
};


#endif


