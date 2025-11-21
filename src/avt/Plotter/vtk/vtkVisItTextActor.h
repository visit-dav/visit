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
//   Cyrus Harrison, Wed Nov 19 11:32:07 PST 2025
//   Add RenderOpaqueGeometry() to allow us to resize on viewport change
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
  int RenderOpaqueGeometry(vtkViewport* viewport) override;
  void ComputeScaledFont(vtkViewport *viewport) override;

protected:
  float TextHeight;

  double GetDPIScaledFontSize(vtkViewport *viewport,
                              double fontHeight);
  vtkVisItTextActor();
  ~vtkVisItTextActor();
};

#endif
