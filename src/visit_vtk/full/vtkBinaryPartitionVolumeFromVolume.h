/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#ifndef VTK_BINARY_PARTITION_VOLUME_FROM_VOLUME_H
#define VTK_BINARY_PARTITION_VOLUME_FROM_VOLUME_H

#include <vtkVolumeFromVolume.h>
#include <FixedLengthBitField.h>

#include <vector>


class vtkCellData;
class vtkPointData;
class vtkPolyData;
class vtkUnstructuredGrid;


// ****************************************************************************
//  Class: vtkBinaryPartitionVolumeFromVolume
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
//  Programmer: Jeremy Meredith
//  Creation:   February 26, 2010
//
//  Modifications:
//
// ****************************************************************************

class vtkBinaryPartitionVolumeFromVolume : private vtkVolumeFromVolume
{
  protected:


  public:
                      vtkBinaryPartitionVolumeFromVolume(int nPts,
                                                         int ptSizeGuess);
    virtual          ~vtkBinaryPartitionVolumeFromVolume() { ; };

    void              ConstructDataSet(vtkPointData *, vtkCellData *,
                                       vtkUnstructuredGrid *, float *,
                                       std::vector<FixedLengthBitField<16> > *oldTags,
                                       std::vector<FixedLengthBitField<16> > *newTags,
                                       int newTagBit);
    void              ConstructDataSet(vtkPointData *, vtkCellData *,
                                       vtkUnstructuredGrid *, int *,
                                       float *, float *,float *,
                                       std::vector<FixedLengthBitField<16> > *oldTags,
                                       std::vector<FixedLengthBitField<16> > *newTags,
                                       int newTagBit);

    using vtkVolumeFromVolume::AddCentroidPoint;
    using vtkDataSetFromVolume::AddPoint;

    void           AddHex(int z, int v0, int v1, int v2, int v3,
                          int v4, int v5, int v6, int v7,
                          bool inOut)
    {
        vtkVolumeFromVolume::AddHex(z, v0, v1, v2, v3, v4, v5, v6, v7);
        hexTags.push_back(inOut ? 1 : 0);
    }
        
    void           AddWedge(int z,int v0,int v1,int v2,int v3,int v4,int v5,
                            bool inOut)
    {
        vtkVolumeFromVolume::AddWedge(z, v0, v1, v2, v3, v4, v5);
        wedgeTags.push_back(inOut ? 1 : 0);
    }
    void           AddPyramid(int z, int v0, int v1, int v2, int v3, int v4,
                              bool inOut)
    {
        vtkVolumeFromVolume::AddPyramid(z, v0, v1, v2, v3, v4); 
        pyramidTags.push_back(inOut ? 1 : 0);
    }
    void           AddTet(int z, int v0, int v1, int v2, int v3,
                          bool inOut)
    {
        vtkVolumeFromVolume::AddTet(z, v0, v1, v2, v3);
        tetTags.push_back(inOut ? 1 : 0);
    }
    void           AddQuad(int z, int v0, int v1, int v2, int v3,
                           bool inOut)
    {
        vtkVolumeFromVolume::AddQuad(z, v0, v1, v2, v3);
        quadTags.push_back(inOut ? 1 : 0);
    }
    void           AddTri(int z, int v0, int v1, int v2,
                          bool inOut)
    {
        vtkVolumeFromVolume::AddTri(z, v0, v1, v2);
        triTags.push_back(inOut ? 1 : 0);
    }
    void           AddLine(int z, int v0, int v1,
                           bool inOut)
    {
        vtkVolumeFromVolume::AddLine(z, v0, v1);
        lineTags.push_back(inOut ? 1 : 0);
    }
    void           AddVertex(int z, int v0,
                             bool inOut)
    {
        vtkVolumeFromVolume::AddVertex(z, v0);
        vertexTags.push_back(inOut ? 1 : 0);
    }

  protected:
    std::vector<int>   hexTags;
    std::vector<int>   wedgeTags;
    std::vector<int>   pyramidTags;
    std::vector<int>   tetTags;
    std::vector<int>   quadTags;
    std::vector<int>   triTags;
    std::vector<int>   lineTags;
    std::vector<int>   vertexTags;
    std::vector<int>  *shapeTags[8];

    void               ConstructDataSet(vtkPointData *, vtkCellData *,
                                        vtkUnstructuredGrid *, 
                                        vtkVolumeFromVolume::CommonPointsStructure &,
                                        std::vector<FixedLengthBitField<16> > *oldTags,
                                        std::vector<FixedLengthBitField<16> > *newTags,
                                        int newTagBit);
};


#endif


