// TAKEN FROM THE 7/25 VTK SOURCE
//
// Modifications:
//   Jeremy Meredith, Wed Jul 31 17:40:32 PDT 2002
//   Renamed to vtkVisItSTLWriter.
//   Changed the class export.

#ifndef __vtkVisItSTLWriter_h
#define __vtkVisItSTLWriter_h

#include <visit_vtk_exports.h>
#include "vtkPolyDataWriter.h"

class VISIT_VTK_API vtkVisItSTLWriter : public vtkPolyDataWriter
{
public:
  static vtkVisItSTLWriter *New();
  vtkTypeRevisionMacro(vtkVisItSTLWriter,vtkPolyDataWriter);

protected:
  vtkVisItSTLWriter();
  ~vtkVisItSTLWriter() {};

  void WriteData();

  void WriteBinarySTL(vtkPoints *pts, vtkCellArray *polys);
  void WriteAsciiSTL(vtkPoints *pts, vtkCellArray *polys);
private:
  vtkVisItSTLWriter(const vtkVisItSTLWriter&);  // Not implemented.
  void operator=(const vtkVisItSTLWriter&);  // Not implemented.
};

#endif

