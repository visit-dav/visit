/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                         avtSIMODataTreeIterator.C                         //
// ************************************************************************* //

#include <avtSIMODataTreeIterator.h>

#include <avtCommonDataFunctions.h>
#include <avtDataRepresentation.h>
#include <avtDataTree.h>
#include <avtExecutionManager.h>
#include <avtExtents.h>

#include <ImproperUseException.h>
#include <DebugStream.h>

#include <string>


// ****************************************************************************
//  Method: avtSIMODataTreeIterator constructor
//
//  Programmer: Hank Childs
//  Creation:   June 20, 2001
//
// ****************************************************************************

avtSIMODataTreeIterator::avtSIMODataTreeIterator()
{
    currentNode = 0;
    totalNodes  = 0;
    overrideOriginalSpatialExtents = false;
    overrideOriginalDataExtents    = false;
}


// ****************************************************************************
//  Method: avtSIMODataTreeIterator destructor
//
//  Programmer: Hank Childs
//  Creation:   November 6, 2001
//
// ****************************************************************************

avtSIMODataTreeIterator::~avtSIMODataTreeIterator()
{
}


// ****************************************************************************
//  Method: avtSIMODataTreeIterator::FinishExecute
//
//  Purpose:
//      If in threaded mode, we need to wait until all work is done.
//
//  Programmer: David Camp
//  Creation:   March 11, 2013
//
// ****************************************************************************

void
avtSIMODataTreeIterator::FinishExecute(void)
{
    // Wait for all work in the queue to finish.
    avtExecutionManagerFinishWork();
}


// ****************************************************************************
//  Method: avtSIMODataTreeIterator::Execute
//
//  Purpose:
//      Defines the pure virtual function execute.  This does the work of
//      taking the input dataset and disabling the upstream pipeline to it.
//      It then sets the output and calls the pure virtual function
//      ExecuteDataTree.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jun  4 13:46:00 PDT 2001
//    Stop using domain's argument since everything should be driven by the
//    input data object anyway.  Re-wrote function and blew away previous
//    comments.
//
//    Hank Childs, Wed Jun 20 18:11:15 PDT 2001
//    Add support for updating progress.
//
//    Hank Childs, Wed Oct 24 14:21:18 PDT 2001
//    Moved PreExecute and PostExecute calls to avtFilter.
//
//    Eric Brugger, Mon Nov  5 13:38:23 PST 2001
//    Modified to always compile the timing code.
//
//    Hank Childs, Mon Feb 28 10:39:28 PST 2005
//    Moved timings code into base class avtFilter.
//
//    Hank Childs, Wed May  9 16:57:25 PDT 2007
//    Make sure that output tree is not NULL, since that causes heartache.
//
//    Hank Childs, Tue Apr 10 17:26:52 PDT 2012
//    Check for NULL input trees.
//
//    David Camp, Thu May 23 12:52:53 PDT 2013
//    Changed Execute into two parts for threaded mode. We execute all data
//    blocks and then wait in FinishExecute to complete work.
//
// ****************************************************************************

void
avtSIMODataTreeIterator::Execute(void)
{
    //
    // This will walk through the data domains in a data tree.
    //
    avtDataTree_p tree    = GetInputDataTree();
    avtDataTree_p newTree;
    if (*tree != NULL)
    {
        totalNodes = tree->GetNumberOfLeaves();
        Execute(tree, newTree);

        // If in threaded mode wait until Execute has completed.
        FinishExecute();
    }
    else
    {
        // This can happen when a filter serves up an empty data tree.
        // It can also happen when we claim memory from intermediate
        // data objects and then go back to execute them.
        debug1 << "Unusual situation: NULL input tree to SIMO iterator.  Likely "
               << "that an exception occurred previously." << endl;
    }

    if (*newTree == NULL)
    {
        //
        // Lots of code assumes that the root tree is non-NULL.  Put a dummy
        // tree in its place.
        //
        newTree = new avtDataTree();
    }

    SetOutputDataTree(newTree);
}


// ****************************************************************************
//  Method: avtSIMODataTreeIterator::ExecuteDataTree
//
//  Purpose:
//      This is a default ExecuteDataTree method for filters that have not
//      been converted to use an avtDataRepresentation. Once all the filters
//      have been converted to work with avtDataRepresentations this method
//      can be removed.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Jul 18 13:45:44 PDT 2014
//
//  Modifications:
//
// ****************************************************************************

avtDataTree_p
avtSIMODataTreeIterator::ExecuteDataTree(avtDataRepresentation *dr)
{
    return ExecuteDataTree(dr->GetDataVTK(), dr->GetDomain(), dr->GetLabel());
}


// ****************************************************************************
//  Method: avtSIMODataTreeIterator::ExecuteDataTree
//
//  Purpose:
//      This function is used to call the execute data tree method on the
//      data leaves. This method should be implemented in the filters that
//      derive from it and it is an error to call it.
//
//  Programmer: Eric Brugger
//  Creation:   Fri Jul 18 13:45:44 PDT 2014
//
//  Modifications:
//
// ****************************************************************************

avtDataTree_p
avtSIMODataTreeIterator::ExecuteDataTree(vtkDataSet *, int, std::string)
{
    EXCEPTION0(ImproperUseException);
}


// ****************************************************************************
//  Method: avtSIMODataTreeIterator::ExecuteDataTreeOnThread
//
//  Purpose:
//  This function is used to call the execute data tree method on the data
//  leaves. This may be done in parallel or serial depending on VisIt build.
//  Also it depends if the filter will can run in parallel.
//
//  Programmer: David Camp
//  Creation:   May 2, 2013
//
//  Modifications:
//    Kevin Bensema, Thu 1 August 12:39 PDT 2013
//    Fixed memory leak by deleting SIMOWorkItem pointer passed in.
//
//    Eric Brugger, Fri Jul 18 13:45:44 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

typedef struct StructSIMOWorkItem 
{
    avtSIMODataTreeIterator *This;
    avtDataTree_p            inDT;
    avtDataTree_p            outDT;
} SIMOWorkItem;

void 
avtSIMODataTreeIterator::ExecuteDataTreeOnThread(void *cbdata)
{
    SIMOWorkItem *work = (SIMOWorkItem *)cbdata;

    //
    // We own the returned dataset because you cannot delete it if
    // it only has one reference and you want to return it.
    //
    avtDataTree_p retDT = work->This->ExecuteDataTree(&(work->inDT->GetDataRepresentation()));
    if( *(retDT) )
        work->outDT->operator=( *(retDT) );

    work->This->UpdateExtents(work->outDT);

    VisitMutexLock("currentNode");
        work->This->currentNode++;
        work->This->UpdateProgress(work->This->currentNode, work->This->totalNodes);
    VisitMutexUnlock("currentNode");

    // Delete the work item pointer.
    delete work;
}

// ****************************************************************************
//  Method: avtSIMODataTreeIterator::Execute
//
//  Purpose:
//    A recursive Execute method.  Walks down the tree and calls the virtual 
//    function ExecuteDataTree on the leaves.
//
//  Arguments:
//      inDT    The tree to traverse. 
//      dom     The domain number that the tree represents. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    April 12, 2001 
//
//  Modifications:
//
//    Hank Childs, Tue Jun 19 12:36:02 PDT 2001
//    Handle NULL data trees better.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Retrieve and pass on the label associated with a dataset. 
//
//    Hank Childs, Tue Nov  6 11:49:18 PST 2001
//    Added support for overriding extents.
//
//    Hank Childs, Sat Jan  5 10:31:48 PST 2002
//    Fix memory leak.
//
//    Kathleen Bonnell, Tue May 16 09:41:46 PDT 2006 
//    Removed call to SetSource(NULL), with new vtk pipeline, it also removes
//    necessary information from the dataset. 
//
//    David Camp, Thu May 23 12:52:53 PDT 2013
//    Removed the return argument and now pass in a variable to be returned.
//    This was done to support threading of the execute mode. The work is now
//    sent to the execution manager to schedule the work. This maybe done in
//    parallel or serial depending on VisIt build or on filter.
//
//    Hank Childs, Alister Maguire, Jeremy Brennan, Wed Dec 21 14:52:34 PDT 2016
//    Modified thread executor to take advantage of virtual bool declared in
//    the .h filed of filters if ThreadSafe() or not.
//
// ****************************************************************************

void
avtSIMODataTreeIterator::Execute(avtDataTree_p inDT, avtDataTree_p &outDT)
{
    CheckAbort();

    if (*inDT == NULL)
    {
        return;
    }

    int nc = inDT->GetNChildren();

    if (nc <= 0 && !inDT->HasData())
    {
        return;
    }

    if (nc == 0)
    {
        SIMOWorkItem *work = new SIMOWorkItem;
        work->This  = this;
        work->inDT  = inDT;
        work->outDT = outDT;
 
        if( *outDT )
        {
            // Schedule the work to be done.
            if (ThreadSafe())
            {
                avtExecutionManagerScheduleWork(ExecuteDataTreeOnThread, (void *)work);
            }
            else
            {
                // Not thread safe, so do it now.
                outDT = new avtDataTree();
                work->outDT = outDT;
                ExecuteDataTreeOnThread(work);
            }
        }
        else
        {
            // There is only one dataset to process, so do it now.
            outDT = new avtDataTree();
            work->outDT = outDT;
            ExecuteDataTreeOnThread(work);
        }
    }
    else
    {
        //
        // there is more than one input dataset to process
        // and we need an output datatree for each
        //
        avtDataTree_p *localOutDT = new avtDataTree_p[nc];
        for (int j = 0; j < nc; j++)
        {
            if (inDT->ChildIsPresent(j))
            {
                localOutDT[j].SetReference( new avtDataTree );
                Execute(inDT->GetChild(j), localOutDT[j]);
            }
            else
            {
                localOutDT[j] = NULL;
            }
        }
        outDT = new avtDataTree(nc, localOutDT);
        delete [] localOutDT;
    }
}


// ****************************************************************************
//  Method: avtSIMODataTreeIterator::UpdateExtents
//
//  Purpose:
//      Updates the extents that we are overriding with the current dataset.
//
//  Arguments:
//      tree    The data tree to merge into the current extents.
//
//  Programmer: Hank Childs
//  Creation:   November 6, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar 11 10:14:20 PST 2004
//    DataExtents now always have a size of 1 (num components = 2).
//
//    Jeremy Meredith, Tue Feb 20 11:04:44 PST 2007
//    Add support for transformed rectilinear grids.
//
//    David Camp, Thu May 23 12:52:53 PDT 2013
//    Changed function to be thread safe. Needed to protect the changing of 
//    the outAtts variable.
//
// ****************************************************************************

void
avtSIMODataTreeIterator::UpdateExtents(avtDataTree_p tree)
{
    if (*tree == NULL)
    {
        return;
    }

    if (overrideOriginalSpatialExtents)
    {
        avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();

        double bounds[6];
        bool gotBounds = false;
        struct {double *se; const double *xform;} info = {bounds, NULL};
        tree->Traverse(CGetSpatialExtents, (void *)&info, gotBounds);
        if (gotBounds)
        {
            avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
            avtExtents        *exts = atts.GetThisProcsOriginalSpatialExtents();
            avtExtents originalSpatialExtents(exts->GetDimension());

            originalSpatialExtents.Merge(bounds);
            exts = outAtts.GetThisProcsOriginalSpatialExtents();

            VisitMutexLock("SIMOSpatial");
            *exts = originalSpatialExtents;
            outAtts.GetOriginalSpatialExtents()->Clear();
            VisitMutexUnlock("SIMOSpatial");
        }
        else
        {
            VisitMutexLock("SIMOSpatial");
            outAtts.GetOriginalSpatialExtents()->Clear();
            VisitMutexUnlock("SIMOSpatial");
        }
    }

    if (overrideOriginalDataExtents)
    {
        avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();

        double range[2];  // who has more than 25 vars?
        bool gotBounds = false;
        tree->Traverse(CGetDataExtents, (void *)range, gotBounds);
        if (gotBounds)
        {
            avtExtents originalDataExtents(1);
            originalDataExtents.Merge(range);
            avtExtents *exts = outAtts.GetThisProcsOriginalDataExtents();

            VisitMutexLock("SIMOData");
            *exts = originalDataExtents;
            outAtts.GetOriginalDataExtents()->Clear();
            VisitMutexUnlock("SIMOData");
        }
        else
        {
            VisitMutexLock("SIMOData");
            outAtts.GetOriginalDataExtents()->Clear();
            VisitMutexUnlock("SIMOData");
        }
    }
}


