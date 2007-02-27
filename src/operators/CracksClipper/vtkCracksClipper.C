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

#include "vtkCracksClipper.h"
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkImplicitBoolean.h>
#include <vtkImplicitFunction.h>
#include <vtkImplicitFunctionCollection.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <vtkVisItUtility.h>


// ---------------------------------------------------------------------------
// An implicit funciton class to be used if a particular cell should
// not be clipped.
// ---------------------------------------------------------------------------

AlwaysNegative *AlwaysNegative::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("AlwaysNegative");
  if(ret)
    {
    return (AlwaysNegative*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new AlwaysNegative;
}

AlwaysNegative::AlwaysNegative()
{
}

AlwaysNegative::~AlwaysNegative()
{
}


vtkCxxRevisionMacro(vtkCracksClipper, "$Revision: 1.00 $");
vtkStandardNewMacro(vtkCracksClipper);


vtkCracksClipper::vtkCracksClipper()
{
  this->cf = vtkImplicitBoolean::New();
  SetInsideOut(true);
  useOppositePlane = false;
  this->emptyFunc = AlwaysNegative::New();

  this->CrackDir    = NULL;
  this->CrackWidth  = NULL;
  this->CellCenters = NULL;
}

vtkCracksClipper::~vtkCracksClipper()
{
  if (this->emptyFunc)
    {
    this->emptyFunc->Delete();
    this->emptyFunc = NULL;
    }
  if (this->cf)
    {
    this->cf->Delete();
    this->cf = NULL;
    }
  this->SetCrackDir(NULL);
  this->SetCrackWidth(NULL);
  this->SetCellCenters(NULL);
}

// ---------------------------------------------------------------------------
//  Method:  SetUpClipFunction 
//
//  Purpose:
//    Creates a clip function based on the crack width and direction
//    for the particular cell.
//
//  Arguments:
//    cellId    The id of the cell currently being clipped.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
//  Modifications:
//
// ---------------------------------------------------------------------------

void 
vtkCracksClipper::SetUpClipFunction(int cellId)
{
  vtkDataSet *input = GetInput();
  vtkCellData *inCD = input->GetCellData();
  vtkCell *cell = input->GetCell(cellId);

  if (this->CrackDir == NULL || 
      this->CrackWidth == NULL || 
      this->CellCenters == NULL)
    {
    EXCEPTION0(ImproperUseException);
    }
 
  vtkFloatArray *cdir = (vtkFloatArray*)inCD->GetArray(this->CrackDir);
  vtkFloatArray *centers = (vtkFloatArray*)inCD->GetArray(this->CellCenters);
  vtkFloatArray *cwidth = (vtkFloatArray*)inCD->GetArray(this->CrackWidth);
    
  if (cdir == NULL|| centers == NULL || cwidth == NULL)
    EXCEPTION0(ImproperUseException); 

  double *dir = cdir->GetTuple(cellId);
  double *center = centers->GetTuple(cellId);
  double crackWidth = cwidth->GetValue(cellId);

  if (crackWidth == 0)
    {
    SetClipFunction(this->emptyFunc);
    return;
    }

  this->cf->GetFunction()->RemoveAllItems();

  vtkPlane *plane = vtkPlane::New();

  double multiplier = 0.5;

  if (!useOppositePlane) 
    {
    plane->SetNormal(-dir[0], -dir[1], -dir[2]);
    }
  else 
    {
    multiplier *= -1;
    plane->SetNormal(dir);
    }

  double po[3], tmp1[3], tmp2[3];
  for (int i = 0; i < 3; i++)
    {
    po[i] = center[i] +  multiplier*crackWidth*dir[i]; 
    }      
  plane->SetOrigin(po);
  this->cf->AddFunction(plane);
  plane->Delete();

  SetClipFunction(this->cf);
}
