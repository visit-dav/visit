// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkVisItTextActor_h
#define __vtkVisItTextActor_h
#include <plotter_exports.h>
#include <vtkTextActor.h>

// ****************************************************************************
// Class: vtkVisItTextActor
//
// Purpose:
//   Subclass of vtkTextActor that scales only as a percentage of viewport height .
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 19 15:20:50 PDT 2011
//
// Modifications:
//
// ****************************************************************************

class PLOTTER_API vtkVisItTextActor : public vtkTextActor
{
public:
  vtkTypeMacro(vtkVisItTextActor,vtkTextActor);

  // Description:
  // Instantiate object.
  static vtkVisItTextActor *New();

  void SetTextHeight(float val);
  vtkGetMacro(TextHeight, float);

  void ComputeScaledFont(vtkViewport *viewport) override;
protected:
  float TextHeight;

  vtkVisItTextActor();
  ~vtkVisItTextActor();
};

#endif
