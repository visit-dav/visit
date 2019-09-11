// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     vtkBinaryPartitionVolumeFromVolume.h                  //
// ************************************************************************* //

#ifndef VTK_BINARY_PARTITION_VOLUME_FROM_VOLUME_H
#define VTK_BINARY_PARTITION_VOLUME_FROM_VOLUME_H

#include <visit_vtk_exports.h>

#include <vtkCSGFixedLengthBitField.h>
#include <vtkVolumeFromVolume.h>

#include <vector>


class vtkCellData;
class vtkDataArray;
class vtkPointData;
class vtkPoints;
class vtkPolyData;
class vtkUnstructuredGrid;


// ****************************************************************************
//  Class: vtkBinaryPartitionVolumeFromVolume
//
//  Purpose:
//      This is a special type of vtkVolumeFromVolume, where each added
//      cell belongs to one half of a binary partition of the original
//      data set.  You specify which half when you add the shape, and
//      when constructing the data set, tell it the index (vtkIdTypeo a bit array)
//      corresponding to the current binary partition function.
//      E.g. on the first pass, you specify bit #0 and "in" cells get the tag
//      0x0 and "out" cels get the tag 0x1.  On the second pass, you specify
//      bit #1, and "in" cells get tagged either 0x0 or 0x1, and "out" cells
//      get tagged either 0x2 or 0x3.  And so on, up to the width of the
//      bit field used here.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 26, 2010
//
//  Modifications:
//   Brad Whitlock, Thu Mar 22 15:31:41 PDT 2012
//   Use vtkIdType. Add double support.
//
//   Eric Brugger, Wed Jul 25 09:49:49 PDT 2012
//   Increase the number of boundaries that can be handled by the mulit-pass
//   CSG discretization from 128 to 512.
//
//   Eric Brugger, Wed Apr  2 12:19:49 PDT 2014
//   I converted the class to use vtkCSGFixedLengthBitField instead of
//   FixedLengthBitField.
//
// ****************************************************************************

class VISIT_VTK_API vtkBinaryPartitionVolumeFromVolume : private vtkVolumeFromVolume
{
  protected:


  public:
                      vtkBinaryPartitionVolumeFromVolume(vtkIdType nPts,
                                                         vtkIdType ptSizeGuess);
    virtual          ~vtkBinaryPartitionVolumeFromVolume() { ; };

    void              ConstructDataSet(vtkPointData *, vtkCellData *,
                                       vtkUnstructuredGrid *, vtkPoints *,
                                       std::vector<vtkCSGFixedLengthBitField> *oldTags,
                                       std::vector<vtkCSGFixedLengthBitField> *newTags,
                                       int newTagBit);
    void              ConstructDataSet(vtkPointData *, vtkCellData *,
                                       vtkUnstructuredGrid *, const int *,
                                       vtkDataArray *, vtkDataArray *, vtkDataArray *,
                                       std::vector<vtkCSGFixedLengthBitField> *oldTags,
                                       std::vector<vtkCSGFixedLengthBitField> *newTags,
                                       int newTagBit);

    using vtkVolumeFromVolume::AddCentroidPoint;
    using vtkDataSetFromVolume::AddPoint;

    void           AddHex(vtkIdType z, vtkIdType v0, vtkIdType v1, vtkIdType v2, vtkIdType v3,
                          vtkIdType v4, vtkIdType v5, vtkIdType v6, vtkIdType v7,
                          bool inOut)
    {
        vtkVolumeFromVolume::AddHex(z, v0, v1, v2, v3, v4, v5, v6, v7);
        hexTags.push_back(inOut ? 1 : 0);
    }

    void           AddWedge(vtkIdType z,vtkIdType v0,vtkIdType v1,vtkIdType v2,vtkIdType v3,vtkIdType v4,vtkIdType v5,
                            bool inOut)
    {
        vtkVolumeFromVolume::AddWedge(z, v0, v1, v2, v3, v4, v5);
        wedgeTags.push_back(inOut ? 1 : 0);
    }
    void           AddPyramid(vtkIdType z, vtkIdType v0, vtkIdType v1, vtkIdType v2, vtkIdType v3, vtkIdType v4,
                              bool inOut)
    {
        vtkVolumeFromVolume::AddPyramid(z, v0, v1, v2, v3, v4);
        pyramidTags.push_back(inOut ? 1 : 0);
    }
    void           AddTet(vtkIdType z, vtkIdType v0, vtkIdType v1, vtkIdType v2, vtkIdType v3,
                          bool inOut)
    {
        vtkVolumeFromVolume::AddTet(z, v0, v1, v2, v3);
        tetTags.push_back(inOut ? 1 : 0);
    }
    void           AddQuad(vtkIdType z, vtkIdType v0, vtkIdType v1, vtkIdType v2, vtkIdType v3,
                           bool inOut)
    {
        vtkVolumeFromVolume::AddQuad(z, v0, v1, v2, v3);
        quadTags.push_back(inOut ? 1 : 0);
    }
    void           AddTri(vtkIdType z, vtkIdType v0, vtkIdType v1, vtkIdType v2,
                          bool inOut)
    {
        vtkVolumeFromVolume::AddTri(z, v0, v1, v2);
        triTags.push_back(inOut ? 1 : 0);
    }
    void           AddLine(vtkIdType z, vtkIdType v0, vtkIdType v1,
                           bool inOut)
    {
        vtkVolumeFromVolume::AddLine(z, v0, v1);
        lineTags.push_back(inOut ? 1 : 0);
    }
    void           AddVertex(vtkIdType z, vtkIdType v0,
                             bool inOut)
    {
        vtkVolumeFromVolume::AddVertex(z, v0);
        vertexTags.push_back(inOut ? 1 : 0);
    }

  protected:
    std::vector<vtkIdType>   hexTags;
    std::vector<vtkIdType>   wedgeTags;
    std::vector<vtkIdType>   pyramidTags;
    std::vector<vtkIdType>   tetTags;
    std::vector<vtkIdType>   quadTags;
    std::vector<vtkIdType>   triTags;
    std::vector<vtkIdType>   lineTags;
    std::vector<vtkIdType>   vertexTags;
    std::vector<vtkIdType>  *shapeTags[8];

    void               ComputeTags(std::vector<vtkCSGFixedLengthBitField> *oldTags,
                                   std::vector<vtkCSGFixedLengthBitField> *newTags,
                                   int newTagBit);
};


#endif


