// ************************************************************************* //
//                          NoDefaultVariableException.C                     //
// ************************************************************************* //

#include <string>

#include <NoDefaultVariableException.h>


using namespace std;


// ****************************************************************************
//  Method: NoDefaultVariableException constructor
//
//  Arguments:
//      opname  The name of the operator.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2002
//
// ****************************************************************************

NoDefaultVariableException::NoDefaultVariableException(string opname)
{
    msg = "The \"" + opname + "\" operator was told to operate on the default "
          + "variable, which does not exist.";
}


