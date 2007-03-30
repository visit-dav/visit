// ************************************************************************* //
//                                avtLegend.h                                //
// ************************************************************************* //

#ifndef AVT_LEGEND_H
#define AVT_LEGEND_H
#include <plotter_exports.h>


#include <ref_ptr.h>


class  vtkActor2D;
class  vtkRenderer;


// ****************************************************************************
//  Class: avtLegend
//
//  Purpose:
//      Make a legend for the plots.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Dec 22 15:57:46 PST 2000
//    Removed inheritance from avtDecorator.
//
// ****************************************************************************

class PLOTTER_API  avtLegend
{
  public:
                                  avtLegend();
    virtual                      ~avtLegend();

    virtual void                  Add(vtkRenderer *);
    virtual void                  Remove(void);

    virtual void                  GetLegendPosition(float &, float &);
    virtual void                  SetLegendPosition(float, float);
    virtual void                  GetLegendSize(float &, float &);
    virtual void                  LegendOn(void);
    virtual void                  LegendOff(void);

    virtual void                  SetForegroundColor(const float [3]);
    virtual void                  SetVarName(const char *) {};

  protected:
    float                         position[2];
    float                         size[2];
    bool                          legendOn;
    bool                          currentlyDrawn;
    vtkActor2D                   *legend;
    vtkRenderer                  *renderer;

    virtual void                  ChangePosition(float, float) = 0;
};


typedef ref_ptr<avtLegend> avtLegend_p;


#endif


