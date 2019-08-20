// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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




