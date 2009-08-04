/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              avtFileWriter.C                              //
// ************************************************************************* //

#include <avtFileWriter.h>

#include <SaveWindowAttributes.h>

#include <avtCallback.h>
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
//    Jeremy Meredith, Thu Apr  5 17:23:37 EDT 2007
//    Added POVRay data file format.
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
//    Kathleen Bonnell, Wed Nov 12 16:59:24 PST 2003 
//    Added argument 'compression'.
//
//    Hank Childs, Tue Apr  6 16:48:58 PDT 2004
//    Do not assume the input is valid.
//
// ****************************************************************************

void
avtFileWriter::Write(const char *filename, avtDataObject_p dob, int quality,
                     bool progressive, int compression, bool binary)
{
    if (IsImageFormat())
    {
        if (*dob == NULL)
        {
            avtCallback::IssueWarning("The file writer was not able to save "
                  "out a NULL image.");
        }
        else
        {
            imgWriter->SetInput(dob);
            imgWriter->Write(imgFormat, filename, quality, progressive,
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
            dsWriter->Write(dsFormat, filename, binary);
        }
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
//    Hank Childs, Tue Feb 15 11:37:36 PST 2005
//    Do not overwrite pre-existing files.  Also, give a warning when trying
//    to save to a file that does not exist.
//
//    Jeremy Meredith, Wed Aug  6 18:03:45 EDT 2008
//    Changed char* to const char* to fix warning.
//
// ****************************************************************************

char *
avtFileWriter::CreateFilename(const char *base, bool family)
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

        bool fileExists = false;
        ifstream ifile(rv);
        if (!ifile.fail())
        {
            fileExists = true;
        }
        if (fileExists && family)
        {
            //
            // We are saving a family, so reject this one and keep going.
            //
            msg = "Although VisIt typically saves out files sequentially, "
                  "some numbers are being skipped when saving out this "
                  "file to avoid overwriting previous saves.";
            keepGoing = true;
        }
        else
        {
            ofstream ofile(rv);
            if (ofile.fail())
            {
                rv = NULL;
                msg = "VisIt cannot write a file in the directory specified.\n"
                      "Note: If you are running client/server, VisIt can only "
                      "save files onto the local client.";
            }
        }
    }

    if (msg != NULL)
    {
        avtCallback::IssueWarning(msg);
    }

    return rv;
}


