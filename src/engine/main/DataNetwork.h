#ifndef NETWORK_H
#define NETWORK_H

#include <avtSILRestriction.h>
#include <avtDataObject.h>
#include <avtDataObjectWriter.h>
#include <avtPlot.h>
#include <Netnodes.h>
#include <string>
#include <vector>
#include <deque>

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
// ****************************************************************************
class DataNetwork
{
public:
    DataNetwork(void);
    ~DataNetwork(void);

    void SetID(int _id) { id = _id; };
    void SetPlottype(const std::string &type) { plottype = type; };
    void SetPlot(avtPlot *_plot) { plot = _plot; };
    void SetTerminalNode(Netnode* t) {terminalNode = t;};
    void AddNode(Netnode *n) { nodeList.push_back(n); };
    void SetPipelineSpec(avtPipelineSpecification_p s) {pspec = s;};
    avtDataObject_p GetOutput(void) { return terminalNode->GetOutput(); };
    void SetDataSpec(avtDataSpecification_p s) {dspec = s;};
    void SetSIL(avtSILRestriction_p &silr_) {silr = silr_;};
    void SetTime(int time_) {time = time_;};

    void ReleaseData(void);

    avtDataSpecification_p& GetDataSpec() {return dspec;};
    avtPipelineSpecification_p GetPipelineSpec() {return pspec;};
    avtDataObjectWriter_p GetWriter(avtDataObject_p dob,
                                    avtPipelineSpecification_p pspec,
                                    WindowAttributes *atts);
    int GetID(void) { return id; };
    Netnode* GetTerminalNode(void) { return terminalNode; };
    avtPlot_p GetPlot(void) { return plot; };
    void SetNetDB(NetnodeDB *d) { netdb = d; };
    NetnodeDB* GetNetDB(void) { return netdb; };
    avtSILRestriction_p GetSIL() {return silr;};
    int GetTime() {return time;};

protected:
    int                         id;
    Netnode                    *terminalNode;
    std::vector<Netnode*>       nodeList;
    avtDataSpecification_p      dspec;
    avtDataObjectWriter_p       writer;
    avtPipelineSpecification_p  pspec;
    NetnodeDB*                  netdb;
    avtPlot_p                   plot;
    std::string                 plottype;
    avtSILRestriction_p         silr;
    int                         time;
};

#endif
