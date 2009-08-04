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
//                          avtSourceFromDataset.C                           //
// ************************************************************************* //

#include <avtSourceFromDataset.h>

#include <vtkDataSet.h>

#include <avtDataset.h>
#include <avtSILRestrictionTraverser.h>

#include <BadDomainException.h>


using     std::vector;


// ****************************************************************************
//  Method: avtSourceFromDataset constructor
//
//  Arguments:
//      d     An array of vtkDataSets that make up multiple domains.
//      nd    The number of datasets in d.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jun  1 16:40:06 PDT 2001
//    Overhauled method, removed previous comments.
//
// ****************************************************************************

avtSourceFromDataset::avtSourceFromDataset(vtkDataSet **d, int nd) 
{
    nDataset = nd;
    datasets = new vtkDataSet*[nDataset];
    for (int i = 0 ; i < nDataset ; i++)
    {
        datasets[i] = d[i];
        if (datasets[i] != NULL)
        {
            datasets[i]->Register(NULL);
        }
    }
}


// ****************************************************************************
//  Method: avtSourceFromDataset destructor
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jun  1 16:40:06 PDT 2001
//    Overhauled method, removed previous comments.
//
// ****************************************************************************

avtSourceFromDataset::~avtSourceFromDataset()
{
    if (datasets != NULL)
    {
        for (int i = 0 ; i < nDataset ; i++)
        {
            if (datasets[i] != NULL)
            {
                datasets[i]->Delete();
            }
        }
        delete [] datasets;
    }
}


// ****************************************************************************
//  Method: avtSourceFromDataset::FetchDataset
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
//  Creation:   June 1, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov 22 16:35:30 PST 2002
//    Used SIL restriction traverser.
//
//    Mark C. Miller, 17Mar03
//    eliminated use of "spec" argument and SIL restrction. 
//    the data specification abstraction doesn't make much sense here because
//    we're basically dealing with raw VTK dataset objects clothed as AVT
//    datasets
// ****************************************************************************

bool
avtSourceFromDataset::FetchDataset(avtDataRequest_p spec,
                                   avtDataTree_p &tree)
{
    int  i;

    //
    // It needs to have "chunk ids".  Just make some up so we can meet the
    // interface.
    //
    int *chunks = new int[nDataset];
    for (i = 0 ; i < nDataset ; i++)
        chunks[i] = i; 
    
    tree = new avtDataTree(nDataset, datasets, chunks);
    delete [] chunks;

    return false;
}
