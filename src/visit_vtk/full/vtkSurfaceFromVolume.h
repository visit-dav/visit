// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           vtkSurfaceFromVolume.h                          //
// ************************************************************************* //

#ifndef VTK_SURFACE_FROM_VOLUME_H
#define VTK_SURFACE_FROM_VOLUME_H

#include <visit_vtk_exports.h>
#include <vtkDataSetFromVolume.h>

#include <vector>


class vtkCellData;
class vtkDataArray;
class vtkPointData;
class vtkPolyData;


// ****************************************************************************
//  Class: vtkSurfaceFromVolume
//
//  Purpose:
//      This class is a data object.  It stores out surfaces, making it similar
//      to vtkPolyData.  However, it assumes that the surfaces it is creating
//      stem from a volume.  In addition, it is assumed that each triangle from
//      the new surface is contained in a cell from the volume.  Finally, it
//      is assumed that each endpoint of the triangle is located on an edge of
//      a cell in the original volume.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 15:55:08 PDT 2003
//    Refactored the surface-independent part into a new vtkDataSetFromVolume.
//    Left the part dealing with triangles and polydata here.
//
//    Brad Whitlock, Tue Mar  6 11:19:45 PST 2012
//    Change int to vtkIdType. Change ConstructPolyData methods so they can
//    support more input coordinate types than just float.
//
// ****************************************************************************

class VISIT_VTK_API vtkSurfaceFromVolume : public vtkDataSetFromVolume
{
  public:
class TriangleList
{
  public:
                   TriangleList();
    virtual       ~TriangleList();
 
    void           AddTriangle(vtkIdType, vtkIdType, vtkIdType, vtkIdType);
 
    vtkIdType      GetTotalNumberOfTriangles(void) const;
    vtkIdType      GetNumberOfLists(void) const;
    int            GetList(int, const vtkIdType *&) const;
 
  protected:
    vtkIdType    **list;
    vtkIdType      currentList;
    vtkIdType      currentTriangle;
    vtkIdType      listSize;
    vtkIdType      trianglesPerList;
};

                      vtkSurfaceFromVolume(int ptSizeGuess)
                           : vtkDataSetFromVolume(ptSizeGuess), tris()
                           { ; };
    virtual          ~vtkSurfaceFromVolume() { ; };

    void              ConstructPolyData(vtkPointData *, vtkCellData *,
                                        vtkPolyData *, vtkPoints *);
    void              ConstructPolyData(vtkPointData *, vtkCellData *,
                                        vtkPolyData *, int *, 
                                        vtkDataArray *, vtkDataArray *, vtkDataArray *);

    void              AddTriangle(vtkIdType zone, vtkIdType v0, vtkIdType v1, vtkIdType v2)
                            { tris.AddTriangle(zone, v0, v1, v2); };

  protected:
    TriangleList      tris;
};


#endif


