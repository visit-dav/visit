// ************************************************************************* //
//                            InvalidMergeException.C                        //
// ************************************************************************* //

#include <stdio.h>   // for sprintf
#include <InvalidMergeException.h>


using namespace std;


// ****************************************************************************
//  Method: InvalidMergeException constructor
//
//  Arguments:
//      n1      The number of domains for the first field.
//      n2      The number of domains for the second field.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2000
//
// ****************************************************************************

InvalidMergeException::InvalidMergeException(int n1, int n2)
{
    char  str[1024];
    sprintf(str, "Cannot merge datasets because of an incompatible field "
                 "%d and %d.", n1, n2);
    msg = str;
}


// ****************************************************************************
//  Method: InvalidMergeException constructor
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2000
//
// ****************************************************************************

InvalidMergeException::InvalidMergeException()
{
    msg = "The two datasets cannot be merged because they overlap.";
}


