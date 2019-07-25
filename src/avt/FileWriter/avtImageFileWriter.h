// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtImageFileWriter.h                           //
// ************************************************************************* //

#ifndef AVT_IMAGE_FILE_WRITER_H
#define AVT_IMAGE_FILE_WRITER_H
#include <file_writer_exports.h>
#include <string>
#include <vector>
#include <avtTerminatingImageSink.h>


typedef enum
{
    WINDOWS_BITMAP     = 0,
    JPEG,             /* 1 */
    PNG,              /* 2 */
    POSTSCRIPT,       /* 3 */
    PPM,              /* 4 */
    RGB,              /* 5 */
    TIFF,             /* 6 */
    OPENEXR           /* 7 */
} ImageFileFormat;

class vtkImageWriter;
class vtkFloatArray;

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
//    I made it reset nFilesWritten if the base changes.
//
//    Brad Whitlock, Tue Sep 19 14:12:37 PDT 2017
//    OpenEXR support. Added WriteFloatImage.
//
// ****************************************************************************

class AVTFILEWRITER_API avtImageFileWriter : public avtTerminatingImageSink
{
  public:
                       avtImageFileWriter();
    virtual           ~avtImageFileWriter();

    std::vector<std::string> Write(ImageFileFormat, const char *filename,
                                   int quality, bool progressive, int compression);
    void               Write(vtkImageWriter *writer, const char *filename);

    char              *CreateFilename(const char *base, bool family,
                                      ImageFileFormat format);

    static const char* WriteToByteArray(avtImageRepresentation &imagerep,
                                        int quality,
                                        bool progressive,
                                        size_t& len);
    static const char* WriteToByteArray(avtImageRepresentation &imagerep,
                                        int quality,
                                        bool progressive,
                                        size_t& len,
                                        int outputWidth,
                                        int outputHeight);
  protected:
    bool               FileHasExtension(const char *filename, const char *ext)
                             const;
    bool               WriteFloatImage(const std::string &filename, vtkFloatArray *,
                                       bool doScale = false, float scale = 1.f);

    static const char *extensions[][4];
    int                nFilesWritten;
    char              *oldFileBase;
};


#endif


