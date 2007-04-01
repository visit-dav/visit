// ************************************************************************* //
//                             avtDataAttributes.C                           //
// ************************************************************************* //

#include <avtDataAttributes.h>

#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>
#include <avtExtents.h>

#include <InvalidMergeException.h>


using     std::string;
using     std::vector;


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
// ****************************************************************************

avtDataAttributes::avtDataAttributes()
{
    trueSpatial               = NULL;
    cumulativeTrueSpatial     = NULL;
    trueData                  = NULL;
    cumulativeTrueData        = NULL;
    effectiveSpatial          = NULL;
    effectiveData             = NULL;
    currentSpatial            = NULL;
    cumulativeCurrentSpatial  = NULL;
    currentData               = NULL;
    cumulativeCurrentData     = NULL;
    canUseCummulativeAsTrueOrCurrent = false;

    centering              = AVT_UNKNOWN_CENT;
    cellOrigin             = 0;
    blockOrigin            = 0;
    topologicalDimension   = -1;
    spatialDimension       = -1;
    variableDimension      = -1;

    cycle                  = 0;
    cycleIsAccurate        = false;
    dtime                  = 0.;
    timeIsAccurate         = false;
    
    xLabel                 = "X-Axis";
    yLabel                 = "Y-Axis";
    zLabel                 = "Z-Axis";

    varname                = "<unknown>";
    filename               = "<unknown>";
    containsGhostZones     = AVT_MAYBE_GHOSTS;
    containsOriginalCells     = false;

    SetCentering(AVT_UNKNOWN_CENT);
    SetTopologicalDimension(3);
    SetSpatialDimension(3);
    SetVariableDimension(3);

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
//    Hank Childs, Tue Sep  4 13:39:02 PDT 2001
//    Reflected use of avtExtents.
//
//    Kathleen Bonnell, Tue Oct  2 15:25:23 PDT 2001 
//    Delete current extents. 
//
//    Kathleen Bonnell, Thu Apr 10 10:31:54 PDT 2003  
//    Delete transform.
//
// ****************************************************************************

avtDataAttributes::~avtDataAttributes()
{
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

    if (trueData != NULL)
    {
        delete trueData;
        trueData = NULL;
    }
    if (cumulativeTrueData != NULL)
    {
        delete cumulativeTrueData;
        cumulativeTrueData = NULL;
    }
    if (effectiveData != NULL)
    {
        delete effectiveData;
        effectiveData = NULL;
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
    if (currentData != NULL)
    {
        delete currentData;
        currentData = NULL;
    }
    if (cumulativeCurrentData != NULL)
    {
        delete cumulativeCurrentData;
        cumulativeCurrentData = NULL;
    }
    if (transform != NULL)
    {
        delete transform;
        transform = NULL;
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
// ****************************************************************************

void
avtDataAttributes::Copy(const avtDataAttributes &di)
{
    SetTopologicalDimension(di.topologicalDimension);
    SetSpatialDimension(di.spatialDimension);
    SetVariableDimension(di.variableDimension);
    SetCentering(di.centering);
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

    *(trueData)                  = *(di.trueData);
    *(cumulativeTrueData)        = *(di.cumulativeTrueData);
    *(effectiveData)             = *(di.effectiveData);
    *(currentData)               = *(di.currentData);
    *(cumulativeCurrentData)     = *(di.cumulativeCurrentData);
    canUseCummulativeAsTrueOrCurrent = di.canUseCummulativeAsTrueOrCurrent;

    labels = di.labels;
    varname = di.varname;
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
//    support macro, CHECK_OR_IGNOR()
// 
// ****************************************************************************

#define CHECK_OR_IGNORE(THIS,THAT)                                         \
    if (THIS != THAT)                                                      \
    {                                                                      \
        if      ( ignoreThis &&  ignoreThat)                               \
            ;                                                              \
        else if ( ignoreThis && !ignoreThat)                               \
            THIS = THAT;                                                   \
        else if (!ignoreThis &&  ignoreThat)                               \
            ;                                                              \
        else if (!ignoreThis && !ignoreThat)                               \
        {                                                                  \
            EXCEPTION2(InvalidMergeException, THIS, THAT);                 \
        }                                                                  \
    }

void
avtDataAttributes::Merge(const avtDataAttributes &da,
   bool ignoreThis, bool ignoreThat)
{
    //
    // Make sure that it will make sense to merge the data.
    //
    CHECK_OR_IGNORE(centering, da.centering);

    if (cellOrigin != da.cellOrigin)
    {
        EXCEPTION2(InvalidMergeException, cellOrigin, da.cellOrigin);
    }
    if (blockOrigin != da.blockOrigin)
    {
        EXCEPTION2(InvalidMergeException, blockOrigin, da.blockOrigin);
    }

    CHECK_OR_IGNORE(variableDimension, da.variableDimension);

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

    if (ignoreThis && ignoreThat)
        ; // no-op because we care about neither
    else if (ignoreThis && !ignoreThat)
    {
        *(trueSpatial)               = *(da.trueSpatial);
        *(cumulativeTrueSpatial)     = *(da.cumulativeTrueSpatial);
        *(effectiveSpatial)          = *(da.effectiveSpatial);
        *(currentSpatial)            = *(da.currentSpatial);
        *(cumulativeCurrentSpatial)  = *(da.cumulativeCurrentSpatial);
 
        *(trueData)                  = *(da.trueData);
        *(cumulativeTrueData)        = *(da.cumulativeTrueData);
        *(effectiveData)             = *(da.effectiveData);
        *(currentData)               = *(da.currentData);
        *(cumulativeCurrentData)     = *(da.cumulativeCurrentData);

        canUseCummulativeAsTrueOrCurrent = da.canUseCummulativeAsTrueOrCurrent;
    }
    else if (!ignoreThis && ignoreThat)
        ; // no-op because we care only about this
    else if (!ignoreThis && !ignoreThat)
    {
        trueSpatial->Merge(*(da.trueSpatial));
        cumulativeTrueSpatial->Merge(*(da.cumulativeTrueSpatial));
        effectiveSpatial->Merge(*(da.effectiveSpatial));
        currentSpatial->Merge(*(da.currentSpatial));
        cumulativeCurrentSpatial->Merge(*(da.cumulativeCurrentSpatial));

        trueData->Merge(*(da.trueData));
        cumulativeTrueData->Merge(*(da.cumulativeTrueData));
        effectiveData->Merge(*(da.effectiveData));
        currentData->Merge(*(da.currentData));
        cumulativeCurrentData->Merge(*(da.cumulativeCurrentData));
        canUseCummulativeAsTrueOrCurrent &= da.canUseCummulativeAsTrueOrCurrent;
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

    if (canUseCummulativeAsTrueOrCurrent &&
        cumulativeTrueSpatial->HasExtents())
    {
        cumulativeTrueSpatial->CopyTo(buff);
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
// ****************************************************************************

bool
avtDataAttributes::GetDataExtents(double *buff)
{
    if (trueData->HasExtents())
    {
        trueData->CopyTo(buff);
        return true;
    }

    if (canUseCummulativeAsTrueOrCurrent &&
        cumulativeTrueData->HasExtents())
    {
        cumulativeTrueData->CopyTo(buff);
        return true;
    }

    return false;
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
// ****************************************************************************

void
avtDataAttributes::SetVariableDimension(int vd)
{
    if (vd == variableDimension)
    {
        return;
    }

    variableDimension  = vd;

    if (trueData != NULL)
    {
        delete trueData;
    }
    trueData = new avtExtents(variableDimension);

    if (cumulativeTrueData != NULL)
    {
        delete cumulativeTrueData;
    }
    cumulativeTrueData = new avtExtents(variableDimension);

    if (effectiveData != NULL)
    {
        delete effectiveData;
    }
    effectiveData = new avtExtents(variableDimension);

    if (currentData != NULL)
    {
        delete currentData;
    }
    currentData = new avtExtents(variableDimension);

    if (cumulativeCurrentData != NULL)
    {
        delete cumulativeCurrentData;
    }
    cumulativeCurrentData = new avtExtents(variableDimension);
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
// ****************************************************************************

void
avtDataAttributes::SetCentering(avtCentering cen)
{
    centering = cen;
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
// ****************************************************************************

void
avtDataAttributes::Write(avtDataObjectString &str,
                         const avtDataObjectWriter *wrtr)
{
    const int numVals = 14;
    int vals[numVals];
    vals[0] = topologicalDimension;
    vals[1] = spatialDimension;
    vals[2] = variableDimension;
    vals[3] = centering;
    vals[4] = cellOrigin;
    vals[5] = blockOrigin;
    vals[6] = cycle;
    vals[7] = (cycleIsAccurate ? 1 : 0);
    vals[8] = (timeIsAccurate ? 1 : 0);
    vals[9] = (int) containsGhostZones;
    vals[10] = (containsOriginalCells ? 1 : 0);
    vals[11] = (canUseTransform ? 1 : 0);
    vals[12] = (canUseCummulativeAsTrueOrCurrent ? 1 : 0);
    vals[13] = windowMode;
    wrtr->WriteInt(str, vals, numVals);
    wrtr->WriteDouble(str, dtime);

    trueSpatial->Write(str, wrtr);
    cumulativeTrueSpatial->Write(str, wrtr);
    effectiveSpatial->Write(str, wrtr);
    currentSpatial->Write(str, wrtr);
    cumulativeCurrentSpatial->Write(str, wrtr);

    trueData->Write(str, wrtr);
    cumulativeTrueData->Write(str, wrtr);
    effectiveData->Write(str, wrtr);
    currentData->Write(str, wrtr);
    cumulativeCurrentData->Write(str, wrtr);

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
// ****************************************************************************

int
avtDataAttributes::Read(char *input)
{
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
    SetVariableDimension(tmp);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    SetCentering( (avtCentering) tmp);
  
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
    canUseTransform = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    canUseCummulativeAsTrueOrCurrent = (tmp != 0 ? true : false);

    memcpy(&tmp, input, sizeof(int));
    input += sizeof(int); size += sizeof(int);
    windowMode = (WINDOW_MODE) tmp;

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

    s = trueData->Read(input);
    input += s; size += s;
    s = cumulativeTrueData->Read(input);
    input += s; size += s;
    s = effectiveData->Read(input);
    input += s; size += s;
    s = currentData->Read(input);
    input += s; size += s;
    s = cumulativeCurrentData->Read(input);
    input += s; size += s;

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
// ****************************************************************************

void
avtDataAttributes::MergeLabels(const vector<string> &l)
{
    bool found = false;
    for (int i = 0; i < l.size(); i++)
    {
        for (int j = 0; j < labels.size(); j++)
        {
            if ( l[i] == labels[j] )
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            labels.push_back(l[i]);
        }
        found = false;
    }
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
// ****************************************************************************

bool
avtDataAttributes::GetCurrentDataExtents(double *buff)
{
    if (currentData->HasExtents())
    {
        currentData->CopyTo(buff);
        return true;
    }

    if (canUseCummulativeAsTrueOrCurrent &&
        cumulativeCurrentData->HasExtents())
    {
        cumulativeCurrentData->CopyTo(buff);
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

    if (canUseCummulativeAsTrueOrCurrent &&
        cumulativeCurrentSpatial->HasExtents())
    {
        cumulativeCurrentSpatial->CopyTo(buff);
        return true;
    }

    return false;
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


