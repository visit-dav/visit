// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkVisItStreamLine_h
#define __vtkVisItStreamLine_h
#include <visit_vtk_exports.h>
#include "vtkStreamer.h"

class VISIT_VTK_API vtkVisItStreamLine : public vtkStreamer
{
public:
  vtkTypeMacro(vtkVisItStreamLine,vtkStreamer);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Construct object with step size set to 1.0.
  static vtkVisItStreamLine *New();

  // Description:
  // Specify the length of a line segment. The length is expressed in terms of
  // elapsed time. Smaller values result in smoother appearing streamlines, but
  // greater numbers of line primitives.
  vtkSetClampMacro(StepLength,double,0.000001,VTK_FLOAT_MAX);
  vtkGetMacro(StepLength,double);

protected:
  vtkVisItStreamLine();
  ~vtkVisItStreamLine() {};

  // Convert streamer array into vtkPolyData
  void Execute();

  // the length of line primitives
  double StepLength;

  void Integrate();
//  int GetNumberOfStreamers() { return vtkStreamer::GetNumberOfStreamers(); };
private:
  vtkVisItStreamLine(const vtkVisItStreamLine&);  // Not implemented.
  void operator=(const vtkVisItStreamLine&);  // Not implemented.

  static VTK_THREAD_RETURN_TYPE ThreadedIntegrate(void *arg);
};

#endif


