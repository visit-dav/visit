
// ************************************************************************* //
//                            avtGTCFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_GTC_FILE_FORMAT_H
#define AVT_GTC_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <hdf5.h>

// ****************************************************************************
//  Class: avtGTCFileFormat
//
//  Purpose:
//      Reads in GTC files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 27 13:18:55 PST 2006
//
//  Modifications:
//
// ****************************************************************************

class avtGTCFileFormat : public avtSTSDFileFormat
{
  public:
                       avtGTCFileFormat(const char *filename);
    virtual           ~avtGTCFileFormat();

    virtual const char    *GetType(void)   { return "GTC"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

  protected:
    bool                   Initialize(bool, bool);
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    hid_t                  fileId;
    int                    nVars;
    bool                   initialized;
};


#endif
