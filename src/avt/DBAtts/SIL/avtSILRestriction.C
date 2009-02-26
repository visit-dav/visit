/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#define STATE_INDEX(S) (((S)==SomeUsed)?1:(((S)==AllUsed)?2:((S)==SomeUsedOtherProc?3:((S)==AllUsedOtherProc?4:0))))

using  std::string;
using  std::vector;

// ****************************************************************************
// Method: VariableNamesEqual
//
// Purpose: 
//   Compares variable names and allows "/var" to be the same as "var".
//
// Arguments:
//   v1 : variable 1.
//   v2 : variable 2.
//
// Returns:    True if the variables are equal; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 29 09:37:17 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

static bool
VariableNamesEqual(const std::string &v1, const std::string &v2)
{
    bool v1BeginsWithSlash = (v1.size() >= 1) ? v1[0] == '/' : false;
    bool v2BeginsWithSlash = (v2.size() >= 1) ? v2[0] == '/' : false;

    if(v1BeginsWithSlash && v2BeginsWithSlash)
        return v1 == v2;
    else if(!v1BeginsWithSlash && !v2BeginsWithSlash)
        return v1 == v2;
    else if(v1BeginsWithSlash)
        return v1.substr(1) == v2;
    else
        return v2.substr(1) == v1;
}

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
//    Hank Childs, Tue Mar 23 07:28:08 PST 2004
//    The compact SIL atts now only has atts for the top set that we are using.
//
//    Brad Whitlock, Wed Apr 14 12:04:11 PDT 2004
//    Fixed for Windows.
//
//    Hank Childs, Sat Nov 15 18:02:46 CST 2008
//    Add special case for when all sets are on.
//
// ****************************************************************************

avtSILRestriction::avtSILRestriction(avtSIL *sil,
                                const CompactSILRestrictionAttributes &silatts)
    : avtSIL(sil)
{
    int   i;

    const vector<unsigned char> &useIt = silatts.GetUseSet();
    int ns = GetNumSets();

    useSet.reserve(ns);
    for (i = 0 ; i < ns ; i++)
    {
        useSet.push_back(NoneUsed);
    }

    topSet = -1;
    const std::string &topSetName = silatts.GetTopSet();
    if (topSetName != "")
    {
        for (i = 0 ; i < wholesList.size() ; i++)
        {
            if (GetSILSet(wholesList[i])->GetName() == topSetName)
            {
                topSet = wholesList[i];
                break;
            }
        }
    }
    else
    {
        debug1 << "Warning.  Encountered a top set that has no name.  This "
               << "is only expected to happen with legacy session files from "
               << "before version 1.3." << endl;
        for (i = 0 ; i < wholesList.size() ; i++)
        {
            vector<int> leaves;
            GetSubsets(wholesList[i], leaves);
            if (leaves.size() == useIt.size())
            {
                topSet = wholesList[i];
            }
        }
        if (topSet == -1)
            topSet = 0;
    }

    if (topSet == -1)
    {
        debug1 << "Was not able to match up " << topSetName.c_str() 
               << " with any of the existing top sets." << endl;
        EXCEPTION0(ImproperUseException);
    }

    if (wholesList.size() == 1 && silatts.GetTopSetIsAllOn())
    {
        TurnOnAll();
    }
    else
    {
        vector<int> leaves;
        GetSubsets(topSet, leaves);
    
        if (leaves.size() == useIt.size())
        {
            const SetState states[5] = {NoneUsed, SomeUsed, AllUsed, 
                                        SomeUsedOtherProc, AllUsedOtherProc};
            for (i = 0 ; i < leaves.size() ; i++)
            {
                int index = (int) useIt[i];
                useSet[leaves[i]] = states[index];
            }
        }
        else
        {
            debug1 << "The SIL from the compact SIL attributes is of a "
                   << "different size than the one on this component" << endl;
            debug1 << "Turning on all sets (what else to do?)" << endl;
            for (i = 0 ; i < leaves.size() ; i++)
            {
                useSet[leaves[i]] = AllUsed;
            }
        }
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
//    Cyrus Harrison, Tue Oct 28 14:00:39 PDT 2008
//    Changed to call higher level EnsureRestrictionCorrectness().
//
// ****************************************************************************

void
avtSILRestriction::EnableCorrectnessChecking(void)
{
    suspendCorrectnessChecking = false;
    int timingsHandle = visitTimer->StartTimer();
    EnsureRestrictionCorrectness();
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
//    Hank Childs, Mon Jun 27 16:08:09 PDT 2005
//    If we are just re-setting the current top set, treat it as a no-op.
//
//    Dave Bremer, Thu Dec 20 16:17:25 PST 2007
//    Rewrote a bit of code, because the isWhole array was eliminated.
//
//    Cyrus Harrison, Tue Oct 28 14:00:39 PDT 2008
//    Changed to call higher level EnsureRestrictionCorrectness().
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
        EXCEPTION2(BadIndexException, ts, ns);
    }

    int i, found = 0;
    for (i = 0 ; i < wholesList.size() && !found; i++)
    {
        if (wholesList[i] == ts)
            found = 1;
    }
    if (!found)
    {
        EXCEPTION0(ImproperUseException);
    }

    //
    // Do we already have this value?
    //
    if (topSet == ts)
        return;

    topSet = ts;

    //
    // Turn off the other whole sets that were not selected as "top".
    //
    int listSize = wholesList.size();
    for (i = 0 ; i < listSize ; i++)
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
        EnsureRestrictionCorrectness();
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
//    Brad Whitlock, Fri Apr 29 09:39:48 PDT 2005
//    Made it use VariableNamesEqual instead of std::string operator ==.
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
        if (VariableNamesEqual(meshname, str))
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
//   the SIL restriction - It calls the recursive analog to propagate the 
//   correct SIL state. It also ensures enumeration and material selection 
//   are mutually exclusive operations.
// 
// Notes: 
//   The enumeration/material selection enforcement feels a bit to specific 
//   to live here but without an advanced way to make these selections mutally
//   exclusive via the SIL API I feel this is our best course of action.
// 
// Programmer: Cyrus Harrison
// Creation:   Tuesday October 28, 2008
//
// Modifications:
//   
//    Mark C. Miller, Wed Feb 25 17:10:12 PST 2009
//    Fix error in indexing used to examine useSet state of material sets
// ****************************************************************************

SetState
avtSILRestriction::EnsureRestrictionCorrectness()
{
    // Call workhorse to propagate the correct SIL state.
    SetState res =  EnsureRestrictionCorrectness(topSet);
    
    // Enums & Material selection do not jive, make sure if some (not all) 
    // materials are selected, any enums are fully selected.
    
    avtSILSet_p set = GetSILSet(topSet);
    const vector<int> &mapsOut = set->GetMapsOut();
 
    // see if a subset of materials are selected
    bool  some_mats = false;
    
    for (int i = 0 ; i < mapsOut.size() && ! some_mats ; i++)
    {
        avtSILCollection_p coll = GetSILCollection(mapsOut[i]);
        if (coll->GetRole() == SIL_MATERIAL)
        {
            const vector<int> &matSetList = coll->GetSubsetList();
            for (int j = 0 ; j < matSetList.size() && ! some_mats ; j++)
                if( useSet[matSetList[j]] == SomeUsed)
                    some_mats = true;
        }
    }
    
    if (some_mats)
    {
        // b/c this case only happens when some mats are selected, we
        // do not have to worry about changing the result of "res"
        // by turning on enum sets, even though the TurnBoolSet
        // calls will update the useSet vector.

        intVector enum_indices;
        for (int i = 0 ; i < mapsOut.size(); i++)
        {
            avtSILCollection_p coll = GetSILCollection(mapsOut[i]);
            if (coll->GetRole() == SIL_ENUMERATION)
            {
                // if we have an enum turn on all subsets of this collection 
                const vector<int> &subsets = coll->GetSubsetList();
                for(int j = 0; j < subsets.size(); j++)
                    TurnBoolSet(subsets[j], true);
            }
        }
    }

    return res;
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
//    Dave Bremer, Thu Dec 20 16:17:25 PST 2007
//    Updated to handle avtSILArrays
//
//    Dave Bremer, Mon Feb  4 17:23:52 PST 2008
//    Added an early-out test that can avoid creating a SIL on demand.
//
//    Hank Childs, Mon Dec  1 15:27:59 PST 2008
//    Add support for SomeUsedOtherProc.
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

    // Early out to avoid creating a set on demand.
    if (!SILSetHasMapsOut(setId))
        return retval;

    //
    // Get the value for all of its subsets.
    //
    avtSILSet_p set = GetSILSet(setId);
    const vector<int> &mapsOut = set->GetMapsOut();
    if (mapsOut.size() > 0)
    {
        int NoneUsedCount = 0;
        int SomeUsedCount = 0;
        int SomeUsedOtherProcCount = 0;
        int AllUsedCount = 0;
        int AllUsedOtherProcCount = 0;

        // Loop through each collection and each set coming out of that
        // collection.
        for (int i = 0; i < mapsOut.size(); i++)
        {
            int collIndex = mapsOut[i];
            avtSILArray_p  pArray = NULL;
            avtSILMatrix_p pMat;
            int newCollIndex = 0;
            EntryType t = GetCollectionSource(collIndex, pArray, pMat, newCollIndex);

            if (t == avtSIL::COLLECTION || t == avtSIL::ARRAY)
            {
                avtSILCollection_p coll = GetSILCollection(collIndex);
                const vector<int> &subsets = coll->GetSubsetList();
                for (int j = 0; j < subsets.size(); j++)
                {
                    SetState s = EnsureRestrictionCorrectness(subsets[j]);
                    if(s == NoneUsed)
                        ++NoneUsedCount;
                    else if(s == SomeUsed)
                        ++SomeUsedCount;
                    else if(s == SomeUsedOtherProc)
                        ++SomeUsedOtherProcCount;
                    else if(s == AllUsedOtherProc)
                        ++AllUsedOtherProcCount;
                    else
                        ++AllUsedCount;
                }
            }
            else
            {
                SetState s = pMat->GetSetState(useSet, newCollIndex);
                if(s == NoneUsed)
                    ++NoneUsedCount;
                else if(s == SomeUsed)
                    ++SomeUsedCount;
                else if(s == SomeUsedOtherProc)
                    ++SomeUsedOtherProcCount;
                else if(s == AllUsedOtherProc)
                    ++AllUsedOtherProcCount;
                else
                    ++AllUsedCount;
            }
        }

        // Based on the counts, decide which return value to use.
        if (SomeUsedCount > 0)
            retval = SomeUsed;
        else if(NoneUsedCount != 0 && AllUsedCount != 0)
            retval = SomeUsed;
        else if(NoneUsedCount != 0 && AllUsedOtherProcCount != 0)
            retval = SomeUsedOtherProc;
        else if(SomeUsedCount != 0 && SomeUsedOtherProcCount != 0)
            retval = SomeUsed;
        else if(SomeUsedCount == 0 && SomeUsedOtherProcCount != 0)
            retval = SomeUsedOtherProc;
        else if(AllUsedOtherProcCount != 0)
            retval = AllUsedOtherProc;
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
//    Cyrus Harrison, Tue Oct 28 14:00:39 PDT 2008
//    Changed to call higher level EnsureRestrictionCorrectness method.
//
// ****************************************************************************

void
avtSILRestriction::TurnOnSet(int ind)
{
    TurnBoolSet(ind, true);

    if (!suspendCorrectnessChecking)
    {
        EnsureRestrictionCorrectness();
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
//    Cyrus Harrison, Tue Oct 28 14:00:39 PDT 2008
//    Changed to call higher level EnsureRestrictionCorrectness().
//
// ****************************************************************************

void
avtSILRestriction::TurnOffSet(int ind)
{
    TurnBoolSet(ind, false);

    if (!suspendCorrectnessChecking)
    {
        EnsureRestrictionCorrectness();
    }
}


// ****************************************************************************
// Method: avtSILRestriction::ReverseSet
//
// Purpose: 
//   Reverses the on/off state all of the sets under the specified state.
//
// Arguments:
//   ind ; The index of the set whose selection we're going to reverse.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 30 14:52:58 PST 2004
//
// Modifications:
//    Cyrus Harrison, Tue Oct 28 14:00:39 PDT 2008
//    Changed to call higher level EnsureRestrictionCorrectness().
//
// ****************************************************************************

void
avtSILRestriction::ReverseSet(int ind)
{
    // Get all of the leaf sets under the specified set.
    intVector leaves;
    GetLeafSets(ind, leaves);

    // Reverse the selection of all of the subsets.
    if(leaves.size() == 0)
    {
        SetState state = (SetState)useSet[ind];
        if(state == AllUsed ||
           state == AllUsedOtherProc)
        {
            TurnOffSet(ind);
        }
        else if(state == NoneUsed)
        {
            TurnOnSet(ind);
        }
    }
    else
    {
        for(int i = 0; i < leaves.size(); ++i)
        {
            int setId = leaves[i];
            SetState state = (SetState)useSet[setId];
            if(state == AllUsed ||
                state == AllUsedOtherProc)
            {
                TurnOffSet(setId);
            }
            else if(state == NoneUsed)
            {
                TurnOnSet(setId);
            }
        }
    }

    if (!suspendCorrectnessChecking)
    {
        EnsureRestrictionCorrectness();
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
//    Cyrus Harrison, Tue Oct 28 14:00:39 PDT 2008
//    Changed to call higher level EnsureRestrictionCorrectness().
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
        EnsureRestrictionCorrectness();
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
//    Account for AllUsedOtherProc designation.
//
//    Hank Childs, Mon Dec  1 15:27:59 PST 2008
//    Add support for SomeUsedOtherProc.
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
    SetState states[] = {NoneUsed, NoneUsed, NoneUsed, NoneUsed, NoneUsed,
                         NoneUsed, SomeUsed, SomeUsed, SomeUsedOtherProc, AllUsedOtherProc,
                         NoneUsed, SomeUsed, AllUsed, SomeUsedOtherProc, AllUsedOtherProc,
                         NoneUsed, SomeUsedOtherProc, SomeUsedOtherProc, SomeUsedOtherProc,
                         NoneUsed, SomeUsedOtherProc, AllUsedOtherProc, SomeUsedOtherProc, AllUsedOtherProc };

    const int nsets = useSet.size();
    for (int i = 0 ; i < nsets ; i++)
    {
        int index = (STATE_INDEX(useSet[i]) * 5) +
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
//    Cyrus Harrison, Tue Oct 28 14:00:39 PDT 2008
//    Changed to call higher level EnsureRestrictionCorrectness().
//
//    Hank Childs, Mon Dec  1 15:27:59 PST 2008
//    Add support for SomeUsedOtherProc.
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
    SetState states[] = {NoneUsed, SomeUsed, AllUsed, SomeUsedOtherProc, AllUsedOtherProc,
                         SomeUsed, SomeUsed, AllUsed, SomeUsedOtherProc, AllUsedOtherProc,
                         AllUsed, AllUsed, AllUsed, AllUsed, AllUsedOtherProc,
                         AllUsedOtherProc, AllUsedOtherProc, AllUsedOtherProc, AllUsedOtherProc, AllUsedOtherProc };

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
        EnsureRestrictionCorrectness();
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
//    Dave Bremer, Thu Dec 20 16:17:25 PST 2007
//    Updated to handle avtSILArrays
//
//    Dave Bremer, Fri Jan 25 13:07:02 PST 2008
//    Optimized by avoiding some calls to create avtSILSets.
//
//    Mark C. Miller, Thu Oct 16 11:58:44 PDT 2008
//    I removed an optimization to return early if 'largest' is -1. This
//    can happen only if 'domains' is empty which can happen if the SIL
//    selection is such that there are fewer domains than processors. However,
//    the optimization skirted logic that effects this processor's
//    knowledge of the state of the SIL relative to other processors. For
//    example a set would never be tagged with AllUsedOtherProc. So, it
//    seems like this optimization should NEVER have been here once logic was
//    added to tag sets with AllUsedOtherProc. For certain, this fixes problems
//    with Enumeration selections (ticket 8742).
//
//    Hank Childs, Mon Dec  1 15:27:59 PST 2008
//    Add support for SomeUsedOtherProc.
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
        int id = GetSILSetID(setsToProcess[i]);
        if (id < 0)
        {
            //
            // This is a special set that contains many domains underneath it.
            // If the maps coming out of it are real, then add their subsets to
            // our list to process.  If not, then the collection goes into a
            // matrix, and we will let the 'other dimension' handle it (ie this
            // is a material and we will let the domains handle it).
            //
            avtSILSet_p currentSet = GetSILSet(setsToProcess[i]);
            const vector<int> &mapsOut = currentSet->GetMapsOut();
            for (int j = 0 ; j < mapsOut.size() ; j++)
            {
                avtSILArray_p  pArray = NULL;
                avtSILMatrix_p pDummyMat;
                int dummy = 0;
                EntryType t = GetCollectionSource(mapsOut[j], pArray, pDummyMat, dummy);

                if (t == avtSIL::COLLECTION || t == avtSIL::ARRAY)
                {
                    avtSILCollection_p coll = GetSILCollection(mapsOut[j]);
                    const vector<int> &subsets =
                                          coll->GetSubsets()->GetAllElements();
                    setsToProcess.reserve( setsToProcess.size() + subsets.size());
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
                    if (useSet[setsToTurnOff[j]] == SomeUsed) 
                        useSet[setsToTurnOff[j]] = SomeUsedOtherProc;
                    else if (useSet[setsToTurnOff[j]] == AllUsed) 
                        useSet[setsToTurnOff[j]] = AllUsedOtherProc;
                }
                else
                {
                    useSet[setsToTurnOff[j]] = NoneUsed;
                }

                //Early out, to avoid creating a lot of unneeded avtSILSets
                if (!SILSetHasMapsOut(setsToTurnOff[j]))
                    continue;

                avtSILSet_p set = GetSILSet(setsToTurnOff[j]);
                const vector<int> &mapsOut = set->GetMapsOut();
                for (int k = 0 ; k < mapsOut.size() ; k++)
                {
                    avtSILArray_p  pArray = NULL;
                    avtSILMatrix_p pMat = NULL;
                    int newCollIndex = 0;
                    EntryType t = GetCollectionSource(mapsOut[k], pArray, 
                                                      pMat, newCollIndex);
                    if (t == avtSIL::COLLECTION)
                    {
                        avtSILCollection_p coll = GetSILCollection(mapsOut[k]);
                        const vector<int> &subsets = coll->GetSubsetList();
                        for (int l = 0; l < subsets.size(); l++)
                        {
                            setsToTurnOff.push_back(subsets[l]);
                        }
                    }
                    else if (t == avtSIL::ARRAY)
                    {
                        pArray->TurnSet(useSet, NoneUsed, forLoadBalance);
                    }
                    else
                    {
                        pMat->TurnSet(useSet, newCollIndex, NoneUsed,
                                     forLoadBalance);
                    }
                }
            }
        }
    }

    EnsureRestrictionCorrectness();
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
//  Modifications:
//
//    Hank Childs, Tue Mar 23 07:23:41 PST 2004
//    Only send state for sets that fall under our top set.  Also report
//    top set by name, not index, since meshes may come and go from timestep
//    to timestep.
//
//    Hank Childs, Sat Nov 15 18:02:46 CST 2008
//    If all of the sets are on, then just set a flag and return early.
//
// ****************************************************************************

CompactSILRestrictionAttributes *
avtSILRestriction::MakeCompactAttributes(void) const
{
    CompactSILRestrictionAttributes *rv = new CompactSILRestrictionAttributes;

    rv->SetTopSet(GetSILSet(topSet)->GetName());
    if (useSet[topSet] == AllUsed && wholesList.size() == 1)
    {
        rv->SetTopSetIsAllOn(true);
        return rv;
    }

    vector<int> leaves;
    GetSubsets(topSet, leaves);
    vector<unsigned char> iUseSet;
    for (int i = 0 ; i < leaves.size() ; i++)
    {
        char  val = (char) STATE_INDEX(useSet[leaves[i]]);
        iUseSet.push_back(val);
    }
    rv->SetUseSet(iUseSet);

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
//    Mark C. Miller, September 23, 2003, 
//    Added per-set info for state of each set's selection
//
//    Hank Childs, Mon Dec  1 15:31:30 PST 2008
//    Added some new stateNames.  Print was crashing before this, because
//    a stateName was missing (out-of-bounds array access error).
//
// ****************************************************************************

void
avtSILRestriction::Print(ostream &out) const
{
    // make labels for state of each set
    static const char *stateNames[5] = {"NoneUsed", "SomeUsed", "AllUsed",
                                        "SomeUsedOtherProc", "AllUsedOtherProc"};

    std::vector< std::string > perSetInfo, dummyInfo;
    for (int i = 0 ; i < useSet.size() ; i++)
    {
        perSetInfo.push_back(stateNames[STATE_INDEX(useSet[i])]);
    }

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
//    Hank Childs, Mon Nov 19 18:18:12 PST 2001
//    Fix inefficiency.
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
// ****************************************************************************

void
avtSILRestriction::GetLeafSets(int ind, vector<int> &leaves) const
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
//  Method: avtSILRestriction::GetSubsets
//
//  Purpose:
//      Gets all of the subsets under a root node.
//
//  Arguments:
//      ind       The index of a set.  This can be any set.
//      subsets   A list of the subsets underneath that set.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2004
//
// ****************************************************************************

void
avtSILRestriction::GetSubsets(int ind, vector<int> &outsets) const
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
    // The root node is a subset of itself.
    //
    isOn[ind] = true;

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
    // "seen".  Then take their maps out and start walking down the tree.  
    // Each map of interest is added to the "maps" list.  Its a bit funny to 
    // iterate over the list we are adding to, but it works (fingerscrossed ;).
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
    // We have promised just the subsets, so while we make our return list,
    // sort out the interior nodes.
    //
    outsets.clear();
    for (i = 0 ; i < nSets ; i++)
        if (isOn[i])
            outsets.push_back(i);

    visitTimer->StopTimer(timingsHandle, "Getting the subsets of a set.");
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
//    Mark C. Miller, Fri Dec  3 00:32:57 PST 2004
//    Fixed bug where loop variable, i, was being used to GetSILSet
//    instead of leaves[i] and otherLeaves[i]
//
//    Hank Childs, Thu Nov  1 14:22:20 PDT 2007
//    If "this" had the same number of leaves as "silr", but the leaves 
//    names and IDs didn't match, then the handling for that case
//    was non-existent.  Fixed now.
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
        compatible = true;
        for (i = 0 ; i < leaves.size() && compatible ; i++)
        {
            avtSILSet_p set1 = GetSILSet(leaves[i]);
            avtSILSet_p set2 = silr->GetSILSet(otherLeaves[i]);
            if ((set1->GetName() != set2->GetName()) ||
                (set1->GetIdentifier() != set2->GetIdentifier()))
                compatible = false;
        }
    }

    if (compatible)
    {
        // copy over the useSet values
        SuspendCorrectnessChecking();
        for(i = 0; i < leaves.size(); ++i)
        {
            useSet[leaves[i]] = silr->useSet[otherLeaves[i]];
        }
        EnableCorrectnessChecking();
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

    // Note: current logic of this routine has compatible always being true.
    return compatible;
}


