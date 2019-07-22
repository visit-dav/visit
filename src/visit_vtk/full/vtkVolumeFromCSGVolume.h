// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        vtkVolumeFromCSGVolume.h                           //
// ************************************************************************* //

#ifndef VTK_VOLUME_FROM_CSG_VOLUME_H
#define VTK_VOLUME_FROM_CSG_VOLUME_H

#include <visit_vtk_exports.h>

#include <vtkCSGFixedLengthBitField.h>
#include <vtkVolumeFromVolume.h>

#include <vector>


class vtkCellData;
class vtkPointData;
class vtkPolyData;
class vtkUnstructuredGrid;


// ****************************************************************************
//  Class: vtkVolumeFromCSGVolume
//
//  Purpose:
//      This is a special type of vtkVolumeFromVolume, where each added
//      cell belongs to one half of a binary partition of the original
//      data set.  You specify which half when you add the shape, and
//      when constructing the data set, tell it the index (into a bit array)
//      corresponding to the current binary partition function.
//      E.g. on the first pass, you specify bit #0 and "in" cells get the tag
//      0x0 and "out" cels get the tag 0x1.  On the second pass, you specify
//      bit #1, and "in" cells get tagged either 0x0 or 0x1, and "out" cells
//      get tagged either 0x2 or 0x3.  And so on, up to the width of the
//      bit field used here.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//
//    Eric Brugger, Thu Apr  3 08:30:17 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//
//    Burlen Loring, Sun Sep  6 14:58:03 PDT 2015
//    Changed the return type of GetNumberOfCells to vtkIdType
//
// ****************************************************************************

class VISIT_VTK_API vtkVolumeFromCSGVolume : public vtkVolumeFromVolume
{
  protected:


  public:
                      vtkVolumeFromCSGVolume(int nPts,
                                             int ptSizeGuess);
    virtual          ~vtkVolumeFromCSGVolume() { ; };

    void              ConstructDataSet(vtkCellData *,
                          vtkUnstructuredGrid *, float *, int,
                          std::vector<vtkCSGFixedLengthBitField> *newTags);

    using vtkVolumeFromVolume::AddCentroidPoint;
    using vtkDataSetFromVolume::AddPoint;

    void           AddHex(int z, int v0, int v1, int v2, int v3,
                          int v4, int v5, int v6, int v7,
                          vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddHex(z, v0, v1, v2, v3, v4, v5, v6, v7);
        hexTags.push_back(tag);
    }
        
    void           AddWedge(int z,int v0,int v1,int v2,int v3,int v4,int v5,
                            vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddWedge(z, v0, v1, v2, v3, v4, v5);
        wedgeTags.push_back(tag);
    }
    void           AddPyramid(int z, int v0, int v1, int v2, int v3, int v4,
                              vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddPyramid(z, v0, v1, v2, v3, v4); 
        pyramidTags.push_back(tag);
    }
    void           AddTet(int z, int v0, int v1, int v2, int v3,
                          vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddTet(z, v0, v1, v2, v3);
        tetTags.push_back(tag);
    }
    void           AddQuad(int z, int v0, int v1, int v2, int v3,
                           vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddQuad(z, v0, v1, v2, v3);
        quadTags.push_back(tag);
    }
    void           AddTri(int z, int v0, int v1, int v2,
                          vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddTri(z, v0, v1, v2);
        triTags.push_back(tag);
    }
    void           AddLine(int z, int v0, int v1,
                           vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddLine(z, v0, v1);
        lineTags.push_back(tag);
    }
    void           AddVertex(int z, int v0,
                             vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddVertex(z, v0);
        vertexTags.push_back(tag);
    }


    void                    InitTraversal();
    vtkIdType               GetNumberOfCells() const;
    void                    NextCell();
    const vtkIdType        *GetCell();
    vtkCSGFixedLengthBitField GetTag();
    int                     GetCellSize() const;
    int                     GetCellVTKType() const;
    void                    SetId(int);
    void                    SetTagBit(int);
    void                    InvalidateCell();

    void                    UpdatePoints(std::vector<float> &);

  protected:
    std::vector<vtkCSGFixedLengthBitField>   hexTags;
    std::vector<vtkCSGFixedLengthBitField>   wedgeTags;
    std::vector<vtkCSGFixedLengthBitField>   pyramidTags;
    std::vector<vtkCSGFixedLengthBitField>   tetTags;
    std::vector<vtkCSGFixedLengthBitField>   quadTags;
    std::vector<vtkCSGFixedLengthBitField>   triTags;
    std::vector<vtkCSGFixedLengthBitField>   lineTags;
    std::vector<vtkCSGFixedLengthBitField>   vertexTags;
    std::vector<vtkCSGFixedLengthBitField>  *shapeTags[8];
    std::vector<vtkCSGFixedLengthBitField>  *curTags;

    int                     ishape;
    int                     shapeCnt[8];
    ShapeList              *curShapes;
    int                     curShapeCnt;
    int                     curShapeSize;
    int                     curShapeVTKType;
    int                     curShape;
};

#endif
