#ifndef __vtkRGBWriter_h
#define __vtkRGBWriter_h
#include <visit_vtk_exports.h>

#include "vtkImageWriter.h"

class VISIT_VTK_API vtkRGBWriter : public vtkImageWriter
{
public:
  static vtkRGBWriter *New();
  vtkTypeMacro(vtkRGBWriter,vtkImageWriter);

protected:
  vtkRGBWriter();
  ~vtkRGBWriter() {};

  virtual void WriteFile(ofstream *file, vtkImageData *data, int ext[6]);
  virtual void WriteFileHeader(ofstream *, vtkImageData *);

private:
  vtkRGBWriter(const vtkRGBWriter&);
  void operator=(const vtkRGBWriter&);
};

#endif
