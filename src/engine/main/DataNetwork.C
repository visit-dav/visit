#include <DataNetwork.h>

#include <avtPlot.h>
#include <DebugStream.h>

using std::string;
using std::vector;
using std::deque;

// ****************************************************************************
//  Constructor:  DataNetwork::DataNetwork
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  9, 2001
//
// ****************************************************************************
DataNetwork::DataNetwork(void)
{
    id = -1;
    pspec = NULL;
    dspec = NULL;
    writer = NULL;
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
// ****************************************************************************
DataNetwork::~DataNetwork(void)
{
    // The terminal node has already been freed by this point.
    terminalNode = NULL;
    nodeList.clear();

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
      writer = GetPlot()->Execute(dob, pspec, atts);
      return writer;
   }
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
    if (*plot != NULL)
    {
        plot->ReleaseData();
    }
}
