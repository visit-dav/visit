// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtDataObjectQuery.h                            //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_QUERY_H
#define AVT_DATA_OBJECT_QUERY_H

#include <query_exports.h>

#include <avtDataObjectSink.h>
#include <vectortypes.h>
#include <MapNode.h>


class QueryAttributes;
class avtSILRestriction;
class SILRestrictionAttributes;

typedef void (*InitializeProgressCallback)(void *, int);
typedef void (*ProgressCallback)(void *, const char *, const char *,int,int);

// ****************************************************************************
//  Class: avtDataObjectQuery
//
//  Purpose:
//      The base class for a query object.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 12, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 23 15:11:44 PDT 2002
//    Add ChangedInput, VerifyInput.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002
//    Removed unnecessary SetAtts.
//
//    Kathleen Bonnell, Fri Jul 11 16:33:16 PDT 2003
//    Added units, rename GetMessage as GetResultMessage.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Tue Feb 10 15:00:32 PST 2004
//    Added OriginalData method.
//
//    Kathleen Bonnell, Thu Apr  1 16:04:44 PST 2004
//    Added PerformQueryInTime, SetTimeVarying, SetSILUseSet.
//
//    Kathleen Bonnell, Fri Apr  2 08:51:17 PST 2004
//    Changed args to PerformQueryInTime.
//
//    Kathleen Bonnell, Tue May  4 14:18:26 PDT 2004
//    Replaced SetSILUseSet with SetILRestriction.
//
//    Kathleen Bonnell, Thu Jun 24 07:45:47 PDT 2004
//    Added intVector and string arguments to PerformQueryInTime.
//
//    Kathleen Bonnell, Wed Jul 28 08:26:05 PDT 2004
//    Added Set/GetUnits.
//
//    Kathleen Bonnell, Mon Jan  3 15:08:37 PST 2005
//    Overloaded SetSILRestriction method with avtSILRestriction_p arg.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added GetTimeCurveSpecs.
//
//    Hank Childs, Thu Feb  8 09:57:39 PST 2007
//    Made GetNFilters be a public method.
//
//    Kathleen Bonnell, Tue Nov 20 10:27:51 PST 2007
//    Remove unused PerformQueryInTime method. Add GetShortDescription method,
//    whereby queries-through-time can specify a short label for y-axis than
//    their query name.
//
//    Kathleen Bonnell, Tue Jul  8 15:40:45 PDT 2008
//    Added timeCurveSpecs MapNode, changed TetTimecurveSpecs signature
//    to return this map node.
//
//    Hank Childs, Fri Dec 24 17:48:48 PST 2010
//    Add method QuerySupportsTimeParallelization.
//
//    Kathleen Bonnell, Fri Jun 17 16:25:09 PDT 2011
//    Added SetInputParams.
//
// ****************************************************************************

class QUERY_API avtDataObjectQuery : public virtual avtDataObjectSink
{
  public:
                            avtDataObjectQuery();
    virtual                ~avtDataObjectQuery();

    virtual const char     *GetType(void) = 0;
    virtual const char     *GetDescription(void) { return NULL; };
    virtual const char     *GetShortDescription(void) { return NULL; };

    virtual void            SetInputParams(const MapNode &) {;};
    virtual bool            OriginalData(void) { return false; };
    virtual void            PerformQuery(QueryAttributes *) = 0;
    virtual std::string     GetResultMessage(void) = 0;

    static void             RegisterInitializeProgressCallback(
                                          InitializeProgressCallback, void *);

    static void             RegisterProgressCallback(ProgressCallback, void *);
    virtual int             GetNFilters();

    virtual void            SetTimeVarying(bool val) { timeVarying = val;}
    virtual const MapNode  &GetTimeCurveSpecs();

    void                    SetSILRestriction(const SILRestrictionAttributes *);
    void                    SetSILRestriction(const avtSILRestriction_p);
    std::string            &GetUnits(void) { return units; }
    void                    SetUnits(const std::string &_units)
                                { units = _units;}

    virtual bool            QuerySupportsTimeParallelization(void)
                                { return false; };
    void                    SetParallelizingOverTime(bool v)
                                { parallelizingOverTime = v; };
    bool                    ParallelizingOverTime(void)
                                { return parallelizingOverTime; };

  protected:
    static InitializeProgressCallback
                                  initializeProgressCallback;
    static void                  *initializeProgressCallbackArgs;

    static ProgressCallback       progressCallback;
    static void                  *progressCallbackArgs;

    void                          Init(const int ntimesteps = 1);

    void                          UpdateProgress(int, int);
    virtual void                  ChangedInput(void);
    virtual void                  VerifyInput(void);

    std::string                   units;
    bool                          timeVarying;
    bool                          parallelizingOverTime;
    avtSILRestriction_p           querySILR;
    MapNode                       timeCurveSpecs;
};

#endif
