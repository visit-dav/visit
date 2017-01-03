/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                            vtkVolumeFromVolume.h                          //
// ************************************************************************* //

#ifndef VTK_VOLUME_FROM_VOLUME_H
#define VTK_VOLUME_FROM_VOLUME_H

#include <visit_vtk_exports.h>
#include <vtkDataSetFromVolume.h>
#include <vtkCellType.h>

#include <vector>


class vtkCellData;
class vtkDataArray;
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
//    Jeremy Meredith, Fri Feb 26 14:00:07 EST 2010
//    Made internal class definitions protected instead of private so
//    we could usefully subclass this.
//
//    Jeremy Meredith, Thu Oct 28 10:26:45 EDT 2010
//    Added external visibility macro to nested classes.
//
//    Brad Whitlock, Thu Mar 22 14:01:17 PDT 2012
//    Adapted the code to use vtkIdType. Added double coordinate support.
//
//    Eric Brugger, Wed Jul 25 11:04:21 PDT 2012
//    Added a clear method to CentroidPointList and added a version of
//    ShapeList::GetList where none of its arguments had const qualifiers.
//
// ****************************************************************************

class VISIT_VTK_API vtkVolumeFromVolume : public vtkDataSetFromVolume
{
public:
struct CentroidPointEntry
{
    vtkIdType     nPts;
    vtkIdType     ptIds[8];
};

class VISIT_VTK_API CentroidPointList
{
  public:
                   CentroidPointList();
    virtual       ~CentroidPointList();
 
    void           Clear();

    vtkIdType            AddPoint(vtkIdType, const vtkIdType*);
 
    vtkIdType            GetTotalNumberOfPoints(void) const;
    vtkIdType            GetNumberOfLists(void) const;
    vtkIdType            GetList(vtkIdType, const CentroidPointEntry *&) const;
 
  protected:
    CentroidPointEntry   **list;
    vtkIdType              currentList;
    vtkIdType              currentPoint;
    vtkIdType              listSize;
    vtkIdType              pointsPerList;
};

class ShapeList
{
  public:
                   ShapeList(vtkIdType size);
    virtual       ~ShapeList();
    virtual int    GetVTKType(void) const = 0;
    int            GetShapeSize(void) const { return shapeSize; };
    vtkIdType      GetTotalNumberOfShapes(void) const;
    vtkIdType      GetNumberOfLists(void) const;
    vtkIdType      GetList(vtkIdType, const vtkIdType *&) const;
    vtkIdType      GetList(vtkIdType, vtkIdType *&) const;
  protected:
    vtkIdType    **list;
    vtkIdType      currentList;
    vtkIdType      currentShape;
    vtkIdType      listSize;
    vtkIdType      shapesPerList;
    int            shapeSize;
};

class VISIT_VTK_API  HexList : public ShapeList
{
  public:
                   HexList();
    virtual       ~HexList();
    virtual int    GetVTKType(void) const { return VTK_HEXAHEDRON; };
    void           AddHex(vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType);
};

class VISIT_VTK_API WedgeList : public ShapeList
{
  public:
                   WedgeList();
    virtual       ~WedgeList();
    virtual int    GetVTKType(void) const { return VTK_WEDGE; };
    void           AddWedge(vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType);
};

class VISIT_VTK_API PyramidList : public ShapeList
{
  public:
                   PyramidList();
    virtual       ~PyramidList();
    virtual int    GetVTKType(void) const { return VTK_PYRAMID; };
    void           AddPyramid(vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType);
};

class VISIT_VTK_API TetList : public ShapeList
{
  public:
                   TetList();
    virtual       ~TetList();
    virtual int    GetVTKType(void) const { return VTK_TETRA; };
    void           AddTet(vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType);
};

class VISIT_VTK_API QuadList : public ShapeList
{
  public:
                   QuadList();
    virtual       ~QuadList();
    virtual int    GetVTKType(void) const { return VTK_QUAD; };
    void           AddQuad(vtkIdType, vtkIdType, vtkIdType, vtkIdType, vtkIdType);
};

class VISIT_VTK_API TriList : public ShapeList
{
  public:
                   TriList();
    virtual       ~TriList();
    virtual int    GetVTKType(void) const { return VTK_TRIANGLE; };
    void           AddTri(vtkIdType, vtkIdType, vtkIdType, vtkIdType);
};

class VISIT_VTK_API LineList : public ShapeList
{
  public:
                   LineList();
    virtual       ~LineList();
    virtual int    GetVTKType(void) const { return VTK_LINE; };
    void           AddLine(vtkIdType, vtkIdType, vtkIdType);
};

class VISIT_VTK_API VertexList : public ShapeList
{
  public:
                   VertexList();
    virtual       ~VertexList();
    virtual int    GetVTKType(void) const { return VTK_VERTEX; };
    void           AddVertex(vtkIdType, vtkIdType);
};

  public:
                      vtkVolumeFromVolume(vtkIdType nPts, vtkIdType ptSizeGuess);
    virtual          ~vtkVolumeFromVolume() { ; };

    void              ConstructDataSet(vtkPointData *, vtkCellData *,
                                       vtkUnstructuredGrid *, vtkPoints *);
    void              ConstructDataSet(vtkPointData *, vtkCellData *,
                                       vtkUnstructuredGrid *, const int *, vtkDataArray *,
                                       vtkDataArray *,vtkDataArray *);

    int            AddCentroidPoint(vtkIdType n, const vtkIdType *p)
                        { return -1 - (int)centroid_list.AddPoint(n, p); }

    void           AddHex(vtkIdType z, vtkIdType v0, vtkIdType v1, vtkIdType v2, vtkIdType v3,
                          vtkIdType v4, vtkIdType v5, vtkIdType v6, vtkIdType v7)
                        { hexes.AddHex(z, v0, v1, v2, v3, v4, v5, v6, v7); }
        
    void           AddWedge(vtkIdType z,vtkIdType v0,vtkIdType v1,vtkIdType v2,vtkIdType v3,vtkIdType v4,vtkIdType v5)
                        { wedges.AddWedge(z, v0, v1, v2, v3, v4, v5); }
    void           AddPyramid(vtkIdType z, vtkIdType v0, vtkIdType v1, vtkIdType v2, vtkIdType v3, vtkIdType v4)
                        { pyramids.AddPyramid(z, v0, v1, v2, v3, v4); }
    void           AddTet(vtkIdType z, vtkIdType v0, vtkIdType v1, vtkIdType v2, vtkIdType v3)
                        { tets.AddTet(z, v0, v1, v2, v3); }
    void           AddQuad(vtkIdType z, vtkIdType v0, vtkIdType v1, vtkIdType v2, vtkIdType v3)
                        { quads.AddQuad(z, v0, v1, v2, v3); }
    void           AddTri(vtkIdType z, vtkIdType v0, vtkIdType v1, vtkIdType v2)
                        { tris.AddTri(z, v0, v1, v2); }
    void           AddLine(vtkIdType z, vtkIdType v0, vtkIdType v1)
                        { lines.AddLine(z, v0, v1); }
    void           AddVertex(vtkIdType z, vtkIdType v0)
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
};


#endif


