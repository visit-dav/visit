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
//                                avtR2Fstddev.C                             //
// ************************************************************************* //

#include <avtR2Fstddev.h>

#include <math.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtR2Fstddev constructor
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

avtR2Fstddev::avtR2Fstddev(int nb, double uv) : avtR2Foperator(nb, uv)
{
    running_total_ave    = new double[nb];
    running_total_stddev = new double[nb];
    count                = new int[nb];
    for (int i = 0 ; i < nb ; i++)
    {
        running_total_ave[i]    = 0.;
        running_total_stddev[i] = 0.;
        count[i]                = 0;
    }
    pass = 0;
}


// ****************************************************************************
//  Method: avtR2Fstddev destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

avtR2Fstddev::~avtR2Fstddev()
{
    delete [] running_total_ave;
    delete [] running_total_stddev;
    delete [] count;
}


// ****************************************************************************
//  Method: avtR2Fstddev::AddData
//
//  Purpose:
//      Adds a single data point to the operator.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

void
avtR2Fstddev::AddData(int b, float v)
{
    if (pass == 0)
    {
        running_total_ave[b] += v;
        count[b]++;
    }
    else
    {
        float diff = (v-running_total_ave[b]);
        running_total_stddev[b] += diff*diff;
    }
}


// ****************************************************************************
//  Method: avtR2Fstddev::FinalizePass
//
//  Purpose:
//      Finalizes the current pass.  In the first pass, calculate the average.
//      In the second pass, calculate the standard deviation.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

float *
avtR2Fstddev::FinalizePass(int pass)
{
    float *rv = NULL;
    if (pass == 0)
    {
        double *rt2  = new double[nBins];
        int    *cnt2 = new int[nBins];
        SumIntArrayAcrossAllProcessors(count, cnt2, nBins);
        SumDoubleArrayAcrossAllProcessors(running_total_ave, rt2, nBins);
        for (int i = 0 ; i < nBins ; i++)
        {
            if (cnt2[i] > 0)
                running_total_ave[i] = rt2[i] / cnt2[i];
            else
                running_total_ave[i] = undefinedVal;
        }
        delete [] rt2;
        delete [] cnt2;
    }
    else
    {
        rv           = new float[nBins];
        double *rt2  = new double[nBins];
        int    *cnt2 = new int[nBins];
        SumIntArrayAcrossAllProcessors(count, cnt2, nBins);
        SumDoubleArrayAcrossAllProcessors(running_total_stddev, rt2, nBins);
        for (int i = 0 ; i < nBins ; i++)
        {
            if (cnt2[i] > 0)
                rv[i] = sqrt(rt2[i] / cnt2[i]);
            else
                rv[i] = undefinedVal;
        }
        delete [] rt2;
        delete [] cnt2;
    }

    this->pass++;
    return rv;
}


