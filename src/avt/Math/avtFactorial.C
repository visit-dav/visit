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

#include "avtFactorial.h"
#include <math.h>
#include <sstream>
#include <iostream>

using namespace std;

// static members
bool          avtFactorial::memoize = true;
doubleVector  avtFactorial::factorialTable;


// ****************************************************************************
//  Method:  avtFactorialGenerator::avtFactorialGenerator
//
//  Purpose:
//     Factorial Generator constructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtFactorial::avtFactorial()
{;}

// ****************************************************************************
//  Method:  avtFactorialGenerator::~avtFactorialGenerator
//
//  Purpose:
//     Factorial Generator destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 5, 2007
//
// ****************************************************************************

avtFactorial::~avtFactorial()
{;}

// ****************************************************************************
//  Method:  avtFactorialGenerator::Eval
//
//  Purpose:
//     Computes the factorial of for integer n >= 0. Uses memoization if 
//     enabled (default).
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************
double
avtFactorial::Eval(int n)
{
    if(n <= 0)
        return 1;

    // loop index
    int i;
    double res = 1.0;
    if(memoize)
    {
        // get current table size
        int ft_size = factorialTable.size();
        // init table if ness
        if(ft_size == 0)
        {
            factorialTable.push_back(1);
            ft_size = 1;
        }
    
        // expand table
        if(ft_size <= n)
        {
            for(i = ft_size; i <= n; ++i)
                factorialTable.push_back(factorialTable[i-1] * i);
        }
        // get result
        res = factorialTable[n];
    }
    else // eval factorial standard way
    {
        for(i = 1; i <= n; ++i)
            res = res * i;
    }
    // return result
    return res;
}

// ****************************************************************************
//  Method:  avtFactorialGenerator::Eval
//
//  Purpose:
//     Sets if this class should use memoization to trade space for speed in
//     factorial evaluation.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************
void
avtFactorial::SetMemoize(bool v)
{
    memoize = v;
    if(!v)
        factorialTable.clear();
}




