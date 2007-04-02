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

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    virtual const char    *GetType(void) { return "NETCDF"; }
    virtual void           ActivateTimestep(void); 
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);

  protected:
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
