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
                      vtkVolumeFromCSGVolume(vtkIdType  nPts,
                                             size_t ptSizeGuess);
    virtual          ~vtkVolumeFromCSGVolume() { ; }

    void              ConstructDataSet(vtkCellData *,
                          vtkUnstructuredGrid *, float *, int,
                          std::vector<vtkCSGFixedLengthBitField> *newTags);

    using vtkVolumeFromVolume::AddCentroidPoint;
    using vtkDataSetFromVolume::AddPoint;

    void           AddHex(vtkIdType z,  vtkIdType v0, vtkIdType v1,
                          vtkIdType v2, vtkIdType v3, vtkIdType v4,
                          vtkIdType v5, vtkIdType v6, vtkIdType v7,
                          vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddHex(z, v0, v1, v2, v3, v4, v5, v6, v7);
        hexTags.push_back(tag);
    }
        
    void           AddWedge(vtkIdType z,  vtkIdType v0, vtkIdType v1,
                            vtkIdType v2, vtkIdType v3, vtkIdType v4,
                            vtkIdType v5,
                            vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddWedge(z, v0, v1, v2, v3, v4, v5);
        wedgeTags.push_back(tag);
    }
    void           AddPyramid(vtkIdType z,  vtkIdType v0, vtkIdType v1,
                              vtkIdType v2, vtkIdType v3, vtkIdType v4,
                              vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddPyramid(z, v0, v1, v2, v3, v4); 
        pyramidTags.push_back(tag);
    }
    void           AddTet(vtkIdType z,  vtkIdType v0, vtkIdType v1,
                          vtkIdType v2, vtkIdType v3,
                          vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddTet(z, v0, v1, v2, v3);
        tetTags.push_back(tag);
    }
    void           AddQuad(vtkIdType z,  vtkIdType v0, vtkIdType v1,
                           vtkIdType v2, vtkIdType v3,
                           vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddQuad(z, v0, v1, v2, v3);
        quadTags.push_back(tag);
    }
    void           AddTri(vtkIdType z,  vtkIdType v0, vtkIdType v1,
                          vtkIdType v2,
                          vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddTri(z, v0, v1, v2);
        triTags.push_back(tag);
    }
    void           AddLine(vtkIdType z, vtkIdType v0, vtkIdType v1,
                           vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddLine(z, v0, v1);
        lineTags.push_back(tag);
    }
    void           AddVertex(vtkIdType z, vtkIdType v0,
                             vtkCSGFixedLengthBitField &tag)
    {
        vtkVolumeFromVolume::AddVertex(z, v0);
        vertexTags.push_back(tag);
    }


    void                    InitTraversal();
    size_t                  GetNumberOfCells() const;
    void                    NextCell();
    const vtkIdType        *GetCell();
    vtkCSGFixedLengthBitField GetTag();
    size_t                  GetCellSize() const;
    unsigned char           GetCellVTKType() const;
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

    size_t               ishape;
    size_t               shapeCnt[8];
    ShapeList           *curShapes;
    size_t               curShapeCnt;
    size_t               curShapeSize;
    unsigned char        curShapeVTKType;
    size_t               curShape;

    vtkVolumeFromCSGVolume(const vtkVolumeFromCSGVolume&) = delete;
    void operator=(const vtkVolumeFromCSGVolume&) = delete;
};

#endif
