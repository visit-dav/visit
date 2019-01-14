/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/


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
