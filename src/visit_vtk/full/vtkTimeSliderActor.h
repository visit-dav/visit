#ifndef VTK_TIME_SLIDER_ACTOR_H
#define VTK_TIME_SLIDER_ACTOR_H
#include <visit_vtk_exports.h>
#include <vtkActor2D.h>

class vtkCellArray;
class vtkPolyData;
class vtkPolyDataMapper2D;

// ****************************************************************************
// Class: vtkTimeSliderActor
//
// Purpose:
//   This class is a time slider actor that can be added to a VTK renderer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 28 11:37:07 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VISIT_VTK_API vtkTimeSliderActor : public vtkActor2D
{
public:
  vtkTypeMacro(vtkTimeSliderActor, vtkActor2D);
  void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Instantiate a time slider actor. 
  static vtkTimeSliderActor *New();

  // Description:
  // Draw the actor as per the vtkProp superclass' API.
  virtual int RenderOverlay(vtkViewport *);
  virtual int RenderOpaqueGeometry(vtkViewport *);
  virtual int RenderTranslucentGeometry(vtkViewport *);

  vtkSetVector4Macro(StartColor, double);
  vtkGetVector4Macro(StartColor, double);

  vtkSetVector4Macro(EndColor, double);
  vtkGetVector4Macro(EndColor, double);

  vtkSetMacro(ParametricTime, double);
  vtkGetMacro(ParametricTime, double);

  vtkSetClampMacro(VerticalDivisions, int, 1, 16);
  vtkGetMacro(VerticalDivisions, int);

  vtkSetClampMacro(DrawEndCaps, int, 0, 1);
  vtkGetMacro(DrawEndCaps, int);

  vtkSetClampMacro(RadialDivisions, int, 5, 100);
  vtkGetMacro(RadialDivisions, int);

  vtkSetClampMacro(Draw3D, int, 0, 1);
  vtkGetMacro(Draw3D, int);

protected:
  vtkTimeSliderActor();
  ~vtkTimeSliderActor();
  void DestroySlider();
  void AddEndCapCells(int, vtkCellArray *);
  void CreateSlider(vtkViewport *viewport);

  double  StartColor[4];
  double  EndColor[4];
  double ParametricTime;

  int    VerticalDivisions;
  int    DrawEndCaps;
  int    RadialDivisions;
  int    Draw3D;

  vtkPolyData         *SliderPolyData;
  vtkActor2D          *SliderActor;
  vtkPolyDataMapper2D *SliderMapper;

private:
  vtkTimeSliderActor(const vtkTimeSliderActor &);
  void operator =(const vtkTimeSliderActor &);
};

#endif
