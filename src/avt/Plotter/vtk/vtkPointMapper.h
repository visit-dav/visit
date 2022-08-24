// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef vtkPointMapper_h
#define vtkPointMapper_h

#include <plotter_exports.h>

#include <vtkPolyDataMapper.h>


// ****************************************************************************
// Class: vtkPointMapper
//
// Purpose:
//   Subclass of vtkPolyDataMapper that can draw points as GL points
//   or sphere imposters.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 17, 2016
//
//  Modifications:
//
// **************************************************************************** 

class PLOTTER_API vtkPointMapper : public vtkPolyDataMapper
{
public:
  static vtkPointMapper *New();
  vtkTypeMacro(vtkPointMapper, vtkPolyDataMapper)
  void PrintSelf(ostream& os, vtkIndent indent) override;


  // Toggle for Sphere Imposters
  vtkSetMacro(UseImposters, bool);
  vtkGetMacro(UseImposters, bool);
  vtkBooleanMacro(UseImposters, bool);

  // Controls for Imposters size
  // (GL Points size controlled by vtkProperty::PointSize).

  // Description:
  // Convenience method to set the array to scale with.
  vtkSetStringMacro(ImposterScaleArray);
  vtkGetStringMacro(ImposterScaleArray);

  // Description:
  // Convenience method to set a single radius to scale with.
  // Will be used if ScaleArray has not been set or does not exist in input.
  vtkSetMacro(ImposterRadius, float);
  vtkGetMacro(ImposterRadius, float);

protected:
  vtkPointMapper();
 ~vtkPointMapper();

  bool UseImposters;
  char *ImposterScaleArray;
  float ImposterRadius;

private:
  vtkPointMapper(const vtkPointMapper&) = delete;
  void operator=(const vtkPointMapper&) = delete;
};

#endif
