// ************************************************************************* //
//                              avtImageSource.C                             //
// ************************************************************************* //

#include <avtImageSource.h>

#include <vtkImageData.h>


// ****************************************************************************
//  Method: avtImageSource constructor
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
// ****************************************************************************

avtImageSource::avtImageSource()
{
    image = new avtImage(this);
}


// ****************************************************************************
//  Method: avtImageSource destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtImageSource::~avtImageSource()
{
    ;
}


// ****************************************************************************
//  Method: avtImageSource::GetOutput
//
//  Purpose:
//      Gets the output avtImage as an avtDataObject.
//
//  Returns:    The source's image, typed as a data object.
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 15:27:03 PST 2002
//    Changed CopyTo to an inline template function.
//
// ****************************************************************************

avtDataObject_p
avtImageSource::GetOutput(void)
{
    avtDataObject_p rv;
    CopyTo(rv, image);

    return rv;
}


// ****************************************************************************
//  Method: avtImageSource::GetVTKOutput
//
//  Purpose:
//      Gets the output as a vtkImageData.
//
//  Returns:    The output of the source as a VTK object.
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
// ****************************************************************************

vtkImageData *
avtImageSource::GetVTKOutput(void)
{
    return image->GetImage().GetImageVTK();
}


// ****************************************************************************
//  Method: avtImageSource::SetOutputImage
//
//  Purpose:
//      Sets the output image.
//
//  Arguments:
//      img     The image as an avtImageRepresentation.
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
// ****************************************************************************

void
avtImageSource::SetOutputImage(const avtImageRepresentation &img)
{
    image->SetImage(img);
}


// ****************************************************************************
//  Method: avtImageSource::SetOutput
//
//  Purpose:
//      Sets the output to be the same as the argument.
//
//  Arguments:
//      img     The new image.
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
// ****************************************************************************

void
avtImageSource::SetOutput(avtImage_p img)
{
    image->SetImage(img->GetImage());
}


// ****************************************************************************
//  Method: avtImageSource::GetOutputImage
//
//  Purpose:
//      Gets the output image.
//
//  Returns:    The image as an avtImageRepresentation.
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2001
//
// ****************************************************************************

avtImageRepresentation &
avtImageSource::GetOutputImage(void)
{
    return image->GetImage();
}


