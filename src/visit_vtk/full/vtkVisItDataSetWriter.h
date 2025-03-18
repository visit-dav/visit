// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//     vtkVisItDataSetWriter.h
//
// This subclass of vtkDataSetWriter is here to provide exactly the same
// functionality as vtkDataSetWriter, but setting the default file format
// to version 4.2 instead of 5.1.
// ************************************************************************* //



#ifndef __vtkVisItDataSetWriter_h
#define __vtkVisItDataSetWriter_h

#include <visit_vtk_exports.h>
#include <vtkDataSetWriter.h>

class VISIT_VTK_API vtkVisItDataSetWriter : public vtkDataSetWriter
{
public:
  static vtkVisItDataSetWriter* New();
  vtkTypeMacro(vtkVisItDataSetWriter, vtkDataSetWriter)

  vtkVisItDataSetWriter();
  ~vtkVisItDataSetWriter() {}

private:
  vtkVisItDataSetWriter(const vtkVisItDataSetWriter&) = delete;
  void operator=(const vtkVisItDataSetWriter&) = delete;
};

#endif
