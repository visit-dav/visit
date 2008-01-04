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
//                               avtFileWriter.h                             //
// ************************************************************************* //

#ifndef AVT_FILE_WRITER_H
#define AVT_FILE_WRITER_H

#include <file_writer_exports.h>

#include <avtDatasetFileWriter.h>
#include <avtImageFileWriter.h>


// ****************************************************************************
//  Class: avtFileWriter
//
//  Purpose:
//      This serves as a front end to writers for datasets and data objects.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Nov  6 07:44:38 PST 2003
//    Add compression arg to Write method.
//
// ****************************************************************************

class AVTFILEWRITER_API avtFileWriter
{
  public:
                           avtFileWriter();
    virtual               ~avtFileWriter();

    void                   SetFormat(int);
    bool                   IsImageFormat(void);

    void                   Write(const char *, avtDataObject_p, int, bool,
                                 int, bool);
    void                   WriteImageDirectly(vtkImageWriter *, const char *,
                                            avtDataObject_p);
  
    char                  *CreateFilename(const char *, bool);

  protected:
    int                    format;
    ImageFileFormat        imgFormat;
    DatasetFileFormat      dsFormat;
    bool                   isImage;
    avtImageFileWriter    *imgWriter;
    avtDatasetFileWriter  *dsWriter;
};


#endif


