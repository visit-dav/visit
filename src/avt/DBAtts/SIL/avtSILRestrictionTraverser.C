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
//                        avtSILRestrictionTraverser.C                       //
// ************************************************************************* //

#include <avtSILRestrictionTraverser.h>

#include <algorithm>

#include <avtSILNamespace.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>


using     std::sort;
using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtSILRestrictionTraverser constructor
//
//  Programmer: Hank Childs
//  Creation:   November 22, 2002
//
// ****************************************************************************

avtSILRestrictionTraverser::avtSILRestrictionTraverser(avtSILRestriction_p s)
{
    silr = s;
    preparedForMaterialSearches = false;
}


// ****************************************************************************
//  Method: avtSILRestrictionTraverser::Equal
//
//  Purpose:
//      Checks to see if theSIL restriction and the argument are equal.
//
//  Notes:     This was not written as an operator because all of our SIL
//             restrictions are stored as reference pointers and using the
//             operators would involve ugly double dereferences.
//
//  Notes:     This is not a comprehensive Equal operator and it can be easily
//             tricked.  It is meant only for comparing restrictions when the
//             underlying SILs are equal.  It is somewhat assumed that the
//             underlying SILs are indeed equal.
//
//  Arguments:
//       silr2    The SIL restriction to compare.
//
//  Returns:      true if the two restrictions are equal, false otherwise.
//
//  Programmer:   Hank Childs
//  Creation:     July 25, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
//    Hank Childs, Fri Nov 22 14:06:36 PST 2002
//    Moved into the SIL restriction traverser class.
//
// ****************************************************************************
 
bool
avtSILRestrictionTraverser::Equal(avtSILRestriction_p silr2)
{
    int ns = silr->GetNumSets();
    if (ns != silr2->GetNumSets())
    {
        return false;
    }
 
    if (silr->topSet != silr2->topSet)
    {
        return false;
    }
 
    vector<unsigned char> &useSet1 = silr->useSet;
    vector<unsigned char> &useSet2 = silr2->useSet;

    for (int i = 0 ; i < ns ; i++)
    {
        if (useSet1[i] != useSet2[i])
        {
            return false;
        }
    }
 
    //
    // Okay, it passed our tests.  This is exhaustive if the underlying SIL is
    // the same.  Otherwise, it was hopefully caught by checking the size of
    // the sets.
    //
    return true;
}

// ****************************************************************************
//  Method:  avtSILRestrictionTraverser::GetEnumerationCount
//
//  Purpose:
//    Count the number of collections that are enumerated scalars.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2006
//
// ****************************************************************************

int
avtSILRestrictionTraverser::GetEnumerationCount()
{
    int count = 0;

    int  i;
    avtSILSet_p set = silr->GetSILSet(silr->topSet);
    const vector<int> &mapsOut = set->GetMapsOut();
 
    //
    // Identify the collection that has role of an enumeration
    //
    const vector<unsigned char> &useSet = silr->useSet;
    bool  foundOneOff = false;
    for (i = 0 ; i < mapsOut.size() ; i++)
    {
        avtSILCollection_p coll = silr->GetSILCollection(mapsOut[i]);
        if (coll->GetRole() == SIL_ENUMERATION)
        {
            count++;
        }
    }
 
    return count;
}


// ****************************************************************************
//  Method:  avtSILRestrictionTraverser::GetEnumerationCount
//
//  Purpose:
//    Count the number of collections that are enumerated scalars.
//
//  Arguments:
//    index      0 <= index < GetEnumerationCount()
//    enumList   output: boolean vector for each set's enabled state
//    name       output: name of the collection (should match scalar var)
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2006
//
// ****************************************************************************

bool
avtSILRestrictionTraverser::GetEnumeration(int index,
                                           vector<bool> &enumList,
                                           string &name)
{
    int count = 0;

    int  i, j;
    enumList.clear();
    avtSILSet_p set = silr->GetSILSet(silr->topSet);
    const vector<int> &mapsOut = set->GetMapsOut();
 
    //
    // Identify the collection that has role of an enumeration
    //
    const vector<unsigned char> &useSet = silr->useSet;
    bool  foundOneOff = false;
    for (i = 0 ; i < mapsOut.size() ; i++)
    {
        avtSILCollection_p coll = silr->GetSILCollection(mapsOut[i]);
        if (coll->GetRole() == SIL_ENUMERATION)
        {
            if (index == count)
            {
                //
                // Now that we have found the right enumeration collection,
                // look at each subset and determine if it is on or off.
                //
                name = coll->GetCategory();
                const vector<int> &setList = coll->GetSubsetList();
                enumList.resize(setList.size());
                for (j = 0 ; j < setList.size() ; j++)
                {
                    bool val = (useSet[setList[j]] != NoneUsed ? true : false);
                    enumList[j] = val;
                    if (!val)
                    {
                        foundOneOff = true;
                    }
                }
            }
            count++;
        }
    }
 
    return foundOneOff;
}


// ****************************************************************************
//  Method: avtSILRestrictionTraverser::GetSpecies
//
//  Purpose:
//      Get the list of species.
//
//  Arguments:
//      specList  A vector of bools.  Each entry is true iff the species it
//                corresponds to should be used.
//
//  Returns:      true if species selection should be performed, false
//                otherwise.
//
//  Programmer:   Hank Childs
//  Creation:     December 19, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
//    Hank Childs, Fri Nov 22 14:06:36 PST 2002
//    Moved into the SIL restriction traverser class.
//
// ****************************************************************************
 
bool
avtSILRestrictionTraverser::GetSpecies(vector<bool> &specList)
{
    int  i, j;
    specList.clear();
    avtSILSet_p set = silr->GetSILSet(silr->topSet);
    const vector<int> &mapsOut = set->GetMapsOut();
 
    //
    // Identify the collection that has role "species"
    //
    const vector<unsigned char> &useSet = silr->useSet;
    bool  foundOneOff = false;
    for (i = 0 ; i < mapsOut.size() ; i++)
    {
        avtSILCollection_p coll = silr->GetSILCollection(mapsOut[i]);
        if (coll->GetRole() == SIL_SPECIES)
        {
            //
            // Now that we have found the species collection, look at each
            // of the species subsets and determine if it is on or off.
            //
            const vector<int> &setList = coll->GetSubsetList();
            specList.resize(setList.size());
            for (j = 0 ; j < setList.size() ; j++)
            {
                bool val = (useSet[setList[j]] != NoneUsed ? true : false);
                specList[j] = val;
                if (!val)
                {
                    foundOneOff = true;
                }
            }
        }
    }
 
    return foundOneOff;
}


// ****************************************************************************
//  Method: avtSILRestrictionTraverser::GetDomainList
//
//  Purpose: Public interface to GetDomainList
//
//  Programmer: Mark C. Miller 
//  Creation:   October 15, 2003
//
// ****************************************************************************
 
void
avtSILRestrictionTraverser::GetDomainList(vector<int> &list)
{
   GetDomainList(list, false);
}

// ****************************************************************************
//  Method: avtSILRestrictionTraverser::GetDomainListAllProcs
//
//  Purpose: Public interface to GetDomainList
//
//  Programmer: Mark C. Miller 
//  Creation:   October 15, 2003
//
// ****************************************************************************
 
void
avtSILRestrictionTraverser::GetDomainListAllProcs(vector<int> &list)
{
   GetDomainList(list, true);
}

// ****************************************************************************
//  Method: avtSILRestrictionTraverser::GetDomainList
//
//  Purpose:
//      Walks through all of the sets and creates a list of the identifiers
//      whose sets have been turned on.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Sep  6 14:09:00 PDT 2001
//    Clear the list so any previous values in the list are not returned.
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
//    Hank Childs, Fri Nov 22 14:06:36 PST 2002
//    Moved into the SIL restriction traverser class.
//
//    Mark C. Miller
//    Made private, added bool for query on all procs or just owner
//
//    Dave Bremer, Fri Jan 25 13:07:02 PST 2008
//    Optimized to avoid some calls to create avtSILSets on demand, and
//    reserved some space in an array to avoid reallocating memory during
//    push_backs.
// ****************************************************************************
 
void
avtSILRestrictionTraverser::GetDomainList(vector<int> &list, bool allProcs)
{
    int timingsHandle = visitTimer->StartTimer();
    list.clear();
 
    int  i, j, k;
 
    //
    // Throw in the identifiers of anything we are supposed to use.  Don't
    // worry about repeats for now.
    //
    vector<int> list_with_repeats;
    vector<int> setList;
    setList.push_back(silr->topSet);
 
    for (i = 0 ; i < setList.size() ; i++)
    {
        int setid = setList[i];
        if (silr->useSet[setid] == NoneUsed)
        {
            continue;
        }

        int id = silr->GetSILSetID(setid);
        if (id >= 0)
        {
            if (!allProcs)
            {
                if ((silr->useSet[setid] == AllUsed) ||
                    (silr->useSet[setid] == SomeUsed))
                {
                    // If a set has an identifier, all subsets must have the same
                    // identifier.
                    list_with_repeats.push_back(id);
                }
            }
            else
            {
                if ((silr->useSet[setid] == AllUsed) ||
                    (silr->useSet[setid] == AllUsedOtherProc) ||
                    (silr->useSet[setid] == SomeUsed))
                {
                    // If a set has an identifier, all subsets must have the same
                    // identifier.
                    list_with_repeats.push_back(id);
                }
            }
        }
        else
        {
            avtSILSet_p set = silr->GetSILSet(setid);
            // If a collection is part of a matrix, all of the subsets must
            // have the same identifier, so we don't need to consider them.
            const vector<int> &outmaps = set->GetRealMapsOut();
            for (j = 0 ; j < outmaps.size() ; j++)
            {
                avtSILCollection_p coll = silr->GetSILCollection(outmaps[j]);
                const vector<int> &subsets =
                                          coll->GetSubsets()->GetAllElements();
                setList.reserve(setList.size() + subsets.size());
                for (k = 0 ; k < subsets.size() ; k++)
                {
                    setList.push_back(subsets[k]);
                }
            }
        }
    }
 
    //
    // Now sort out the repeats.
    //
    sort(list_with_repeats.begin(), list_with_repeats.end());
    int last = -1;  // -1 is guaranteed to be different than l_w_r[0].
    for (i = 0 ; i < list_with_repeats.size() ; i++)
    {
        if (list_with_repeats[i] != last)
        {
            list.push_back(list_with_repeats[i]);
            last = list_with_repeats[i];
        }
    }
    visitTimer->StopTimer(timingsHandle, "Getting the domain list");
}


// ****************************************************************************
//  Method: avtSILRestrictionTraverser::UsesAllData
//
//  Purpose:
//      Determines if this SIL uses all of the data by going through the leaves
//      and making sure every one is selected.
//
//  Returns:    True if all of the data is used, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   June 17, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Sep 13 19:00:22 PDT 2001
//    Use only the leaves that are underneath this top set when trying to
//    determine if all of the data is used.  This is because the return value
//    was not accurate when there was multiple top level sets.
//
//    Hank Childs, Fri Nov 22 14:06:36 PST 2002
//    Moved into the SIL restriction traverser class.
//
//    Dave Bremer, Thu Dec 20 16:17:25 PST 2007
//    Updated to handle avtSILArrays
// ****************************************************************************
 
bool
avtSILRestrictionTraverser::UsesAllData(void)
{
    int timingsHandle = visitTimer->StartTimer();
 
    bool retval = true;
 
    vector<int> setList;
    setList.push_back(silr->topSet);
 
    const vector<unsigned char> &useSet = silr->useSet;
    for (int i = 0 ; i < setList.size() ; i++)
    {
        int setId = setList[i];
        if (!useSet[setId])
        {
            retval = false;
            break;
        }
 
        //
        // Get the value for all of its subsets.
        //
        avtSILSet_p set = silr->GetSILSet(setId);
        const vector<int> &mapsOut = set->GetMapsOut();
 
        // Loop through each collection and each set coming out of that
        // collection.
        bool subMapDoesntUseAllData = false;
        for (int j = 0; j < mapsOut.size(); j++)
        {
            int collIndex = mapsOut[j];
            avtSILArray_p  pArray = NULL;
            avtSILMatrix_p pMat = NULL;
            int newCollIndex = 0;
            avtSIL::EntryType t = silr->GetCollectionSource(collIndex, pArray, 
                                                            pMat, newCollIndex);
            if (t == avtSIL::COLLECTION)
            {
                avtSILCollection_p coll = silr->GetSILCollection(collIndex);
                const vector<int> &subsets = coll->GetSubsetList();
                for (int k = 0; k < subsets.size(); k++)
                {
                    setList.push_back(subsets[k]);
                }
            }
            else if (t == avtSIL::ARRAY)
            {
                SetState ss = pArray->GetSetState(useSet);
                if (ss != AllUsed)
                {
                    subMapDoesntUseAllData = true;
                    break;
                }
            }
            else
            {
                SetState ss = pMat->GetSetState(useSet, newCollIndex);
                if (ss != AllUsed)
                {
                    subMapDoesntUseAllData = true;
                    break;
                }
            }
        }
        if (subMapDoesntUseAllData)
        {
            retval = false;
            break;
        }
    }
 
    visitTimer->StopTimer(timingsHandle, "Testing to see if we use all the "
                                         "data.");
    return retval;
}


// ****************************************************************************
//  Method: avtSILRestrictionTraverser::UsesAllDomains
//
//  Purpose:
//      Determines if we use all of the domains.  This is used for deciding
//      whether or not we can eliminate ghost zones or not.
//
//  Returns:    true if we are using all of the domains, false otherwise.
//
//  Notes:      This routine assumes that the domain relation comes off the
//              top level set.  It is not set up for assemblies, etc.
//
//  Programmer: Hank Childs
//  Creation:   September 14, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Nov 14 15:55:49 PST 2001
//    Temporarily commented out a sanity check so that it would work with
//    the SAF database.
//
//    Hank Childs, Mon Nov 19 18:25:51 PST 2001
//    Fixed some erroneous assumptions about the layout of a SIL.  It probably
//    only worked for generic databases.
//
//    Hank Childs, Mon Jul 29 15:45:55 PDT 2002
//    Fix a stupid bug that just happened to work when there was only one mesh.
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
//    Hank Childs, Fri Nov 22 14:06:36 PST 2002
//    Moved into the SIL restriction traverser class.
//
// ****************************************************************************
 
bool
avtSILRestrictionTraverser::UsesAllDomains(void)
{
    int timingsHandle = visitTimer->StartTimer();
 
    int   i;
 
    //
    // Find the domains map coming out of the top level set.
    //
    avtSILSet_p set = silr->GetSILSet(silr->topSet);
    const vector<int> &mapsOut = set->GetMapsOut();
    avtSILCollection_p domainCollection = NULL;
    for (i = 0 ; i < mapsOut.size() ; i++)
    {
        avtSILCollection_p coll = silr->GetSILCollection(mapsOut[i]);
        if (coll->GetRole() == SIL_DOMAIN)
        {
            domainCollection = coll;
            break;
        }
    }
 
    if (*domainCollection == NULL)
    {
        //
        // There is no domain collection coming off the top level set.  This
        // probably means that we have a one domain mesh.  This logic will need
        // to be improved when we have assemblies, etc.
        //
        // Since we have only one domain, assume we are using them all.
        //
        visitTimer->StopTimer(timingsHandle, "Testing to see if we should use "
                                             "all the domains.");
        return true;
    }
 
    //
    // Determine how domains come off the top level set.  This is the maximum
    // domains possible.
    //
    const vector<int> &targetDomainList = domainCollection->GetSubsetList();
    int nTargetDomains = targetDomainList.size();

    //
    // Go through every one of the domains and see if it is being used.  If
    // not, then we have answered our question (not all of the domains are
    // are being used) and return false.
    //
    const vector<unsigned char> &useSet = silr->useSet;
    bool retval = true;
    for (i = 0 ; i < nTargetDomains ; i++)
    {
        if (useSet[targetDomainList[i]] == NoneUsed)
        {
            retval = false;
            break;
        }
    }
 
    visitTimer->StopTimer(timingsHandle, "Testing to see if we use all the "
                                         "domains.");
 
    //
    // Every domain underneath the top level set is being used in some way
    // (verified by the above for loop).  Good enough for me.
    //
    return retval;
}


// ****************************************************************************
//  Method: avtSILRestrictionTraverser::GetMaterials
//
//  Purpose:
//      Determines which materials should be selected for a specific data
//      chunk.
//
//  Arguments:
//      chunk   The chunk of interest.
//      sms     An output parameter.  True if material selection is needed,
//              false otherwise (false is for no materials, or all materials
//              are selected).
//
//  Returns:    A constant reference to the list of materials it should use.
//
//  Programmer: Hank Childs
//  Creation:   July 30, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Aug 14 09:17:29 PDT 2002
//    Re-wrote portion of routine to find leaf sets to be more accurate
//    (because it will only get the materials underneath the current top set)
//    and more accurate (since it will rule out leaf sets that do not have the
//    same chunk number more quickly).
//
//    Hank Childs, Thu Nov 21 08:13:54 PST 2002
//    Re-wrote routine to be more efficient and to work for all cases.
//
//    Hank Childs, Fri Nov 22 14:06:36 PST 2002
//    Moved into the SIL restriction traverser class.
//
// ****************************************************************************
 
const vector<string> &
avtSILRestrictionTraverser::GetMaterials(int chunk, bool &sms)
{
    static const vector<string> nomats;

    if (!preparedForMaterialSearches)
    {
        PrepareForMaterialSearches();
    }
 
    if (materialListForChunk.size() == 0)
    {
        sms = false;
        return nomats;
    }

    if (chunk < 0 || chunk >= materialListForChunk.size() ||
        materialListForChunk[chunk] < 0)
    {
        debug1 << "They have asked for a chunk (" << chunk << ") that we "
               << "have never heard of before.  Not much we can do." << endl;
        EXCEPTION0(ImproperUseException);
    }
 
    sms = shouldMatSelect[materialListForChunk[chunk]];
    return materialList[materialListForChunk[chunk]];
}


// ****************************************************************************
//  Method: avtSILRestrictionTraverser::PrepareForMaterialSearches
//
//  Purpose:
//      Find the right materials for a given chunk requires walking through
//      almost all of the SIL.  Rather than doing this repeatedly for each
//      chunk, do it once for all chunks.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Nov 22 14:06:36 PST 2002
//    Moved into the SIL restriction traverser class.
//
//    Dave Bremer, Thu Dec 20 16:17:25 PST 2007
//    Updated to handle avtSILArrays
//
//    Dave Bremer, Mon Jan 28 18:58:05 PST 2008
//    Added an early out if a given set would have no maps out.  
//    In that case it can avoid creating the set on demand.
// ****************************************************************************
 
void
avtSILRestrictionTraverser::PrepareForMaterialSearches(void)
{
    int timingsHandle = visitTimer->StartTimer();

    //TODO:  See if this increased size is a problem. -DJB
    vector<bool> setIsInProcessList(silr->GetNumSets(), false);
 
    vector<int> setsToProcess;
    setsToProcess.push_back(silr->topSet);
    setIsInProcessList[silr->topSet] = true;
 
    const vector<unsigned char> &useSet = silr->useSet;
    for (int i = 0 ; i < setsToProcess.size() ; i++)
    {
        if (useSet[setsToProcess[i]] == NoneUsed)
        {
            continue;
        }
        if (!silr->SILSetHasMapsOut(setsToProcess[i]))
        {
            continue;
        }

        avtSILSet_p currentSet = silr->GetSILSet(setsToProcess[i]);
        int chunk = currentSet->GetIdentifier();
        const vector<int> &mapsOut = currentSet->GetMapsOut();
        for (int j = 0 ; j < mapsOut.size() ; j++)
        {
            avtSILArray_p  pArray = NULL;
            avtSILMatrix_p pMat = NULL;
            int newCollIndex = 0;
            avtSIL::EntryType t = silr->GetCollectionSource(mapsOut[j], pArray, 
                                                            pMat, newCollIndex);
            if (t == avtSIL::COLLECTION || t == avtSIL::ARRAY)
            {
                avtSILCollection_p coll = silr->GetSILCollection(mapsOut[j]);
                if (coll->GetRole() == SIL_MATERIAL)
                {
                    if (chunk == -1)
                    {
                        continue;
                    }
                    else
                    {
                        const vector<int> &subsets =
                                          coll->GetSubsets()->GetAllElements();
                        int usedOne = 0;
                        int didntUseOne = 0;
                        MaterialList l;
                        for (int k = 0 ; k < subsets.size() ; k++)
                        {
                            if (useSet[subsets[k]])
                            {
                                usedOne++;
                                l.push_back(
                                       silr->GetSILSet(subsets[k])->GetName());
                            }
                            else
                            {
                                didntUseOne++;
                            }
                        }
                        bool shouldMatSel = (usedOne != 0 && didntUseOne != 0
                                             ? true : false);
                        AddMaterialList(chunk, l, shouldMatSel);
                    }
                }
                else
                {
                    const avtSILNamespace *ns = coll->GetSubsets();
                    const vector<int> &subsets = ns->GetAllElements();

                    for (int k = 0 ; k < subsets.size() ; k++)
                    {
                        if (!setIsInProcessList[subsets[k]])
                        {
                            setsToProcess.push_back(subsets[k]);
                            setIsInProcessList[subsets[k]] = true;
                        }
                    }
                }
            }
            else
            {
                if (pMat->GetRoleForCollection(newCollIndex) == SIL_MATERIAL)
                {
                    MaterialList l;
                    bool shouldMatSel = pMat->GetMaterialList(newCollIndex, l,
                                                              useSet);
                    AddMaterialList(chunk, l, shouldMatSel);
                }
            }
        }
    }
 
    preparedForMaterialSearches = true;
    visitTimer->StopTimer(timingsHandle, "Getting the material list");
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtSILRestrictionTraverser::AddMaterialList
//
//  Purpose:
//      Adds a material list to our internal data structure.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Nov 22 14:06:36 PST 2002
//    Moved into the SIL restriction traverser class.
//
// ****************************************************************************
 
void
avtSILRestrictionTraverser::AddMaterialList(int chunk, MaterialList &l,
                                            bool shouldMatSel)
{
    while (chunk >= materialListForChunk.size())
    {
        materialListForChunk.push_back(-1);
    }
    materialListForChunk[chunk] = materialList.size();
    materialList.push_back(l);
    shouldMatSelect.push_back(shouldMatSel);
}


// ****************************************************************************
//  Method: avtSILRestrictionTraverser::UsesData
//
//  Purpose:
//      Determines if a set is being used.
//
//  Arguments:
//      setId    The index of the set.
//
//  Returns:     true if that set is being used, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    July 26, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
//    Hank Childs, Fri Nov 22 14:06:36 PST 2002
//    Moved into the SIL restriction traverser class.
//
// ****************************************************************************
 
bool
avtSILRestrictionTraverser::UsesData(int setId) const
{
    int ns = silr->GetNumSets();
    if (setId < 0 || setId >= ns)
    {
        EXCEPTION2(BadIndexException, setId, ns);
    }
    return (silr->useSet[setId] != NoneUsed);
}
 
 
// ****************************************************************************
//  Method: avtSILRestrictionTraverser::UsesSetData
//
//  Purpose:
//      Returns the state of set with respect to whether it is being used.
//
//  Arguments:
//      setId    The index of the set.
//
//  Returns:     The state of the set.
//
//  Programmer:  Hank Childs
//  Creation:    July 26, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Nov 14 10:30:56 PST 2002
//    Remove access to 'sets' data member to enable SIL matrices.
//
//    Hank Childs, Fri Nov 22 14:06:36 PST 2002
//    Moved into the SIL restriction traverser class.
//
// ****************************************************************************
 
SetState
avtSILRestrictionTraverser::UsesSetData(int setId) const
{
    int ns = silr->GetNumSets();
    if (setId < 0 || setId >= ns)
    {
        EXCEPTION2(BadIndexException, setId, ns);
    }
    return (SetState) silr->useSet[setId];
}


 
// ****************************************************************************
//  Method: avtSILRestrictionTraverser::UsesAllMaterials
//
//  Purpose:
//    Determines if all materials are used. 
//
//  Returns:    True if all materials are selected or there are no materials, 
//              false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 12, 2004 
//
//  Modifications:
//
// ****************************************************************************
 
bool
avtSILRestrictionTraverser::UsesAllMaterials()
{
    if (preparedForMaterialSearches && materialListForChunk.size() == 0)
    {
        // no materials
        return true;
    }
 
    int  i, j;
    avtSILSet_p set = silr->GetSILSet(silr->topSet);
    const vector<int> &mapsOut = set->GetMapsOut();
 
    //
    // Identify the collection that has role "material"
    //
    const vector<unsigned char> &useSet = silr->useSet;
    bool  allUsed = true;
    for (i = 0 ; i < mapsOut.size() && allUsed; i++)
    {
        avtSILCollection_p coll = silr->GetSILCollection(mapsOut[i]);
        if (coll->GetRole() == SIL_MATERIAL)
        {
            //
            // Now that we have found the material collection, look at each
            // of the material subsets and determine if it is on or off.
            //
            const vector<int> &setList = coll->GetSubsetList();
            for (j = 0 ; j < setList.size() && allUsed; j++)
            {
                allUsed = (useSet[setList[j]] != NoneUsed); 
            }
        }
    }
 
    return allUsed;
}


