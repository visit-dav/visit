// ************************************************************************* //
//                            avtImageFileWriter.C                           //
// ************************************************************************* //

#include <avtImageFileWriter.h>

#include <vtkBMPWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkTIFFWriter.h>
#include <vtkPostScriptWriter.h>
#include <vtkPPMWriter.h>
#include <vtkRGBWriter.h>
#include <vtkPNGWriter.h>

#include <DebugStream.h>


// This array contains strings that correspond to the file types that are 
// enumerated in the ImageFileFormat enum.
const char *avtImageFileWriter::extensions[] = {
".bmp", ".jpeg", ".png", ".ps", ".ppm", ".rgb", ".tif"};


// ****************************************************************************
//  Method: avtImageFileWriter constructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2001
//
// ****************************************************************************

avtImageFileWriter::avtImageFileWriter()
{
    nFilesWritten = 0;
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
// ****************************************************************************

void
avtImageFileWriter::Write(ImageFileFormat format, const char *filename,
                          int quality, bool progressive, int compression)
{
    vtkImageWriter *writer = NULL;

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
        writer = vtkPPMWriter::New();
        break;
    case RGB:
        writer = vtkRGBWriter::New();
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
        writer->SetFileName(filename);
        writer->SetInput(GetImageRep().GetImageVTK());
        writer->Write();
        writer->Delete();
    }
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
        writer->SetInput(GetImageRep().GetImageVTK());
        writer->Write();
    }
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
//   
// ****************************************************************************

char *
avtImageFileWriter::CreateFilename(const char *base, bool family,
                                   ImageFileFormat format)
{
    char *str = NULL;
    int len = strlen(base);

    // Get memory for the filename. The 9 is for "0000.tif"
    str = new char[len + 1 + 9];

    // The family flag indicates whether or not to include the number of
    // files written as part of the filename.
    if(family)
    {
        sprintf(str, "%s%04d%s", base, nFilesWritten,
                extensions[(int)format]);
    }
    else
    {
        sprintf(str, "%s", base);

        if(!FileHasExtension(base, extensions[(int)format]))
        {
            sprintf(str, "%s%s", str, extensions[(int)format]);
        }
    }

    // Increment the number of files written.
    ++nFilesWritten;

    return str;
}
