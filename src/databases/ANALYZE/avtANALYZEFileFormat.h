
// ************************************************************************* //
//                            avtANALYZEFileFormat.h                         //
// ************************************************************************* //

#ifndef AVT_ANALYZE_FILE_FORMAT_H
#define AVT_ANALYZE_FILE_FORMAT_H

#include <database_exports.h>
#include <avtSTSDFileFormat.h>
#include <DSRObject.h>

// ****************************************************************************
//  Class: avtANALYZEFileFormat
//
//  Purpose:
//      Reads in ANALYZE files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 24 16:51:26 PST 2003
//
// ****************************************************************************

class avtANALYZEFileFormat : public avtSTSDFileFormat
{
  public:
                       avtANALYZEFileFormat(const char *filename);
    virtual           ~avtANALYZEFileFormat() {;};

    virtual const char    *GetType(void)   { return "ANALYZE"; };

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);

  protected:
    void                   Initialize();
    void                   GetMeshExtents(float *) const;

    // DATA MEMBERS
    DSRObject              fileInformation;
    std::string            dataFilename;
    bool                   initialized;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
