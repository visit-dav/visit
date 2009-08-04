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
//                       avtMedianFilterExpression.C                         //
// ************************************************************************* //

#include <avtMedianFilterExpression.h>

#include <algorithm>

#include <vtkDataArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtCallback.h>

#include <ExpressionException.h>


using std::sort;


// ****************************************************************************
//  Method: avtMedianFilterExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
// ****************************************************************************

avtMedianFilterExpression::avtMedianFilterExpression()
{
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtMedianFilterExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
// ****************************************************************************

avtMedianFilterExpression::~avtMedianFilterExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMedianFilterExpression::PreExecute
//
//  Purpose:
//      Initialize the haveIssuedWarning flag.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
// ****************************************************************************

void
avtMedianFilterExpression::PreExecute(void)
{
    avtUnaryMathExpression::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Function: FindMedian
//
//  Purpose:
//      Finds the median in a manner efficient for small lists.
//
//  Programmer: Hank Childs
//  Creation:   August 19, 2005
//
// ****************************************************************************

static float
FindMedian(float *list, int nlist)
{
    int half = nlist/2;
    if (nlist % 2 == 0)
        half--;

    float cantBeSmallerThan = -FLT_MAX;
    float cantBeBiggerThan = +FLT_MAX;
    for (int i = 0 ; i < nlist ; i++)
    {
        if (list[i] <= cantBeSmallerThan)
            continue;
        if (list[i] >= cantBeBiggerThan)
            continue;
        int numSmaller = 0;
        int numBigger  = 0;
        int numSame    = 0;
        for (int j = 0 ; j < nlist ; j++)
        {
             if (i == j)
                 continue;
             if (list[i] == list[j])
                 numSame++;
             else if (list[i] < list[j])
                 numBigger++;
             else
                 numSmaller++;
        }
        bool enoughSmaller = false;
        if ((numSmaller + numSame) >= half)
            enoughSmaller = true;
        else
            cantBeSmallerThan = list[i];
        bool enoughBigger = false;
        if ((numBigger + numSame) >= half)
            enoughBigger = true;
        else
            cantBeBiggerThan = list[i];
        if (enoughSmaller && enoughBigger)
            return list[i];
    }

    // Wow.  Should never get here.
    return list[0];
}


// ****************************************************************************
//  Method: avtMedianFilterExpression::DoOperation
//
//  Purpose:
//      Calculates the median.  I'm sure we could be much smarter about how
//      the median is calculated.  (Right now, it just sorts them and chooses
//      the middle.)
//
//  Arguments:
//      in1           The first input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Aug 19 11:08:56 PDT 2005
//    Use a more efficient method to determine the median.
//
//    Hank Childs, Tue Aug 23 09:37:51 PDT 2005
//    Fix indexing bug.
//
//    Hank Childs, Fri Jun  9 14:34:50 PDT 2006
//    Remove unused variable.
//
// ****************************************************************************

void
avtMedianFilterExpression::DoOperation(vtkDataArray *in1, vtkDataArray *out, 
                           int ncomponents,int ntuples)
{
    if (cur_mesh->GetDataObjectType() != VTK_RECTILINEAR_GRID &&
        cur_mesh->GetDataObjectType() != VTK_STRUCTURED_GRID)
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The mean filter expression only "
                                      "operates on structured grids.");
            haveIssuedWarning = true;
        }
        return;
    }

    int dims[3];
    if (cur_mesh->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) cur_mesh;
        rgrid->GetDimensions(dims);
    }
    else
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) cur_mesh;
        sgrid->GetDimensions(dims);
    }

    bool nodeCentered = true;
    if (dims[0]*dims[1]*dims[2] != ntuples)
        nodeCentered = false;

    if (!nodeCentered)
    {
        dims[0] -= 1;
        dims[1] -= 1;
        dims[2] -= 1;
    }

    float all_vals[27];
    int   numSamps = 0;

    if (dims[2] <= 1)
    {
        for (int i = 0 ; i < dims[0] ; i++)
        {
            for (int j = 0 ; j < dims[1] ; j++)
            {
                int idx = j*dims[0]+i;
                numSamps = 0;
                for (int ii = i-1 ; ii <= i+1 ; ii++)
                {
                    if (ii < 0 || ii >= dims[0])
                        continue;
                    for (int jj = j-1 ; jj <= j+1 ; jj++)
                    {
                        if (jj < 0 || jj >= dims[1])
                            continue;
                        int idx2 = jj*dims[0] + ii;
                        all_vals[numSamps] = in1->GetTuple1(idx2);
                        numSamps++;
                    }
                }
                float median = FindMedian(all_vals, numSamps);
                out->SetTuple1(idx, median);
            }
        }
    }
    else
    {
        for (int i = 0 ; i < dims[0] ; i++)
        {
            for (int j = 0 ; j < dims[1] ; j++)
            {
                for (int k = 0 ; k < dims[2] ; k++)
                {
                    int idx = k*dims[0]*dims[1] + j*dims[0]+i;
                    numSamps = 0;
                    for (int ii = i-1 ; ii <= i+1 ; ii++)
                    {
                        if (ii < 0 || ii >= dims[0])
                            continue;
                        for (int jj = j-1 ; jj <= j+1 ; jj++)
                        {
                            if (jj < 0 || jj >= dims[1])
                                continue;
                            for (int kk = k-1 ; kk <= k+1 ; kk++)
                            {
                                if (kk < 0 || kk >= dims[2])
                                    continue;
                                int idx2 = kk*dims[1]*dims[0] + jj*dims[0] +ii;
                                all_vals[numSamps] = in1->GetTuple1(idx2);
                                numSamps++;
                            }
                        }
                    }
                    float median = FindMedian(all_vals, numSamps);
                    out->SetTuple1(idx, median);
                }
            }
        }
    }
}


