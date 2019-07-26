// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               VisWinTriad.h                               //
// ************************************************************************* //

#ifndef VIS_WIN_TRIAD_H
#define VIS_WIN_TRIAD_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkTriad2D;

class VisWindowColleagueProxy;


// ****************************************************************************
//  Class: VisWinTriad
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It places
//      a triad in the lower left hand corner of the screen.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 11:01:24 PDT 2000
//    Added HasPlots/NoPlots and addedTriad.
//
//    Hank Childs, Wed Jul 12 09:12:02 PDT 2000
//    Removed axis actors in favor of new vtkTriad2D class.
//
//    Kathleen Bonnell, Wed Jun 27 14:56:09 PDT 2001 
//    Added SetVisibility. 
//
//    Alister Maguire, Thu Mar  1 16:08:42 PST 2018
//    Added SetLineWidth, SetFontFamily, SetBold, and SetItalic. 
//
// ****************************************************************************

class VISWINDOW_API VisWinTriad : public VisWinColleague
{
  public:
                              VisWinTriad(VisWindowColleagueProxy &);
    virtual                  ~VisWinTriad();

    virtual void              SetForegroundColor(double, double, double);
    virtual void              SetLineWidth(float, float, float);
    virtual void              SetFontFamily(int);
    virtual void              SetBold(bool);
    virtual void              SetItalic(bool);

    virtual void              Start3DMode(void);
    virtual void              Stop3DMode(void);

    virtual void              HasPlots(void);
    virtual void              NoPlots(void);

    void                      SetVisibility(bool);

  protected:
    vtkTriad2D               *triad;

    bool                      addedTriad;

    void                      AddTriadToWindow(void);
    void                      RemoveTriadFromWindow(void);
    bool                      ShouldAddTriad(void);
};


#endif


