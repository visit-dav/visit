#include "vtkOpenGLMeshPlotMapper.h"

#include <vtkOpenGLHelper.h>

#include <vtkCellArray.h>
#include <vtkHardwareSelector.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>
#include <vtkOpenGLActor.h>
#include <vtkOpenGLCamera.h>
#include <vtkOpenGLIndexBufferObject.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLVertexArrayObject.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>


#include <vtk_glew.h>



class vtkOpenGLMeshPlotMapperHelper : public vtkOpenGLPolyDataMapper
{
public:
  static vtkOpenGLMeshPlotMapperHelper* New();
  vtkTypeMacro(vtkOpenGLMeshPlotMapperHelper, vtkOpenGLPolyDataMapper)

  vtkMeshPlotMapper *Owner;

  bool DrawingLines;
  bool DrawingPolys;

protected:
  vtkOpenGLMeshPlotMapperHelper();
  ~vtkOpenGLMeshPlotMapperHelper();

  virtual void SetPropertyShaderParameters(vtkOpenGLHelper &cellBO,
                                           vtkRenderer *ren, vtkActor *actor);
  // Description:
  // Does the VBO/IBO need to be rebuilt
  virtual bool GetNeedToRebuildBufferObjects(vtkRenderer *ren, vtkActor *act);

  virtual void RenderPieceDraw(vtkRenderer *ren, vtkActor *act);


private:
  vtkOpenGLMeshPlotMapperHelper(const vtkOpenGLMeshPlotMapperHelper&); // Not implemented.
  void operator=(const vtkOpenGLMeshPlotMapperHelper&); // Not implemented.
};

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkOpenGLMeshPlotMapperHelper)

//-----------------------------------------------------------------------------
vtkOpenGLMeshPlotMapperHelper::vtkOpenGLMeshPlotMapperHelper()
{
  this->Owner = NULL;
  this->DrawingLines = false;
  this->DrawingPolys = false;
}


//-----------------------------------------------------------------------------
vtkOpenGLMeshPlotMapperHelper::~vtkOpenGLMeshPlotMapperHelper()
{
}

//-----------------------------------------------------------------------------
void vtkOpenGLMeshPlotMapperHelper::SetPropertyShaderParameters(
  vtkOpenGLHelper &cellBO,
  vtkRenderer *ren, vtkActor *actor)
{
  vtkShaderProgram *program = cellBO.Program;
  vtkProperty *ppty = actor->GetProperty();
  ppty->SetAmbient(1.);
  ppty->SetDiffuse(0.);
  ppty->SetOpacity(this->Owner->GetOpacity());
  if (this->DrawingLines)
      ppty->SetColor(this->Owner->GetLinesColor());
  else if (this->DrawingPolys)
      ppty->SetColor(this->Owner->GetPolysColor());
  
  this->Superclass::SetPropertyShaderParameters(cellBO, ren, actor);

}

//-----------------------------------------------------------------------------
void vtkOpenGLMeshPlotMapperHelper::RenderPieceDraw(vtkRenderer *ren, vtkActor *actor)
{
  int linesIC = this->Primitives[PrimitiveLines].IBO->IndexCount;
  int polysIC = this->Primitives[PrimitiveTris].IBO->IndexCount;

  // draw surface first
  if (this->Owner->GetUsePolys())
  {
    this->DrawingPolys = true;
    this->DrawingLines = false;
    this->Primitives[PrimitiveLines].IBO->IndexCount = 0;
    this->Superclass::RenderPieceDraw(ren, actor);
  }
  // draw lines second
  this->DrawingPolys = false;
  this->DrawingLines = true;
  this->Primitives[PrimitiveLines].IBO->IndexCount = linesIC;
  this->Primitives[PrimitiveTris].IBO->IndexCount = 0;
  this->Superclass::RenderPieceDraw(ren, actor);
  this->Primitives[PrimitiveTris].IBO->IndexCount = polysIC;
}

//-------------------------------------------------------------------------
bool vtkOpenGLMeshPlotMapperHelper::GetNeedToRebuildBufferObjects(
  vtkRenderer *vtkNotUsed(ren),
  vtkActor *act)
{
  // picking state does not require a rebuild, unlike our parent
  if (this->VBOBuildTime < this->GetMTime() ||
      this->VBOBuildTime < act->GetMTime() ||
      this->VBOBuildTime < this->CurrentInput->GetMTime() ||
      this->VBOBuildTime < this->Owner->GetMTime()
      )
    {
    return true;
    }
  return false;
}


//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkOpenGLMeshPlotMapper)

//-----------------------------------------------------------------------------
vtkOpenGLMeshPlotMapper::vtkOpenGLMeshPlotMapper()
{
  this->Helper = vtkOpenGLMeshPlotMapperHelper::New();
  this->Helper->Owner = this;
}

vtkOpenGLMeshPlotMapper::~vtkOpenGLMeshPlotMapper()
{
  this->Helper->Delete();
  this->Helper = 0;
}

void vtkOpenGLMeshPlotMapper::RenderPiece(vtkRenderer *ren, vtkActor *act)
{
  if (this->GetMTime() > this->HelperUpdateTime)
    {
    this->Helper->vtkPolyDataMapper::ShallowCopy(this);
    this->Helper->Modified();
    this->HelperUpdateTime.Modified();
    }

  // superclass method is called because this method isn't overridden in helper
  this->Helper->RenderPiece(ren,act);
}

//-----------------------------------------------------------------------------
void vtkOpenGLMeshPlotMapper::ReleaseGraphicsResources(vtkWindow* win)
{
  this->Helper->ReleaseGraphicsResources(win);
  this->Helper->SetInputData(0);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkOpenGLMeshPlotMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Helper->PrintSelf(os, indent);
  this->Superclass::PrintSelf(os, indent);
}
