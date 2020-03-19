// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkPolyDataRelevantPointsFilter - strips away points and associated
// point data that are not referenced by any cell in the input. 
// .SECTION Description
// vtkPolyDataRelevantPointsFilter is a filter that takes polygonal data as 
// input and generates polygonal data as output. 
// vtkPolyDataRelevantPointsFilter removes points and associated point data
// that are not referenced by any cell in the input data set.
//
// .SECTION Caveats
//
// .SECTION See Also
// vtkPolyDataAlgorithm

#ifndef __vtkPolyDataRelevantPointsFilter_h
#define __vtkPolyDataRelevantPointsFilter_h
#include <visit_vtk_light_exports.h>

#include "vtkPolyDataAlgorithm.h"

// ***************************************************************************
//  Modifications:
//    Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002 
//    Replace 'GetClassName' with vtkTypeMacro to match VTK 4.0 API.
//    Moved unimplmented copy constructor and operator= to private section. 
//
//    Eric Brugger, Wed Jan  9 13:12:39 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ***************************************************************************

class VISIT_VTK_LIGHT_API vtkPolyDataRelevantPointsFilter : 
  public vtkPolyDataAlgorithm
{
public:
  static vtkPolyDataRelevantPointsFilter *New();
  vtkTypeMacro(vtkPolyDataRelevantPointsFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkPolyDataRelevantPointsFilter(){};
 ~vtkPolyDataRelevantPointsFilter(){};

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;

private:
  vtkPolyDataRelevantPointsFilter(const vtkPolyDataRelevantPointsFilter&); 
  void operator=(const vtkPolyDataRelevantPointsFilter&); 
};

#endif
