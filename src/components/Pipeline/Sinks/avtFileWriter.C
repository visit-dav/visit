// ************************************************************************* //
//                              avtFileWriter.C                              //
// ************************************************************************* //

#include <avtFileWriter.h>

#include <SaveWindowAttributes.h>

#include <ImproperUseException.h>

// ****************************************************************************
//  Method: avtFileWriter constructor
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

avtFileWriter::avtFileWriter()
{
    format = -1;
    imgWriter = new avtImageFileWriter();
    dsWriter  = new avtDatasetFileWriter();
}


// ****************************************************************************
//  Method: avtFileWriter destructor
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

avtFileWriter::~avtFileWriter()
{
    if (imgWriter != NULL)
    {
        delete imgWriter;
        imgWriter = NULL;
    }
    if (dsWriter != NULL)
    {
        delete dsWriter;
        dsWriter = NULL;
    }
}


// ****************************************************************************
//  Method: avtFileWriter::SetFormat
//
//  Purpose:
//      Sets the format the file writer should write out to.
//
//  Arguments:
//      f       The format type, in terms relative to SaveWindowAttributes.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
// 
//  Modifications:
//
//    Hank Childs, Mon May 27 13:22:47 PDT 2002
//    Added support for VTK, OBJ, and curves.
//
//    Jeremy Meredith, Thu Jul 25 11:50:24 PDT 2002
//    Made file format be a true enum.
//
//    Brad Whitlock, Wed Dec 4 17:10:49 PST 2002
//    I added postscript.
//
//    Jeremy Meredith, Sat Apr 12 15:09:28 PDT 2003
//    Added the Ultra file format.
//
// ****************************************************************************

void
avtFileWriter::SetFormat(int f)
{
    if (f == SaveWindowAttributes::BMP)
    {
        imgFormat = WINDOWS_BITMAP;
        isImage = true;
    }
    else if (f == SaveWindowAttributes::CURVE)
    {
        dsFormat = CURVE;
        isImage = false;
    }
    else if (f == SaveWindowAttributes::JPEG)
    {
        imgFormat = JPEG;
        isImage = true;
    }
    else if (f == SaveWindowAttributes::OBJ)
    {
        dsFormat = OBJ;
        isImage = false;
    }
    else if (f == SaveWindowAttributes::PNG)
    {
        imgFormat = PNG;
        isImage = true;
    }
    else if (f == SaveWindowAttributes::POSTSCRIPT)
    {
        imgFormat = POSTSCRIPT;
        isImage = true;
    }
    else if (f == SaveWindowAttributes::PPM)
    {
        imgFormat = PPM;
        isImage = true;
    }
    else if (f == SaveWindowAttributes::RGB)
    {
        imgFormat = RGB;
        isImage = true;
    }
    else if (f == SaveWindowAttributes::STL)
    {
        dsFormat = STL;
        isImage = false;
    }
    else if (f == SaveWindowAttributes::TIFF)
    {
        imgFormat = TIFF;
        isImage = true;
    }
    else if (f == SaveWindowAttributes::ULTRA)
    {
        dsFormat = ULTRA;
        isImage = false;
    }
    else if (f == SaveWindowAttributes::VTK)
    {
        dsFormat = VTK;
        isImage = false;
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }
    format = f;
}


// ****************************************************************************
//  Method: avtFileWriter::IsImageFormat
//
//  Purpose:
//      Determines if the file format writes out images.  This is important for
//      the type of object fed into this object as input.
//
//  Returns:    true if it is an image format, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

bool
avtFileWriter::IsImageFormat(void)
{
    if (format < 0)
    {
        EXCEPTION0(ImproperUseException);
    }
    return isImage;
}


// ****************************************************************************
//  Method: avtFileWriter::Write
//
//  Purpose:
//      Writes out the file.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//
//    Hank Childs, Tue May 28 10:51:17 PDT 2002
//    Added argument 'binary'.
//
// ****************************************************************************

void
avtFileWriter::Write(const char *filename, avtDataObject_p dob, int quality,
                     bool progressive, bool binary)
{
    if (IsImageFormat())
    {
        imgWriter->SetInput(dob);
        imgWriter->Write(imgFormat, filename, quality, progressive);
    }
    else
    {
        dsWriter->SetInput(dob);
        dsWriter->Write(dsFormat, filename, binary);
    }
}


// ****************************************************************************
//  Method: avtFileWriter::WriteImageDirectly
//
//  Purpose:
//      Writes out the file.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

void
avtFileWriter::WriteImageDirectly(vtkImageWriter *wrtr, const char *filename,
                                  avtDataObject_p dob)
{
    imgWriter->SetInput(dob);
    imgWriter->Write(wrtr, filename);
}


// ****************************************************************************
//  Method: avtFileWriter::CreateFilename
//
//  Purpose:
//      Creates the filename based on knowledge of appropriate extensions, etc.
//
//  Arguments:
//      base      The base name for the file.
//      family    Whether or not this file is part of a family of files to be
//                saved.
//
//  Notes:        The memory returned by this routine must be freed by the
//                calling function.
//
//  Programmer:   Hank Childs
//  Creation:     May 24, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Feb 24 18:22:04 PST 2003
//    Account for family in filename.
//
// ****************************************************************************

char *
avtFileWriter::CreateFilename(const char *base, bool family)
{
    char *rv = NULL;
    if (IsImageFormat())
    {
        rv = imgWriter->CreateFilename(base, family, imgFormat);
    }
    else
    {
        rv = dsWriter->CreateFilename(base, family, dsFormat);
    }

    return rv;
}


