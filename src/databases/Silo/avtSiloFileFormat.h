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
class     avtMaterial;
class     avtSpecies;


typedef struct
{
    std::string    vector_name;
    std::string    component1;
    std::string    component2;
    std::string    component3;
}  VectorDefvar;

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

  protected:
    DBfile              **dbfiles;
    int                   tocIndex;
    bool                  readGlobalInfo;
    bool                  hasDisjointElements;

    static bool           madeGlobalSiloCalls;

    std::vector<VectorDefvar>   defvars;
    std::vector<std::string>    domainDirs;

    // The following fields are for determining multimeshes for multivars
    std::vector<std::string>                  firstSubMesh;
    std::vector<std::string>                  firstSubMeshVarName;
    std::vector<std::vector<std::string> >    allSubMeshDirs;
    std::vector<std::string>                  actualMeshName;
    std::vector<int>                          blocksForMesh;
    std::map<std::string,std::vector<int> >   blocksForMultivar;

    GroupInfo                                 groupInfo;


    DBfile               *OpenFile(int);
    DBfile               *OpenFile(const char *);
    virtual void          CloseFile(int);
    void                  ReadDir(DBfile *,const char *,avtDatabaseMetaData *);
    void                  DoRootDirectoryWork(avtDatabaseMetaData*);
    void                  BroadcastGlobalInfo(avtDatabaseMetaData*);

    vtkDataArray         *GetQuadVar(DBfile *, const char *, const char *,int);
    vtkDataArray         *GetUcdVar(DBfile *, const char *, const char *, int);
    vtkDataArray         *GetPointVar(DBfile *, const char *);

    vtkDataArray         *GetDefvarVectorVar(int, int);
    vtkDataArray         *GetStandardVectorVar(int, const char *);

    vtkDataArray         *GetQuadVectorVar(DBfile*, const char *, const char *,
                                           int);
    vtkDataArray         *GetUcdVectorVar(DBfile*,const char*,const char*,int);
    vtkDataArray         *GetPointVectorVar(DBfile *, const char *);

    vtkDataSet           *CreateCurvilinearMesh(DBquadmesh *);
    vtkDataSet           *CreateRectilinearMesh(DBquadmesh *);
    vtkDataSet           *GetPointMesh(DBfile *, const char *);
    vtkDataSet           *GetQuadMesh(DBfile *, const char *);
    vtkDataSet           *GetUnstructuredMesh(DBfile *, const char *,
                                              int, const char *);
    void                  ReadInConnectivity(vtkUnstructuredGrid *,
                                             DBzonelist *, int);
    void                  GetConnectivityAndGroupInformation(DBfile *);
    void                  GetConnectivityAndGroupInformationFromFile(DBfile *,
                               int &, int *&,int *&,int &,int *&,int &,int *&);
    void                  FindStandardConnectivity(DBfile *, int &, int *&,
                             int *&, int &, int *&, int &, int *&, bool, bool);
    void                  FindGmapConnectivity(DBfile *, int &, int *&, int *&,
                                     int &, int *&, int &, int *&, bool, bool);

    avtFacelist          *CalcExternalFacelist(DBfile *, char *);
    avtMaterial          *CalcMaterial(DBfile *, char *);
    avtSpecies           *CalcSpecies(DBfile *, char *);

    void                  GetQuadGhostZones(DBquadmesh *, vtkDataSet *);
    void                  VerifyQuadmesh(DBquadmesh *, const char *);
    void                  DetermineFileAndDirectory(char *, DBfile *&,char *&);
    void                  DetermineFilenameAndDirectory(char *,char *,char *&);
    void                  GetRelativeVarName(const char *,const char *,char *);

    std::string           DetermineMultiMeshForSubVariable(DBfile*,const char*,
                                                      char**,int, const char*);
    int                   GetMeshtype(DBfile *, char *);
    void                  GetMeshname(DBfile *, char *, char *);
    void                 *GetComponent(DBfile *, char *, char *);

    void                  GetTimeVaryingInformation(DBfile *);
    void                  GetVectorDefvars(const char *);
    void                  RegisterDomainDirs(const char * const *, int,
                                             const char*);
    bool                  ShouldGoToDir(const char *);
    void                  ReadGlobalInformation(DBfile *);
};


#endif


