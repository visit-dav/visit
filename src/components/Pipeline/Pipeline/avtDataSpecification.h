// ************************************************************************* //
//                             avtDataSpecification.h                        //
// ************************************************************************* //

#ifndef AVT_DATA_SPECIFICATION_H
#define AVT_DATA_SPECIFICATION_H
#include <pipeline_exports.h>


#include <vector>

#include <array_ref_ptr.h>
#include <ref_ptr.h>

#include <avtSILRestriction.h>
#include <avtGhostData.h>


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


class avtDataSpecification;
typedef ref_ptr<avtDataSpecification> avtDataSpecification_p;


// ****************************************************************************
//  Class: avtDataSpecification
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
// ****************************************************************************

class PIPELINE_API avtDataSpecification
{
  public:
                                 avtDataSpecification(const char *, int,
                                                      avtSILRestriction_p);
                                 avtDataSpecification(const char *, int, int);
                                 avtDataSpecification(avtDataSpecification_p,
                                                      avtSILRestriction_p);
                                 avtDataSpecification(avtDataSpecification_p,
                                                      int);
                                 avtDataSpecification(avtDataSpecification_p,
                                                      const char *);
                                 avtDataSpecification(avtDataSpecification_p);
    virtual                     ~avtDataSpecification();

    avtDataSpecification        &operator=(const avtDataSpecification &);
    bool                         operator==(const avtDataSpecification &);

    int                          GetTimestep(void)     { return timestep; };
    void                         SetTimestep(int t)     { timestep = t; };
    const char                  *GetVariable(void)     { return variable; };
    avtSILSpecification         &GetSIL(void)          { return sil; };
    avtSILRestriction_p          GetRestriction(void);

    const char                  *GetDBVariable(void) {return db_variable;};
    void                         SetDBVariable(const char *);
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
    void                         ForceMaterialInterfaceReconstructionOn(void) 
                                     { mustDoMIR = true; };
    void                         ForceMaterialInterfaceReconstructionOff(void) 
                                     { mustDoMIR = false; };

    bool                         NeedInternalSurfaces(void) 
                                     { return needInternalSurfaces; };
    void                         TurnInternalSurfacesOn(void) 
                                     { needInternalSurfaces = true; };
    void                         TurnInternalSurfacesOff(void) 
                                     { needInternalSurfaces = false; };

    bool                         NeedBoundarySurfaces(void) 
                                     { return needBoundarySurfaces; };
    void                         TurnBoundarySurfacesOn(void) 
                                     { needBoundarySurfaces = true; };
    void                         TurnBoundarySurfacesOff(void) 
                                     { needBoundarySurfaces = false; };

    bool                         NeedStructuredIndices(void)
                                     { return needStructuredIndices; };
    void                         SetNeedStructuredIndices(bool v)
                                     { needStructuredIndices = v; };

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

    bool                         UseNewMIRAlgorithm(void)
                                     { return useNewMIRAlgorithm;}
    void                         SetUseNewMIRAlgorithm(bool unma)
                                     { useNewMIRAlgorithm = unma; }


    bool                         VariablesAreTheSame(const avtDataSpecification &);

  protected:
    int                          timestep;
    char                        *variable;
    avtSILSpecification          sil;
    std::vector<CharStrRef>      secondaryVariables;

    // db_variable is the name of a variable that exists in the database.
    // variable is the active variable that will be drawn.  Typically, 
    // db_variable and variable are the same.  But they differ when variable
    // is an expression, for example.
    // Note: if you set up a db_variable, it must still be added as a
    // secondary variable.
    char                        *db_variable;

    bool                         needZones;
    bool                         needNodes;
    bool                         needGlobalZones;
    bool                         needGlobalNodes;
    bool                         mayRequireZones;
    bool                         mayRequireNodes;
    bool                         mustDoMIR;
    bool                         needInternalSurfaces;
    bool                         needBoundarySurfaces;
    bool                         needValidFaceConnectivity;
    bool                         needStructuredIndices;
    bool                         needMixedVariableReconstruction;
    bool                         needSmoothMaterialInterfaces;
    bool                         needCleanZonesOnly;
    bool                         useNewMIRAlgorithm;
    bool                         maintainOriginalConnectivity;
    avtGhostDataType             desiredGhostDataType;

    //
    // If we are processing in parallel, this information may have been lost.
    // This is meant as a read-only bool for the database, so that it can make
    // decisions like how to communicate ghost zones.
    //
    bool                         usesAllDomains;
};


#endif


