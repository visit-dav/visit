#ifndef __vtkVisItStreamLine_h
#define __vtkVisItStreamLine_h
#include <visit_vtk_exports.h>
#include "vtkStreamer.h"

class VISIT_VTK_API vtkVisItStreamLine : public vtkStreamer
{
public:
  vtkTypeRevisionMacro(vtkVisItStreamLine,vtkStreamer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with step size set to 1.0.
  static vtkVisItStreamLine *New();

  // Description:
  // Specify the length of a line segment. The length is expressed in terms of
  // elapsed time. Smaller values result in smoother appearing streamlines, but
  // greater numbers of line primitives.
  vtkSetClampMacro(StepLength,float,0.000001,VTK_LARGE_FLOAT);
  vtkGetMacro(StepLength,float);

protected:
  vtkVisItStreamLine();
  ~vtkVisItStreamLine() {};

  // Convert streamer array into vtkPolyData
  void Execute();

  // the length of line primitives
  float StepLength;

  void Integrate();
//  int GetNumberOfStreamers() { return vtkStreamer::GetNumberOfStreamers(); };
private:
  vtkVisItStreamLine(const vtkVisItStreamLine&);  // Not implemented.
  void operator=(const vtkVisItStreamLine&);  // Not implemented.

  static VTK_THREAD_RETURN_TYPE ThreadedIntegrate(void *arg);
};

#endif


