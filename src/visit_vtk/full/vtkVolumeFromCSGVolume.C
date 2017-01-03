/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                         vtkVolumeFromCSGVolume.C                          //
// ************************************************************************* //

#include <vtkVolumeFromCSGVolume.h>

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

vtkVolumeFromCSGVolume::vtkVolumeFromCSGVolume(int nPts, int ptSizeGuess)
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
    int conn_size = 0;
    for (int i = 0 ; i < nshapes ; i++)
    {
        const vtkIdType *list;
        int nlists = shapes[i]->GetNumberOfLists();
        int shapesize = shapes[i]->GetShapeSize();
        for (vtkIdType j = 0 ; j < nlists ; j++)
        {
            int listSize = shapes[i]->GetList(j, list);
            for (int k = 0 ; k < listSize ; k++)
            {
                if (list[0] != -1)
                {
                    ncells++;
                    conn_size += shapesize+1;
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
    int current_index = 0;
    for (int i = 0 ; i < nshapes ; i++)
    {
        const vtkIdType *list;
        int nlists = shapes[i]->GetNumberOfLists();
        int shapesize = shapes[i]->GetShapeSize();
        int vtk_type = shapes[i]->GetVTKType();
        for (vtkIdType j = 0 ; j < nlists ; j++)
        {
            int listSize = shapes[i]->GetList(j, list);
            for (int k = 0 ; k < listSize ; k++)
            {
                if (list[0] == -1)
                {
                    list += shapesize+1;
                    continue;
                }
                outCD->CopyData(inCD, list[0], cellId);
                for (int l = 0 ; l < shapesize ; l++)
                {
                    ids[l] = list[l+1];
                }
                list += shapesize+1;
                *nl++ = shapesize;
                *cl++ = current_index;
                *ct++ = vtk_type;
                for (int l = 0 ; l < shapesize ; l++)
                    *nl++ = ids[l];
                current_index += shapesize+1;
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
    for (int i = 0 ; i < nshapes ; i++)
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
    for (int i = 0; i < nshapes; i++)
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
        curShapes = NULL;
        curShapeCnt = 0;
        curShapeSize = 0;
        curShapeVTKType = 0;
        curTags = NULL;
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

vtkIdType
vtkVolumeFromCSGVolume::GetNumberOfCells() const
{
    vtkIdType nCells = 0;
    for (int i = 0; i < nshapes; i++)
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
    vtkIdType ilist = curShape / 1024;
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

int
vtkVolumeFromCSGVolume::GetCellSize() const
{
    return curShapeSize - 1;
}

// ****************************************************************************
//  Method: vtkVolumeFromCSGVolume::GetCellSize
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

int
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
    vtkIdType ilist = curShape / 1024;
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
    int ishape = 0;
    while (ishape < nshapes &&
           shapeCnt[ishape] == shapes[ishape]->GetTotalNumberOfShapes())
        ishape++;
    if (ishape >= nshapes)
        return;

    //
    // Construct all the points that are along edges and add them to
    // the points list.
    //
    int nLists = pt_list.GetNumberOfLists();
    for (int i = 0 ; i < nLists ; i++)
    {
        const PointEntry *pe_list = NULL;
        const int nPts = pt_list.GetList(i, pe_list);
        for (int j = 0 ; j < nPts ; j++)
        {
            const PointEntry &pe = pe_list[j];
            const int idx1 = pe.ptIds[0];
            const int idx2 = pe.ptIds[1];

            const float *pt1 = &pts[3*idx1];
            const float *pt2 = &pts[3*idx2];

            // Now that we have the original points, calculate the new one.
            const float p  = pe.percent;
            const float bp = 1. - p;
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
    const int centroidStart = (int)pts.size() / 3;
    nLists = centroid_list.GetNumberOfLists();
    for (int i = 0 ; i < nLists ; i++)
    {
        const CentroidPointEntry *ce_list = NULL;
        const int nPts = centroid_list.GetList(i, ce_list);
        for (int j = 0 ; j < nPts ; j++)
        {
            const CentroidPointEntry &ce = ce_list[j];
            const float weight_factor = 1. / ce.nPts;
            float pt[3] = {0., 0., 0.};
            for (int k = 0 ; k < ce.nPts ; k++)
            {
                pt[0] += pts[ce.ptIds[k]*3];
                pt[1] += pts[ce.ptIds[k]*3+1];
                pt[2] += pts[ce.ptIds[k]*3+2];
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
    numPrevPts = pts.size() / 3;

    //
    // Update the shape data.
    //
    for (int i = 0 ; i < nshapes ; i++)
    {
        vtkIdType *list;
        nLists = shapes[i]->GetNumberOfLists();
        int shapesize = shapes[i]->GetShapeSize();
        for (vtkIdType j = 0 ; j < nLists ; j++)
        {
            int listSize = shapes[i]->GetList(j, list);
            for (int k = 0 ; k < listSize ; k++)
            {
                if (list[0] != -1)
                {
                    for (int l = 0 ; l < shapesize ; l++)
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
