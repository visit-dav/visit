// ************************************************************************* //
//                       StubReferencedException.C                          //
// ************************************************************************* //
#include <StubReferencedException.h>

// ****************************************************************************
//  Method: StubReferencedException constructor
//
//  Purpose: handle stub reference exception
//
//  Programmer: Mark C. Miller
//  Creation:   October 25, 2005 
//
// ****************************************************************************
StubReferencedException::StubReferencedException(const char *stubName)
{
    msg = stubName;
}
