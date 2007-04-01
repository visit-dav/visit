// ************************************************************************* //
//                           avtNullDataWriter.C                             //
// ************************************************************************* //

#include <avtNullDataWriter.h>

#include <avtDataObjectString.h>


// ****************************************************************************
//  Method: avtNullDataWriter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNullDataWriter::avtNullDataWriter()
{
    ;
}


// ****************************************************************************
//  Method: avtNullDataWriter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNullDataWriter::~avtNullDataWriter()
{
    ;
}


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
