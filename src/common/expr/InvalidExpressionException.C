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


