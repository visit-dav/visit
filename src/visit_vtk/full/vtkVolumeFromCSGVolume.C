// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         vtkVolumeFromCSGVolume.C                          //
// ************************************************************************* //

#include "vtkVolumeFromCSGVolume.h"

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
//  Method: vtkVolumeFromCSGVolume constructor
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//
// ****************************************************************************

vtkVolumeFromCSGVolume::vtkVolumeFromCSGVolume(vtkIdType nPts, size_t ptSizeGuess)
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

    curTags = nullptr;

    ishape = 0;
    for (size_t i = 0; i < 8; ++i)
        shapeCnt[i] = 0;
    curShapes = nullptr;
    curShapeCnt = 0;
    curShapeSize = 0;
    curShapeVTKType = VTK_EMPTY_CELL;
    curShape = 0;
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::ConstructDataSet
//
//  Purpose:
//    Constructs the output dataset and updates the tags.
//    We simply update the tags here, then defer to the base class to do
//    the actual construction.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//    Eric Brugger, Thu Apr  3 08:31:24 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//    
// ****************************************************************************

void
vtkVolumeFromCSGVolume::ConstructDataSet(vtkCellData *inCD,
                                    vtkUnstructuredGrid *output,
                                    float *pts_ptr, int npts,
                                    vector<vtkCSGFixedLengthBitField> *tags)
{
    vtkCellData  *outCD = output->GetCellData();

    //
    // Create the output points.
    //
    vtkPoints *outPts = vtkPoints::New(VTK_FLOAT);
    outPts->SetNumberOfPoints(npts);

    for (int i = 0; i < npts; i++)
        outPts->SetPoint(i, pts_ptr + 3*i);

    output->SetPoints(outPts);
    outPts->Delete();

    //
    // Create the output shapes and cell data.
    //
    int ncells = 0;
    vtkIdType conn_size = 0;
    for (size_t i = 0 ; i < nshapes ; i++)
    {
        const vtkIdType *list;
        size_t nlists = shapes[i]->GetNumberOfLists();
        size_t shapesize = shapes[i]->GetShapeSize();
        for (size_t j = 0 ; j < nlists ; j++)
        {
            size_t listSize = shapes[i]->GetList(j, list);
            for (size_t k = 0 ; k < listSize ; k++)
            {
                if (list[0] != -1)
                {
                    ncells++;
                    conn_size += static_cast<vtkIdType>(shapesize+1);
                }
                list += shapesize+1;
            }
        }
    }

    outCD->CopyAllocate(inCD, ncells);

    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues(conn_size);
    vtkIdType *nl = nlist->GetPointer(0);

    vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
    cellTypes->SetNumberOfValues(ncells);
    unsigned char *ct = cellTypes->GetPointer(0);

    vtkIdTypeArray *cellLocations = vtkIdTypeArray::New();
    cellLocations->SetNumberOfValues(ncells);
    vtkIdType *cl = cellLocations->GetPointer(0);

    vtkIdType ids[1024]; // 8 (for hex) should be max, but...
    int cellId = 0;
    vtkIdType current_index = 0;
    for (size_t i = 0 ; i < nshapes ; i++)
    {
        const vtkIdType *list;
        size_t nlists = shapes[i]->GetNumberOfLists();
        size_t shapesize = shapes[i]->GetShapeSize();
        unsigned char vtk_type = shapes[i]->GetVTKType();
        for (size_t j = 0 ; j < nlists ; j++)
        {
            size_t listSize = shapes[i]->GetList(j, list);
            for (size_t k = 0 ; k < listSize ; k++)
            {
                if (list[0] == -1)
                {
                    list += shapesize+1;
                    continue;
                }
                outCD->CopyData(inCD, list[0], cellId);
                for (size_t l = 0 ; l < shapesize ; l++)
                {
                    ids[l] = list[l+1];
                }
                list += shapesize+1;
                *nl++ = static_cast<vtkIdType>(shapesize);
                *cl++ = current_index;
                *ct++ = vtk_type;
                for (size_t l = 0 ; l < shapesize ; l++)
                    *nl++ = ids[l];
                current_index += static_cast<vtkIdType>(shapesize+1);
                cellId++;
            }
        }
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(ncells, nlist);
    nlist->Delete();

    output->SetCells(cellTypes, cellLocations, cells);
    cellTypes->Delete();
    cellLocations->Delete();
    cells->Delete();

    //
    // Create the output tags.
    //
    tags->clear();
    for (size_t i = 0 ; i < nshapes ; i++)
    {
        vector<vtkCSGFixedLengthBitField> *tagList = shapeTags[i];
        for (size_t j = 0; j < tagList->size(); j++)
        {
            if (tagList->operator[](j).TestBit(VTK_CSG_MAX_BITS) == false)
               tags->push_back(tagList->operator[](j));
        }
    }
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::InitTraversal
//
//  Purpose:
//    Initialize the traversal of the cells.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//    
// ****************************************************************************

void
vtkVolumeFromCSGVolume::InitTraversal()
{
    for (size_t i = 0; i < nshapes; i++)
        shapeCnt[i] = shapes[i]->GetTotalNumberOfShapes();

    ishape = 0;
    while (ishape < nshapes && shapeCnt[ishape] == 0)
        ishape++;
    if  (ishape < nshapes)
    {
        curShapes = shapes[ishape];
        curShapeCnt = shapeCnt[ishape];
        curShapeSize = shapes[ishape]->GetShapeSize() + 1;
        curShapeVTKType = shapes[ishape]->GetVTKType();
        curTags = shapeTags[ishape];
    }
    else
    {
        curShapes = nullptr;
        curShapeCnt = 0;
        curShapeSize = 0;
        curShapeVTKType = 0;
        curTags = nullptr;
    }
    curShape = 0;
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::GetNumberOfCells
//
//  Purpose:
//    Return the number of cells.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//
//    Burlen Loring, Sun Sep  6 14:58:03 PDT 2015
//    Changed the return type of GetNumberOfCells to vtkIdType
//
// ****************************************************************************

size_t
vtkVolumeFromCSGVolume::GetNumberOfCells() const
{
    size_t nCells = 0;
    for (size_t i = 0; i < nshapes; i++)
        nCells += shapeCnt[i];
    return nCells;
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::NextCell
//
//  Purpose:
//    Advance to the next cell.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//    
// ****************************************************************************

void
vtkVolumeFromCSGVolume::NextCell()
{
    curShape++;
    while (curShape >= curShapeCnt && ishape < nshapes - 1)
    {
        ishape++;
        curShapes = shapes[ishape];
        curShapeCnt = shapeCnt[ishape];
        curShapeSize = shapes[ishape]->GetShapeSize() + 1;
        curShapeVTKType = shapes[ishape]->GetVTKType();
        curTags = shapeTags[ishape];
        curShape = 0;
    }
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::GetCell
//
//  Purpose:
//    Get the current cell.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//    
// ****************************************************************************

const vtkIdType *
vtkVolumeFromCSGVolume::GetCell()
{
    const vtkIdType *list;
    size_t ilist = curShape / 1024;
    curShapes->GetList(ilist, list);
    return (&(list[(curShape % 1024)*curShapeSize]));
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::GetTag
//
//  Purpose:
//    Get the current tag.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//    Eric Brugger, Thu Apr  3 08:31:24 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//    
// ****************************************************************************

vtkCSGFixedLengthBitField
vtkVolumeFromCSGVolume::GetTag()
{
    return curTags->operator[](curShape);
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::GetCellSize
//
//  Purpose:
//    Get the size of the current cell.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//    
// ****************************************************************************

size_t
vtkVolumeFromCSGVolume::GetCellSize() const
{
    return curShapeSize - 1;
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::GetCellVTKType
//
//  Purpose:
//    Get the VTK cell type of the current cell.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//    
// ****************************************************************************

unsigned char
vtkVolumeFromCSGVolume::GetCellVTKType() const
{
    return curShapeVTKType;
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::SetId
//
//  Purpose:
//    Set the id of the current cell.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//    
// ****************************************************************************

void
vtkVolumeFromCSGVolume::SetId(int id)
{
    vtkIdType *list;
    size_t ilist = curShape / 1024;
    curShapes->GetList(ilist, list);
    list[(curShape % 1024)*curShapeSize] = id;
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::SetTagBit
//
//  Purpose:
//    Set the tag bit of the current cell.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//    
// ****************************************************************************

void
vtkVolumeFromCSGVolume::SetTagBit(int tagBit)
{
    curTags->operator[](curShape).SetBit(tagBit);
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::InvalideCell
//
//  Purpose:
//    Invalidate the current cell (set the last tag bit and set the id to -1.)
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//    Eric Brugger, Thu Apr  3 08:31:24 PDT 2014
//    I converted the class to use vtkCSGFixedLengthBitField instead of
//    FixedLengthBitField.
//    
// ****************************************************************************

void
vtkVolumeFromCSGVolume::InvalidateCell()
{
    curTags->operator[](curShape).SetBit(VTK_CSG_MAX_BITS);
    SetId(-1);
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::InvalideCell
//
//  Purpose:
//    Update the points.
//
//  Programmer: Eric Brugger
//  Creation:   July 23, 2012
//
//  Modifications:
//    
// ****************************************************************************

void
vtkVolumeFromCSGVolume::UpdatePoints(vector<float> &pts)
{
    //
    // Determine if no shapes have been added and exit early if so.
    //
    size_t ishape2 = 0;
    while (ishape2 < nshapes &&
           shapeCnt[ishape2] == shapes[ishape2]->GetTotalNumberOfShapes())
        ishape2++;
    if (ishape2 >= nshapes)
        return;

    //
    // Construct all the points that are along edges and add them to
    // the points list.
    //
    size_t nLists = pt_list.GetNumberOfLists();
    for (size_t i = 0 ; i < nLists ; i++)
    {
        const PointEntry *pe_list = nullptr;
        const size_t nPts = pt_list.GetList(i, pe_list);
        for (size_t j = 0 ; j < nPts ; j++)
        {
            const PointEntry &pe = pe_list[j];
            const vtkIdType idx1 = pe.ptIds[0];
            const vtkIdType idx2 = pe.ptIds[1];

            const float *pt1 = &pts[size_t(3*idx1)];
            const float *pt2 = &pts[size_t(3*idx2)];

            // Now that we have the original points, calculate the new one.
            const float p  = pe.percent;
            const float bp = 1.f - p;
            float pt[3];
            pt[0] = pt1[0]*p + pt2[0]*bp;
            pt[1] = pt1[1]*p + pt2[1]*bp;
            pt[2] = pt1[2]*p + pt2[2]*bp;

            pts.push_back(pt[0]);
            pts.push_back(pt[1]);
            pts.push_back(pt[2]);
        }
    }
    pt_list.Clear();
    edges.Clear();

    //
    // Construct all the centroid points and add them to the points list.
    //
    const int centroidStart = static_cast<int>(pts.size()) / 3;
    nLists = centroid_list.GetNumberOfLists();
    for (size_t i = 0 ; i < nLists ; i++)
    {
        const CentroidPointEntry *ce_list = nullptr;
        const size_t nPts = centroid_list.GetList(i, ce_list);
        for (size_t j = 0 ; j < nPts ; j++)
        {
            const CentroidPointEntry &ce = ce_list[j];
            const float weight_factor = 1.f / float(ce.nPts);
            float pt[3] = {0.f, 0.f, 0.f};
            for (size_t k = 0 ; k < ce.nPts ; k++)
            {
                pt[0] += pts[size_t(ce.ptIds[k]*3)];
                pt[1] += pts[size_t(ce.ptIds[k]*3+1)];
                pt[2] += pts[size_t(ce.ptIds[k]*3+2)];
            }
            pt[0] *= weight_factor;
            pt[1] *= weight_factor;
            pt[2] *= weight_factor;

            pts.push_back(pt[0]);
            pts.push_back(pt[1]);
            pts.push_back(pt[2]);
        }
    }
    centroid_list.Clear();
    numPrevPts = vtkIdType(pts.size() / 3);

    //
    // Update the shape data.
    //
    for (size_t i = 0 ; i < nshapes ; i++)
    {
        vtkIdType *list;
        nLists = shapes[i]->GetNumberOfLists();
        size_t shapesize = shapes[i]->GetShapeSize();
        for (size_t j = 0 ; j < nLists ; j++)
        {
            size_t listSize = shapes[i]->GetList(j, list);
            for (size_t k = 0 ; k < listSize ; k++)
            {
                if (list[0] != -1)
                {
                    for (size_t l = 0 ; l < shapesize ; l++)
                    {
                        if (list[l+1] < 0)
                            list[l+1] = centroidStart-1 - list[l+1];
                    }
                }
                list += shapesize+1;
            }
        }
    }
}
