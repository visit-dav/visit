// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            vtkAxisDepthSort.C                             //
// ************************************************************************* //

#include <vtkAxisDepthSort.h>

#include <visit-config.h> // For LIB_VERSION_LE
#include <stdlib.h>

#include <vtkCellData.h>
#include <vtkExecutive.h>
#include <vtkGenericCell.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

static int CoordSorter(const void *, const void *);

typedef struct 
{
    float    coord;
    int      cell_id;
} coord_cell_id_pair;

vtkStandardNewMacro(vtkAxisDepthSort);

// ****************************************************************************
//  Modifications:
//    Hank Childs, Fri Mar 11 08:26:34 PST 2005
//    Fix memory leak.
//
//    Eric Brugger, Wed Jan  9 11:20:53 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

vtkAxisDepthSort::vtkAxisDepthSort()
{
    SetNumberOfOutputPorts(6);
    // Base class does 0.

    vtkPolyData *p = vtkPolyData::New();
    this->GetExecutive()->SetOutputData(1, p);
    p->Delete();

    p = vtkPolyData::New();
    this->GetExecutive()->SetOutputData(2, p);
    p->Delete();

    p = vtkPolyData::New();
    this->GetExecutive()->SetOutputData(3, p);
    p->Delete();

    p = vtkPolyData::New();
    this->GetExecutive()->SetOutputData(4, p);
    p->Delete();

    p = vtkPolyData::New();
    this->GetExecutive()->SetOutputData(5, p);
    p->Delete();
}


vtkPolyData *
vtkAxisDepthSort::GetPlusXOutput(void)
{
    return vtkPolyData::SafeDownCast(
        this->GetExecutive()->GetOutputData(0));
}


vtkPolyData *
vtkAxisDepthSort::GetMinusXOutput(void)
{
    return vtkPolyData::SafeDownCast(
        this->GetExecutive()->GetOutputData(1));
}


vtkPolyData *
vtkAxisDepthSort::GetPlusYOutput(void)
{
    return vtkPolyData::SafeDownCast(
        this->GetExecutive()->GetOutputData(2));
}


vtkPolyData *
vtkAxisDepthSort::GetMinusYOutput(void)
{
    return vtkPolyData::SafeDownCast(
        this->GetExecutive()->GetOutputData(3));
}


vtkPolyData *
vtkAxisDepthSort::GetPlusZOutput(void)
{
    return vtkPolyData::SafeDownCast(
        this->GetExecutive()->GetOutputData(4));
}


vtkPolyData *
vtkAxisDepthSort::GetMinusZOutput(void)
{
    return vtkPolyData::SafeDownCast(
        this->GetExecutive()->GetOutputData(5));
}


// ****************************************************************************
//  Modifications:
//    Eric Brugger, Wed Jan  9 11:20:53 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ****************************************************************************

int
vtkAxisDepthSort::RequestData(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);

    //
    // Initialize some frequently used values.
    //
    vtkPolyData *input = vtkPolyData::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));

    int ncells = input->GetNumberOfCells();

    coord_cell_id_pair *pairs = new coord_cell_id_pair[ncells];

    //
    // Create some automatic variables that we will use when generating the
    // outputs.
    //
    vtkPointData *inPD       = input->GetPointData();
    vtkCellData  *inCD       = input->GetCellData();
    int           doCellData = (inCD->GetNumberOfArrays() > 0 ? true : false);
    vtkPointData *outPD      = NULL;
    vtkCellData  *outCD      = NULL;
    vtkIdType     npts       = 0;
#if LIB_VERSION_LE(VTK, 8,1,0)
    vtkIdType    *cellPts    = nullptr;
#else
    const vtkIdType *cellPts = nullptr;
#endif
    input->BuildCells();
 
    //
    // Start off by getting the center of the cell for each cell.
    //
    float *loc = new float[ncells*3];
    double bnds[6];
    for (int i = 0 ; i < ncells ; i++)
    {
        input->GetCellBounds(i, bnds);
        loc[3*i]   = (bnds[0]+bnds[1])/2.;
        loc[3*i+1] = (bnds[2]+bnds[3])/2.;
        loc[3*i+2] = (bnds[4]+bnds[5])/2.;
    }

    //
    // Create an array to sort in X and use qsort to sort it.
    //
    for (int i = 0 ; i < ncells ; i++)
    {
        pairs[i].coord   = loc[3*i];
        pairs[i].cell_id = i;
    }
    qsort(pairs, ncells, sizeof(coord_cell_id_pair), CoordSorter);

    //
    // Now create the PlusX/MinusX output.
    //
    vtkPolyData *minusX = GetMinusXOutput();
    minusX->SetPoints(input->GetPoints());
    minusX->Allocate(input);
    outPD = minusX->GetPointData();
    outCD = minusX->GetCellData();
    outPD->PassData(inPD);
    if (doCellData)
        outCD->CopyAllocate(inCD);
    for (int i = 0 ; i < ncells ; i++)
    {
        int cell = pairs[i].cell_id;
        input->GetCellPoints(cell, npts, cellPts);
        minusX->InsertNextCell(input->GetCellType(cell), npts, cellPts);
        if (doCellData)
            outCD->CopyData(inCD, i, cell);
    }

    vtkPolyData *plusX = GetPlusXOutput();
    plusX->SetPoints(input->GetPoints());
    plusX->Allocate(input);
    outPD = plusX->GetPointData();
    outCD = plusX->GetCellData();
    outPD->PassData(inPD);
    if (doCellData)
        outCD->CopyAllocate(inCD);
    for (int i = ncells-1 ; i >= 0 ; i--)
    {
        int cell = pairs[i].cell_id;
        input->GetCellPoints(cell, npts, cellPts);
        plusX->InsertNextCell(input->GetCellType(cell), npts, cellPts);
        if (doCellData)
            outCD->CopyData(inCD, i, cell);
    }

    //
    // Create an array to sort in Y and use qsort to sort it.
    //
    for (int i = 0 ; i < ncells ; i++)
    {
        pairs[i].coord   = loc[3*i+1];
        pairs[i].cell_id = i;
    }
    qsort(pairs, ncells, sizeof(coord_cell_id_pair), CoordSorter);

    //
    // Now create the PlusY/MinusY output.
    //
    vtkPolyData *minusY = GetMinusYOutput();
    minusY->SetPoints(input->GetPoints());
    minusY->Allocate(input);
    outPD = minusY->GetPointData();
    outCD = minusY->GetCellData();
    outPD->PassData(inPD);
    if (doCellData)
        outCD->CopyAllocate(inCD);
    for (int i = 0 ; i < ncells ; i++)
    {
        int cell = pairs[i].cell_id;
        input->GetCellPoints(cell, npts, cellPts);
        minusY->InsertNextCell(input->GetCellType(cell), npts, cellPts);
        if (doCellData)
            outCD->CopyData(inCD, i, cell);
    }

    vtkPolyData *plusY = GetPlusYOutput();
    plusY->SetPoints(input->GetPoints());
    plusY->Allocate(input);
    outPD = plusY->GetPointData();
    outCD = plusY->GetCellData();
    outPD->PassData(inPD);
    if (doCellData)
        outCD->CopyAllocate(inCD);
    for (int i = ncells-1 ; i >= 0 ; i--)
    {
        int cell = pairs[i].cell_id;
        input->GetCellPoints(cell, npts, cellPts);
        plusY->InsertNextCell(input->GetCellType(cell), npts, cellPts);
        if (doCellData)
            outCD->CopyData(inCD, i, cell);
    }

    //
    // Create an array to sort in Z and use qsort to sort it.
    //
    for (int i = 0 ; i < ncells ; i++)
    {
        pairs[i].coord   = loc[3*i+2];
        pairs[i].cell_id = i;
    }
    qsort(pairs, ncells, sizeof(coord_cell_id_pair), CoordSorter);

    //
    // Now create the PlusZ/MinusZ output.
    //
    vtkPolyData *minusZ = GetMinusZOutput();
    minusZ->SetPoints(input->GetPoints());
    minusZ->Allocate(input);
    outPD = minusZ->GetPointData();
    outCD = minusZ->GetCellData();
    outPD->PassData(inPD);
    if (doCellData)
        outCD->CopyAllocate(inCD);
    for (int i = 0 ; i < ncells ; i++)
    {
        int cell = pairs[i].cell_id;
        input->GetCellPoints(cell, npts, cellPts);
        minusZ->InsertNextCell(input->GetCellType(cell), npts, cellPts);
        if (doCellData)
            outCD->CopyData(inCD, i, cell);
    }

    vtkPolyData *plusZ = GetPlusZOutput();
    plusZ->SetPoints(input->GetPoints());
    plusZ->Allocate(input);
    outPD = plusZ->GetPointData();
    outCD = plusZ->GetCellData();
    outPD->PassData(inPD);
    if (doCellData)
        outCD->CopyAllocate(inCD);
    for (int i = ncells-1 ; i >= 0 ; i--)
    {
        int cell = pairs[i].cell_id;
        input->GetCellPoints(cell, npts, cellPts);
        plusZ->InsertNextCell(input->GetCellType(cell), npts, cellPts);
        if (doCellData)
            outCD->CopyData(inCD, i, cell);
    }

    //
    // Clean up memory.
    //
    delete [] loc;
    delete [] pairs;

    return 1;
}


int
CoordSorter(const void *arg1, const void *arg2)
{
    const coord_cell_id_pair *c1 = (const coord_cell_id_pair *) arg1;
    const coord_cell_id_pair *c2 = (const coord_cell_id_pair *) arg2;

    if (c1[0].coord > c2[0].coord)
    {
        return 1;
    }
    else if (c1[0].coord < c2[0].coord)
    {
        return -1;
    }

    return 0;
}
