// ************************************************************************* //
//                              ViewerOperator.h                             //
// ************************************************************************* //

#ifndef VIEWER_OPERATOR_H
#define VIEWER_OPERATOR_H
#include <viewer_exports.h>

// Forward declarations
class AttributeSubject;
class ViewerPlot;
class ViewerOperatorPluginInfo;
class avtPluginFilter;


// ****************************************************************************
//  Class: ViewerOperator
//
//  Purpose:
//      ViewerOperator is an abstract base class from which the concrete
//      operators are derived.
//
//  Note:
//
//  Programmer: Eric Brugger
//  Creation:   September 15, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Dec 13 11:23:19 PST 2000
//    Made it also use a plot when creating an operator and store it for later.
//
//    Jeremy Meredith, Wed Jul 25 13:02:06 PDT 2001
//    Now a real class, not just an abstract base.  Needed for operator
//    plugins.
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
//    Brad Whitlock, Tue Oct 9 15:21:56 PST 2001
//    Added a method for handling tools.
//
//    Brad Whitlock, Mon Feb 11 14:15:53 PST 2002
//    Added a method for accessing operator attributes.
//
//    Brad Whitlock, Fri Feb 22 17:25:21 PST 2002
//    I removed the engine proxy argument from ExecuteEngineRPC.
//
//    Kathleen Bonnell, Wed Jun 5 13:34:53 PDT 2002 
//    Added method to retrieve the name of the operator. 
//
//    Eric Brugger, Wed Jan 15 15:44:16 PST 2003
//    Modify the plot data member so that it is not const.
//
//    Brad Whitlock, Thu Apr 10 10:58:58 PDT 2003
//    I added Removeable, Moveable, and AllowsSubsequentOperators.
//
//    Jeremy Meredith, Wed May 21 13:08:31 PDT 2003
//    Added NeedsRecalculation.
//
// ****************************************************************************

class VIEWER_API ViewerOperator
{
  public:
    ViewerOperator(const int type_,
                   ViewerOperatorPluginInfo *viewerPluginInfo_,
                   ViewerPlot *plot_);
    virtual ~ViewerOperator();

    bool ExecuteEngineRPC() const;
    int  GetType() const;

    void SetClientAttsFromOperator();
    void SetOperatorAttsFromClient();
    bool SetOperatorAtts(const AttributeSubject *toolAtts);
    const AttributeSubject *GetOperatorAtts() const;
    const char * GetName() const;

    bool Removeable() const;
    bool Moveable() const;
    bool AllowsSubsequentOperators() const;

    bool NeedsRecalculation() const;

  protected:
    ViewerPlot                   *plot;

  private:
    bool                          needsRecalculation;
    int                           type;
    ViewerOperatorPluginInfo     *viewerPluginInfo;
    AttributeSubject             *operatorAtts;
    avtPluginFilter              *avtfilter;

};

#endif
