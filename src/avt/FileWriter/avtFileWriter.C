// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtFileWriter.C                              //
// ************************************************************************* //

#include <avtFileWriter.h>

#include <SaveWindowAttributes.h>

#include <avtCallback.h>
#include <ImproperUseException.h>

#include <FileFunctions.h>

#include <cerrno>
#include <cstring>

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
//    Jeremy Meredith, Thu Apr  5 17:23:37 EDT 2007
//    Added POVRay data file format.
//
//    Dave Pugmire, Thu Jul  8 08:30:11 EDT 2010
//    Added PLY writer.
//
//    Brad Whitlock, Tue Sep 19 14:36:51 PDT 2017
//    OpenEXR writer.
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
    else if (f == SaveWindowAttributes::POVRAY)
    {
        dsFormat = POVRAY;
        isImage = false;
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
    else if (f == SaveWindowAttributes::PLY)
    {
        dsFormat = PLY;
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
    else if (f == SaveWindowAttributes::EXR)
    {
        imgFormat = OPENEXR;
        isImage = true;
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
//  Method: avtFileWriter::SetOptions
//
//  Purpose:
//    Sends DBOptionsAttributes to dsWriter.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 31, 2018
//
// ****************************************************************************

void
avtFileWriter::SetOptions(const DBOptionsAttributes &opts)
{
   if (dsWriter)
        dsWriter->SetOptions(opts);
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
//    Kathleen Bonnell, Wed Nov 12 16:59:24 PST 2003 
//    Added argument 'compression'.
//
//    Hank Childs, Tue Apr  6 16:48:58 PDT 2004
//    Do not assume the input is valid.
//
// ****************************************************************************

std::vector<std::string>
avtFileWriter::Write(const char *filename, avtDataObject_p dob, int quality,
                     bool progressive, int compression, bool binary)
{
    std::vector<std::string> fnList;
    if (IsImageFormat())
    {
        if (*dob == NULL)
        {
            avtCallback::IssueWarning((std::string("The file writer was not able to save "
                  "out a NULL image for ")+std::string(filename) + std::string(".")).c_str());
        }
        else
        {
            imgWriter->SetInput(dob);
            fnList = imgWriter->Write(imgFormat, filename, quality, progressive,
                                      compression);
        }
    }
    else
    {
        if (*dob == NULL)
        {
            avtCallback::IssueWarning("The file writer was not able to save "
                  "out a NULL surface.");
        }
        else
        {
            dsWriter->SetInput(dob);
            dsWriter->Write(dsFormat, filename, quality, compression, binary);
            fnList.push_back(filename);
        }
    }
    return fnList;
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
//      fileChecks Whether to check file permissions, etc.
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
//    Hank Childs, Tue Feb 15 11:37:36 PST 2005
//    Do not overwrite pre-existing files.  Also, give a warning when trying
//    to save to a file that does not exist.
//
//    Jeremy Meredith, Wed Aug  6 18:03:45 EDT 2008
//    Changed char* to const char* to fix warning.
//
//    Brad Whitlock, Tue Mar 22 19:35:40 PDT 2016
//    Add check permissions flag to CreateFilename.
//
// ****************************************************************************

char *
avtFileWriter::CreateFilename(const char *base, bool family, bool fileChecks)
{
    char *rv = NULL;
    const char *msg = NULL;
    bool keepGoing = true;
    while (keepGoing)
    {
        keepGoing = false;
        if (IsImageFormat())
        {
            rv = imgWriter->CreateFilename(base, family, imgFormat);
        }
        else
        {
            rv = dsWriter->CreateFilename(base, family, dsFormat);
        }

        if(fileChecks)
        {
            bool fileExists = true;
            bool isWriteable = false;
            FileFunctions::VisItStat_t statbuf;

            if (FileFunctions::VisItStat(rv, &statbuf) != 0)
            {
                int errnum = errno;

                if (errno == ENOENT)
                {
                    fileExists = false;
                }
                else
                {
                    char statmsg[512];
                    snprintf(statmsg, sizeof(statmsg), 
                        "VisIt encountered error \"%s\\n"
                        "attempting to stat file \"%s\"\n"
                        "prior to writing", strerror(errnum), rv);
                    msg = statmsg;
                }
            }
#if defined(_WIN32)
            else if (statbuf.st_mode & _S_IWRITE)
#else
            else if (statbuf.st_mode & S_IWUSR)
#endif
            {
                isWriteable = true;
            }
            else
            {
                char wrtmsg[512];
                snprintf(wrtmsg, sizeof(wrtmsg), 
                        "The file \"%s\" is not writeable\n", rv);
                msg = wrtmsg;
            }

            if (fileExists && isWriteable && family)
            {
                //
                // We are saving a family, so reject this one and keep going.
                //
                msg = "Although VisIt typically saves out files sequentially, "
                      "some numbers are being skipped when saving out this "
                      "file to avoid overwriting previous saves.";
                keepGoing = true;
            }
        }
    }

    if (msg != NULL)
    {
        avtCallback::IssueWarning(msg);
    }

    return rv;
}


