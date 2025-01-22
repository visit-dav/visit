// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVisItDataSetWriter.h"
#include <vtkObjectFactory.h>
#include <iostream>

vtkStandardNewMacro(vtkVisItDataSetWriter)

vtkVisItDataSetWriter::vtkVisItDataSetWriter() : vtkDataSetWriter()
{
  this->FileVersion = VTK_LEGACY_READER_VERSION_4_2;
}

