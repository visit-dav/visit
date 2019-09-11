// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkOpenEXRWriter_h
#define __vtkOpenEXRWriter_h
#include <visit_vtk_exports.h>

#include "vtkImageWriter.h"
class vtkFloatArray;

// ****************************************************************************
// Class: vtkOpenEXRWriter
//
// Purpose:
//   Writes OpenEXR files.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 27 14:56:41 PDT 2017
//
// Modifications:
//
// ****************************************************************************

class VISIT_VTK_API vtkOpenEXRWriter : public vtkImageWriter
{
public:
  static vtkOpenEXRWriter *New();
  vtkTypeMacro(vtkOpenEXRWriter,vtkImageWriter);

  void SetZBuffer(vtkFloatArray *arr);
  vtkFloatArray *GetZBuffer();

protected:
  vtkOpenEXRWriter();
  virtual ~vtkOpenEXRWriter();

  virtual void Write() override;

private:
  vtkOpenEXRWriter(const vtkOpenEXRWriter&);
  void operator=(const vtkOpenEXRWriter&);

  vtkFloatArray *zbuffer;
};

#endif
