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


