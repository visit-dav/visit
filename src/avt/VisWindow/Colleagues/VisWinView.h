// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               VisWinView.h                                //
// ************************************************************************* //

#ifndef VIS_WIN_VIEW_H
#define VIS_WIN_VIEW_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


// ****************************************************************************
//  Class: VisWinView
//
//  Purpose:
//      Handles the view information and resetting of cameras.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Nov 10 10:19:14 PST 2000
//    Added code to support turning perspective projections on and off.
//
//    Hank Childs, Fri Jan  5 17:55:42 PST 2001
//    Removed antiquated method SetCameraToView.
//
//    Eric Brugger, Mon Mar 12 14:48:34 PST 2001
//    I removed the Get/SetPerspectiveProjection methods.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002
//    Added support for curve mode.
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added new AxisArray window mode.
//
//    Eric Brugger, Tue Dec  9 14:26:32 PST 2008
//    Added the ParallelAxes window mode.
//
//    Kathleen Biagas, Thu Aug  2 13:03:41 MST 2018
//    Removed ResetView, an ancient code path.
//
//    Kathleen Biagas, Wed Aug 17, 2022
//    Added SetOsprayRendering.
//
//    Kevin Griffin, Thu 26 Oct 2023 09:51:22 AM PDT
//    Added SetAnariRendering.
//
// ****************************************************************************

class VISWINDOW_API VisWinView : public VisWinColleague
{
  public:
                         VisWinView(VisWindowColleagueProxy &);
    virtual             ~VisWinView() {;}

    const avtViewInfo   &GetViewInfo(void);
    void                 SetViewInfo(const avtViewInfo &);

    virtual void         Start2DMode(void);
    virtual void         Stop2DMode(void);

    virtual void         StartCurveMode(void);
    virtual void         StopCurveMode(void);

    virtual void         StartAxisArrayMode(void);
    virtual void         StopAxisArrayMode(void);

    virtual void         StartParallelAxesMode(void);
    virtual void         StopParallelAxesMode(void);

    void                 SetOsprayRendering(bool enabled)
                             { viewInfo.useOSPRay = enabled; }

    void                 SetAnariRendering(const bool enabled)
                             { viewInfo.useAnari = enabled; }

  protected:
    avtViewInfo          viewInfo;
};


#endif
