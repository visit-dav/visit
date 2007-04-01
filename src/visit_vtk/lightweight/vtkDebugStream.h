#ifndef VTK_DEBUG_STREAM_H
#define VTK_DEBUG_STREAM_H
#include <visit_vtk_exports.h>

#include <vtkOutputWindow.h>

class VISIT_VTK_API vtkDebugStream : public vtkOutputWindow
{
public:
  vtkTypeMacro(vtkDebugStream,vtkObject);

  static vtkDebugStream *New();

  static void   Initialize(void);

  virtual void  DisplayText(const char *);

protected:
  vtkDebugStream() {};
  virtual ~vtkDebugStream() {};

private:
  vtkDebugStream(const vtkDebugStream&); 
  void operator=(const vtkDebugStream&);
};

#endif
