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
//                               avtR2Fmaximum.C                             //
// ************************************************************************* //

#include <avtR2Fmaximum.h>

#include <float.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtR2Fmaximum constructor
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

avtR2Fmaximum::avtR2Fmaximum(int nb, double uv) : avtR2Foperator(nb, uv)
{
    max = new float[nb];
    for (int i = 0 ; i < nb ; i++)
        max[i] = -FLT_MAX;
}


// ****************************************************************************
//  Method: avtR2Fmaximum destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtR2Fmaximum::~avtR2Fmaximum()
{
    delete [] max;
}


// ****************************************************************************
//  Method: avtR2Fmaximum::AddData
//
//  Purpose:
//      Adds a single data point to the operator.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

void
avtR2Fmaximum::AddData(int b, float v)
{
    if (v > max[b])
        max[b] = v;
}


// ****************************************************************************
//  Method: avtR2Fmaximum::FinalizePass
//
//  Purpose:
//      Finalizes the current pass.  In the case of "maximum", there is only
//      one pass.  So calculate the final maximums and return them.  The caller
//      is responsible for freeing the memory.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Feb 25 15:28:11 PST 2006
//    Use the undefined val.
//
// ****************************************************************************

float *
avtR2Fmaximum::FinalizePass(int pass)
{
    float *rv = new float[nBins];
    UnifyMaximumFloatArrayAcrossAllProcessors(max, rv, nBins);
    for (int i = 0 ; i < nBins ; i++)
        rv[i] = (rv[i] == -FLT_MAX ? undefinedVal : rv[i]);
    return rv;
}


