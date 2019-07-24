// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//             vtkBinaryPartitionVolumeFromVolume.C                          //
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

vtkBinaryPartitionVolumeFromVolume::vtkBinaryPartitionVolumeFromVolume(
    vtkIdType nPts, vtkIdType ptSizeGuess) : vtkVolumeFromVolume(nPts, ptSizeGuess)
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
//    Brad Whitlock, Thu Mar 22 15:29:44 PDT 2012
//    Call ComputeTags instead of going into generic ConstructDataSet.
//
//    Eric Brugger, Wed Jul 25 09:51:52 PDT 2012
//    Increase the number of boundaries that can be handled by the mulit-pass
//    CSG discretization from 128 to 512.
//
//    Eric Brugger, Wed Apr  2 12:22:08 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//
// ****************************************************************************

void
vtkBinaryPartitionVolumeFromVolume::ConstructDataSet(vtkPointData *inPD,
    vtkCellData *inCD, vtkUnstructuredGrid *output, vtkPoints *pts,
    vector<vtkCSGFixedLengthBitField> *oldTags,
    vector<vtkCSGFixedLengthBitField> *newTags,
    int newTagBit)
{
    ComputeTags(oldTags, newTags, newTagBit);
    vtkVolumeFromVolume::ConstructDataSet(inPD, inCD, output, pts);
}

void
vtkBinaryPartitionVolumeFromVolume::ConstructDataSet(vtkPointData *inPD,
    vtkCellData *inCD, vtkUnstructuredGrid *output, const int *dims,
    vtkDataArray *X, vtkDataArray *Y, vtkDataArray *Z,
    vector<vtkCSGFixedLengthBitField> *oldTags,
    vector<vtkCSGFixedLengthBitField> *newTags,
    int newTagBit)
{
    ComputeTags(oldTags, newTags, newTagBit);
    vtkVolumeFromVolume::ConstructDataSet(inPD, inCD, output, dims, X, Y, Z);
}

void
vtkBinaryPartitionVolumeFromVolume::ComputeTags(
    vector<vtkCSGFixedLengthBitField> *oldTags,
    vector<vtkCSGFixedLengthBitField> *newTags,
    int newTagBit)
{
    size_t ncells = 0;
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
        const vtkIdType *shapeList;
        const vector<vtkIdType> *tagList = shapeTags[i];
        vtkIdType nlists = shapes[i]->GetNumberOfLists();
        int shapesize = shapes[i]->GetShapeSize();
        int indexInShape = 0;
        for (int j = 0 ; j < nlists ; j++)
        {
            int listSize = shapes[i]->GetList(j, shapeList);
            for (int k = 0 ; k < listSize ; k++)
            {
                // Update the partition bit
                bool half = !(tagList->operator[](indexInShape));
                vtkCSGFixedLengthBitField bf;
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
}


