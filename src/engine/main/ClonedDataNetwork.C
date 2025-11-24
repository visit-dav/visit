// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ClonedDataNetwork.h>

#include <avtPlot.h>
#include <DebugStream.h>


// ****************************************************************************
//  Constructor:  ClonedDataNetwork::ClonedDataNetwork
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 18, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar 15 17:41:26 PST 2005
//    Initialize clone.
//
//    Brad Whitlock, Thu Jan 19 15:19:16 PST 2012
//    Initialize expressionNode.
//
// ****************************************************************************
ClonedDataNetwork::ClonedDataNetwork(DataNetwork *base) : DataNetwork()
{
    SetNetDB(base->GetNetDB());
    plot = base->GetPlot();
    plottype = base->GetPlottype();
    SetContract(base->GetContract());
    SetDataSpec(base->GetDataSpec());
    nodeList = base->GetNodeList();
    expressionNode = base->GetExpressionNode();
    clone = true;
}

// ****************************************************************************
//  Constructor:  ClonedDataNetwork::ClonedDataNetwork
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 18, 2004 
//
//  Modifications:
//
// ****************************************************************************
ClonedDataNetwork::~ClonedDataNetwork(void)
{
    ReleaseData();
}

// ****************************************************************************
//  Method:  ClonedDataNetwork::AddNode
//
//  Purpose:
//    Add a Netnode to the node list. This method intercepts any calls to
//    the parent class' AddNode method so that any Netnodes added after the
//    DataNetwork is cloned can be saved and then deleted in ReleaseData.
//
//  Programmer:  Eric Brugger
//  Creation:    November 24, 2025 
//
//  Modifications:
//
// ****************************************************************************
void
ClonedDataNetwork::AddNode(Netnode *n)
{
    cNodeList.push_back(n);
    DataNetwork::AddNode(n);
}

// ****************************************************************************
//  Method:  ClonedDataNetwork::ReleaseData
//
//  Purpose:
//    Releases unused data.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 18, 2004 
//
//  Modifications:
//    Eric Brugger, Mon Nov 24 10:17:20 PST 2025
//    I added code to delete cNodeList.
//
// ****************************************************************************
void
ClonedDataNetwork::ReleaseData(void)
{
    for (int i = 0; i < cNodeList.size(); ++i)
    {
        delete cNodeList[i];
    }
    if (*cPlot != NULL)
    {
        cPlot->ReleaseData();
    }
}
