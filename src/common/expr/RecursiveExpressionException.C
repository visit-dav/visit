// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************ //
//                      RecursiveExpressionException.C                      //
// ************************************************************************ //

#include <RecursiveExpressionException.h>

using namespace std;

// ****************************************************************************
//  Method: RecursiveExpressionException constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 15, 2003
//
// ****************************************************************************
RecursiveExpressionException::RecursiveExpressionException(string var)
{
    msg = "The expression was recursive in variable \"" + var + "\".";
}


