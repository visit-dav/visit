/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
// ****************************************************************************
class DataNetwork
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
    void SetPipelineSpec(avtPipelineSpecification_p s) {pspec = s;};
    avtDataObject_p GetOutput(void) { return terminalNode->GetOutput(); };
    void SetDataSpec(avtDataSpecification_p s) {dspec = s;};
    void SetTime(int time_) {time = time_;};

    virtual void ReleaseData(void);

    avtDataSpecification_p& GetDataSpec() {return dspec;};
    avtPipelineSpecification_p GetPipelineSpec() {return pspec;};
    avtDataObjectWriter_p GetWriter(avtDataObject_p dob,
                                    avtPipelineSpecification_p pspec,
                                    WindowAttributes *atts);
    int GetNetID(void) const { return nid; };
    int GetWinID(void) const { return wid; };
    Netnode* GetTerminalNode(void) { return terminalNode; };
    virtual avtPlot_p GetPlot(void) { return plot; };
    avtActor_p GetActor(avtDataObject_p dob, bool force = false);
    bool ActorIsNull(void) { return (*plotActor == NULL); };
    void SetNetDB(NetnodeDB *d) { netdb = d; };
    NetnodeDB* GetNetDB(void) { return netdb; };
    virtual int GetTime() {return time;};
    std::string &GetPlottype(void) { return plottype; };
    std::string &GetPlotName(void) { return plotName; };
    std::vector<Netnode*>       &GetNodeList(void) { return nodeList; }; 

protected:
    int                         nid;
    int                         wid;
    Netnode                    *terminalNode;
    std::vector<Netnode*>       nodeList;
    avtDataSpecification_p      dspec;
    avtDataObjectWriter_p       writer;
    avtActor_p                  plotActor;
    double                      bgColor[3];
    double                      fgColor[3];
    avtPipelineSpecification_p  pspec;
    NetnodeDB*                  netdb;
    avtPlot_p                   plot;
    std::string                 plottype;
    std::string                 plotName;
    int                         time;
    bool                        clone;
};

#endif
