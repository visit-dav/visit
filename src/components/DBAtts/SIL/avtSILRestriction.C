// ************************************************************************* //
//                           avtSILRestriction.C                             //
// ************************************************************************* //

#include <avtSILRestriction.h>

#include <avtSILMatrix.h>
#include <avtSILNamespace.h>

#include <CompactSILRestrictionAttributes.h>
#include <SILRestrictionAttributes.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <IncompatibleDomainListsException.h>
#include <InvalidVariableException.h>
#include <TimingsManager.h>

#define STATE_INDEX(S) (((S)==SomeUsed)?1:(((S)==AllUsed)?2:((S)==AllUsedOtherProc?3:0)))

using  std::string;
using  std::vector;


// ****************************************************************************
//  Method: avtSILRestriction constructor
//
//  Programmer: Brad Whitlock
//  Creation:   June 21, 2001
//
//  Modifications:
//   
//    Hank Childs, Thu Feb  7 16:09:04 PST 2002
//    Initialized suspendCorrectnessChecking.
//
// ****************************************************************************

avtSILRestriction::avtSILRestriction() : avtSIL(), useSet()
{
    suspendCorrectnessChecking = false;
    topSet = -1;
}


// ****************************************************************************
//  Method: avtSILRestriction constructor
//
//  Arguments:
//      silr    The SIL restriction to copy.
//
//  Programmer: Hank Childs
//  Creation:   May 21, 2001
//
//  Modifications:
//   
//    Brad Whitlock, Fri Jun 29 14:10:25 PST 2001
//    Added call to useSet's reserve  method to allocate prevent resizing.
//
//    Hank Childs, Thu Feb  7 16:09:04 PST 2002
//    Initialized suspendCorrectnessChecking.
//
// ****************************************************************************

avtSILRestriction::avtSILRestriction(avtSILRestriction_p silr)
    : avtSIL((avtSIL *) *silr)
{
    useSet = silr->useSet;
    topSet = silr->topSet;
    suspendCorrectnessChecking = false;
}


// ****************************************************************************
//  Method: avtSILRestriction constructor
//
//  Arguments:
//      silr    The SIL restriction to copy.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
// ****************************************************************************

avtSILRestriction::avtSILRestriction(const avtSILRestriction &silr)
    : avtSIL((avtSIL *) &silr)
{
    useSet = silr.useSet;
    topSet = silr.topSet;
    suspendCorrectnessChecking = false;
}


// ****************************************************************************
//  Method: avtSILRestriction assignment operator
//
//  Arguments:
//      silr    The SIL restriction to copy.
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
// ****************************************************************************

const avtSILRestriction &
avtSILRestriction::operator=(const avtSILRestriction &silr)
{
    avtSIL::operator=(silr);

    useSet = silr.useSet;
    topSet = silr.topSet;
    suspendCorrectnessChecking = false;

    return *this;
}


// ****************************************************************************
//  Method: avtSILRestriction constructor
//
//  Arguments:
//      sil     The SIL to copy.
//
//  Notes:      All of the sets will be used by default.
//     
//  Programmer: Hank Childs
//  Creation:   May 21, 2001
//
//  Modifications:
//
//    Brad Whitlock, Fri Jun 29 14:10:59 PST 2001
//    Changed useSet to be a vector of SetState.
//
//    Hank Childs, Thu Feb  7 16:09:04 PST 2002
//    Initialized suspendCorrectnessChecking.
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
// ****************************************************************************

avtSILRestriction::avtSILRestriction(avtSIL *sil)
    : avtSIL(sil), useSet(GetNumSets(), AllUsed)
{
    topSet = -1;
    suspendCorrectnessChecking = false;
}


// ****************************************************************************
//  Method: avtSILRestriction constructor
//
//  Arguments:
//      silatts    The attributes for a SIL restriction.
//
//  Programmer:    Hank Childs
//  Creation:      June 14, 2001
//
//  Modifications:
//
//    Brad Whitlock, Fri Jun 29 14:18:04 PST 2001
//    Made useAtts a vector of SetState.
//
//    Hank Childs, Thu Feb  7 16:09:04 PST 2002
//    Initialized suspendCorrectnessChecking.
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
// ****************************************************************************

avtSILRestriction::avtSILRestriction(const SILRestrictionAttributes &silatts)
    : avtSIL(silatts.GetSilAtts())
{
    const SetState states[3] = {NoneUsed, SomeUsed, AllUsed};

    topSet = silatts.GetTopSet();
    const vector<unsigned char> &useIt = silatts.GetUseSet();
    int ns = GetNumSets();
    useSet.reserve(ns);
    for (int i = 0 ; i < ns ; i++)
    {
        useSet.push_back(states[useIt[i]]);
    }
    suspendCorrectnessChecking = false;
}


// ****************************************************************************
//  Method: avtSILRestriction constructor
//
//  Arguments:
//      sil        A sil corresponding to the silatts.
//      silatts    Compact attributes for a SIL restriction.
//
//  Programmer:    Hank Childs
//  Creation:      December 14, 2001
//
//  Modifications:
//   
//    Hank Childs, Thu Feb  7 16:09:04 PST 2002
//    Initialized suspendCorrectnessChecking.
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
// ****************************************************************************

avtSILRestriction::avtSILRestriction(avtSIL *sil,
                                const CompactSILRestrictionAttributes &silatts)
    : avtSIL(sil)
{
    const SetState states[3] = {NoneUsed, SomeUsed, AllUsed};

    topSet = silatts.GetTopSet();
    const vector<unsigned char> &useIt = silatts.GetUseSet();
    int ns = GetNumSets();
    useSet.reserve(ns);
    for (int i = 0 ; i < ns ; i++)
    {
        int index = (int) useIt[i];
        useSet.push_back(states[index]);
    }
    suspendCorrectnessChecking = false;
}


// ****************************************************************************
//  Method: avtSILRestriction::SuspendCorrectnessChecking
//
//  Purpose:
//      Suspends the checking to make sure that the SIL is consistent.  This
//      exists to do things more efficiently when there is a mass on/off/toggle
//      operation.
//
//  Programmer: Hank Childs
//  Creation:   February 7, 2002
//
//  Modifications:
//    Brad Whitlock, Tue May 7 13:17:38 PST 2002
//    Set the flag to true.
//
// ****************************************************************************

void
avtSILRestriction::SuspendCorrectnessChecking(void)
{
    suspendCorrectnessChecking = true;
}


// ****************************************************************************
//  Method: avtSILRestriction::EnableCorrectnessChecking
//
//  Purpose:
//      Enables the checking to make sure that the SIL is consistent.  This
//      exists to do things more efficiently when there is a mass on/off/toggle
//      operation.
//
//  Programmer: Hank Childs
//  Creation:   February 7, 2002
//
//  Modifications:
//    Brad Whitlock, Tue May 7 13:17:38 PST 2002
//    Set the flag to false.
//
// ****************************************************************************

void
avtSILRestriction::EnableCorrectnessChecking(void)
{
    suspendCorrectnessChecking = false;
    int timingsHandle = visitTimer->StartTimer();
    EnsureRestrictionCorrectness(topSet);
    visitTimer->StopTimer(timingsHandle, "Ensuring restriction correctness");
}


// ****************************************************************************
//  Method: avtSILRestriction::SetTopSet
//
//  Purpose:
//      Sets which set is the "top set" for the SIL restriction.  A SIL
//      restriction can only correspond to one mesh -- this sets which mesh
//      that is.  There will typically be only one mesh in a file, so this
//      is normally a non-issue.
//
//  Arguments:
//      ts      The index of the set that should be regarded as the top set.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2001
//
//  Modifications:
//
//    Brad Whitlock, Fri Jun 29 17:31:42 PST 2001
//    Added a call to make sure the top set is correct.
//
//    Hank Childs, Thu Feb  7 16:19:11 PST 2002
//    Accounted for new style of suspending correctness inspections.
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
// ****************************************************************************

void
avtSILRestriction::SetTopSet(int ts)
{
    //
    // Make sure this is a valid choice for the top set.
    //
    int ns = GetNumSets();
    if (ts < 0 || ts >= ns)
    {
        EXCEPTION2(BadIndexException, ts, isWhole.size());
    }
    if (!isWhole[ts])
    {
        EXCEPTION0(ImproperUseException);
    }

    topSet = ts;

    //
    // Turn off the other whole sets that were not selected as "top".
    //
    int listSize = wholesList.size();
    for (int i = 0 ; i < listSize ; i++)
    {
        if (wholesList[i] != topSet)
        {
            TurnOffSet(wholesList[i]);
        }
    }

    // Correct any inconsistencies in the on/off state of the sets.
    if (!suspendCorrectnessChecking)
    {
        int timingsHandle = visitTimer->StartTimer();
        EnsureRestrictionCorrectness(topSet);
        visitTimer->StopTimer(timingsHandle, 
                                           "Ensuring restriction correctness");
    }
}


// ****************************************************************************
//  Method: avtSILRestriction::SetTopSet
//
//  Purpose:
//      Takes a character string and matches it to a whole set, allowing the
//      top set to be selected by set name rather than index.
//
//  Programmer: Hank Childs
//  Creation:   July 26, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
// ****************************************************************************

void
avtSILRestriction::SetTopSet(const char *meshname)
{
    int setIndex = -1;
    int listSize = wholesList.size();
    for (int i = 0 ; i < listSize ; i++)
    {
        avtSILSet_p set = GetSILSet(wholesList[i]);
        const string &str = set->GetName();
        if (strcmp(meshname, str.c_str()) == 0)
        {
            setIndex = wholesList[i];
            break;
        }
    }
 
    if (setIndex >= 0)
    {
        SetTopSet(setIndex);
    }
    else
    {
        debug1 << "Could not match up mesh " << meshname << " with a set."
               << endl;
        EXCEPTION0(ImproperUseException);
    }
}


// ****************************************************************************
// Method: avtSILRestriction::EnsureRestrictionCorrectness
//
// Purpose: 
//   This is called after making some modification to the state of sets in
//   the SIL restriction. It is intended that the method be called with the
//   topSet and it recursively fixes any disagreements on the set states. This
//   ensures that turning off all the leaves of a set affects the supersets
//   all the way up to the topSet.
//
// Arguments:
//   setId : The set for which we're ensuring correctness.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 29 17:26:28 PST 2001
//
// Modifications:
//   
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
// ****************************************************************************

SetState
avtSILRestriction::EnsureRestrictionCorrectness(int setId)
{
    //
    // Make sure that setId is a valid index.
    //
    int ns = GetNumSets();
    if (setId < 0 || setId >= ns)
    {
        EXCEPTION2(BadIndexException, setId, ns);
    }

    // Use the state of the current set.
    SetState retval = (SetState) useSet[setId];

    //
    // Get the value for all of its subsets.
    //
    avtSILSet_p set = GetSILSet(setId);
    const vector<int> &mapsOut = set->GetMapsOut();
    if (mapsOut.size() > 0)
    {
        int NoneUsedCount = 0;
        int SomeUsedCount = 0;
        int AllUsedCount = 0;

        // Loop through each collection and each set coming out of that
        // collection.
        for (int i = 0; i < mapsOut.size(); i++)
        {
            int collIndex = mapsOut[i];
            if (RealCollection(collIndex))
            {
                avtSILCollection_p coll = GetSILCollection(mapsOut[i]);
                const vector<int> &subsets = coll->GetSubsetList();
                for (int j = 0; j < subsets.size(); j++)
                {
                    SetState s = EnsureRestrictionCorrectness(subsets[j]);
                    if(s == NoneUsed)
                        ++NoneUsedCount;
                    else if(s == SomeUsed)
                        ++SomeUsedCount;
                    else
                        ++AllUsedCount;
                }
            }
            else
            {
                avtSILMatrix_p mat;
                int newCollIndex = 0;
                TranslateCollectionInfo(collIndex, mat, newCollIndex);
                SetState s = mat->GetSetState(useSet, newCollIndex);
                if(s == NoneUsed)
                    ++NoneUsedCount;
                else if(s == SomeUsed)
                    ++SomeUsedCount;
                else
                    ++AllUsedCount;
            }
        }

        // Based on the counts, decide which return value to use.
        if (SomeUsedCount > 0)
            retval = SomeUsed;
        else if(NoneUsedCount != 0 && AllUsedCount != 0)
            retval = SomeUsed;
        else if(AllUsedCount != 0)
            retval = AllUsed;
        else if(NoneUsedCount != 0)
            retval = NoneUsed;
        else
            retval = SomeUsed;

        // Set the state based the results of the children.
        useSet[setId] = retval;
    }
 
    return retval;
}


// ****************************************************************************
//  Method: avtSILRestriction::TurnOnSet
//
//  Purpose:
//      Sets a set to be on, including all of the subsets from it.
//
//  Arguments:
//      ind     The index of the set to turn off.
//
//  Programmer: Hank Childs
//  Creation:   May 21, 2001
//
//  Modifications:
//
//    Brad Whitlock, Fri Jun 29 14:20:39 PST 2001
//    Added code to make sure the restriction is correct.
//
//    Hank Childs, Thu Feb  7 16:19:11 PST 2002
//    Accounted for new style of suspending correctness inspections.
//
// ****************************************************************************

void
avtSILRestriction::TurnOnSet(int ind)
{
    TurnBoolSet(ind, true);

    if (!suspendCorrectnessChecking)
    {
        EnsureRestrictionCorrectness(topSet);
    }
}


// ****************************************************************************
//  Method: avtSILRestriction::TurnOffSet
//
//  Purpose:
//      Sets a set to be off, including all of the subsets from it.
//
//  Arguments:
//      ind     The index of the set to turn off.
//
//  Programmer: Hank Childs
//  Creation:   May 21, 2001
//
//  Modifications:
//
//    Brad Whitlock, Fri Jun 29 14:20:39 PST 2001
//    Added code to make sure the restriction is correct.
//
//    Hank Childs, Thu Feb  7 16:19:11 PST 2002
//    Accounted for new style of suspending correctness inspections.
//
// ****************************************************************************

void
avtSILRestriction::TurnOffSet(int ind)
{
    TurnBoolSet(ind, false);

    if (!suspendCorrectnessChecking)
    {
        EnsureRestrictionCorrectness(topSet);
    }
}


// ****************************************************************************
//  Method: avtSILRestriction::TurnBoolSet
//
//  Purpose:
//      Sets a set to be a Boolean value, including all of the subsets from it.
//
//  Arguments:
//      ind     The index of the set to turn off.
//
//  Programmer: Hank Childs
//  Creation:   May 21, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
// ****************************************************************************

void
avtSILRestriction::TurnBoolSet(int ind, bool b)
{
    //
    // Make sure that ind is a valid index.
    //
    int ns = GetNumSets();
    if (ind < 0 || ind >= ns)
    {
        EXCEPTION2(BadIndexException, ind, ns);
    }

    //
    // Make this set be the correct Boolean value.
    //
    useSet[ind] = b ? AllUsed : NoneUsed;

    //
    // Turn all of its subsets off.
    //
    avtSILSet_p set = GetSILSet(ind);
    const vector<int> &mapsOut = set->GetMapsOut();
    for (int i = 0 ; i < mapsOut.size() ; i++)
    {
        avtSILCollection_p coll = GetSILCollection(mapsOut[i]);
        const vector<int> &subsets = coll->GetSubsetList();
        for (int j = 0 ; j < subsets.size() ; j++)
        {
            TurnBoolSet(subsets[j], b);
        }
    }
}


// ****************************************************************************
//  Method: avtSILRestriction::TurnOnAll
//
//  Purpose:
//      Turns all of the sets to on.
//
//  Programmer: Hank Childs
//  Creation:   May 21, 2001
//
// ****************************************************************************

void
avtSILRestriction::TurnOnAll(void)
{
    for (int i = 0 ; i < useSet.size() ; i++)
    {
        useSet[i] = AllUsed;
    }
}


// ****************************************************************************
//  Method: avtSILRestriction::TurnOffAll
//
//  Purpose:
//      Turns all of the sets to off.
//
//  Programmer: Hank Childs
//  Creation:   May 21, 2001
//
// ****************************************************************************

void
avtSILRestriction::TurnOffAll(void)
{
    for (int i = 0 ; i < useSet.size() ; i++)
    {
        useSet[i] = NoneUsed;
    }
}


// ****************************************************************************
//  Method: avtSILRestriction::Intersect
//
//  Purpose:
//      Intersects a SILRestriction with this one.  The resulting intersection
//      will be put in this object.
//
//  Arguments:
//      silr    The SIL restriction to intersect.
//
//  Programmer: Hank Childs
//  Creation:   May 21, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Dec  4 17:23:12 PST 2002
//    Moved code to 'FastIntersect' (see accompanying comment in its header for
//    more details).  Blew away previous comments.
//
// ****************************************************************************

void
avtSILRestriction::Intersect(avtSILRestriction_p silr)
{
    FastIntersect(silr);

    //
    // Correct any inconsistencies in the on/off state of the sets. This is
    // meant to correct the case where we have Some & Some which could either
    // be Some or None.
    //
    if (!suspendCorrectnessChecking)
    {
        EnsureRestrictionCorrectness(topSet);
    }
}


// ****************************************************************************
//  Method: avtSILRestriction::FastIntersect
//
//  Purpose:
//      Intersects a SILRestriction with this one.  The resulting intersection
//      will be put in this object.  The 'Fast' qualifier is as follows: the
//      general Intersect must be concerned with intersecting two arbitrary
//      restrictions.  The issue is that a set may have 'some' in one 
//      restriction and 'some' in another, but the intersection of those two
//      may be 'some' and it may be 'none'.  Determining the correct value
//      requires calling 'EnsureRestrictionCorrectness'.  'FastIntersect' only
//      applies if you are *sure* that 'some' crossed with 'some' will always
//      be 'some'.
//
//  Arguments:
//      silr    The SIL restriction to intersect.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Nov 14 08:13:58 PST 2003
//    Account for AllUsedOnOtherProc designation.
//
// ****************************************************************************

void
avtSILRestriction::FastIntersect(avtSILRestriction_p silr)
{
    if (useSet.size() != silr->useSet.size())
    {
        EXCEPTION2(IncompatibleDomainListsException, useSet.size(),
                   silr->useSet.size());
    }

    //
    // Compare each set in the SIL restriction. We use the states array to
    // determine most values. We then call EnsureRestrictionCorrectness to
    // figure out the cases that we're not sure about.
    //
    SetState states[] = {NoneUsed, NoneUsed, NoneUsed, NoneUsed,
                         NoneUsed, SomeUsed, SomeUsed, AllUsedOtherProc,
                         NoneUsed, SomeUsed, AllUsed,  AllUsedOtherProc,
                         NoneUsed, AllUsedOtherProc, AllUsedOtherProc,
                                                            AllUsedOtherProc };

    const int nsets = useSet.size();
    for (int i = 0 ; i < nsets ; i++)
    {
        int index = (STATE_INDEX(useSet[i]) * 4) +
                     STATE_INDEX(silr->useSet[i]);
        useSet[i] = states[index];
    }
}


// ****************************************************************************
//  Method: avtSILRestriction::Union
//
//  Purpose:
//      Unions a SILRestriction with this one.  The resulting union will be put
//      in this object.
//
//  Arguments:
//      silr    The SIL restriction to union.
//
//  Programmer: Hank Childs
//  Creation:   May 21, 2001
//
//  Modifications:
//
//    Brad Whitlock, Fri Jun 29 16:27:48 PST 2001
//    Changed to support the fact that the restriction is no longer exactly
//    boolean.
//
//    Hank Childs, Thu Feb  7 16:19:11 PST 2002
//    Accounted for new style of suspending correctness inspections.
//
//    Hank Childs, Fri Nov 14 08:13:58 PST 2003
//    Account for AllUsedOnOtherProc designation.
//
// ****************************************************************************

void
avtSILRestriction::Union(avtSILRestriction_p silr)
{
    if (useSet.size() != silr->useSet.size())
    {
        EXCEPTION2(IncompatibleDomainListsException, useSet.size(),
                   silr->useSet.size());
    }

    //
    // Compare each set in the SIL restriction. We use the states array to
    // determine most values. We then call EnsureRestrictionCorrectness to
    // figure out the cases that we're not sure about.
    //
    SetState states[] = {NoneUsed, SomeUsed, AllUsed, AllUsedOtherProc,
                         SomeUsed, SomeUsed, AllUsed, SomeUsed /* ? */,
                         AllUsed, AllUsed, AllUsed, AllUsed,
                         AllUsedOtherProc, SomeUsed, AllUsed,
                                                            AllUsedOtherProc };

    for (int i = 0 ; i < useSet.size() ; i++)
    {
        int index = (STATE_INDEX(useSet[i]) * 4) +
                     STATE_INDEX(silr->useSet[i]);
        useSet[i] = states[index];
    }

    //
    // Correct any inconsistencies in the on/off state of the sets. This is
    // meant to correct the case where we have Some | Some which could either
    // be Some or All.
    //
    if (!suspendCorrectnessChecking)
    {
        EnsureRestrictionCorrectness(topSet);
    }
}


// ****************************************************************************
//  Method: avtSILRestriction::RestrictDomains
//
//  Purpose: Public interface to restrict domains
//
//  Arguments:
//      domains   A list of domains to use.
//
//  Programmer: Mark C. Miller
//  Creation:   October 15, 2003 
//
// ****************************************************************************

void
avtSILRestriction::RestrictDomains(const vector<int> &domains)
{
    RestrictDomains(domains, false);
}

// ****************************************************************************
//  Method: avtSILRestriction::RestrictDomainsForLoadBalance
//
//  Purpose: Public interface to restrict domains for load balancing 
//
//  Arguments:
//      domains   A list of domains to use.
//
//  Programmer: Mark C. Miller
//  Creation:   October 15, 2003 
//
// ****************************************************************************

void
avtSILRestriction::RestrictDomainsForLoadBalance(const vector<int> &domains)
{
    RestrictDomains(domains, true);
}

// ****************************************************************************
//  Method: avtSILRestriction::RestrictDomains
//
//  Purpose:
//      A convenience routine for filters that don't want to worry about how
//      to traverse a SIL.
//
//  Arguments:
//      domains   A list of domains to use.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2001
//
//  Modifications:
//    Brad Whitlock, Fri Jun 29 17:11:13 PST 2001
//    Changed to reflect the fact that the restriction uses SetState instead
//    of just using bool.
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
//    Hank Childs, Wed Dec  4 16:15:58 PST 2002
//    Re-wrote for performance.
//
//    Mark C. Miller, Wed Oct 15 16:24:13 PDT 2003
//    Made private, added bool for special behavior if for load balancing
//
//    Hank Childs, Thu Nov 13 16:52:23 PST 2003
//    Pass 'forLoadBalance' argument on to SIL matrix 'TurnSet' call.
//
// ****************************************************************************

void
avtSILRestriction::RestrictDomains(const vector<int> &domains,
                                   bool forLoadBalance)
{
    int timingsHandle = visitTimer->StartTimer();

    int  i;

    if (topSet < 0)
    {
        //
        // We don't know which set to restrict if the top set hasn't been set.
        //
        EXCEPTION0(ImproperUseException);
    }

    //
    // The input is not exactly what we want -- we want a vector where we can
    // index it by domain number for fast lookups -- so create that vector.
    //
    int largest = -1;
    for (i = 0 ; i < domains.size() ; i++)
    {
        if (domains[i] > largest)
        {
            largest = domains[i];
        }
    }
    if (largest < 0)
    {
        TurnOffAll();
        visitTimer->StopTimer(timingsHandle, "Restricting the domain list");
        return;
    }

    //
    // Create a vector called lookup whose entries are all initially false.
    // The lookup vector is a list of the domains that will be turned on.
    //
    vector<bool> lookup(largest + 1, false);
    for (i = 0 ; i < domains.size() ; i++)
    {
        lookup[domains[i]] = true;
    }

    vector<int> setsToProcess;
    setsToProcess.push_back(topSet);
    for (i = 0 ; i < setsToProcess.size() ; i++)
    {
        avtSILSet_p currentSet = GetSILSet(setsToProcess[i]);
        int id = currentSet->GetIdentifier();
        if (id < 0)
        {
            //
            // This is a special set that contains many domains underneath it.
            // If the maps coming out of it are real, then add their subsets to
            // our list to process.  If not, then the collection goes into a
            // matrix, and we will let the 'other dimension' handle it (ie this
            // is a material and we will let the domains handle it).
            //
            const vector<int> &mapsOut = currentSet->GetMapsOut();
            for (int j = 0 ; j < mapsOut.size() ; j++)
            {
                if (RealCollection(mapsOut[j]))
                {
                    avtSILCollection_p coll = GetSILCollection(mapsOut[j]);
                    const vector<int> &subsets =
                                          coll->GetSubsets()->GetAllElements();
                    for (int k = 0 ; k < subsets.size() ; k++)
                    {
                        setsToProcess.push_back(subsets[k]);
                    }
                }
            }
        }
        else if (id < largest+1 && lookup[id] == true)
        {
            // We do not need to 'restrict' these domains.
            continue;
        }
        else
        {
            // Everything for this set and below should be turned off.
            vector<int> setsToTurnOff;
            setsToTurnOff.push_back(setsToProcess[i]);
            for (int j = 0 ; j < setsToTurnOff.size() ; j++)
            {
                if (forLoadBalance)
                {
                    if (useSet[setsToTurnOff[j]] != NoneUsed) 
                        useSet[setsToTurnOff[j]] = AllUsedOtherProc;
                    else
                        useSet[setsToTurnOff[j]] = NoneUsed;
                }
                else
                {
                    useSet[setsToTurnOff[j]] = NoneUsed;
                }
                avtSILSet_p set = GetSILSet(setsToTurnOff[j]);
                const vector<int> &mapsOut = set->GetMapsOut();
                for (int k = 0 ; k < mapsOut.size() ; k++)
                {
                    if (RealCollection(mapsOut[k]))
                    {
                        avtSILCollection_p coll = GetSILCollection(mapsOut[k]);
                        const vector<int> &subsets = coll->GetSubsetList();
                        for (int l = 0; l < subsets.size(); l++)
                        {
                            setsToTurnOff.push_back(subsets[l]);
                        }
                    }
                    else
                    {
                        avtSILMatrix_p mat;
                        int newCollIndex = 0;
                        TranslateCollectionInfo(mapsOut[k], mat, newCollIndex);
                        mat->TurnSet(useSet, newCollIndex, NoneUsed,
                                     forLoadBalance);
                    }
                }
            }
        }
    }

    visitTimer->StopTimer(timingsHandle, "Restricting the domain list");
}


// ****************************************************************************
//  Method: avtSILRestriction::MakeAttributes
//
//  Purpose:
//      Makes the attributes for the SIL restriction.
//
//  Returns:     The attributes object for a SIL restriction.
//
//  Programmer:  Hank Childs
//  Creation:    May 21, 2001
//
//  Modifications:
//    Brad Whitlock, Fri Jun 29 17:14:45 PST 2001
//    Modified it to accomodate non-bool useSet values.
//
// ****************************************************************************

SILRestrictionAttributes *
avtSILRestriction::MakeAttributes(void) const
{
    SILRestrictionAttributes *rv = new SILRestrictionAttributes;

    SILAttributes *sa = MakeSILAttributes();
    rv->SetSilAtts(*sa);
    delete sa;

    //
    // classwriter (which makes the state objects) cannot handle bool vectors.
    // Make our vector into an int vector so it can handle it.
    //
    vector<unsigned char> iUseSet;
    for (int i = 0 ; i < useSet.size() ; i++)
    {
        int  val = STATE_INDEX(useSet[i]);
        iUseSet.push_back(val);
    }
    rv->SetUseSet(iUseSet);

    rv->SetTopSet(topSet);

    return rv;
}


// ****************************************************************************
//  Method: avtSILRestriction::MakeCompactAttributes
//
//  Purpose:
//      Makes compact attributes for a SIL restriction.
//
//  Returns:     Compact attributes object for a SIL restriction.
//
//  Programmer:  Hank Childs
//  Creation:    December 14, 2001
//
// ****************************************************************************

CompactSILRestrictionAttributes *
avtSILRestriction::MakeCompactAttributes(void) const
{
    CompactSILRestrictionAttributes *rv = new CompactSILRestrictionAttributes;

    vector<unsigned char> iUseSet;
    for (int i = 0 ; i < useSet.size() ; i++)
    {
        char  val = (char) STATE_INDEX(useSet[i]);
        iUseSet.push_back(val);
    }
    rv->SetUseSet(iUseSet);

    rv->SetTopSet(topSet);

    return rv;
}


// ****************************************************************************
//  Method: avtSILRestriction::Print
//
//  Purpose: 
//      Prints the SIL restriction to a stream. This differs from avtSIL's 
//      Print method in that is also prints out whether or not sets are marked 
//      as active.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jun 22 15:45:18 PST 2001
//
//  Modifications:
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Dramatically reduced the sets being printed out, because of new matrix
//    format.
//
//    Mark C. Miller, 23Sep03, added per-set info for state of each set's
//    selection
//
// ****************************************************************************

void
avtSILRestriction::Print(ostream &out) const
{
    // make labels for state of each set
    static const char *stateNames[3] = {"NoneUsed", "SomeUsed", "AllUsed"};

    std::vector< std::string > perSetInfo, dummyInfo;
    for (int i = 0 ; i < useSet.size() ; i++)
        perSetInfo.push_back(stateNames[STATE_INDEX(useSet[i])]);

    avtSIL::Print(out, perSetInfo, dummyInfo, dummyInfo);
    out << "Top Set = " << topSet << endl;
}


// ****************************************************************************
//  Method: avtSILRestriction::GetLeafSets
//
//  Purpose:
//      Gets all of the leaf sets under a specific "root" set.
//
//  Arguments:
//      ind     The index of a set.  This can be any set.
//      leaves  A list of the leaves underneath that set.
//
//  Programmer: Hank Childs
//  Creation:   September 13, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Nov 14 15:55:49 PST 2001
//    Added an indirection through a map to correct a bug.
//
//    Fix inefficiency.
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
// ****************************************************************************

void
avtSILRestriction::GetLeafSets(int ind, vector<int> &leaves)
{
    int timingsHandle = visitTimer->StartTimer();

    int   i, j;

    //
    // Improbable, but make sure we have valid input.
    //
    int nSets = GetNumSets();
    if (ind < 0 || ind >= nSets)
    {
        EXCEPTION2(BadIndexException, ind, nSets);
    }

    //
    // This may take some memory, but start off by setting every set as "off"
    //
    vector<bool> isOn(nSets);
    for (i = 0 ; i < nSets ; i++)
    {
        isOn[i] = false;
    }

    //
    // We will need to walk over all of the maps coming out of "ind".  Start
    // by taking all of the maps coming directly out and then follow them all
    // down.
    //
    avtSILSet_p set = GetSILSet(ind);
    const vector<int> &initialMaps = set->GetMapsOut();
    vector<int> maps;
    for (i = 0 ; i < initialMaps.size() ; i++)
    {
        maps.push_back(initialMaps[i]);
    }

    //
    // Go over each node that comes out of the initial map.  Mark them as
    // "seen" (we will address issues of what is a leaf or not later).  Then
    // take their maps out and start walking down the tree.  Each map of
    // interest is added to the "maps" list.  Its a bit funny to iterate over
    // the list we are adding to, but it works (fingers crossed ;).
    //
    vector<bool> alreadyProcessedMap;
    int numCollections = GetNumCollections();
    alreadyProcessedMap.resize(numCollections);
    for (i = 0 ; i < numCollections ; i++)
    {
        alreadyProcessedMap[i] = false;
    }

    int numMapsProcessed = 0;
    while (numMapsProcessed < maps.size())
    {
        int cur = maps[numMapsProcessed];
        numMapsProcessed++;
        if (alreadyProcessedMap[cur])
        {
            continue;
        }
        avtSILCollection_p coll = GetSILCollection(cur);
            
        const avtSILNamespace *nms = coll->GetSubsets();

        //
        // Add all of the maps of each of the subsets.
        //
        const vector<int> subsets = nms->GetAllElements();
        for (i = 0 ; i < subsets.size() ; i++)
        {
            isOn[subsets[i]] = true;
            avtSILSet_p set = GetSILSet(subsets[i]);
            const vector<int> &subsetMap = set->GetMapsOut();
            for (j = 0 ; j < subsetMap.size() ; j++)
            {
                maps.push_back(subsetMap[j]);
            }
        }

        alreadyProcessedMap[cur] = true;
    }

    //
    // We have promised just the leaves, so while we make our return list,
    // sort out the interior nodes.
    //
    leaves.clear();
    for (i = 0 ; i < nSets ; i++)
    {
        if (isOn[i])
        {
            avtSILSet_p set = GetSILSet(i);
            const vector<int> &maps = set->GetMapsOut();
            if (maps.size() <= 0)
            {
                leaves.push_back(i);
            }
        }
    }

    visitTimer->StopTimer(timingsHandle, "Getting the leaf nodes.");
}


// ****************************************************************************
// Method: avtSILRestriction::SetFromCompatibleRestriction
//
// Purpose: 
//   Sets the current SIL restriction based on the passed in SIL restriction
//   if the two are compatible.
//
// Arguments:
//   silr : The SIL restriction that we're going to use.
//
// Returns:    Whether or not the sets were compatible.
//
// Note:       This code assumes the SIL restrictions are compatible if amoung 
//             the minimum number of leaf sets under their respective top
//             sets, the names of all leaf sets are the same.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 7 14:12:09 PST 2002
//
// Modifications:
//   
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
//    Mark C. Miller - 24Sep03, Modified to support differing number of leaf
//       Sets
//
//    Mark C. Miller, Tue Nov 18 18:58:24 PST 2003
//    Added better support for creating a compatible SIL when the two SILs
//    are not identical in structure -- mainly for time-varying SILs
//
//    Mark C. Miller, Tue Mar 16 18:23:11 PST 2004
//    Added code to test top-set names and return false if we think the SILs
//    are for totally different meshes
//
// ****************************************************************************

bool
avtSILRestriction::SetFromCompatibleRestriction(avtSILRestriction_p silr)
{
    bool compatible = false;
    int i;
    vector<int> leaves;
    vector<int> otherLeaves;

    //
    // if we think these are different meshes, do nothing & return false
    //
    string topName = GetSILSet(GetTopSet())->GetName();
    string otherTopName = silr->GetSILSet(silr->GetTopSet())->GetName();
    if (topName != otherTopName)
        return false;

    //
    // Get the leaf sets for each SIL restriction.
    //
    GetLeafSets(topSet, leaves);
    silr->GetLeafSets(silr->topSet, otherLeaves);

    //
    // If the number of leaf sets are the same, compare their names/ids 
    // Otherwise, try to match up non-domain sets (e.g. id!=-1)
    //
    if (leaves.size() == otherLeaves.size())
    {
        // confirm all leaves names/ids match
        for(i = 0; i < leaves.size(); ++i)
        {
            compatible = true;
            avtSILSet_p set1 = GetSILSet(i);
            avtSILSet_p set2 = silr->GetSILSet(i);
            if ((set1->GetName() != set2->GetName()) ||
                (set1->GetIdentifier() != set2->GetIdentifier()))
            {
                compatible = false;
                break;
            }
        }

        // copy over the useSet values
        if (compatible)
        {
            SuspendCorrectnessChecking();
            for(i = 0; i < leaves.size(); ++i)
            {
                useSet[leaves[i]] = silr->useSet[otherLeaves[i]];
            }
            EnableCorrectnessChecking();
        }

    }
    else
    {
        SuspendCorrectnessChecking();
        compatible = true;

        //
        // first, set all the leaves useSet flag to something invalid
        // so we can confirm they've all be visited later
        //
        for (i = 0; i < leaves.size(); i++)
            useSet[leaves[i]] = 0xAA;

        //
        // Now, for every non-domain set, find its equivalent in the
        // input SIL and set this set's selection similarly
        //
        for (i = 0; i < GetNumSets(); i++)
        {

           avtSILSet_p set1 = GetSILSet(i);
           if (set1->GetIdentifier() < 0)
           {
              int set2Index;

              // try to find the equivalently named
              // set in the input SIL
              TRY
              {
                  set2Index = silr->GetSetIndex(set1->GetName());
              }
              CATCH(InvalidVariableException)
              {
                  set2Index = -1;
              }
              ENDTRY

              // ok, we found it, now set the selection for this 
              if (set2Index != -1)
              {
                  if (silr->useSet[set2Index] == AllUsed)
                  {
                      TurnOnSet(i);
                  }
                  else if (silr->useSet[set2Index] == NoneUsed)
                  {
                      TurnOffSet(i);
                  }
              }
           }
        }

        //
        // ok, any sets whose useSet flag is still
        // 0xAA were never touched. We can turn 'em off,
        // turn 'em on, or return compatible = false
        // The latter is the least desireable choice.
        // We choose to turn them on.
        //
        for (i = 0; i < leaves.size(); i++)
        {
            if (useSet[leaves[i]] == 0xAA)
                useSet[leaves[i]] = AllUsed;
        }

        EnableCorrectnessChecking();

    }

    return compatible;
}
