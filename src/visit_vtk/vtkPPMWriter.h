#ifndef __vtkPPMWriter_h
#define __vtkPPMWriter_h
#include <visit_vtk_exports.h>

#include "vtkImageWriter.h"

class VISIT_VTK_API vtkPPMWriter : public vtkImageWriter
{
public:
  static vtkPPMWriter *New();
  vtkTypeMacro(vtkPPMWriter,vtkImageWriter);

protected:
  vtkPPMWriter();
  ~vtkPPMWriter() {};

  virtual void WriteFile(ofstream *file, vtkImageData *data, int ext[6]);
  virtual void WriteFileHeader(ofstream *, vtkImageData *);

private:
  vtkPPMWriter(const vtkPPMWriter&);
  void operator=(const vtkPPMWriter&);
};

#endif
