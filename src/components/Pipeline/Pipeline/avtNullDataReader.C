// ************************************************************************* //
//                              avtNullDataReader.C                          //
// ************************************************************************* //

#include <avtNullDataReader.h>


// ****************************************************************************
//  Method: avtNullDataReader constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   January 8, 2003 
//
// ****************************************************************************

avtNullDataReader::avtNullDataReader()
{
    haveReadNullData = false;
}


// ****************************************************************************
//  Method: avtNullDataReader destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNullDataReader::~avtNullDataReader()
{
    ;
}


// ****************************************************************************
//  Method: avtNullDataReader::Read
//
//  Purpose:
//      Takes in a character string and reads the null data out of it.
//
//  Arguments:
//      input  the string to read
//
//  Returns:    The size of the null data bytes.
//
//  Programmer: Mark C. Miller 
//  Creation:   January 8, 2003 
//
// ****************************************************************************

int
avtNullDataReader::Read(char *input)
{
    int size = 0;

    //
    // Find out how long the NullData is.
    //
    int  length;
    memcpy(&length, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
 
    haveReadNullData = true;

    return size;
}

