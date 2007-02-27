/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "vtkCrackWidthFilter.h"
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkMassProperties.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkQuad.h>
#include <vtkSlicer.h>
#include <vtkTriangle.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <vtkVisItUtility.h>

#include <math.h>


#ifdef SUNOS
#include <ieeefp.h> // for 'finite'
#endif


vtkCxxRevisionMacro(vtkCrackWidthFilter, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkCrackWidthFilter);

// ***************************************************************************
//  Method:  vtkCrackWidthFilter constructor
//
//  Programmer:  Kathleen Bonnell
//  Creation:    August 22, 2005
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep 13 07:45:06 PDT 2006
//    Added vtkCellIntersections.
//
//    Kathleen Bonnell, Fri Oct 13 11:05:01 PDT 2006 
//    Removed vtkCellIntersections, added vtkMassProperties, vtkSlicer.
//
// ***************************************************************************

vtkCrackWidthFilter::vtkCrackWidthFilter()
{
  this->triangle = vtkTriangle::New();
  this->quad = vtkQuad::New();
  this->MaxCrack1Width = 0.f;
  this->MaxCrack2Width = 0.f;
  this->MaxCrack3Width = 0.f;
  this->Crack1Var = NULL;
  this->Crack2Var = NULL;
  this->Crack3Var = NULL;
  this->StrainVar = NULL;
  this->Slicer = vtkSlicer::New();
  this->MassProp = vtkMassProperties::New();
}


// ***************************************************************************
//  Method:  vtkCrackWidthFilter destructor
//
//  Programmer:  Kathleen Bonnell
//  Creation:    August 22, 2005
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep 13 07:45:06 PDT 2006
//    Added vtkCellIntersections.
//
//    Kathleen Bonnell, Fri Oct 13 11:05:01 PDT 2006 
//    Removed vtkCellIntersections, added vtkMassProperties, vtkSlicer.
//
// ***************************************************************************

vtkCrackWidthFilter::~vtkCrackWidthFilter()
{
  if (this->triangle)
    {
    this->triangle->Delete();
    this->triangle = NULL;
    }
  if (this->quad)
    {
    this->quad->Delete();
    this->quad = NULL;
    }
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
//  Method:  OrderThem 
//
//  Purpose:  Creates a max-to-min ordering based on the passed deltas. 
//
//  Programmer:  Kathleen Bonnell
//  Creation:    October 13, 2006 
//
//  Modifications:
//
// ***************************************************************************

void OrderThem(double delta1, double delta2, double delta3, int co[3])
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
//  Method:  vtkCrackWidthFilter::Execute
//
//  Purpose:  Executes this filter.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    August 22, 2005
//
//  Modifications:
//    Modified to determine crack width in max-to-min order based on
//    the value of the StrainVar for each crack dir.  Do ALL crack widths.
//
// ***************************************************************************

void
vtkCrackWidthFilter::Execute()
{
  if (this->StrainVar == NULL)
    EXCEPTION0(ImproperUseException); 

  if (this->Crack1Var == NULL &&
      this->Crack2Var == NULL &&
      this->Crack3Var == NULL)
    EXCEPTION0(ImproperUseException); 

  
  vtkDataSet *input = GetInput();
  vtkCellData *inCD = input->GetCellData();
  this->Slicer->SetInput(input);

  vtkFloatArray *cd1 = NULL;
  if (this->Crack1Var != NULL)
    cd1 = (vtkFloatArray*)inCD->GetArray(this->Crack1Var);
  vtkFloatArray *cd2 = NULL;
  if (this->Crack2Var != NULL)
    cd2 = (vtkFloatArray*)inCD->GetArray(this->Crack2Var);
  vtkFloatArray *cd3 = NULL;
  if (this->Crack3Var != NULL)
    cd3 = (vtkFloatArray*)inCD->GetArray(this->Crack3Var);
  vtkFloatArray *strain = (vtkFloatArray*)inCD->GetArray(this->StrainVar);

  if (strain == NULL)
    EXCEPTION0(ImproperUseException); 
  if (cd1 == NULL || cd2 == NULL || cd3 == NULL)
    EXCEPTION0(ImproperUseException); 

  vtkDataSet *output = GetOutput();
  output->DeepCopy(input);

  int numCells = input->GetNumberOfCells();

  // Prepare the arrays
  vtkFloatArray *crack1Width = vtkFloatArray::New();
  crack1Width->SetName("avtCrack1Width");
  crack1Width->SetNumberOfComponents(1);
  crack1Width->SetNumberOfTuples(numCells);

  vtkFloatArray *crack2Width = vtkFloatArray::New();
  crack2Width->SetName("avtCrack2Width");
  crack2Width->SetNumberOfComponents(1);
  crack2Width->SetNumberOfTuples(numCells);
 
  vtkFloatArray *crack3Width = vtkFloatArray::New();
  crack3Width->SetName("avtCrack3Width");
  crack3Width->SetNumberOfComponents(1);
  crack3Width->SetNumberOfTuples(numCells);

  vtkFloatArray *cellCenters = vtkFloatArray::New();
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
  vtkFloatArray *crackWidth;
  double delta, cw, zVol, *dir, *maxCW;
  int crackOrder[3]; 

  for (int cellId = 0; cellId < numCells; cellId++)
    {
    double center[3] = {VTK_LARGE_FLOAT, VTK_LARGE_FLOAT, VTK_LARGE_FLOAT};
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

    OrderThem(delta1, delta2, delta3, crackOrder);

    vtkCell *cell = input->GetCell(cellId);
    vtkVisItUtility::GetCellCenter(cell, center);
    cellCenters->SetTuple(cellId, center);

    if (vol)
      zVol = vol->GetComponent(cellId, 0);
    else
      EXCEPTION0(ImproperUseException); 

    double cwsum = 0.; 
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
          cw = CrackWidthForCell(cell, cellId, center, delta, dir, zVol, 0);
          cwsum += cw;
          }
        else 
          {
          cw = CrackWidthForCell(cell, cellId, center, delta, dir, zVol,cwsum);
          }
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
}


// ***************************************************************************
//  Method:  vtkCrackWidthFilter::CrackWidthForCell
//
//  Purpose:  Determines the crack width for a given cell
//  
//  Arguments:
//    cell       The cell.
//    cellId     The id of the cell.
//    center     The coordinates of the cell center.
//    delta      A component of strain_tensor 
//    dir        A vector representing the diretion of the crack. 
//    zoneVol    A volume of this cell.
//
//  Returns:     The width of the crack.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    August 22, 2005
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep 13 07:52:03 PDT 2006
//    Use vtkCellIntersetions.
//
//    Kathleen Bonnell,  Fri Oct 13 11:05:01 PDT 2006
//    Removed use of vtkCellIntersetions. Use area of plane that slices
//    the cell.
//
// ***************************************************************************

double
vtkCrackWidthFilter::CrackWidthForCell(vtkCell *cell, int cellId, 
  const double *center, const double delta, const double *dir, 
  const double zVol, const double L1L2)
{
  double L = L1L2;
  if (L1L2 == 0)
    {
    this->Slicer->SetCellList(&cellId, 1);
    this->Slicer->SetNormal(const_cast<double*>(dir));
    this->Slicer->SetOrigin(const_cast<double*>(center));
    this->MassProp->SetInput(this->Slicer->GetOutput());
    this->MassProp->Update();
    L =  zVol / this->MassProp->GetSurfaceArea();
  }
  return L*(1.0-exp(-delta));
}


// ***************************************************************************
//  Method:  vtkCrackWidthFilter::GetMaxCrackWidth
//
//  Purpose:  Returns the maximum calculated crack width for a specified
//            crack direction.
//  
//  Arguments:
//    whichCrack The specified crack direction.
//
//  Returns:     The maximum crack width.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    August 22, 2005
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

