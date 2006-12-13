/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
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
#include <avtSTMDFileFormat.h>

#include <string>
#include <vector>
#include <map>
#include <set>

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

    int                   GetCycle();
    int                   GetCycleFromFilename(const char *f) const;
    double                GetTime();

    void                  ActivateTimestep(void);

  protected:
    DBfile              **dbfiles;
    int                   tocIndex;
    int                   dontForceSingle; // used primarily for testing
    bool                  readGlobalInfo;
    bool                  connectivityIsTimeVarying;
    bool                  hasDisjointElements;

    static bool           madeGlobalSiloCalls;

    std::set<std::string>    domainDirs;

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

    std::map<std::string, std::string>        multivarToMultimeshMap;

    GroupInfo                                 groupInfo;

    std::map<std::string, std::vector<int> >  arbMeshZoneRangesToSkip;

    std::vector<avtDataSelection_p>           selList;
    std::vector<bool>                        *selsApplied;

    DBfile               *GetFile(int);
    DBfile               *OpenFile(int, bool skipGlobalInfo = false);
    DBfile               *OpenFile(const char *, bool skipGlobalInfo = false);
    virtual void          CloseFile(int);
    void                  ReadDir(DBfile *,const char *,avtDatabaseMetaData *);
    void                  DoRootDirectoryWork(avtDatabaseMetaData*);
    void                  BroadcastGlobalInfo(avtDatabaseMetaData*);
    void                  StoreMultimeshInfo(const char *const dirname, int which_mm,
                                             const char *const name_w_dir,
                                             int meshnum, const DBmultimesh *const mm);
    void                  AddCSGMultimesh(const char *const dirname, int which_mm,
                                          const char *const multimesh_name,
                                          avtDatabaseMetaData *md,
                                          const DBmultimesh *const mm, DBfile *dbfile);

    vtkDataArray         *GetQuadVar(DBfile *, const char *, const char *,int);
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
    vtkDataSet           *GetPointMesh(DBfile *, const char *);
    vtkDataSet           *GetQuadMesh(DBfile *, const char *, int);
    vtkDataSet           *GetCurve(DBfile *, const char *);
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

    void                  GetMultivarToMultimeshMap(DBfile *);

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


