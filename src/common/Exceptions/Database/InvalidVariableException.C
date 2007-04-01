// ************************************************************************* //
//                          InvalidVariableException.C                       //
// ************************************************************************* //

#include <string>

#include <InvalidVariableException.h>


using namespace std;


// ****************************************************************************
//  Method: InvalidVariableException constructor
//
//  Arguments:
//      varname   The name of the invalid variable.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2000
//
// ****************************************************************************

InvalidVariableException::InvalidVariableException(string varname)
{
    if (varname == "")
        msg = "There was no variable specified.";
    else
        msg = "An invalid variable (" + varname + ") was specified.";
}


