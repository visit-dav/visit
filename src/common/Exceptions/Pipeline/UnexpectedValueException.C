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
// ****************************************************************************
UnexpectedValueException::UnexpectedValueException(int badVal, int expVal)
{
    char str[1024];
    sprintf(str, "Expected %d, Got %d", badVal, expVal);
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(double badVal, double expVal)
{
    char str[1024];
    sprintf(str, "Expected %f, Got %f", badVal, expVal);
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(int badVal, string expVal)
{
    char str[1024];
    sprintf(str, "Expected %d, Got %s", badVal, expVal.c_str());
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(double badVal, string expVal)
{
    char str[1024];
    sprintf(str, "Expected %f, Got %s", badVal, expVal.c_str());
    msg = str;
}
 
UnexpectedValueException::UnexpectedValueException(string badVal, string expVal)
{
    char str[1024];
    sprintf(str, "Expected %s, Got %s", badVal.c_str(), expVal.c_str());
    msg = str;
}
