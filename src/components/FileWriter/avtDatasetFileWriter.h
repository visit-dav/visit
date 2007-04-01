// ************************************************************************* //
//                           avtDatasetFileWriter.h                          //
// ************************************************************************* //

#ifndef AVT_DATASET_FILE_WRITER_H
#define AVT_DATASET_FILE_WRITER_H

#include <file_writer_exports.h>

#include <visitstream.h>
#include <string>
#include <vector>

#include <avtOriginatingDatasetSink.h>


typedef enum
{
    CURVE                  = 0,
    OBJ,                  /* 1 */
    STL,                  /* 2 */
    VTK,                  /* 3 */
    ULTRA                 /* 4 */
} DatasetFileFormat;


// ****************************************************************************
//  Class: avtDatasetFileWriter
//
//  Purpose:
//      A type of dataset sink that writes the dataset to the specified file
//      format.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//    Jeremy Meredith, Sat Apr 12 15:09:28 PDT 2003
//    Added the Ultra file format.
//
//    Jeremy Meredith, Tue Dec 30 09:16:12 PST 2003
//    Removed the obsolete Curve file format.  Renamed ULTRA to Curve.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers 
//    have problems with them.
//
// ****************************************************************************

class AVTFILEWRITER_API avtDatasetFileWriter : public avtOriginatingDatasetSink
{
  public:
                       avtDatasetFileWriter();
    virtual           ~avtDatasetFileWriter();

    void               Write(DatasetFileFormat, const char *filename, bool);

    char              *CreateFilename(const char *base, bool family,
                                      DatasetFileFormat);

  protected:
    static const char *extensions[];
    int                nFilesWritten;

    void               WriteSTLFile(const char *, bool);

    void               WriteCurveFile(const char *);

    void               WriteVTKFamily(const char *, bool);
    int                WriteVTKTree(avtDataTree_p, int, const char *, bool);
    void               WriteVTKFile(vtkDataSet *, const char *, bool);

    void               WriteOBJFamily(const char *);
    int                WriteOBJTree(avtDataTree_p, int, const char *);
    void               WriteOBJFile(vtkDataSet *, const char *, const char *);

    vtkDataSet        *GetSingleDataset(void);
    char              *GenerateName(const char *, const char *,
                                    std::vector<std::string> &);
};


#endif


