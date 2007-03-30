// ************************************************************************* //
//                             avtDataSpecification.C                        //
// ************************************************************************* //


#include <avtDataSpecification.h>

#include <avtSILRestrictionTraverser.h>

#include <ImproperUseException.h>


using     std::vector;


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
// ****************************************************************************

avtDataSpecification::avtDataSpecification(const char *var, int ts,
                                           avtSILRestriction_p s)
{
    mayRequireZones = false;
    needZones = false;
    needNodes = false;
    useGhostZones = true;
    needInternalSurfaces = false;
    needBoundarySurfaces = false;
    needValidFaceConnectivity = false;
    needStructuredIndices = false;
    usesAllDomains = true;
    needMixedVariableReconstruction = false;
    needSmoothMaterialInterfaces = false;
    needCleanZonesOnly = false;

    timestep  = ts;

    sil.useRestriction = true;
    sil.silr = s;

    variable  = new char[strlen(var)+1];
    strcpy(variable, var);
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
// ****************************************************************************

avtDataSpecification::avtDataSpecification(const char *var, int ts, int ch)
{
    mayRequireZones = false;
    needZones = false;
    needNodes = false;
    useGhostZones = true;
    needInternalSurfaces = false;
    needBoundarySurfaces = false;
    needValidFaceConnectivity = false;
    needStructuredIndices = false;
    usesAllDomains = true;
    needMixedVariableReconstruction = false;
    needSmoothMaterialInterfaces = false;
    needCleanZonesOnly = false;

    timestep  = ts;

    sil.useRestriction = false;
    sil.dataChunk = ch;

    variable  = new char[strlen(var)+1];
    strcpy(variable, var);
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
// ****************************************************************************

avtDataSpecification::avtDataSpecification(avtDataSpecification_p spec,
                                           avtSILRestriction_p silr)
{
    variable = NULL;
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
// ****************************************************************************

avtDataSpecification::avtDataSpecification(avtDataSpecification_p spec,
                                           int cdi)
{
    variable = NULL;
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
// ****************************************************************************

avtDataSpecification::avtDataSpecification(avtDataSpecification_p spec,
                                           const char *name)
{
    variable = NULL;
    (*this) = **spec;
    if (variable != NULL)
    {
        delete [] variable;
    }
    variable = new char[strlen(name)+1];
    strcpy(variable, name);
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
// ****************************************************************************

avtDataSpecification::avtDataSpecification(avtDataSpecification_p spec)
{
    variable = NULL;
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
// ****************************************************************************

avtDataSpecification &
avtDataSpecification::operator=(const avtDataSpecification &spec)
{
    if (variable != NULL)
    {
        delete [] variable;
    }

    timestep = spec.timestep;
    variable = new char[strlen(spec.variable)+1];
    strcpy(variable, spec.variable);

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
    useGhostZones                   = spec.useGhostZones;
    needZones                       = spec.needZones;
    needNodes                       = spec.needNodes;
    needInternalSurfaces            = spec.needInternalSurfaces;
    needBoundarySurfaces            = spec.needBoundarySurfaces;
    needValidFaceConnectivity       = spec.needValidFaceConnectivity;
    needStructuredIndices           = spec.needStructuredIndices;
    usesAllDomains                  = spec.usesAllDomains;
    needMixedVariableReconstruction = spec.needMixedVariableReconstruction;
    needSmoothMaterialInterfaces    = spec.needSmoothMaterialInterfaces;
    needCleanZonesOnly              = spec.needCleanZonesOnly;

    secondaryVariables = spec.secondaryVariables;

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

    if (mayRequireZones != ds.mayRequireZones)
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

    if (useGhostZones != ds.useGhostZones)
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
//  Method: avtDataSpecification destructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2001
//
// ****************************************************************************

avtDataSpecification::~avtDataSpecification()
{
    if (variable != NULL)
    {
        delete [] variable;
        variable = NULL;
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
//  Method: avtDataSpecification::NoGhostZones
//
//  Purpose:
//      This tells the reader that ghost zones are not necessary and that if
//      they weren't present, that would not be a problem.  In reality, the
//      ghost zones are still delivered by most readers, but the vtkGhostLevels
//      array will not be sent down, meaning that processing time drops
//      slightly.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2001
//
// ****************************************************************************

void
avtDataSpecification::NoGhostZones(void)
{
    useGhostZones = false;
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


