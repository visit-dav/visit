// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// Heavily borrowed from vtkOBJExporter.
//
// Written by Hank Childs on May 27th, 2002.

// Modifications:
//   Justin Privitera, Fri Nov  3 15:25:32 PDT 2023
//   Added writeMTL, MTLHasTexture, basename, and texFilename.
// ****************************************************************************


#ifndef __vtkVisItOBJWriter_
#define __vtkVisItOBJWriter_

#include <visit_vtk_exports.h>
#include <vtkPolyDataWriter.h>

class VISIT_VTK_API vtkVisItOBJWriter : public vtkPolyDataWriter
{
public:
  static vtkVisItOBJWriter *New();
  vtkTypeMacro(vtkVisItOBJWriter,vtkPolyDataWriter);

  // Description:
  // Give a label indicating what is in the file.
  vtkSetStringMacro(Label);
  vtkGetStringMacro(Label);

  void SetWriteMTL(bool);
  void SetMTLHasTexture(bool);
  void SetBasename(std::string);
  void SetTexFilename(std::string);

protected:
  vtkVisItOBJWriter();
  ~vtkVisItOBJWriter();

  void WriteData() override;
private:
  vtkVisItOBJWriter(const vtkVisItOBJWriter&);  // Not implemented.
  void operator=(const vtkVisItOBJWriter&);  // Not implemented.

  char *Label;
  bool writeMTL;
  bool hasTex;
  std::string basename;
  std::string texFilename;
};

#endif

