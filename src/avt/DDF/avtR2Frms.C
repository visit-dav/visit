/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
//                                 avtR2Frms.C                               //
// ************************************************************************* //

#include <avtR2Frms.h>

#include <avtParallel.h>
#include <math.h>


// ****************************************************************************
//  Method: avtR2Frms constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Aug 10 10:34:46 PDT 2010
//
//  Modifications:
//
// ****************************************************************************

avtR2Frms::avtR2Frms(int nb, double uv) : avtR2Foperator(nb, uv)
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
//  Method: avtR2Frms destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Aug 10 10:34:46 PDT 2010
//
// ****************************************************************************

avtR2Frms::~avtR2Frms()
{
    delete [] running_total;
    delete [] count;
}


// ****************************************************************************
//  Method: avtR2Frms::AddData
//
//  Purpose:
//      Adds a single data point to the operator.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Aug 10 10:34:46 PDT 2010
//
// ****************************************************************************

void
avtR2Frms::AddData(int b, float v)
{
    running_total[b] += v*v;
    count[b]++;
}


// ****************************************************************************
//  Method: avtR2Frms::FinalizePass
//
//  Purpose:
//      Finalizes the current pass.  In the case of "rms", there is only
//      one pass. The array  running_total contains:
//          v = (x0^2 + x1^2 + ... + xN^2)
//      To finalize calculate rms = sqrt(v/N) for each bin.
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Aug 10 10:34:46 PDT 2010
//
//  Modifications:
//
// ****************************************************************************

float *
avtR2Frms::FinalizePass(int pass)
{
    float  *rv   = new float[nBins];
    double *rt2  = new double[nBins];
    int    *cnt2 = new int[nBins];
    SumIntArrayAcrossAllProcessors(count, cnt2, nBins);
    SumDoubleArrayAcrossAllProcessors(running_total, rt2, nBins);
    for (int i = 0 ; i < nBins ; i++)
    {
        if (cnt2[i] > 0)
            rv[i] = sqrt(rt2[i] / cnt2[i]);
        else
            rv[i] = undefinedVal;
    }
    delete [] rt2;
    delete [] cnt2;

    return rv;
}


