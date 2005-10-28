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
//    Eric Brugger, Mon Jul 14 15:52:46 PDT 2003
//    Moved SetVarName, SetTitle and SetMessage to avtLegend.
//
//    Eric Brugger, Wed Jul 16 08:03:26 PDT 2003
//    I added GetLegendSize.  I renamed SetColorBar to SetColorBarVisibility
//    and SetRange to SetVarRangeVisibility.  I also added data members to
//    track the number of levels, the colorbar visibility and variable range
//    visibility.
//
//    Eric Brugger, Thu Jul 17 08:17:40 PDT 2003
//    Added maxSize argument to GetLegendSize.
//
//    Kathleen Bonnell, Tue Oct 25 11:13:14 PDT 2005 
//    Added virtual methods ChangeTitle, ChangeFontHeight.
// 
// ****************************************************************************

class PLOTTER_API avtLevelsLegend : public avtLegend
{
  public:
                    avtLevelsLegend();
    virtual        ~avtLevelsLegend();

    virtual void    GetLegendSize(float, float &, float &);

    void            SetColorBarVisibility(const int);
    void            SetRange(float min, float max);
    void            SetLevels(const std::vector<double> &);
    void            SetLevels(const std::vector<std::string> &);
    void            SetLookupTable(vtkLookupTable *);
    void            SetLabelColorMap(const LevelColorMap &);
    void            SetReverseOrder(const bool);
    void            SetVarRangeVisibility(const int);
    void            SetVarRange(float min, float max);


  protected:
    float                      min, max;
    int                        nLevels;

    vtkLookupTable            *lut;
    vtkVerticalScalarBarActor *sBar;

    int                        barVisibility;
    int                        rangeVisibility;

    virtual void               ChangePosition(float, float);
    virtual void               ChangeTitle(const char *);
    virtual void               ChangeFontHeight(float);
};


#endif


