// ************************************************************************* //
//                              avtImageReader.C                             //
// ************************************************************************* //

#include <avtImageReader.h>


// ****************************************************************************
//  Method: avtImageReader constructor
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
// ****************************************************************************

avtImageReader::avtImageReader()
{
    haveReadImage = false;
}


// ****************************************************************************
//  Method: avtImageReader::Read
//
//  Purpose:
//      Takes in a character string and reads the image out of it.
//
//  Arguments:
//      input  the string to read
//
//  Returns:    The size of the image in bytes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 18, 2000
//
// ****************************************************************************

int
avtImageReader::Read(char *input)
{
    int  size = 0;

    //
    // Find out how long the image is.
    //
    int  length;
    memcpy(&length, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
 
    //
    // Save the image as one of our data members.
    //
    imgRep = avtImageRepresentation(input, length);
    size += length;

    haveReadImage = false;

    GetOutputImage() = imgRep;
    return size;
}


// ****************************************************************************
//  Method: avtImageReader::FetchImage
//
//  Purpose:
//      Meets the interface of its base type and returns the image
//      representation to the base class.
//
//  Arguments:
//      <unnamed>   The data specification.  It is unclear how to use this with
//                  images right now.
//      rep         Should be assigned the image representation for this image.
//
//  Returns:        true if the image has changed since the last time it was
//                  fetched.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
// ****************************************************************************

bool
avtImageReader::FetchImage(avtDataSpecification_p ,avtImageRepresentation &rep)
{
    rep = imgRep;

    bool rv = !haveReadImage;
    haveReadImage = true;

    return rv;
}


