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
