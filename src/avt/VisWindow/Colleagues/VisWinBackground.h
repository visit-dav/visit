// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          VisWinBackground.h                               //
// ************************************************************************* //

#ifndef VIS_WIN_BACKGROUND_H
#define VIS_WIN_BACKGROUND_H
#include <viswindow_exports.h>

#include <VisWinColleague.h>

// Forward declarations.
class vtkBackgroundActor;
class vtkTexturedBackgroundActor;
class VisWindowColleagueProxy;

// ****************************************************************************
// Class: VisWinBackground
//
// Purpose:
//   This colleague displays a gradient background or an image background
//   for the Vis window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 28 10:08:09 PDT 2001
//
// Modifications:
//    Brad Whitlock, Wed Nov 14 15:23:23 PST 2007
//    Added background image support.
//
//    Brad Whitlock, Mon Feb 12 17:46:06 PST 2018
//    Added methods to set/get visibility.
//
// ****************************************************************************

class VISWINDOW_API VisWinBackground : public VisWinColleague
{
  public:
                              VisWinBackground(VisWindowColleagueProxy &);
    virtual                  ~VisWinBackground();

    virtual void              SetGradientBackgroundColors(int,
                                                    double, double, double,
                                                    double, double, double);
    virtual void              SetBackgroundMode(int);
    virtual void              SetBackgroundImage(const std::string &,int,int);
    virtual void              UpdatePlotList(std::vector<avtActor_p> &);

    void SetVisibility(int);
    int  GetVisibility() const;
  protected:
    static bool                 sphereModeError1;
    static bool                 sphereModeError2;

    vtkBackgroundActor         *bgActor;
    vtkTexturedBackgroundActor *textureActor;
    bool                        addedBackground;
    bool                        visible;

    void                        AddBackgroundToWindow(int);
    void                        RemoveBackgroundFromWindow();
};

#endif
