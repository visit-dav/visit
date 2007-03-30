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
// ****************************************************************************

class VISWINDOW_API VisWinInteractions : public VisWinColleague
{
  public:                    
                             VisWinInteractions(VisWindowColleagueProxy &,
                                                VisWindowInteractorProxy &);
    virtual                 ~VisWinInteractions();  
   
    void                     SetInteractionMode(INTERACTION_MODE);
    INTERACTION_MODE         GetInteractionMode() const;
    void                     SetBoundingBoxMode(bool);
    bool                     GetBoundingBoxMode() const;
    void                     SetSpinMode(bool);
    bool                     GetSpinMode() const;

    virtual void             Start2DMode();
    virtual void             Stop2DMode();
    virtual void             Start3DMode();
    virtual void             Stop3DMode();
    virtual void             StartCurveMode();
    virtual void             StopCurveMode();

    virtual void             NoPlots();
    virtual void             HasPlots();

  protected:
    INTERACTION_MODE         mode;
    bool                     bboxMode;
    bool                     spinMode;
    VisitHotPointInteractor *hotPointInteractor;
};


#endif


