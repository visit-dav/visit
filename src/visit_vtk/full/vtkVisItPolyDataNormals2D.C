// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVisItPolyDataNormals2D.h"

#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkTriangle.h>


vtkStandardNewMacro(vtkVisItPolyDataNormals2D);

// ****************************************************************************
//  Constructor:  vtkVisItPolyDataNormals2D::vtkVisItPolyDataNormals2D
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 10, 2014
//
// ****************************************************************************
vtkVisItPolyDataNormals2D::vtkVisItPolyDataNormals2D()
{
    ComputePointNormals = true;
}


// ****************************************************************************
//  Method:  vtkVisItPolyDataNormals2D::Execute
//
//  Purpose:
//    main update function; decide which type of algorithm to perform
//
//  Arguments:
//    none
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 10, 2014
//
//  Modifications:
// ****************************************************************************
int
vtkVisItPolyDataNormals2D::RequestData(vtkInformation *vtkNotUsed(request),
    vtkInformationVector **vtkNotUsed(inputVector),
    vtkInformationVector *vtkNotUsed(outputVector))
{
    // get the input and output
    vtkPolyData *input = GetPolyDataInput(0);
    vtkPolyData *output = GetOutput();

    if (ComputePointNormals)
    {
        // Point normals
        if (input->GetPointData()->GetNormals() != NULL)
        {
            GetOutput()->ShallowCopy(this->GetInput());
            return 1;
        }

        ExecutePoint(input, output);
    }
    else
    {
        // Cell normals
        if (input->GetCellData()->GetNormals() != NULL)
        {
            GetOutput()->ShallowCopy(this->GetInput());
            return 1;
        }

        ExecuteCell(input, output);
    }
    return 1;
}


// ****************************************************************************
//  Method:  vtkVisItPolyDataNormals2D::ExecutePoint
//
//  Purpose:
//    Create normals at the points by averaging from neighboring cells.
//
//  Arguments:
//    none
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 10, 2014
//
//  Modifications:
//    Kathleen Biagas, THu Aug 11, 2022
//    Support VTK9: use vtkCellArrayIterator.
//
// ****************************************************************************
void
vtkVisItPolyDataNormals2D::ExecutePoint(
  vtkPolyData *input, vtkPolyData *output)
{
    // Get all the input and output objects we'll need to reference
    vtkCellArray *inCA  = input->GetPolys();
    vtkCellArray *inCL  = input->GetLines();
    vtkPointData *inPD  = input->GetPointData();
    vtkCellData  *inCD  = input->GetCellData();
    vtkPoints    *inPts = input->GetPoints();

    vtkIdType nPolys  = inCA->GetNumberOfCells();
    vtkIdType nLines  = inCL->GetNumberOfCells();
    vtkIdType nVerts  = input->GetVerts()->GetNumberOfCells();
    vtkIdType nTotalCells = nPolys + nLines + nVerts;

    vtkIdType nPoints = input->GetNumberOfPoints();

    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    // Pass through things which will be unchanged
    output->GetFieldData()->ShallowCopy(input->GetFieldData());

    // Allocate and copy the output points; there will be no extras
    vtkPoints *outPts = vtkPoints::New(inPts->GetDataType());
    outPts->SetNumberOfPoints(nPoints);
    outPD->CopyAllocate(inPD,nPoints);
    vtkIdType ptIdx = 0;
    for (vtkIdType i = 0 ; i < nPoints ; i++)
    {
        double pt[3];
        inPts->GetPoint(i, pt);
        outPts->SetPoint(ptIdx, pt);
        outPD->CopyData(inPD, i, ptIdx);
        ptIdx++;
    }
    output->SetPoints(outPts);
    outPts->Delete();

    // Create and initialize the normals array
    vtkFloatArray *newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->SetNumberOfTuples(nPoints);
    newNormals->SetName("Normals");
    // Accumulate in double-array since VTK computes double normal
    double *dnormals = new double[nPoints*3];
    for (vtkIdType i = 0 ; i < nPoints ; i++)
    {
        dnormals[i*3+0] = 0.;
        dnormals[i*3+1] = 0.;
        dnormals[i*3+2] = 0.;
    }

    // Create the output cells, accumulating cell normals to the points
    output->Allocate(inCL->GetNumberOfConnectivityEntries());
    outCD->CopyAllocate(inCD, nTotalCells);

    auto connPtrL = vtk::TakeSmartPointer(inCL->NewIterator());
    vtkIdType i = 0;
    for (connPtrL->GoToFirstCell(); !connPtrL->IsDoneWithTraversal(); connPtrL->GoToNextCell(), ++i)
    {
        outCD->CopyData(inCD, nVerts+i, nVerts+i);
        vtkIdType nPtIds;
        const vtkIdType *ptIds;
        connPtrL->GetCurrentCell(nPtIds, ptIds);
        int type = VTK_LINE;
        if (nPtIds != 2)
            type = VTK_POLY_LINE;
        output->InsertNextCell(type, nPtIds, ptIds);

        double pt0[3], pt1[3];
        inPts->GetPoint(ptIds[0], pt0);
        inPts->GetPoint(ptIds[1], pt1);
        double dx = pt1[0] - pt0[0];
        double dy = pt1[1] - pt0[1];
        // this gets normalized later
        double normal[3];
        normal[0] = dy;
        normal[1] = -dx;
        normal[2] = 0;
        for (vtkIdType j = 0 ; j < nPtIds ; j++)
        {
            vtkIdType p = ptIds[j];
            dnormals[p*3+0] += normal[0];
            dnormals[p*3+1] += normal[1];
            dnormals[p*3+2] += normal[2];
        }
    }

    // Renormalize the normals; they've only been accumulated so far,
    // and store in the vtkFloatArray.
    float *newNormalPtr = (float*)newNormals->GetPointer(0);
    for (vtkIdType i = 0 ; i < nPoints ; i++)
    {
        double nx = dnormals[i*3+0];
        double ny = dnormals[i*3+1];
        double nz = dnormals[i*3+2];
        double length = sqrt(nx*nx + ny*ny + nz*nz);
        if (length != 0.0)
        {
            newNormalPtr[i*3+0] = (float)(nx/length);
            newNormalPtr[i*3+1] = (float)(ny/length);
            newNormalPtr[i*3+2] = (float)(nz/length);
        }
        else
        {
            newNormalPtr[i*3+0] = 0.;
            newNormalPtr[i*3+1] = 0.;
            newNormalPtr[i*3+2] = 0.;
        }
    }
    outPD->SetNormals(newNormals);
    newNormals->Delete();
    delete [] dnormals;

    // copy the original vertices to the output
    output->SetVerts(input->GetVerts());

    // copy the data from the vertices now.
    for (vtkIdType i = 0 ; i < nVerts ; i++)
        outCD->CopyData(inCD, i, i);

    // copy the original polys to the output
    output->SetPolys(input->GetPolys());

    // copy the data from the polys now.
    for (vtkIdType i = 0 ; i < nPolys ; i++)
        outCD->CopyData(inCD, nVerts+nLines+i, nVerts+nLines+i);
}


// ****************************************************************************
//  Method:  vtkVisItPolyDataNormals2D::ExecuteCell
//
//  Purpose:
//    Create normals for each cell.
//
//  Arguments:
//    none
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 10, 2014
//
//  Modifications:
//    Kathleen Biagas, THu Aug 11, 2022
//    Support VTK9: use vtkCellArrayIterator.
//
// ****************************************************************************
void
vtkVisItPolyDataNormals2D::ExecuteCell(vtkPolyData *input, vtkPolyData *output)
{
    // Get all the input and output objects we'll need to reference
    output->ShallowCopy(input);

    vtkPoints    *inPts = input->GetPoints();

    vtkIdType nCells  = input->GetNumberOfCells();

    // Create the normals array
    vtkFloatArray *newNormals;
    newNormals = vtkFloatArray::New();
    newNormals->SetNumberOfComponents(3);
    newNormals->SetNumberOfTuples(nCells);
    newNormals->SetName("Normals");
    float *newNormalPtr = (float*)newNormals->GetPointer(0);

    // The verts come before the polys.  So add normals for them.
    vtkIdType numPrimitivesWithoutNormals = 0;
    numPrimitivesWithoutNormals += input->GetVerts()->GetNumberOfCells();
    for (vtkIdType i = 0 ; i < numPrimitivesWithoutNormals ; i++)
    {
        newNormalPtr[0] = 0.;
        newNormalPtr[1] = 0.;
        newNormalPtr[2] = 1.;
        newNormalPtr += 3;
    }

    vtkCellArray *inCL  = input->GetLines();
    auto  connPtrL = vtk::TakeSmartPointer(inCL->NewIterator());
    for (connPtrL->GoToFirstCell(); !connPtrL->IsDoneWithTraversal(); connPtrL->GoToNextCell())
    {
        vtkIdType nVerts;
        const vtkIdType *cell;
        connPtrL->GetCurrentCell(nVerts, cell);
        double v0[3], v1[3];
        double normal[3] = {0, 0, 1};
        if (nVerts == 2)
        {
            inPts->GetPoint(cell[0], v0);
            inPts->GetPoint(cell[1], v1);
            double dx = v1[0] - v0[0];
            double dy = v1[1] - v0[1];
            double len = sqrt(dx*dx + dy*dy);
            if (len != 0)
            {
                normal[0] = dy / len;
                normal[1] = -dx / len;
                normal[2] = 0;
            }
        }
        newNormalPtr[0] = (float)(normal[0]);
        newNormalPtr[1] = (float)(normal[1]);
        newNormalPtr[2] = (float)(normal[2]);
        newNormalPtr += 3;
    }

    // The triangle strips come after the polys.  So add normals for them.
    numPrimitivesWithoutNormals = 0;
    numPrimitivesWithoutNormals += input->GetPolys()->GetNumberOfCells();
    for (vtkIdType i = 0 ; i < numPrimitivesWithoutNormals ; i++)
    {
        newNormalPtr[0] = 0.f;
        newNormalPtr[1] = 0.f;
        newNormalPtr[2] = 1.f;
        newNormalPtr += 3;
    }

    // The triangle strips come after the polys.  So add normals for them.
    numPrimitivesWithoutNormals = 0;
    numPrimitivesWithoutNormals += input->GetStrips()->GetNumberOfCells();
    for (vtkIdType i = 0 ; i < numPrimitivesWithoutNormals ; i++)
    {
        newNormalPtr[0] = 0.f;
        newNormalPtr[1] = 0.f;
        newNormalPtr[2] = 1.f;
        newNormalPtr += 3;
    }

    output->GetCellData()->SetNormals(newNormals);
    newNormals->Delete();
}
