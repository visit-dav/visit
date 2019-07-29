// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef NETWORK_H
#define NETWORK_H

#include <avtSILRestriction.h>
#include <avtDataObject.h>
#include <avtDataObjectWriter.h>
#include <avtActor.h>
#include <avtPlot.h>
#include <Netnodes.h>
#include <string>
#include <vector>
#include <deque>
#include <engine_main_exports.h>

// ****************************************************************************
//  Class: DataNetwork
//
//  Purpose:
//      Stores a single AVT network.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 29, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Jan 17 10:26:34 PST 2001
//    Changed networks to be a database followed by a series of filters
//    followed by a plot.
//
//    Hank Childs, Tue Jun 12 14:42:09 PDT 2001
//    Changed type of filter to be the more arbitrary avtFilter.  Also added
//    a field for pipeline specifications.
//
//    Jeremy Meredith, Thu Jul 26 03:37:47 PDT 2001
//    Made networks use avtPluginFilters.
//
//    Jeremy Meredith, Fri Nov  9 10:22:05 PST 2001
//    Added a data specification, a plot type, and a network id.
//    Added a constructor and a method to release the used data.
//
//    Sean Ahern, Tue May 28 09:49:02 PDT 2002
//    Made this a tree.
//
//    Sean Ahern, Mon Dec 23 13:09:50 PST 2002
//    Okay, so it's not a tree.
//
//    Kathleen Bonnell, Wed Mar 31 16:53:03 PST 2004
//    Made some methods virtual, so this class can be inherited from. 
//
//    Mark C. Miller, Thu May 27 11:05:15 PDT 2004
//    Removed window attributes arg from GetActor method
//
//    Kathleen Bonnell, Thu Oct 21 15:55:46 PDT 2004 
//    Added ActorIsNull method. 
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Changed id to nid
//    Added wid for window id 
//    Added methods to set/get window id
//
//    Kathleen Bonnell, Tue Mar 15 17:41:26 PST 2005 
//    Added clone.
//
//    Brad Whitlock, Wed Mar 21 23:02:05 PST 2007
//    Added plotName.
//
//    Gunther H. Weber, Thu Apr 12 11:02:44 PDT 2007
//    Added AddFilterNodeAfterExpressionEveluator that adds node directly
//    after expression evaluator
//
//    Kathleen Bonnell, Fri Sep 28 08:34:36 PDT 2007
//    Added optional bool arg to GetActor to force rexecution. 
//
//    Hank Childs, Fri Jan 16 14:37:33 PST 2009
//    Add a variable field.
//
//    Brad Whitlock, Mon Aug 22 10:36:08 PDT 2011
//    I added a selectionName field.
//
//    Brad Whitlock, Tue Jan 10 14:38:24 PST 2012
//    Set/get the expression node.
//
// ****************************************************************************

class ENGINE_MAIN_API DataNetwork
{
public:
             DataNetwork(void);
    virtual ~DataNetwork(void);

    void SetNetID(int _id) { nid = _id; };
    void SetWinID(int _id) { wid = _id; };
    void SetPlottype(const std::string &type) { plottype = type; };
    void SetPlotName(const std::string &n) { plotName = n; };
    virtual void SetPlot(avtPlot *_plot) { plot = _plot; };
    void SetTerminalNode(Netnode* t) {terminalNode = t;};
    void AddNode(Netnode *n) { nodeList.push_back(n); };
    
    void AddFilterNodeAfterExpressionEvaluator(NetnodeFilter *f);
    void SetContract(avtContract_p s) {contract = s;};
    avtDataObject_p GetOutput(void) { return terminalNode->GetOutput(); };
    void SetDataSpec(avtDataRequest_p s) {dataRequest = s;};
    void SetTime(int time_) {time = time_;};
    void SetVariable(const std::string &v) {var = v;};
    void SetSelectionName(const std::string &s) { selectionName = s; }

    virtual void ReleaseData(void);

    avtDataRequest_p& GetDataSpec() {return dataRequest;};
    avtContract_p GetContract() {return contract;};
    avtDataObjectWriter_p GetWriter(avtDataObject_p dob,
                                    avtContract_p contract,
                                    WindowAttributes *atts);
    int GetNetID(void) const { return nid; };
    int GetWinID(void) const { return wid; };
    Netnode* GetTerminalNode(void) { return terminalNode; };
    void SetExpressionNode(Netnode *f);
    Netnode *GetExpressionNode() const;

    virtual avtPlot_p GetPlot(void) { return plot; };
    avtActor_p GetActor(avtDataObject_p dob, bool force = false);
    bool ActorIsNull(void) { return (*plotActor == NULL); };
    void SetNetDB(NetnodeDB *d) { netdb = d; };
    NetnodeDB* GetNetDB(void) { return netdb; };
    virtual int GetTime() {return time;};
    const std::string &GetVariable() {return var;};
    std::string &GetPlottype(void) { return plottype; };
    std::string &GetPlotName(void) { return plotName; };
    std::vector<Netnode*>       &GetNodeList(void) { return nodeList; }; 
    const std::string &GetSelectionName() const { return selectionName; }

protected:
    int                         nid;
    int                         wid;
    Netnode                    *terminalNode;
    Netnode                    *expressionNode;
    std::vector<Netnode*>       nodeList;
    avtDataRequest_p            dataRequest;
    avtDataObjectWriter_p       writer;
    avtActor_p                  plotActor;
    avtContract_p               contract;
    NetnodeDB*                  netdb;
    avtPlot_p                   plot;
    std::string                 plottype;
    std::string                 plotName;
    std::string                 var;
    int                         time;
    bool                        clone;
    std::string                 selectionName;
};

#endif
