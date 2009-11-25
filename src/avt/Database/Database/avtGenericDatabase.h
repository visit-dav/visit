/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                            avtGenericDatabase.h                           //
// ************************************************************************* //

#ifndef AVT_GENERIC_DATABASE_H
#define AVT_GENERIC_DATABASE_H

#include <database_exports.h>

#include <string>
#include <vector>

#include <avtDatasetDatabase.h>
#include <avtMaterial.h>
#include <avtSpecies.h>
#include <avtVariableCache.h>
#include <avtTransformManager.h>

#include <vectortypes.h>

struct    avtDatasetCollection;
class     avtMatSpeciesMetaData;
class     avtDomainBoundaries;
class     avtStreamingGhostGenerator;
class     avtStructuredDomainBoundaries;
class     avtStructuredDomainNesting;
class     avtFileFormatInterface;
class     avtMixedVariable;
class     avtSILRestrictionTraverser;
class     PickAttributes;
class     PickVarInfo;
class     vtkRectilinearGrid;
class     vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtGenericDatabase
//
//  Purpose:
//      A database that handles a time sequence of data.  It does not care
//      which file format the data comes from because that is handled for it
//      by a file format interface.  The databases specific responsibilities
//      include unwinding SIL restrictions, performing those restrictions,
//      creating a SIL from a table of contents, caching, and combining
//      VTK datasets to make an avtDataset.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Mar 12 15:32:50 PST 2001
//    Added logic to create a SIL from an ordinary table of contents.
//
//    Kathleen Bonnell, Tue Mar 27 08:59:23 PST 2001 
//    Added logic to create a Mesh dataset. 
//
//    Kathleen Bonnell, Mon Apr  9 14:47:12 PDT 2001 
//    changed return type of GetOutput to avtDataTree_p. 
//
//    Kathleen Bonnell, Fri Jun 15 11:34:26 PDT 2001 
//    Added method AddOriginalCellsArray.
//
//    Hank Childs, Fri Aug 17 16:35:12 PDT 2001
//    Inherited from avtDatasetDatabase.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Make GetMaterialList return the material names in an argument.  
//
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001 
//    Added another vector<string> argument to MaterialSelect method. 
//
//    Hank Childs, Tue Oct 23 09:16:53 PDT 2001
//    Added support for reading multiple variables.
//
//    Kathleen Bonnell, Mon Nov 12 11:33:55 PST 2001 
//    Added Query and CreateOriginalZones.
//
//    Jeremy Meredith, Mon Dec 17 14:37:04 PST 2001
//    Added AddSpecies, GetSpeciesDataset, GetSpecies.
//
//    Hank Childs, Wed Dec 19 09:39:15 PST 2001
//    Additional support for species.
//
//    Jeremy Meredith, Thu Mar 14 17:44:19 PST 2002
//    Added support for internal surfaces during material selection.
//
//    Kathleen Bonnell, Tue Mar 26 15:23:11 PST 2002 
//    Added argument to AddOriginalCellsArray.
//
//    Kathleen Bonnell, Wed Mar 27 15:47:14 PST 2002  
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Wed Mar 27 15:47:14 PST 2002  
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Mon May 20 17:01:31 PDT 2002 
//    Change AddOriginalCellsArray parameter from 'int &' to 'const int'. 
//
//    Sean Ahern, Tue May 21 11:58:02 PDT 2002
//    Added a virtual method for freeing up resources.
//    
//    Hank Childs, Mon Jun 24 17:35:37 PDT 2002
//    Added support for groups.
//
//    Kathleen Bonnell, Fri Jun 28 09:32:26 PDT 2002
//    Added methods QueryScalars, QueryVectors, QueryMaterial. 
//    
//    Hank Childs, Thu Jul  4 13:21:20 PDT 2002
//    Add better support for mixed variables, as well as secondary variables
//    when reconstructing material interfaces.  Also pushed the struct
//    avtDatasetCollection into its own file.
//
//    Jeremy Meredith, Tue Aug 13 14:48:27 PDT 2002
//    Added extra information to the material selection routines.
//
//    Kathleen Bonnell, Thu Aug 15 17:48:38 PDT 2002 
//    Added an argument to AddGroups, so that collections beneath the
//    group will have the correct titles. 
//
//    Jeremy Meredith, Thu Oct 24 15:37:56 PDT 2002
//    Added smoothing option and clean zones only option for MIR.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Moved Query method to parent class.  Made QueryScalars/Vectors/Material
//    virtual methods. 
//
//    Kathleen Bonnell, Fri Dec  6 16:25:20 PST 2002 
//    Added QueryNodes. 
//    
//    Kathleen Bonnell, Fri Dec 27 14:09:40 PST 2002 
//    Added arguments to QueryNodes. 
//    
//    Kathleen Bonnell, Fri Apr 18 14:11:24 PDT 2003   
//    Added QueryMesh. 
//
//    Brad Whitlock, Wed May 14 09:16:44 PDT 2003
//    I added an optional int argument to SetDatabaseMetaData.
//
//    Jeremy Meredith, Thu Jun 12 09:05:38 PDT 2003
//    Added another flag to MaterialSelect.
//
//    Hank Childs, Wed Jun 18 09:20:16 PDT 2003
//    Added CreateOriginalNodes.
//
//    Kathleen Bonnell, Fri Jun 20 13:52:00 PDT 2003  
//    Added QueryZones, added parameter to other Query methods.
//
//    Hank Childs, Tue Jul 29 15:59:56 PDT 2003
//    Added AssociateBounds and ScaleMesh.
//
//    Kathleen Bonnell, Tue Sep  9 16:51:10 PDT 2003 
//    Changed PickVarInfo argument in QueryMesh to std::string.
//
//    Jeremy Meredith, Mon Sep 15 09:43:26 PDT 2003
//    Added a MIR algorithm flag to MaterialSelect and GetMIR.
//
//    Hank Childs, Mon Sep 22 07:52:34 PDT 2003
//    Added support for tensors.
//
//    Mark C. Miller, 30Sep03, Added support for time varying MD/SIL
//
//    Kathleen Bonnell, Tue Nov 18 14:07:13 PST 2003
//    Add bool and string vector args to QueryNodes, QueryZones, in support
//    of logical zone coords.
//
//    Kathleen Bonnell, Thu Nov 20 15:10:23 PST 2003 
//    Added QuerySpecies. 
//    
//    Kathleen Bonnell, Thu Nov 20 17:47:57 PST 2003 
//    Add 'FindElementForPoint'. 
//
//    Kathleen Bonnell, Wed Dec 17 14:58:31 PST 2003 
//    Updated arguments lists for QueryNodes and QueryZones so that multiple
//    types of coordinates could be retrieved. 
//
//    Kathleen Bonnell, Mon Dec 22 14:39:30 PST 2003 
//    Add 'GetDomainName'. 
//
//    Mark C. Miller, Tue Mar 16 14:40:19 PST 2004
//    Added timestep argument to PopulateIOInformation
//
//    Hank Childs, Fri Apr  9 09:02:57 PDT 2004
//    Calculate material indices directly from the material for that timestep.
//
//    Kathleen Bonnell, Tue May 25 16:16:25 PDT 2004 
//    Add QueryZoneCenter. 
//
//    Kathleen Bonnell, Wed Jun  9 12:44:48 PDT 2004 
//    Add bool arg to QueryMesh. 
//
//    Kathleen Bonnell, Thu Jun 10 18:15:11 PDT 2004 
//    Rename QueryZoneCenter to QueryCoords, added bool arg.
//
//    Mark C. Miller, Mon Aug  9 19:12:24 PDT 2004
//    Added methods to get global node and zone ids
//
//    Hank Childs, Fri Aug 13 15:57:29 PDT 2004
//    Beef up ghost data communication routines.
//
//    Hank Childs, Thu Sep 23 09:48:24 PDT 2004
//    Add more methods for global node and zone ids.
//
//    Kathleen Bonnell, Thu Sep 23 17:48:37 PDT 2004 
//    Added args to QueryZones and QueryNodes, to support ghost-element 
//    indication. 
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added vector of bools for data selections to ReadDataset
//
//    Kathleen Bonnell, Wed Dec 15 08:41:17 PST 2004 
//    Changed 'std::vector<std::string>' to 'stringVector', 'std::vector<int>'
//    to intVector, 'std::vector<bool>' to 'boolVector'.  Added method
//    'QueryGlobalIds'. 
//
//    Kathleen Bonnell, Wed Dec 15 17:32:29 PST 2004 
//    Added 'LocalIdForGlobal'.
//
//    Kathleen Bonnell, Thu Dec 16 17:11:19 PST 2004
//    Added another bool arg to QueryCoords. 
//
//    Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//    Added const char* arg to QueryCoords. 
//
//    Hank Childs, Sun Feb 27 11:20:39 PST 2005
//    Added argument to CanDoDynamicLoadBalancing.
//
//    Hank Childs, Sat Mar  5 19:28:52 PST 2005
//    Added argument to functions that typically do collective communication,
//    indicating whether or not they can safely do collective communication
//    (typically for error handling) or whether they should blow it off.
//
//    Hank Childs, Fri Mar 11 11:21:57 PST 2005
//    Fix memory leak with non-cachable variables.
//
//    Brad Whitlock, Sat Apr 2 00:31:24 PDT 2005
//    Added methods to get Label datasets.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool arg, forceReadAllCyclesTimes, to SetDatabaseMetaData
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added method SetCycleTimeInDatabaseMetaData
//
//    Hank Childs, Mon Jun 27 16:24:23 PDT 2005
//    Added argument to GetDataset.
//
//    Hank Childs, Tue Jul 19 14:52:08 PDT 2005
//    Added support for array variables.
//
//    Hank Childs, Wed Aug 17 09:10:35 PDT 2005
//    Add support for simplifying heavily mixed zones.
//
//    Jeremy Meredith, Thu Aug 18 16:39:54 PDT 2005
//    Added a new isovolume MIR algorithm, and a VF cutoff for it.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Replaced type conversion args on many Get functions to data spec 
//
//    Jeremy Meredith, Mon Aug 28 16:39:17 EDT 2006
//    Added support for enumerated scalars.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added transform manager. Added data spec to GetMaterial method.
//
//    Cyrus Harrison, Tue Jun 19 11:05:49 PDT 2007
//    Added issuedOriginalConnectivityWarning to support once per session
//    matvf/specmf warning about ghost zones.
//
//    Kathleen Bonnell, Thu Jun 21 17:00:26 PDT 2007 
//    Added method CreateAMRIndices.
//
//    Hank Childs, Thu Jul 26 16:32:50 PDT 2007
//    Added method CreateSimplifiedNestingRepresentation.
//
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Added treatAllDBsAsTimeVarying to PopulateSIL
//
//    Cyrus Harrison, Fri Sep 14 13:59:30 PDT 2007
//    Added floating point format argument to QueryNodes and QueryZones
//
//    Hank Childs, Sun Oct 28 21:09:44 PST 2007
//    Added Boolean argument to GetDomainBoundaryInformation.
//
//    John C. Anderson, Thu Jan 15 10:20:20 2009
//    Added annealing time to GetMIR and MaterialSelect.
//
//    Hank Childs, Sun Feb 10 19:43:59 MST 2008
//    Add support for streaming ghost generation.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Jeremy Meredith, Fri Feb 13 12:04:16 EST 2009
//    Added MIR iteration capability.
//
// ****************************************************************************

class DATABASE_API avtGenericDatabase : public avtDatasetDatabase
{
  public:
                               avtGenericDatabase(avtFileFormatInterface *);
    virtual                   ~avtGenericDatabase();

    virtual void               GetAuxiliaryData(avtDataRequest_p,
                                                VoidRefList &,
                                                const char *type, void *args);
    virtual avtDataTree_p      GetOutput(avtDataRequest_p,
                                         avtSourceFromDatabase *);

    virtual void               FreeUpResources(void);
    virtual int                NumStagesForFetch(avtDataRequest_p);

    virtual const char        *GetFilename(int);

    virtual bool               HasInvariantMetaData(void) const;
    virtual bool               HasInvariantSIL(void) const;
    virtual bool               CanDoStreaming(avtDataRequest_p);

    virtual void               ActivateTimestep(int stateIndex);

    virtual bool               FindElementForPoint(const char *, const int, 
                                    const int, const char *, double[3], int &);
    virtual void               GetDomainName(const std::string &, const int ts,
                                    const int dom, std::string &);

  protected:
    avtFileFormatInterface    *Interface;
    avtVariableCache           cache;
    vtkDataArray              *nonCachableVariableReference;
    vtkDataSet                *nonCachableMeshReference;

    avtTransformManager       *xformManager;

    int                        lastTimestep;

    static  bool               issuedOriginalConnectivityWarning;

    virtual void               PopulateSIL(avtSIL *, int=0,
                                   bool treatAllDBsAsTimeVarying = false);
    virtual void               PopulateIOInformation(int ts, avtIOInformation &);
    virtual void               SetCycleTimeInDatabaseMetaData(avtDatabaseMetaData *md, int timeState);
    virtual void               SetDatabaseMetaData(avtDatabaseMetaData *md,
                                   int timeState = 0,
                                   bool forceReadAllCyclesTimes = false);

    vtkDataSet                *GetDataset(const char *, int, int, const char *,
                                          const std::vector<CharStrRef> &,
                                          avtDataRequest_p, 
                                          avtSourceFromDatabase *);
    vtkDataSet                *GetScalarVarDataset(const char *, int, int,
                                                   const char *,
                                                   const avtDataRequest_p);
    vtkDataSet                *GetMeshDataset(const char *, int, int,
                                              const char *, const avtDataRequest_p); 
    vtkDataSet                *GetVectorVarDataset(const char *, int, int,
                                                   const char *,
                                                   const avtDataRequest_p);
    vtkDataSet                *GetSymmetricTensorVarDataset(const char *, int,
                                                            int, const char *,
                                                            const avtDataRequest_p);
    vtkDataSet                *GetTensorVarDataset(const char *, int, int,
                                                   const char *,
                                                   const avtDataRequest_p);
    vtkDataSet                *GetArrayVarDataset(const char *, int, int,
                                                   const char *,
                                                   const avtDataRequest_p);
    vtkDataSet                *GetMaterialDataset(const char *, int, int,
                                                  const char *,
                                                  const avtDataRequest_p);
    vtkDataSet                *GetSpeciesDataset(const char *, int, int,
                                                 const char *,
                                                 const avtDataRequest_p);
    vtkDataSet                *GetLabelVarDataset(const char *, int, int,
                                                  const char *,
                                                  const avtDataRequest_p);
    vtkDataArray              *GetScalarVariable(const char *, int, int,
                                                 const char *,
                                                 const avtDataRequest_p);
    vtkDataArray              *GetVectorVariable(const char *, int, int,
                                                 const char *,
                                                 const avtDataRequest_p);
    vtkDataArray              *GetSymmetricTensorVariable(const char *,int,int,
                                                          const char *,
                                                          const avtDataRequest_p);
    vtkDataArray              *GetTensorVariable(const char *, int, int,
                                                 const char *,
                                                 const avtDataRequest_p);
    vtkDataArray              *GetArrayVariable(const char *, int, int,
                                                 const char *,
                                                 const avtDataRequest_p);
    vtkDataArray              *GetSpeciesVariable(const char *, int, int,
                                                  const char *, int);
    vtkDataArray              *GetLabelVariable(const char *, int, int,
                                                const char *);
    vtkDataArray              *GetGlobalNodeIds(int, const char *, int);
    vtkDataArray              *GetGlobalZoneIds(int, const char *, int);
    void                       AddSecondaryVariables(vtkDataSet *, int, int,
                                                     const char *,
                                              const std::vector<CharStrRef> &,
                                              const avtDataRequest_p);
    vtkDataSet                *GetMesh(const char *, int, int, const char *,
                                       avtDataRequest_p);

    void                       AddOriginalCellsArray(vtkDataSet *, const int);
    void                       AddOriginalNodesArray(vtkDataSet *, const int);

    bool                       PrepareMaterialSelect(int, bool,
                                                  avtSILRestrictionTraverser &, 
                                                  stringVector &);
    avtDataTree_p              MaterialSelect(vtkDataSet *, avtMaterial *, 
                                              std::vector<avtMixedVariable *>,
                                              int, const char *, int,
                                              stringVector &, 
                                              stringVector &, 
                                              bool, bool, bool, bool, bool,
                                              bool, bool, int, int,
                                              int, float, float, int, bool, 
                                              bool&, bool&, bool);
    void_ref_ptr               GetMIR(int, const char *, int, vtkDataSet*,
                                      avtMaterial *, int, bool, bool, bool,
                                      bool, int, int, int, float,
                                      float, int, bool, bool&, bool&,bool, 
                                      avtMaterial *&);
    avtMaterial               *GetMaterial(int, const char *, int, const avtDataRequest_p = 0);
    avtSpecies                *GetSpecies(int, const char *, int);
    void                       GetMaterialIndices(avtMaterial *,
                                                  stringVector &, 
                                                  intVector &);

    void                       ReadDataset(avtDatasetCollection &, 
                                  intVector &, avtDataRequest_p &,
                                  avtSourceFromDatabase *,
                                  boolVector &);

    avtDomainBoundaries       *GetDomainBoundaryInformation(
                                      avtDatasetCollection &, intVector &, 
                                      avtDataRequest_p,
                                      bool confirmInputMeshHasRightSize = true);
    avtStreamingGhostGenerator *GetStreamingGhostGenerator(void);
    bool                       CommunicateGhosts(avtGhostDataType,
                                    avtDatasetCollection &, intVector &,
                                    avtDataRequest_p &,
                                    avtSourceFromDatabase *, intVector &,bool);
    bool                       CommunicateGhostZonesFromDomainBoundaries(
                                    avtDomainBoundaries *,
                                    avtDatasetCollection &, intVector &, 
                                    avtDataRequest_p &,
                                    avtSourceFromDatabase *);
    bool                       CommunicateGhostZonesFromDomainBoundariesFromFile(
                                    avtDatasetCollection &, intVector &, 
                                    avtDataRequest_p &,
                                    avtSourceFromDatabase *);
    bool                       CommunicateGhostNodesFromDomainBoundariesFromFile(
                                    avtDatasetCollection &, intVector &, 
                                    avtDataRequest_p &,
                                    avtSourceFromDatabase *, intVector &);
    bool                       CommunicateGhostZonesFromGlobalNodeIds(
                                    avtDatasetCollection &, intVector &, 
                                    avtDataRequest_p &,
                                    avtSourceFromDatabase *);
    bool                       CommunicateGhostNodesFromGlobalNodeIds(
                                    avtDatasetCollection &, intVector &, 
                                    avtDataRequest_p &,
                                    avtSourceFromDatabase *);
    bool                       CommunicateGhostZonesWhileStreaming(
                                    avtDatasetCollection &, intVector &, 
                                    avtDataRequest_p &,
                                    avtSourceFromDatabase *);

    bool                       ApplyGhostForDomainNesting(avtDatasetCollection &,
                                  intVector &, intVector &, 
                                  avtDataRequest_p &, bool);
    void                       MaterialSelect(avtDatasetCollection &,
                                 intVector &, avtDataRequest_p &,
                                 avtSourceFromDatabase *, bool);
    void                       SpeciesSelect(avtDatasetCollection &,
                                 intVector &, boolVector &, 
                                 avtDataRequest_p &,
                                 avtSourceFromDatabase *);
    void                       EnumScalarSelect(avtDatasetCollection &,
                                                const boolVector &,
                                                const avtDatabaseMetaData*,
                                                const string &);
    void                       CreateOriginalZones(avtDatasetCollection &,
                                                   intVector &, 
                                                   avtSourceFromDatabase *);
    void                       CreateOriginalNodes(avtDatasetCollection &,
                                                   intVector &, 
                                                   avtSourceFromDatabase *);
    void                       CreateGlobalZones(avtDatasetCollection &,
                                                   intVector &, 
                                                   avtSourceFromDatabase *,
                                                   avtDataRequest_p &);
    void                       CreateGlobalNodes(avtDatasetCollection &,
                                                   intVector &, 
                                                   avtSourceFromDatabase *,
                                                   avtDataRequest_p &);
    void                       CreateStructuredIndices(avtDatasetCollection &,
                                                       avtSourceFromDatabase*);
    bool                       CreateSimplifiedNestingRepresentation(
                                                   avtDatasetCollection &,
                                                   intVector &, intVector &,
                                                   avtSourceFromDatabase*,
                                                   avtDataRequest_p &);
    vtkUnstructuredGrid       *CreateSimplifiedNestingRepresentation(
                                              vtkRectilinearGrid *, int,
                                              intVector &,
                                              avtStructuredDomainNesting *,
                                              avtStructuredDomainBoundaries *,
                                              avtGhostDataType);
    void                       CreateAMRIndices(avtDatasetCollection &,
                                                intVector &,
                                                avtDataRequest_p &, 
                                                avtSourceFromDatabase*,
                                                int level);
    void                       UpdateInternalState(int);

    virtual bool               QueryScalars(const std::string &, const int, 
                                            const int , const int ,
                                            const intVector &, 
                                            PickVarInfo &, const bool);
    virtual bool               QueryVectors(const std::string &, const int, 
                                            const int, const int,
                                            const intVector &, 
                                            PickVarInfo &, const bool);
    virtual bool               QueryTensors(const std::string &, const int, 
                                            const int, const int,
                                            const intVector &,
                                            PickVarInfo &, const bool);
    virtual bool               QuerySymmetricTensors(const std::string &,
                                            const int, const int, const int,
                                            const intVector &, 
                                            PickVarInfo &, const bool);
    virtual bool               QueryLabels(const std::string &, const int, 
                                            const int , const int ,
                                            const intVector &, 
                                            PickVarInfo &, const bool);
    virtual bool               QueryArrays(const std::string &, const int, 
                                            const int , const int ,
                                            const intVector &, 
                                            PickVarInfo &, const bool);
    virtual bool               QueryMaterial(const std::string &, const int, 
                                            const int , const int,
                                            const intVector &, 
                                            PickVarInfo &, const bool);
    virtual bool               QuerySpecies(const std::string &, const int, 
                                            const int , const int ,
                                            const intVector &, 
                                            PickVarInfo &, const bool);
    virtual bool               QueryNodes(const std::string &, const int, 
                                          const std::string &, const int, 
                                          bool &, const int, 
                                          intVector &, intVector &, 
                                          const bool, double [3], 
                                          const int, const bool, const bool,
                                          const bool, stringVector &,
                                          stringVector &, stringVector &,
                                          const bool, const bool,
                                          stringVector &, stringVector &);

    virtual bool               QueryMesh(const std::string &, const int, const int, 
                                         std::string &, const bool);

    virtual bool               QueryZones(const std::string&, const int, 
                                          const std::string &, int &,
                                          bool &, const int, intVector &, 
                                          intVector &, const bool,
                                          double [3], const int, const bool, 
                                          const bool, const bool, 
                                          stringVector &, stringVector &,
                                          stringVector &, const bool, const bool, 
                                          stringVector &, stringVector &); 
    virtual bool               QueryCoords(const std::string &, const int,
                                           const int, const int, double[3],
                                           const bool, const bool, const char *mn = NULL);
    virtual void               QueryGlobalIds(const int, const std::string &,
                                        const int, const bool, const int, 
                                        const intVector &, int &, intVector &);

    virtual int                LocalIdForGlobal(const int, const std::string &,
                                        const int, const bool, const int );

    void                       AssociateBounds(vtkDataSet *);
    void                       ScaleMesh(vtkDataSet *);

    void                       ManageMemoryForNonCachableVar(vtkDataArray *);
    void                       ManageMemoryForNonCachableMesh(vtkDataSet *);
};


#endif


