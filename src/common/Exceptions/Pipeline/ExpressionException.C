// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************ //
//                          ExpressionException.C                           //
// ************************************************************************ //

#include <ExpressionException.h>

using namespace std;

// ****************************************************************************
//  Method: ExpressionException constructor
//
//  Programmer: Sean Ahern
//  Creation:   Fri Mar 22 13:23:12 PST 2002
//
// ****************************************************************************
ExpressionException::ExpressionException(string name, string reason)
{
    msg = "The '" + name + "' expression failed because " + reason;
}

// ****************************************************************************
//  Method: ExpressionException constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   December 18, 2007
//
// ****************************************************************************
ExpressionException::ExpressionException(const char *name, string reason)
{
    if (name == NULL)
        msg = "The expression failed because " + reason;
    else
        msg = "The '" + string(name) + "' expression failed because " +
              reason;
}



// ****************************************************************************
//  Method: ExpressionParseException constructor
//
//  Programmer: Sean Ahern
//  Creation:   Mon Dec 10 13:12:23 EST 2007
//
// ****************************************************************************
ExpressionParseException::ExpressionParseException(string reason)
{
    msg = "Expression parsing failed because " + reason;
}


