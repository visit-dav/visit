// ************************************************************************* // 
//                            avtLevelsLegend.h                              // 
// ************************************************************************* //

#ifndef AVT_LEVELS_LEGEND_H
#define AVT_LEVELS_LEGEND_H
#include <plotter_exports.h>


#include <avtLegend.h>
#include <vector>
#include <string>
#include <maptypes.h>

class  vtkVerticalScalarBarActor;
class  vtkLookupTable;


// ****************************************************************************
//  Class: avtLevelsLegend
//
//  Purpose:
//      Provides access to the vtk class that builds a legend. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 1, 2001
//
//  Modifications:
//    Kathleen Bonnell, Mon Apr  2 18:30:33 PDT 2001
//    Added method 'SetVarRange' so that limits text in legend will always
//    be the extents of the var, even if user sets artificial limits.
//
//    Kathleen Bonnell, Mon Sep 24 15:58:35 PDT 2001 
//    Added member vtkLookupTable. 
//
//    Kathleen Bonnell, Mon Apr 29 13:37:14 PDT 2002  
//    Added member varName. 
//
//    Kathleen Bonnell, Tue Sep 10 14:14:18 PDT 2002 
//    Added method 'SetLabelColorMap', which maps a label to its corresponding
//    index into the lookup table. 
//
//    Brad Whitlock, Fri Nov 15 10:13:14 PDT 2002
//    I changed the map type.
//
//    Kathleen Bonnell, Mon May 19 13:47:48 PDT 2003  
//    Added method 'SetReverseOrder'.
//
// ****************************************************************************

class PLOTTER_API avtLevelsLegend : public avtLegend
{
  public:
                    avtLevelsLegend();
    virtual        ~avtLevelsLegend();

    void            SetColorBar(const int);
    void            SetLevels(const std::vector<double> &);
    void            SetLevels(const std::vector<std::string> &);
    void            SetLookupTable(vtkLookupTable *);
    void            SetMessage(const char *msg);
    void            SetVarRange(float min, float max);
    void            SetRange(float min, float max);
    void            SetRange(const int);
    void            SetTitle(const char *title);
    virtual void    SetVarName(const char *name);
    void            SetLabelColorMap(const LevelColorMap &);
    void            SetReverseOrder(const bool);


  protected:
    float                      min, max;
    vtkLookupTable            *lut;
    vtkVerticalScalarBarActor *sBar;
    char                      *varName;
    virtual void               ChangePosition(float, float);
};


#endif


