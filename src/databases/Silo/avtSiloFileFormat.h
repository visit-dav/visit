// ************************************************************************* //
//                           avtSiloFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_SILO_FILE_FORMAT_H
#define AVT_SILO_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <string>
#include <vector>
#include <map>

#include <silo.h>

#include <void_ref_ptr.h>

class     vtkUnstructuredGrid;

class     avtFacelist;
class     avtIntervalTree;
class     avtMaterial;
class     avtSpecies;


typedef struct
{
    bool                haveGroups;
    int                 ndomains;
    int                 numgroups;
    std::vector<int>    ids;
} GroupInfo;


// ****************************************************************************
//  Class: avtSiloFileFormat
//
//  Purpose:
//      A Silo file format reader.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Mar 20 09:53:40 PST 2001
//    Allowed for reading vector variables.
//
//    Hank Childs, Fri May 11 11:35:16 PDT 2001
//    Added I/O hints to tune performance.
//
//    Kathleen Bonnell, Tue Jun 12 13:23:24 PDT 2001 
//    Add method to get ghost zones from quad meshes. 
//
//    Hank Childs, Fri Nov  2 14:41:59 PST 2001
//    Read in variables from directories beyond the top level.
//
//    Eric Brugger, Fri Dec  7 13:39:42 PST 2001
//    I modified CloseFile to be virtual.
//
//    Jeremy Meredith, Thu Dec 13 17:52:43 PST 2001
//    Added species support.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Jeremy Meredith, Wed Mar 19 11:51:39 PST 2003
//    Changed DetermineMultiMeshForSubVariable to handle more advanced
//    matching.  Added some more data members to this effect.
//
//    Jeremy Meredith, Tue Jul 15 09:23:45 PDT 2003
//    Split out some of ReadDir into its own functions so that parallelizing
//    the file reading was easier.
//
//    Hank Childs, Thu Aug 14 09:15:55 PDT 2003
//    Remove vector defvars support (it is now handled correctly).
//
//    Mark C. Miller, Mon Feb  9 16:10:16 PST 2004
//    Added interface method, ActivateTimestep
//
//    Hank Childs, Fri Feb 13 17:18:04 PST 2004
//    Added argument to CalcMaterial.
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Added GetFile() method
//    Added optional bools to skip global information during OpenFile
//
//    Mark C. Miller, Mon Aug  9 19:12:24 PDT 2004
//    Added methods to get global node and zone ids
//    Added method to allocate and determine meshame for a ucd mesh
//
//    Mark C. Miller, Thu Oct 14 15:18:31 PDT 2004
//    Added GetSpatialExtents and GetDataExtents
//
//    Mark C. Miller, Thu Oct 21 22:11:28 PDT 2004
//    Added arbMeshZoneRangesToSkip to support DBzonelists with arb. zones.
//
//    Kathleen Bonnell, Tue Feb  8 13:41:05 PST 2005 
//    Added int arg to GetQuadMesh. 
//
// ****************************************************************************

class avtSiloFileFormat : public avtSTMDFileFormat
{
  public:
                          avtSiloFileFormat(const char *);
    virtual              ~avtSiloFileFormat();
    
    virtual void          FreeUpResources(void);
    virtual const char   *GetType(void) { return "Silo File Format"; };
    
    virtual void         *GetAuxiliaryData(const char *var, int,
                                           const char *type, void *args,
                                           DestructorFunction &);

    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);

    avtMaterial          *GetMaterial(int, const char *);
    avtSpecies           *GetSpecies(int, const char *);
    avtFacelist          *GetExternalFacelist(int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);
    virtual void          PopulateIOInformation(avtIOInformation &);

    void                  ActivateTimestep(void);

  protected:
    DBfile              **dbfiles;
    int                   tocIndex;
    bool                  readGlobalInfo;
    bool                  hasDisjointElements;

    static bool           madeGlobalSiloCalls;

    std::vector<std::string>    domainDirs;

    // The following fields are for determining multimeshes for multivars
    std::vector<std::string>                  firstSubMesh;
    std::vector<std::string>                  firstSubMeshVarName;
    std::vector<std::vector<std::string> >    allSubMeshDirs;
    std::vector<std::string>                  actualMeshName;
    std::vector<int>                          blocksForMesh;
    std::map<std::string,std::vector<int> >   blocksForMultivar;

    std::vector<DBmultimesh *>                multimeshes;
    std::vector<std::string>                  multimesh_name;
    std::vector<DBmultivar *>                 multivars;
    std::vector<std::string>                  multivar_name;
    std::vector<DBmultimat *>                 multimats;
    std::vector<std::string>                  multimat_name;
    std::vector<DBmultimatspecies *>          multimatspecies;
    std::vector<std::string>                  multimatspec_name;

    GroupInfo                                 groupInfo;

    std::map<std::string, std::vector<int> >  arbMeshZoneRangesToSkip;


    DBfile               *GetFile(int);
    DBfile               *OpenFile(int, bool skipGlobalInfo = false);
    DBfile               *OpenFile(const char *, bool skipGlobalInfo = false);
    virtual void          CloseFile(int);
    void                  ReadDir(DBfile *,const char *,avtDatabaseMetaData *);
    void                  DoRootDirectoryWork(avtDatabaseMetaData*);
    void                  BroadcastGlobalInfo(avtDatabaseMetaData*);

    vtkDataArray         *GetQuadVar(DBfile *, const char *, const char *,int);
    vtkDataArray         *GetUcdVar(DBfile *, const char *, const char *, int);
    vtkDataArray         *GetPointVar(DBfile *, const char *);

    vtkDataArray         *GetStandardVectorVar(int, const char *);

    vtkDataArray         *GetQuadVectorVar(DBfile*, const char *, const char *,
                                           int);
    vtkDataArray         *GetUcdVectorVar(DBfile*,const char*,const char*,int);
    vtkDataArray         *GetPointVectorVar(DBfile *, const char *);

    vtkDataSet           *CreateCurvilinearMesh(DBquadmesh *);
    vtkDataSet           *CreateRectilinearMesh(DBquadmesh *);
    vtkDataSet           *GetPointMesh(DBfile *, const char *);
    vtkDataSet           *GetQuadMesh(DBfile *, const char *, int);
    vtkDataSet           *GetUnstructuredMesh(DBfile *, const char *,
                                              int, const char *);
    void                  ReadInConnectivity(vtkUnstructuredGrid *,
                                             DBzonelist *, int,
                                             std::vector<int>&);
    void                  GetConnectivityAndGroupInformation(DBfile *);
    void                  GetConnectivityAndGroupInformationFromFile(DBfile *,
                               int &, int *&,int *&,int &,int *&,int &,int *&);
    void                  FindStandardConnectivity(DBfile *, int &, int *&,
                             int *&, int &, int *&, int &, int *&, bool, bool);
    void                  FindGmapConnectivity(DBfile *, int &, int *&, int *&,
                                     int &, int *&, int &, int *&, bool, bool);

    avtFacelist          *CalcExternalFacelist(DBfile *, char *);
    avtMaterial          *CalcMaterial(DBfile *, char *, const char *, int dom);
    avtSpecies           *CalcSpecies(DBfile *, char *);
    vtkDataArray         *GetGlobalNodeIds(int, const char *);
    vtkDataArray         *GetGlobalZoneIds(int, const char *);
    
    avtIntervalTree      *GetSpatialExtents(const char *);
    avtIntervalTree      *GetDataExtents(const char *);

    void                  GetQuadGhostZones(DBquadmesh *, vtkDataSet *);
    void                  VerifyQuadmesh(DBquadmesh *, const char *);
    void                  DetermineFileAndDirectory(char *, DBfile *&,char *&);
    void                  DetermineFilenameAndDirectory(char *,char *,char *&);
    void                  GetRelativeVarName(const char *,const char *,char *);
    char                 *AllocAndDetermineMeshnameForUcdmesh(int, const char *);

    std::string           DetermineMultiMeshForSubVariable(DBfile*,const char*,
                                                      char**,int, const char*);
    int                   GetMeshtype(DBfile *, char *);
    void                  GetMeshname(DBfile *, char *, char *);
    void                 *GetComponent(DBfile *, char *, char *);

    void                  GetTimeVaryingInformation(DBfile *,
                              avtDatabaseMetaData *md = NULL);
    void                  GetVectorDefvars(const char *);
    void                  RegisterDomainDirs(const char * const *, int,
                                             const char*);
    bool                  ShouldGoToDir(const char *);
    void                  ReadGlobalInformation(DBfile *);

    DBmultimesh          *GetMultimesh(const char *path, const char *name);
    DBmultimesh          *QueryMultimesh(const char *path, const char *name);
    DBmultivar           *GetMultivar(const char *path, const char *name);
    DBmultivar           *QueryMultivar(const char *path, const char *name);
    DBmultimat           *GetMultimat(const char *path, const char *name);
    DBmultimat           *QueryMultimat(const char *path, const char *name);
    DBmultimatspecies    *GetMultimatspec(const char *path, const char *name);
    DBmultimatspecies    *QueryMultimatspec(const char *path, const char *name);
};


#endif


