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
//  Modifications:
//    Brad Whitlock, Thu Aug 26 09:54:15 PDT 2004
//    Added support for meshes that have coordinates.
//
//    Brad Whitlock, Wed Sep 15 17:04:01 PST 2004
//    Rewrote some aspects of support for meshes with coordinates so the
//    "coords" attributes are used.
//
// ****************************************************************************

class avtPixieFileFormat : public avtMTSDFileFormat
{
    struct TraversalInfo
    {
        avtPixieFileFormat *This;
        int                level;
        std::string        path;
        bool               hasCoords;
        std::string        coordX;
        std::string        coordY;
        std::string        coordZ;
    };

    struct VarInfo
    {
        bool        timeVarying;
        hsize_t     dims[3];
        hid_t       nativeVarType;
        std::string fileVarName;
        bool        hasCoords;
        std::string coordX;
        std::string coordY;
        std::string coordZ;
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
    virtual void           ActivateTimestep(int ts);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

protected:
    void                   Initialize();
    void                   DetermineVarDimensions(const VarInfo &info,
                                                  int *dims,
                                                  int &nSpatialDims) const;
    bool                   MeshIsCurvilinear(const std::string &) const;
    vtkDataSet            *CreatePointMesh(int timestate, const VarInfo &,
                                           int nDims) const;
    vtkDataSet            *CreateCurvilinearMesh(int timestate, const VarInfo &,
                                                 int nnodes[3],
                                                 int nDims);
    bool                   ReadCoordinateFields(int timestate, const VarInfo &,
                                                float *coords[3], int dims[3],
                                                int nDims) const;
    bool                   ReadVariableFromFile(int timestate,
                                                const std::string &,
                                                const VarInfo &it,
                                                void *dest) const;


    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    hid_t                  fileId;
    VarInfoMap             variables;
    VarInfoMap             meshes;
    int                    nTimeStates;
    bool                   haveMeshCoords;
    std::string            timeStatePrefix;

    static herr_t GetVariableList(hid_t, const char *, void *);
};


#endif
