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
//  Method:  ClonedDataNetwork::ReleaseData
//
//  Purpose:
//    Releases unused data.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 18, 2004 
//
//  Modifications:
//
// ****************************************************************************
void
ClonedDataNetwork::ReleaseData(void)
{
    if (*cPlot != NULL)
    {
        cPlot->ReleaseData();
    }
}
