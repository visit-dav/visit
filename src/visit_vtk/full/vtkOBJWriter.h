// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// Heavily borrowed from vtkOBJExporter.
//
// Written by Hank Childs on May 27th, 2002.


#ifndef __vtkOBJWriter_
#define __vtkOBJWriter_

#include <visit_vtk_exports.h>
#include <vtkPolyDataWriter.h>

class VISIT_VTK_API vtkOBJWriter : public vtkPolyDataWriter
{
public:
  static vtkOBJWriter *New();
  vtkTypeMacro(vtkOBJWriter,vtkPolyDataWriter);

  // Description:
  // Give a label indicating what is in the file.
  vtkSetStringMacro(Label);
  vtkGetStringMacro(Label);

  void SetWriteMTL(bool writeMTL);
  void SetMTLHasTexture(bool hasTex);

protected:
  vtkOBJWriter();
  ~vtkOBJWriter();

  void WriteData() override;
private:
  vtkOBJWriter(const vtkOBJWriter&);  // Not implemented.
  void operator=(const vtkOBJWriter&);  // Not implemented.

  char *Label;
};

#endif

