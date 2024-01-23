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

  void SetWriteMTL(bool);
  void SetMTLHasTexture(bool);
  void SetBasename(std::string);
  void SetTexFilename(std::string);

protected:
  vtkOBJWriter();
  ~vtkOBJWriter();

  void WriteData() override;
private:
  vtkOBJWriter(const vtkOBJWriter&);  // Not implemented.
  void operator=(const vtkOBJWriter&);  // Not implemented.

  char *Label;
  bool writeMTL;
  bool hasTex;
  std::string basename;
  std::string texFilename;
};

#endif

