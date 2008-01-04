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
//                               avtR2Faverage.C                             //
// ************************************************************************* //

#include <avtR2Faverage.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtR2Faverage constructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Feb 25 15:22:19 PST 2006
//    Add undefinedVal.
//
// ****************************************************************************

avtR2Faverage::avtR2Faverage(int nb, double uv) : avtR2Foperator(nb, uv)
{
    running_total = new double[nb];
    count         = new int[nb];
    for (int i = 0 ; i < nb ; i++)
    {
        running_total[i] = 0.;
        count[i]         = 0;
    }
}


// ****************************************************************************
//  Method: avtR2Faverage destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtR2Faverage::~avtR2Faverage()
{
    delete [] running_total;
    delete [] count;
}


// ****************************************************************************
//  Method: avtR2Faverage::AddData
//
//  Purpose:
//      Adds a single data point to the operator.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

void
avtR2Faverage::AddData(int b, float v)
{
    running_total[b] += v;
    count[b]++;
}


// ****************************************************************************
//  Method: avtR2Faverage::FinalizePass
//
//  Purpose:
//      Finalizes the current pass.  In the case of "average", there is only
//      one pass.  So calculate the final averages and return them.  The caller
//      is responsible for freeing the memory.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Feb 25 15:22:19 PST 2006
//    Use the "undefinedVal".
//
// ****************************************************************************

float *
avtR2Faverage::FinalizePass(int pass)
{
    float  *rv   = new float[nBins];
    double *rt2  = new double[nBins];
    int    *cnt2 = new int[nBins];
    SumIntArrayAcrossAllProcessors(count, cnt2, nBins);
    SumDoubleArrayAcrossAllProcessors(running_total, rt2, nBins);
    for (int i = 0 ; i < nBins ; i++)
    {
        if (cnt2[i] > 0)
            rv[i] = rt2[i] / cnt2[i];
        else
            rv[i] = undefinedVal;
    }
    delete [] rt2;
    delete [] cnt2;

    return rv;
}


