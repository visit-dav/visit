/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                           avtSiloFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_SILO_FILE_FORMAT_H
#define AVT_SILO_FILE_FORMAT_H

#include <avtDataSelection.h>
#include <avtIOInformation.h>
#include <avtSTMDFileFormat.h>
#include <void_ref_ptr.h>

#include <silo.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include <avtSiloMBObjectCache.h>
#include <avtLocalStructuredDomainBoundaries.h>

class     vtkUnstructuredGrid;

class     avtFacelist;
class     avtIntervalTree;
class     avtMaterial;
class     avtSpecies;

class     DBOptionsAttributes;

typedef struct _GroupInfo
{
    _GroupInfo() : haveGroups(false), ndomains(0), numgroups(0) {}
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
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added methods to GetCycle/Time
//
//    Mark C. Miller, Wed Aug 10 12:00:00 PDT 2005
//    Added resample data selections to support CSG mesh discretization
//
//    Jeremy Meredith, Tue Sep 13 15:57:05 PDT 2005
//    Changed domainDirs to a set to ensure log(n) search times.
//
//    Mark C. Miller, Thu Sep 15 11:30:18 PDT 2005
//    Added CanCacheVariable, primarily to support csg discretization.
//    It will be removed after 3d, adaptive csg discretization is available
//    in avtGenericDatabase
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Removed CanCacheVariable 
//
//    Mark C. Miller, Wed Nov 29 15:08:21 PST 2006
//    Added connectivityIsTimeVarying
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Removed RegisterDataSelections. Was only used by CSG stuff 
//    Added dontForceSingle. Added support for CSG multimeshes and CSG vars
//
//    Mark C. Miller, Wed Dec 13 16:55:30 PST 2006
//    Added method/data member for explicit multivar to multimesh mapping to
//    support time-invariant multimeshes stored in separate files from vars.
//
//    Mark C. Miller, Mon Jul  9 14:23:22 PDT 2007
//    Added data member codeNameGuess which is to store a guess of the name
//    of the code that produced the data.
//
//    Mark C. Miller, Tue Aug 28 19:17:44 PDT 2007
//    Made it deal with case where multimesh and blocks are all in same
//    dir in the file. Added args to DetermineFile[name]AndDirectory to
//    support this.
//
//    Cyrus Harrison, Fri Sep  7 15:24:27 PDT 2007
//    Added FindMultiMeshAdjConnectivity to support the Silo MultiMesh
//    Adjacency object.
//
//    Hank Childs, Mon Oct  8 13:01:31 PDT 2007
//    Added optional argument to GetConnectivityAndGroupInformation.
//
//    Mark C. Miller, Wed Feb  6 12:27:09 PST 2008
//    Added topDir data member for case where entire time series is in 
//    a single silo file.
//
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Added 'using' statements (above) and removed all 'std::' where
//    appropriate. Added GetNodelistVars, AddNodelistEnumerations
//
//    Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//    Added bools to control ignoring of extents.
//
//    Jeremy Meredith, Thu Aug  7 16:16:00 EDT 2008
//    Made GetComponent accept const char*'s as input.
//
//    Cyrus Harrison, Mon Oct 27 10:33:39 PDT 2008
//    Added FindDecomposedMeshType() to help with creating the correct type of
//    domain boundries object in GetConnectivityAndGroupInformation.
//
//    Mark C. Miller, Tue Dec 23 22:14:30 PST 2008
//    Added support for ANNOTATION_INT nodelists (special case)
//
//    Mark C. Miller, Wed Mar  4 08:54:57 PST 2009
//    Added tri-state variables to better manage ignoring of extents. 
//
//    Mark C. Miller, Mon Mar 16 23:33:32 PDT 2009
//    Removed usingOldExtents. Moved logic for 'old' extents interface to
//    CommonPluginInfo where old (obsolete) options can be merged with current
//    interface.
//
//    Mark C. Miller, Thu Jun 18 20:54:17 PDT 2009
//    Replaced DBtoc* arg to ReadXXX functions with list of names of objects
//    to process. Added RemoveMultixxx functions to help manage cached
//    multi-objects in presence of exceptions.
//
//    Mark C. Miller, Mon Mar 16 23:33:32 PDT 2009
//    Removed usingOldExtents. Moved logic for 'old' extents interface to
//    CommonPluginInfo where old (obsolete) options can be merged with current
//    interface.
//
//    Mark C. Miller, Thu Jun 18 20:54:17 PDT 2009
//    Replaced DBtoc* arg to ReadXXX functions with list of names of objects
//    to process. Added RemoveMultixxx functions to help manage cached
//    multi-objects in presence of exceptions.
//
//    Mark C. Miller, Thu Oct 29 15:15:37 PDT 2009
//    Removed arbMeshZoneRangesToSkip. Added HandleGlobalZoneIds. Adjusted
//    interface to ReadInConnectivity.
//
//    Mark C. Miller, Tue Dec 15 13:50:40 PST 2009
//    Added HasInvariantMetadata/SIL and metadataIsTimeVarying.
//
//    Mark C. Miller, Tue Jan 12 17:48:39 PST 2010
//    Made HandleGlobalZoneIds work for different versions of Silo.
//
//    Mark C. Miller, Wed Jan 27 13:13:20 PST 2010
//    Added an extra level of indirection to the arbMeshXXXRemap maps to
//    make sure they work for multi-block case.
//
//    Cyrus Harrison, Wed Mar 24 10:59:17 PDT 2010
//    Added haveAmrGroupInfo, which is used to prevent collsion of amr
//    levels info with connectivity group info.
//
//    Mark C. Miller, Mon Mar 29 17:30:10 PDT 2010
//    Added siloDriver so we know what driver succeeded in openig the file.
//
//    Eric Brugger, Thu May 27 15:54:50 PDT 2010
//    I added RemapFacelistForPolyhedronZones.
//
//    Cyrus Harrison, Mon Jun 14 15:45:46 PDT 2010
//    Added metadataIsTimeVaryingChecked & CheckForTimeVaryingMetadata().
//
//    Mark C. Miller, Sun Aug 29 23:19:50 PDT 2010
//    Added method to expand (material) subsetted ucd variables.
//
//    Cyrus Harrison,
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Mon Jul  9 21:24:45 PDT 2012
//    Added method to handle nodelists vars from MRG Trees. Adjusted interfaces
//    that read UCD mesh and arb. connectivity to support repeating calls for
//    different segments of a zonelist.
//
//    Mark C. Miller, Tue Jul 10 20:11:42 PDT 2012
//    Removed RemapFacelistForPolyhedronZones since it did not use any class
//    members and could therefore be defined as a static function in the .C
//    file.
//
//    Cyrus Harrison, Wed Mar 13 09:55:42 PDT 2013
//    Added useLocalDomainBoundries.
//
//    Brad Whitlock, Thu Jun 19 11:24:21 PDT 2014
//    Pass meshname to PopulateIOInformation.
//
//    Eric Brugger, Wed Nov 19 08:28:22 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
//    Mark C. Miller, Tue Feb  2 14:48:25 PST 2016
//    Added firstAllEmptyMultimesh and emptyObjectList to handle cases where
//    a multi-mesh consists of all empty blocks.
//
//    Mark C. Miller, Wed Jun 15 09:16:56 PDT 2016
//    Added support for loading the block decomposition as a variable
//    under certain conditions.
// ****************************************************************************

class avtSiloFileFormat : public avtSTMDFileFormat
{
  public:
                          avtSiloFileFormat(const char *, DBOptionsAttributes*);
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
    virtual bool          PopulateIOInformation(const std::string &meshname, avtIOInformation &);

    int                   GetCycle();
    int                   GetCycleFromFilename(const char *f) const;
    double                GetTime();

    void                  ActivateTimestep(void);
    bool                  HasInvariantMetaData(void) const { return !metadataIsTimeVarying; };
    bool                  HasInvariantSIL(void) const { return !metadataIsTimeVarying; };

  protected:
    enum AANTriState {Always=0, Auto, Never};
    DBfile              **dbfiles;
    int                   tocIndex;
    int                   forceSingle;
    int                   siloDriver;
    AANTriState           ignoreSpatialExtentsAAN;
    AANTriState           ignoreDataExtentsAAN;
    bool                  ignoreSpatialExtents;
    bool                  ignoreDataExtents;
    bool                  searchForAnnotInt;
    bool                  readGlobalInfo;
    bool                  connectivityIsTimeVarying;
    bool                  metadataIsTimeVarying;
    bool                  metadataIsTimeVaryingChecked;
    bool                  hasDisjointElements;
    std::string           codeNameGuess;
    bool                  addBlockDecompositionAsVar;
    bool                  haveAddedBlockDecompositionAsVar;
    bool                  ignoreMissingBlocks;

    bool                  ioInfoValid;
    avtIOInformation      ioInfo;

    std::set<std::string> domainDirs;

    // The following fields are for determining multimeshes for multivars
    std::vector<std::string>             firstSubMesh;
    std::vector<std::string>             firstSubMeshVarName;
    std::vector<std::vector<std::string> > allSubMeshDirs;
    std::vector<std::string>             actualMeshName;
    std::vector<int>                     blocksForMesh;
    std::map<std::string,std::vector<int> >   blocksForMultivar;

    avtSiloMBObjectCache multimeshCache;
    avtSiloMBObjectCache multivarCache;
    avtSiloMBObjectCache multimatCache;
    avtSiloMBObjectCache multispecCache;

    std::string                          firstAllEmptyMultimesh;
    std::map<std::string, bool>          emptyObjectsList;

    std::map<std::string, std::string>   multivarToMultimeshMap;

    GroupInfo                       groupInfo;
    bool                            haveAmrGroupInfo;
    bool                            useLocalDomainBoundries;
    std::vector<int>                domainToBlockGrouping;


    std::map<std::string, std::map<int, std::vector<int>* > > arbMeshCellReMap;
    std::map<std::string, std::map<int, std::vector<int>* > > arbMeshNodeReMap;

    std::vector<avtDataSelection_p>      selList;
    std::vector<bool>                   *selsApplied;

    std::string                          topDir;

    // to support block structured nodelist convention
    std::map<int, std::vector<int> > nlBlockToWindowsMap;
    int                             numNodeLists;
    int                             numAnnotIntLists;

    DBfile               *GetFile(int);
    DBfile               *OpenFile(int, bool skipGlobalInfo = false);
    DBfile               *OpenFile(const char *, bool skipGlobalInfo = false);
    virtual void          CloseFile(int);

    void                  ReadDir(DBfile*,const char*,avtDatabaseMetaData*);
    void                  ReadTopDirStuff(DBfile*,const char*,avtDatabaseMetaData*,char**);
    void                  ReadMultimeshes(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadQuadmeshes(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadUcdmeshes(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadPointmeshes(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadCurves(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadCSGmeshes(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadMultivars(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadQuadvars(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadUcdvars(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadPointvars(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadCSGvars(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadMaterials(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadMultimats(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadSpecies(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadMultispecies(DBfile*,int,char**,const char*,avtDatabaseMetaData*);
    void                  ReadDefvars(DBfile*,int,char**,const char*,avtDatabaseMetaData*);

    void                  DoRootDirectoryWork(avtDatabaseMetaData*);
    void                  BroadcastGlobalInfo(avtDatabaseMetaData*);
    void                  StoreMultimeshInfo(const char *const dirname,
                                             const char *const name_w_dir,
                                             int meshnum,
                                             avtSiloMultiMeshCacheEntry *obj);
    void                  AddCSGMultimesh(const char *const dirname,
                                          const char *const multimesh_name,
                                          avtDatabaseMetaData *md,
                                          avtSiloMultiMeshCacheEntry *mm_ent,
                                          DBfile *dbfile);

    vtkDataArray         *GetNodelistsVar(int);
    vtkDataArray         *GetAnnotIntNodelistsVar(int, std::string);
    vtkDataArray         *GetMrgTreeNodelistsVar(int, std::string);
    vtkDataArray         *GetBlockDecompositionAsVar(int, std::string);
    vtkDataArray         *GetQuadVar(DBfile *, const char *, const char *,int);
    void                  ExpandUcdvar(DBucdvar *uv, const char *vname, const char *tvn,
                              int domain);
    vtkDataArray         *GetUcdVar(DBfile *, const char *, const char *, int);
    vtkDataArray         *GetPointVar(DBfile *, const char *);
    vtkDataArray         *GetCsgVar(DBfile *, const char *);

    vtkDataArray         *GetStandardVectorVar(int, const char *);

    vtkDataArray         *GetQuadVectorVar(DBfile*, const char *, const char *,
                                           int);
    vtkDataArray         *GetUcdVectorVar(DBfile*,const char*,const char*,int);
    vtkDataArray         *GetPointVectorVar(DBfile *, const char *);
    vtkDataArray         *GetCsgVectorVar(DBfile *, const char *);

    vtkDataSet           *CreateCurvilinearMesh(DBquadmesh *);
    vtkDataSet           *CreateRectilinearMesh(DBquadmesh *);
    vtkDataSet           *GetCSGMesh(DBfile *, const char *, int);
    vtkDataSet           *GetPointMesh(DBfile *, const char *, int);
    vtkDataSet           *GetQuadMesh(DBfile *, const char *, int);
    vtkDataSet           *GetCurve(DBfile *, const char *);
    vtkDataSet           *GetUnstructuredMesh(DBfile *, const char *,
                                              int, const char *);

#ifdef SILO_VERSION_GE
#if SILO_VERSION_GE(4,7,1)
    void                  HandleGlobalZoneIds(const char *, int, void*, int, int);
#else
    void                  HandleGlobalZoneIds(const char *, int, int*, int);
#endif
#else
    void                  HandleGlobalZoneIds(const char *, int, int*, int);
#endif
    void                  ReadInConnectivity(vtkUnstructuredGrid *,
                              DBucdmesh *um, const char *, int);
    void                  ReadInArbConnectivity(const char *, vtkUnstructuredGrid*,
                                                DBucdmesh*,int,int=0);
    avtMeshType           FindDecomposedMeshType(DBfile *dbfile);
    void                  GetConnectivityAndGroupInformation(DBfile *, bool = false);
    void                  GetConnectivityAndGroupInformationFromFile(DBfile *,
                               int &, int *&,int *&,int &,int *&,int &,int *&);
    void                  FindStandardConnectivity(DBfile *, int &, int *&,
                             int *&, int &, int *&, int &, int *&, bool, bool);
    void                  FindGmapConnectivity(DBfile *, int &, int *&, int *&,
                                     int &, int *&, int &, int *&, bool, bool);
    void                  FindMultiMeshAdjConnectivity(DBfile *, int &, int *&,
                                     int *&, int &, int *&, int &, int *&,
                                     bool, bool);

    void                  GetMultivarToMultimeshMap(DBfile *);

    avtFacelist          *CalcExternalFacelist(DBfile *, const char *);
    avtMaterial          *CalcMaterial(DBfile *, const char *, const char *, int dom);
    avtSpecies           *CalcSpecies(DBfile *, const char *);
    vtkDataArray         *GetGlobalNodeIds(int, const char *);
    vtkDataArray         *GetGlobalZoneIds(int, const char *);

    avtIntervalTree      *GetSpatialExtents(const char *);
    avtIntervalTree      *GetDataExtents(const char *);

    avtLocalStructuredDomainBoundaryList *
                          GetLocalDomainBoundaryInfo(int, const char *);

    void                  GetQuadGhostZones(DBquadmesh *, vtkDataSet *);
    void                  VerifyQuadmesh(DBquadmesh *, const char *);

    void                  DetermineFileAndDirectory(const char *input,
                                                    const char *mesh_dirname,
                                                    DBfile *&dbfile_out,
                                                    std::string &location_out);

    void                  DetermineFilenameAndDirectory(const char *input,
                                                        const char *mesh_dirname,
                                                        std::string &filename_out,
                                                        std::string &location_out);

    void                  GetRelativeVarName(const char *,const char *,char *);
    char                 *AllocAndDetermineMeshnameForUcdmesh(int, const char *);

    std::string           DetermineMultiMeshForSubVariable(DBfile *dbfile,
                                                           const char *name,
                                                           avtSiloMBObjectCacheEntry *obj,
                                                           const char *curdir);
    int                   GetMeshname(DBfile *, const char *, char *);
    void                 *GetComponent(DBfile *, char *, const char *);

    void                  GetTimeVaryingInformation(DBfile *,
                              avtDatabaseMetaData *md = NULL);
    void                  GetVectorDefvars(const char *);
    void                  RegisterDomainDirs(avtSiloMBObjectCacheEntry *obj,
                                             const char *dirname);
    bool                  ShouldGoToDir(const char *);
    void                  ReadGlobalInformation(DBfile *);

    void                  AddNodelistEnumerations(DBfile *dbfile, avtDatabaseMetaData *md,
                              std::string meshname);

    void                  GetMeshHelper(int domain, const char *m, DBmultimesh **mm, int *type,
                              DBfile **_domain_file, std::string &directory_mesh_out);
    void                  AddAnnotIntNodelistEnumerations(DBfile *dbfile, avtDatabaseMetaData *md,
                              std::string meshname, avtSiloMultiMeshCacheEntry *obj);

    void GetMultiMesh(const char *path, const char *name,
        avtSiloMultiMeshCacheEntry **mm_ent, bool *valid_var=0);
    avtSiloMultiMeshCacheEntry *QueryMultiMesh(const char *path, const char *name);

    void GetMultiVar(const char *path, const char *name,
        avtSiloMultiVarCacheEntry **mv_ent, bool *valid_var=0);
    avtSiloMultiVarCacheEntry  *QueryMultiVar(const char *path, const char *name);

    void GetMultiMat(const char *path, const char *name,
        avtSiloMultiMatCacheEntry **mm_ent, bool *valid_var=0);
    avtSiloMultiMatCacheEntry  *QueryMultiMat(const char *path, const char *name);

    void GetMultiSpec(const char *path, const char *name,
        avtSiloMultiSpecCacheEntry **ms_ent, bool *valid_var=0);
    avtSiloMultiSpecCacheEntry *QueryMultiSpec(const char *path, const char *name);

    void                        CheckForTimeVaryingMetadata(DBfile *toc);

    bool                  PopulateIOInformationEx(const std::string &meshname,
                                                  avtIOInformation &ioInfo);
};


#endif


