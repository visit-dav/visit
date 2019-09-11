// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       UnexpectedValueException.C                          //
// ************************************************************************* //
#include <UnexpectedValueException.h>

// for sprintf
#include <stdio.h>

using std::string;
 
// ****************************************************************************
//  Method: UnexpectedValueException constructor
//
//  Purpose: handle unexpected value exception for different types
//
//  Programmer: Mark C. Miller
//  Creation:   December 9, 2003
//
//  Modifications
//
//    Mark C. Miller, Mon Nov  5 16:50:10 PST 2007
//    Fixed bonehead error where interpretation of bad/expected was reversed.
//
// ****************************************************************************
UnexpectedValueException::UnexpectedValueException(int expVal, int badVal)
{
    char str[1024];
    sprintf(str, "Expected %d, Got %d", expVal, badVal);
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(double expVal, double badVal)
{
    char str[1024];
    sprintf(str, "Expected %f, Got %f", expVal, badVal);
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(string expVal, int badVal)
{
    char str[1024];
    sprintf(str, "Expected %s, Got %d", expVal.c_str(), badVal);
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(string expVal, double badVal)
{
    char str[1024];
    sprintf(str, "Expected %s, Got %f", expVal.c_str(), badVal);
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(string expVal, string badVal)
{
    char str[1024];
    sprintf(str, "Expected %s, Got %s", expVal.c_str(), badVal.c_str());
    msg = str;
}
