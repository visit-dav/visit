// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVertexExtractor.h"

#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkVertexExtractor);

//----------------------------------------------------------------------------
vtkVertexExtractor::vtkVertexExtractor()
{
}

//----------------------------------------------------------------------------
vtkVertexExtractor::~vtkVertexExtractor()
{
}

//----------------------------------------------------------------------------
int vtkVertexExtractor::RequestData( vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    vtkPointSet *input = vtkPointSet::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPointSet *output = vtkPointSet::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    // Handle the trivial case
    vtkIdType numCells = input->GetNumberOfCells();
    if (numCells <= 0)
    {
        output->Initialize(); //output is empty
        return 1;
    }

    if ( !(input->GetDataObjectType() == VTK_POLY_DATA ||
           input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID ))
    {
        output->Initialize(); //output is empty
    }


    vtkIdType numPts = input->GetNumberOfPoints();

    vtkIdType *ptMap = new vtkIdType [numPts];
    std::fill_n(ptMap,numPts,-1);

    vtkCellData *inCD = input->GetCellData();
    vtkCellData *outCD = output->GetCellData();
    outCD->CopyAllocate(inCD);

    vtkIdType numNewPts = 0;
    vtkIdList *ptIds = vtkIdList::New();
    ptIds->SetNumberOfIds(1);


    if (input->GetDataObjectType() == VTK_POLY_DATA)
    {
        vtkPolyData *inputPoly = vtkPolyData::SafeDownCast(input);
        if(inputPoly->GetNumberOfVerts() > 0)
        {
            vtkIdType cellId = 0, currentCellId = 0;
            vtkIdType npts;
            const vtkIdType *pts;
            vtkCellArray *inVerts = inputPoly->GetVerts(); 
            vtkCellArray *verts = vtkCellArray::New();
            for (inVerts->InitTraversal(); inVerts->GetNextCell(npts, pts);
                 ++currentCellId)
            {
                for (vtkIdType i = 0; i < npts; ++i)
                {
                    if (ptMap[pts[i]] < 0)
                    {
                        ptMap[pts[i]] = numNewPts++;
                    }
                    ptIds->SetId(0,ptMap[pts[i]]);
                    cellId = verts->InsertNextCell(ptIds);
                    outCD->CopyData(inCD,currentCellId,cellId);
                }
            }
            vtkPolyData *outputPoly = vtkPolyData::SafeDownCast(output);
            outputPoly->SetVerts(verts);
            verts->Delete();
        }
    }
    else // VTK_UNSTRUCTURED_GRID
    {
        vtkUnstructuredGrid *inputUG = vtkUnstructuredGrid::SafeDownCast(input);
        vtkUnstructuredGrid *outputUG = vtkUnstructuredGrid::SafeDownCast(output);

        vtkIdType newCellId = 0, npts = 0, ptId = 0;
        vtkIdList *pts = vtkIdList::New();
        outputUG->Allocate(numCells);
        for (vtkIdType cellId=0; cellId < numCells; ++cellId)
        {
            int cellType = input->GetCellType(cellId);
            if (cellType == VTK_POLY_VERTEX || cellType == VTK_VERTEX)
            {
                input->GetCellPoints(cellId, pts);
                for (vtkIdType i = 0; i < pts->GetNumberOfIds(); ++i)
                {
                    ptId = pts->GetId(i);
                    if (ptMap[ptId] < 0)
                    {
                        ptMap[ptId] = numNewPts++;
                    }
                    ptIds->SetId(0,ptMap[ptId]);
                    newCellId = outputUG->InsertNextCell(VTK_VERTEX,ptIds);
                    outCD->CopyData(inCD,cellId,newCellId);
                }
            }
        }
    }


    if(numNewPts > 0)
    {
        vtkPointData *inPD = input->GetPointData();
        vtkPointData *outPD = output->GetPointData();
        // Copy referenced input points to new points array
        outPD->CopyAllocate(inPD);
        vtkPoints *inPts = input->GetPoints();
        vtkPoints *outPts = vtkPoints::New();
        outPts->SetNumberOfPoints(numNewPts);
        for (vtkIdType ptId=0; ptId < numPts; ++ptId)
        {
            if (ptMap[ptId] >= 0)
            {
                outPts->SetPoint(ptMap[ptId], inPts->GetPoint(ptId));
                outPD->CopyData(inPD,ptId,ptMap[ptId]);
            }
        }
        output->SetPoints(outPts);
        outPts->Delete();
    }
    else
    {
        output->Initialize(); //output is empty
    }

    // Clean up
    ptIds->Delete();
    delete [] ptMap;

    return 1;
}

//----------------------------------------------------------------------------
int vtkVertexExtractor::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPointSet");
  return 1;
}

