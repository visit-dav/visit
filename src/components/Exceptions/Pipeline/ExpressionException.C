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
ExpressionException::ExpressionException(string reason)
{
    msg = "The expression failed because " + reason;
}


