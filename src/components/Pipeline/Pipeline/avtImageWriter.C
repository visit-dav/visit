// ************************************************************************* //
//                              avtImageWriter.C                             //
// ************************************************************************* //

#include <avtImageWriter.h>

#include <avtDataObjectString.h>


// ****************************************************************************
//  Method: avtImageWriter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtImageWriter::avtImageWriter()
{
    ;
}


// ****************************************************************************
//  Method: avtImageWriter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtImageWriter::~avtImageWriter()
{
    ;
}


// ****************************************************************************
//  Method: avtImageWriter::DataObjectWrite
//
//  Arguments:
//      str         A string to append the image to.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 18, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Dec 28 16:33:13 PST 2000
//    Changed to account for changes in avtDataObjectWriter.
//
//    Hank Childs, Sat May 26 09:02:06 PDT 2001
//    Changed arguments to be an avtDataObjectString to get around
//    string::append bottleneck.
//
//    Hank Childs, Sun Sep 16 17:30:43 PDT 2001
//    Reflect new interface for data object strings.
//
//    Hank Childs, Mon Oct  1 09:31:50 PDT 2001
//    Renamed to DataObjectWrite.
//
//    Mark C. Miller, Mon Oct 31 18:12:49 PST 2005
//    Added code to support compression of data object string
//
// ****************************************************************************

void
avtImageWriter::DataObjectWrite(avtDataObjectString &str)
{
    avtImageRepresentation &image = GetImageRep();
    int length;
    unsigned char *imagestr = useCompression ?
                              image.GetCompressedImageString(length) :
                              image.GetImageString(length);

    //
    // Write out how long the image string is.
    //
    WriteInt(str, length);

    //
    // Append on our image.
    //
    str.Append((char *)imagestr, length,
               avtDataObjectString::DATA_OBJECT_STRING_DOES_NOT_OWN_REFERENCE);
}


