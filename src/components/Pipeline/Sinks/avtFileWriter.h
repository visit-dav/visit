// ************************************************************************* //
//                               avtFileWriter.h                             //
// ************************************************************************* //

#ifndef AVT_FILE_WRITER_H
#define AVT_FILE_WRITER_H

#include <pipeline_exports.h>

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

class PIPELINE_API avtFileWriter
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


