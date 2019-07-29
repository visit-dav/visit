// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtCurveMapper.h                                  //
// ************************************************************************* //

#ifndef AVT_CURVE_MAPPER_H
#define AVT_CURVE_MAPPER_H


#include <avtMapper.h>
#include <LineAttributes.h>

// ****************************************************************************
//  Class: avtCurveMapper
//
//  Purpose:
//
//  Programmer: Kathleen Biagas 
//  Creation:   April 13, 2017
//
//  Modifications:
//
//      Alister Maguire, Mon Jun  4 15:13:43 PDT 2018
//      Added SetViewScale to allow adding a view scale
//      to the mapper. 
//
//      Alister Maguire, Mon Jun 11 10:15:25 PDT 2018
//      Added time cue options. 
//
// ****************************************************************************

class avtCurveMapper : virtual public avtMapper
{
  public:
                              avtCurveMapper();
    virtual                  ~avtCurveMapper();


    void                      SetColor(double [3]);
    void                      SetColor(double, double, double); 

    void                      SetLineWidth(_LineWidth lw);

    void                      SetDrawCurve(bool);
    void                      SetDrawPoints(bool);
    void                      SetStaticPoints(bool);

    void                      SetPointSize(double);
    void                      SetPointStride(int);
    void                      SetPointDensity(int);
    void                      SetSymbolType(int);
    void                      SetTimeForTimeCue(double);
    void                      SetDoBallTimeCue(bool);
    void                      SetTimeCueBallSize(double);
    void                      SetTimeCueBallColor(double [3]);
    void                      SetDoLineTimeCue(bool);
    void                      SetTimeCueLineWidth(double);
    void                      SetTimeCueLineColor(double [3]);
    void                      SetDoCropTimeCue(bool);
    bool                      SetFullFrameScaling(bool, const double *);
    bool                      SetViewScale(const double);            

  protected:
    virtual vtkDataSetMapper *CreateMapper(void); 
    virtual void              CustomizeMappers(void);

  private:
    double                    curveColor[3]; 
    _LineWidth                lineWidth;
    bool                      drawCurve;
    bool                      drawPoints;
    double                    pointSize;
    int                       pointStride;
    bool                      staticPoints;
    int                       pointDensity;
    double                    ffScale[3];
    int                       symbolType;
    double                    timeForTimeCue;
    bool                      doBallTimeCue;
    double                    timeCueBallSize;
    double                    timeCueBallColor[3]; 
    bool                      doLineTimeCue;
    double                    timeCueLineWidth;
    double                    timeCueLineColor[3];
    bool                      doCropTimeCue;

};

#endif


