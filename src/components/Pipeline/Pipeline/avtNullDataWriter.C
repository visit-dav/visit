// ************************************************************************* //
//                           avtNullDataWriter.C                             //
// ************************************************************************* //

#include <avtNullDataWriter.h>

#include <avtDataObjectString.h>


// ****************************************************************************
//  Method: avtNullDataWriter::DataObjectWrite
//
//  Arguments:
//      str         A string to append the null data to.
//
//  Programmer: Mark C. Miller 
//  Creation:   January 7, 2003 
//
// ****************************************************************************

void
avtNullDataWriter::DataObjectWrite(avtDataObjectString &str)
{
    //
    // Write out how long the null data is.
    //
    WriteInt(str, 0);

}
