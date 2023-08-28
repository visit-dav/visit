// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          VisWinInteractions.h                             //
// ************************************************************************* //

#ifndef VIS_WIN_INTERACTIONS_H
#define VIS_WIN_INTERACTIONS_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>
#include <VisWindowInteractorProxy.h>

#include <VisWindowTypes.h>


class VisWindowColleagueProxy;
class VisWindowInteractorProxy;
class VisitHotPointInteractor;

// ****************************************************************************
//  Class: VisWinInteractions
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It controls
//      the interactor to the plots.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//     Brad Whitlock, Tue Nov 7 16:11:20 PST 2000
//     I added a method to query the interaction mode.
//
//     Brad Whitlock, Thu Nov 9 15:50:02 PST 2000
//     I added a method to set the bbox mode.
//
//     Hank Childs, Tue Sep 18 10:41:30 PDT 2001
//     Make destructor virtual.
//
//     Brad Whitlock, Mon Oct 1 14:41:22 PST 2001
//     Replaced all of the interactors with a single VisitHotPointInteractor.
//
//     Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002
//     Add support for curve mode.
//
//     Brad Whitlock, Wed Sep 10 15:27:35 PST 2003
//     Added support for temporarily suspending spin mode.
//
//     Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//     Added new AxisArray window mode.
//
//     Eric Brugger, Tue Dec  9 14:17:09 PST 2008
//     Added the ParallelAxes window mode.
//
//     Jeremy Meredith, Tue Feb  2 13:43:43 EST 2010
//     Added a new setting for when tools should update.
//
//     Hank Childs, Sat Mar 13 18:07:25 PST 2010
//     Add "auto" setting to bounding box mode.
//
//     Eric Brugger, Thu Oct 27 13:37:23 PDT 2011
//     Added the ability to enable/disable interaction mode changes to
//     support adding a multi resolution dispaly capability for AMR data.
//
//     Kathleen Biagas, Wed Aug 17, 2022
//     Added SetOsprayRendering.
//
// ****************************************************************************

class VISWINDOW_API VisWinInteractions : public VisWinColleague
{
  public:
                             VisWinInteractions(VisWindowColleagueProxy &,
                                                VisWindowInteractorProxy &);
    virtual                 ~VisWinInteractions();

    void                     SetInteractionMode(INTERACTION_MODE);
    INTERACTION_MODE         GetInteractionMode() const;
    void                     SetBoundingBoxMode(int);
    int                      GetBoundingBoxMode() const;
    void                     SetSpinMode(bool);
    bool                     GetSpinMode() const;
    void                     SetSpinModeSuspended(bool);
    bool                     GetSpinModeSuspended() const;
    void                     SetToolUpdateMode(TOOLUPDATE_MODE);
    TOOLUPDATE_MODE          GetToolUpdateMode() const;
    void                     SetEnableInteractionModeChanges(bool);
    bool                     GetEnableInteractionModeChanges() const;

    virtual void             Start2DMode();
    virtual void             Stop2DMode();
    virtual void             Start3DMode();
    virtual void             Stop3DMode();
    virtual void             StartCurveMode();
    virtual void             StopCurveMode();
    virtual void             StartAxisArrayMode();
    virtual void             StopAxisArrayMode();
    virtual void             StartParallelAxesMode();
    virtual void             StopParallelAxesMode();

    virtual void             NoPlots();
    virtual void             HasPlots();

    void                     SetOsprayRendering(bool enabled);

    VisitHotPointInteractor* GetHotPointInteractor() { return hotPointInteractor; }
  protected:
    INTERACTION_MODE         mode;
    TOOLUPDATE_MODE          toolUpdateMode;
    int                      bboxMode;
    bool                     spinMode;
    bool                     spinModeSuspended;
    bool                     enableInteractionModeChanges;
    VisitHotPointInteractor *hotPointInteractor;
};


#endif


