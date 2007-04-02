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
//      n1      The first field's integer.
//      n2      The second field's integer.
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
//  Arguments:
//      n1      The first field's double precision number.
//      n2      The second field's double precision number.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2005
//
// ****************************************************************************

InvalidMergeException::InvalidMergeException(double n1, double n2)
{
    char  str[1024];
    sprintf(str, "Cannot merge datasets because of an incompatible field "
                 "%g and %g.", n1, n2);
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


// ****************************************************************************
//  Method: InvalidMergeException constructor
//
//  Programmer: Hank Childs
//  Creation:   August 29, 2005
//
// ****************************************************************************

InvalidMergeException::InvalidMergeException(const char *m)
{
    msg = m;
}


