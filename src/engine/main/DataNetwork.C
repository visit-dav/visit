#include <DataNetwork.h>

#include <avtActor.h>
#include <avtPlot.h>
#include <DebugStream.h>
#include <WindowAttributes.h>

using std::string;
using std::vector;
using std::deque;

// ****************************************************************************
//  Constructor:  DataNetwork::DataNetwork
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  9, 2001
//
//  Modifications:
//
//    Mark C. Miller, Thu May 27 11:05:15 PDT 2004
//    Added missing initialization for plotActor data member
// ****************************************************************************
DataNetwork::DataNetwork(void)
{
    id = -1;
    pspec = NULL;
    dspec = NULL;
    writer = NULL;
    plotActor = NULL;
}

// ****************************************************************************
//  Constructor:  DataNetwork::DataNetwork
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  9, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jul 23 11:04:30 PDT 2002
//    Reflect that the terminal node has already been deleted by the time we
//    get here.
//
//    Hank Childs, Sun Mar  7 16:03:33 PST 2004
//    Do not clear the nodes before releasing the data -- we would then not
//    release the data in those nodes.
//
// ****************************************************************************
DataNetwork::~DataNetwork(void)
{
    // The terminal node has already been freed by this point.
    terminalNode = NULL;

    ReleaseData();
}

// ****************************************************************************
//  Method:  DataNetwork::GetWriter
//
//  Purpose: Cache data object writer for network output
//
//  Programmer:  Mark C. Miller
//  Creation:    May 5, 2003 
//
// ****************************************************************************
avtDataObjectWriter_p
DataNetwork::GetWriter(avtDataObject_p dob, avtPipelineSpecification_p pspec,
                          WindowAttributes *atts)
{

   if (*writer != NULL)
   {
      return writer;
   }
   else
   {
      avtDataObjectWriter_p tmpWriter = GetPlot()->Execute(dob, pspec, atts);
      if (GetPlot()->CanCacheWriterExternally())
         writer = tmpWriter;
      return tmpWriter;
   }
}

// ****************************************************************************
//  Method:  DataNetwork::GetActor
//
//  Purpose: Cache data actor for plot
//
//  Programmer:  Mark C. Miller
//  Creation:    December 5, 2003 
//
//  Modifications:
//
//     Mark C. Miller, Thu May 27 11:05:15 PDT 2004
//     Removed window attributes argument and dependencies
//
// ****************************************************************************
avtActor_p
DataNetwork::GetActor(avtDataObject_p dob)
{
    if (*plotActor == NULL)
    {
        // do the part of the execute we'd do in the viewer
        plotActor = GetPlot()->Execute(NULL, dob);
   }

   return plotActor;

}


// ****************************************************************************
//  Method:  DataNetwork::ReleaseData
//
//  Purpose:
//    Releases unused data.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 19, 2001
//
//  Modifications:
//      Sean Ahern, Mon May 20 16:28:39 PDT 2002
//      Retrofitted.
//
//      Sean Ahern, Wed May 22 14:51:34 PDT 2002
//      Added a check to see if the database should be cleaned.
//
//      Hank Childs, Tue Jul 23 11:04:30 PDT 2002
//      Make sure terminalNode is not NULL before calling a method on it.
//
//      Mark C. Miller, 30Apr03 
//      Make sure plot is not NULL before calling a method on it.
//
// ****************************************************************************
void
DataNetwork::ReleaseData(void)
{
    debug1 << "Releasing all data associated with network " << id << endl;
    int nNodes = nodeList.size();
    for (int i = 0 ; i < nNodes ; i++)
    {
        nodeList[i]->ReleaseData();
    }
    if (terminalNode != NULL)
    {
        terminalNode->ReleaseData();
    }
    if (*plotActor != NULL)
       plotActor = (avtActor *) 0;
    if (*plot != NULL)
    {
        plot->ReleaseData();
    }
    if (*writer)
    {
       writer->GetInput()->ReleaseData();
    }
}
