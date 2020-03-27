// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef vtkOpenGLPointMapper_h
#define vtkOpenGLPointMapper_h

#include <plotter_exports.h>

#include "vtkPointMapper.h"

class vtkImposterHelper;
class vtkPointHelper;

class PLOTTER_API vtkOpenGLPointMapper : public vtkPointMapper
{
public:
  static vtkOpenGLPointMapper *New();
  vtkTypeMacro(vtkOpenGLPointMapper, vtkPointMapper)
  void PrintSelf(ostream &os, vtkIndent indent) override;

  void ReleaseGraphicsResources(vtkWindow *) override;

protected:
  vtkOpenGLPointMapper();
 ~vtkOpenGLPointMapper();

  virtual void RenderPiece(vtkRenderer *ren, vtkActor *act) override;

  vtkImposterHelper *IHelper;
  vtkPointHelper *PHelper;
  vtkTimeStamp IHelperUpdateTime;
  vtkTimeStamp PHelperUpdateTime;

private:
  vtkOpenGLPointMapper(const vtkOpenGLPointMapper&) = delete;
  void operator=(const vtkOpenGLPointMapper&) = delete;

};

#endif
