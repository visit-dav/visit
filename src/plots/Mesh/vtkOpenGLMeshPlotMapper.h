
#ifndef vtkOpenGLMeshPlotMapper_h
#define vtkOpenGLMeshPlotMapper_h

#include "vtkMeshPlotMapper.h"

class vtkOpenGLMeshPlotMapperHelper;

class vtkOpenGLMeshPlotMapper : public vtkMeshPlotMapper
{
public:
  static vtkOpenGLMeshPlotMapper* New();
  vtkTypeMacro(vtkOpenGLMeshPlotMapper, vtkMeshPlotMapper)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);


protected:
  vtkOpenGLMeshPlotMapper();
  ~vtkOpenGLMeshPlotMapper();

  virtual void RenderPiece(vtkRenderer *ren, vtkActor *act);

  vtkOpenGLMeshPlotMapperHelper *Helper;
  vtkTimeStamp HelperUpdateTime;

private:
  vtkOpenGLMeshPlotMapper(const vtkOpenGLMeshPlotMapper&); // Not implemented.
  void operator=(const vtkOpenGLMeshPlotMapper&); // Not implemented.
};

#endif
