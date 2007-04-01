// ************************************************************************* //
//                           vtkSurfaceFromVolume.h                          //
// ************************************************************************* //

#ifndef VTK_SURFACE_FROM_VOLUME_H
#define VTK_SURFACE_FROM_VOLUME_H

#include <vtkDataSetFromVolume.h>

#include <vector>


class vtkCellData;
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
// ****************************************************************************

class vtkSurfaceFromVolume : public vtkDataSetFromVolume
{

class TriangleList
{
  public:
                   TriangleList();
    virtual       ~TriangleList();
 
    void           AddTriangle(int, int, int, int);
 
    int            GetTotalNumberOfTriangles(void) const;
    int            GetNumberOfLists(void) const;
    int            GetList(int, const int *&) const;
 
  protected:
    int          **list;
    int            currentList;
    int            currentTriangle;
    int            listSize;
    int            trianglesPerList;
};

  public:
                      vtkSurfaceFromVolume(int ptSizeGuess)
                           : vtkDataSetFromVolume(ptSizeGuess), tris()
                           { ; };
    virtual          ~vtkSurfaceFromVolume() { ; };

    void              ConstructPolyData(vtkPointData *, vtkCellData *,
                                        vtkPolyData *, float *);
    void              ConstructPolyData(vtkPointData *, vtkCellData *,
                                        vtkPolyData *, int *, float *, float *,
                                        float *);

    void              AddTriangle(int zone, int v0, int v1, int v2)
                            { tris.AddTriangle(zone, v0, v1, v2); };

  protected:
    TriangleList      tris;
};


#endif


