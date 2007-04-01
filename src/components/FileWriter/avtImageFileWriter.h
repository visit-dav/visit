// ************************************************************************* //
//                            avtImageFileWriter.h                           //
// ************************************************************************* //

#ifndef AVT_IMAGE_FILE_WRITER_H
#define AVT_IMAGE_FILE_WRITER_H
#include <pipeline_exports.h>


#include <avtOriginatingImageSink.h>


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
//    Inherited from avtOriginatingImageSink.
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
// ****************************************************************************

class PIPELINE_API avtImageFileWriter : public avtOriginatingImageSink
{
  public:
                       avtImageFileWriter();
    virtual           ~avtImageFileWriter() {;};

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
};


#endif


