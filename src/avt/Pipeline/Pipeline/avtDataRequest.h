/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                                avtDataRequest.h                           //
// ************************************************************************* //

#ifndef AVT_DATA_REQUEST_H
#define AVT_DATA_REQUEST_H

#include <pipeline_exports.h>


#include <vector>
#include <map>

#include <array_ref_ptr.h>
#include <ref_ptr.h>

#include <avtSILRestriction.h>
#include <avtGhostData.h>

#include <avtDataSelection.h>

class PIPELINE_API avtSILSpecification
{
  public:
    bool                  useRestriction;
    avtSILRestriction_p   silr;
    int                   dataChunk;

    void                  GetDomainList(std::vector<int> &);
    bool                  UsesAllData(void);
    bool                  UsesAllDomains(void);
    bool                  EmptySpecification(void);
    bool                  operator==(const avtSILSpecification &);
};


class  avtWebpage;
class  avtDataRequest;
typedef ref_ptr<avtDataRequest> avtDataRequest_p;


// ****************************************************************************
//  Class: avtDataRequest
//
//  Purpose:
//      This is a specification of what the database should output.  That
//      includes the variable, timestep, and how the SIL (mesh) is restricted.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jun  5 14:00:24 PDT 2001
//    Allow for data to be specified as a data chunk as well.
//
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001
//    Added member 'needDomainLabels' and access methods.
//
//    Hank Childs, Tue Oct 23 08:58:30 PDT 2001
//    Allow for secondary variables.
//
//    Hank Childs, Wed Nov 28 13:14:31 PST 2001
//    Add a hook to prevent processing ghost zones.
//
//    Kathleen Bonnell, Wed Dec 12 10:50:01 PST 2001
//    Added hook to retrieve and turn on/off 'needZones' flag.
//
//    Jeremy Meredith, Thu Mar 14 17:28:40 PST 2002
//    Added needInternalSurfaces.
//
//    Hank Childs, Fri Jun  7 16:39:34 PDT 2002
//    Added support routines for secondary variables.
//
//    Kathleen Bonnell, Wed Sep  4 14:43:43 PDT 2002
//    Removed 'needDomainlabels' and related methods.
//
//    Hank Childs, Mon Sep 30 17:30:02 PDT 2002
//    Add needStructuredIndices.
//
//    Jeremy Meredith, Thu Oct 24 11:34:20 PDT 2002
//    Added options for forced material interface reconstruction for mixed
//    variables, for material interface smoothing, and for clean zones only.
//
//    Sean Ahern, Wed Jan  8 17:16:49 PST 2003
//    Added the ability to remove all secondary variables in one swell foop.
//
//    Jeremy Meredith, Thu Jun 12 08:47:03 PDT 2003
//    Added option for boundary surfaces.
//
//    Hank Childs, Tue Aug 12 17:27:32 PDT 2003
//    Added explicit flag for forcing material interface reconstruction.
//
//    Jeremy Meredith, Fri Sep  5 15:14:22 PDT 2003
//    Added flag to use new MIR algorithm.
//
//    Hank Childs, Thu Sep 25 08:28:28 PDT 2003
//    Allow the setting of a "db" variable.
//
//    Kathleen Bonnell, Thu Apr 22 15:15:57 PDT 2004
//    Added method 'VariablesAreTheSame'.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004
//    Added methods Set/GetMayRequireNodes.
//
//    Kathleen Bonnell, Mon Jun 28 08:05:38 PDT 2004
//    Added SetTimestep method.
//
//    Jeremy Meredith, Fri Jul  9 16:49:24 PDT 2004
//    Added GetSecondaryVariablesWithoutDuplicates.
//
//    Hank Childs, Tue Aug 10 13:47:48 PDT 2004
//    Added maintainOriginalConnectivity.
//
//    Hank Childs, Thu Sep 23 09:23:01 PDT 2004
//    Added support for global ids.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added support for data selections
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added methods to set/get admissibleDataTypes
//    Added methods to set/get needNativePrecision
//
//    Hank Childs, Tue Aug 16 16:11:57 PDT 2005
//    Added methods to simplify heavily mixed zones.
//
//    Jeremy Meredith, Thu Aug 18 16:24:07 PDT 2005
//    Added an isovolume MIR algorithm and a VF flag for it.
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Remove "DBVariable" construct.  Also added OriginalVariable which is
//    needed when GeneralContracts are requested.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added members and methods for mesh discretization
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added flatness tolerance.
//
//    Mark C. Miller, Tue Dec  5 18:14:58 PST 2006
//    Moved implementation of SetDiscMode to .C file
//
//    Hank Childs, Fri Jun 15 12:58:49 PDT 2007
//    Added method DebugDump.
//
//    Kathleen Bonnell, Thu Jun 21 16:31:59 PDT 2007
//    Added method NeedAMRIndices and SetNeedAMRIndices.  The int represents
//    level requested.
//
//    Hank Childs, Wed Jul 25 11:22:11 PDT 2007
//    Add support for special representations of wireframe subset plots.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Brad Whitlock, Wed Jan 23 15:43:25 PST 2008
//    Added transformVectorsDuringProject.
//
//    Cyrus Harrison, Tue Feb 12 13:35:19 PST 2008
//    Added needPostGhostMaterialInfo.
//
//    John C. Anderson, Thu Jan 15 10:20:20 2009
//    Added annealing time for Discrete MIR.
//
//    Jeremy Meredith, Fri Feb 13 11:22:39 EST 2009
//    Added MIR iteration capability.
//
//    Hank Childs, Fri Sep  3 12:10:47 PDT 2010
//    Added Boolean for whether the velocity field must be continuous.
//
//    Brad Whitlock, Thu Sep  1 10:56:29 PDT 2011
//    Add selectionName.
//
//    Brad Whitlock, Wed Jan  4 16:42:43 PST 2012
//    Added flag for how to handle missing data.
//
//    Mark C. Miller, Fri Apr 13 10:41:35 PDT 2012
//    I const qualified a couple of methods preventing use of this class
//    in a const setting.
//
//    Burlen Loring, Wed Aug  5 12:05:27 PDT 2015
//    Added SetVariable method.
//
//    Alister Maguire, Mon Nov 27 14:16:21 PST 2017
//    Added forceConstructMaterialLabels to allow access to 
//    material labels without a material variable. 
//
// ****************************************************************************

class PIPELINE_API avtDataRequest
{
  public:
                                 avtDataRequest(const char *, int, avtSILRestriction_p);
                                 avtDataRequest(const char *, int, int);
                                 avtDataRequest(avtDataRequest_p, avtSILRestriction_p);
                                 avtDataRequest(avtDataRequest_p, int);
                                 avtDataRequest(avtDataRequest_p, const char *);
                                 avtDataRequest(avtDataRequest_p);
    virtual                     ~avtDataRequest();

    avtDataRequest        &operator=(const avtDataRequest &);
    bool                         operator==(const avtDataRequest &);

    int                          GetTimestep(void) const { return timestep; };
    void                         SetTimestep(int t)     { timestep = t; };
    void                         SetVariable(const char *v);
    const char                  *GetVariable(void) const { return variable; };
    avtSILSpecification         &GetSIL(void)          { return sil; };
    avtSILRestriction_p          GetRestriction(void);

    const char                  *GetOriginalVariable(void)
                                                       {return orig_variable;};
    void                         SetOriginalVariable(const char *);
    void                         AddSecondaryVariable(const char *);
    void                         RemoveSecondaryVariable(const char *);
    void                         RemoveAllSecondaryVariables()
                                               { secondaryVariables.clear(); };
    bool                         HasSecondaryVariable(const char *);
    const std::vector<CharStrRef> &
                                 GetSecondaryVariables(void)
                                       { return secondaryVariables; };
    std::vector<CharStrRef>      GetSecondaryVariablesWithoutDuplicates(void);

    bool                         MustDoMaterialInterfaceReconstruction(void)
                                     { return mustDoMIR; };
    bool                         ForceConstructMaterialLabels(void)
                                     { return forceConstructMaterialLabels; };
    void                         ForceMaterialInterfaceReconstructionOn(void)
                                     { mustDoMIR = true; };
    void                         ForceMaterialInterfaceReconstructionOff(void)
                                     { mustDoMIR = false; };
    void                         ForceMaterialLabelsConstructionOn(void)
                                     { forceConstructMaterialLabels = true; };
    void                         ForceMaterialLabelsConstructionOff(void)
                                     { forceConstructMaterialLabels = false; };

    bool                         MustRemoveFacesBeforeGhosts(void)
                                     { return mustRemoveFacesBeforeGhosts; };
    void                         ForceRemoveFacesBeforeGhostsOn(void)
                                     { mustRemoveFacesBeforeGhosts = true; };
    void                         ForceRemoveFacesBeforeGhostsOff(void)
                                     { mustRemoveFacesBeforeGhosts = false; };

    bool                         NeedInternalSurfaces(void)
                                     { return needInternalSurfaces; };
    void                         TurnInternalSurfacesOn(void)
                                     { needInternalSurfaces = true; };
    void                         TurnInternalSurfacesOff(void)
                                     { needInternalSurfaces = false; };

    //
    // These methods change the nature of the data produced by the source
    // (i.e the database).  So when this methods are called, it changes
    // the type of data that comes down the pipeline.
    //
    bool                         GetBoundarySurfaceRepresentation(void)
                                     { return getBoundarySurfaceRep; };
    void                         TurnBoundarySurfaceRepresentationOn(void)
                                     { getBoundarySurfaceRep = true; };
    void                         TurnBoundarySurfaceRepresentationOff(void)
                                     { getBoundarySurfaceRep = false; };
    bool                         GetSimplifiedNestingRepresentation(void)
                                     { return getSimplifiedNestingRep; };
    void                         TurnSimplifiedNestingRepresentationOn(void)
                                     { getSimplifiedNestingRep = true; };
    void                         TurnSimplifiedNestingRepresentationOff(void)
                                     { getSimplifiedNestingRep = false; };

    bool                         NeedStructuredIndices(void)
                                     { return needStructuredIndices; };
    void                         SetNeedStructuredIndices(bool v)
                                     { needStructuredIndices = v; };

    bool                         VelocityFieldMustBeContinuous(void)
                                     { return velocityMustBeContinuous; };
    void                         SetVelocityFieldMustBeContinuous(bool v)
                                     { velocityMustBeContinuous = v; };

    int                          NeedAMRIndices(void)
                                     { return needAMRIndices; };
    void                         SetNeedAMRIndices(int v)
                                     { needAMRIndices = v; };

    bool                         NeedZoneNumbers(void)
                                     { return needZones; };
    void                         TurnZoneNumbersOn(void)
                                     { needZones = true; };
    void                         TurnZoneNumbersOff(void)
                                     { needZones = false; };

    bool                         NeedNodeNumbers(void)
                                     { return needNodes; };
    void                         TurnNodeNumbersOn(void)
                                     { needNodes = true; };
    void                         TurnNodeNumbersOff(void)
                                     { needNodes = false; };

    bool                         NeedGlobalZoneNumbers(void)
                                     { return needGlobalZones; };
    void                         TurnGlobalZoneNumbersOn(void)
                                     { needGlobalZones = true; };
    void                         TurnGlobalZoneNumbersOff(void)
                                     { needGlobalZones = false; };

    bool                         NeedGlobalNodeNumbers(void)
                                     { return needGlobalNodes; };
    void                         TurnGlobalNodeNumbersOn(void)
                                     { needGlobalNodes = true; };
    void                         TurnGlobalNodeNumbersOff(void)
                                     { needGlobalNodes = false; };

    bool                         MayRequireZones(void)
                                     { return mayRequireZones; };
    void                         SetMayRequireZones(bool val)
                                     { mayRequireZones = val; };

    bool                         MayRequireNodes(void)
                                     { return mayRequireNodes; };
    void                         SetMayRequireNodes(bool val)
                                     { mayRequireNodes = val; };

    bool                         MustMaintainOriginalConnectivity(void)
                                     { return maintainOriginalConnectivity; };
    void                         SetMaintainOriginalConnectivity(bool val)
                                     { maintainOriginalConnectivity = val; };

    void                         SetDesiredGhostDataType(avtGhostDataType t)
                                     { desiredGhostDataType = t; };
    avtGhostDataType             GetDesiredGhostDataType(void)
                                     { return desiredGhostDataType; };

    bool                         NeedValidFaceConnectivity()
                                     { return needValidFaceConnectivity; };
    void                         SetNeedValidFaceConnectivity(bool nvfc)
                                     { needValidFaceConnectivity = nvfc; };

    bool                         UsesAllDomains(void)
                                     { return usesAllDomains; };
    void                         SetUsesAllDomains(bool uad)
                                     { usesAllDomains = uad; };

    bool                         NeedMixedVariableReconstruction(void)
                                     { return needMixedVariableReconstruction;}
    void                         SetNeedMixedVariableReconstruction(bool mvr)
                                     { needMixedVariableReconstruction = mvr; }

    bool                         NeedSmoothMaterialInterfaces(void)
                                     { return needSmoothMaterialInterfaces;}
    void                         SetNeedSmoothMaterialInterfaces(bool smi)
                                     { needSmoothMaterialInterfaces = smi; }

    bool                         NeedCleanZonesOnly(void)
                                     { return needCleanZonesOnly;}
    void                         SetNeedCleanZonesOnly(bool czo)
                                     { needCleanZonesOnly = czo; }

    int                          MIRAlgorithm(void)
                                     { return mirAlgorithm;}
    void                         SetMIRAlgorithm(int ma)
                                     { mirAlgorithm = ma; }

    int                          MIRNumIterations(void)
                                     { return mirNumIterations;}
    void                         SetMIRNumIterations(int n)
                                     { mirNumIterations = n; }

    float                        MIRIterationDamping(void)
                                     { return mirIterationDamping;}
    void                         SetMIRIterationDamping(float d)
                                     { mirIterationDamping = d; }

    float                        IsovolumeMIRVF(void)
                                     { return isovolumeMIRVF;}
    void                         SetIsovolumeMIRVF(float vf)
                                     { isovolumeMIRVF = vf; }

    int                          AnnealingTime(void)
                                     { return annealingTime;}
    void                         SetAnnealingTime(int t)
                                     { annealingTime = t; }

    bool                         SimplifyHeavilyMixedZones(void)
                                     { return simplifyHeavilyMixedZones;}
    void                         SetSimplifyHeavilyMixedZones(bool shmz)
                                     { simplifyHeavilyMixedZones = shmz; }

    int                          MaxMaterialsPerZone(void)
                                     { return maxMatsPerZone;}
    void                         SetMaxMaterialsPerZone(int mmpz)
                                     { maxMatsPerZone = mmpz; }

    bool                         VariablesAreTheSame(const avtDataRequest &);

    int                          AddDataSelection(avtDataSelection *sel);
                                 // Maybe this method could also be called
                                 // "AddDataSelection", but I'm worried about
                                 // overloading/compile problems...
    int                          AddDataSelectionRefPtr(avtDataSelection_p sel);
    void                         RemoveAllDataSelections();
    const avtDataSelection_p     GetDataSelection(int id) const;
    const std::vector<avtDataSelection_p> GetAllDataSelections() const;
    std::vector<avtDataSelection_p> GetAllDataSelections();


    bool                         NeedNativePrecision(void) const
                                     { return needNativePrecision; }
    void                         SetNeedNativePrecision(bool nnp)
                                     { needNativePrecision = nnp; }

    void                         UpdateAdmissibleDataTypes(
                                     const std::vector<int> &admissibleTypes);
    void                         UpdateAdmissibleDataTypes(int dt1);
    void                         UpdateAdmissibleDataTypes(int dt1, int dt2);
    void                         UpdateAdmissibleDataTypes(int dt1, int dt2, int dt3);
    static std::vector<int>      AllAdmissibleDataTypes();

    bool                         IsAdmissibleDataType(int dataType) const;
    std::vector<int>             GetAdmissibleDataTypes() const;
    void                         InitAdmissibleDataTypes();

    void                         SetDiscTol(double tol) { discTol = tol; };
    double                       DiscTol() const { return discTol; };

    void                         SetFlatTol(double tol) { flatTol = tol; };
    double                       FlatTol() const { return flatTol; };

    void                         SetDiscMode(int mode);
    int                          DiscMode() const { return discMode; };

    void                         SetDiscBoundaryOnly(bool b)
                                     { discBoundaryOnly = b; };
    bool                         DiscBoundaryOnly() const
                                     { return discBoundaryOnly; };

    void                         SetPassNativeCSG(bool p)
                                     { passNativeCSG = p; };
    bool                         PassNativeCSG() const
                                     { return passNativeCSG; };

    bool                         TransformVectorsDuringProject() const
                                     { return transformVectorsDuringProject; }
    void                         SetTransformVectorsDuringProject(bool b)
                                     { transformVectorsDuringProject = b; }

    bool                         NeedPostGhostMaterialInfo() const
                                     { return needPostGhostMaterialInfo; }
    void                         SetNeedPostGhostMaterialInfo(bool b)
                                     { needPostGhostMaterialInfo = b; }

    void                         SetSelectionName(const std::string &s)
                                     { selectionName = s; }
    const std::string           &GetSelectionName() const
                                     { return selectionName; }

    typedef enum {
        MISSING_DATA_IGNORE,   // Ignore any missing data
        MISSING_DATA_REMOVE,   // Remove all missing data
        MISSING_DATA_IDENTIFY  // Identify missing data by adding avtMissingData
                               // array to the datasets so we can show which
                               // cells contain missing data.
    } MissingDataBehavior_t;
    void                         IgnoreMissingData()
                                     { missingDataBehavior = MISSING_DATA_IGNORE;}
    void                         RemoveMissingData()
                                     { missingDataBehavior = MISSING_DATA_REMOVE;}
    void                         IdentifyMissingData()
                                     { missingDataBehavior = MISSING_DATA_IDENTIFY;}
    MissingDataBehavior_t        MissingDataBehavior() const
                                     { return missingDataBehavior; }

    void                         DebugDump(avtWebpage *);

  protected:
    int                          timestep;
    char                        *variable;
    avtSILSpecification          sil;
    std::vector<CharStrRef>      secondaryVariables;

    // original_variable is the name of the variable that the pipeline is
    // plotting.  This variable may get removed from the data specification
    // because it has already been generated by an expression.  Regardless,
    // the information of what the original_variable is important, because
    // sometimes a "general" data specification is requested and that
    // specification should use the original variable.  (This happens when
    // a query asks the top of the pipeline -- a database source -- to
    // create a data specification.)
    char                        *orig_variable;

    bool                         needZones;
    bool                         needNodes;
    bool                         needGlobalZones;
    bool                         needGlobalNodes;
    bool                         mayRequireZones;
    bool                         mayRequireNodes;
    bool                         mustDoMIR;
    bool                         forceConstructMaterialLabels;
    bool                         mustRemoveFacesBeforeGhosts;
    bool                         needInternalSurfaces;
    bool                         getBoundarySurfaceRep;
    bool                         getSimplifiedNestingRep;
    bool                         needValidFaceConnectivity;
    bool                         needStructuredIndices;
    int                          needAMRIndices;
    bool                         needMixedVariableReconstruction;
    bool                         needSmoothMaterialInterfaces;
    bool                         needCleanZonesOnly;
    bool                         velocityMustBeContinuous;
    int                          mirAlgorithm;
    int                          mirNumIterations;
    float                        mirIterationDamping;
    float                        isovolumeMIRVF;
    int                          annealingTime;
    bool                         simplifyHeavilyMixedZones;
    int                          maxMatsPerZone;
    bool                         maintainOriginalConnectivity;
    bool                         needNativePrecision;
    avtGhostDataType             desiredGhostDataType;
    std::map<int,bool>           admissibleDataTypes;
    double                       discTol;
    double                       flatTol;
    int                          discMode;
    bool                         discBoundaryOnly;
    bool                         passNativeCSG;
    bool                         transformVectorsDuringProject;
    bool                         needPostGhostMaterialInfo;
    std::string                  selectionName;
    MissingDataBehavior_t        missingDataBehavior;

    //
    // If we are processing in parallel, this information may have been lost.
    // This is meant as a read-only bool for the database, so that it can make
    // decisions like how to communicate ghost zones.
    //
    bool                         usesAllDomains;

    //
    // A place for each filter to add information regarding its selection
    //
    std::vector<avtDataSelection_p>  selList;

  private:
    // This method is defined to prevent accidental use of a bitwise copy
    // implementation.  If you want to re-define it to do something
    // meaningful, that's fine.
                       avtDataRequest(const avtDataRequest &) {;};
};

#endif
