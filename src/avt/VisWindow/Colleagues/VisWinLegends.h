// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              VisWinLegends.h                              //
// ************************************************************************* //

#ifndef VIS_WIN_LEGENDS_H
#define VIS_WIN_LEGENDS_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>

class     vtkVisItTextActor;

// ****************************************************************************
//  Class: VisWinLegends
//
//  Purpose:
//      Manages the legends for each plot -- their placement and whether they
//      should have annotation for cycles/times, etc.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Apr 11 12:11:13 PDT 2002
//    Added flag to turn off the main database.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    vtkScaledTextActor has been deprecated in favor of vtkTextActor. 
//    vtkTextActor defines its own mapper, so vtkTextMapper not needed. 
//
//    Eric Brugger, Mon Jul 14 16:27:28 PDT 2003
//    Changed the way database information is handled.
//
//    Eric Brugger, Wed Jul 16 09:47:15 PDT 2003
//    Removed GetPosition.
//
//    Kathleen Bonnell, Thu Nov 13 12:21:25 PST 2003 
//    Added bool return type to CreateDatabaseInfo. 
//
//    Brad Whitlock, Thu Mar 22 02:17:55 PDT 2007
//    Renamed PositionLegends to UpdateLegendInfo.
//
//    Cyrus Harrison, Sun Jun 17 21:42:53 PDT 2007
//    Added explicit pass of the database name to easily support path
//    expansion modes.
//
//    Brad Whitlock, Tue Jan 29 16:16:14 PST 2008
//    Added SetDatabaseInfoTextAttributes.
//
//    Brad Whitlock, Mon Mar  2 14:10:22 PST 2009
//    I added time scale and offset.
//
//    Kathleen Biagas, Wed Sep  7 16:22:39 PDT 2011
//    Added timeInfo to SetVisibility call.
//
//    Brad Whitlock, Mon Sep 19 16:06:46 PDT 2011
//    Switch to vtkVisItTextActor.
//
// ****************************************************************************

class VISWINDOW_API VisWinLegends : public VisWinColleague
{
  public:
                                  VisWinLegends(VisWindowColleagueProxy &);
    virtual                      ~VisWinLegends();

    virtual void                  SetForegroundColor(double, double, double);
    virtual void                  UpdatePlotList(std::vector<avtActor_p> &);

    void                          SetVisibility(bool db, 
                                                int path_exp_mode,
                                                bool legend,
                                                bool timeInfo);
    void                          SetDatabaseInfoTextAttributes(
                                      const VisWinTextAttributes &);
    void                          SetTimeScaleAndOffset(double,double);
  protected:
    vtkVisItTextActor            *dbInfoActor;
    bool                          dbInfoIsAdded;
    bool                          dbInfoVisible;
    bool                          timeVisible;
    VisWinTextAttributes          dbInfoTextAttributes;
    double                        dbInfoTimeScale;
    double                        dbInfoTimeOffset;

    int                           pathExpansionMode;
    bool                          legendVisible;
    bool                          homogeneous;

    static const double            leftColumnPosition;
    static const double            rightColumnPosition;
    static const double            dbInfoHeight;

    void                          UpdateLegendInfo(std::vector<avtActor_p> &);
    void                          UpdateDBInfo(std::vector<avtActor_p> &);

  private:
     bool                         CreateDatabaseInfo(char *,
                                                     const std::string &,
                                                     avtDataAttributes &);
};


#endif


