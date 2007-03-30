// ************************************************************************* //
//                           avtSourceFromImage.C                            //
// ************************************************************************* //

#include <avtSourceFromImage.h>

#include <vtkImageData.h>

#include <avtImage.h>


// ****************************************************************************
//  Method: avtSourceFromImage constructor
//
//  Arguments:
//     img      The image.
//     zb       The zbuffer.  This is owned by this object after this call.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2001
//
// ****************************************************************************

avtSourceFromImage::avtSourceFromImage(vtkImageData *img, float *zb)
{
    image = img;
    if (image != NULL)
    {
        image->Register(NULL);
    }
    zbuffer = zb;
}


// ****************************************************************************
//  Method: avtSourceFromImage destructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2001
//
// ****************************************************************************

avtSourceFromImage::~avtSourceFromImage()
{
    if (image != NULL)
    {
        image->Delete();
    }

    //
    // The zbuffer is not deleted here because its output image owns it.  This
    // could be a problem if the output image has never claimed it, though.
    //
}


// ****************************************************************************
//  Method: avtSourceFromImage::SetImage
//
//  Purpose:
//      Set the image for this source.
//
//  Arguments:
//      img     The new image.
//      zb      The zbuffer.  This is owned by this object after this call.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2001
//
// ****************************************************************************

void
avtSourceFromImage::SetImage(vtkImageData *img, float *zb)
{
    if (image != NULL)
    {
        image->Delete();
    }

    image = img;
    zbuffer = zb;

    if (image != NULL)
    {
        image->Register(NULL);
    }
}


// ****************************************************************************
//  Method: avtSourceFromImage::FetchImage
//
//  Purpose:
//      This copies the image (or shares a reference) to the output image.
//
//  Arguments:
//      spec    The data specification.  It is unclear to me how to use this
//              right now.
//      rep     The image representation to copy this into.
//
//  Returns:    Should return something based on whether the image has changed,
//              but it always returns false for now.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2001
//
// ****************************************************************************

bool
avtSourceFromImage::FetchImage(avtDataSpecification_p,
                               avtImageRepresentation &rep)
{
    if (zbuffer != NULL)
    {
        rep = avtImageRepresentation(image, zbuffer);
    }
    else
    {
        rep = avtImageRepresentation(image);
    }

    return false;
}


