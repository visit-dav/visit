#ifndef vtkMeshPlotMapper_h
#define vtkMeshPlotMapper_h


#include <vtkPolyDataMapper.h>

class vtkMeshPlotMapper : public vtkPolyDataMapper
{
public:
  static vtkMeshPlotMapper* New();
  vtkTypeMacro(vtkMeshPlotMapper, vtkPolyDataMapper);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetLinesColor(double rgb[3]); 
  double *GetLinesColor();

  void SetPolysColor(double rgb[3]); 
  double *GetPolysColor();

  vtkSetMacro(UsePolys, bool);
  vtkGetMacro(UsePolys, bool);

  vtkSetMacro(Opacity, double);
  vtkGetMacro(Opacity, double);

protected:
  vtkMeshPlotMapper();
 ~vtkMeshPlotMapper();
  
  double LinesColor[3];

  double PolysColor[3];
  bool UsePolys;

  double Opacity;
private:
  vtkMeshPlotMapper(const vtkMeshPlotMapper&);
  void operator=(const vtkMeshPlotMapper&);
};

#endif
