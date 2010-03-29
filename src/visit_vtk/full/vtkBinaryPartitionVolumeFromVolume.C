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
//                           vtkBinaryPartitionVolumeFromVolume.C                          //
// ************************************************************************* //

#include <vtkBinaryPartitionVolumeFromVolume.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>

using std::vector;


// ****************************************************************************
//  Method: vtkBinaryPartitionVolumeFromVolume constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   February 26, 2010
//
//  Modifications:
//
// ****************************************************************************

vtkBinaryPartitionVolumeFromVolume::vtkBinaryPartitionVolumeFromVolume(int nPts, int ptSizeGuess)
    : vtkVolumeFromVolume(nPts, ptSizeGuess)
{
    shapeTags[0] = &tetTags;
    shapeTags[1] = &pyramidTags;
    shapeTags[2] = &wedgeTags;
    shapeTags[3] = &hexTags;
    shapeTags[4] = &quadTags;
    shapeTags[5] = &triTags;
    shapeTags[6] = &lineTags;
    shapeTags[7] = &vertexTags;

    tetTags.reserve(ptSizeGuess / 20);
    pyramidTags.reserve(ptSizeGuess / 20);
    wedgeTags.reserve(ptSizeGuess / 20);
    hexTags.reserve(ptSizeGuess / 20);
    quadTags.reserve(ptSizeGuess / 20);
    triTags.reserve(ptSizeGuess / 20);
    lineTags.reserve(ptSizeGuess / 20);
    vertexTags.reserve(ptSizeGuess / 20);
}


// ****************************************************************************
//  Method: vtkBinaryPartitionVolumeFromVolume::ConstructDataSet
//
//  Purpose:
//      Constructs the output dataset and updates the tags.
//      We simply update the tags here, then defer to the base class
//      to do the actual construction.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 26, 2010
//
//  Modifications:
//    
// ****************************************************************************

void
vtkBinaryPartitionVolumeFromVolume::ConstructDataSet(vtkPointData *inPD, vtkCellData *inCD,
                                    vtkUnstructuredGrid *output,
                                    float *pts_ptr,
                                    vector<FixedLengthBitField<16> > *oldTags,
                                    vector<FixedLengthBitField<16> > *newTags,
                                    int newTagBit)
{
    CommonPointsStructure cps;
    cps.hasPtsList = true;
    cps.pts_ptr = pts_ptr;
    ConstructDataSet(inPD, inCD, output, cps, oldTags, newTags, newTagBit);
}

void
vtkBinaryPartitionVolumeFromVolume::ConstructDataSet(vtkPointData *inPD, vtkCellData *inCD,
                                    vtkUnstructuredGrid *output,
                                    int *dims, float *X, float *Y, float *Z,
                                    vector<FixedLengthBitField<16> > *oldTags,
                                    vector<FixedLengthBitField<16> > *newTags,
                                    int newTagBit)
{
    CommonPointsStructure cps;
    cps.hasPtsList = false;
    cps.dims = dims;
    cps.X = X;
    cps.Y = Y;
    cps.Z = Z;
    ConstructDataSet(inPD, inCD, output, cps, oldTags, newTags, newTagBit);
}

void
vtkBinaryPartitionVolumeFromVolume::ConstructDataSet(vtkPointData *inPD,
     vtkCellData *inCD,
    vtkUnstructuredGrid *output,
    CommonPointsStructure &cps,
    vector<FixedLengthBitField<16> > *oldTags,
    vector<FixedLengthBitField<16> > *newTags,
    int newTagBit)
{
    int ncells = 0;
    for (int i = 0 ; i < nshapes ; i++)
        ncells += shapeTags[i]->size();


    if (newTags)
    {
        newTags->clear();
        newTags->resize(ncells);
    }

    int cellId = 0;
    for (int i = 0 ; i < nshapes ; i++)
    {
        const int *shapeList;
        const vector<int> *tagList = shapeTags[i];
        int nlists = shapes[i]->GetNumberOfLists();
        int shapesize = shapes[i]->GetShapeSize();
        int indexInShape = 0;
        for (int j = 0 ; j < nlists ; j++)
        {
            int listSize = shapes[i]->GetList(j, shapeList);
            for (int k = 0 ; k < listSize ; k++)
            {
                // Update the partition bit
                bool half = !(tagList->operator[](indexInShape));
                FixedLengthBitField<16> bf;
                if (oldTags)
                    bf = oldTags->operator[](shapeList[0]);
                if (newTags && half)
                    bf.SetBit(newTagBit);
                if (newTags)
                    newTags->operator[](cellId) = bf;

                shapeList += shapesize+1;
                cellId++;
                indexInShape++;
            }
        }
    }

    vtkVolumeFromVolume::ConstructDataSet(inPD, inCD, output, cps); 
}


