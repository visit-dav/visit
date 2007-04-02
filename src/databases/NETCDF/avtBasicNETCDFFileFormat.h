#ifndef BASIC_NETCDF_READER_H
#define BASIC_NETCDF_READER_H
#include <avtSTSDFileFormat.h>
#include <vectortypes.h>
#include <map>

class NETCDFFileObject;
class avtFileFormatInterface;

// ****************************************************************************
//  Class: avtBasicNETCDFFileFormat
//
//  Purpose:
//      Reads in NETCDF files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Aug 10 15:21:14 PST 2005
//
//  Modifications:
//    Brad Whitlock, Wed Apr 26 17:39:32 PST 2006
//    Added a new flag.
//
//    Brad Whitlock, Tue May 16 14:00:05 PST 2006
//    Added override of GetCycleFromFilename.
//
// ****************************************************************************

class avtBasicNETCDFFileFormat : public avtSTSDFileFormat
{
public: 
   static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtBasicNETCDFFileFormat(const char *filename, 
                                                NETCDFFileObject *);
                       avtBasicNETCDFFileFormat(const char *filename);
    virtual           ~avtBasicNETCDFFileFormat();

    virtual const char    *GetType(void) { return "NETCDF"; }
    virtual void           ActivateTimestep(void); 
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);

protected:
    virtual int            GetCycleFromFilename(const char *f) const;
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    bool                   ReturnValidDimensions(const intVector &dims,
                                                 int validDims[3],
                                                 int &nValidDims);
private:
    
    NETCDFFileObject      *fileObject;

    typedef std::map<std::string, intVector> MeshNameMap;
    bool                   meshNamesCreated;
    MeshNameMap            meshNames;
};

#endif
