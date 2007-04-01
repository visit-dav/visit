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

    filename               = "<unknown>";
    containsGhostZones     = AVT_MAYBE_GHOSTS;
    containsOriginalCells     = false;

    SetTopologicalDimension(3);
    SetSpatialDimension(3);

    transform = NULL;
    canUseTransform = true;

    windowMode = WINMODE_NONE;
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

    for (int i = 0 ; i < variables.size() ; i++)
    {
        out << "Variable = " << variables[i].varname.c_str() << endl;
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
        AddVariable(vname);
        SetVariableDimension(di.variables[i].dimension, vname);
        SetCentering(di.variables[i].centering, vname);
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
    CopyTransform(di.transform);
    canUseTransform = di.canUseTransform;
    windowMode = di.windowMode;
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
    MergeTransform(da.transform);
    canUseTransform &= da.canUseTransform;
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
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname + ".\n";
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
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname + ".\n";
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
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname + ".\n";
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
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname + ".\n";
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
        string reason = "Attempting to retrieve data extents of non-existent ";
        reason = reason +  " variable: " + varname + ".\n";
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
        string reason = "Attempting to set dimension of non-existent ";
        reason = reason +  " variable: " + varname + ".\n";
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
        string reason = "Attempting to retrieve dimension of non-existent ";
        reason = reason +  " variable: " + varname + ".\n";
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
        string reason = "Attempting to set centering of non-existent";
        reason = reason +  " variable: " + varname + ".\n";
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
        string reason = "Attempting to retrieve centering of non-existent";
        reason = reason +  " variable: " + varname + ".\n";
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
// ****************************************************************************

void
avtDataAttributes::Write(avtDataObjectString &str,
                         const avtDataObjectWriter *wrtr)
{
    int   i;

    int numVals = 14 + 2*variables.size();
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
    vals[9] = (canUseTransform ? 1 : 0);
    vals[10] = (canUseCumulativeAsTrueOrCurrent ? 1 : 0);
    vals[11] = windowMode;
    vals[12] = activeVariable;
    vals[13] = variables.size();
    for (i = 0 ; i < variables.size() ; i++)
    {
        vals[14+2*i]   = variables[i].dimension;
        vals[14+2*i+1] = variables[i].centering;
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
        wrtr->WriteInt(str, variables[i].varname.size());
        str.Append((char *) variables[i].varname.c_str(),
                   variables[i].varname.size(),
                   avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
        variables[i].trueData->Write(str, wrtr);
        variables[i].cumulativeTrueData->Write(str, wrtr);
        variables[i].effectiveData->Write(str, wrtr);
        variables[i].currentData->Write(str, wrtr);
        variables[i].cumulativeCurrentData->Write(str, wrtr);
    }

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

    WriteLabels(str, wrtr);
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
    SetCanUseTransform(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetCanUseCumulativeAsTrueOrCurrent(tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    windowMode = (WINDOW_MODE) tmp;

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    activeVariable = tmp;

    int numVars;
    memcpy(&numVars, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);

    int *varDims = new int[numVars];
    avtCentering *centerings = new avtCentering[numVars];
    for (i = 0 ; i < numVars ; i++)
    {
        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        varDims[i] = tmp;
        memcpy(&tmp, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        centerings[i] = (avtCentering) tmp;
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
        int varname_length;
        memcpy(&varname_length, input, sizeof(int));
        input += sizeof(int); size += sizeof(int);
        string varname(input, varname_length);
        size += varname_length;
        input += varname_length;
        AddVariable(varname.c_str());
        SetCentering(centerings[i], varname.c_str());
        SetVariableDimension(varDims[i], varname.c_str());
 
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

    s = ReadLabels(input); 
    input += s; size += s;

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
        string reason = "Attempting to retrieve data extents of non-existent";
        reason = reason +  " variable: " + varname + ".\n";
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
// ****************************************************************************

void
avtDataAttributes::AddVariable(const std::string &s)
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
    new_var.dimension = -1;
    new_var.centering = AVT_UNKNOWN_CENT;
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


