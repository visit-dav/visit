#ifndef AVT_LODI_FILE_FORMAT_H
#define AVT_LODI_FILE_FORMAT_H
#include <avtMTSDFileFormat.h>
#include <vectortypes.h>

class NETCDFFileObject;
class avtFileFormatInterface;

// ****************************************************************************
// Class: avtLODIFileFormat
//
// Purpose:
//   Reads LODI data from a NETCDF file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 15 09:51:09 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class avtLODIFileFormat : public avtMTSDFileFormat
{
public:
    static bool        Identify(NETCDFFileObject *);
    static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f,
                       const char *const *list, int nList, int nBlock);

                       avtLODIFileFormat(const char *filename,
                                         NETCDFFileObject *);
                       avtLODIFileFormat(const char *filename);
    virtual           ~avtLODIFileFormat();

    virtual const char    *GetType(void) { return "LODI"; }
    virtual void           ActivateTimestep(int ts);
    virtual void           FreeUpResources(void);

    virtual int            GetNTimesteps(void);
    virtual void           GetTimes(std::vector<double> &);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

  private:
    bool                   ReadTimes();

    NETCDFFileObject      *fileObject;
    doubleVector           times;
    bool                   timesRead;
};

#endif
