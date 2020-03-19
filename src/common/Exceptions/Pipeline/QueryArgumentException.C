// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      QueryArgumentException.C                             //
// ************************************************************************* //

#include <QueryArgumentException.h>



// ****************************************************************************
//  Method: QueryArgumentException constructor
//
//  Input:
//    missingArg : name of missing argument. 
//  
//  Programmer: Kathleen Biagas 
//  Creation:   August 23, 2011
//
// ****************************************************************************

QueryArgumentException::QueryArgumentException(const std::string &missingArg)
{
    msg =  "There was an error parsing query arguments.  Could not find argument `" + missingArg + "`";
}

// ****************************************************************************
//  Method: QueryArgumentException constructor
//  
//  Input:
//    qArg : name of argument with invalid type. 
//    argTypes : acceptable types for the argument.
//
//  Programmer: Kathleen Biagas 
//  Creation:   August 23, 2011
//
// ****************************************************************************

QueryArgumentException::QueryArgumentException(const std::string &qArg, 
                                               const std::string &argTypes)
{
    msg =  "There was an error parsing query arguments.  Argument `";
    msg += qArg;
    msg += "`";
    msg += " should have type: ";
    msg += argTypes;
    msg += ".";
}

// ****************************************************************************
//  Method: QueryArgumentException constructor
//  
//  Input:
//    qArg : name of argument with invalid type. 
//    argTypes : acceptable types for the argument.
//
//  Programmer: Kathleen Biagas 
//  Creation:   January 10, 2013 
//
// ****************************************************************************

QueryArgumentException::QueryArgumentException(const std::string &qArg, 
                                               const int num)
{
    msg =  "There was an error parsing query arguments.  Argument `";
    msg += qArg;
    msg += "`";
    msg += " should have ";
    msg += num;
    if (num == 1)
        msg += "at least 1 element.";
    else
        msg += " elements.";
}

