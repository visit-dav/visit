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
//                           avtTwoPassDatasetQuery.C                        //
// ************************************************************************* //

#include <avtTwoPassDatasetQuery.h>

#include <vtkDataSet.h>


// ****************************************************************************
//  Constructor:  avtTwoPassDatasetQuery::avtTwoPassDatasetQuery
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 16, 2003
//
// ****************************************************************************
avtTwoPassDatasetQuery::avtTwoPassDatasetQuery()
    : avtDatasetQuery()
{
}


// ****************************************************************************
//  Method: avtTwoPassDatasetQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTwoPassDatasetQuery::~avtTwoPassDatasetQuery()
{
    ;
}


// ****************************************************************************
//  Method:  avtTwoPassDatasetQuery::Execute
//
//  Purpose:
//    The main execute method -- overrides base class virtual function.
//
//  Arguments:
//    inDT       the datatree
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 16, 2003
//
// ****************************************************************************
void
avtTwoPassDatasetQuery::Execute(avtDataTree_p inDT)
{
    // Two passes -- the length of this stage is double
    totalNodes *= 2;

    // Execute one pass, then the other
    ExecuteNthPass(inDT,1);

    MidExecute();

    ExecuteNthPass(inDT,2);
}


// ****************************************************************************
//  Method: avtTwoPassDatasetQuery::Execute
//
//  Purpose:
//    Recursive method to traverse an avtDataTree, calling Execute on the leaf
//    nodes. 
//
//  Notes:  Started as a direct copy of Execute() method of
//          avtDatasetQuery, added extensions for two passes
//
//  Programmer: Jeremy Meredith
//  Creation:   April 16, 2003
//
//  Modifications:
//    Kathleen Bonnell, Wed May 17 15:22:06 PDT 2006
//    Remove call to SetSource(NULL) as it now removes information necessary
//    for the dataset.
//
// ****************************************************************************
void
avtTwoPassDatasetQuery::ExecuteNthPass(avtDataTree_p inDT, int pass)
{
    if (*inDT == NULL)
    {
        return;
    }
 
    int nc = inDT->GetNChildren();
 
    if (nc <= 0 && !inDT->HasData())
    {
        return;
    }
 
    if ( nc == 0 )
    {
        //
        // There is only one dataset to process.
        //
        vtkDataSet *in_ds = inDT->GetDataRepresentation().GetDataVTK();
        int dom = inDT->GetDataRepresentation().GetDomain();
 
        //
        // Setting the source to NULL for the input will break the
        // pipeline.
        //
        //in_ds->SetSource(NULL);

        if (pass==1)
            Execute1(in_ds, dom);
        if (pass==2)
            Execute2(in_ds, dom);

        currentNode++;
        UpdateProgress(currentNode, totalNodes);
    }
    else
    {
        //
        // There is more than one input dataset to process.
        // Recurse through the children.
        //
        for (int j = 0; j < nc; j++)
        {
            if (inDT->ChildIsPresent(j))
            {
                ExecuteNthPass(inDT->GetChild(j), pass);
            }
        }
    }
}

