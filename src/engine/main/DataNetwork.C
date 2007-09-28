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

#include <DataNetwork.h>

#include <avtActor.h>
#include <avtPlot.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
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
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id (wid)
//
//    Kathleen Bonnell, Tue Mar 15 17:41:26 PST 2005 
//    Initialize clone.
//
//    Brad Whitlock, Wed Mar 21 23:03:25 PST 2007
//    Initialize plotName.
//
// ****************************************************************************
DataNetwork::DataNetwork(void)
{
    nid = -1;
    wid = -1;
    pspec = NULL;
    dspec = NULL;
    writer = NULL;
    plotActor = NULL;
    clone = false;
    plotName = "";
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
//    Kathleen Bonnell, Tue Mar 15 17:41:26 PST 2005 
//    Don't release data if this network is a clone. 
//    
//    Hank Childs, Tue Mar 22 10:08:11 PST 2005
//    Fix memory leak.
//
// ****************************************************************************
DataNetwork::~DataNetwork(void)
{
    // The terminal node has already been freed by this point.
    terminalNode = NULL;

    if (!clone)
    {
        ReleaseData();
        for (int i = 0 ; i < nodeList.size() ; i++)
        {
            delete nodeList[i];
        }
    }
}

// ****************************************************************************
//  Method:  DataNetwork::AddFilterNodeAfterExpressionEvaluator()
//
//  Purpose: Add a filter at the beginning of the pipeline (after expression
//           evaluator.
//
//  Programmer:  Gunther H. Weber
//  Creation:    Apr 12, 2007
//
// ****************************************************************************
void DataNetwork::AddFilterNodeAfterExpressionEvaluator(NetnodeFilter *f)
{
    std::vector<Netnode*>::iterator it = nodeList.begin();
    ++it;
    if (it != nodeList.end())
    {
	// There is another Netnode ...
	NetnodeFilter *f2 = dynamic_cast<NetnodeFilter*>(*it);
	if (f2)
	{
	    // ... and it is a filter
	    // -> Insert out filter and connect inputs and outputs
	    f->GetInputNodes() = f2->GetInputNodes();
	    f2->GetInputNodes().clear();
	    f2->GetInputNodes().push_back(f);
	    nodeList.insert(it, f);
	}
	else
	{
	    // ... it is not a filter
	    debug1 << "DataNetwork::AddFilterNodeAfterExpressionEvaluator(NetnodeFilter *f): Subsequent Netnode is not a filter!" << endl;
	}
    }
    else
    {
	// There is no other filter
	// -> Add our filter as last Netnode and conncect inputs
	f->GetInputNodes().push_back(*nodeList.begin());
	nodeList.push_back(f);
    }
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
//     Kathleen Bonnell,  Thu Oct 21 15:55:46 PDT 2004
//     Allow dob arg to be NULL, but test for it and throw Exception when
//     necessary. 
//
//     Kathleen Bonnell, Fri Sep 28 08:34:36 PDT 2007
//     Added option bool arg to force rexecution. 
//
// ****************************************************************************
avtActor_p
DataNetwork::GetActor(avtDataObject_p dob, bool force)
{
    if (*plotActor == NULL || force) 
    {
        if (*dob != NULL)
        {
            // do the part of the execute we'd do in the viewer
            if (*plotActor != NULL)
                plotActor = (avtActor *) 0;
                
            plotActor = GetPlot()->Execute(NULL, dob);
        }
        else 
        {
            debug1 << "Attempting to retrieve a plot's actor with "
                   << "no input." << endl;
            EXCEPTION0(ImproperUseException);
        }
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
//      Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//      Changed id to nid 
//
// ****************************************************************************
void
DataNetwork::ReleaseData(void)
{
    debug1 << "Releasing all data associated with network " << nid << endl;
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
