/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                            avtImageFileWriter.h                           //
// ************************************************************************* //

#ifndef AVT_IMAGE_FILE_WRITER_H
#define AVT_IMAGE_FILE_WRITER_H
#include <file_writer_exports.h>


#include <avtTerminatingImageSink.h>


typedef enum
{
    WINDOWS_BITMAP     = 0,
    JPEG,             /* 1 */
    PNG,              /* 2 */
    POSTSCRIPT,       /* 3 */
    PPM,              /* 4 */
    RGB,              /* 5 */
    TIFF              /* 6 */
} ImageFileFormat;

class vtkImageWriter;

// ****************************************************************************
//  Class: avtImageFileWriter
//
//  Purpose:
//      A type of image sink that writes the image to a specified file format.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2001
//
//  Modifications:
//
//    Brad Whitlock, Tue Feb 13 14:40:23 PST 2001
//    I added a couple member functions and some static data members.
//
//    Hank Chlids, Mon Jun  4 09:06:58 PDT 2001
//    Inherited from avtTerminatingImageSink.
//
//    Brad Whitlock, Wed Jan 23 14:57:52 PST 2002
//    I added a couple new arguments to the Write method that allow some
//    format specific options to be passed in.
//
//    Brad Whitlock, Wed Feb 20 17:17:42 PST 2002
//    I added a new Write method that takes a writer.
//
//    Hank Childs, Fri May 24 10:51:28 PDT 2002
//    Stop passing images as arguments, since SetInput is the approved route.
//
//    Kathleen Bonnell, Thu Nov  6 07:44:38 PST 2003
//    Added compression arg to Write method.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers 
//    have problems with them.
//
//    Brad Whitlock, Mon Mar 6 17:35:28 PST 2006
//    I made it reset nFilesWritten if the nase changes.
//
// ****************************************************************************

class AVTFILEWRITER_API avtImageFileWriter : public avtTerminatingImageSink
{
  public:
                       avtImageFileWriter();
    virtual           ~avtImageFileWriter();

    void               Write(ImageFileFormat, const char *filename,
                             int quality, bool progressive, int compression);
    void               Write(vtkImageWriter *writer, const char *filename);

    char              *CreateFilename(const char *base, bool family,
                                      ImageFileFormat format);

  protected:
    bool               FileHasExtension(const char *filename, const char *ext)
                             const;

    static const char *extensions[];
    int                nFilesWritten;
    char              *oldFileBase;
};


#endif


