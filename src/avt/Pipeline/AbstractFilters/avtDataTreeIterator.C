/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                                avtDataTreeIterator.C                              //
// ************************************************************************* //

#include <vtkDataSet.h>

#include <avtDataTree.h>
#include <avtDataTreeIterator.h>


// ****************************************************************************
//  Method: avtDataTreeIterator constructor
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
// ****************************************************************************

avtDataTreeIterator::avtDataTreeIterator()
{
    lastDataset = NULL;
}


// ****************************************************************************
//  Method: avtDataTreeIterator destructor
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
// ****************************************************************************

avtDataTreeIterator::~avtDataTreeIterator()
{
    if (lastDataset != NULL)
    {
        lastDataset->Delete();
        lastDataset = NULL;
    }
}


// ****************************************************************************
//  Method: avtDataTreeIterator::ManageMemory
//
//  Purpose:
//      Is a resting spot for the last dataset a filter has processed.  Many
//      filters have a problem of what to do with their datasets when they
//      return from ExecuteData, since they have bumped the reference count
//      and decrementing it before returning would destruct the data before
//      it is returned.  This is a mechanism to store it so the derived types
//      can forget about it.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu May  6 11:35:15 PDT 2004
//    Made sure not to delete the last reference to something if it was the
//    thing we were about to add a reference to.  In other words, if the
//    new dataset is the same as the old one, noop.
//
// ****************************************************************************

void
avtDataTreeIterator::ManageMemory(vtkDataSet *ds)
{
    if (ds == lastDataset)
        return;

    if (lastDataset != NULL)
    {
        lastDataset->Delete();
    }

    lastDataset = ds;
    if (lastDataset != NULL)
    {
        lastDataset->Register(NULL);
    }
}


// ****************************************************************************
//  Method: avtDataTreeIterator::ReleaseData
//
//  Purpose:
//      Releases any problem-size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
// ****************************************************************************

void
avtDataTreeIterator::ReleaseData(void)
{
    avtSIMODataTreeIterator::ReleaseData();
    ManageMemory(NULL);  // Cleans out any stored datasets.
}


// ****************************************************************************
//  Method: avtDataTreeIterator::ExecuteDataTree
//
//  Purpose:
//      Defines the pure virtual function executedomaintree.  
//      Serves as a wrapper for the ExecuteDomain method.
//
//  Arguments:
//      ds      The vtkDataSet to pass to the derived type.
//      dom     The domain number of the input dataset.
//      label   The label associated with this datset.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 9, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001
//    Made this method return avtDataTree.
//
//    Kathleen Bonnell, Wed Sep 19 13:35:35 PDT 2001 
//    Added string argument so that labels will get passed on. 
// 
//    Hank Childs, Fri Oct 19 10:56:55 PDT 2001
//    Allow for derived types to return NULL.
//
//    Hank Childs, Wed Sep 11 09:17:46 PDT 2002
//    Pass the label down to the derived types as well.
//
//    Hank Childs, Mon Jun 27 10:02:55 PDT 2005
//    Choose better file names when doing a "-dump" in parallel.
//
//    Hank Childs, Tue Jul  5 09:41:28 PDT 2005
//    Fix cut-n-paste bug with last change.
//
//    Hank Childs, Wed Aug 31 09:10:11 PDT 2005
//    Make sure that -dump in parallel increments the dump index.
//
//    Hank Childs, Thu Dec 21 15:38:53 PST 2006
//    Removed -dump functionality, since it is now handled at a lower level.
//
// ****************************************************************************

avtDataTree_p
avtDataTreeIterator::ExecuteDataTree(vtkDataSet* ds, int dom, std::string label)
{
    vtkDataSet *out_ds = ExecuteData(ds, dom, label);
    if (out_ds == NULL)
    {
        return NULL;
    }

    return new avtDataTree(out_ds, dom, label);
}
