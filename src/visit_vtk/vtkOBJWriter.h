// Heavily borrowed from vtkOBJExporter.
//
// Written by Hank Childs on May 27th, 2002.


#ifndef __vtkOBJWriter_
#define __vtkOBJWriter_

#include <stdio.h>
#include <visit_vtk_exports.h>
#include "vtkPolyDataWriter.h"
#include "vtkPolyData.h"

class VISIT_VTK_API vtkOBJWriter : public vtkPolyDataWriter
{
public:
  static vtkOBJWriter *New();
  vtkTypeRevisionMacro(vtkOBJWriter,vtkPolyDataWriter);

  // Description:
  // Give a label indicating what is in the file.
  vtkSetStringMacro(Label);
  vtkGetStringMacro(Label);

protected:
  vtkOBJWriter();
  ~vtkOBJWriter();

  void WriteData();
private:
  vtkOBJWriter(const vtkOBJWriter&);  // Not implemented.
  void operator=(const vtkOBJWriter&);  // Not implemented.

  char *Label;
};

#endif

