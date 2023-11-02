// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtDatasetFileWriter.h                          //
// ************************************************************************* //

#ifndef AVT_DATASET_FILE_WRITER_H
#define AVT_DATASET_FILE_WRITER_H

#include <file_writer_exports.h>

#include <visitstream.h>
#include <string>
#include <vector>

#include <avtTerminatingDatasetSink.h>
#include <DBOptionsAttributes.h>

class vtkRectilinearGrid;

typedef enum
{
    CURVE                  = 0,
    OBJ,                  /* 1 */
    STL,                  /* 2 */
    VTK,                  /* 3 */
    ULTRA,                /* 4 */
    POVRAY,               /* 5 */
    PLY                   /* 6 */
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
//    Brad Whitlock, Mon Mar 6 17:35:28 PST 2006
//    I made it reset nFilesWritten if the nase changes.
//
//    Jeremy Meredith, Mon Apr 23 13:54:01 EDT 2007
//    Added POVRay output file format.
//
//    Jeremy Meredith, Thu May 31 11:06:48 EDT 2007
//    Added support for POVRay .df3 density files for volume rendering.
//
//    Dave Pugmire, Thu Jul  8 08:30:11 EDT 2010
//    Added PLY writer.
//
//    Kathleen Biagas, Fri Aug 31 13:21:14 PDT 2018
//    Added DBOptionsAttributes.
//
// ****************************************************************************

class AVTFILEWRITER_API avtDatasetFileWriter : public avtTerminatingDatasetSink
{
  public:
                       avtDatasetFileWriter();
    virtual           ~avtDatasetFileWriter();

    void               Write(DatasetFileFormat, const char *filename, int, int, bool);

    char              *CreateFilename(const char *base, bool family,
                                      DatasetFileFormat);

    void               SetOptions(const DBOptionsAttributes &);

    static void        WriteOBJFile(vtkDataSet *, const char *, const char *);

  protected:
    static const char *extensions[];
    int                nFilesWritten;
    char              *oldFileBase;

    void               WriteSTLFile(const char *, bool);

    void               WritePLYFile(const char *, bool);

    void               WriteCurveFile(const char *, int, int);

    void               WritePOVRayFamily(const char *);
    int                WritePOVRayTree(avtDataTree_p, int, const char *,
                                       double*, double*);
    void               WritePOVRayFile(vtkDataSet*, const char *, int,
                                       double*, double*);
    bool               WritePOVRayDF3File(vtkRectilinearGrid*,
                                          const char *, double[6]);

    void               WriteVTKFamily(const char *, bool);
    int                WriteVTKTree(avtDataTree_p, int, const char *, bool);
    void               WriteVTKFile(vtkDataSet *, const char *, bool);

    void               WriteOBJFamily(const char *);
    int                WriteOBJTree(avtDataTree_p, int, const char *);

    vtkDataSet        *GetSingleDataset(void);
    char              *GenerateName(const char *, const char *,
                                    std::vector<std::string> &);

  private:
    DBOptionsAttributes opts;
};


#endif


