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
//    Jeremy Meredith, Thu Jun 12 08:47:03 PDT 2003
//    Added option for boundary surfaces.
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
    const char                  *GetVariable(void)     { return variable; };
    avtSILSpecification         &GetSIL(void)          { return sil; };
    avtSILRestriction_p          GetRestriction(void);

    void                         AddSecondaryVariable(const char *);
    void                         RemoveSecondaryVariable(const char *);
    bool                         HasSecondaryVariable(const char *);
    const std::vector<CharStrRef> &
                                 GetSecondaryVariables(void)
                                       { return secondaryVariables; };

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

    bool                         MayRequireZones(void) 
                                     { return mayRequireZones; };
    void                         SetMayRequireZones(bool val) 
                                     { mayRequireZones = val; };

    bool                         GhostZonesAreAppropriate(void)
                                     { return useGhostZones; };
    void                         NoGhostZones(void);

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

  protected:
    int                          timestep;
    char                        *variable;
    avtSILSpecification          sil;
    std::vector<CharStrRef>      secondaryVariables;

    bool                         needZones;
    bool                         mayRequireZones;
    bool                         useGhostZones;
    bool                         needInternalSurfaces;
    bool                         needBoundarySurfaces;
    bool                         needValidFaceConnectivity;
    bool                         needStructuredIndices;
    bool                         needMixedVariableReconstruction;
    bool                         needSmoothMaterialInterfaces;
    bool                         needCleanZonesOnly;

    //
    // If we are processing in parallel, this information may have been lost.
    // This is meant as a read-only bool for the database, so that it can make
    // decisions like how to communicate ghost zones.
    //
    bool                         usesAllDomains;
};


#endif


