// ************************************************************************* //
//                        avtSourceFromAVTImage.C                          //
// ************************************************************************* //

#include <avtSourceFromAVTImage.h>

#include <avtCommonDataFunctions.h>
#include <avtImage.h>
#include <avtSILRestrictionTraverser.h>

#include <TimingsManager.h>

using     std::vector;

avtDataObject_p   DataObjectFromImage(avtImage_p);


// ****************************************************************************
//  Method: avtSourceFromAVTImage constructor
//
//  Arguments:
//      img     An avtImage_p
//
//  Programmer: Mark C. Miller
//  Creation:   June 19, 2001
//
// ****************************************************************************

avtSourceFromAVTImage::avtSourceFromAVTImage(avtImage_p img)
    : avtInlinePipelineSource(DataObjectFromImage(img))
{
    image = img->GetImage();
    GetTypedOutput()->GetInfo().Copy(img->GetInfo());
}


// ****************************************************************************
//  Method: avtSourceFromAVTImage destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSourceFromAVTImage::~avtSourceFromAVTImage()
{
    ;
}


// ****************************************************************************
//  Method: avtSourceFromAVTImage::FetchImage
//
//  Purpose:
//      Gets the image 
//
//  Arguments:
//      img    The output image.
//
//  Returns:    Whether or not this dataset is different from the last time it
//              was updated.
//       
//  Programmer: Mark C. Miller
//  Creation:   June 19, 2001
//
// ****************************************************************************

bool
avtSourceFromAVTImage::FetchImage(avtDataSpecification_p spec,
                          avtImageRepresentation &outImage)
{
    int timingsHandle = visitTimer->StartTimer();

    outImage = image;

    visitTimer->StopTimer(timingsHandle, "Fetching image AVT image");

    return false;
}


// ****************************************************************************
//  Function: DataObjectFromImage
//
//  Purpose:
//      Creates a pointer typed to the base class.  This should happen free
//      with C++, but does not because of reference pointers.
//
//  Programmer: Mark C. Miller
//  Creation:   September 11, 2001
//
// ****************************************************************************

avtDataObject_p
DataObjectFromImage(avtImage_p img)
{
    avtDataObject_p rv;
    CopyTo(rv, img);

    return rv;
}


