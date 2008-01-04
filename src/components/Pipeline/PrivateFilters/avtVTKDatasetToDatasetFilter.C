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
//                       avtVTKDatasetToDatasetFilter.C                      //
// ************************************************************************* //

#include <vtkDataSet.h>

#include <avtCommonDataFunctions.h>
#include <avtDataset.h>
#include <avtDomainList.h>
#include <avtLoadBalancer.h>
#include <avtVTKDatasetToDatasetFilter.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtVTKDatasetToDatasetFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//      
// ****************************************************************************

avtVTKDatasetToDatasetFilter::avtVTKDatasetToDatasetFilter()
{
    source = NULL;
}


// ****************************************************************************
//  Method: avtVTKDatasetToDatasetFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

avtVTKDatasetToDatasetFilter::~avtVTKDatasetToDatasetFilter()
{
    if (source != NULL)
    {
        delete source;
        source = NULL;
    }
}


// ****************************************************************************
//  Method: avtVTKDatasetToDatasetFilter::SetInput
//
//  Purpose:
//      Overrides the base SetInput function.  This allows for there to be
//      no avtDataset input.
//
//  Arguments:
//      list    A list of VTK datasets.
//      listN   The number of datasets in l.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

void
avtVTKDatasetToDatasetFilter::SetInput(vtkDataSet **list, int listN)
{
    if (source != NULL)
    {
        //
        // The input was set previously.  Delete it so we don't have a leak.
        //
        delete source;
        source = NULL;
    }

    source = new avtSourceFromDataset(list, listN);
    avtDatasetToDatasetFilter::SetInput(source->GetOutput());

    modified = true;
}


// ****************************************************************************
//  Method: avtVTKDatasetToDatasetFilter::UpdateInput
//
//  Purpose:
//      Overrides the avtDatasetToDatasetFilter UpdateInput method.  Since this
//      will end the avt pipeline, it updates the vtk input.
//
//  Arguments:
//      dl      A list of the domains of interest to the filters below this
//              one in the pipeline.
//
//  Returns:    true if the input was modified.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Sep 26 09:42:26 PDT 2000
//    Added call to GetDomainVTK since FetchDomain returns an avtDomain.
//
//    Hank Childs, Tue Jan 30 18:52:48 PST 2001
//    Load balance in parallel.
//
//    Kathleen Bonnell, Fri Feb  9 16:57:06 PST 2001 
//    Added call to GetDomain() since FetchDomain returns an avtDomainTree.
//
//    Hank Childs, Thu Mar  1 14:45:20 PST 2001
//    Switched FetchDomain to FetchDomains (with s).
//
//    Kathleen Bonnell, Mon Apr  9 13:02:51 PDT 2001 
//    Reflect change from avtDomainTree to avtDataTree. Added call
//    to Traverse to perform the actual update.
//
// ****************************************************************************

bool
avtVTKDatasetToDatasetFilter::UpdateInput(avtDomainList *dl)
{
    //
    // Make this work well when we are testing in parallel.
    //
    avtLoadBalancer lb;
    lb.Calculate(dl);

    avtDataTree_p tree = source->FetchDomains(dl);
    tree->Traverse(CUpdateData, NULL, modified); 

    return modified;
}


// ****************************************************************************
//  Method: avtVTKDatasetToDatasetFilter::Execute
//
//  Purpose:
//      Executes the filter.  This takes the domains being used by the filters
//      downstream and puts them in the avtDataset output.
//
//  Arguments:
//      <unused>   A list of the domains that should be executed.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Sep 28 13:06:55 PDT 2000
//    Made this do the work that CreateOutputDatasets used to.
//
//    Kathleen Bonnell, Fri Feb  9 16:57:06 PST 2001 
//    Added call to GetDomain() since FetchDomain returns an avtDomainTree.
//    Made SetOutputDomain send an avtDomainTree.
//
//    Hank Childs, Mon Feb 12 17:28:35 PST 2001
//    Only GetDomain's if they are on the domain list.
//
//    Hank Childs, Thu Mar  1 14:45:20 PST 2001
//    Switched FetchDomain to FetchDomains (with s).
//
//    Kathleen Bonnell, Mon Apr  9 13:02:51 PDT 2001 
//    Reflect that output is now single avtDataTree, instead of multiple 
//    avtDomainTrees (one per domain).
//
// ****************************************************************************

void
avtVTKDatasetToDatasetFilter::Execute(avtDomainList *dl)
{
    avtDataTree_p tree = source->FetchDomains(dl);

    //
    // Pass the output of the source on as the output of this filter.  
    // Since this is an AVT filter, there are no two filters owning 
    // the same output (there are in VTK).
    //
    SetOutputDataTree(tree);
}


// ****************************************************************************
//  Method: avtVTKDatasetToDatasetFilter::CalcDomainList
//
//  Purpose:
//      Defines pure virtual function.  This filter doesn't eliminate any
//      domains, so just instruct our domain list to use all domains.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

void
avtVTKDatasetToDatasetFilter::CalcDomainList(void)
{
    domains->UseAllDomains();
}


