// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtFileWriter.h                             //
// ************************************************************************* //

#ifndef AVT_FILE_WRITER_H
#define AVT_FILE_WRITER_H

#include <file_writer_exports.h>

#include <avtDatasetFileWriter.h>
#include <avtImageFileWriter.h>
#include <DBOptionsAttributes.h>


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
//    Brad Whitlock, Tue Mar 22 19:35:40 PDT 2016
//    Add check permissions flag to CreateFilename.
//
//    Kathleen Biagas, Fri Aug 31 13:17:20 PDT 2018
//    Add SetOtions (goes straight to dsWriter).
//
// ****************************************************************************

class AVTFILEWRITER_API avtFileWriter
{
  public:
                           avtFileWriter();
    virtual               ~avtFileWriter();

    void                   SetFormat(int);
    bool                   IsImageFormat(void);

    std::vector<std::string> Write(const char *, avtDataObject_p, int, bool,
                                   int, bool);
    void                   WriteImageDirectly(vtkImageWriter *, const char *,
                                            avtDataObject_p);
  
    char                  *CreateFilename(const char *base, bool family,
                                          bool fileChecks = true);

    void                   SetOptions(const DBOptionsAttributes &);

  protected:
    int                    format;
    ImageFileFormat        imgFormat;
    DatasetFileFormat      dsFormat;
    bool                   isImage;
    avtImageFileWriter    *imgWriter;
    avtDatasetFileWriter  *dsWriter;
};


#endif


