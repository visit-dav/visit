// ************************************************************************* //
//                            avtPixieFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Pixie_FILE_FORMAT_H
#define AVT_Pixie_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>

#include <hdf5.h>
#include <vector>
#include <map>

// ****************************************************************************
//  Class: avtPixieFileFormat
//
//  Purpose:
//      Reads in Pixie files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Aug 13 14:31:43 PST 2004
//
// ****************************************************************************

class avtPixieFileFormat : public avtMTSDFileFormat
{
    struct TraversalInfo
    {
        avtPixieFileFormat *This;
        int                level;
        std::string        path;
    };

    struct VarInfo
    {
        hsize_t     dims[3];
        hid_t       nativeVarType;
        std::string fileVarName;
    };

    typedef std::map<std::string, VarInfo> VarInfoMap;
public:
                       avtPixieFileFormat(const char *);
    virtual           ~avtPixieFileFormat();

    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "Pixie"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

protected:
    void                   Initialize();
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    hid_t                  fileId;
    VarInfoMap             variables;
    VarInfoMap             meshes;
    int                    nTimeStates;

    static herr_t GetVariableList(hid_t, const char *, void *);
};


#endif
