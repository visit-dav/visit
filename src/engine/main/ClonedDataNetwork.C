#include <ClonedDataNetwork.h>

#include <avtPlot.h>
#include <DebugStream.h>


// ****************************************************************************
//  Constructor:  ClonedDataNetwork::ClonedDataNetwork
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 18, 2004 
//
// ****************************************************************************
ClonedDataNetwork::ClonedDataNetwork(DataNetwork *base) : DataNetwork()
{
    SetNetDB(base->GetNetDB());
    plot = base->GetPlot();
    plottype = base->GetPlottype();
    SetPipelineSpec(base->GetPipelineSpec());
    SetDataSpec(base->GetDataSpec());
    nodeList = base->GetNodeList();
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
