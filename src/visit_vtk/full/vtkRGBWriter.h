// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

  void WriteFile(ostream *file, vtkImageData *data, int ext[6], int wExt[6]) override;
  void WriteFileHeader(ostream *, vtkImageData *, int wExt[6]) override;

private:
  vtkRGBWriter(const vtkRGBWriter&);
  void operator=(const vtkRGBWriter&);
};

#endif
