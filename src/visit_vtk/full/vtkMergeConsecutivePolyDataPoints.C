// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    vtkMergeConsecutivePolyDataPoints.C                    //
// ************************************************************************* //

#include "vtkMergeConsecutivePolyDataPoints.h"

#include <vtkInformation.h>
#include <vtkInformationVector.h>

vtkStandardNewMacro(vtkMergeConsecutivePolyDataPoints);


vtkMergeConsecutivePolyDataPoints::vtkMergeConsecutivePolyDataPoints()
{
}

// ****************************************************************************
//  Method: vtkMergeConsecutivePolyDataPoints::RequestData
//
//  Purpose:
//    The RequestData method that does the actual work of merging consecutive
//    points within polydata primitives.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Oct 15 15:23:20 PDT 2026
//
//  Modifications:
//
// ****************************************************************************

int
vtkMergeConsecutivePolyDataPoints::RequestData(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
    vtkDebugMacro(<<"Executing vtkMergeConsecutivePolyDataPoints");

    // Get the info objects.
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    //
    // Initialize some frequently used values.
    //
    vtkPolyData  *input = vtkPolyData::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPolyData *output = vtkPolyData::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    // Copy input to output.
    output->ShallowCopy(input);

    vtkPoints* inPts = input->GetPoints();
    vtkIdType numPts = input->GetNumberOfPoints();

    vtkDebugMacro(<< "Beginning merging consecutive PolyData points");
    if ((numPts < 1) || (inPts == nullptr))
    {
        vtkDebugMacro(<< "No data to Operate On!");
        return 1;
    }

    vtkIdType* newPts = new vtkIdType[input->GetMaxCellSize()];

    vtkIdType i;
    vtkIdType npts = 0;
    const vtkIdType* pts = nullptr;
    double x1[3], x2[3];

    vtkCellArray *inLines = input->GetLines(), *newLines = nullptr;
    vtkCellArray *inPolys = input->GetPolys(), *newPolys = nullptr;
    vtkCellArray *inStrips = input->GetStrips(), *newStrips = nullptr;

    vtkIdType checkAbortInterval = 0;
    vtkIdType progressCounter = 0;

    // Process the lines.
    if (!this->CheckAbort() && inLines->GetNumberOfCells() > 0)
    {
        newLines = vtkCellArray::New();
        newLines->AllocateExact(inLines->GetNumberOfCells(), inLines->GetNumberOfConnectivityIds());

        vtkDebugMacro(<< "Starting Lines");
        checkAbortInterval = std::min(inLines->GetNumberOfCells() / 10 + 1, (vtkIdType)1000);
        progressCounter = 0;
        for (inLines->InitTraversal(); inLines->GetNextCell(npts, pts); )
        {
            if (progressCounter % checkAbortInterval == 0 && this->CheckAbort())
            {
                break;
            }
            progressCounter++;
            inPts->GetPoint(pts[0], x1);
            newPts[0] = pts[0];
            for (i = 1; i < npts; i++)
            {
                inPts->GetPoint(pts[i], x2);
                if (x1[0] == x2[0] && x1[1] == x2[1] && x1[2] == x2[2])
                {
                    newPts[i] = newPts[i-1];
                }
                else
                {
                    newPts[i] = pts[i];
                }
                x1[0] = x2[0]; x1[1] = x2[1]; x1[2] = x2[2];
            }
            newLines->InsertNextCell(npts, newPts);
        }
    }
    this->UpdateProgress(0.33);

    // Process the polys.
    if (!this->CheckAbort() && inPolys->GetNumberOfCells() > 0)
    {
        newPolys = vtkCellArray::New();
        newPolys->AllocateExact(inPolys->GetNumberOfCells(), inPolys->GetNumberOfConnectivityIds());

        vtkDebugMacro(<< "Starting Polys");
        checkAbortInterval = std::min(inPolys->GetNumberOfCells() / 10 + 1, (vtkIdType)1000);
        progressCounter = 0;
        for (inPolys->InitTraversal(); inPolys->GetNextCell(npts, pts); )
        {
            if (progressCounter % checkAbortInterval == 0 && this->CheckAbort())
            {
                break;
            }
            progressCounter++;
            inPts->GetPoint(pts[0], x1);
            newPts[0] = pts[0];
            for (i = 1; i < npts; i++)
            {
                inPts->GetPoint(pts[i], x2);
                if (x1[0] == x2[0] && x1[1] == x2[1] && x1[2] == x2[2])
                {
                    newPts[i] = newPts[i-1];
                }
                else
                {
                    newPts[i] = pts[i];
                }
                x1[0] = x2[0]; x1[1] = x2[1]; x1[2] = x2[2];
            }
            newPolys->InsertNextCell(npts, newPts);
        }
    }
    this->UpdateProgress(0.67);

    // Process the strips.
    if (!this->CheckAbort() && inStrips->GetNumberOfCells() > 0)
    {
        newStrips = vtkCellArray::New();
        newStrips->AllocateExact(inStrips->GetNumberOfCells(), inStrips->GetNumberOfConnectivityIds());

        vtkDebugMacro(<< "Starting Strips");
        checkAbortInterval = std::min(inStrips->GetNumberOfCells() / 10 + 1, (vtkIdType)1000);
        progressCounter = 0;
        for (inStrips->InitTraversal(); inStrips->GetNextCell(npts, pts); )
        {
            if (progressCounter % checkAbortInterval == 0 && this->CheckAbort())
            {
                break;
            }
            progressCounter++;
            inPts->GetPoint(pts[0], x1);
            newPts[0] = pts[0];
            for (i = 1; i < npts; i++)
            {
                inPts->GetPoint(pts[i], x2);
                if (x1[0] == x2[0] && x1[1] == x2[1] && x1[2] == x2[2])
                {
                    newPts[i] = newPts[i-1];
                }
                else
                {
                    newPts[i] = pts[i];
                }
                x1[0] = x2[0]; x1[1] = x2[1]; x1[2] = x2[2];
            }
            newStrips->InsertNextCell(npts, newPts);
        }
    }
    this->UpdateProgress(1.00);

    // Release memory.
    delete[] newPts;

    // Now transfer all the new Lines/Polys/Strips into the output.
    if (newLines)
    {
        output->SetLines(newLines);
        newLines->Delete();
    }
    if (newPolys)
    {
        output->SetPolys(newPolys);
        newPolys->Delete();
    }
    if (newStrips)
    {
        output->SetStrips(newStrips);
        newStrips->Delete();
    }

    return 1;
}

// ****************************************************************************
//  Method: vtkMergeConsecutivePolyDataPoints::FillInputPortInformation
//
//  Purpose:
//    Require the input data to be vtkPolyData.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Oct 15 15:23:20 PDT 2026
//
// ****************************************************************************

int
vtkMergeConsecutivePolyDataPoints::FillInputPortInformation(int, vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");

    return 1;
}

// ****************************************************************************
//  Method: vtkMergeConsecutivePolyDataPoints::PrintSelf
//
//  Purpose:
//    Print the information in this instance of the class.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Oct 15 15:23:20 PDT 2026
//
// ****************************************************************************

void
vtkMergeConsecutivePolyDataPoints::PrintSelf(ostream &os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
}
