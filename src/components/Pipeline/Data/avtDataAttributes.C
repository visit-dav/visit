// ************************************************************************* //
//                             avtDataAttributes.C                           //
// ************************************************************************* //

#include <avtDataAttributes.h>

#include <algorithm>

#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>
#include <avtExtents.h>

#include <BadIndexException.h>
#include <ImproperUseException.h>
#include <InvalidMergeException.h>

using     std::string;
using     std::vector;
using     std::sort;


// ****************************************************************************
//  Method: avtDataAttributes constructor
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 13:39:02 PDT 2001
//    Removed unneeded fields.  Made all extents use avtExtents.
//
//    Hank Childs, Tue Sep 11 09:14:01 PDT 2001
//    Initialize more data members.
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Initialize current extents. 
//
//    Hank Childs, Fri Dec 21 07:18:33 PST 2001
//    Initialize varname.
//
//    Hank Childs, Fri Mar  8 16:39:30 PST 2002
//    Initialize cycle, dtime, and filename.
//
//    Hank Childs, Sun Jun 16 19:36:34 PDT 2002
//    Initialize cellOrigin and blockOrigin.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002
//    Initialize containsGhostZones.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003  
//    Initialize containsOriginalCells.
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Initialize transform.
//
//    Eric Brugger, Wed Aug 20 09:28:48 PDT 2003
//    Initialize windowMode.
// 
//    Hank Childs, Wed Oct 15 20:12:49 PDT 2003
//    Initialize labels.
//
//    Hank Childs, Mon Feb 23 08:59:42 PST 2004
//    Allow for multiple variables.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Initialize containsOriginalNodes, invTransform, canUseInvTransform.
//
//    Kathleen Bonnell, Tue Oct 12 16:11:15 PDT 2004 
//    Initialize keepNodeZoneArrays.
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Initialize containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Hank Childs, Sat Jan  1 11:23:50 PST 2005
//    Initialize meshname.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Initialize numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Initialize mirOccurred.
//
// ****************************************************************************

avtDataAttributes::avtDataAttributes()
{
    trueSpatial               = NULL;
    cumulativeTrueSpatial     = NULL;
    currentSpatial            = NULL;
    effectiveSpatial          = NULL;
    cumulativeCurrentSpatial  = NULL;

    canUseCumulativeAsTrueOrCurrent = false;

    activeVariable         = -1;
    cellOrigin             = 0;
    blockOrigin            = 0;
    topologicalDimension   = -1;
    spatialDimension       = -1;

    cycle                  = 0;
    cycleIsAccurate        = false;
    dtime                  = 0.;
    timeIsAccurate         = false;
    
    xLabel                 = "X-Axis";
    yLabel                 = "Y-Axis";
    zLabel                 = "Z-Axis";

    meshname               = "<unknown>";
    filename               = "<unknown>";
    containsGhostZones     = AVT_MAYBE_GHOSTS;
    containsOriginalCells  = false;
    containsOriginalNodes  = false;
    keepNodeZoneArrays     = false;
    containsGlobalZoneIds  = false;
    containsGlobalNodeIds  = false;

    SetTopologicalDimension(3);
    SetSpatialDimension(3);

    invTransform = NULL;
    canUseInvTransform = true;

    transform = NULL;
    canUseTransform = true;

    windowMode = WINMODE_NONE;

    numStates = 1;
    mirOccurred = false;
}


// ****************************************************************************
//  Method: avtDataAttributes destructor
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
//      
//  Modifications:
//
//    Hank Childs, Mon Feb 23 09:18:58 PST 2004
//    Moved all code in the former destructor to DestructSelf.  Also moved
//    appropriate comments.
//
// ****************************************************************************

avtDataAttributes::~avtDataAttributes()
{
    DestructSelf();
}


// ****************************************************************************
//  Method: avtDataAttributes::DestructSelf
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//      
//  Modifications:
//
//    Hank Childs, Tue Sep  4 13:39:02 PDT 2001
//    Reflected use of avtExtents.
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Delete current extents. 
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Delete transform.
//
//    Hank Childs, Mon Feb 23 08:59:42 PST 2004
//    Allow for multiple variables.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Delete invTransform.
//
// ****************************************************************************

void
avtDataAttributes::DestructSelf(void)
{
    spatialDimension = -1;
    if (trueSpatial != NULL)
    {
        delete trueSpatial;
        trueSpatial = NULL;
    }
    if (cumulativeTrueSpatial != NULL)
    {
        delete cumulativeTrueSpatial;
        cumulativeTrueSpatial = NULL;
    }
    if (effectiveSpatial != NULL)
    {
        delete effectiveSpatial;
        effectiveSpatial = NULL;
    }
    if (currentSpatial != NULL)
    {
        delete currentSpatial;
        currentSpatial = NULL;
    }
    if (cumulativeCurrentSpatial != NULL)
    {
        delete cumulativeCurrentSpatial;
        cumulativeCurrentSpatial = NULL;
    }

    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].trueData != NULL)
        {
            delete variables[i].trueData;
            variables[i].trueData = NULL;
        }
        if (variables[i].cumulativeTrueData != NULL)
        {
            delete variables[i].cumulativeTrueData;
            variables[i].cumulativeTrueData = NULL;
        }
        if (variables[i].effectiveData != NULL)
        {
            delete variables[i].effectiveData;
            variables[i].effectiveData = NULL;
        }
        if (variables[i].currentData != NULL)
        {
            delete variables[i].currentData;
            variables[i].currentData = NULL;
        }
        if (variables[i].cumulativeCurrentData != NULL)
        {
            delete variables[i].cumulativeCurrentData;
            variables[i].cumulativeCurrentData = NULL;
        }
    }
    variables.clear();

    if (invTransform != NULL)
    {
        delete invTransform;
        invTransform = NULL;
    }
    if (transform != NULL)
    {
        delete transform;
        transform = NULL;
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::Print
//
//  Purpose:
//      Prints out the contents of the data attributes.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Mar 24 09:18:39 PDT 2004
//    Fixed for Windows compiler.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Print containsOriginalNodes, invTransform.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Print VarInfo's treatAsASCII. 
//
//    Brad Whitlock, Wed Jul 21 15:25:11 PST 2004
//    Added variable units.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added vector of bools for which data selections were applied
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Added containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Hank Childs, Sat Jan  1 11:23:50 PST 2005
//    Added meshname.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Added numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Added mirOccurred.
//
// ****************************************************************************

void
avtDataAttributes::Print(ostream &out)
{
    out << "Spatial dimension = " << spatialDimension << endl;
    out << "Topological dimension = " << topologicalDimension << endl;
    out << "Cell origin = " << cellOrigin << endl;
    out << "Block origin = " << blockOrigin << endl;
    if (!timeIsAccurate)
        out << "Time is not known. Suspected to be " << dtime << endl;
    else
        out << "Time = " << dtime << endl;
    if (!cycleIsAccurate)
        out << "Cycle is not known. Suspected to be " << cycle << endl;
    else
        out << "Cycle = " << cycle << endl;

    if (containsOriginalCells)
        out << "This dataset contains the original cells list." << endl;
    else
        out << "This dataset's connectivity has changed." << endl;

    if (containsOriginalNodes)
        out << "This dataset contains the original nodes list." << endl;

    if (containsGlobalZoneIds)
        out << "This dataset contains the global zones list." << endl;

    if (containsGlobalNodeIds)
        out << "This dataset contains the global nodes list." << endl;

    switch (containsGhostZones)
    {
      case AVT_NO_GHOSTS:
        out << "There are no ghost zones in this dataset." << endl;
        break;
      case AVT_HAS_GHOSTS:
        out << "There are ghost zones in this dataset." << endl;
        break;
      case AVT_CREATED_GHOSTS:
        out << "There are generated ghost zones in this dataset." << endl;
        break;
      case AVT_MAYBE_GHOSTS:
        out << "There maybe ghost zones in this dataset." << endl;
        break;
    }

    if (!canUseInvTransform)
        out << "An operation has been performed on this data that prevents "
            << "using an invTransform" << endl;
    if (invTransform != NULL)
        out << invTransform << endl;

    if (!canUseTransform)
        out << "An operation has been performed on this data that prevents "
            << "using a transform" << endl;
    if (transform != NULL)
        out << transform << endl;

    if (canUseCumulativeAsTrueOrCurrent)
        out << "The cumulative extents can be used as true or current" << endl;
    else
        out << "The cumulative extents can not be used as true or current" 
            << endl;

    switch (windowMode)
    {
      case WINMODE_2D:
        out << "The window mode is 2D" << endl;
        break;
      case WINMODE_3D:
        out << "The window mode is 3D" << endl;
        break;
      case WINMODE_CURVE:
        out << "The window mode is curve" << endl;
        break;
      case WINMODE_NONE:
        out << "The window mode is none" << endl;
        break;
    }

    if (labels.size() > 0)
    {
        out << "The labels are: ";
        for (int i = 0 ; i < labels.size() ; i++)
        {
            out << labels[i].c_str();
            if (i < labels.size()-1)
               out << ", ";
        }
        out << endl;
    }

    out << "The mesh's name is " << meshname.c_str() << endl;
    out << "The filename is " << filename.c_str() << endl;
    out << "The X-units are " << xUnits.c_str() << endl;
    out << "The Y-units are " << yUnits.c_str() << endl;
    out << "The Z-units are " << zUnits.c_str() << endl;
    out << "The X-labels are " << xLabel.c_str() << endl;
    out << "The Y-labels are " << yLabel.c_str() << endl;
    out << "The Z-labels are " << zLabel.c_str() << endl;

    if (trueSpatial != NULL)
    {
        out << "True spatial = " << endl;
        trueSpatial->Print(out);
    }
    if (cumulativeTrueSpatial != NULL)
    {
        out << "Cumulative true spatial = " << endl;
        cumulativeTrueSpatial->Print(out);
    }
    if (effectiveSpatial != NULL)
    {
        out << "Effective spatial = " << endl;
        effectiveSpatial->Print(out);
    }
    if (currentSpatial != NULL)
    {
        out << "Current spatial = " << endl;
        currentSpatial->Print(out);
    }
    if (cumulativeCurrentSpatial != NULL)
    {
        out << "Cumulative current spatial = " << endl;
        cumulativeCurrentSpatial->Print(out);
    }

    int i;
    for (i = 0 ; i < variables.size() ; i++)
    {
        out << "Variable = " << variables[i].varname.c_str() << endl;
        if(variables[i].varunits != "")
            out << "Units = " << variables[i].varunits.c_str() << endl;
        out << "Dimension = " << variables[i].dimension << endl;
        switch (variables[i].centering)
        {
          case AVT_NODECENT:
            out << "Centering is nodal." << endl;
            break;
          case AVT_ZONECENT:
            out << "Centering is zonal." << endl;
            break;
          case AVT_NO_VARIABLE:
            out << "Centering is <no-variable> ... this shouldn't happen." 
                << endl;
            break;
          case AVT_UNKNOWN_CENT:
            out << "Centering is unknown." << endl;
            break;
        }
        if (variables[i].treatAsASCII)
            out << "Treat as ASCII." << endl;

        if (variables[i].trueData != NULL)
        {
            out << "True data = " << endl;
            variables[i].trueData->Print(out);
        }
        if (variables[i].cumulativeTrueData != NULL)
        {
            out << "Cumulative true data = " << endl;
            variables[i].cumulativeTrueData->Print(out);
        }
        if (variables[i].effectiveData != NULL)
        {
            out << "Effective data = " << endl;
            variables[i].effectiveData->Print(out);
        }
        if (variables[i].currentData != NULL)
        {
            out << "Current data = " << endl;
            variables[i].currentData->Print(out);
        }
        if (variables[i].cumulativeCurrentData != NULL)
        {
            out << "Cumulative current data = " << endl;
            variables[i].cumulativeCurrentData->Print(out);
        }

    }

    out << "Selections Applied: ";
    for (i = 0; i < selectionsApplied.size(); i++)
        out << (selectionsApplied[i] ? "T " : "F ");
    out << endl;

    out << "Num states: " << numStates << endl;
    if (mirOccurred)
        out << "Material Interace Reconstruction occurred: " << endl;

}


// ****************************************************************************
//  Method: avtDataAttributes::Copy
//
//  Purpose:
//      Copies the data attributes from the argument to this object.
//
//  Arguments:
//      di      Data attributes from another object.
//
//  Programmer: Hank Childs
//  Creation:   March 24, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 13:39:02 PDT 2001
//    Reflected use of avtExtents.
//
//    Kathleen Bonnell, Wed Sep 19 14:54:15 PDT 2001 
//    Copy over labels. 
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Copy over current extents. 
//
//    Hank Childs, Fri Dec 21 07:18:33 PST 2001
//    Copy over varname.
//
//    Hank Childs, Fri Mar  8 16:39:30 PST 2002
//    Copy over cycle, dtime, and filename.
//
//    Hank Childs, Sun Jun 16 19:36:34 PDT 2002
//    Copy cellOrigin and blockOrigin.
//
//    Hank Childs, Thu Sep 26 21:55:02 PDT 2002
//    Copy units.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002
//    Copy containsGhostZones.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003  
//    Copy containsOriginalCells.
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Copy transform.
//
//    Eric Brugger, Wed Aug 20 09:28:48 PDT 2003
//    Copy windowMode.
// 
//    Hank Childs, Wed Oct 15 20:12:49 PDT 2003
//    Copy labels.
//
//    Hank Childs, Mon Feb 23 09:18:58 PST 2004
//    Copy all variables.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Copy containsOriginalNodes, invTransform.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Copy varinfo's treatAsASCII.
//
//    Brad Whitlock, Tue Jul 20 14:02:32 PST 2004
//    Copied variable units.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added selectionsApplied
//
//    Kathleen Bonnell, Tue Oct 12 16:11:15 PDT 2004 
//    Added keepNodeZoneArrays. 
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Added containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Hank Childs, Sat Jan  1 11:23:50 PST 2005
//    Added meshname.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Added numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Added mirOccurred.
//
// ****************************************************************************

void
avtDataAttributes::Copy(const avtDataAttributes &di)
{
    DestructSelf();

    SetTopologicalDimension(di.topologicalDimension);
    SetSpatialDimension(di.spatialDimension);

    SetCellOrigin(di.cellOrigin);
    SetBlockOrigin(di.blockOrigin);
    if (di.cycleIsAccurate)
    {
        SetCycle(di.cycle);
    }
    else
    {
        cycleIsAccurate = false;
    }
    if (di.timeIsAccurate)
    {
        SetTime(di.dtime);
    }
    else
    {
        timeIsAccurate = false;
    }

    SetMeshname(di.GetMeshname());
    SetFilename(di.GetFilename());
    SetXUnits(di.GetXUnits());
    SetYUnits(di.GetYUnits());
    SetZUnits(di.GetZUnits());
    SetXLabel(di.GetXLabel());
    SetYLabel(di.GetYLabel());
    SetZLabel(di.GetZLabel());

    *(trueSpatial)               = *(di.trueSpatial);
    *(cumulativeTrueSpatial)     = *(di.cumulativeTrueSpatial);
    *(effectiveSpatial)          = *(di.effectiveSpatial);
    *(currentSpatial)            = *(di.currentSpatial);
    *(cumulativeCurrentSpatial)  = *(di.cumulativeCurrentSpatial);

    canUseCumulativeAsTrueOrCurrent = di.canUseCumulativeAsTrueOrCurrent;
    for (int i = 0 ; i < di.variables.size() ; i++)
    {
        const char *vname = di.variables[i].varname.c_str();
        AddVariable(vname, di.variables[i].varunits);
        SetVariableDimension(di.variables[i].dimension, vname);
        SetCentering(di.variables[i].centering, vname);
        SetTreatAsASCII(di.variables[i].treatAsASCII, vname);
        *(variables[i].trueData)              = *(di.variables[i].trueData);
        *(variables[i].cumulativeTrueData)    = 
                                      *(di.variables[i].cumulativeTrueData);
        *(variables[i].effectiveData)         =
                                      *(di.variables[i].effectiveData);
        *(variables[i].currentData)           = *(di.variables[i].currentData);
        *(variables[i].cumulativeCurrentData) = 
                                      *(di.variables[i].cumulativeCurrentData);
    }
    activeVariable = di.activeVariable;

    labels = di.labels;
    SetContainsGhostZones(di.GetContainsGhostZones());
    SetContainsOriginalCells(di.GetContainsOriginalCells());
    SetContainsOriginalNodes(di.GetContainsOriginalNodes());
    SetKeepNodeZoneArrays(di.GetKeepNodeZoneArrays());
    SetContainsGlobalZoneIds(di.GetContainsGlobalZoneIds());
    SetContainsGlobalNodeIds(di.GetContainsGlobalNodeIds());
    CopyInvTransform(di.invTransform);
    canUseInvTransform = di.canUseInvTransform;
    CopyTransform(di.transform);
    canUseTransform = di.canUseTransform;
    windowMode = di.windowMode;
    selectionsApplied = di.selectionsApplied;
    numStates = di.numStates;
    mirOccurred = di.mirOccurred;
}


// ****************************************************************************
//  Method: avtDataAttributes::Merge
//
//  Purpose:
//      Merges two dataset attributes.
//
//  Arguments:
//      da      The data attributes to merge in this object.
//
//  Programmer: Hank Childs
//  Creation:   March 25, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 13:39:02 PDT 2001
//    Reflected use of avtExtents.
//
//    Kathleen Bonnell, Wed Sep 19 14:54:15 PDT 2001 
//    Call MergeLabels method. 
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Merge current extents. 
//
//    Hank Childs, Fri Mar  8 16:39:30 PST 2002
//    Merge cycle and dtime.
//
//    Hank Childs, Sun Jun 16 19:36:34 PDT 2002
//    Merge cellOrigin and blockOrigin.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002
//    Merge containsGhostZones.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003  
//    Merge containsOriginalCells.
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Merge transform.
//
//    Kathleen Bonnell, Wed Jun 18 17:50:20 PDT 2003 
//    Account for avtGhostType AVT_CREATED_GHOSTS. 
//
//    Eric Brugger, Wed Aug 20 09:28:48 PDT 2003
//    Merge windowMode.
//
//    Mark C. Miller, Thu Jan 29 19:40:25 PST 2004
//    Added bools to ignore this or that. We might need to ignore this or that
//    if in parallel, we can't guarentee that one of the two operands to the
//    Merge operator has valid data members for ALL of them. Right now, the
//    only attribute that is 'ignored' is variable dimension. If we discover
//    that other circumstances arise other attribute data members can't be
//    guraenteed to be valid, we can add more logic here. Added the
//    support macro, CHECK_OR_IGNORE()
// 
//    Hank Childs, Mon Feb 23 09:18:58 PST 2004
//    Account for multiple variables.  Remove CHECK_OR_IGNORE, since the
//    logic got too complex for multiple variables.  Note that this is a
//    broader change than what Mark made on 1/04.  I have spoken with Mark and
//    he believes it is a reasonable thing to do.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Merge containsOriginalNodes, invTransform.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Test for equivalent treatAsASCII values.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added selectionsApplied
//
//    Kathleen Bonnell, Tue Oct 12 16:11:15 PDT 2004 
//    Added keepNodeZoneArrays. 
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Added containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Added numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Added mirOccurred.
//
// ****************************************************************************

void
avtDataAttributes::Merge(const avtDataAttributes &da,
   bool ignoreThis, bool ignoreThat)
{
    int   i;

    if (ignoreThat)
    {
        //
        // The argument da is not valid.  So we are going to stay with what
        // we have.  Just return.
        //
        return;
    }
    if (ignoreThis)
    {
        //
        // We know that the argument da is valid.  In addition, we are not
        // valid.  So just copy 'da' and return.
        //
        Copy(da);
        return;
    }

    //
    // From here on, we can assume that both the 'this' object and the 'da'
    // object are valid.  If not, we would have already returned.
    //

    if (variables.size() != da.variables.size())
    {
        EXCEPTION2(InvalidMergeException, variables.size(), 
                   da.variables.size());
    }
    for (i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname != da.variables[i].varname)
        {
            EXCEPTION0(InvalidMergeException);
        }
        if (variables[i].centering != da.variables[i].centering)
        {
            EXCEPTION2(InvalidMergeException, variables[i].centering,
                       da.variables[i].centering);
        }
        if (variables[i].treatAsASCII != da.variables[i].treatAsASCII)
        {
            EXCEPTION0(InvalidMergeException);
        }
        if (variables[i].dimension != da.variables[i].dimension)
        {
            EXCEPTION2(InvalidMergeException, variables[i].dimension,
                       da.variables[i].dimension);
        }
    }
    if (activeVariable != da.activeVariable)
    {
        EXCEPTION2(InvalidMergeException, activeVariable, da.activeVariable);
    }

    if (cellOrigin != da.cellOrigin)
    {
        EXCEPTION2(InvalidMergeException, cellOrigin, da.cellOrigin);
    }
    if (blockOrigin != da.blockOrigin)
    {
        EXCEPTION2(InvalidMergeException, blockOrigin, da.blockOrigin);
    }

    if (spatialDimension != da.spatialDimension)
    {
        EXCEPTION2(InvalidMergeException, spatialDimension,
                   da.spatialDimension);
    }
    if (topologicalDimension != da.topologicalDimension)
    {
        EXCEPTION2(InvalidMergeException, topologicalDimension,
                   da.topologicalDimension);
    }
    if (cycleIsAccurate && da.cycleIsAccurate && cycle != da.cycle)
    {
        EXCEPTION2(InvalidMergeException, cycle, da.cycle);
    }
    if (timeIsAccurate && da.timeIsAccurate && dtime != da.dtime)
    {
        EXCEPTION2(InvalidMergeException, dtime, da.dtime);
    }
    if (windowMode != da.windowMode)
    {
        EXCEPTION2(InvalidMergeException, windowMode, da.windowMode);
    }
    if (numStates != da.numStates)
    {
        EXCEPTION2(InvalidMergeException, numStates, da.numStates);
    }

    if (selectionsApplied.size() != da.selectionsApplied.size())
    {
        EXCEPTION2(InvalidMergeException, selectionsApplied.size(),
                                        da.selectionsApplied.size());
    }
    else
    {
        for (int i = 0; i < selectionsApplied.size(); i++)
        {
            if (selectionsApplied[i] != da.selectionsApplied[i])
            {
                EXCEPTION2(InvalidMergeException, selectionsApplied[i], 
                                               da.selectionsApplied[i]);
            }
        }
    }

    if (GetContainsGhostZones() == AVT_MAYBE_GHOSTS)
    {
        if (da.GetContainsGhostZones() == AVT_HAS_GHOSTS)
        {
            SetContainsGhostZones(AVT_HAS_GHOSTS);
        }
        else if (da.GetContainsGhostZones() == AVT_CREATED_GHOSTS)
        {
            SetContainsGhostZones(AVT_CREATED_GHOSTS);
        }
    }
    else if (GetContainsGhostZones() == AVT_NO_GHOSTS)
    {
        SetContainsGhostZones(da.GetContainsGhostZones());
    }

    if (!GetContainsOriginalCells()) 
    {
        SetContainsOriginalCells(da.GetContainsOriginalCells());
    }
    if (!GetContainsOriginalNodes()) 
    {
        SetContainsOriginalNodes(da.GetContainsOriginalNodes());
    }
    if (!GetKeepNodeZoneArrays()) 
    {
        SetKeepNodeZoneArrays(da.GetKeepNodeZoneArrays());
    }
    if (!GetContainsGlobalZoneIds()) 
    {
        SetContainsGlobalZoneIds(da.GetContainsGlobalZoneIds());
    }
    if (!GetContainsGlobalNodeIds()) 
    {
        SetContainsGlobalNodeIds(da.GetContainsGlobalNodeIds());
    }


    canUseCumulativeAsTrueOrCurrent &= da.canUseCumulativeAsTrueOrCurrent;

    trueSpatial->Merge(*(da.trueSpatial));
    cumulativeTrueSpatial->Merge(*(da.cumulativeTrueSpatial));
    effectiveSpatial->Merge(*(da.effectiveSpatial));
    currentSpatial->Merge(*(da.currentSpatial));
    cumulativeCurrentSpatial->Merge(*(da.cumulativeCurrentSpatial));

    for (i = 0 ; i < variables.size() ; i++)
    {
        variables[i].trueData->Merge(*(da.variables[i].trueData));
        variables[i].cumulativeTrueData->Merge(
                                        *(da.variables[i].cumulativeTrueData));
        variables[i].effectiveData->Merge(*(da.variables[i].effectiveData));
        variables[i].currentData->Merge(*(da.variables[i].currentData));
        variables[i].cumulativeCurrentData->Merge(
                                     *(da.variables[i].cumulativeCurrentData));
    }

    MergeLabels(da.labels);
    MergeInvTransform(da.invTransform);
    MergeTransform(da.transform);
    canUseInvTransform &= da.canUseInvTransform;
    canUseTransform &= da.canUseTransform;
    mirOccurred |= da.mirOccurred;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetSpatialExtents
//
//  Purpose:
//      Gets the spatial extents for the data object.  Hides the logic 
//      regarding which set of extents should be used first, etc.
//
//  Arguments:
//      buff     A buffer to copy the extents into.
//
//  Returns:     true if it found some good extents, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    September 4, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Nov 13 12:18:16 PST 2001
//    Don't use effective spatial extents.
//
// ****************************************************************************

bool
avtDataAttributes::GetSpatialExtents(double *buff)
{
    if (trueSpatial->HasExtents())
    {
        trueSpatial->CopyTo(buff);
        return true;
    }

    if (canUseCumulativeAsTrueOrCurrent &&
        cumulativeTrueSpatial->HasExtents())
    {
        cumulativeTrueSpatial->CopyTo(buff);
        return true;
    }

    return false;
}

// ****************************************************************************
//  Method: avtDataAttributes::GetAnySpatialExtents
//
//  Purpose:
//      Gets any spatial extents for the data object if any exists starting
//      with most useful to least useful
//
//  Arguments:
//      buff     A buffer to copy the extents into.
//
//  Returns:     true if it found some good extents, false otherwise.
//
//  Programmer:  Mark C. Miller
//  Creation:    February 29, 2004
//
// ****************************************************************************

bool
avtDataAttributes::GetAnySpatialExtents(double *buff)
{
    if (trueSpatial->HasExtents())
    {
        trueSpatial->CopyTo(buff);
        return true;
    }

    if (cumulativeTrueSpatial->HasExtents())
    {
        cumulativeTrueSpatial->CopyTo(buff);
        return true;
    }

    if (currentSpatial->HasExtents())
    {
        currentSpatial->CopyTo(buff);
        return true;
    }

    if (cumulativeCurrentSpatial->HasExtents())
    {
        cumulativeCurrentSpatial->CopyTo(buff);
        return true;
    }

    if (effectiveSpatial->HasExtents())
    {
        effectiveSpatial->CopyTo(buff);
        return true;
    }

    return false;
}

// ****************************************************************************
//  Method: avtDataAttributes::GetDataExtents
//
//  Purpose:
//      Gets the data extents for the data object.  Hides the logic 
//      regarding which set of extents should be used first, etc.
//
//  Arguments:
//      buff     A buffer to copy the extents into.
//
//  Returns:     true if it found some good extents, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    September 4, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Nov 13 12:18:16 PST 2001
//    Don't use effective data extents.
//
//    Hank Childs, Mon Feb 23 14:19:15 PST 2004
//    Account for multiple variables.
//
// ****************************************************************************

bool
avtDataAttributes::GetDataExtents(double *buff, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
        return false;

    if (variables[index].trueData->HasExtents())
    {
        variables[index].trueData->CopyTo(buff);
        return true;
    }

    if (canUseCumulativeAsTrueOrCurrent &&
        variables[index].cumulativeTrueData->HasExtents())
    {
        variables[index].cumulativeTrueData->CopyTo(buff);
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetTrueDataExtents
//
//  Purpose:
//      Gets the true data extents for the data object.  Hides the logic 
//      regarding which set of extents should be used first, etc.
//
//  Arguments:
//      varname  The variable to get the extents for.  If this argument is
//               NULL, then the extents for the active variable will be 
//               returned.
//
//  Returns:     the extents object for varname.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtExtents *
avtDataAttributes::GetTrueDataExtents(const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].trueData;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCumulativeTrueDataExtents
//
//  Purpose:
//      Gets the cumulative true data extents for the data object.
//
//  Arguments:
//      varname  The variable to get the extents for.  If this argument is
//               NULL, then the extents for the active variable will be 
//               returned.
//
//  Returns:     the extents object for varname.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtExtents *
avtDataAttributes::GetCumulativeTrueDataExtents(const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].cumulativeTrueData;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetEffectiveDataExtents
//
//  Purpose:
//      Gets the effective data extents for the data object.
//
//  Arguments:
//      varname  The variable to get the extents for.  If this argument is
//               NULL, then the extents for the active variable will be 
//               returned.
//
//  Returns:     the extents object for varname.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtExtents *
avtDataAttributes::GetEffectiveDataExtents(const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].effectiveData;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCurrentDataExtents
//
//  Purpose:
//      Gets the current data extents for the data object.
//
//  Arguments:
//      varname  The variable to get the extents for.  If this argument is
//               NULL, then the extents for the active variable will be 
//               returned.
//
//  Returns:     the extents object for varname.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtExtents *
avtDataAttributes::GetCurrentDataExtents(const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].currentData;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCumulativeCurrentDataExtents
//
//  Purpose:
//      Gets the cumulative current data extents for the data object.
//
//  Arguments:
//      varname  The variable to get the extents for.  If this argument is
//               NULL, then the extents for the active variable will be 
//               returned.
//
//  Returns:     the extents object for varname.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtExtents *
avtDataAttributes::GetCumulativeCurrentDataExtents(const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].cumulativeCurrentData;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetTopologicalDimension
//
//  Purpose:
//      Sets the topological dimension for the mesh.
//
//  Arguments:
//      td       The new topological dimension.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
// ****************************************************************************

void
avtDataAttributes::SetTopologicalDimension(int td)
{
    topologicalDimension = td;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetSpatialDimension
//
//  Purpose:
//      Sets the spatial dimension for the mesh.
//
//  Arguments:
//      td       The new spatial dimension.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Oct  3 10:57:13 PDT 2001
//    Add currentSpatial, cumulativeCurrentSpatial.
//
// ****************************************************************************

void
avtDataAttributes::SetSpatialDimension(int td)
{
    if (td == spatialDimension)
    {
        return;
    }

    spatialDimension = td;

    if (trueSpatial != NULL)
    {
        delete trueSpatial;
    }
    trueSpatial = new avtExtents(spatialDimension);

    if (cumulativeTrueSpatial != NULL)
    {
        delete cumulativeTrueSpatial;
    }
    cumulativeTrueSpatial = new avtExtents(spatialDimension);

    if (effectiveSpatial != NULL)
    {
        delete effectiveSpatial;
    }
    effectiveSpatial = new avtExtents(spatialDimension);

    if (currentSpatial != NULL)
    {
        delete currentSpatial;
    }
    currentSpatial = new avtExtents(spatialDimension);

    if (cumulativeCurrentSpatial != NULL)
    {
        delete cumulativeCurrentSpatial;
    }
    cumulativeCurrentSpatial = new avtExtents(spatialDimension);
}


// ****************************************************************************
//  Method: avtDataAttributes::SetVariableDimension
//
//  Purpose:
//      Sets the variable dimension.
//
//  Arguments:
//      vd       The new variable dimension.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Oct  3 10:57:13 PDT 2001
//    Add currentData, cumulativeCurrentData.
//
//    Hank Childs, Mon Feb 23 14:19:15 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Thu Mar 11 10:32:04 PST 2004 
//    DataExtents now always have dimension of 1. 
//
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

void
avtDataAttributes::SetVariableDimension(int vd, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to set dimension of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    if (vd == variables[index].dimension)
    {
        return;
    }

    variables[index].dimension  = vd;

    if (variables[index].trueData != NULL)
    {
        delete variables[index].trueData;
    }
    variables[index].trueData = new avtExtents(1);

    if (variables[index].cumulativeTrueData != NULL)
    {
        delete variables[index].cumulativeTrueData;
    }
    variables[index].cumulativeTrueData = new avtExtents(1);

    if (variables[index].effectiveData != NULL)
    {
        delete variables[index].effectiveData;
    }
    variables[index].effectiveData =new avtExtents(1);

    if (variables[index].currentData != NULL)
    {
        delete variables[index].currentData;
    }
    variables[index].currentData = new avtExtents(1);

    if (variables[index].cumulativeCurrentData != NULL)
    {
        delete variables[index].cumulativeCurrentData;
    }
    variables[index].cumulativeCurrentData = new avtExtents(1);
}


// ****************************************************************************
//  Method: avtDataAttributes::GetVariableDimension
//
//  Purpose:
//      Gets the variable dimension of a specific variable.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

int
avtDataAttributes::GetVariableDimension(const char *varname) const
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve dimension of non-existent ";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].dimension;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetCentering
//
//  Purpose:
//      Sets the centering type.
//
//  Arguments:
//      cen        The new centering type.
//
//  Programmer:    Hank Childs
//  Creation:      March 19, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Feb 23 14:19:15 PST 2004
//    Account for multiple variables.
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Fri Oct 22 13:57:58 PDT 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

void
avtDataAttributes::SetCentering(avtCentering cen, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to set centering of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    variables[index].centering = cen;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCentering
//
//  Purpose:
//      Gets the variable centering of a specific variable.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Fri Oct 22 13:57:58 PDT 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

avtCentering
avtDataAttributes::GetCentering(const char *varname) const
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve centering of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].centering;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetCellOrigin
//
//  Purpose:
//      Sets the cell origin.
//
//  Arguments:
//      origin     The new cell origin.
//
//  Programmer:    Hank Childs
//  Creation:      June 16, 2002
//
// ****************************************************************************

void
avtDataAttributes::SetCellOrigin(int origin)
{
    cellOrigin = origin;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetBlockOrigin
//
//  Purpose:
//      Sets the block origin.
//
//  Arguments:
//      origin     The new block origin.
//
//  Programmer:    Hank Childs
//  Creation:      June 16, 2002
//
// ****************************************************************************

void
avtDataAttributes::SetBlockOrigin(int origin)
{
    blockOrigin = origin;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetCycle
//
//  Purpose:
//      Sets the cycle number.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
// ****************************************************************************

void
avtDataAttributes::SetCycle(int c)
{
    cycle = c;
    cycleIsAccurate = true;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetTime
//
//  Purpose:
//      Sets the problem time.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
// ****************************************************************************

void
avtDataAttributes::SetTime(double d)
{
    dtime = d;
    timeIsAccurate = true;
}


// ****************************************************************************
//  Method: avtDataAttributes::Write
//
//  Purpose:
//      Writes the data attributes object out to a stream (string).
//
//  Arguments:
//      str      The string to write to.
//      wrtr     The writer that has information/methods about the destination
//               format.
//
//  Progammer: Hank Childs
//  Creation:  March 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sat May 26 10:06:58 PDT 2001
//    Made use of avtDataObjectString::Append instead of string::append to
//    prevent bottleneck.
//
//    Hank Childs, Tue Sep  4 13:39:02 PDT 2001
//    Reflected use of avtExtents.
//
//    Hank Childs, Wed Sep 19 10:24:02 PDT 2001
//    Make use of new array writing facilities.
//
//    Kathleen Bonnell, Thu Sep 20 14:24 PDT 2001 
//    Call WriteLabels method. 
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Write current extents. 
//
//    Hank Childs, Fri Mar  8 17:02:10 PST 2002
//    Write the cycle, time, and filename.
//
//    Hank Childs, Sun Jun 16 19:36:34 PDT 2002
//    Write cellOrigin and blockOrigin.
//
//    Hank Childs, Thu Sep 26 21:55:02 PDT 2002
//    Write units.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002
//    Write containsGhostZones.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003  
//    Write containsOriginalCells.
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Write transform.
//
//    Eric Brugger, Wed Aug 20 09:28:48 PDT 2003
//    Write windowMode.
// 
//    Hank Childs, Wed Oct 15 20:12:49 PDT 2003
//    Write labels.
//
//    Hank Childs, Mon Feb 23 14:19:15 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Write containsOriginalNodes, invTransform.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Write treatAsASCII. 
//
//    Brad Whitlock, Tue Jul 20 14:13:37 PST 2004
//    Added units.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added selectionsApplied
//
//    Kathleen Bonnell, Tue Oct 12 16:11:15 PDT 2004 
//    Added keepNodeZoneArrays. 
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Added containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Hank Childs, Sat Jan  1 11:23:50 PST 2005
//    Added meshname.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Added numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Added mirOccurred.
//
// ****************************************************************************

void
avtDataAttributes::Write(avtDataObjectString &str,
                         const avtDataObjectWriter *wrtr)
{
    int   i;

    int numVals = 21 + 3*variables.size();
    int *vals = new int[numVals];
    vals[0] = topologicalDimension;
    vals[1] = spatialDimension;
    vals[2] = cellOrigin;
    vals[3] = blockOrigin;
    vals[4] = cycle;
    vals[5] = (cycleIsAccurate ? 1 : 0);
    vals[6] = (timeIsAccurate ? 1 : 0);
    vals[7] = (int) containsGhostZones;
    vals[8] = (containsOriginalCells ? 1 : 0);
    vals[9] = (containsOriginalNodes ? 1 : 0);
    vals[10] = (keepNodeZoneArrays ? 1 : 0);
    vals[11] = (containsGlobalZoneIds ? 1 : 0);
    vals[12] = (containsGlobalNodeIds ? 1 : 0);
    vals[13] = (canUseInvTransform ? 1 : 0);
    vals[14] = (canUseTransform ? 1 : 0);
    vals[15] = (canUseCumulativeAsTrueOrCurrent ? 1 : 0);
    vals[16] = windowMode;
    vals[17] = numStates;
    vals[18] = mirOccurred;
    vals[19] = activeVariable;
    vals[20] = variables.size();
    for (i = 0 ; i < variables.size() ; i++)
    {
        vals[21+3*i]   = variables[i].dimension;
        vals[21+3*i+1] = variables[i].centering;
        vals[21+3*i+2] = (variables[i].treatAsASCII ? 1 : 0);
    }
    wrtr->WriteInt(str, vals, numVals);
    wrtr->WriteDouble(str, dtime);

    trueSpatial->Write(str, wrtr);
    cumulativeTrueSpatial->Write(str, wrtr);
    effectiveSpatial->Write(str, wrtr);
    currentSpatial->Write(str, wrtr);
    cumulativeCurrentSpatial->Write(str, wrtr);

    for (i = 0 ; i < variables.size() ; i++)
    {
        // Write the variable name
        wrtr->WriteInt(str, variables[i].varname.size());
        str.Append((char *) variables[i].varname.c_str(),
                   variables[i].varname.size(),
                   avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

        // Write the units name.
        int unitlen = variables[i].varunits.size();
        wrtr->WriteInt(str, unitlen);
        if(unitlen > 0)
        {
            str.Append((char *) variables[i].varunits.c_str(),
                        unitlen,
                        avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
        }

        variables[i].trueData->Write(str, wrtr);
        variables[i].cumulativeTrueData->Write(str, wrtr);
        variables[i].effectiveData->Write(str, wrtr);
        variables[i].currentData->Write(str, wrtr);
        variables[i].cumulativeCurrentData->Write(str, wrtr);
    }

    wrtr->WriteInt(str, meshname.size());
    str.Append((char *) meshname.c_str(), meshname.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

    wrtr->WriteInt(str, filename.size());
    str.Append((char *) filename.c_str(), filename.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

    wrtr->WriteInt(str, xUnits.size());
    str.Append((char *) xUnits.c_str(), xUnits.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    wrtr->WriteInt(str, yUnits.size());
    str.Append((char *) yUnits.c_str(), yUnits.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    wrtr->WriteInt(str, zUnits.size());
    str.Append((char *) zUnits.c_str(), zUnits.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

    wrtr->WriteInt(str, xLabel.size());
    str.Append((char *) xLabel.c_str(), xLabel.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    wrtr->WriteInt(str, yLabel.size());
    str.Append((char *) yLabel.c_str(), yLabel.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    wrtr->WriteInt(str, zLabel.size());
    str.Append((char *) zLabel.c_str(), zLabel.size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

    wrtr->WriteInt(str, selectionsApplied.size());
    for (i = 0; i < selectionsApplied.size(); i++)
        wrtr->WriteInt(str, selectionsApplied[i] ? 1 : 0);

    WriteLabels(str, wrtr);
    WriteInvTransform(str, wrtr);
    WriteTransform(str, wrtr);

    delete [] vals;
}


// ****************************************************************************
//  Method: avtDataAttributes::Read
//
//  Purpose:
//      Read the data attributes object from a stream (string).
//
//  Arguments:
//      input  The string to read from.
//
//  Returns:   The amount read.
//
//  Progammer: Hank Childs
//  Creation:  March 25, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Sep 20 14:24 PDT 2001 
//    Call ReadLabels method. 
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Read current extents. 
//
//    Hank Childs, Fri Mar  8 17:02:10 PST 2002
//    Read the cycle, time, and filename.
//
//    Hank Childs, Sun Jun 16 19:36:34 PDT 2002
//    Read cellOrigin and blockOrigin.
//
//    Hank Childs, Thu Sep 26 21:55:02 PDT 2002
//    Read units.
//
//    Hank Childs, Mon Sep 30 08:57:30 PDT 2002
//    Read containsGhostZones.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003  
//    Read containsOriginalCells.
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Read transform.
//
//    Eric Brugger, Wed Aug 20 09:28:48 PDT 2003
//    Read windowMode.
// 
//    Hank Childs, Wed Oct 15 20:12:49 PDT 2003
//    Read labels.
//
//    Hank Childs, Mon Feb 23 14:19:15 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Tue Jun  1 15:08:30 PDT 2004 
//    Read containsOriginalNodes, invTransform.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Read treatAsASCII. 
//
//    Brad Whitlock, Tue Jul 20 14:03:19 PST 2004
//    Added units.
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added selectionsApplied
//
//    Kathleen Bonnell, Tue Oct 12 16:11:15 PDT 2004 
//    Added keepNodeZoneArrays. 
//
//    Kathleen Bonnell, Thu Dec  9 16:12:33 PST 2004 
//    Added containsGlobalNodeIds, containsGlobalZoneIds.
//
//    Hank Childs, Sat Jan  1 11:23:50 PST 2005
//    Added meshname.
//
//    Kathleen Bonnell, Thu Jan 27 09:14:35 PST 2005 
//    Added numStates.
//
//    Kathleen Bonnell, Thu Feb  3 09:27:22 PST 2005 
//    Added mirOccurred.
//
//    Hank Childs, Thu Mar  3 16:20:49 PST 2005
//    Clear the selectionsApplied array before pushing new entries back.
//
// ****************************************************************************

int
avtDataAttributes::Read(char *input)
{
    int     i;
    int     size = 0;
    int     tmp;
    double  dtmp;

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetTopologicalDimension(tmp);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetSpatialDimension(tmp);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetCellOrigin(tmp);
  
    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetBlockOrigin(tmp);
  
    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    cycle = tmp;

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    cycleIsAccurate = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    timeIsAccurate = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetContainsGhostZones( (avtGhostType) tmp);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetContainsOriginalCells(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetContainsOriginalNodes(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetKeepNodeZoneArrays(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetContainsGlobalZoneIds(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetContainsGlobalNodeIds(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetCanUseInvTransform(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetCanUseTransform(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetCanUseCumulativeAsTrueOrCurrent(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    windowMode = (WINDOW_MODE) tmp;

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    numStates = tmp;

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    mirOccurred = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    activeVariable = tmp;

    int numVars;
    memcpy(&numVars, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);

    int *varDims = new int[numVars];
    avtCentering *centerings = new avtCentering[numVars];
    bool *ascii = new bool[numVars];
    for (i = 0 ; i < numVars ; i++)
    {
        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        varDims[i] = tmp;
        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        centerings[i] = (avtCentering) tmp;
        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        ascii[i] = (tmp != 0 ? true : false);
    }

    memcpy(&dtmp, input, sizeof(double));
    input += sizeof(double); size += sizeof(double);
    dtime = dtmp;

    int s;
    s = trueSpatial->Read(input);
    input += s; size += s;
    s = cumulativeTrueSpatial->Read(input);
    input += s; size += s;
    s = effectiveSpatial->Read(input);
    input += s; size += s;
    s = currentSpatial->Read(input);
    input += s; size += s;
    s = cumulativeCurrentSpatial->Read(input);
    input += s; size += s;

    for (i = 0 ; i < numVars ; i++)
    {
        // Get the name of the variable
        int varname_length;
        memcpy(&varname_length, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        string varname(input, varname_length);
        size += varname_length;
        input += varname_length;

        // Get the length of the name of the units.
        int unit_length;
        memcpy(&unit_length, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);

        // Add the variable.
        if(unit_length > 0)
        {
            string units(input, unit_length);
            AddVariable(varname, units);
        }
        else
            AddVariable(varname);

        // Adjust the size and input pointer in accordance with the
        // size of the units that we had. We do it here in case
        // we didn't have any units.
        size += unit_length;
        input += unit_length;

        SetCentering(centerings[i], varname.c_str());
        SetVariableDimension(varDims[i], varname.c_str());
        SetTreatAsASCII(ascii[i], varname.c_str());
 
        s = variables[i].trueData->Read(input);
        input += s; size += s;
        s = variables[i].cumulativeTrueData->Read(input);
        input += s; size += s;
        s = variables[i].effectiveData->Read(input);
        input += s; size += s;
        s = variables[i].currentData->Read(input);
        input += s; size += s;
        s = variables[i].cumulativeCurrentData->Read(input);
        input += s; size += s;
    }
    delete [] varDims;
    delete [] centerings;
    delete [] ascii;

    int meshnameSize;
    memcpy(&meshnameSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string l2(input, meshnameSize);
    meshname = l2;
    size += meshnameSize;
    input += meshnameSize;

    int filenameSize;
    memcpy(&filenameSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string l(input, filenameSize);
    filename = l;
    size += filenameSize;
    input += filenameSize;

    int unitSize;
    memcpy(&unitSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string xu(input, unitSize);
    xUnits = xu;
    size += unitSize;
    input += unitSize;
    memcpy(&unitSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string yu(input, unitSize);
    yUnits = yu;
    size += unitSize;
    input += unitSize;
    memcpy(&unitSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string zu(input, unitSize);
    zUnits = zu;
    size += unitSize;
    input += unitSize;

    int labelSize;
    memcpy(&labelSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string xl(input, labelSize);
    xLabel = xl;
    size += labelSize;
    input += labelSize;
    memcpy(&labelSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string yl(input, labelSize);
    yLabel = yl;
    size += labelSize;
    input += labelSize;
    memcpy(&labelSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    string zl(input, labelSize);
    zLabel = zl;
    size += labelSize;
    input += labelSize;

    int selectionsSize;
    memcpy(&selectionsSize, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    selectionsApplied.clear();
    for (i = 0; i < selectionsSize; i++)
    {
        int tmp;
        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        selectionsApplied.push_back(tmp);
    }

    s = ReadLabels(input); 
    input += s; size += s;

    s = ReadInvTransform(input); 
    input += s; 
    size  += s;

    s = ReadTransform(input); 
    input += s; 
    size  += s;

    return size;
}


// ****************************************************************************
//  Method: avtDataAttributes::MergeLabels
//
//  Purpose:
//    Merged labels with passed labels list.  
//
//  Arguments:
//    l        The list of labels with which to merge. 
//
//  Progammer: Kathleen Bonnell 
//  Creation:  September 19, 2001 
//
//  Modifications:
//
//    Hank Childs, Thu Mar 18 20:20:48 PST 2004
//    Re-wrote to avoid a quadratic performance.
//
// ****************************************************************************

void
avtDataAttributes::MergeLabels(const vector<string> &l)
{
    if (labels.size() == l.size())
    {
        bool foundDifference = false;
        for (int i = 0 ; i < labels.size() ; i++)
            if (labels[i] != l[i])
            {
                foundDifference = true;
                break;
            }
        if (!foundDifference)
            return;
    }

    vector<string> list1 = l;
    vector<string> list2 = labels;
    sort(list1.begin(), list1.end());
    sort(list2.begin(), list2.end());
    int list1_counter = 0;
    int list2_counter = 0;
    vector<string> master_list;
    while ((list1_counter < list1.size()) || (list2_counter < list2.size()))
    {
        if ((list1_counter < list1.size()) && (list2_counter < list2.size()))
        {
            if (list1[list1_counter] == list2[list2_counter])
            {
                master_list.push_back(list1[list1_counter]);
                list1_counter++;
                list2_counter++;
            }
            else if (list1[list1_counter] < list2[list2_counter])
                master_list.push_back(list1[list1_counter++]);
            else
                master_list.push_back(list2[list2_counter++]);
        }
        else if (list1_counter < list1.size())
            master_list.push_back(list1[list1_counter++]);
        else
            master_list.push_back(list2[list2_counter++]);
    }

    labels = master_list;
}


// ****************************************************************************
//  Method: avtDataAttributes::WriteLabels
//
//  Purpose:
//      Writes the data object information to a stream.
//
//  Arguments:
//      str     The string (stream) to write to.
//      wrtr    The writer that handles conversion to the destination format.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 19, 20001
//
// ****************************************************************************

void
avtDataAttributes::WriteLabels(avtDataObjectString &str,
                                const avtDataObjectWriter *wrtr)
{
    wrtr->WriteInt(str, labels.size());
    for (int i = 0; i < labels.size(); i++)
    {
       wrtr->WriteInt(str, labels[i].size());
       str.Append((char *) labels[i].c_str(), labels[i].size(),
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::ReadLabels
//
//  Purpose:
//    Reads the label information from a stream.
//
//  Arguments:
//    input     The string (stream) to write to.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2001
//
// ****************************************************************************

int
avtDataAttributes::ReadLabels(char *input)
{
    int size = 0;
    int numLabels;
    memcpy(&numLabels, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);

    int labelSize;
    for (int i = 0; i < numLabels; i++)
    {
        memcpy(&labelSize, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        string l(input, labelSize);
        labels.push_back(l);
        size += labelSize;
        input += labelSize;
    }
    return size;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetLabels
//
//  Purpose:
//    Sets the labels according to the passed argument.
//
//  Arguments:
//    l         The labels.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2001
//
// ****************************************************************************

void
avtDataAttributes::SetLabels(const vector<string> &l)
{
    if (!labels.empty())
    {
        labels.clear();
    }

    for (int i = 0; i < l.size(); i++)
    {
        labels.push_back(l[i]);
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::GetLabels
//
//  Purpose:
//    Returns the member labels.
//
//  Arguments:
//    l         A place to store the labels.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2001
//
// ****************************************************************************

void
avtDataAttributes::GetLabels(vector<string> &l)
{
    if (!l.empty())
    {
        l.clear();
    }

    for (int i = 0; i < labels.size(); i++)
    {
        l.push_back(labels[i]);
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCurrentDataExtents
//
//  Purpose:
//      Gets the current data extents for the data object.  Hides the logic 
//      regarding which set of extents should be used first, etc.
//
//  Arguments:
//      buff     A buffer to copy the extents into.
//
//  Returns:     true if it found some good extents, false otherwise.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    October 2, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Feb 23 08:59:42 PST 2004
//    Allow for multiple variables.
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
//    Hank Childs, Fri Oct 22 13:57:58 PDT 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

bool
avtDataAttributes::GetCurrentDataExtents(double *buff, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the variable dimension of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve data extents of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    if (variables[index].currentData->HasExtents())
    {
        variables[index].currentData->CopyTo(buff);
        return true;
    }

    if (canUseCumulativeAsTrueOrCurrent &&
        variables[index].cumulativeCurrentData->HasExtents())
    {
        variables[index].cumulativeCurrentData->CopyTo(buff);
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetCurrentSpatialExtents
//
//  Purpose:
//      Gets the current spatial extents for the data object.  Hides the logic 
//      regarding which set of extents should be used first, etc.
//
//  Arguments:
//      buff     A buffer to copy the extents into.
//
//  Returns:     true if it found some good extents, false otherwise.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    October 2, 2001
//
// ****************************************************************************

bool
avtDataAttributes::GetCurrentSpatialExtents(double *buff)
{
    if (currentSpatial->HasExtents())
    {
        currentSpatial->CopyTo(buff);
        return true;
    }

    if (canUseCumulativeAsTrueOrCurrent &&
        cumulativeCurrentSpatial->HasExtents())
    {
        cumulativeCurrentSpatial->CopyTo(buff);
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetVariableName
//
//  Purpose:
//      Returns the name of the active variable.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 31 08:03:47 PST 2004
//    Added a reason to the exception.
//
// ****************************************************************************

const std::string &
avtDataAttributes::GetVariableName(void) const
{
    if (activeVariable < 0)
    {
        string reason = "Attempting to retrieve non-existent";
        reason = reason +  " active variable.\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[activeVariable].varname;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetVariableName
//
//  Purpose:
//      Returns the name of the variable at an index.
//
//  Programmer: Hank Childs
//  Creation:   February 24, 2004
//
// ****************************************************************************

const std::string &
avtDataAttributes::GetVariableName(int index) const
{
    if (index < 0 || index >= variables.size())
    {
        EXCEPTION2(BadIndexException, index, variables.size());
    }

    return variables[index].varname;
}

// ****************************************************************************
// Method: avtDataAttributes::GetVariableUnits
//
// Purpose: 
//   Returns the active variable's units.
//
// Returns:    A reference to the active variable's units.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 20 16:36:00 PST 2004
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 27 14:44:20 PDT 2004
//   Allow retrieval by varname.
//   
// ****************************************************************************

const std::string &
avtDataAttributes::GetVariableUnits(const char *varname) const
{
    if (varname == NULL)
    {
        if (activeVariable < 0)
        {
            string reason = "Attempting to retrieve non-existent"
                            " active variable.\n";
            EXCEPTION1(ImproperUseException, reason);
        }

        return variables[activeVariable].varunits;
    }
    else
    {
        int index = VariableNameToIndex(varname);
        if (index < 0 || index >= variables.size())
        {
            EXCEPTION2(BadIndexException, index, variables.size());
        }

        return variables[index].varunits;
    }
}

// ****************************************************************************
// Method: avtDataAttributes::GetVariableUnits
//
// Purpose: 
//   Returns the units for the variable at an index.
//
// Arguments:
//   index : The index of the variable whose units we want.
//
// Returns:    A reference to the variable units.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 20 12:22:22 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

const std::string &
avtDataAttributes::GetVariableUnits(int index) const
{
    if (index < 0 || index >= variables.size())
    {
        EXCEPTION2(BadIndexException, index, variables.size());
    }

    return variables[index].varunits;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetNumberOfVariables
//
//  Purpose:
//      Returns the number of variables in this object.
//
//  Programmer: Hank Childs
//  Creation:   February 24, 2004
//
// ****************************************************************************

int
avtDataAttributes::GetNumberOfVariables(void) const
{
    return variables.size();
}


// ****************************************************************************
//  Method: avtDataAttributes::SetActiveVariable
//
//  Purpose:
//      Sets the active variable.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
// ****************************************************************************

void
avtDataAttributes::SetActiveVariable(const char *v)
{
    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname == v)
        {
            activeVariable = i;
            break;
        }
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::AddVariable
//
//  Purpose:
//      Adds a variable to the data attributes.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004
//    Initialize treatAsASCII.
//
//    Brad Whitlock, Tue Jul 20 12:24:28 PDT 2004
//    Added the units argument so units can be passed in if they are known.
//
// ****************************************************************************

void
avtDataAttributes::AddVariable(const std::string &s)
{
    AddVariable(s, "");
}

void
avtDataAttributes::AddVariable(const std::string &s, const std::string &units)
{
    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname == s)
        {
            //
            // We already have this variable -- just return.
            //
            return;
        }
    }

    VarInfo new_var;
    new_var.varname = s;
    new_var.varunits = units;
    new_var.dimension = -1;
    new_var.centering = AVT_UNKNOWN_CENT;
    new_var.treatAsASCII = false;
    new_var.trueData = NULL;
    new_var.cumulativeTrueData = NULL;
    new_var.effectiveData = NULL;
    new_var.currentData = NULL;
    new_var.cumulativeCurrentData = NULL;
    variables.push_back(new_var);
}

// ****************************************************************************
//  Method: avtDataAttributes::ValidVariable
//
//  Purpose:
//      Determines if a variable is valid (meaning it is contained in this
//      object).
//
//  Arguments:
//      vname   A variable name.
//
//  Returns:    true if we have information for vname, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
// ****************************************************************************

bool
avtDataAttributes::ValidVariable(const std::string &vname) const
{
    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname == vname)
        {
            return true;
        }
    }

    return false;
}


// ****************************************************************************
//  Method: avtDataAttributes::ValidActiveVariable
//
//  Purpose:
//      Determines if there is a valid active variable.
//
//  Returns:    true if we have information for vname, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
// ****************************************************************************

bool
avtDataAttributes::ValidActiveVariable(void) const
{
    if ((activeVariable >= 0) && (activeVariable < variables.size()))
        return true;

    return false;
}


// ****************************************************************************
//  Method: avtDataAttributes::RemoveVariable
//
//  Purpose:
//      Removes a variable from the data attributes.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
// ****************************************************************************

void
avtDataAttributes::RemoveVariable(const std::string &s)
{
    vector<VarInfo> new_vars;

    bool haveActiveVar = false;
    string activeVar;
    if (activeVariable >= 0)
    {
        haveActiveVar = true;
        activeVar = variables[activeVariable].varname;
    }

    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname != s)
            new_vars.push_back(variables[i]);
    }
    variables = new_vars;

    if (haveActiveVar)
    {
        activeVariable = VariableNameToIndex(activeVar.c_str());
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::VariableNameToIndex
//
//  Purpose:
//      Converts a variable name into an index.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2004
//
// ****************************************************************************

int
avtDataAttributes::VariableNameToIndex(const char *vname) const
{
    if (vname == NULL)
        return activeVariable;
    if (vname[0] == '\0')
        return activeVariable;

    for (int i = 0 ; i < variables.size() ; i++)
    {
        if (variables[i].varname == vname)
            return i;
    }

    return -1;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetInvTransform
//
//  Purpose:
//    Sets the transform matrix according to the passed argument.
//
//  Notes:
//    If the passed argument is NULL, no change occurs.
//
//    If this object's transformed matrix is NULL, a new matrix is constructed 
//    from the passed argument.
//
//    If this object's transform matrix is NOT NULL,  then it is multiplied
//    by the matrix created from the passed argument.  This allows for 
//    multiple transforms to occur in the same pipeline.
//   
//  Arguments:
//    D         The values of the transform matrix.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::SetInvTransform(const double *D)
{
   if (D == NULL)
       return;

   if (invTransform == NULL)
   {
       invTransform = new avtMatrix(D);
   }
   else
   {
       *invTransform = *invTransform * avtMatrix(D);
   }
}


// ****************************************************************************
//  Method: avtDataAttributes::CopyTransform
//
//  Purpose:
//    Sets the transform matrix according to the passed argument.
//
//  Arguments:
//    m         The transform matrix to be copied.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::CopyInvTransform(const avtMatrix *m)
{
   if (m == NULL)
       return;

   if (invTransform == NULL)
       invTransform = new avtMatrix();

   *invTransform = *m;
}


// ****************************************************************************
//  Method: avtDataAttributes::HasInvTransform
//
//  Purpose:
//      Returns whether or not there is a transfrom matrix in this object.
//
//  Returns:    true if it has transform, false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 10, 2003 
//
// ****************************************************************************

bool
avtDataAttributes::HasInvTransform()
{
    return invTransform != NULL;
}


// ****************************************************************************
//  Method: avtDataAttributes::MergeInvTransform
//
//  Purpose:
//    Merged the transform matrix with passed matrix.  This performs
//    multiplication of the matrices if neither are NULL. 
//
//  Arguments:
//    m        The transform matrix with which to merge. 
//
//  Progammer: Kathleen Bonnell 
//  Creation:  April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::MergeInvTransform(const avtMatrix *m)
{
    if (m == NULL)
        return;

    if (invTransform == NULL)
        invTransform = new avtMatrix(*m);
    else
        *invTransform = (*invTransform) * (*m);
}


// ****************************************************************************
//  Method: avtDataAttributes::WriteInvTransform
//
//  Purpose:
//      Writes the transform matrix to a stream.
//
//  Arguments:
//      str     The string (stream) to write to.
//      wrtr    The writer that handles conversion to the destination format.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003
//
// ****************************************************************************

void
avtDataAttributes::WriteInvTransform(avtDataObjectString &str,
                                const avtDataObjectWriter *wrtr)
{
    int has = (int)HasInvTransform();

    wrtr->WriteInt(str, has);

    if (has)
    {
       double *d = (*invTransform)[0];    
       wrtr->WriteDouble(str, d, 16);
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::ReadInvTransform
//
//  Purpose:
//    Reads the transform matrix from a stream.
//
//  Arguments:
//    input     The string (stream) to read from.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

int
avtDataAttributes::ReadInvTransform(char *input)
{
    int size = 0;
    int hasInvTransform;
    memcpy(&hasInvTransform, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);

    if (hasInvTransform)
    {
        double d[16];
        int s = sizeof(double) * 16;
        memcpy(d, input, s);
        input += s; 
        size += s;
        SetInvTransform(d);
    }
    return size;
}


// ****************************************************************************
//  Method: avtDataAttributes::SetTransform
//
//  Purpose:
//    Sets the transform matrix according to the passed argument.
//
//  Notes:
//    If the passed argument is NULL, no change occurs.
//
//    If this object's transformed matrix is NULL, a new matrix is constructed 
//    from the passed argument.
//
//    If this object's transform matrix is NOT NULL,  then it is multiplied
//    by the matrix created from the passed argument.  This allows for 
//    multiple transforms to occur in the same pipeline.
//   
//  Arguments:
//    D         The values of the transform matrix.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::SetTransform(const double *D)
{
   if (D == NULL)
       return;

   if (transform == NULL)
   {
       transform = new avtMatrix(D);
   }
   else
   {
       *transform = *transform * avtMatrix(D);
   }
}


// ****************************************************************************
//  Method: avtDataAttributes::CopyTransform
//
//  Purpose:
//    Sets the transform matrix according to the passed argument.
//
//  Arguments:
//    m         The transform matrix to be copied.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::CopyTransform(const avtMatrix *m)
{
   if (m == NULL)
       return;

   if (transform == NULL)
       transform = new avtMatrix();

   *transform = *m;
}


// ****************************************************************************
//  Method: avtDataAttributes::HasTransform
//
//  Purpose:
//      Returns whether or not there is a transfrom matrix in this object.
//
//  Returns:    true if it has transform, false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 10, 2003 
//
// ****************************************************************************

bool
avtDataAttributes::HasTransform()
{
    return transform != NULL;
}


// ****************************************************************************
//  Method: avtDataAttributes::MergeTransform
//
//  Purpose:
//    Merged the transform matrix with passed matrix.  This performs
//    multiplication of the matrices if neither are NULL. 
//
//  Arguments:
//    m        The transform matrix with which to merge. 
//
//  Progammer: Kathleen Bonnell 
//  Creation:  April 10, 2003 
//
// ****************************************************************************

void
avtDataAttributes::MergeTransform(const avtMatrix *m)
{
    if (m == NULL)
        return;

    if (transform == NULL)
        transform = new avtMatrix(*m);
    else
        *transform = (*transform) * (*m);
}


// ****************************************************************************
//  Method: avtDataAttributes::WriteTransform
//
//  Purpose:
//      Writes the transform matrix to a stream.
//
//  Arguments:
//      str     The string (stream) to write to.
//      wrtr    The writer that handles conversion to the destination format.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003
//
// ****************************************************************************

void
avtDataAttributes::WriteTransform(avtDataObjectString &str,
                                const avtDataObjectWriter *wrtr)
{
    int has = (int)HasTransform();

    wrtr->WriteInt(str, has);

    if (has)
    {
       double *d = (*transform)[0];    
       wrtr->WriteDouble(str, d, 16);
    }
}


// ****************************************************************************
//  Method: avtDataAttributes::ReadTransform
//
//  Purpose:
//    Reads the transform matrix from a stream.
//
//  Arguments:
//    input     The string (stream) to read from.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 10, 2003 
//
// ****************************************************************************

int
avtDataAttributes::ReadTransform(char *input)
{
    int size = 0;
    int hasTransform;
    memcpy(&hasTransform, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);

    if (hasTransform)
    {
        double d[16];
        int s = sizeof(double) * 16;
        memcpy(d, input, s);
        input += s; 
        size += s;
        SetTransform(d);
    }
    return size;
}

// ****************************************************************************
//  Method: avtDataAttributes::SetTreatAsASCII
//
//  Purpose:
//    Sets whether or not this var's data should be treated as ascii values. 
//
//  Arguments:
//    ascii       The new treatAsASCII value,
//
//  Programmer:    Kathleen Bonnell 
//  Creation:      July 21, 2004 
//
//  Modifications:
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

void
avtDataAttributes::SetTreatAsASCII(const bool ascii, const char *varname)
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to set the treatAsASCII value of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to set TreatAsASCII of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    variables[index].treatAsASCII = ascii;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetTreatAsASCII
//
//  Purpose:
//    Gets the value specifying if the variable's data should be treated as 
//    ascii values.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 21, 2004 
//
//  Modifications:
//
//    Hank Childs, Wed Dec  1 15:29:56 PST 2004
//    Make sure varname is non-NULL, or we'll crash.
//
// ****************************************************************************

bool
avtDataAttributes::GetTreatAsASCII(const char *varname) const
{
    int index = VariableNameToIndex(varname);
    if (index < 0)
    {
        //
        // We were asked to get the treatAsASCII of a non-existent
        // variable.
        //
        const char *varname_to_print = (varname != NULL ? varname
                                         : "<null>");
        string reason = "Attempting to retrieve TreatAsASCII of non-existent";
        reason = reason +  " variable: " + varname_to_print + ".\n";
        EXCEPTION1(ImproperUseException, reason);
    }

    return variables[index].treatAsASCII;
}

// ****************************************************************************
//  Method: avtDataAttributes::SetSelectionsApplied
//
//  Purpose: Sets the vector of bools indicating which selections have been
//     applied.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 28, 2004 
//
// ****************************************************************************

void
avtDataAttributes::SetSelectionsApplied(std::vector<bool> &selsApplied)
{
    selectionsApplied = selsApplied;
}


// ****************************************************************************
//  Method: avtDataAttributes::GetSelectionApplied
//
//  Purpose: Given the id of a data selection, returns the flag indicating
//     if it was applied. 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 28, 2004 
//
// ****************************************************************************

bool
avtDataAttributes::GetSelectionApplied(int selID) const
{
    if (selID < 0 || selID >= selectionsApplied.size())
        return false;
    else
        return selectionsApplied[selID];
}

// ****************************************************************************
//  Method: avtDataAttributes::GetSelectionsApplied
//
//  Purpose: Returns the whole vector of bools for which selections  were
//  applied
//
//  Programmer: Mark C. Miller 
//  Creation:   September 28, 2004 
//
// ****************************************************************************

const vector<bool> &
avtDataAttributes::GetSelectionsApplied() const
{
    return selectionsApplied;
}


// ****************************************************************************
//  Method: avtDataAttributes::TransformSpatialExtents
//
//  Purpose:
//      Transforms all of the spatial extents using a callback function.
//      The advantage of this routine is that all of the different extents
//      flavors don't have to be enumerated by the caller.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

void
avtDataAttributes::TransformSpatialExtents(avtDataAttributes &outAtts,
              void (*ProjectExtentsCallback)(const double *, double *, void *),
              void *args)
{
    double in[6], out[6]; // 6 is biggest possible -- not necessarily using
                          // all 6 -- up to callback function to decide.

    if (GetTrueSpatialExtents()->HasExtents())
    {
        GetTrueSpatialExtents()->CopyTo(in);
        ProjectExtentsCallback(in, out, args);
        outAtts.GetTrueSpatialExtents()->Set(out);
    }

    if (GetCumulativeTrueSpatialExtents()->HasExtents())
    {
        GetCumulativeTrueSpatialExtents()->CopyTo(in);
        ProjectExtentsCallback(in, out, args);
        outAtts.GetCumulativeTrueSpatialExtents()->Set(out);
    }

    if (GetEffectiveSpatialExtents()->HasExtents())
    {
        GetEffectiveSpatialExtents()->CopyTo(in);
        ProjectExtentsCallback(in, out, args);
        outAtts.GetEffectiveSpatialExtents()->Set(out);
    }

    if (GetCurrentSpatialExtents()->HasExtents())
    {
        GetCurrentSpatialExtents()->CopyTo(in);
        ProjectExtentsCallback(in, out, args);
        outAtts.GetCurrentSpatialExtents()->Set(out);
    }

    if (GetCumulativeCurrentSpatialExtents()->HasExtents())
    {
        GetCumulativeCurrentSpatialExtents()->CopyTo(in);
        ProjectExtentsCallback(in, out, args);
        outAtts.GetCumulativeCurrentSpatialExtents()->Set(out);
    }
}


