// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

//=========================================================================
// .NAME vtkLineoutFilter - 
// .SECTION Description
// vtkLineoutFilter is a filter object that applies a probe filter to
// the input.  The probe filter output is transformed into x-y pairs
// (vertices), with x representing the distance along the probe-line
// and y the interpolated scalar value at that point. 
//
// Points determined as 'invalid' by the probe filter
// are not included in the output.
//
//

#ifndef __vtkLineoutFilter_h
#define __vtkLineoutFilter_h
#include <visit_vtk_exports.h>

#include <vtkPolyDataAlgorithm.h>

class vtkCellDataToPointData;
class vtkLineSource;
class vtkVisItProbeFilter;

// ***************************************************************************
//  Class: vtkLineoutFilter
//
//  Modifications:
//    Kathleen Bonnell, Fri Mar 28 12:09:01 PDT 2008
//    Removed cd2pd, use VisIt version of vtkProbeFilter.
//
//    Eric Brugger, Thu Jan 10 09:45:55 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ***************************************************************************

class VISIT_VTK_API vtkLineoutFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkLineoutFilter *New();
  vtkTypeMacro(vtkLineoutFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set/Get the endpoints of the line used for probing. 
  vtkSetVector3Macro(Point1, double); 
  vtkGetVectorMacro(Point1, double, 3); 

  vtkSetVector3Macro(Point2, double); 
  vtkGetVectorMacro(Point2, double, 3); 

  vtkSetMacro(NumberOfSamplePoints, int);
  vtkGetMacro(NumberOfSamplePoints, int);

protected:
  vtkLineoutFilter();
  ~vtkLineoutFilter();

  vtkLineSource          *LineSource;
  vtkVisItProbeFilter    *Probe;

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

private:
  double          Point1[3];
  double          Point2[3];
  int             NumberOfSamplePoints;

  vtkLineoutFilter(const vtkLineoutFilter&);
  void operator=(const vtkLineoutFilter&);
};

#endif
