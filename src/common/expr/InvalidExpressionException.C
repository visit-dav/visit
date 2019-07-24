// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************ //
//                       InvalidExpressionException.C                       //
// ************************************************************************ //

#include <InvalidExpressionException.h>

using namespace std;

// ****************************************************************************
//  Method: InvalidExpressionException constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 15, 2003
//
// ****************************************************************************
InvalidExpressionException::InvalidExpressionException(string reason)
{
    msg = "The expression was invalid: " + reason;
}


