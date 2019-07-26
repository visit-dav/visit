// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

class  vtkVisItScalarBarActor;
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
//    Brad Whitlock, Wed Mar 21 21:39:27 PST 2007
//    Added some new methods for setting legend properties.
//
//    Dave Bremer, Mon Oct 13 12:36:09 PDT 2008
//    Added SetNumberFormat()
//
//    Hank Childs, Fri Jan 23 15:43:01 PST 2009
//    Added min/max visibility.
//
//    Kathleen Bonnell, Thu Oct  1 14:18:11 PDT 2009
//    Added methods and ivars that allow more user control of tick marks
//    and tick labels.
//
// ****************************************************************************

class PLOTTER_API avtLevelsLegend : public avtLegend
{
  public:
                    avtLevelsLegend();
    virtual        ~avtLevelsLegend();

    virtual void    GetLegendSize(double, double &, double &);

    virtual void    SetTitleVisibility(bool);
    virtual bool    GetTitleVisibility() const;
    virtual void    SetLabelVisibility(int);
    virtual int     GetLabelVisibility() const;
    virtual void    SetMinMaxVisibility(bool);
    virtual bool    GetMinMaxVisibility() const;

    virtual void    SetLegendScale(double xScale, double yScale);
    virtual void    SetBoundingBoxVisibility(bool);
    virtual void    SetBoundingBoxColor(const double *);
    virtual void    SetOrientation(LegendOrientation);
    virtual void    SetFont(int family, bool bold, bool italic, bool shadow);
    virtual void    SetNumberFormat(const char *);

    void            SetColorBarVisibility(const bool);
    void            SetRange(double min, double max);
    void            SetLevels(const std::vector<double> &);
    void            SetLevels(const std::vector<std::string> &);
    void            SetLookupTable(vtkLookupTable *);
    void            SetLabelColorMap(const LevelColorMap &);
    void            SetReverseOrder(const bool);
    void            SetVarRangeVisibility(const bool);
    void            SetVarRange(double min, double max);

    virtual void    SetUseSuppliedLabels(bool);
    virtual bool    GetUseSuppliedLabels(void); 
    virtual void    SetSuppliedLabels(const stringVector &);
    virtual void    GetCalculatedLabels(stringVector &);
    virtual int     GetType(void) { return 1; }


  protected:
    double                     min, max;
    int                        nLevels;
    double                     scale[2];
    double                     maxScale;
    bool                       setMaxScale;

    vtkLookupTable            *lut;
    vtkVisItScalarBarActor    *sBar;

    bool                       barVisibility;
    bool                       rangeVisibility;
    bool                       titleVisibility;
    int                        labelVisibility;
    bool                       minmaxVisibility;

    virtual void               ChangePosition(double, double);
    virtual void               ChangeTitle(const char *);
    virtual void               ChangeFontHeight(double);
};


#endif


