/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#include "vtkCracksClipper.h"
#include <vtkAppendFilter.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkImplicitBoolean.h>
#include <vtkImplicitFunction.h>
#include <vtkImplicitFunctionCollection.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>

#include <ImproperUseException.h>


// ---------------------------------------------------------------------------
// An implicit function class to be used if a particular cell should
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


vtkCxxRevisionMacro(vtkCracksClipper, "$Revision: 2.00 $");
vtkStandardNewMacro(vtkCracksClipper);


// ****************************************************************************
//  Constructor:  vtkCracksClipper::vtkCracksClipper
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Aug 29 13:38:08 EDT 2006
//    Added support for leaving cells whole.
//
//    Hank Childs, Sat Sep 29 11:14:58 PDT 2007
//    Initialize new data members.
//
//    Kathleen Biagas, Tue Aug 14 15:24:07 MST 2012 
//    Remove members that are declared in parent class.
//    Set up a preliminary empty clip function, tell parent to not precompute
//    the clip.
//
// ****************************************************************************

vtkCracksClipper::vtkCracksClipper()
{
  this->cf = vtkImplicitBoolean::New();
  this->useOppositePlane = false;
  this->emptyFunc = AlwaysNegative::New();

  this->CrackDir    = NULL;
  this->CrackWidth  = NULL;
  this->CellCenters = NULL;

  SetClipFunction(this->emptyFunc);
  SetPrecomputeClipScalars(false);
}

// ****************************************************************************
//  Destructor:  vtkCracksClipper::~vtkCracksClipper
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Sep 29 11:14:58 PDT 2007
//    Clean up new data members.
//
//    Kathleen Biagas, Tue Aug 14 15:24:07 MST 2012 
//    Remove members that are declared in parent class.
//
// ****************************************************************************

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
//  Method:  ModifyClip 
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
//    Kathleen Bonnell, Tue Jul 1 15:08:17 PDT 2008
//    Removed unreferenced variables.
//
//    Kathleen Biagas, Tue Aug 14 15:24:07 MST 2012
//    Renamed.  Support double-precision.
//
// ---------------------------------------------------------------------------

void 
vtkCracksClipper::ModifyClip(vtkIdType cellId)
{
  vtkDataSet *input = GetInput();
  vtkCellData *inCD = input->GetCellData();

  if (this->CrackDir == NULL || 
      this->CrackWidth == NULL || 
      this->CellCenters == NULL)
    {
    EXCEPTION0(ImproperUseException);
    }
 
  vtkDataArray *cdir    = inCD->GetArray(this->CrackDir);
  vtkDataArray *centers = inCD->GetArray(this->CellCenters);
  vtkDataArray *cwidth  = inCD->GetArray(this->CrackWidth);
    
  if (cdir == NULL|| centers == NULL || cwidth == NULL)
    EXCEPTION0(ImproperUseException); 

  double *dir = cdir->GetTuple(cellId);
  double *center = centers->GetTuple(cellId);
  double crackWidth = cwidth->GetComponent(cellId, 0);

  if (crackWidth == 0)
    {
    SetClipFunction(this->emptyFunc);
    return;
    }

  this->cf->GetFunction()->RemoveAllItems();

  vtkPlane *plane = vtkPlane::New();

  double multiplier = 0.5;

  if (!this->useOppositePlane) 
    {
    plane->SetNormal(-dir[0], -dir[1], -dir[2]);
    }
  else 
    {
    multiplier *= -1;
    plane->SetNormal(dir);
    }

  double po[3];
  for (int i = 0; i < 3; i++)
    {
    po[i] = center[i] +  multiplier*crackWidth*dir[i]; 
    }      
  plane->SetOrigin(po);
  this->cf->AddFunction(plane);
  plane->Delete();

  SetClipFunction(this->cf);
}

void 
vtkCracksClipper::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  if (this->CrackDir)
      os << "crack direction var : " << this->CrackDir << endl;
  if (this->CrackWidth)
      os << "crack width var: " << this->CrackDir << endl;
  if (this->CellCenters)
      os << "cell centers var: " << this->CrackDir << endl;
  if (this->useOppositePlane)
      os << " using opposite plane" << endl;
  else 
      os << " not using opposite plane" << endl;
      os << " using opposite plane" << endl;
}

