// ************************************************************************* //
//                      IncompatibleDomainListsException.C                   //
// ************************************************************************* //

#include <stdio.h>  // for sprintf
#include <IncompatibleDomainListsException.h>


// ****************************************************************************
//  Method: IncompatibleDomainListsException constructor
//
//  Arguments:
//      n1      The number of domains for the first domain list.
//      n2      The number of domains for the second domain list.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// ****************************************************************************

IncompatibleDomainListsException::IncompatibleDomainListsException(int n1, 
                                                                   int n2)
{
    char  str[1024];
    sprintf(str, "Cannot relate domain lists with %d domains and %d domains.",
                n1, n2);
    msg = str;
}


