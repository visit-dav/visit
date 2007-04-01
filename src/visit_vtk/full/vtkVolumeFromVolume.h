// ************************************************************************* //
//                            vtkVolumeFromVolume.h                          //
// ************************************************************************* //

#ifndef VTK_VOLUME_FROM_VOLUME_H
#define VTK_VOLUME_FROM_VOLUME_H

#include <vtkDataSetFromVolume.h>

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
//
// ****************************************************************************

class vtkVolumeFromVolume : public vtkDataSetFromVolume
{

class ShapeList
{
  public:
                   ShapeList(int size);
    virtual       ~ShapeList();
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
    void           AddHex(int, int, int, int, int, int, int, int, int);
};

class WedgeList : public ShapeList
{
  public:
                   WedgeList();
    virtual       ~WedgeList();
    void           AddWedge(int, int, int, int, int, int, int);
};

class PyramidList : public ShapeList
{
  public:
                   PyramidList();
    virtual       ~PyramidList();
    void           AddPyramid(int, int, int, int, int, int);
};

class TetList : public ShapeList
{
  public:
                   TetList();
    virtual       ~TetList();
    void           AddTet(int, int, int, int, int);
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



  public:
                      vtkVolumeFromVolume(int nPts, int ptSizeGuess)
                           : vtkDataSetFromVolume(nPts, ptSizeGuess)
                           { ; };
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

  protected:
    CentroidPointList  centroid_list;
    HexList            hexes;
    WedgeList          wedges;
    PyramidList        pyramids;
    TetList            tets;
};


#endif


