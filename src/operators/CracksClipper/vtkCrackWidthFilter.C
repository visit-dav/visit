// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkCrackWidthFilter.h"

#include <math.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMassProperties.h>
#include <vtkObjectFactory.h>
#include <vtkSlicer.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <vtkVisItUtility.h>

vtkStandardNewMacro(vtkCrackWidthFilter);

// ***************************************************************************
//  Method: vtkCrackWidthFilter constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 22, 2005
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep 13 07:45:06 PDT 2006
//    Added vtkCellIntersections.
//
//    Kathleen Bonnell, Fri Oct 13 11:05:01 PDT 2006 
//    Removed vtkCellIntersections, added vtkMassProperties, vtkSlicer.
//
//    Kathleen Biagas, Tue Aug 14 13:05:17 MST 2012 
//    Removed unused vtkQuad and vtkTriangle.
//
// ***************************************************************************

vtkCrackWidthFilter::vtkCrackWidthFilter()
{
  this->MaxCrack1Width = 0.;
  this->MaxCrack2Width = 0.;
  this->MaxCrack3Width = 0.;
  this->Crack1Var = NULL;
  this->Crack2Var = NULL;
  this->Crack3Var = NULL;
  this->StrainVar = NULL;
  this->Slicer = vtkSlicer::New();
  this->MassProp = vtkMassProperties::New();
}

// ***************************************************************************
//  Method: vtkCrackWidthFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 22, 2005
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep 13 07:45:06 PDT 2006
//    Added vtkCellIntersections.
//
//    Kathleen Bonnell, Fri Oct 13 11:05:01 PDT 2006 
//    Removed vtkCellIntersections, added vtkMassProperties, vtkSlicer.
//
//    Kathleen Biagas, Tue Aug 14 13:05:17 MST 2012 
//    Removed unused vtkQuad and vtkTriangle.
//
// ***************************************************************************

vtkCrackWidthFilter::~vtkCrackWidthFilter()
{
  this->SetCrack1Var(NULL);
  this->SetCrack2Var(NULL);
  this->SetCrack3Var(NULL);
  this->SetStrainVar(NULL);
  if (this->Slicer)
    {
    this->Slicer->Delete();
    this->Slicer = NULL;
    }
  if (this->MassProp)
    {
    this->MassProp->Delete();
    this->MassProp = NULL;
    }
}

// ***************************************************************************
//  Method: vtkCrackWidthFilter_OrderThem 
//
//  Purpose: Creates a max-to-min ordering based on the passed deltas. 
//  
//  Notes:  This code is duplicated in avtCrackWidthExpression.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 13, 2006 
//
//  Modifications:
//
// ***************************************************************************

void 
vtkCrackWidthFilter_OrderThem(double delta1, double delta2, double delta3, 
    int co[3])
{
  int min, mid, max;
  if (delta1 <= delta2 && delta1 <= delta3)
    min = 0; 
  else if (delta2 <= delta1 && delta2 <= delta3)
    min = 1; 
  else 
    min = 2; 

  if (delta1 >= delta2 && delta1 >= delta3)
    max = 0; 
  else if (delta2 >= delta1 && delta2 >= delta3)
    max = 1; 
  else 
    max = 2; 

  if (min == 0)
    mid = (max == 1 ? 2 : 1);    
  else if (min == 1)
    mid = (max == 2 ? 0 : 2);    
  else 
    mid = (max == 0 ? 1 : 0);    

  co[0] = max;
  co[1] = mid;
  co[2] = min;
}

// ***************************************************************************
//  Method: vtkCrackWidthFilter::RequestData
//
//  Purpose: Executes this filter.
//  
//  Notes:  This code is duplicated in avtCrackWidthExpression.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 22, 2005
//
//  Modifications:
//    Modified to determine crack width in max-to-min order based on
//    the value of the StrainVar for each crack dir.  Do ALL crack widths.
//
//    Kathleen Biagas, Tue Aug 14 1:07:23 MST 2012
//    Support double-precision.
//
//    Eric Brugger, Wed Jan  9 16:25:38 PST 2013
//    Modified to inherit from vtkDataSetAlgorithm.
//
//    Eric Brugger, Thu Sep  6 16:40:01 PDT 2018
//    Modified the filter to calculate the proper crack width for the third
//    crack direction.
//
// ***************************************************************************

int
vtkCrackWidthFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  //
  // Initialize some frequently used values.
  //
  vtkDataSet  *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (this->StrainVar == NULL)
    EXCEPTION0(ImproperUseException); 

  if (this->Crack1Var == NULL &&
      this->Crack2Var == NULL &&
      this->Crack3Var == NULL)
    EXCEPTION0(ImproperUseException); 

  vtkCellData *inCD = input->GetCellData();
  this->Slicer->SetInputData(input);

  vtkDataArray *cd1 = NULL;
  if (this->Crack1Var != NULL)
    cd1 = inCD->GetArray(this->Crack1Var);
  vtkDataArray *cd2 = NULL;
  if (this->Crack2Var != NULL)
    cd2 = inCD->GetArray(this->Crack2Var);
  vtkDataArray *cd3 = NULL;
  if (this->Crack3Var != NULL)
    cd3 = inCD->GetArray(this->Crack3Var);
  vtkDataArray *strain = inCD->GetArray(this->StrainVar);

  if (strain == NULL)
    EXCEPTION0(ImproperUseException); 
  if (cd1 == NULL || cd2 == NULL || cd3 == NULL)
    EXCEPTION0(ImproperUseException); 

  output->DeepCopy(input);

  vtkIdType numCells = input->GetNumberOfCells();

  // Prepare the arrays
  vtkDoubleArray *crack1Width = vtkDoubleArray::New();
  crack1Width->SetName("avtCrack1Width");
  crack1Width->SetNumberOfComponents(1);
  crack1Width->SetNumberOfTuples(numCells);

  vtkDoubleArray *crack2Width = vtkDoubleArray::New();
  crack2Width->SetName("avtCrack2Width");
  crack2Width->SetNumberOfComponents(1);
  crack2Width->SetNumberOfTuples(numCells);
 
  vtkDoubleArray *crack3Width = vtkDoubleArray::New();
  crack3Width->SetName("avtCrack3Width");
  crack3Width->SetNumberOfComponents(1);
  crack3Width->SetNumberOfTuples(numCells);

  vtkDoubleArray *cellCenters = vtkDoubleArray::New();
  cellCenters->SetName("avtCellCenters");
  cellCenters->SetNumberOfComponents(3);
  cellCenters->SetNumberOfTuples(numCells);

  vtkDataArray *vol = input->GetCellData()->GetArray("cracks_vol");

  //
  // formula for calculating crack width:
  //    crackwidth = L * (1 - (exp(-delta))  
  //    where: 
  //      L = ZoneVol / (Area perpendicular to crackdir)
  //        find Area by slicing the cell by plane with origin == cell center
  //        and Normal == crackdir.  Take area of that slice.
  //
  //      delta = T11 for crack dir1 = component 0 of strain_tensor 
  //              T22 for crack dir2 = component 4 of strain_tensor
  //              T33 for crack dir3 = component 8 of strain_tensor
  //

  // 
  // step through cells, calculating cell centers and crack widths for 
  // each crack direction.  Terminate early when possible.
  // 
  vtkDoubleArray *crackWidth = NULL;
  double delta = 0, L, cw, zVol, *dir = NULL, *maxCW = NULL;
  int crackOrder[3]; 

  for (vtkIdType cellId = 0; cellId < numCells; cellId++)
    {
    double center[3] = {VTK_FLOAT_MAX, VTK_FLOAT_MAX, VTK_FLOAT_MAX};
    double delta1 = strain->GetComponent(cellId, 0);
    double delta2 = strain->GetComponent(cellId, 4);
    double delta3 = strain->GetComponent(cellId, 8);

    if (delta1 == 0 && delta2 == 0 && delta3 == 0)
      {
      cellCenters->SetTuple(cellId, center); 
      crack1Width->SetValue(cellId, 0);
      crack2Width->SetValue(cellId, 0);
      crack3Width->SetValue(cellId, 0);
      continue;
      }

    vtkCrackWidthFilter_OrderThem(delta1, delta2, delta3, crackOrder);

    vtkCell *cell = input->GetCell(cellId);
    vtkVisItUtility::GetCellCenter(cell, center);
    cellCenters->SetTuple(cellId, center);

    if (vol)
      zVol = vol->GetComponent(cellId, 0);
    else
      EXCEPTION0(ImproperUseException); 

    double L1L2 = 1.; 
    for (int crack = 0; crack < 3; crack++)
      {
      switch(crackOrder[crack])
        {
        case 0: crackWidth = crack1Width;
                dir = cd1->GetTuple(cellId);
                delta = delta1;
                maxCW = &this->MaxCrack1Width;
                break; 
        case 1: crackWidth = crack2Width;
                dir = cd2->GetTuple(cellId);
                delta = delta2;
                maxCW = &this->MaxCrack2Width;
                break; 
        case 2: crackWidth = crack3Width;
                dir = cd3->GetTuple(cellId);
                delta = delta3;
                maxCW = &this->MaxCrack3Width;
                break; 
        }
        if (delta == 0 || (dir[0] == 0 && dir[1] == 0 && dir[2] == 0))
          {
          crackWidth->SetValue(cellId, 0);
          continue;
          }

        if (crack < 2)
          {
          L = LengthForCell(cell, cellId, center, dir, zVol, 0);
          L1L2 *= L;
          }
        else 
          {
          L = LengthForCell(cell, cellId, center, dir, zVol, L1L2);
          }
        cw = L*(1.0-exp(-delta));
        crackWidth->SetValue(cellId, cw);
        if (cw > *maxCW)
          *maxCW = cw;
      }
    }

  output->GetCellData()->AddArray(cellCenters);
  output->GetCellData()->CopyFieldOn("avtCellCenters");
  cellCenters->Delete();

  output->GetCellData()->AddArray(crack1Width);
  output->GetCellData()->CopyFieldOn("avtCrack1Width");
  crack1Width->Delete();

  output->GetCellData()->AddArray(crack2Width);
  output->GetCellData()->CopyFieldOn("avtCrack2Width");
  crack2Width->Delete();
  
  output->GetCellData()->AddArray(crack3Width);
  output->GetCellData()->CopyFieldOn("avtCrack3Width");
  crack3Width->Delete();

  return 1;
}

// ***************************************************************************
//  Method: vtkCrackWidthFilter::LengthForCell
//
//  Purpose: Determines the effective length in the crack direction for a
//           given cell
//  
//  Notes:  This code is duplicated in avtCrackWidthExpression.
//
//  Arguments:
//    cell      The cell.
//    cellId    The id of the cell.
//    center    The coordinates of the cell center.
//    dir       A vector representing the direction of the crack. 
//    zVol      The volume of this cell.
//    L1L2      Zero for the first 2 cracks. L1 * L2 for the third crack,
//              where L1 is effective length in the crack direction for the
//              first crack and L2 is the same for the second crack.
//
//  Returns:    The width of the crack.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 22, 2005
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep 13 07:52:03 PDT 2006
//    Use vtkCellIntersetions.
//
//    Kathleen Bonnell,  Fri Oct 13 11:05:01 PDT 2006
//    Removed use of vtkCellIntersetions. Use area of plane that slices
//    the cell.
//
//    Eric Brugger, Thu Sep  6 16:40:01 PDT 2018
//    I modified the function to return the effective length in the crack
//    direction instead of the crack width and changed the name to match.
//
//    Kathleen Biagas, Wed June 15, 2022
//    Guard against divide-by-zero.
//
// ***************************************************************************

double
vtkCrackWidthFilter::LengthForCell(vtkCell *cell, vtkIdType cellId, 
  const double *center, const double *dir, const double zVol,
  const double L1L2)
{
  double L = 0;
  if (L1L2 == 0)
    {
    this->Slicer->SetCellList(&cellId, 1);
    this->Slicer->SetNormal(const_cast<double*>(dir));
    this->Slicer->SetOrigin(const_cast<double*>(center));
    this->MassProp->SetInputConnection(this->Slicer->GetOutputPort());
    this->MassProp->Update();
    double sa = this->MassProp->GetSurfaceArea();
    if (sa != 0.)
        L =  zVol / sa;
    }
  else
    {
    L = zVol / L1L2;
    }
  return L;
}

// ***************************************************************************
//  Method: vtkCrackWidthFilter::GetMaxCrackWidth
//
//  Purpose: Returns the maximum calculated crack width for a specified
//           crack direction.
//  
//  Arguments:
//    whichCrack The specified crack direction.
//
//  Returns:    The maximum crack width.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 22, 2005
//
//  Modifications:
//
// ***************************************************************************

double
vtkCrackWidthFilter::GetMaxCrackWidth(int whichCrack)
{
    switch(whichCrack)
    {
        case 0: return this->MaxCrack1Width;
        case 1: return this->MaxCrack2Width;
        case 2: return this->MaxCrack3Width;
        default: return this->MaxCrack1Width;
    }
}
