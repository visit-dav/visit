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

#include <vtkDataSetToPolyDataFilter.h>
#include <vtkSetGet.h>
#include <visit_vtk_exports.h>

class vtkCellDataToPointData;
class vtkLineSource;
class vtkProbeFilter;

class VISIT_VTK_API vtkLineoutFilter : public vtkDataSetToPolyDataFilter
{
public:
  static vtkLineoutFilter *New();
  vtkTypeMacro(vtkLineoutFilter,vtkDataSetToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  ~vtkLineoutFilter() ;

  vtkCellDataToPointData *cd2pd;
  vtkLineSource          *LineSource;
  vtkProbeFilter         *Probe;

  void Execute();

private:
  double          Point1[3];
  double          Point2[3];
  int             NumberOfSamplePoints;

  vtkLineoutFilter(const vtkLineoutFilter&);
  void operator=(const vtkLineoutFilter&);
};

#endif


