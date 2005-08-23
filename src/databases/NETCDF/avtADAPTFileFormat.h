#ifndef AVT_ADAPT_FILE_FORMAT_H
#define AVT_ADAPT_FILE_FORMAT_H
#include <avtSTSDFileFormat.h>

class NETCDFFileObject;
class avtFileFormatInterface;

// ****************************************************************************
// Class: avtADAPTFileFormat
//
// Purpose:
//   Reads ADAPT data from a NETCDF file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 09:51:09 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class avtADAPTFileFormat : public avtSTSDFileFormat
{
  public:
    static bool        Identify(NETCDFFileObject *);
    static avtFileFormatInterface *CreateInterface(
                       NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtADAPTFileFormat(const char *filename,
                                          NETCDFFileObject *);
                       avtADAPTFileFormat(const char *filename);
    virtual           ~avtADAPTFileFormat();

    virtual const char    *GetType(void) { return "ADAPT"; }
    virtual void           ActivateTimestep(void);
    virtual void           FreeUpResources(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    std::string            GetGridFileName();
    NETCDFFileObject      *GetMeshFile();

    vtkDataSet            *CreateMain2D(const float *, int, const float *, int);
    vtkDataSet            *CreateMain(const float *, int, const float *, int);
  private:
    NETCDFFileObject      *fileObject;
    NETCDFFileObject      *meshFile;
};

#endif
