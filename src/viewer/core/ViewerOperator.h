// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              ViewerOperator.h                             //
// ************************************************************************* //

#ifndef VIEWER_OPERATOR_H
#define VIEWER_OPERATOR_H
#include <viewercore_exports.h>
#include <ViewerBase.h>

// Forward declarations
class AttributeSubject;
class AttributeSubjectMap;
class DataNode;
class ViewerPlot;
class ViewerOperatorPluginInfo;
class avtDatabaseMetaData;
class ExpressionList;

#include <vector>
#include <string>

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
//    Brad Whitlock, Wed Jul 16 17:08:10 PST 2003
//    Added GetPluginID, CreateNode, SetFromNode.
//
//    Kathleen Bonnell, Thu Sep 11 11:37:51 PDT 2003
//    Added fromDefault arg to constructor.
//
//    Brad Whitlock, Fri Apr 2 11:04:16 PDT 2004
//    I added a copy constructor and a method to set the plot pointer
//    for the operator.
//
//    Brad Whitlock, Mon Feb 12 17:49:49 PST 2007
//    Added ViewerBase base class.
//
//    Jeremy Meredith, Tue Feb 19 14:23:39 EST 2008
//    Allow operators to create new variables.
//
//    Jeremy Meredith, Tue Feb 19 15:39:42 EST 2008
//    Allow operators to construct full expressions for their new variables.
//
//    Brad Whitlock, Tue Apr 29 15:14:33 PDT 2008
//    Added GetMenuName.
//
//    Hank Childs, Thu Dec 30 22:37:51 PST 2010
//    Change signature of GetCreatedVariables.
//
//    Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//    Add operator keyframing.
//
// ****************************************************************************

class VIEWERCORE_API ViewerOperator : public ViewerBase
{
  public:
    ViewerOperator(const int type_, 
                   ViewerOperatorPluginInfo *viewerPluginInfo_,
                   const bool keyframeMode_,
                   const int cacheIndex_, const int cacheSize_,
                   ViewerPlot *plot_, const bool fromDefault);
    ViewerOperator(const ViewerOperator &);
    virtual ~ViewerOperator();

    int  GetType() const;
    const char *GetPluginID() const;
    const char *GetName() const;
    const char *GetMenuName() const;

    void SetPlot(ViewerPlot *p);
    void SetClientAttsFromOperator();
    void SetOperatorAttsFromClient(const bool activePlot,
                                   const bool applyToAll);
    bool SetOperatorAtts(const AttributeSubject *toolAtts);
    const AttributeSubject *GetOperatorAtts() const;

    bool Removeable() const;
    bool Moveable() const;
    bool AllowsSubsequentOperators() const;

    bool NeedsRecalculation() const;
    bool ExecuteEngineRPC() const;

    ExpressionList *GetCreatedVariables(const avtDatabaseMetaData *md);

    void CreateNode(DataNode *);
    void SetFromNode(DataNode *, const std::string &);

    void UpdateOperatorAtts();
    std::string GetOperatorVarDescription();

    void SetKeyframeMode(const bool keyframeMode_);
    void SetCacheIndex(const int cacheIndex_);
    void UpdateCacheSize(const int cacheSize_);
    const int *GetKeyframeIndices(int &nIndices) const;
    void DeleteKeyframe(const int index);
    void MoveKeyframe(const int oldIndex, const int newIndex);

  protected:
    ViewerPlot                   *plot;
    
  private:
    bool                          needsRecalculation;
    int                           type;
    ViewerOperatorPluginInfo     *viewerPluginInfo;

    AttributeSubjectMap          *operatorAtts;
    AttributeSubject             *curOperatorAtts;

    bool                          keyframeMode;
    int                           cacheIndex;
    int                           cacheSize;
};

#endif
