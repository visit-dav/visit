/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                                   avtSIL.C                                //
// ************************************************************************* //

#include <avtSIL.h>

#include <algorithm>

#include <avtSILCollection.h>
#include <avtSILNamespace.h>
#include <avtSILSet.h>

#include <NamespaceAttributes.h>
#include <SILAttributes.h>
#include <SILArrayAttributes.h>
#include <SILMatrixAttributes.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <TimingsManager.h>


using std::string;
using std::vector;


// ****************************************************************************
//  Method: avtSIL constructor
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2002
//
//  Modifications:
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Removed the haveAddedMatrices flag
// ****************************************************************************

avtSIL::avtSIL()
{
}


// ****************************************************************************
//  Method: avtSIL constructor
//
//  Purpose:
//      Creates a SIL object from SILAttributes.
//
//  Arguments:
//      atts    The SIL attributes.
//
//  Programmer: Hank Childs
//  Creation:   March 29, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Jun 15 10:46:46 PDT 2001
//    Add set identifiers.
//
//    Hank Childs, Fri Nov 15 06:39:37 PST 2002
//    Add SIL matrices.
//
//    Brad Whitlock, Tue Mar 13 11:07:55 PDT 2007
//    Updated due to code generation changes.
//
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Changed to add sets, arrays, matrices, and collections in order
//
// ****************************************************************************

avtSIL::avtSIL(const SILAttributes &atts)
{
    int ii, iCurrSet = 0, iCurrMat = 0, iCurrArray = 0, iCurrCol = 0;
    const intVector      &attsOrder = atts.GetOrder();

    const vector<string> &setNames = atts.GetSetNames();
    const vector<int>    &setIDs   = atts.GetSetIds();

    const vector<string> &cats     = atts.GetCategory();
    const vector<int>    &role     = atts.GetRole();
    const vector<int>    &superset = atts.GetSuperset();

    for (ii = 0; ii < attsOrder.size(); ii++)
    {
        if ((EntryType)attsOrder[ii] == WHOLE_SET)
        {
            avtSILSet_p set = new avtSILSet(setNames[iCurrSet], setIDs[iCurrSet]);
            AddWhole(set);
            iCurrSet++;
        }
        else if ((EntryType)attsOrder[ii] == SUBSET)
        {
            avtSILSet_p set = new avtSILSet(setNames[iCurrSet], setIDs[iCurrSet]);
            AddSubset(set);
            iCurrSet++;
        }
        else if ((EntryType)attsOrder[ii] == ARRAY)
        {
            const SILArrayAttributes &a = atts.GetArrays(iCurrArray);
            avtSILArray_p arr = new avtSILArray(a);
            AddArray(arr);
            iCurrArray++;
        }
        else if ((EntryType)attsOrder[ii] == MATRIX)
        {
            const SILMatrixAttributes &ma = atts.GetMatrices(iCurrMat);
            avtSILMatrix_p matrix = new avtSILMatrix(ma);
            AddMatrix(matrix);
            iCurrMat++;
        }
        else if ((EntryType)attsOrder[ii] == COLLECTION)
        {
            const NamespaceAttributes &nsa = atts.GetNspace(iCurrCol);
            avtSILNamespace *ns = avtSILNamespace::GetNamespace(&nsa);
            SILCategoryRole r = (SILCategoryRole) role[iCurrCol];
            avtSILCollection_p coll = new avtSILCollection(cats[iCurrCol], r,
                                                           superset[iCurrCol], ns);
            AddCollection(coll);
            iCurrCol++;
        }
    }
    if (order.size() != attsOrder.size())
        EXCEPTION0(ImproperUseException);

    for (ii = 0; ii < order.size(); ii++)
    {
        if ((int)order[ii] != attsOrder[ii])
            EXCEPTION0(ImproperUseException);
    }
}


// ****************************************************************************
//  Method: avtSIL constructor
//
//  Arguments:
//      sil     The SIL to copy.
// 
//  Programmer: Hank Childs
//  Creation:   May 21, 2001
//
//  Modifications:
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Removed the haveAddedMatrices flag
// ****************************************************************************

avtSIL::avtSIL(avtSIL *sil)
{
    *this = *sil;
}


// ****************************************************************************
//  Method: avtSIL assignment operator
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Dec  5 08:40:50 PST 2002
//    Clear out the object before copying over the new attributes.
//
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Added a check for assignment to self.  Sse the assignment operator to copy
//    vectors rather than using pushbacks, for a speed gain.  Change matrix
//    handling a little to allow removal of the ReAddMatrix method.
//
// ****************************************************************************

const avtSIL &
avtSIL::operator=(const avtSIL &sil)
{
    if (this == &sil)
        return *this;
    
    int  i;
    //
    // First, clear out what we have in this object.
    //
    collections.clear();
    sets.clear();
    wholesList.clear();
    matrices.clear();
    arrays.clear();

    //
    // Now copy over the other SILs attributes.
    //
    collections = sil.collections;
    sets = sil.sets;
    wholesList = sil.wholesList;
    order = sil.order;
    arrays = sil.arrays;

    // I think we make a copy of the matrix so we can change the
    // copy of 'this' that it has.
    for (i = 0 ; i < sil.matrices.size() ; i++)
    {
        avtSILMatrix_p new_sm = new avtSILMatrix(*(sil.matrices[i]));
        new_sm->SetSIL(this);
        matrices.push_back(new_sm);
    }

    return *this;
}


// ****************************************************************************
//  Method: avtSIL::AddCollection
//
//  Purpose:
//      Adds a collection to this SIL.
//
//  Arguments:
//      c       A collection.
//
//  Notes:      Every set index in the collection must be valid in the SIL.
//  
//  Programmer: Hank Childs
//  Creation:   March 8, 2001
//
//  Modifications:
//    Hank Childs, Wed Dec  4 07:44:09 PST 2002
//    Do not allow collections to be added after SIL matrices.
//
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Rewritten to allow sets, collections, arrays, and matrices to be 
//    added in any order.
//
// ****************************************************************************

void
avtSIL::AddCollection(avtSILCollection_p c)
{
    //
    // Add this collection to our list of collections.
    //
    collections.push_back(c);
    order.push_back(COLLECTION);

    //
    // We just put our collection in the vector, so our index is one less than
    // the size (ie the maximum element).
    //
    int  collIndex = GetNumCollections() - 1;

    //
    // Check to make sure that the index of the superset for this collection
    // is valid.
    //
    int  supersetIndex = c->GetSupersetIndex();
    int  setsSize      = GetNumSets();
    if (supersetIndex < 0 || supersetIndex >= setsSize)
    {
        EXCEPTION2(BadIndexException, supersetIndex, setsSize);
    }

    //
    // Tell the superset that it has a map going out.
    //
    bool isTemp;
    GetSILSet(supersetIndex, isTemp)->AddMapOut(collIndex);
    if (isTemp)
        EXCEPTION1(ImproperUseException, 
                   "Trying to add a collection as a "
                   "child of a matrix or array");

    //
    // The namespace knows all of the subsets that for the collection.
    //
    const avtSILNamespace *ns = c->GetSubsets();
    const vector<int> &v = ns->GetAllElements();

    //
    // Tell all of the subsets in the namespace that they have a map going
    // into them.
    //
    for (int i = 0 ; i < v.size() ; i++)
    {
        int subset = v[i];
        if (subset < 0 || subset >= setsSize)
        {
            EXCEPTION2(BadIndexException, subset, setsSize);
        }
        GetSILSet(subset, isTemp)->AddMapIn(collIndex);
    }
}


// ****************************************************************************
//  Method: avtSIL::AddSubset
//
//  Purpose:
//      Adds a subset to this SIL.
//
//  Arguments:
//      s       The subset.
//
//  Returns:    The index of the set in this SIL.  This is used to set up
//              collections later.
//
//  Programmer: Hank Childs
//  Creation:   March 8, 2001
//
//  Modifications:
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Rewritten to allow sets, collections, arrays, and matrices to be 
//    added in any order.
//
// ****************************************************************************

int
avtSIL::AddSubset(avtSILSet_p s)
{
    order.push_back(SUBSET);
    return AddSet(s);
}


// ****************************************************************************
//  Method: avtSIL::AddWhole
//
//  Purpose:
//      Adds a whole to this SIL.
//
//  Arguments:
//      w       The whole.
//
//  Returns:    The index of the set in this SIL.  This is used to set up
//              collections later.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
//  Modifications:
//    Hank Childs, Mon May 21 09:10:46 PDT 2001
//    Update wholesList.
//
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Rewritten to allow sets, collections, arrays, and matrices to be 
//    added in any order.
//
// ****************************************************************************

int
avtSIL::AddWhole(avtSILSet_p w)
{
    int index = AddSet(w);
    wholesList.push_back(index);
    order.push_back(WHOLE_SET);

    return index;
}


// ****************************************************************************
//  Method: avtSIL::AddSet
//
//  Purpose:
//      Adds a set to the list of sets.
//
//  Arguments:
//      s       The set.
//
//  Returns:    The index of the set in this SIL.  This is used to set up
//              collections later.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Dec  4 07:44:09 PST 2002
//    Do not allow collections to be added after SIL matrices.
//
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Rewritten to allow sets, collections, arrays, and matrices to be 
//    added in any order.
//
// ****************************************************************************

int
avtSIL::AddSet(avtSILSet_p s)
{
    sets.push_back(s);
    return (GetNumSets()-1);
}


// ****************************************************************************
//  Method: avtSIL::AddArray
//
//  Purpose:
//      Adds a SIL array to the SIL.
//
//  Programmer: Dave Bremer
//  Creation:   Dec 18, 2007
//
//  Modifications:
//
// ****************************************************************************

void
avtSIL::AddArray(avtSILArray_p  a)
{
    int iColIndex = GetNumCollections();

    a->SetFirstSetIndex(GetNumSets());
    a->SetCollectionIndex(iColIndex);

    bool isTemp;
    GetSILSet(a->GetParent(), isTemp)->AddMapOut(iColIndex);
    if (isTemp)
        EXCEPTION1(ImproperUseException,
                   "Trying to add an array as a "
                   "child of a matrix or array set");

    arrays.push_back(a);
    order.push_back(ARRAY);
}


// ****************************************************************************
//  Method: avtSIL::AddMatrix
//
//  Purpose:
//      Adds a SIL matrix to the SIL.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Aug  6 22:22:31 PDT 2003
//    Fix some bugs with multiple SIL matrices.
//
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Rewritten to allow sets, collections, arrays, and matrices to be 
//    added in any order.
//
// ****************************************************************************

void
avtSIL::AddMatrix(avtSILMatrix_p m)
{
    int  i;

    int coll_count = GetNumCollections();
    int matrix_index = matrices.size();

    m->SetSIL(this);
    m->SetStartSet(GetNumSets());
    m->SetStartCollection(coll_count);
    matrices.push_back(m);

    const vector<int> &set1 = m->GetSet1();
    int rowsize = set1.size();

    for (i = 0 ; i < rowsize ; i++)
    {
        bool isTemp;
        avtSILSet_p pSet = GetSILSet(set1[i], isTemp);
        if (!isTemp)
            pSet->AddMatrixMapOut(coll_count);
        coll_count++;
    }

    const vector<int> &set2 = m->GetSet2();
    int columnsize = set2.size();
    for (i = 0 ; i < columnsize ; i++)
    {
        bool isTemp;
        avtSILSet_p pSet = GetSILSet(set2[i], isTemp);
        if (!isTemp)
            pSet->AddMatrixMapOut(coll_count);
        coll_count++;
    }
    
    order.push_back(MATRIX);
}


// ****************************************************************************
//  Method: avtSIL::GetNumSets
//
//  Purpose:
//      Gets the number of sets in the SIL plus the number in the matrices
//      and arrays.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
//  Modifications:
//
//    Dave Bremer, Tue Dec 18 16:21:57 PST 2007
//    Add in the sets contained in the arrays into the total.
// ****************************************************************************

int
avtSIL::GetNumSets(void) const
{
    int nsets = sets.size();
    int i;
    int nmatrix = matrices.size();
    for (i = 0 ; i < nmatrix ; i++)
    {
        nsets += matrices[i]->GetNumSets();
    }
    int narrays = arrays.size();
    for (i = 0 ; i < narrays ; i++)
    {
        nsets += arrays[i]->GetNumSets();
    }
    return nsets;
}


// ****************************************************************************
//  Method: avtSIL::GetNumCollections
//
//  Purpose:
//      Gets the number of sets in the SIL plus the number in the matrices.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
//  Modifications:
//
//    Dave Bremer, Tue Dec 18 16:21:57 PST 2007
//    Add the collections contained in the arrays to the total.
// ****************************************************************************

int
avtSIL::GetNumCollections(void) const
{
    int ncoll = collections.size();
    int nmatrix = matrices.size();
    for (int i = 0 ; i < nmatrix ; i++)
    {
        ncoll += matrices[i]->GetNumCollections();
    }
    ncoll += arrays.size();

    return ncoll;
}


// ****************************************************************************
//  Method: avtSIL::GetSILSet
//
//  Purpose:
//      Gets a set from a SIL.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Jan  3 13:23:26 PST 2005
//    Added test for negative index.
//
//    Dave Bremer, Tue Dec 18 16:24:08 PST 2007
//    Made this a stub method.
// ****************************************************************************

avtSILSet_p
avtSIL::GetSILSet(int index) const
{
    bool dummy;
    return GetSILSet(index, dummy);
}




// ****************************************************************************
//  Method: avtSIL::GetSILSet
//
//  Purpose:
//      Gets a set from a SIL.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
//  Modifications:
//    Dave Bremer, Tue Dec 18 16:24:08 PST 2007
//    Totally rewrote this because sets, matrices, and arrays can now be added
//    in any order.  Sets can be made on demand from arrays or matrices, and 
//    maps in and out may have to be added on demand.
// ****************************************************************************

avtSILSet_p
avtSIL::GetSILSet(int index, bool &isTemporary) const
{
    if (index < 0)
        EXCEPTION2(BadIndexException, index, GetNumSets());

    int tmpIndex = index, ii = 0, jj = 0, 
        iCurrSet = 0, iCurrMat = 0, iCurrArray = 0;

    for (ii = 0; ii < order.size(); ii++)
    {
        if (order[ii] == WHOLE_SET || order[ii] == SUBSET)
        {
            if (tmpIndex == 0)
            {
                isTemporary = false;
                return sets[iCurrSet];
            }
            else
            {
                tmpIndex--;
                iCurrSet++;
            }
        }
        else if (order[ii] == ARRAY)
        {
            if (tmpIndex < arrays[iCurrArray]->GetNumSets())
            {
                isTemporary = true;
                avtSILSet_p rv = arrays[iCurrArray]->GetSILSet(tmpIndex);
                AddMapsToTemporarySet(rv, index);
                return rv;
            }
            else
            {
                tmpIndex -= arrays[iCurrArray]->GetNumSets();
                iCurrArray++;
            }
        }
        else if (order[ii] == MATRIX)
        {
            if (tmpIndex < matrices[iCurrMat]->GetNumSets())
            {
                isTemporary = true;
                avtSILSet_p rv = matrices[iCurrMat]->GetSILSet(tmpIndex);
                //AddMapsToTemporarySet(rv, index);
                return rv;
            }
            else
            {
                tmpIndex -= matrices[iCurrMat]->GetNumSets();
                iCurrMat++;
            }
        }
    }
    EXCEPTION2(BadIndexException, index, GetNumSets());
}



// ****************************************************************************
//  Method: avtSIL::AddMapsToTemporarySet
//
//  Purpose:
//      SILSets generated on the fly, coming from an array or matrix, may need
//      to have maps in or out added
//
//  Programmer: Dave Bremer
//  Creation:   Tue Dec 18 17:29:13 PST 2007
//
// ****************************************************************************

void
avtSIL::AddMapsToTemporarySet(avtSILSet_p pSet, int setIndex) const
{
    //Add maps out for a SILSet created on demand, and contained by a matrix
    int ii;
    for (ii = 0; ii < matrices.size(); ii++)
    {
        int col = matrices[ii]->SetIsInCollection(setIndex);
        if (col >= 0)
        {
            pSet->AddMatrixMapOut(col);
        }
    }

    //Add maps in for a SILSet created on demand, and contained 
    //in another collection
    int iCurrCol = 0, //index of next elem in collections.
        iColID = 0,   //ID of the next collection
        iCurrMat = 0;
    for (ii = 0; ii < order.size(); ii++)
    {
        if (order[ii] == COLLECTION)
        {
            if (collections[iCurrCol]->ContainsElement(setIndex))
            {
                pSet->AddMapIn(iColID);
            }
            iCurrCol++;
            iColID++;
        }
        else if (order[ii] == ARRAY)
        {
            iColID++;
        }
        else if (order[ii] == MATRIX)
        {
            iColID += matrices[iCurrMat]->GetNumCollections();
            iCurrMat++;
        }
    }
}


// ****************************************************************************
//  Method: avtSIL::GetSILCollection
//
//  Purpose:
//      Gets a collection from a SIL.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
//  Modifications:
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Rewritten to allow sets, collections, arrays, and matrices to be 
//    added in any order.
//
// ****************************************************************************

avtSILCollection_p
avtSIL::GetSILCollection(int index) const
{
    if (index < 0)
        EXCEPTION2(BadIndexException, index, GetNumCollections());

    int tmpIndex = index, ii = 0, iCurrCol = 0, iCurrMat = 0, iCurrArray = 0;
    for (ii = 0; ii < order.size(); ii++)
    {
        if (order[ii] == COLLECTION)
        {
            if (tmpIndex == 0)
            {
                return collections[iCurrCol];
            }
            else
            {
                tmpIndex--;
                iCurrCol++;
            }
        }
        else if (order[ii] == ARRAY)
        {
            if (tmpIndex == 0)
            {
                return arrays[iCurrArray]->GetSILCollection();
            }
            else
            {
                tmpIndex--;
                iCurrArray++;
            }
        }
        else if (order[ii] == MATRIX)
        {
            if (tmpIndex < matrices[iCurrMat]->GetNumCollections())
            {
                return matrices[iCurrMat]->GetSILCollection(tmpIndex);
            }
            else
            {
                tmpIndex -= matrices[iCurrMat]->GetNumCollections();
                iCurrMat++;
            }
        }
    }
    EXCEPTION2(BadIndexException, index, GetNumCollections());
}


// ****************************************************************************
//  Method: avtSIL::GetSetIndex
//
//  Purpose:
//      Returns the set index given a set name.
//
//  Arguments:
//      name    The name of the set.
//    collID    The ID of the collection for this set, or -999 if unused.
//
//  Returns:    The index of the set.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
//  Modifications:
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Rewritten to allow sets, collections, arrays, and matrices to be 
//    added in any order.  Merged the two very similar GetSetIndex methods
//    into one in which matching collID may not be required.
// ****************************************************************************

int
avtSIL::GetSetIndex(const std::string &name, int collID) const
{
    int ii, index = -1, iTotalSets = 0, iCurrSet = 0, iCurrMat = 0, iCurrArray = 0;
    for (ii = 0; ii < order.size(); ii++)
    {
        if (order[ii] == WHOLE_SET || order[ii] == SUBSET)
        {
            if (sets[iCurrSet]->GetName() == name)
            {
                const intVector &mapsIn = sets[iCurrSet]->GetMapsIn();
                if (collID == -999 ||
                    std::find(mapsIn.begin(), mapsIn.end(), collID) != mapsIn.end())
                {
                    index = iTotalSets;
                    break;
                }
            }
            else
            {
                iTotalSets++;
                iCurrSet++;
            }
        }
        else if (order[ii] == ARRAY)
        {
            int tmpIndex = arrays[iCurrArray]->GetSetIndex(name);
            if (tmpIndex != -1)
            {
                if (collID == -999)
                {
                    index = tmpIndex;
                    break;
                }
                else
                {
                    avtSILSet_p pSet = GetSILSet(tmpIndex);

                    const intVector &mapsIn = pSet->GetMapsIn();
                    if (std::find(mapsIn.begin(), mapsIn.end(), collID) != mapsIn.end())
                    {
                        index = tmpIndex;
                        break;
                    }
                }
            }

            iTotalSets += arrays[iCurrArray]->GetNumSets();
            iCurrArray++;
        }
        else if (order[ii] == MATRIX)
        {
            iTotalSets += matrices[iCurrMat]->GetNumSets();
            iCurrMat++;
        }
    }
    if (index == -1)
    {
        EXCEPTION1(InvalidVariableException, name);
    }
    return index;
}


// ****************************************************************************
//  Method: avtSIL::GetCollectionIndex
//
//  Purpose:
//      Returns the index of a collection.
//
//  Arguments:
//      name      The category of the collection.
//      superset  The superset to which this collection should belong.
//
//  Returns:    The index of the collection.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 8, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan 26 07:44:00 PST 2006
//    Added int superset arg.
//
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Rewritten to allow sets, collections, arrays, and matrices to be 
//    added in any order.
//
// ****************************************************************************

int
avtSIL::GetCollectionIndex(std::string name, int superset) const
{
    int ii, index = -1, iTotalCols = 0, iCurrCol = 0, iCurrMat = 0, iCurrArray = 0;
    for (ii = 0; ii < order.size(); ii++)
    {
        if (order[ii] == COLLECTION)
        {
            if (collections[iCurrCol]->GetCategory() == name &&
                collections[iCurrCol]->GetSupersetIndex() == superset )
            {
                index = iTotalCols;
                break;
            }
            else
            {
                iTotalCols++;
                iCurrCol++;
            }
        }
        else if (order[ii] == ARRAY)
        {
            avtSILCollection_p  pCol = arrays[iCurrArray]->GetSILCollection();
            if (pCol->GetCategory() == name &&
                pCol->GetSupersetIndex() == superset )
            {
                index = iTotalCols;
                break;
            }
            else
            {
                iTotalCols++;
                iCurrArray++;
            }
        }
        else if (order[ii] == MATRIX)
        {
            iTotalCols += matrices[iCurrMat]->GetNumCollections();
            iCurrMat++;
        }
    }
    if (index == -1)
    {
        EXCEPTION1(InvalidVariableException, name);
    }
    return index;
}


// ****************************************************************************
//  Method: avtSIL::GetCollectionSource
//
//  Purpose:
//      Returns an int indicating whether the collection was added explicitly,
//      or implicitly through an array or matrix.  If one of the latter, 
//      either outArray or both outMatrix and outIndex will be filled in.
//      This method replaces both RealCollection and TranslateCollectionInfo.
//
//  Returns:    An enum:  COLLECTION, ARRAY, or MATRIX, and potentially fills
//              in the output vals.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 14:58:20 PST 2007
// ****************************************************************************

avtSIL::EntryType
avtSIL::GetCollectionSource(int index, 
                            avtSILArray_p  &outArray, 
                            avtSILMatrix_p &outMatrix, int &outIndex)
{
    outArray = NULL;
    outMatrix = NULL;
    outIndex = -1;

    if (index < 0)
        EXCEPTION2(BadIndexException, index, GetNumCollections());

    int tmpIndex = index, ii = 0, iCurrArray = 0, iCurrMat = 0;
    for (ii = 0; ii < order.size(); ii++)
    {
        if (order[ii] == COLLECTION)
        {
            if (tmpIndex == 0)
            {
                return COLLECTION;
            }
            else
                tmpIndex--;
        }
        else if (order[ii] == ARRAY)
        {
            if (tmpIndex == 0)
            {
                outArray = arrays[iCurrArray];
                return ARRAY;
            }
            else
            {
                iCurrArray++;
                tmpIndex--;
            }
        }
        else if (order[ii] == MATRIX)
        {
            if (tmpIndex < matrices[iCurrMat]->GetNumCollections())
            {
                outMatrix = matrices[iCurrMat];
                outIndex  = tmpIndex;
                return MATRIX;
            }
            else
            {
                tmpIndex -= matrices[iCurrMat]->GetNumCollections();
                iCurrMat++;
            }
        }
    }
    EXCEPTION2(BadIndexException, index, GetNumCollections());
}


// ****************************************************************************
//  Method: avtSIL::MakeSILAttributes
//
//  Purpose:
//      Puts the SIL in AttributeSubject form.
//
//  Programmer: Hank Childs
//  Creation:   March 29, 2001
// 
//  Modifications:
//
//    Hank Childs, Fri Jun 15 10:46:46 PDT 2001
//    Add set identifiers.
//
//    Hank Childs, Fri Nov 15 06:39:37 PST 2002
//    Add SIL matrices.
//
//    Brad Whitlock, Tue Mar 13 11:09:25 PDT 2007
//    Updated due to code generation changes.
//
//    Hank Childs, Wed Dec 19 08:39:46 PST 2007
//    Add timing information.
//
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    Added the copying of order and array data, and removed the isWhole array
//
// ****************************************************************************

SILAttributes *
avtSIL::MakeSILAttributes(void) const
{
    int t0 = visitTimer->StartTimer();
    int   i;

    SILAttributes *rv = new SILAttributes;

    vector<int>  tmpOrder(order.size());
    for (i = 0 ; i < order.size() ; i++)
        tmpOrder[i] = (int)order[i];

    rv->SetOrder(tmpOrder);

    //
    // Add the sets.
    //
    int nSets = sets.size();
    rv->SetNSets(nSets);
    vector<string> names;
    vector<int>    ids;
    for (i = 0 ; i < nSets ; i++)
    {
        avtSILSet_p s = sets[i];
        names.push_back(s->GetName());
        ids.push_back(s->GetIdentifier());
    }
    rv->SetSetNames(names);
    rv->SetSetIds(ids);

    //
    // Add the collections.
    //
    int nCols = collections.size();
    rv->SetNCollections(nCols);
    vector<string> cats;
    vector<int> roles;
    vector<int> supersets;
    for (i = 0 ; i < nCols ; i++)
    {
        avtSILCollection_p col = collections[i];
        cats.push_back(col->GetCategory());
        roles.push_back(col->GetRole());
        supersets.push_back(col->GetSupersetIndex());
        const avtSILNamespace *ns = col->GetSubsets();
        NamespaceAttributes *na = ns->GetAttributes();
        rv->AddNspace(*na);
        delete na;
    }
    rv->SetCategory(cats);
    rv->SetRole(roles);
    rv->SetSuperset(supersets);

    //
    // Add the matrices.
    //
    int nMatrices = matrices.size();
    for (i = 0 ; i < nMatrices ; i++)
    {
        SILMatrixAttributes *atts = matrices[i]->MakeAttributes();
        rv->AddMatrices(*atts);
        delete atts;
    }

    //
    // Add the arrays.
    //
    int nArrays = arrays.size();
    for (i = 0 ; i < nArrays ; i++)
    {
        SILArrayAttributes *atts = arrays[i]->MakeAttributes();
        rv->AddArrays(*atts);
        delete atts;
    }

    visitTimer->StopTimer(t0, "Setting up SIL attributes");
    return rv;
}


// ****************************************************************************
//  Method: avtSIL::Print
//
//  Purpose:
//      Prints out a SIL.  Meant for debugging purposes only.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Nov 14 16:46:58 PST 2002
//    Print out the SIL matrices as well.
//
//    Mark C. Miller, 23Sep03
//    Added additional perXXXInfo options
//
//    Brad Whitlock, Mon Oct 20 15:20:37 PST 2003
//    I made it work on Windows again.
//
//    Dave Bremer, Thu Dec 20 16:49:10 PST 2007
//    No longer printing whether a set is whole or not.
// ****************************************************************************

void
avtSIL::Print(ostream &out) const
{
    std::vector< std::string > dummyInfo;
    Print(out, dummyInfo, dummyInfo, dummyInfo);
}

void
avtSIL::Print(ostream &out,
    std::vector< std::string > perSetInfo,
    std::vector< std::string > perCollInfo,
    std::vector< std::string > perMatInfo) const
{
    int  i;
    bool useInfo;

    int nSets = sets.size();
    if (perSetInfo.size() == nSets)
        useInfo = true; 
    else
        useInfo = false;
    for (i = 0 ; i < nSets ; i++)
    {
        out << "Set" << i << " " << (useInfo ? perSetInfo[i].c_str() : "") << endl;

        avtSILSet_p s = sets[i];
        s->Print(out);
    }

    int nColls = collections.size();
    if (perCollInfo.size() == nColls)
        useInfo = true; 
    else
        useInfo = false;
    for (i = 0 ; i < nColls ; i++)
    {
        out << "Collection " << i << " " << (useInfo ? perCollInfo[i].c_str() : "") << endl;
        avtSILCollection_p c = collections[i];
        c->Print(out);
    }

    int nMats = matrices.size();
    if (perMatInfo.size() == nMats)
        useInfo = true; 
    else
        useInfo = false;
    for (i = 0 ; i < nMats ; i++)
    {
        out << "Matrix " << i << " " << (useInfo ? perMatInfo[i].c_str() : "") << endl;
        avtSILMatrix_p m = matrices[i];
        m->Print(out);
    }
}



