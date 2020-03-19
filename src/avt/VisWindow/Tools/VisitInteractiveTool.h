// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_INTERACTIVE_TOOL_H
#define VISIT_INTERACTIVE_TOOL_H
#include <viswindow_exports.h>
#include <vector>
#include <VisWindowTypes.h>
#include <VisWindowToolProxy.h>
#include <avtToolInterface.h>

// A vector of HotPoints.
typedef std::vector<HotPoint> HotPointVector;

// ****************************************************************************
// Class: VisitInteractiveTool
//
// Purpose: 
//   This is the base class for interactive tools.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 12:17:29 PDT 2001
//
// Modifications:
//   Jeremy Meredith, Tue Oct  9 23:55:02 PDT 2001
//   Added some functions for point transforms.
//
//   Brad Whitlock, Mon Feb 11 14:42:15 PST 2002
//   Added a pure virtual method for returning the tool interface.
//
//   Brad Whitlock, Wed Jun 18 15:36:34 PST 2002
//   Added NoPlots method.
//
//   Brad Whitlock, Wed Oct 30 09:22:33 PDT 2002
//   Moved FacingAxis from VisitLineTool to here. Added
//   ComputeDepthTranslationDistance.
//
//   Kathleen Bonnell, Tue Nov  5 08:32:59 PST 2002
//   Added virtual methods TurnLightingOn/Off. 
//
//   Kathleen Bonnell, Wed May 28 16:14:22 PDT 2003 
//   Added method ReAddToWindow.
//
//   Kathleen Bonnell, Fri Jun  6 15:36:24 PDT 2003 
//   Added FullFrameOn/Off methods.
//
//   Mark Blair, Wed Sep 13 12:18:53 PDT 2006
//   Added ShowsHotPointHighlights method.
//
//   Jeremy Meredith, Fri Feb  1 18:10:41 EST 2008
//   Added virtual UpdatePlotList.
//
//   Jeremy Meredith, Tue Feb  2 13:16:55 EST 2010
//   Pulled CallCallback up from individual tools into a pure virtual function
//   here.  It already existed in every tool, and it is now required to
//   be visible to get the "OnToolClose" tool update mode to work correctly.
//
//   Jeremy Meredith, Wed May 19 11:00:51 EDT 2010
//   Added support for 3D axis scaling.
//
//   Burlen Loring, Thu Oct  8 13:45:14 PDT 2015
//   Fix a compiler warning
//
// ****************************************************************************

class VISWINDOW_API VisitInteractiveTool
{
  public:
                          VisitInteractiveTool(VisWindowToolProxy &p) : proxy(p)
                             { lastX = lastY = -1; enabled = false; };
    virtual              ~VisitInteractiveTool()
                             { ; };

    virtual void          Enable()  { enabled = true; };
    virtual void          Disable() { enabled = false; };
    bool                  IsEnabled() const { return enabled; };
    virtual bool          IsAvailable() const { return true; };
    virtual bool          ShowsHotPointHighlights() const { return true; };

    virtual void          Start2DMode() {; };
    virtual void          Stop2DMode() {; };
    virtual void          Start3DMode() {; };
    virtual void          Stop3DMode() {; };
    virtual void          NoPlots() {; };
    virtual void          UpdatePlotList(std::vector<avtActor_p> &) { }

    virtual void          SetBackgroundColor(double, double, double) {; };
    virtual void          SetForegroundColor(double, double, double) {; };

    virtual const char *  GetName() const = 0;
    virtual avtToolInterface &GetInterface() = 0;
    virtual void          UpdateTool() = 0;

    virtual void          UpdateView() {; };

    const HotPointVector &HotPoints() const { return hotPoints; };

    void                  SetLastLocation(int x, int y)
                             { lastX = x; lastY = y; };

    virtual void          TurnLightingOn()  {; }; 
    virtual void          TurnLightingOff() {; }; 
    virtual void          ReAddToWindow() {; }; 

    virtual void          FullFrameOn(const double, const int)  {; }; 
    virtual void          FullFrameOff() {; }; 

    virtual void          Set3DAxisScalingFactors(bool, const double[3]) { }

    virtual void          CallCallback() = 0;

  protected:
    void                  ComputeDisplayToWorld(double x, double y, double z,
                                                double *worldPt);
    void                  ComputeWorldToDisplay(double x, double y, double z,
                                                double *displayPt);
    avtVector             ComputeDisplayToWorld(const avtVector &v);
    avtVector             ComputeWorldToDisplay(const avtVector &v);
    avtVector             DisplayToUnit(const avtVector&);
    int                   FacingAxis();
    avtVector             ComputeDepthTranslationDistance();

    VisWindowToolProxy   &proxy;
    bool                  enabled;
    HotPointVector        hotPoints;
    int                   lastX;
    int                   lastY;
};

#endif
