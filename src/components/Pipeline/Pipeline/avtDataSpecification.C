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
//                             avtDataSpecification.C                        //
// ************************************************************************* //

#include <avtDataSpecification.h>

#include <avtSILRestrictionTraverser.h>

#include <ImproperUseException.h>

#include <vtkSystemIncludes.h>

#include <vector>
#include <map>

using     std::vector;
using     std::map;


// ****************************************************************************
//  Method: avtDataSpecification constructor
//
//  Arguments:
//      var          The variable for this dataset.
//      ts           The timestep for this dataset.
//      s            The SIL restriction.
//
//  Programmer:  Hank Childs
//  Creation:    May 19, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001
//    Initialize needDomainLabels.
//
//    Hank Childs, Wed Nov 28 13:23:17 PST 2001
//    Initialize useGhostZones.
//
//    Jeremy Meredith, Thu Mar 14 17:30:11 PST 2002
//    Initialize needInternalSurfaces.
//
//    Hank Childs, Tue Jun 18 16:49:01 PDT 2002
//    Initialize mayRequireZones.
//
//    Jeremy Meredith, Tue Aug 13 12:48:12 PDT 2002
//    Initialize needValidFaceConnectivity.
//
//    Kathleen Bonnell, Wed Sep  4 14:43:43 PDT 2002  
//    Removed needDomainLabels. 
//
//    Hank Childs, Mon Sep 30 17:31:59 PDT 2002
//    Initialize needStructuredIndices.
//
//    Hank Childs, Wed Oct  9 10:08:04 PDT 2002
//    Initialize usesAllDomains.
//
//    Jeremy Meredith, Thu Oct 24 16:15:11 PDT 2002
//    Added material options.
//
//    Jeremy Meredith, Tue Jun 10 13:21:15 PDT 2003
//    Added needBoundarySurfaces.
//
//    Hank Childs, Wed Jun 18 09:20:16 PDT 2003
//    Added needNodes.
//
//    Hank Childs, Tue Aug 12 17:27:32 PDT 2003
//    Added mustDoMIR.
//
//    Jeremy Meredith, Mon Sep 15 17:12:16 PDT 2003
//    Added a flag for the material interface algorithm to use.
//
//    Hank Childs, Thu Sep 25 08:28:28 PDT 2003
//    Set the db variable.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Initialize mayRequireNodes. 
//
//    Hank Childs, Tue Aug 10 14:51:34 PDT 2004
//    Remove useGhostZones, add maintainOriginalConnectivity and
//    desiredGhostDataType.
//
//    Hank Childs, Thu Sep 23 09:23:01 PDT 2004
//    Added needGlobalZones and needGlobalNodes.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added admissibleDataTypes and needNativePrecision
//
//    Hank Childs, Tue Aug 16 16:17:03 PDT 2005
//    Add support for simplifying heavily mixed zones.
//
//    Jeremy Meredith, Thu Aug 18 17:54:51 PDT 2005
//    Added a new isovolume algorithm, with adjustable VF cutoff.
//
//    Jeremy Meredith, Fri Aug 19 17:20:41 PDT 2005
//    Set the default back to Tet temporarily.  This is only to make sure
//    the test suite still passes.
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Change db_variable to orig_variable.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added members for mesh discretization 
// ****************************************************************************

avtDataSpecification::avtDataSpecification(const char *var, int ts,
                                           avtSILRestriction_p s)
{
    mayRequireZones = false;
    mayRequireNodes = false;
    needZones = false;
    needNodes = false;
    needGlobalZones = false;
    needGlobalNodes = false;
    needInternalSurfaces = false;
    mustDoMIR = false;
    needBoundarySurfaces = false;
    needValidFaceConnectivity = false;
    needStructuredIndices = false;
    usesAllDomains = true;
    needMixedVariableReconstruction = false;
    needSmoothMaterialInterfaces = false;
    needCleanZonesOnly = false;
    mirAlgorithm = 0; // 0=Tet 1==Zoo 2=Isovolume
    isovolumeMIRVF = 0.5;
    simplifyHeavilyMixedZones = false;
    maxMatsPerZone = 3;
    desiredGhostDataType = NO_GHOST_DATA;
    maintainOriginalConnectivity = false;
    needNativePrecision = false;
    discTol = 0.01;
    discMode = 1; // adaptive
    discBoundaryOnly = false;
    passNativeCSG = false;

    InitAdmissibleDataTypes();

    timestep  = ts;

    sil.useRestriction = true;
    sil.silr = s;

    variable  = new char[strlen(var)+1];
    strcpy(variable, var);

    //
    // Assume the 'orig' variable is the input variable.  If this is not true,
    // it will be corrected later.
    //
    orig_variable = new char[strlen(var)+1];
    strcpy(orig_variable, var);
}


// ****************************************************************************
//  Method: avtDataSpecification constructor
//
//  Arguments:
//      var          The variable for this dataset.
//      ts           The timestep for this dataset.
//      ch           The index of the data chunk.
//
//  Programmer:  Hank Childs
//  Creation:    June 5, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001
//    Initialize needDomainLabels.
//
//    Jeremy Meredith, Thu Mar 14 17:30:11 PST 2002
//    Initialize needInternalSurfaces.
//
//    Hank Childs, Tue Jun 18 16:49:01 PDT 2002
//    Initialize mayRequireZones.
//
//    Jeremy Meredith, Tue Aug 13 12:48:42 PDT 2002
//    Initialize needValidFaceConnectivity.
//
//    Kathleen Bonnell, Wed Sep  4 14:43:43 PDT 2002  
//    Removed needDomainLabels. 
//
//    Hank Childs, Mon Sep 30 17:31:59 PDT 2002
//    Initialize needStructuredIndices.
//
//    Hank Childs, Wed Oct  9 10:08:04 PDT 2002
//    Initialize usesAllDomains.
//
//    Jeremy Meredith, Thu Oct 24 16:15:11 PDT 2002
//    Added material options.
//
//    Jeremy Meredith, Tue Jun 10 13:21:24 PDT 2003
//    Added needBoundarySurfaces.
//
//    Hank Childs, Wed Jun 18 09:20:16 PDT 2003
//    Added needNodes.
//
//    Hank Childs, Tue Aug 12 17:27:32 PDT 2003
//    Added mustDoMIR.
//
//    Jeremy Meredith, Mon Sep 15 17:12:16 PDT 2003
//    Added a flag for the material interface algorithm to use.
//
//    Hank Childs, Thu Sep 25 08:28:28 PDT 2003
//    Set the db variable.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Initialize mayRequireNodes. 
//
//    Hank Childs, Tue Aug 10 14:51:34 PDT 2004
//    Remove useGhostZones, add maintainOriginalConnectivity and
//    desiredGhostDataType.
//
//    Hank Childs, Thu Sep 23 09:23:01 PDT 2004
//    Added needGlobalZones and needGlobalNodes.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added admissibleDataTypes and needNativePrecision
//
//    Hank Childs, Tue Aug 16 16:17:03 PDT 2005
//    Add support for simplifying heavily mixed zones.
//
//    Jeremy Meredith, Thu Aug 18 17:54:51 PDT 2005
//    Added a new isovolume algorithm, with adjustable VF cutoff.
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Change db_variable to orig_variable.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added members for mesh discretization 
// ****************************************************************************

avtDataSpecification::avtDataSpecification(const char *var, int ts, int ch)
{
    mayRequireZones = false;
    mayRequireNodes = false;
    needZones = false;
    needNodes = false;
    needGlobalZones = false;
    needGlobalNodes = false;
    mustDoMIR = false;
    needInternalSurfaces = false;
    needBoundarySurfaces = false;
    needValidFaceConnectivity = false;
    needStructuredIndices = false;
    usesAllDomains = true;
    needMixedVariableReconstruction = false;
    needSmoothMaterialInterfaces = false;
    needCleanZonesOnly = false;
    mirAlgorithm = 1; // 0=Tet 1==Zoo 2=Isovolume
    isovolumeMIRVF = 0.5;
    simplifyHeavilyMixedZones = false;
    maxMatsPerZone = 3;
    desiredGhostDataType = NO_GHOST_DATA;
    maintainOriginalConnectivity = false;
    needNativePrecision = false;
    discTol = 0.01;
    discMode = 1; // adaptive
    discBoundaryOnly = false;
    passNativeCSG = false;

    InitAdmissibleDataTypes();

    timestep  = ts;

    sil.useRestriction = false;
    sil.dataChunk = ch;

    variable  = new char[strlen(var)+1];
    strcpy(variable, var);

    //
    // Assume the 'db' variable is the input variable.  If this is not true,
    // it will be corrected later.
    //
    orig_variable = new char[strlen(var)+1];
    strcpy(orig_variable, var);
}


// ****************************************************************************
//  Method: avtDataSpecification constructor
//
//  Arguments:
//      spec     An old specification that should be copied.
//      silr     The new SIL restriction that should be used in its place.
//
//  Programmer:  Hank Childs
//  Creation:    June 5, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Sep  6 15:39:36 PDT 2001
//    Initialized variable to NULL.
//
//    Hank Childs, Thu Sep 25 08:28:28 PDT 2003
//    Initialized db_variable to NULL.
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Change db_variable to orig_variable.
//
// ****************************************************************************

avtDataSpecification::avtDataSpecification(avtDataSpecification_p spec,
                                           avtSILRestriction_p silr)
{
    variable = NULL;
    orig_variable = NULL;
    (*this) = **spec;
    sil.useRestriction = true;
    sil.silr = silr;
}


// ****************************************************************************
//  Method: avtDataSpecification constructor
//
//  Arguments:
//      spec     An old specification that should be copied.
//      cdi      The index of the chunk of data.
//
//  Programmer:  Hank Childs
//  Creation:    June 5, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Sep 25 08:28:28 PDT 2003
//    Initialized db_variable to NULL.
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Change db_variable to orig_variable.
//
// ****************************************************************************

avtDataSpecification::avtDataSpecification(avtDataSpecification_p spec,
                                           int cdi)
{
    variable = NULL;
    orig_variable = NULL;
    (*this) = **spec;
    sil.useRestriction = false;
    sil.dataChunk = cdi;
}


// ****************************************************************************
//  Method: avtDataSpecification constructor
//
//  Arguments:
//      spec     An old specification that should be copied.
//      name     The name of the new variable.
//
//  Programmer:  Hank Childs
//  Creation:    June 6, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jan 14 14:23:22 PST 2002
//    Fix memory leak.
//
//    Hank Childs, Thu Sep 25 08:28:28 PDT 2003
//    Handle the db_variable as well.
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Change db_variable to orig_variable.
//
// ****************************************************************************

avtDataSpecification::avtDataSpecification(avtDataSpecification_p spec,
                                           const char *name)
{
    variable = NULL;
    orig_variable = NULL;

    (*this) = **spec;

    if (variable != NULL)
    {
        delete [] variable;
    }
    if (orig_variable != NULL)
    {
        delete [] orig_variable;
    }

    variable = new char[strlen(name)+1];
    strcpy(variable, name);
    orig_variable = new char[strlen(name)+1];
    strcpy(orig_variable, name);
}


// ****************************************************************************
//  Method: avtDataSpecification constructor
//
//  Arguments:
//      spec     An old specification that should be copied.
//
//  Programmer:  Hank Childs
//  Creation:    June 7, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Sep 25 08:28:28 PDT 2003
//    Initialized db_variable to NULL.
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Change db_variable to orig_variable.
//
// ****************************************************************************

avtDataSpecification::avtDataSpecification(avtDataSpecification_p spec)
{
    variable = NULL;
    orig_variable = NULL;
    (*this) = **spec;
}


// ****************************************************************************
//  Method: avtDataSpecification assignment operator
//
//  Arguments:
//      spec    The specification to copy.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Oct 19 16:21:42 PDT 2001
//    Copy over needDomainLabels.
//
//    Hank Childs, Tue Oct 23 08:58:30 PDT 2001
//    Copy over secondary variables as well.
//
//    Hank Childs, Tue Dec 18 15:29:49 PST 2001
//    Copy over need zones.
//
//    Jeremy Meredith, Thu Mar 14 18:01:52 PST 2002
//    Copy needInternalSurfaces.
//
//    Hank Childs, Tue Jun 18 16:49:01 PDT 2002
//    Copy over mayRequireZones.
//
//    Jeremy Meredith, Tue Aug 13 12:48:57 PDT 2002
//    Copy needValidFaceConnectivity.
//
//    Kathleen Bonnell, Wed Sep  4 14:43:43 PDT 2002  
//    Removed needDomainLabels.
//
//    Hank Childs, Mon Sep 30 17:31:59 PDT 2002
//    Copy needStructuredIndices.
//
//    Hank Childs, Wed Oct  9 10:08:04 PDT 2002
//    Copy usesAllDomains.
//
//    Jeremy Meredith, Thu Oct 24 16:15:11 PDT 2002
//    Added material options.
//
//    Jeremy Meredith, Tue Jun 10 13:21:32 PDT 2003
//    Added needBoundarySurfaces.
//
//    Hank Childs, Tue Aug 12 17:27:32 PDT 2003
//    Added mustDoMIR.
//
//    Jeremy Meredith, Mon Sep 15 17:12:16 PDT 2003
//    Added a flag for the material interface algorithm to use.
//
//    Hank Childs, Thu Sep 25 08:28:28 PDT 2003
//    Handle db_variable.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Added mayRequireNodes. 
//
//    Hank Childs, Tue Aug 10 14:51:34 PDT 2004
//    Remove useGhostZones, add maintainOriginalConnectivity and
//    desiredGhostDataType.
//
//    Hank Childs, Thu Sep 23 09:23:01 PDT 2004
//    Added needGlobalZones and needGlobalNodes.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added data selection list
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added admissibleDataTypes and needNativePrecision
//
//    Hank Childs, Tue Aug 16 16:17:03 PDT 2005
//    Add support for simplifying heavily mixed zones.
//
//    Jeremy Meredith, Thu Aug 18 17:54:51 PDT 2005
//    Added a new isovolume algorithm, with adjustable VF cutoff.
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Change db_variable to orig_variable.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added members for mesh discretization 
// ****************************************************************************

avtDataSpecification &
avtDataSpecification::operator=(const avtDataSpecification &spec)
{
    if (variable != NULL)
    {
        delete [] variable;
    }
    if (orig_variable != NULL)
    {
        delete [] orig_variable;
    }

    timestep = spec.timestep;

    variable = new char[strlen(spec.variable)+1];
    strcpy(variable, spec.variable);

    orig_variable = new char[strlen(spec.orig_variable)+1];
    strcpy(orig_variable, spec.orig_variable);

    sil.useRestriction = spec.sil.useRestriction;
    if (sil.useRestriction)
    {
        sil.silr = spec.sil.silr;
    }
    else
    {
        sil.dataChunk = spec.sil.dataChunk;
    }

    mayRequireZones                 = spec.mayRequireZones;
    mayRequireNodes                 = spec.mayRequireNodes;
    mustDoMIR                       = spec.mustDoMIR;
    needZones                       = spec.needZones;
    needNodes                       = spec.needNodes;
    needGlobalZones                 = spec.needGlobalZones;
    needGlobalNodes                 = spec.needGlobalNodes;
    needInternalSurfaces            = spec.needInternalSurfaces;
    needBoundarySurfaces            = spec.needBoundarySurfaces;
    needValidFaceConnectivity       = spec.needValidFaceConnectivity;
    needStructuredIndices           = spec.needStructuredIndices;
    usesAllDomains                  = spec.usesAllDomains;
    needMixedVariableReconstruction = spec.needMixedVariableReconstruction;
    needSmoothMaterialInterfaces    = spec.needSmoothMaterialInterfaces;
    needCleanZonesOnly              = spec.needCleanZonesOnly;
    simplifyHeavilyMixedZones       = spec.simplifyHeavilyMixedZones;
    maxMatsPerZone                  = spec.maxMatsPerZone;
    mirAlgorithm                    = spec.mirAlgorithm;
    isovolumeMIRVF                  = spec.isovolumeMIRVF;
    desiredGhostDataType            = spec.desiredGhostDataType;
    maintainOriginalConnectivity    = spec.maintainOriginalConnectivity;
    needNativePrecision             = spec.needNativePrecision;
    admissibleDataTypes             = spec.admissibleDataTypes;
    discTol                         = spec.discTol;
    discMode                        = spec.discMode;
    discBoundaryOnly                = spec.discBoundaryOnly;
    passNativeCSG                   = spec.passNativeCSG;

    secondaryVariables = spec.secondaryVariables;

    selList = spec.selList;

    return *this;
}


// ****************************************************************************
//  Method: avtDataSpecification comparison operator
//
//  Arguments:
//      ds      The data specification to compare against.
//
//  Returns:    true if this data specification equals the argument, false
//              otherwise.
//
//  Programmer: Hank Childs
//  Creation:   July 26, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001
//    Compare needDomainLabels.
//
//    Hank Childs, Tue Oct 23 08:58:30 PDT 2001
//    Check to make sure the secondary variables are equal as well.
//
//    Hank Childs, Wed Nov 28 13:23:17 PST 2001
//    Check against useGhostZones.
//
//    Jeremy Meredith, Thu Mar 14 18:02:00 PST 2002
//    Check against needInternalSurfaces.
//
//    Hank Childs, Tue Jun 18 16:49:01 PDT 2002
//    Check against mayRequireZones.
//
//    Jeremy Meredith, Tue Aug 13 12:49:38 PDT 2002
//    Check needValidFaceConnectivity.
//
//    Kathleen Bonnell, Wed Sep  4 14:43:43 PDT 2002  
//    Removed needDomainLabels. 
//
//    Hank Childs, Mon Sep 30 17:31:59 PDT 2002
//    Compare needStructuredIndices.
//
//    Hank Childs, Wed Oct  9 10:08:04 PDT 2002
//    Compare usesAllDomains.
//
//    Jeremy Meredith, Thu Oct 24 16:15:11 PDT 2002
//    Compare material options.
//
//    Jeremy Meredith, Tue Jun 10 13:21:40 PDT 2003
//    Compare needBoundarySurfaces.
//
//    Hank Childs, Wed Jun 18 09:20:16 PDT 2003
//    Compare needNodes.
//
//    Hank Childs, Tue Aug 12 17:27:32 PDT 2003
//    Added mustDoMIR.
//
//    Jeremy Meredith, Mon Sep 15 17:12:16 PDT 2003
//    Added a flag for the material interface algorithm to use.
//
//    Hank Childs, Thu Sep 25 08:28:28 PDT 2003
//    Compare db_variable.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Added mayRequireNodes. 
//
//    Hank Childs, Tue Aug 10 14:51:34 PDT 2004
//    Remove useGhostZones, add maintainOriginalConnectivity and
//    desiredGhostDataType.
//
//    Hank Childs, Thu Sep 23 09:23:01 PDT 2004
//    Added needGlobalZones and needGlobalNodes.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added data selection list
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added admissibleDataTypes and needNativePrecision
//
//    Hank Childs, Tue Aug 16 16:17:03 PDT 2005
//    Add support for simplifying heavily mixed zones.
//
//    Jeremy Meredith, Thu Aug 18 17:54:51 PDT 2005
//    Added a new isovolume algorithm, with adjustable VF cutoff.
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Change db_variable to orig_variable.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added members for mesh discretization 
// ****************************************************************************

bool
avtDataSpecification::operator==(const avtDataSpecification &ds)
{
    if (timestep != ds.timestep)
    {
        return false;
    }

    //
    // Assumption here that we don't have NULL pointers.
    //
    if (strcmp(variable, ds.variable) != 0)
    {
        return false;
    }
    if (strcmp(orig_variable, ds.orig_variable) != 0)
    {
        return false;
    }

    if (!(sil == ds.sil))
    {
        return false;
    }

    if (needZones != ds.needZones)
    {
        return false;
    }

    if (needNodes != ds.needNodes)
    {
        return false;
    }

    if (needGlobalZones != ds.needGlobalZones)
    {
        return false;
    }

    if (needGlobalNodes != ds.needGlobalNodes)
    {
        return false;
    }

    if (mustDoMIR != ds.mustDoMIR)
    {
        return false;
    }

    if (mayRequireZones != ds.mayRequireZones)
    {
        return false;
    }

    if (mayRequireNodes != ds.mayRequireNodes)
    {
        return false;
    }

    if (needInternalSurfaces != ds.needInternalSurfaces)
    {
        return false;
    }

    if (needBoundarySurfaces != ds.needBoundarySurfaces)
    {
        return false;
    }

    if (desiredGhostDataType != ds.desiredGhostDataType)
    {
        return false;
    }

    if (maintainOriginalConnectivity != ds.maintainOriginalConnectivity)
    {
        return false;
    }

    if (needValidFaceConnectivity != ds.needValidFaceConnectivity)
    {
        return false;
    }

    if (needStructuredIndices != ds.needStructuredIndices)
    {
        return false;
    }

    if (usesAllDomains != ds.usesAllDomains)
    {
        return false;
    }

    if (needMixedVariableReconstruction != ds.needMixedVariableReconstruction)
    {
        return false;
    }

    if (needSmoothMaterialInterfaces != ds.needSmoothMaterialInterfaces)
    {
        return false;
    }

    if (needCleanZonesOnly != ds.needCleanZonesOnly)
    {
        return false;
    }

    if (mirAlgorithm != ds.mirAlgorithm)
    {
        return false;
    }

    if (isovolumeMIRVF != ds.isovolumeMIRVF)
    {
        return false;
    }

    if (simplifyHeavilyMixedZones != ds.simplifyHeavilyMixedZones)
    {
        return false;
    }

    if (maxMatsPerZone != ds.maxMatsPerZone)
    {
        return false;
    }

    if (needNativePrecision != ds.needNativePrecision)
    {
        return false;
    }

    if (secondaryVariables.size() != ds.secondaryVariables.size())
    {
        return false;
    }
    int i;
    for (i = 0 ; i < secondaryVariables.size() ; i++)
    {
        const char *my_str  = *(secondaryVariables[i]);
        const char *his_str = *(ds.secondaryVariables[i]);
        if (strcmp(my_str, his_str) != 0)
        {
            return false;
        }
    }

    if (selList.size() != ds.selList.size())
    {
        return false;
    }
    for (i = 0; i < selList.size(); i++)
    {
        if (*selList[i] != *(ds.selList[i]))
            return false;
    }

    if (admissibleDataTypes != ds.admissibleDataTypes)
        return false;

    if (discTol != ds.discTol)
        return false;
   
    if (discMode != ds.discMode)
        return false;

    if (discBoundaryOnly != ds.discBoundaryOnly)
        return false;

    if (passNativeCSG != ds.passNativeCSG)
        return false;

    return true;
}


// ****************************************************************************
//  Method: avtDataSpecification destructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Sep 25 08:28:28 PDT 2003
//    Destruct db_variable.
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Change db_variable to orig_variable.
//
// ****************************************************************************

avtDataSpecification::~avtDataSpecification()
{
    if (variable != NULL)
    {
        delete [] variable;
        variable = NULL;
    }
    if (orig_variable != NULL)
    {
        delete [] orig_variable;
        orig_variable = NULL;
    }
}


// ****************************************************************************
//  Method: avtDataSpecification::SetOriginalVariable
//
//  Purpose:
//      Sets the variable that is known to be good on the database.
//
//  Programmer: Hank Childs
//  Creation:   September 25, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Change db_variable to orig_variable.
//
// ****************************************************************************

void
avtDataSpecification::SetOriginalVariable(const char *v)
{
    if (orig_variable != NULL)
    {
        delete [] orig_variable;
        orig_variable = NULL;
    }
    if (v != NULL)
    {
        orig_variable = new char[strlen(v)+1];
        strcpy(orig_variable, v);
    }
}


// ****************************************************************************
//  Method: avtDataSpecification::GetRestriction
//
//  Purpose:
//      Gets the restriction for a SIL.  Performs some error checking to make
//      sure the SIL is valid.
//
//  Returns:    The SIL restriction.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
// ****************************************************************************

avtSILRestriction_p
avtDataSpecification::GetRestriction(void)
{
    if (!sil.useRestriction)
    {
        EXCEPTION0(ImproperUseException);
    }

    return sil.silr;
}


// ****************************************************************************
//  Method: avtDataSpecification::AddSecondaryVariable
//
//  Purpose:
//      Adds a secondary variable to an array.
//
//  Arguments:
//      var     The name of the variable that should be read in.
//
//  Programmer: Hank Childs
//  Creation:   October 23, 2001
//
// ****************************************************************************

void
avtDataSpecification::AddSecondaryVariable(const char *var)
{
    char *v2 = new char[strlen(var)+1];
    strcpy(v2, var);
    CharStrRef ref = v2;
    secondaryVariables.push_back(ref);

    //
    // We don't need to free v2 since it is now in the world of reference
    // pointers.
    //
}


// ****************************************************************************
//  Method: avtDataSpecification::HasSecondaryVariable
//
//  Purpose:
//      Determines if we have a secondary variable.
//
//  Arguments:
//      var     The name of the variable to compare.
//
//  Returns:    true if we have the variable, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
// ****************************************************************************

bool
avtDataSpecification::HasSecondaryVariable(const char *var)
{
    for (int i = 0 ; i < secondaryVariables.size() ; i++)
    {
        if (strcmp(var, *(secondaryVariables[i])) == 0)
        {
            return true;
        }
    }

    return false;
}


// ****************************************************************************
//  Method: avtDataSpecification::RemoveSecondaryVariable
//
//  Purpose:
//      Removes a secondary variable from our list.
//
//  Arguments:
//      var     The name of the variable to remove.
//
//  Programmer: Hank Childs
//  Creation:   June 7, 2002
//
// ****************************************************************************

void
avtDataSpecification::RemoveSecondaryVariable(const char *var)
{
    vector<CharStrRef> newList;
    for (int i = 0 ; i < secondaryVariables.size() ; i++)
    {
        if (strcmp(var, *(secondaryVariables[i])) != 0)
        {
            newList.push_back(secondaryVariables[i]);
        }
    }

    secondaryVariables = newList;
}


// ****************************************************************************
//  Method: avtDataSpecification::VariablesAreTheSame
//
//  Purpose:
//      This compares variables with the passed object. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 22, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Change db_variable to orig_variable.
//
// ****************************************************************************

bool
avtDataSpecification::VariablesAreTheSame(const avtDataSpecification &ds)
{
    //
    // Assumption here that we don't have NULL pointers.
    //
    if (strcmp(variable, ds.variable) != 0)
    {
        return false;
    }
    if (strcmp(orig_variable, ds.orig_variable) != 0)
    {
        return false;
    }
    if (secondaryVariables.size() != ds.secondaryVariables.size())
    {
        return false;
    }
    for (int i = 0 ; i < secondaryVariables.size() ; i++)
    {
        const char *my_str  = *(secondaryVariables[i]);
        const char *his_str = *(ds.secondaryVariables[i]);
        if (strcmp(my_str, his_str) != 0)
        {
            return false;
        }
    }
    return true;
}


// ****************************************************************************
//  Method:  avtDataSpecification::GetSecondaryVariablesWithoutDuplicates
//
//  Purpose:
//    Return the list of secondary variables, removing duplicates of the
//    primary variable and not allowing duplicates within the secondary
//    variables.
//
//  Arguments:
//    none.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  9, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Sep 23 10:10:12 PDT 2005
//    Fix typo in test.
//
// ****************************************************************************

vector<CharStrRef>
avtDataSpecification::GetSecondaryVariablesWithoutDuplicates(void)
{
    vector<CharStrRef> newList;
    for (int i = 0 ; i < secondaryVariables.size() ; i++)
    {
        bool duplicate = false;

        // don't allow duplicates of the primary variable
        if (variable && strcmp(variable, *(secondaryVariables[i])) == 0)
        {
            duplicate = true;
        }

        // don't allow duplicates of other secondary variables; just take
        // the first instance of it
        for (int j = 0 ; j < i && !duplicate; j++)
        {
            if (strcmp(*(secondaryVariables[i]),*(secondaryVariables[j])) == 0)
            {
                duplicate = true;
            }
        }

        // add it to the list if it wasn't a duplicate
        if (!duplicate)
        {
            newList.push_back(secondaryVariables[i]);
        }
    }

    return newList;
}



// ****************************************************************************
//  Method: avtDataSpecification::AddDataSelection
//
//  Purpose: Adds a data selection to the specification
//
//  Programmer: Mark C. Miller 
//  Creation:   September 28, 2004 
//
//  Modifications:
//    Brad Whitlock, Mon Nov 1 16:07:37 PST 2004
//    I added a return statement since the return type is int.
//
//    Hank Childs, Fri Mar 11 10:54:13 PST 2005
//    Return the index of the selection, since that is what most callers count
//    on.
//
// ****************************************************************************

int
avtDataSpecification::AddDataSelection(avtDataSelection *sel)
{
    selList.push_back(sel);
    return selList.size()-1;
}

// ****************************************************************************
//  Method: avtDataSpecification::RemoveAllDataSelections
//
//  Purpose: Removes all data selections from the specification 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 28, 2004 
//
// ****************************************************************************

void
avtDataSpecification::RemoveAllDataSelections()
{
    selList.clear();
}

// ****************************************************************************
//  Method: avtDataSpecification::GetDataSelection
//
//  Purpose: Gets data selection at the specified index 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 28, 2004 
//
// ****************************************************************************

const avtDataSelection_p
avtDataSpecification::GetDataSelection(int id) const
{
    if (id < 0 || id >= selList.size())
        return 0;
    return selList[id];
}

// ****************************************************************************
//  Method: avtDataSpecification::GetAllDataSelections
//
//  Purpose: Gets all data selections in the specification 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 28, 2004 
//
// ****************************************************************************

const std::vector<avtDataSelection_p>
avtDataSpecification::GetAllDataSelections() const
{
    return selList;
}


// ****************************************************************************
//  Method: avtDataSpecification::InitAdmissibleDataTypes
//
//  Purpose: Initialize admissible data types to all true
//
//  Programmer: Mark C. Miller 
//  Creation:   March 23, 2005 
//
// ****************************************************************************

void
avtDataSpecification::InitAdmissibleDataTypes()
{
    admissibleDataTypes.clear();
    admissibleDataTypes[VTK_BIT]            = true;
    admissibleDataTypes[VTK_CHAR]           = true;
    admissibleDataTypes[VTK_UNSIGNED_CHAR]  = true;
    admissibleDataTypes[VTK_SHORT]          = true;
    admissibleDataTypes[VTK_UNSIGNED_SHORT] = true;
    admissibleDataTypes[VTK_INT]            = true;
    admissibleDataTypes[VTK_UNSIGNED_INT]   = true;
    admissibleDataTypes[VTK_LONG]           = true;
    admissibleDataTypes[VTK_UNSIGNED_LONG]  = true;
    admissibleDataTypes[VTK_FLOAT]          = true;
    admissibleDataTypes[VTK_DOUBLE]         = true;
    admissibleDataTypes[VTK_ID_TYPE]        = true;
}

// ****************************************************************************
//  Method: avtDataSpecification::UpdateAdmissibleDataTypes
//
//  Purpose: Merges a set of admissible types into the current list of
//  admissible types
//
//  Programmer: Mark C. Miller 
//  Creation:   March 23, 2005 
//
//  Modifications:
//    Brad Whitlock, Tue May 10 15:03:30 PST 2005
//    Fixed for win32.
//
// ****************************************************************************

void
avtDataSpecification::UpdateAdmissibleDataTypes(vector<int> admissibleTypes)
{
    std::map<int,bool>::iterator it;
    for (it = admissibleDataTypes.begin();
         it != admissibleDataTypes.end(); it++)
    {
        bool isAnAdmissibleType = false;
        for (int i = 0; i < admissibleTypes.size(); i++)
        {
            if (admissibleTypes[i] == it->first)
            {
                isAnAdmissibleType = true;
                break;
            }
        }
        if (isAnAdmissibleType == false)
            it->second = false;
    }
}

// ****************************************************************************
//  Method: avtDataSpecification::IsAdmissibleDataType
//
//  Purpose: Return bool indicating if the given type is admissible 
//
//  Programmer: Mark C. Miller 
//  Creation:   March 23, 2005 
//
//  Modifications:
//    Brad Whitlock, Tue May 10 15:03:30 PST 2005
//    Fixed for win32.
//
// ****************************************************************************

bool
avtDataSpecification::IsAdmissibleDataType(int theType) const
{
    std::map<int,bool>::const_iterator fit =
        admissibleDataTypes.find(theType);
    if (fit != admissibleDataTypes.end())
        return fit->second;
    return false;
}

// ****************************************************************************
//  Method: avtDataSpecification::GetAdmissibleDataTypes
//
//  Purpose: Return vector of admissible data types 
//
//  Programmer: Mark C. Miller 
//  Creation:   March 23, 2005 
//
//  Modifications:
//    Brad Whitlock, Tue May 10 15:03:30 PST 2005
//    Fixed for win32.
//
// ****************************************************************************

vector<int>
avtDataSpecification::GetAdmissibleDataTypes() const
{
    vector<int> admissibleTypes;
    std::map<int,bool>::const_iterator it;
    for (it = admissibleDataTypes.begin();
         it != admissibleDataTypes.end(); it++)
    {
        if (it->second)
            admissibleTypes.push_back(it->first);
    }
    return admissibleTypes;
}

// ****************************************************************************
//  Method: avtSILSpecification::GetDomainList
//
//  Purpose:
//      Gets a domain list regardless of whether a SIL was specified, or if
//      a data chunk was specified.
//
//  Arguments:
//      list    A place to put the list of domains.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov 22 15:58:40 PST 2002
//    Use SIL restriction traverser.
//
// ****************************************************************************

void
avtSILSpecification::GetDomainList(vector<int> &list)
{
    list.clear();
    if (useRestriction)
    {
        avtSILRestrictionTraverser trav(silr);
        trav.GetDomainList(list);
    }
    else
    {
        list.push_back(dataChunk);
    }
}


// ****************************************************************************
//  Method: avtSILSpecification::UsesAllData
//
//  Purpose:
//      Determines if this SIL specification uses all of the data.
//
//  Programmer: Hank Childs
//  Creation:   June 17, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov 22 15:58:40 PST 2002
//    Use SIL restriction traverser.
//
// ****************************************************************************

bool
avtSILSpecification::UsesAllData(void)
{
    bool rv = false;
    if (useRestriction)
    {
        avtSILRestrictionTraverser trav(silr);
        rv = trav.UsesAllData();
    }
    else
    {
        rv = (dataChunk < 0 ? true : false);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtSILSpecification::UsesAllDomains
//
//  Purpose:
//      Determines if this SIL specification uses all of the domains.
//
//  Programmer: Hank Childs
//  Creation:   September 14, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov 22 15:58:40 PST 2002
//    Use SIL restriction traverser.
//
// ****************************************************************************

bool
avtSILSpecification::UsesAllDomains(void)
{
    bool rv = false;
    if (useRestriction)
    {
        avtSILRestrictionTraverser trav(silr);
        rv = trav.UsesAllDomains();
    }
    else
    {
        rv = (dataChunk < 0 ? true : false);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtSILSpecification::EmptySpecification
//
//  Purpose:
//      Determines if this SIL specification is empty.
//
//  Programmer: Hank Childs
//  Creation:   July 17, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov 22 15:58:40 PST 2002
//    Use SIL restriction traverser.
//
// ****************************************************************************

bool
avtSILSpecification::EmptySpecification(void)
{
    bool rv = false;
    if (useRestriction)
    {
        avtSILRestrictionTraverser trav(silr);
        if (trav.UsesSetData(silr->GetTopSet()) == NoneUsed)
        {
            rv = true;
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtSILSpecification::operator==
//
//  Purpose:
//      Determines if two SIL specifications are equal.
//
//  Arguments:
//      s       A SIL specification.
//
//  Returns:    true if the two specification are equal, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   July 26, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov 22 15:58:40 PST 2002
//    Use SIL restriction traverser.
//
// ****************************************************************************

bool
avtSILSpecification::operator==(const avtSILSpecification &s)
{
    if (useRestriction != s.useRestriction)
    {
        return false;
    }

    if (useRestriction)
    {
        avtSILRestrictionTraverser trav(silr);
        if (!(trav.Equal(s.silr)))
        {
            return false;
        }
    }
    else
    {
        if (dataChunk != s.dataChunk)
        {
            return false;
        }
    }

    return true;
}
