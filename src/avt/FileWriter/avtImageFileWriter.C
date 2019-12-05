// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtImageFileWriter.C                           //
// ************************************************************************* //

#include <avtImageFileWriter.h>

#include <visit-config.h>

#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkBMPWriter.h>
#include <vtkPNMWriter.h>
#include <vtkImageData.h>
#include <vtkJPEGWriter.h>
#include <vtkTIFFWriter.h>
#include <vtkPostScriptWriter.h>
#include <vtkRGBWriter.h>
#include <vtkPNGWriter.h>
#ifdef HAVE_LIBOPENEXR
#include <vtkOpenEXRWriter.h>
#endif
#include <vtkUnsignedCharArray.h>
#include <vtkBase64Utilities.h>
#include <vtkImageResample.h>
#include <vtkZLibDataCompressor.h>

#include <DebugStream.h>

// This array contains strings that correspond to the file types that are 
// enumerated in the ImageFileFormat enum.
const char *avtImageFileWriter::extensions[][4] = {
{".bmp", ".BMP", NULL, NULL},
{".jpeg", ".jpg", ".JPEG", ".JPG"},
{".png", ".PNG", NULL, NULL},
{".ps", ".PS", NULL, NULL},
{".ppm", ".PPM", NULL, NULL},
{".rgb", ".RGB", NULL, NULL},
{".tif", ".tiff", ".TIF", ".TIFF"},
{".exr", ".EXR", NULL, NULL}
};


// ****************************************************************************
//  Method: avtImageFileWriter constructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Mar 6 17:36:50 PST 2006
//    Added oldFileBase.
//
// ****************************************************************************

avtImageFileWriter::avtImageFileWriter()
{
    nFilesWritten = 0;
    oldFileBase = 0;
}


// ****************************************************************************
//  Method: avtImageFileWriter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//    Brad Whitlock, Mon Mar 6 17:37:26 PST 2006
//    Added oldFileBase.
//
// ****************************************************************************

avtImageFileWriter::~avtImageFileWriter()
{
    if(oldFileBase != 0)
        delete [] oldFileBase;
}


// ****************************************************************************
//  Method: avtImageFileWriter::Write
//
//  Purpose:
//      Writes out the image to a file.
//
//  Arguments:
//      format   The format to write the file in.
//      base     The basename of the file.
//
//  Programmer:  Hank Childs
//  Creation:    February 12, 2001
//
//  Modifications:
//    Brad Whitlock, Tue Feb 13 14:31:02 PST 2001
//    I added more file types.
//
//    Hank Childs, Sun Jan 13 14:03:17 PST 2002
//    Fix memory leak.
//
//    Brad Whitlock, Wed Jan 23 14:59:40 PST 2002
//    I added JPEG and PNG.
//
//    Brad Whitlock, Thu Apr 4 14:55:12 PST 2002
//    Changed CopyTo to an inline template function.
//
//    Hank Childs, Fri May 24 10:53:42 PDT 2002
//    Remove the argument for the image.
//
//    Brad Whitlock, Wed Dec 4 17:05:42 PST 2002
//    I added postscript.
//
//    Kathleen Bonnell, Thu Nov  6 07:44:38 PST 2003 
//    Added compression arg, used with TIFF writer. 
//
//    Hank Childs, Mon Jan  5 21:26:34 PST 2004
//    Use vtkVisItTIFFWriter, which does not exhibit strangeness like
//    vtkTIFFWriter does on the SGIs using CC.
//
//    Brad Whitlock, Tue Sep 19 14:15:40 PDT 2017
//    OpenEXR support. Write ZBuffer, luminance image, value image if present.
//    I made it return a list of filenames.
//
//    Alister Maguire, Mon Dec  2 14:14:42 MST 2019
//    Replaced our custom vtkPPMWriter with the standard vtkPNMWriter.
//
// ****************************************************************************

std::vector<std::string>
avtImageFileWriter::Write(ImageFileFormat format, const char *filename,
                          int quality, bool progressive, int compression)
{
    vtkImageWriter *writer = NULL;
    bool writeZ = true;
    bool writeLuminance = true;
    bool writeValue = true;
    std::vector<std::string> fnList;

    // Create a different writer object based on the desired file type.
    switch(format)
    {
    case WINDOWS_BITMAP:
        writer = vtkBMPWriter::New();
        break;
    case JPEG:
        {
             vtkJPEGWriter *jpegWriter = vtkJPEGWriter::New();
             writer = jpegWriter;
             jpegWriter->SetQuality(quality);
             jpegWriter->SetProgressive(progressive?1:0);
        }
        break;
    case PNG:
        writer = vtkPNGWriter::New();
        break;
    case POSTSCRIPT:
        writer = vtkPostScriptWriter::New();
        break;
    case PPM:
        writer = vtkPNMWriter::New();
        break;
    case RGB:
        writer = vtkRGBWriter::New();
        break;
    case OPENEXR:
#ifdef HAVE_LIBOPENEXR
        { // new scope
        vtkOpenEXRWriter *exrWriter = vtkOpenEXRWriter::New();
        // Pass the Z buffer, if there is one, to the writer.
        exrWriter->SetZBuffer(GetImageRep().GetZBufferVTK());
        writer = exrWriter;
        }
#else
        debug1 << "VisIt was not built with OpenEXR support." << endl;
#endif
        // OpenEXR writer will write these images into the same file
        // so don't let the code later on in this method execute.
        writeZ = false;
        writeValue = false;
        writeLuminance = false;
        break;
    case TIFF:
    default:
        writer = vtkTIFFWriter::New();
        ((vtkTIFFWriter*)writer)->SetCompression(compression);
        break;
    }

    // Use the writer to write the image to a file.
    if(writer)
    {
        // We might have a situation where the ImageScalars are 1 channel 
        // float as in a value image. Make sure we have a vtkUnsignedCharArray.
        // We let this go through in case we have float data we have openexr.
        if(GetImageRep().GetRGBBufferVTK() != NULL || writeValue == false)
        {
            debug5 << "Writing RGB(A) data to " << filename << endl;

            // Write the image.
            fnList.push_back(filename);
            writer->SetFileName(filename);
            writer->SetInputData(GetImageRep().GetImageVTK());
            writer->Write();
        }
    }

    // If the ZBuffer is present, we made a request upstream for it. Write
    // it out as zlib-compressed floats.
    if(writeZ && GetImageRep().GetZBufferVTK() != NULL)
    {
        std::string zfilename(filename);
        std::string::size_type dot = zfilename.rfind(".");
        if(dot != std::string::npos)
        {
            zfilename = zfilename.substr(0, dot) + ".depth.Z";
        }
        else
            zfilename.append(".depth.Z");
        debug5 << "Writing Z data to " << zfilename << endl;
        // We scale Z from [0,1] to [0,256]. The 256 value is used by Cinema to
        // discard fragments in a shader.
        if(WriteFloatImage(zfilename, GetImageRep().GetZBufferVTK(), true, 256.f))
            fnList.push_back(zfilename);
    }

    // If the value array is present, write it.
    if(writeValue)
    {
        vtkFloatArray *valueF = NULL;
        if(GetImageRep().GetRGBBufferVTK() != NULL)
        {
            // we have uchar image data so look for "value" data.
            vtkDataArray *value = GetImageRep().GetImageVTK()->GetPointData()->GetArray("value");
            valueF = vtkFloatArray::SafeDownCast(value);
            if(valueF != NULL)
            {
                debug5 << "Found value data in array \"value\"" << endl;
            }
        }
        else
        {
            vtkDataArray *value = GetImageRep().GetImageVTK()->GetPointData()->GetScalars();
            vtkFloatArray *f = vtkFloatArray::SafeDownCast(value);
            if(f != NULL && 
               strcmp(f->GetName(), "ImageScalars") == 0 &&
               f->GetNumberOfComponents() == 1)
            {
                // It looks like a value image.
                valueF = f;
                debug5 << "Found value data as float ImageScalars." << endl;
            }
            else
            {
                debug5 << "Scalars were called " << value->GetName() << endl;
            }
        }

        if(valueF != NULL)
        {
            std::string vfilename(filename);
            std::string::size_type dot = vfilename.rfind(".");
            if(dot != std::string::npos)
            {
                vfilename = vfilename.substr(0, dot) + ".value.Z";
            }
            else
                vfilename.append(".value.Z");
            debug5 << "Writing value data to " << vfilename << endl;
            if(WriteFloatImage(vfilename, valueF))
                fnList.push_back(vfilename);
        }
    }

    // Write the luminance image. Let's write it after value because 
    // changing the scalar this way seems flaky.
    vtkDataArray *lum = GetImageRep().GetImageVTK()->GetPointData()->GetArray("luminance");
    if(writer != NULL && writeLuminance && lum != NULL)
    {
//        vtkDataArray *scalars = GetImageRep().GetImageVTK()->GetPointData()->GetScalars();
        // override scalars.
        GetImageRep().GetImageVTK()->GetPointData()->SetScalars(lum);

        std::string lfilename(filename);
        std::string::size_type dot = lfilename.rfind(".");
        std::string ext(std::string(".lum") + std::string(extensions[format][0]));
        if(dot != std::string::npos)
        {
            lfilename = lfilename.substr(0, dot) + ext;
        }
        else
            lfilename.append(ext);

        debug5 << "Writing luminance data to " << lfilename << endl;
        fnList.push_back(lfilename);
        writer->SetFileName(lfilename.c_str());
        writer->Write();

        // restore scalars
//        if(scalars != NULL)
//            GetImageRep().GetImageVTK()->GetPointData()->SetScalars(scalars);
    }

    if(writer != NULL)
        writer->Delete();

    return fnList;
}

// ****************************************************************************
// Method: avtImageFileWriter::Write
//
// Purpose: 
//   Overloaded Write method that allows a user-defined image writer to be
//   used.
//
// Arguments:
//   writer   : The image writer to be used.
//   filename : The name of the file to write.
//
// Notes:       This method allows the user to pass a custom image writer
//              in and have it save the images. I use this for printing. I
//              created a vtkQtImagePrinter which is a subclass of
//              vtkImageWriter and it uses some Qt functionality to print the
//              image. It seemed less evil to expose a little VTK here than it
//              did to put a class that requires Qt into AVT.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 17:21:12 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Apr 4 14:55:47 PST 2002
//   Changed CopyTo to an inline template function.
//
//   Hank Childs, Fri May 24 10:53:42 PDT 2002
//   Remove the argument for the image.
//
// ****************************************************************************

void
avtImageFileWriter::Write(vtkImageWriter *writer, const char *filename)
{
    // Use the writer to write the image.
    if(writer)
    {
        writer->SetFileName(filename);
        writer->SetInputData(GetImageRep().GetImageVTK());
        writer->Write();
    }
}

// ****************************************************************************
// Method: avtImageFileWriter::WriteFloatImage
//
// Purpose:
//   Writes a float image as a zlib-compressed file.
//
// Arguments:
//   filename : The name of the file to write.
//
// Returns:    
//
// Note:       This helps support Cinema.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 21 12:02:53 PDT 2017
//
// Modifications:
//
// ****************************************************************************

bool
avtImageFileWriter::WriteFloatImage(const std::string &filename, vtkFloatArray *arr,
    bool doScale, float scale)
{
    bool retval = false;
    const char *mName = "avtImageFileWriter::WriteFloatImage: ";
    debug5 << mName << "We have a arr array with "
           << arr->GetNumberOfTuples() << " tuples." << endl;
    vtkZLibDataCompressor *compressor = vtkZLibDataCompressor::New();
    if(compressor != NULL)
    {
        debug5 << mName << "Created compressor." << endl;
        FILE *f = fopen(filename.c_str(), "wb");
        if(f != NULL)
        {
            debug5 << "Opened " << filename << " for output." << endl;

            // Get the size of the image. We have to flip vertically 
            // as we write it.
            int width, height;
            GetImageRep().GetSize(&height, &width);
            debug5 << mName << "width=" << width << ", height=" << height << endl;

#define VISIT_Z_FLIP
            float *ubuf = NULL;
            bool deleteubuf = false;
            if(doScale)
            {
                // Flip the buffer.
                const float *zbase = (const float *)arr->GetVoidPointer(0);
                ubuf = new float[width*height];
                deleteubuf = true;
                float *dest = ubuf;
                for(int j = 0; j < height; ++j)
                {
#ifdef  VISIT_Z_FLIP
                    const float *src = zbase + (height-1-j) * width; // flip
#else
                    const float *src = zbase + j * width;
#endif
                    for(int i = 0; i < width; ++i)
                        dest[i] = src[i] * scale;
                    dest += width;
                }
            }
            else
            {
#ifdef VISIT_Z_FLIP
                // Flip the buffer.
                const float *zbase = (const float *)arr->GetVoidPointer(0);
                ubuf = new float[width*height];
                deleteubuf = true;
                float *dest = ubuf;
                for(int j = 0; j < height; ++j)
                {
                    const float *src = zbase + (height-1-j) * width; // flip
                    memcpy(dest, src, sizeof(float) * width);
                    dest += width;
                }
#else
                const float *ubuf = (const float *)arr->GetVoidPointer(0);
#endif
            }

            // Compress the whole buffer at once.
            size_t usize = sizeof(float) * width * height;
            size_t maxSize = compressor->GetMaximumCompressionSpace(usize);
            debug5 << mName << "usize=" << usize << ", maxSize=" << maxSize << endl;
            unsigned char *cbuf = new unsigned char[maxSize];
            size_t csize = compressor->Compress(
                                   (const unsigned char *)ubuf,
                                   usize,
                                   cbuf,
                                   maxSize);

            // Write to the file.
            size_t n = fwrite(cbuf, sizeof(unsigned char), csize, f);
            debug5 << mName << "Told to write " << csize << " bytes. Wrote "
                   << n << " bytes." << endl;

            if(deleteubuf)
                delete [] ubuf;

            delete [] cbuf;
            fclose(f);
            retval = true;
        }
        compressor->Delete();
    }

    debug5 << mName << "retval=" << (retval?"true":"false") << endl;
    return retval;
}

// ****************************************************************************
// Method: avtImageFileWriter::FileHasExtension
//
// Purpose: 
//   Determines whether or not a filename already has the appropriate file
//   extension.
//
// Arguments:
//   filename : The filename that is being checked.
//   ext      : The file extension.
//
// Returns:    True if the ext string is at th end of the filename.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 14:46:46 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
avtImageFileWriter::FileHasExtension(const char *filename, const char *ext) const
{
    int extlen = strlen(ext);
    int filelen = strlen(filename);

    return (strcmp(filename + filelen - extlen, ext) == 0);
}

// ****************************************************************************
// Method: avtImageFileWriter::CreateFilename
//
// Purpose: 
//   Creates a filename with the appropriate file extension and returns it as
//   a dynamically allocated array of characters.
//
// Arguments:
//   base   : The base filename.
//   family : A flag indicating whether or not to include numbers in the
//            returned filename.
//   format : The desired image format.
//
// Returns:    A dyanimcally allocated array of characters that contains the
//             filename.
//
// Note:       The calling procedure must free the memory.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 13 14:54:17 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 6 17:39:39 PST 2006
//   Added code to reset nFilesWritten if the file base changes.
//
//   Andreas Schafer, Thu Dec  2 09:22:34 PST 2010
//   Fixed a problem that caused the file base from being ignored when files
//   did not have an extension.
//
//   Brad Whitlock, Thu Sep 18 22:55:57 PDT 2014
//   Check some other file extensions before appending the default file
//   extension for the format. This is because some file formats have allowable
//   file extension variants.
//
// ****************************************************************************

char *
avtImageFileWriter::CreateFilename(const char *base, bool family,
                                   ImageFileFormat format)
{
    char *str = NULL;
    int len = strlen(base);
    int iformat = (int)format;

    // Reset the nFilesWritten count if the file base changes.
    if(family)
    {
        if(oldFileBase == NULL)
        {
            oldFileBase = new char[len+1];
            strcpy(oldFileBase, base);
        }
        else if(strcmp(oldFileBase, base) != 0)
        {
            delete [] oldFileBase;
            oldFileBase = new char[len+1];
            strcpy(oldFileBase, base);

            nFilesWritten = 0;
        }
    }

    // Get memory for the filename. The 10 is for "0000.jpeg"
    len += (1 + 10);
    str = new char[len];

    // The family flag indicates whether or not to include the number of
    // files written as part of the filename.
    if(family)
    {
        snprintf(str, len, "%s%04d%s", base, nFilesWritten,
                 extensions[iformat][0]);
    }
    else
    {
        snprintf(str, len, "%s", base);

        // We're passing a full filename. See if we need to append a 
        // file extension.
        bool hasExtension = false;
        for(int i = 0; i < 4; ++i)
        {
            if(extensions[iformat][i] != NULL)
                hasExtension |= FileHasExtension(base, extensions[iformat][i]);
        }

        if(!hasExtension)
        {
            snprintf(str, len, "%s%s", base, extensions[iformat][0]);
        }
    }

    // Increment the number of files written.
    ++nFilesWritten;

    return str;
}

// ****************************************************************************
// Method: avtImageFileWriter::WriteToByteArray
//
// Purpose:
//   Writes image to byte array
//
// Arguments:
//
// Returns:    string representation of image. Memory needs to be freed by the
//             calling function.
//
//
// Programmer: Hari Krishnan
// Creation:   October 13, 2012
//
// Modifications:
// ****************************************************************************

const char*
avtImageFileWriter::WriteToByteArray(avtImageRepresentation &imagerep,
                                     int quality,
                                     bool progressive,
                                     size_t &len)
{

    vtkJPEGWriter* writer = vtkJPEGWriter::New();

    writer->SetWriteToMemory(true);
    writer->SetInputData(imagerep.GetImageVTK());
    writer->SetQuality(quality);
    writer->SetProgressive(progressive?1:0);

    writer->Write();

    vtkUnsignedCharArray* array = writer->GetResult();

    char * result = NULL;
    len = 0;

    if(array)
    {

        vtkIdType size = array->GetSize();
        if(size > 0)
        {
            len = size;
            result = new char [len];
            memcpy(result,array->GetVoidPointer(0),sizeof(char)*len);
        }
    }

    writer->Delete();

    return result;
}


const char*
avtImageFileWriter::WriteToByteArray(avtImageRepresentation &imagerep,
                                    int quality,
                                    bool progressive,
                                    size_t& len,
                                    int outputWidth,
                                    int outputHeight)
{


    vtkImageData* imagedata = imagerep.GetImageVTK();

    vtkImageResample* reslice = vtkImageResample::New();

    int extents[6];
    imagedata->GetExtent(extents);

    reslice->SetOutputSpacing(((double)extents[1])/outputWidth,
                              ((double)extents[3])/outputHeight,
                              0);

    reslice->SetInputData(imagedata);
    reslice->Update();

    vtkJPEGWriter* writer = vtkJPEGWriter::New();


    writer->SetWriteToMemory(true);
    writer->SetInputData(reslice->GetOutput());
    writer->SetQuality(quality);
    writer->SetProgressive(progressive?1:0);

    writer->Write();

    vtkUnsignedCharArray* array = writer->GetResult();

    char * result = NULL;
    len = 0;

    if(array)
    {

        vtkIdType size = array->GetSize();
        if(size > 0)
        {
            len = size;
            result = new char [len];
            memcpy(result,array->GetVoidPointer(0),sizeof(char)*len);
        }
    }

    reslice->Delete();
    writer->Delete();

    return result;
}
