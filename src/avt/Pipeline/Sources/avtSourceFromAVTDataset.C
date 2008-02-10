/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                        avtSourceFromAVTDataset.C                          //
// ************************************************************************* //

#include <avtSourceFromAVTDataset.h>

#include <avtCommonDataFunctions.h>
#include <avtDataset.h>
#include <avtSILRestrictionTraverser.h>

#include <TimingsManager.h>


using     std::vector;


avtDataObject_p   DataObjectFromDataset(avtDataset_p);


// ****************************************************************************
//  Method: avtSourceFromAVTDataset constructor
//
//  Arguments:
//      d     An avtdataset.
//
//  Programmer: Hank Childs
//  Creation:   June 19, 2001
//
// ****************************************************************************

avtSourceFromAVTDataset::avtSourceFromAVTDataset(avtDataset_p d)
    : avtInlinePipelineSource(DataObjectFromDataset(d))
{
    tree = d->GetDataTree();
    GetTypedOutput()->GetInfo().Copy(d->GetInfo());
}


// ****************************************************************************
//  Method: avtSourceFromAVTDataset destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSourceFromAVTDataset::~avtSourceFromAVTDataset()
{
    ;
}


// ****************************************************************************
//  Method: avtSourceFromAVTDataset::FetchDataset
//
//  Purpose:
//      Gets all of the domains specified in the domain list.
//
//  Arguments:
//      spec    A specification of which domains to use.
//      tree    The output data tree.
//
//  Returns:    Whether or not this dataset is different from the last time it
//              was updated.
//       
//  Programmer: Hank Childs
//  Creation:   June 19, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Sep 16 13:46:40 PDT 2001
//    Added code to prevent trees from getting unnecessarily large.
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001
//    Restructured to make use of new tree method PruneTree. 
//
//    Jeremy Meredith, Hank Childs, Fri Jul 26 13:57:48 PDT 2002
//    Some use cases (e.g. calling this method on the viewer)
//    do not require a SIL restriction, so allow for this now.
//
//    Hank Childs, Fri Nov 22 16:34:04 PST 2002
//    Use the SIL restriction traverser.
//
//    Hank Childs, Wed Mar  2 10:09:31 PST 2005
//    Do not assume tree is non-NULL.
//
//    Hank Childs, Fri Nov 30 16:47:33 PST 2007
//    Add timing information.
//
// ****************************************************************************

bool
avtSourceFromAVTDataset::FetchDataset(avtDataRequest_p spec,
                                      avtDataTree_p &outtree)
{
    int timingsHandle = visitTimer->StartTimer();

    vector<int> list;
    if (spec->GetSIL().useRestriction)
    {
        int t0 = visitTimer->StartTimer();
        avtSILRestrictionTraverser trav(spec->GetRestriction());
        visitTimer->StopTimer(t0, "SourceFromAVTDataset:: setting up SILtrav");
        int t1 = visitTimer->StartTimer();
        trav.GetDomainList(list);
        visitTimer->StopTimer(t1, "SourceFromAVTDataset:: getting domains");
        if (*tree != NULL)
        {
            int t2 = visitTimer->StartTimer();
            outtree = tree->PruneTree(list);
            visitTimer->StopTimer(t2, "SourceFromAVTDataset:: prune tree");
        }
        else
            outtree = NULL;
    }
    else
    {
        int dataChunk = spec->GetSIL().dataChunk;
        if (dataChunk >= 0)
        {
            list.push_back(dataChunk);
            if (*tree != NULL)
            {
                int t2 = visitTimer->StartTimer();
                outtree = tree->PruneTree(list);
                visitTimer->StopTimer(t2, "SourceFromAVTDataset:: prunetree2");
            }
            else
                outtree = NULL;
        }
        else
        {
            outtree = tree;
        }
    }

    visitTimer->StopTimer(timingsHandle, "Fetching dataset AVT dataset");

    return false;
}


// ****************************************************************************
//  Function: DataObjectFromDataset
//
//  Purpose:
//      Creates a pointer typed to the base class.  This should happen free
//      with C++, but does not because of reference pointers.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 15:24:42 PST 2002
//    Changed CopyTo to an inline template function.
//
// ****************************************************************************

avtDataObject_p
DataObjectFromDataset(avtDataset_p ds)
{
    avtDataObject_p rv;
    CopyTo(rv, ds);

    return rv;
}


