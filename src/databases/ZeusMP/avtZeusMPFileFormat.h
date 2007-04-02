
// ************************************************************************* //
//                            avtZeusMPFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_ZeusMP_FILE_FORMAT_H
#define AVT_ZeusMP_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <hdfi.h>

#include <vector>
#include <map>

// ****************************************************************************
//  Class: avtZeusMPFileFormat
//
//  Purpose:
//      Reads in ZeusMPHDF files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jan 13 10:36:24 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

class avtZeusMPFileFormat : public avtSTSDFileFormat
{
    struct DatasetInfo
    {
        int32 index;
        int32 ndims;
        int32 dims[3];
    };

    typedef std::map<std::string, DatasetInfo> DatasetInfoMap;
public:
                           avtZeusMPFileFormat(const char *filename);
    virtual               ~avtZeusMPFileFormat();

    virtual bool           ReturnsValidTime() const;
    virtual double         GetTime(void);

    virtual const char    *GetType(void)   { return "ZeusMP HDF"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

    virtual void           ActivateTimestep(void);
protected:
    int32 GetFileHandle();
    void  GetFileInformation();

    // DATA MEMBERS
    int32                  hdffile;
    int                    meshCoordinateSystem;
    DatasetInfoMap         varsToDatasets;
    bool                   haveFileInformation;
    bool                   haveFileTime;
    double                 fileTime;
    vtkDataArray          *coords[3];

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
