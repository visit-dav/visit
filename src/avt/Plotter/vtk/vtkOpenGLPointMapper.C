/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

  vtkUnsignedCharArray *MapScalars(double alpha) VTK_OVERRIDE;

protected:
  vtkImposterHelper();
  ~vtkImposterHelper() VTK_OVERRIDE;


private:
  vtkImposterHelper(const vtkImposterHelper&) VTK_DELETE_FUNCTION;
  void operator=(const vtkImposterHelper&)    VTK_DELETE_FUNCTION;

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
  ~vtkPointHelper() VTK_OVERRIDE;

private:
  vtkPointHelper(const vtkPointHelper&)
    VTK_DELETE_FUNCTION;
  void operator=(const vtkPointHelper&) VTK_DELETE_FUNCTION;
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
