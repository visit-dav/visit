// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkOpenGLPointMapper.h"

#include <vtkOpenGLHelper.h>

#include <vtkObjectFactory.h>
#include <vtkFloatArray.h>
#include <vtkOpenGLActor.h>
#include <vtkOpenGLCamera.h>
#include <vtkOpenGLIndexBufferObject.h>
#include <vtkOpenGLSphereMapper.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLVertexArrayObject.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkUnsignedCharArray.h>


// A helper class to draw points as sphere imposters
class vtkImposterHelper : public vtkOpenGLSphereMapper
{
public:
  static vtkImposterHelper* New();
  vtkTypeMacro(vtkImposterHelper, vtkOpenGLSphereMapper)

  vtkUnsignedCharArray *MapScalars(double alpha) override;

protected:
  vtkImposterHelper();
  ~vtkImposterHelper() override;


private:
  vtkImposterHelper(const vtkImposterHelper&) = delete;
  void operator=(const vtkImposterHelper&) = delete;

};

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkImposterHelper)

//-----------------------------------------------------------------------------
vtkImposterHelper::vtkImposterHelper()
{
}

//-----------------------------------------------------------------------------
vtkImposterHelper::~vtkImposterHelper()
{
}


//-----------------------------------------------------------------------------
vtkUnsignedCharArray *
vtkImposterHelper::MapScalars(double alpha)
{
  vtkPolyData * input = this->GetInput();

  if (this->ScalarVisibility)
    {
    vtkDataArray *colorsArray = input->GetPointData()->GetArray("Colors");

    if (colorsArray != NULL)
      {
      if ( this->Colors )
        {
        // Get rid of old colors
        this->Colors->UnRegister(this);
        }
      this->Colors = vtkUnsignedCharArray::SafeDownCast(colorsArray);
      this->Colors->Register(this);
      this->Colors->Delete();
      }
    else
      {
      this->Superclass::MapScalars(alpha);
      }
    }
  else
    {
    this->Superclass::MapScalars(alpha);
    }
  return this->Colors;
}


// A helper class to draw gl points directly
class vtkPointHelper : public vtkOpenGLPolyDataMapper
{
public:
  static vtkPointHelper* New();
  vtkTypeMacro(vtkPointHelper, vtkOpenGLPolyDataMapper)

protected:
  vtkPointHelper();
  ~vtkPointHelper() override;

private:
  vtkPointHelper(const vtkPointHelper&)=delete;
  void operator=(const vtkPointHelper&)=delete;
};

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPointHelper)

//-----------------------------------------------------------------------------
vtkPointHelper::vtkPointHelper()
{
}

//-----------------------------------------------------------------------------
vtkPointHelper::~vtkPointHelper()
{
}



//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkOpenGLPointMapper)

//-----------------------------------------------------------------------------
vtkOpenGLPointMapper::vtkOpenGLPointMapper()
{
  this->IHelper = vtkImposterHelper::New();
  this->PHelper = vtkPointHelper::New();
}

vtkOpenGLPointMapper::~vtkOpenGLPointMapper()
{
  this->IHelper->Delete();
  this->IHelper = 0;
  this->PHelper->Delete();
  this->PHelper = 0;
}

//----------------------------------------------------------------------------
void vtkOpenGLPointMapper::RenderPiece(vtkRenderer *ren, vtkActor *act)
{
  if (this->UseImposters)
    {
    if (this->GetMTime() > this->IHelperUpdateTime)
      {
      this->IHelper->vtkPolyDataMapper::ShallowCopy(this);
      this->IHelper->SetScaleArray(this->ImposterScaleArray);
      this->IHelper->SetRadius(this->ImposterRadius);
      this->IHelper->Modified();
      this->IHelperUpdateTime.Modified();
      }
    this->IHelper->RenderPiece(ren,act);
    }
  else
    {
    if (this->GetMTime() > this->PHelperUpdateTime)
      {
      this->PHelper->vtkPolyDataMapper::ShallowCopy(this);
      this->PHelper->Modified();
      this->PHelperUpdateTime.Modified();
      }
    this->PHelper->RenderPiece(ren,act);
    }
}

//-----------------------------------------------------------------------------
void vtkOpenGLPointMapper::ReleaseGraphicsResources(vtkWindow* win)
{
  this->IHelper->ReleaseGraphicsResources(win);
  this->IHelper->SetInputData(0);
  this->PHelper->ReleaseGraphicsResources(win);
  this->PHelper->SetInputData(0);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkOpenGLPointMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  os << "\nOGL, IHelper:" << endl;
  this->IHelper->PrintSelf(os, indent);
  os << "\nOGL, PHelper:" << endl;
  this->PHelper->PrintSelf(os, indent);
  os << "\nOGL, Super:" << endl;
  this->Superclass::PrintSelf(os, indent);
}
