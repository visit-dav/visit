// ************************************************************************* //
//                        avtLabeledCurveMapper.h                            //
// ************************************************************************* //

#ifndef AVT_LABELEDCURVE_MAPPER_H
#define AVT_LABELEDCURVE_MAPPER_H
#include <plotter_exports.h>


#include <avtDecorationsMapper.h>

class     vtkMaskPoints;
class     vtkDataSet;


// ****************************************************************************
//  Class: avtLabeledCurveMapper
//
//  Purpose:
//      A mapper for labeled curves.  This extends the functionality of a 
//      decorations mapper by mapping a label-glyph onto a curve dataset.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul 23 15:01:55 PDT 2002 
//    Added member to keep track of label visibility.
// 
// ****************************************************************************

class PLOTTER_API  avtLabeledCurveMapper : public avtDecorationsMapper
{
  public:
                               avtLabeledCurveMapper();
    virtual                   ~avtLabeledCurveMapper();

    void                       SetLabelColor(double [3]);
    void                       SetLabelColor(double, double, double);
    void                       SetLabel(std::string &);
    void                       SetScale(double);
    void                       SetLabelVisibility(bool);

  protected:
    std::string                label; 
    bool                       labelVis;
    double                     labelColor[3];
    double                     scale;

    vtkMaskPoints            **filter;
    int                        nFilters;

    virtual void               CustomizeMappers(void);

    virtual void               SetDatasetInput(vtkDataSet *, int);
    virtual void               SetUpFilters(int);
};


#endif


