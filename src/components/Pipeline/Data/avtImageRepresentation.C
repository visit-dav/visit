// ************************************************************************* //
//                        avtImageRepresentation.C                           //
// ************************************************************************* //

#include <vtkImageData.h>
#include <vtkImageClip.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPointsWriter.h>

#include <avtImageRepresentation.h>
#include <NoInputException.h>
#include <ImproperUseException.h>


//
// Function Prototypes
//

void   CreateStringFromInput(vtkImageData *, float *, unsigned char *&, int &);
void   CreateStringFromVTKInput(vtkImageData *, char *&, int &);
void   GetImageFromString(unsigned char *, vtkImageData *&, float *&);


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
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

avtImageRepresentation::avtImageRepresentation(vtkImageData *d)
{
    Initialize();

    asVTK        = d;
    if (asVTK != NULL)
    {
        asVTK->Register(NULL);
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
// ****************************************************************************

avtImageRepresentation::avtImageRepresentation(vtkImageData *d, float *z)
{
    Initialize();

    asVTK        = d;
    if (asVTK != NULL)
    {
        asVTK->Register(NULL);
    }

    zbuffer      = z;
    zbufferRef   = new int(1);
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
    asChar = new unsigned char[asCharLength];
    for (int i = 0 ; i < asCharLength ; i++)
        asChar[i] = d[i];

    asCharRef = new int(1);
}


// ****************************************************************************
//  Method: avtImageRepresentation copy constructor
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

avtImageRepresentation::avtImageRepresentation(const avtImageRepresentation &r)
{
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
// ****************************************************************************

void
avtImageRepresentation::Initialize(void)
{
    asVTK        = NULL;
    zbuffer      = NULL;
    zbufferRef   = NULL;
    asChar       = NULL;
    asCharLength = 0;
    asCharRef    = NULL;
    rowOrigin    = 0;
    colOrigin    = 0;
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
// ****************************************************************************

void
avtImageRepresentation::Copy(const avtImageRepresentation &r)
{
    Initialize();

    if (r.asVTK)
    {
        asVTK  = r.asVTK;
        asVTK->Register(NULL);
    }
    if (r.zbuffer)
    {
        zbuffer    = r.zbuffer;
        zbufferRef = r.zbufferRef;
        (*zbufferRef)++;
    }
    if (r.asChar)
    {
        asChar       = r.asChar;
        asCharLength = r.asCharLength; 
        asCharRef    = r.asCharRef;
        (*asCharRef)++;
    }
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
// ****************************************************************************

void
avtImageRepresentation::DestructSelf(void)
{
    if (asVTK)
    {
        asVTK->Delete();
        asVTK = NULL;
    }
    if (zbuffer)
    {
        (*zbufferRef)--;
        if (*zbufferRef <= 0)
        {
            delete [] zbuffer;
            delete zbufferRef;
        }
        zbuffer    = NULL;
        zbufferRef = NULL;
    }
    if (asChar)
    {
        (*asCharRef)--;
        if (! *asCharRef)
        {
            delete [] asChar;
            delete asCharRef;
        }
        asChar = NULL;
        asCharRef = NULL;
    }
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
// ****************************************************************************

const avtImageRepresentation &
avtImageRepresentation::operator=(const avtImageRepresentation &rhs)
{
    DestructSelf();
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
//  Purpose:
//      Gets the image as a character string.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2000
//
// ****************************************************************************

unsigned char *
avtImageRepresentation::GetImageString(int &length)
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

        GetImageFromString(asChar, asVTK, zbuffer);
        if (zbuffer != NULL)
        {
            zbufferRef = new int(1);
        }
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
// ****************************************************************************

float *
avtImageRepresentation::GetZBuffer(void)
{
    //
    // Not all images have z-buffers.  If we have a VTK image and no z-buffer
    // then there is no z-buffer.  If we don't have the VTK image, we should
    // parse out the image and potentially the z-buffer from the marshall
    // string.
    //
    if (asVTK == NULL)
    {
        if (asChar == NULL)
        {
            EXCEPTION0(NoInputException);
        }

        GetImageFromString(asChar, asVTK, zbuffer);
        if (zbuffer != NULL)
        {
            zbufferRef = new int(1);
        }
    }

    return zbuffer;
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
// ****************************************************************************
unsigned char *
avtImageRepresentation::GetRGBBuffer(void)
{
    if (asVTK == NULL)
    {
        if (asChar == NULL)
        {
            EXCEPTION0(NoInputException);
        }

        GetImageFromString(asChar, asVTK, zbuffer);
        if (zbuffer != NULL)
        {
            zbufferRef = new int(1);
        }
    }

    return (unsigned char *)asVTK->GetScalarPointer(0, 0, 0);
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
// ****************************************************************************

vtkImageData *
avtImageRepresentation::NewImage(int width, int height)
{
    vtkImageData *image = vtkImageData::New();
    image->SetWholeExtent(0, width-1, 0, height-1, 0, 0);
    image->SetUpdateExtent(0, width-1, 0, height-1, 0, 0);
    image->SetExtent(0, width-1, 0, height-1, 0, 0);
    image->SetSpacing(1., 1., 1.);
    image->SetOrigin(0., 0., 0.);
    image->SetNumberOfScalarComponents(3);
    image->SetScalarType(VTK_UNSIGNED_CHAR);
    image->AllocateScalars();

    return image;
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
// ****************************************************************************

void
CreateStringFromInput(vtkImageData *img, float *zbuffer, unsigned char *&str,
                      int &len)
{
    //
    // Figure out what the width and height should be before we start altering
    // the image (the image should not be altered, but you never know with VTK)
    //
    int dims[3];
    img->GetDimensions(dims);
    int width  = dims[0];
    int height = dims[1];

    //
    // Get the VTK image data as a string.
    //
    char *imgStr    = NULL;
    int   imgLength = 0;
    CreateStringFromVTKInput(img, imgStr, imgLength);

    len = 0;

    // One int to say whether or not there is going to be a z-buffer
    len += sizeof(int);

    // Put the vtk image next.
    len += sizeof(int);
    len += imgLength;
 
    if (zbuffer != NULL)
    {
        // Put the width and height in the string
        len += 2*sizeof(int);

        // Allocate space for the z-buffer.
        len += (width*height)*sizeof(float);
    }

    str = new unsigned char[len];
    unsigned char *current = str;

    //
    // Put in a flag saying if there will be a zbuffer at the end.
    //
    int  zBufferFlag = (zbuffer != NULL ? 1 : 0);
    memcpy(current, &zBufferFlag, sizeof(int));
    current += sizeof(int);

    //
    // Copy over the vtk image.
    //
    memcpy(current, &imgLength, sizeof(int));
    current += sizeof(int);
    memcpy(current, imgStr, imgLength);
    current += imgLength;
    delete [] imgStr;

    //
    // If there is a zbuffer, copy it over.
    //
    if (zbuffer != NULL)
    {
        memcpy(current, &width, sizeof(int));
        current += sizeof(int);
        memcpy(current, &height, sizeof(int));
        current += sizeof(int);
        memcpy(current, zbuffer, (height*width)*sizeof(float));
        current += (height*width)*sizeof(float);
    }
}


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
// ****************************************************************************

void
CreateStringFromVTKInput(vtkImageData *img, char *&str, int &len)
{
    //
    // Technique for writing image to a string stolen from
    // vtkMultiProcessController.
    //
    vtkImageData *tmp  = vtkImageData::New();
    tmp->ShallowCopy(img);

    vtkImageClip *clip = vtkImageClip::New();
    clip->SetInput(tmp);
    clip->SetOutputWholeExtent(img->GetExtent());

    vtkStructuredPointsWriter *writer = vtkStructuredPointsWriter::New();
    writer->SetFileTypeToBinary();
    writer->WriteToOutputStringOn();
    writer->SetInput(clip->GetOutput());
    writer->SetFileTypeToBinary();
    writer->Write();

    len = writer->GetOutputStringLength();
    str = writer->RegisterAndGetOutputString();

    writer->Delete();
    clip->Delete();
    tmp->Delete();
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
// ****************************************************************************

void
GetImageFromString(unsigned char *str, vtkImageData *&img, float *&zbuffer)
{
    int isZBuffer;
    memcpy(&isZBuffer, str, sizeof(int));
    str += sizeof(int);
    
    int imgLength;
    memcpy(&imgLength, str, sizeof(int));
    str += sizeof(int);

    vtkStructuredPointsReader *reader = vtkStructuredPointsReader::New();
    reader->SetReadFromInputString(1);
    reader->SetInputString((char *) str, imgLength);
    img = reader->GetOutput();
    img->Register(NULL);
    img->Update();
    img->SetSource(NULL);
    reader->Delete();
    str += imgLength;

    if (isZBuffer != 0)
    {
        int width, height;
        memcpy(&width, str, sizeof(int));
        str += sizeof(int);
        memcpy(&height, str, sizeof(int));
        str += sizeof(int);

        zbuffer = new float[width*height];
        memcpy(&zbuffer, str, (width*height)*sizeof(float));
        str += (width*height)*sizeof(float);
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
//
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

    if (asVTK == NULL)
    {
        if (asChar == NULL)
        {
            EXCEPTION0(NoInputException);
        }

        GetImageFromString(asChar, asVTK, zbuffer);
    }

    int *imageDims = asVTK->GetDimensions();
    *_rowSize = imageDims[1]; // #rows is y-size
    *_colSize = imageDims[0]; // #cols is x-size
}
