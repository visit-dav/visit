// ************************************************************************* //
//                          VisWinBackground.h                               //
// ************************************************************************* //

#ifndef VIS_WIN_BACKGROUND_H
#define VIS_WIN_BACKGROUND_H
#include <viswindow_exports.h>

#include <VisWinColleague.h>

// Forward declarations.
class vtkBackgroundActor;
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

  protected:
    vtkBackgroundActor        *bgActor;

    void                      AddBackgroundToWindow(void);
    void                      RemoveBackgroundFromWindow(void);

    bool                      addedBackground;
};

#endif
