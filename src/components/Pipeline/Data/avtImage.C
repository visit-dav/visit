// ************************************************************************* //
//                                 avtImage.C                                //
// ************************************************************************* //

#include <avtImage.h>

#include <avtImageSource.h>
#include <avtImageWriter.h>


// ****************************************************************************
//  Method: avtImage constructor
//
//  Arguments:
//      src     An data object source that is the upstream object.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

avtImage::avtImage(avtDataObjectSource *src)
    : avtDataObject(src)
{
    ;
}


// ****************************************************************************
//  Method: avtImage::SetImage
//
//  Purpose:
//      Sets this objects image.  This is a protected method that should only
//      be accessed by avtImageSource.
//
//  Arguments:
//      ir      The image representation of the new image.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

void
avtImage::SetImage(const avtImageRepresentation &ir)
{
    image = ir;
}


// ****************************************************************************
//  Method: avtImage::GetImage
//
//  Purpose:
//      Gets the image as an avtImageRepresentation.
//
//  Returns:    the image.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

avtImageRepresentation &
avtImage::GetImage(void)
{
    return image;
}


// ****************************************************************************
//  Method: avtImage::InstantiateWriter
//
//  Purpose:
//      Instantiates a writer that is appropriate for an avtImage.
//
//  Programmer: Hank Childs
//  Creation:   October 1, 2001
//
// ****************************************************************************

avtDataObjectWriter *
avtImage::InstantiateWriter(void)
{
    return new avtImageWriter;
}


// ****************************************************************************
//  Method: avtImage::ReleaseData
//
//  Purpose:
//      Free the data associated with this image.
//
//  Programmer: Hank Childs
//  Creation:   November 5, 2001
//
// ****************************************************************************

void
avtImage::ReleaseData(void)
{
    image.ReleaseData();
}


// ****************************************************************************
//  Method: avtImage::DerivedCopy
//
//  Purpose:
//      Copy over the image.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2001
//
// ****************************************************************************

void
avtImage::DerivedCopy(avtDataObject *dob)
{
    avtImage *img = (avtImage *) dob;
    image = img->image;
}


