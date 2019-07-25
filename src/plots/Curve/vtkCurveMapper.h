// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         vtkCurveMapper.h                                  //
// ************************************************************************* //


#ifndef vtkCurveMapper_h
#define vtkCurveMapper_h

#include <vtkPolyDataMapper.h>
#include <vtkLineSource.h>
#include <vtkNew.h>


#include <string>

class vtkTrivialProducer;

// ****************************************************************************
// Class: vtkCurveMapper
//
// Purpose:
//   Subclass of vtkPolyDataMapper that can render curves(lines) and points
//   along those lines as points or symbols. 
//
//  Programmer: Kathleen Biagas 
//  Creation:   April 13, 2017 
//
//  Modifications:
//
//      Alister Maguire, Mon Jun  4 15:13:43 PDT 2018
//      Added ViewScale to scale the up vector from the model view
//      matrix when necessary. 
//
//      Alister Maguire, Mon Jun 11 10:15:25 PDT 2018
//      Implemented the time cue options. 
//
// **************************************************************************** 

class vtkCurveMapper : public vtkPolyDataMapper
{
public:
  static vtkCurveMapper *New();
  vtkTypeMacro(vtkCurveMapper,vtkPolyDataMapper);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void Render(vtkRenderer *, vtkActor *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;

  // Toggle for drawing the curve lines
  vtkGetMacro(DrawCurve, bool);
  vtkSetMacro(DrawCurve, bool);
 
  // Settings for drawing the curve points
  vtkGetMacro(DrawPoints, bool);
  vtkSetMacro(DrawPoints, bool);
  vtkGetMacro(SymbolType, int);
  vtkSetClampMacro(SymbolType, int, 0, 6);
  vtkSetMacro(PointStride, int);
  vtkGetMacro(PointStride, int);
  vtkGetMacro(StaticPoints, bool);
  vtkSetMacro(StaticPoints, bool);
  vtkSetMacro(PointDensity, int);
  vtkGetMacro(PointDensity, int);
  vtkSetMacro(PointSize, double);
  vtkGetMacro(PointSize, double);

  vtkSetMacro(ViewScale, double);
  vtkGetMacro(ViewScale, double);

  // Settings for time cue
  vtkSetMacro(TimeForTimeCue, double);
  vtkGetMacro(TimeForTimeCue, double);
  vtkSetMacro(DoBallTimeCue, bool);
  vtkGetMacro(DoBallTimeCue, bool);
  vtkSetMacro(TimeCueBallSize, double);
  vtkGetMacro(TimeCueBallSize, double);
  vtkSetVector3Macro(TimeCueBallColor, double);
  vtkGetVector3Macro(TimeCueBallColor, double);
  vtkSetMacro(DoLineTimeCue, bool);
  vtkGetMacro(DoLineTimeCue, bool);
  vtkSetMacro(TimeCueLineWidth, double);
  vtkGetMacro(TimeCueLineWidth, double);
  vtkSetVector3Macro(TimeCueLineColor, double);
  vtkGetVector3Macro(TimeCueLineColor, double);
  vtkSetMacro(DoCropTimeCue, bool);
  vtkGetMacro(DoCropTimeCue, bool);

  vtkSetVector3Macro(FFScale, double);
  vtkGetVector3Macro(FFScale, double);

protected:
  vtkCurveMapper();
  ~vtkCurveMapper();

  void RenderPiece(vtkRenderer *, vtkActor *) override;
 

  bool        DrawCurve;
  bool        DrawPoints;
  int         SymbolType;
  int         PointStride;
  bool        StaticPoints;
  int         PointDensity;
  double      PointSize;
  double      ViewScale;
  double      TimeForTimeCue;
  bool        DoBallTimeCue;
  double      TimeCueBallSize;
  double      TimeCueBallColor[3];
  bool        DoLineTimeCue;
  double      TimeCueLineWidth;
  double      TimeCueLineColor[3];
  bool        DoCropTimeCue;
  double      FFScale[3];

  
  vtkNew<vtkPolyData>             LinesPolyData; 
  vtkNew<vtkTrivialProducer>      LinesOutput;
  vtkNew<vtkPolyDataMapper>       LinesMapper;
  bool                            LinesDataInitialized;

  vtkNew<vtkPolyData>             PointsPolyData; 
  vtkNew<vtkTrivialProducer>      PointsOutput;
  vtkNew<vtkPolyDataMapper>       PointsMapper;
  bool                            PointsDataInitialized;

  vtkNew<vtkPolyData>             BallCuePolyData;
  vtkNew<vtkTrivialProducer>      BallCueOutput;
  vtkNew<vtkPolyDataMapper>       BallCueMapper;
  bool                            BallCueDataInitialized;

  vtkNew<vtkLineSource>           LineCueData;
  vtkNew<vtkPolyDataMapper>       LineCueMapper;
  bool                            LineCueDataInitialized;

private:
  vtkCurveMapper(const vtkCurveMapper&); 
  void operator=(const vtkCurveMapper&);

  void UpdateLinesData(vtkRenderer *);
  void UpdatePointsData(vtkRenderer *);
  void UpdateBallCueData(vtkRenderer *);
  void UpdateLineCueData(vtkRenderer *);
  bool GetAspect(vtkRenderer *, int &, double &, double &,
                 int &, double &, double &);
  void SetUpPoints(vtkRenderer *);
  void SetUpBallCue(vtkRenderer *);
  void SetUpLineCue(vtkRenderer *);

};

#endif
