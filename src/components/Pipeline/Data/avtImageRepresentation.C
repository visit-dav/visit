/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                        avtImageRepresentation.C                           //
// ************************************************************************* //
#include <vtkCharArray.h>
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


//
// Function Prototypes
//

void   CreateStringFromInput(vtkImageData *, float *, unsigned char *&, int &);
void   CreateStringFromVTKInput(vtkImageData *, unsigned char *&, int &);


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
//  Modifications:
//
//    Hank Childs, Mon Feb  6 14:59:43 PST 2006
//    Allow the z-buffer to be directly acquired instead of copied.
//
// ****************************************************************************

avtImageRepresentation::avtImageRepresentation(vtkImageData *d, float *z, 
                                               bool iNowOwn)
{
    Initialize();
    int width = 0;
    int height = 0;

    asVTK        = d;
    if (asVTK != NULL)
    {
        asVTK->Register(NULL);
        width  = asVTK->GetDimensions()[0];
        height = asVTK->GetDimensions()[1];
    }

    if (z != NULL)
    {
       if (iNowOwn)
           zbuffer = z;
       else
       {
           zbuffer      = new float[width * height];
           memcpy(zbuffer, z, sizeof(float) * width * height);
       }
       zbufferRef   = new int(1);
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
//  Modifications:
//
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added compression data members
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

    rowOrigin = r.rowOrigin;
    colOrigin = r.colOrigin;
    compressionRatio = r.compressionRatio;
    timeToCompress = r.timeToCompress;
    timeToDecompress = r.timeToDecompress;
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
//  Modifications:
//
//    Hank Childs, Tue Dec 18 10:25:09 PST 2007
//    Remove const qualification of return type.
//
// ****************************************************************************

avtImageRepresentation &
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

        GetImageFromString(asChar, asCharLength, asVTK, zbuffer);
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

        GetImageFromString(asChar, asCharLength, asVTK, zbuffer);
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
    // make a shallow copy for if we need to attach zbuffer data
    vtkImageData *tmp  = vtkImageData::New();
    tmp->ShallowCopy(img);

    //
    // Figure out what the width and height should be before we start altering
    // the image (the image should not be altered, but you never know with VTK)
    //
    int dims[3];
    tmp->GetDimensions(dims);
    int width  = dims[0];
    int height = dims[1];

    // add the zbuffer as point data if we have one
    if (zbuffer)
    {
       vtkFloatArray *zArray = vtkFloatArray::New();
       zArray->SetNumberOfComponents(1);
       int iOwnIt = 1;  // 1 means we own it -- you don't delete it.
       zArray->SetArray(zbuffer, width * height, iOwnIt);
       zArray->SetName("zbuffer");
       tmp->GetPointData()->AddArray(zArray);
       zArray->Delete();
    }

    CreateStringFromVTKInput(tmp, str, len);

    tmp->Delete();
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
//  Modifications:
//
//    Hank Childs, Thu Jan 22 15:57:19 PST 2004
//    Remove image clip because it has a dependence on vtkImaging library.
//
// ****************************************************************************

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
    writer->SetInput(tmp);
    writer->SetFileTypeToBinary();
    writer->Write();

    len = writer->GetOutputStringLength();
    str = (unsigned char *) writer->RegisterAndGetOutputString();

    writer->Delete();
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
// ****************************************************************************

void avtImageRepresentation::GetImageFromString(unsigned char *str,
        int strLength, vtkImageData *&img, float *&zbuffer)
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
    img = reader->GetOutput();
    img->Update();
    img->SetScalarType(VTK_UNSIGNED_CHAR);
    img->Register(NULL);
    //  calling SetSource sets' PipelineInformation to NULL, and then
    //  vtkImageData no longer knows its scalar data type, and who knows
    //  what else.
    //img->SetSource(NULL);
    reader->Delete();
    charArray->Delete();

    // If there is a "zbuffer" point data array, then get it too
    vtkDataArray *zArray = img->GetPointData()->GetArray("zbuffer");
    if (zArray)
    {
       int size = zArray->GetSize();
       zbuffer = new float[size];
       memcpy(zbuffer, zArray->GetVoidPointer(0), size * sizeof(float));
       zbufferRef   = new int(1);

       // remove the zbuffer data from the vtkImageData object 
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

        GetImageFromString(asChar, asCharLength, asVTK, zbuffer);
    }

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
// ****************************************************************************

int
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
// ****************************************************************************

float
avtImageRepresentation::GetCompressionRatio() const
{
    if (compressionRatio != -1.0)
        return compressionRatio;

    if (asChar != NULL)
    {
        float ratioc;
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
// ****************************************************************************

float
avtImageRepresentation::GetTimeToCompress() const
{
    if (timeToCompress != -1.0)
        return timeToCompress;

    if (asChar != NULL)
    {
        float timec;
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
