// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtImageRepresentation.C                           //
// ************************************************************************* //
#include <vtkAbstractArray.h>
#include <vtkCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPointsWriter.h>

#include <avtImageRepresentation.h>
#include <avtCommonDataFunctions.h>
#include <NoInputException.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Function: CreateStringFromVTKInput
//
//  Purpose:
//      Creates a string from a vtk input.
//
//  Arguments:
//      img     The image.
//      str     The string the image is written on.
//      len     The length of string.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Jan 22 15:57:19 PST 2004
//    Remove image clip because it has a dependence on vtkImaging library.
//
// ****************************************************************************

static
void
CreateStringFromVTKInput(vtkImageData *img, unsigned char *&str, int &len)
{
    //
    // Keep Update from propagating.
    //
    vtkImageData *tmp = vtkImageData::New();
    tmp->ShallowCopy(img);

    vtkStructuredPointsWriter *writer = vtkStructuredPointsWriter::New();
    writer->SetFileTypeToBinary();
    writer->WriteToOutputStringOn();
    writer->SetInputData(tmp);
    writer->SetFileTypeToBinary();
    writer->Write();

    len = writer->GetOutputStringLength();
    str = (unsigned char *) writer->RegisterAndGetOutputString();

    writer->Delete();
    tmp->Delete();
}

// ****************************************************************************
//  Function: CreateStringFromInput
//
//  Purpose:
//      Creates a string from the input (a vtk image and a zbuffer).
//
//  Arguments:
//      img      The VTK image.
//      zbuffer  The zbuffer.
//      str      A string that contains img and zbuffer.
//      len      The length of str.
//
//  Programmer:  Hank Childs
//  Creation:    February 13, 2001
//
//  Modifications:
//
//    Hank Childs, Sat Jan  5 11:32:03 PST 2002
//    Clean up memory leak.
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    use a vtk data array to store the z-buffer.
//
// ****************************************************************************

static
void
CreateStringFromInput(vtkImageData *img, vtkFloatArray *zbuf, unsigned char *&str,
                      int &len)
{
    // make a shallow copy for if we need to attach zbuffer data
    vtkImageData *tmp  = vtkImageData::New();
    tmp->ShallowCopy(img);

    // add the zbuffer as point data if we have one
    if (zbuf)
       tmp->GetPointData()->AddArray(zbuf);

    CreateStringFromVTKInput(tmp, str, len);

    tmp->Delete();
}





// ****************************************************************************
//  Method: avtImageRepresentation default constructor
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

avtImageRepresentation::avtImageRepresentation()
{
    Initialize();
}

// ****************************************************************************
//  Method: avtImageRepresentation constructor
//
//  Programmer: Brad Whitlock
//  Creation:  Thu Feb  1 16:27:29 PST 2018
//
// ****************************************************************************

avtImageRepresentation::avtImageRepresentation(int w, int h, int ncomp)
{
    Initialize();

    asVTK = NewImage(w, h, ncomp);

    zbuffer = vtkFloatArray::New();
    zbuffer->SetNumberOfTuples(w*h);
    zbuffer->SetName("zbuffer");
}


// ****************************************************************************
//  Method: avtImageRepresentation constructor
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

avtImageRepresentation::avtImageRepresentation(vtkImageData *d)
{
    Initialize();

    asVTK = d;

    if (asVTK)
        asVTK->Register(NULL);
}

// ****************************************************************************
//  Method: avtImageRepresentation constructor
//
//  Programmer: Burlen Loring
//  Creation:  Thu Sep  3 13:00:33 PDT 2015
//
// ****************************************************************************

avtImageRepresentation::avtImageRepresentation(
    vtkImageData *im, vtkFloatArray *z)
{
    Initialize();

    asVTK = im;
    if (asVTK)
        asVTK->Register(NULL);

    zbuffer = z;
    if (zbuffer)
    {
        zbuffer->SetName("zbuffer");
        zbuffer->Register(NULL);
    }
}


// ****************************************************************************
//  Method: avtImageRepresentation constructor
//
//  Arguments:
//      d       The image
//      z       The z-buffer.  The image representation owns this arrays after
//              this call (it will delete it -- not the client).
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Feb  6 14:59:43 PST 2006
//    Allow the z-buffer to be directly acquired instead of copied.
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    use a vtk data array to store the z-buffer.
//
//    Kathleen Biagas, Tue May 10 17:03:10 PDT 2016
//    Use vtkAbstractArray::VTK_DATA_ARRAY_DELETE.
//
// ****************************************************************************

avtImageRepresentation::avtImageRepresentation(vtkImageData *d, float *z,
                                               bool take)
{
    Initialize();

    asVTK = d;

    if (!asVTK)
        return;

    asVTK->Register(NULL);

    if (z)
    {
        zbuffer = vtkFloatArray::New();
        zbuffer->SetName("zbuffer");
        const int *dims = asVTK->GetDimensions();
        int npix = dims[0]*dims[1];
       if (take)
           zbuffer->SetArray(z, npix, 0, vtkAbstractArray::VTK_DATA_ARRAY_DELETE);
       else
           memcpy(zbuffer->WritePointer(0, npix), z, npix*sizeof(float));
    }
}


// ****************************************************************************
//  Method: avtImageRepresentation constructor
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

avtImageRepresentation::avtImageRepresentation(char *d, int dl)
{
    Initialize();

    asCharLength = dl;
    if (asCharLength > 0 && d != NULL)
    {
       asChar = new unsigned char[asCharLength];
       memcpy(asChar, d, asCharLength * sizeof(char));
       asCharRef = new int(1);
    }
}


// ****************************************************************************
//  Method: avtImageRepresentation copy constructor
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
//  Modifications:
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    initialize the object before copying.
//
// ****************************************************************************

avtImageRepresentation::avtImageRepresentation(const avtImageRepresentation &r)
{
    Initialize();
    Copy(r);
}


// ****************************************************************************
//  Method: avtImageRepresentation destructor
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

avtImageRepresentation::~avtImageRepresentation()
{
    DestructSelf();
}


// ****************************************************************************
//  Method: avtImageRepresentation::ReleaseData
//
//  Purpose:
//      Releases data the data associated with the image rep.
//
//  Programmer: Hank Childs
//  Creation:   November 5, 2001
//
// ****************************************************************************

void
avtImageRepresentation::ReleaseData(void)
{
    DestructSelf();
    Initialize();
}


// ****************************************************************************
//  Method: avtImageRepresentation::Initialize
//
//  Purpose:
//      Initializes all of the data members to NULL/0.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
//  Modifications:
//
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added compression data members
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    use a vtk data array to store the z-buffer.
//
//
// ****************************************************************************

void
avtImageRepresentation::Initialize(void)
{
    asVTK        = NULL;
    zbuffer      = NULL;
    asChar       = NULL;
    asCharLength = 0;
    asCharRef    = NULL;
    rowOrigin    = 0;
    colOrigin    = 0;

    compressionRatio = -1.0;
    timeToCompress   = -1.0;
    timeToDecompress = -1.0;
}


// ****************************************************************************
//  Method: avtImageRepresentation::Copy
//
//  Purpose:
//      Copies the argument into this object.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
//  Modifications:
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    use a vtk data array to store the z-buffer.
//
// ****************************************************************************

void
avtImageRepresentation::Copy(const avtImageRepresentation &r)
{
    if (&r == this)
        return;

    ReleaseData();

    asVTK = r.asVTK;
    if (asVTK)
        asVTK->Register(NULL);

    zbuffer = r.zbuffer;
    if (zbuffer)
        zbuffer->Register(NULL);

    asChar = r.asChar;
    if (asChar)
    {
        asCharLength = r.asCharLength;
        asCharRef    = r.asCharRef;
        (*asCharRef)++;
    }

    rowOrigin = r.rowOrigin;
    colOrigin = r.colOrigin;

    compressionRatio = r.compressionRatio;
    timeToCompress = r.timeToCompress;
    timeToDecompress = r.timeToDecompress;
}

// ****************************************************************************
//  Method: avtImageRepresentation::SetZBuffer
//
//  Purpose:
//     Sets the z-buffer from the object passed in.
//
//  Programmer: Burlen Loring
//  Creation:   Tue Sep  1 07:28:09 PDT 2015
//
// ****************************************************************************

void
avtImageRepresentation::SetZBufferVTK(vtkFloatArray *z)
{
    if (z == zbuffer)
        return;

    if (zbuffer)
        zbuffer->Delete();

    zbuffer = z;

    if (zbuffer)
    {
        zbuffer->Register(NULL);
        zbuffer->SetName("zbuffer");
    }
}


// ****************************************************************************
//  Method: avtImageRepresentation::SetImageVTK
//
//  Purpose
//     Set the image
//
//  Programmer: Burlen Loring
//  Creation:   Tue Sep  1 07:28:09 PDT 2015
//
// ****************************************************************************

void
avtImageRepresentation::SetImageVTK(vtkImageData *im)
{
    if (im == asVTK)
        return;

    if (asVTK)
        asVTK->Delete();

    asVTK = im;
    if (asVTK)
        asVTK->Register(NULL);

    if (asChar)
    {
        (*asCharRef)--;
        if (! *asCharRef)
        {
            delete [] asChar;
            delete asCharRef;
        }
    }

    asChar = NULL;
    asCharRef = NULL;
    asCharLength = 0;
}

// ****************************************************************************
//  Method: avtImageRepresentation::DestructSelf
//
//  Purpose:
//      Reduces all of the reference counts that this object currently has.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
//  Modifications:
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    use a vtk data array to store the z-buffer.
//
// ****************************************************************************

void
avtImageRepresentation::DestructSelf(void)
{
    if (asVTK)
        asVTK->Delete();

    if (zbuffer)
        zbuffer->Delete();

    if (asChar)
    {
        (*asCharRef)--;
        if (! *asCharRef)
        {
            delete [] asChar;
            delete asCharRef;
        }
    }

    Initialize();
}


// ****************************************************************************
//  Method: avtImageRepresentation::operator=
//
//  Purpose:
//      Sets the image from another.
//
//  Arguments:
//      rhs     the source avtImageRepresentation
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Dec 18 10:25:09 PST 2007
//    Remove const qualification of return type.
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    let copy handle freeing the existing data
//
// ****************************************************************************

avtImageRepresentation &
avtImageRepresentation::operator=(const avtImageRepresentation &rhs)
{
    Copy(rhs);
    return *this;
}


// ****************************************************************************
//  Method: avtImageRepresentation::Valid
//
//  Purpose:
//      Determines if this object is valid.
//
//  Returns:    true if the object is valid, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

bool
avtImageRepresentation::Valid(void)
{
    return (asVTK != NULL || asChar != NULL ? true : false);
}


// ****************************************************************************
//  Method: avtImageRepresentation::GetImageString
//
//  Purpose: Public interface to GetImageString
//
//  Programmer: Mark C. Miller 
//  Creation:   October 24, 2005
//
// ****************************************************************************

unsigned char *
avtImageRepresentation::GetImageString(int &length)
{
    const bool useCompression = false;
    return GetImageString(length, useCompression);
}

// ****************************************************************************
//  Method: avtImageRepresentation::GetCompressedImageString
//
//  Purpose: Public interface to GetImageString
//
//  Programmer: Mark C. Miller 
//  Creation:   October 24, 2005
//
// ****************************************************************************

unsigned char *
avtImageRepresentation::GetCompressedImageString(int &length)
{
    const bool useCompression = true;
    return GetImageString(length, useCompression);
}

// ****************************************************************************
//  Method: avtImageRepresentation::GetImageString
//
//  Purpose:
//      Gets the image as a character string.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
//  Modifications:
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added compression
//
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Changed to use common data function for compression 
// ****************************************************************************

unsigned char *
avtImageRepresentation::GetImageString(int &length, bool compress)
{
    if (asChar == NULL)
    {
        if (asVTK == NULL)
        {
            EXCEPTION0(NoInputException);
        }

        CreateStringFromInput(asVTK, zbuffer, asChar, asCharLength);
        asCharRef = new int(1);
    }

    if (compress)
    {
        int asCharLengthNew = 0;
        unsigned char *asCharNew = 0;

        if (CCompressDataString(asChar, asCharLength,
                                &asCharNew, &asCharLengthNew,
                                &timeToCompress, &compressionRatio))
        {
            delete [] asChar;
            asChar = asCharNew;
            asCharLength = asCharLengthNew;
        }
    }

    length = asCharLength;
    return asChar;
}


// ****************************************************************************
//  Method: avtImageRepresentation::GetImageVTK
//
//  Purpose:
//      Gets the image as a vtkImageData.
//
//  Returns:      The image as a vtkImageData.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
//  Modifications:
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    use a vtk data array to store the z-buffer.
//
// ****************************************************************************

vtkImageData *
avtImageRepresentation::GetImageVTK(void)
{
    if (asVTK == NULL)
    {
        if (asChar == NULL)
        {
            EXCEPTION0(NoInputException);
        }
        GetImageFromString(asChar, asCharLength, asVTK, zbuffer);
    }
    return asVTK;
}


// ****************************************************************************
//  Method: avtImageRepresentation::GetZBuffer
//
//  Purpose:
//      Gets the z-buffer of the image.
//
//  Returns:    The z-buffer for the image, if it exists.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
//  Modifications:
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    use a vtk data array to store the z-buffer.
//    call GetImageVTK to marshal if necessary
//
// ****************************************************************************

float *
avtImageRepresentation::GetZBuffer(void)
{
    // Not all images have z-buffers.  If we have a VTK image and no z-buffer
    // then there is no z-buffer.  If we don't have the VTK image, we should
    // parse out the image and potentially the z-buffer from the marshall
    // string.
    GetImageVTK();
    if (zbuffer)
        return zbuffer->GetPointer(0);
    return NULL;
}

// ****************************************************************************
//  Method: avtImageRepresentation::GetZBufferVTK
//
//  Purpose:
//      Gets the z-buffer of the image.
//
//  Returns:    The z-buffer for the image, if it exists.
//
//  Programmer: Burlen Loring
//  Creation:   Fri Sep  4 11:52:29 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

vtkFloatArray *
avtImageRepresentation::GetZBufferVTK()
{
    GetImageVTK(); // for marshalling from string
    if (zbuffer)
        return zbuffer;
    return NULL;
}


// ****************************************************************************
//  Method: avtImageRepresentation::GetRGBBuffer
//
//  Purpose:
//      Gets the rgb-buffer of the image.
//
//  Returns:    The rgb-buffer for the image, if it exists.
//
//  Programmer: Mark C. Miller 
//  Creation:   04Mar03 
//
//  Modifications:
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    use a vtk data array to store the z-buffer.
//
// ****************************************************************************
unsigned char *
avtImageRepresentation::GetRGBBuffer(void)
{
    GetImageVTK(); // for marshalling from string
    return (unsigned char *)asVTK->GetScalarPointer(0, 0, 0);
}

// ****************************************************************************
//  Method: avtImageRepresentation::GetRGBBuffer
//
//  Purpose:
//      Gets the rgb-buffer of the image.
//
//  Returns:    The rgb-buffer for the image, if it exists.
//
//  Programmer: Burlen Loring
//  Creation:   Fri Sep  4 12:53:18 PDT 2015
//
// ****************************************************************************
vtkUnsignedCharArray *
avtImageRepresentation::GetRGBBufferVTK()
{
    GetImageVTK(); // for marshalling from string
    return dynamic_cast<vtkUnsignedCharArray*>(
        asVTK->GetPointData()->GetArray("ImageScalars"));
}

// ****************************************************************************
//  Method: avtImageRepresentation::GetNumberOfColorChannels
//
//  Purpose:
//      Gets then number of color channels, 3 for rgb
//      and 4 for rgba
//
//  Programmer: Burlen Loring
//  Creation:   Fri Sep  4 11:49:33 PDT 2015
//
//  Modifications:
//
// ****************************************************************************
int
avtImageRepresentation::GetNumberOfColorChannels()
{
    GetImageVTK(); // for marshalling from string
    return
    asVTK->GetPointData()->GetArray("ImageScalars")->GetNumberOfComponents();
}


// ****************************************************************************
//  Method: avtImageRepresentation::NewImage
//
//  Purpose:
//      A static method that creates an image of the specified size.  Intended
//      to make this routine be a single point of source.
//
//  Arguments:
//      width    The width of the image.
//      height   The height of the image.
//
//  Returns:     A vtkImageData of size width x height.
//
//  Programmer:  Hank Childs
//  Creation:    January 25, 2001
//
//  Modifications:
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    use a vtk data array to store the z-buffer.
//    make the number of color channels a parameter so that
//    we can also store rgba images
//
// ****************************************************************************

vtkImageData *
avtImageRepresentation::NewImage(int width, int height, int nchan)
{
    vtkImageData *image = vtkImageData::New();
    image->SetExtent(0, width-1, 0, height-1, 0, 0);
    image->SetSpacing(1., 1., 1.);
    image->SetOrigin(0., 0., 0.);
    image->AllocateScalars(VTK_UNSIGNED_CHAR, nchan);
    return image;
}

// ****************************************************************************
// Method: avtImageRepresentation::NewValueImage
//
// Purpose:
//   Create a new value image.
//
// Arguments:
//      width    The width of the image.
//      height   The height of the image.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 12:56:42 PDT 2017
//
// Modifications:
//
// ****************************************************************************

vtkImageData *
avtImageRepresentation::NewValueImage(int width, int height)
{
    vtkImageData *image = vtkImageData::New();
    image->SetExtent(0, width-1, 0, height-1, 0, 0);
    image->SetSpacing(1., 1., 1.);
    image->SetOrigin(0., 0., 0.);
    image->AllocateScalars(VTK_FLOAT, 1);
    return image;
}

// ****************************************************************************
//  Function: GetImageFromString
//
//  Purpose:
//      Takes in a string and returns an image and a zbuffer if one is there.
//
//  Arguments:
//      str     The input string.
//      len     The length of the input str.
//      img     The image to populate.
//      zbuffer The zbuffer to populate (if there is one in str).
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2001
//
//  Modifications:
//
//     Mark C. Miller, 22Jul03
//     Re-wrote to avoid use of memcpy. 
//
//     Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//     Added compression support
//
//     Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//     Changed to use common data function for compression 
//
//     Kathleen Bonnell, Wed May 17 14:51:16 PDT 2006
//     Removed call to SetSource(NULL) as it now removes information necessary
//     for the dataset.
//
//    Kathleen Biagas, Fri Jan 25 16:40:41 PST 2013
//    Call update on reader, not data object.
//
//    Burlen Loring, Tue Sep  1 07:28:33 PDT 2015
//    use a vtk data array to store the z-buffer.
//
// ****************************************************************************

void
avtImageRepresentation::GetImageFromString(unsigned char *str,
        int strLength, vtkImageData *&img, vtkFloatArray *&zbuf)
{

    int strLengthNew = 0;
    unsigned char *strNew = 0;
    if (CDecompressDataString(str, strLength, &strNew, &strLengthNew,
                              &timeToCompress, &timeToDecompress,
                              &compressionRatio))
    {
        delete [] str;
        str = strNew;
        strLength = strLengthNew;

        // update the object, too
        asChar = strNew;
        asCharLength = strLengthNew;
    }

    // read the string assuming its just an image
    vtkStructuredPointsReader *reader = vtkStructuredPointsReader::New();
    vtkCharArray *charArray = vtkCharArray::New();
    int iOwnIt = 1;  // 1 means we own it -- you don't delete it.
    charArray->SetArray((char *) str, strLength, iOwnIt);
    reader->SetReadFromInputString(1);
    reader->SetInputArray(charArray);
    reader->Update();

    if (img)
        img->Delete();

    img = reader->GetOutput();
    img->SetScalarType(VTK_UNSIGNED_CHAR, img->GetInformation());
    img->Register(NULL);

    reader->Delete();
    charArray->Delete();

    // If there is a "zbuffer" point data array, then get it too
    if (zbuf)
        zbuf->Delete();

    zbuf = dynamic_cast<vtkFloatArray*>(
        img->GetPointData()->GetArray("zbuffer"));

    if (zbuf)
    {
        zbuf->Register(NULL);
        img->GetPointData()->RemoveArray("zbuffer");
    }
}

// ****************************************************************************
//  Method: GetSize
//
//  Purpose: Return the size of the image in rows and columns of pixels
//
//  Programmer: Mark C. Miller 
//  Creation:   25Feb03 
//
//  Modifications:
//
//    Burlen Loring, Fri Sep  4 12:48:47 PDT 2015
//    Use GetImageVTK to do marshalling
//
// ****************************************************************************

void
avtImageRepresentation::GetSize(int *_rowSize, int *_colSize)
{
   // What do we do if the image hasn't been parsed out of the string
   // when this function is called? In the other query functions of this
   // class, we automatically call GetImageFromString on the caller's
   // behalf. However, all those calls ultimately need that to occur
   // to return the object a caller is looking for; a vtkImageData object
   // or a z-buffer. Here, we only want to return two ints representing
   // the size. So, we wind up doing problem sized work to satisfy
   // the query. Nonetheless, its a good assumption the client actually
   // wants some that data sometime in the future anyway.
    GetImageVTK();

    int *imageDims = asVTK->GetDimensions();
    *_rowSize = imageDims[1]; // #rows is y-size
    *_colSize = imageDims[0]; // #cols is x-size
}

// ****************************************************************************
//  Method: GetSize
//
//  Purpose: Return the size of the image in rows and columns of pixels
//
//  Programmer: Mark C. Miller 
//  Creation:   31Mar04 
//
// ****************************************************************************

void
avtImageRepresentation::GetSize(int *_rowSize, int *_colSize) const
{
    if (asVTK == NULL)
    {
        if (asChar == NULL)
        {
            EXCEPTION0(NoInputException);
        }

        *_rowSize = 0;
        *_colSize = 0;
        return;
    }

    int *imageDims = asVTK->GetDimensions();
    *_rowSize = imageDims[1]; // #rows is y-size
    *_colSize = imageDims[0]; // #cols is x-size
}

// ****************************************************************************
//  Method: GetNumberOfCells 
//
//  Purpose: Return the number of cells (pixels) in the image 
//
//  Programmer: Mark C. Miller 
//  Creation:   19Aug03 
//
//  Modifications:
//
//    Mark C. Miller, Wed Nov  5 09:48:13 PST 2003
//    Added option to count polygons only
//
//    Burlen Loring, Sun Sep  6 14:58:03 PDT 2015
//    Changed the return type of GetNumberOfCells to long long
//
// ****************************************************************************

long long
avtImageRepresentation::GetNumberOfCells(bool polysOnly) const
{
    if (asVTK == NULL)
    {
        if (asChar == NULL)
        {
            EXCEPTION0(NoInputException);
        }
        return -1;
   }
   else
   {
      if (polysOnly)
         return 0;
      else
         return asVTK->GetNumberOfCells();
   }
}

// ****************************************************************************
//  Method: GetCompressionRatio
//
//  Purpose: Return the compression ratio, if any, w/o uncompressing string
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005 
//
//  Modification:
//
//    Burlen Loring, Fri Sep  4 12:49:59 PDT 2015
//    initialize the return to fix an error reported by valgrind
//
// ****************************************************************************

float
avtImageRepresentation::GetCompressionRatio() const
{
    if (compressionRatio != -1.0)
        return compressionRatio;

    if (asChar != NULL)
    {
        float ratioc = 1.0f;
        CGetCompressionInfoFromDataString(asChar, asCharLength,
            0, &ratioc);
        return ratioc;
    }

    return compressionRatio;
}

// ****************************************************************************
//  Method: GetTimeToCompress
//
//  Purpose: Return the compression time, if any, w/o uncompressing string
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005 
//
//  Modification:
//
//    Burlen Loring, Fri Sep  4 12:49:59 PDT 2015
//    initialize the return to fix an error reported by valgrind
//
// ****************************************************************************

float
avtImageRepresentation::GetTimeToCompress() const
{
    if (timeToCompress != -1.0)
        return timeToCompress;

    if (asChar != NULL)
    {
        float timec = 0.0f;
        CGetCompressionInfoFromDataString(asChar, asCharLength,
            &timec, 0);
        return timec;
    }

    return timeToCompress;
}

// ****************************************************************************
//  Method: GetTimeToDecompress 
//
//  Purpose: Return the decompression time, if any, w/o uncompressing string
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005 
//
// ****************************************************************************

float
avtImageRepresentation::GetTimeToDecompress() const
{
    return timeToDecompress;
}
