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
#include <SILMatrixAttributes.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>


using std::string;
using std::vector;


// ****************************************************************************
//  Method: avtSIL constructor
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2002
//
// ****************************************************************************

avtSIL::avtSIL()
{
    haveAddedMatrices = false;
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
// ****************************************************************************

avtSIL::avtSIL(const SILAttributes &atts)
{
    haveAddedMatrices = false;

    int   i;

    //
    // Add the sets to the SIL.
    //
    int nSets = atts.GetNSets();
    const vector<string> &setNames = atts.GetSetNames();
    const vector<int>    &setIDs   = atts.GetSetIds();
    const vector<int>    &setTypes = atts.GetIsWhole();
    for (i = 0 ; i < nSets ; i++)
    {
        avtSILSet_p set = new avtSILSet(setNames[i], setIDs[i]);
        if (setTypes[i] == 0)
        {
            AddWhole(set);
        }
        else
        {
            AddSubset(set);
        }
    }

    //
    // Add the collections to the SIL.
    //
    int nColls = atts.GetNCollections();
    const vector<string> &cats = atts.GetCategory();
    const vector<int> &role = atts.GetRole();
    const vector<int> &superset = atts.GetSuperset();
    for (i = 0 ; i < nColls ; i++)
    {
        const NamespaceAttributes &nsa = atts.GetNamespaceAttributes(i);
        avtSILNamespace *ns = avtSILNamespace::GetNamespace(&nsa);
        SILCategoryRole r = (SILCategoryRole) role[i];
        avtSILCollection_p coll = new avtSILCollection(cats[i], r,
                                                       superset[i], ns);
        AddCollection(coll);
    }

    //
    // Add the matrices to the SIL.
    //
    int nMatrices = atts.GetNumSILMatrixAttributess();
    for (i = 0 ; i < nMatrices ; i++)
    {
        const SILMatrixAttributes &ma = atts.GetSILMatrixAttributes(i);
        avtSILMatrix_p matrix = new avtSILMatrix(ma);
        AddMatrix(matrix);
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
// ****************************************************************************

avtSIL::avtSIL(avtSIL *sil)
{
    haveAddedMatrices = false;
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
// ****************************************************************************

const avtSIL &
avtSIL::operator=(const avtSIL &sil)
{
    int  i;
    
    //
    // First, clear out what we have in this object.
    //
    collections.clear();
    sets.clear();
    isWhole.clear();
    wholesList.clear();
    matrices.clear();
    haveAddedMatrices = false;

    //
    // Now copy over the other SILs attributes.
    //
    for (i = 0 ; i < sil.collections.size() ; i++)
    {
        collections.push_back(sil.collections[i]);
    }
    for (i = 0 ; i < sil.sets.size() ; i++)
    {
        sets.push_back(sil.sets[i]);
    }
    for (i = 0 ; i < sil.isWhole.size() ; i++)
    {
        isWhole.push_back(sil.isWhole[i]);
    }
    for (i = 0 ; i < sil.wholesList.size() ; i++)
    {
        wholesList.push_back(sil.wholesList[i]);
    }
    for (i = 0 ; i < sil.matrices.size() ; i++)
    {
        avtSILMatrix_p new_sm = new avtSILMatrix(*(sil.matrices[i]));
        ReAddMatrix(new_sm);
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
//
//    Hank Childs, Wed Dec  4 07:44:09 PST 2002
//    Do not allow collections to be added after SIL matrices.
//
// ****************************************************************************

void
avtSIL::AddCollection(avtSILCollection_p c)
{
    if (haveAddedMatrices)
    {
        debug1 << "All normal collections must be added before SIL matrices "
               << "are added." << endl;
        EXCEPTION0(ImproperUseException);
    }
 
    //
    // Add this collection to our list of collections.
    //
    collections.push_back(c);

    //
    // We just put our collection in the vector, so our index is one less than
    // the size (ie the maximum element).
    //
    int  collIndex   = collections.size()-1;

    //
    // Check to make sure that the index of the superset for this collection
    // is valid.
    //
    int  supersetIndex = c->GetSupersetIndex();
    int  setsSize      = sets.size();
    if (supersetIndex < 0 || supersetIndex >= setsSize)
    {
        EXCEPTION2(BadIndexException, supersetIndex, setsSize);
    }

    //
    // Tell the superset that it has a map going out.
    //
    sets[supersetIndex]->AddMapOut(collIndex);

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
        int  subset = v[i];
        if (subset < 0 || subset >= setsSize)
        {
            EXCEPTION2(BadIndexException, subset, setsSize);
        }
        sets[subset]->AddMapIn(collIndex);
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
// ****************************************************************************

int
avtSIL::AddSubset(avtSILSet_p s)
{
    int index = AddSet(s);
    isWhole.push_back(false);

    return index;
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
//
//    Hank Childs, Mon May 21 09:10:46 PDT 2001
//    Update wholesList.
//
// ****************************************************************************

int
avtSIL::AddWhole(avtSILSet_p w)
{
    int index = AddSet(w);
    isWhole.push_back(true);
    wholesList.push_back(index);

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
// ****************************************************************************

int
avtSIL::AddSet(avtSILSet_p s)
{
    if (haveAddedMatrices)
    {
        debug1 << "All normal sets must be added before SIL matrices "
               << "are added." << endl;
        EXCEPTION0(ImproperUseException);
    }

    sets.push_back(s);

    //
    // 's' is the last set in 'sets', so its index is one less than the size
    // of 'sets'.
    //
    return (sets.size()-1);
}


// ****************************************************************************
//  Method: avtSIL::ReAddMatrix
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
// ****************************************************************************

void
avtSIL::ReAddMatrix(avtSILMatrix_p m)
{
    haveAddedMatrices = true;
    m->SetSIL(this);
    m->SetStartSet(GetNumSets());
    m->SetStartCollection(GetNumCollections());
    matrices.push_back(m);

    // Since we are re-adding the matrix, all of the sets already know 
    // about it, so no more work is needed.
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
// ****************************************************************************

void
avtSIL::AddMatrix(avtSILMatrix_p m)
{
    int  i;

    int coll_count = GetNumCollections();
    int matrix_index = matrices.size();

    m->SetSIL(this);
    m->SetStartSet(GetNumSets());
    m->SetStartCollection(GetNumCollections());
    matrices.push_back(m);

    const vector<int> &set1 = m->GetSet1();
    int rowsize = set1.size();
    for (i = 0 ; i < rowsize ; i++)
    {
        sets[set1[i]]->AddMatrixRow(matrix_index, i, coll_count++);
    }

    const vector<int> &set2 = m->GetSet2();
    int columnsize = set2.size();
    for (i = 0 ; i < columnsize ; i++)
    {
        sets[set2[i]]->AddMatrixColumn(matrix_index, i, coll_count++);
    }

    haveAddedMatrices = true;
}


// ****************************************************************************
//  Method: avtSIL::GetNumSets
//
//  Purpose:
//      Gets the number of sets in the SIL plus the number in the matrices.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
// ****************************************************************************

int
avtSIL::GetNumSets(void) const
{
    int nsets = sets.size();
    int nmatrix = matrices.size();
    for (int i = 0 ; i < nmatrix ; i++)
    {
        nsets += matrices[i]->GetNumSets();
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
// ****************************************************************************

avtSILSet_p
avtSIL::GetSILSet(int index) const
{
    if (index < 0)
        EXCEPTION2(BadIndexException, index, GetNumSets());

    if (index < sets.size())
    {
        return sets[index];
    }

    int tmp_index = index - sets.size();
    int nmatrices = matrices.size();
    for (int i = 0 ; i < nmatrices ; i++)
    {
         if (tmp_index < matrices[i]->GetNumSets())
         {
             return matrices[i]->GetSILSet(tmp_index);
         }
         tmp_index -= matrices[i]->GetNumSets();
    }

    EXCEPTION2(BadIndexException, index, GetNumSets());
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
// ****************************************************************************

avtSILCollection_p
avtSIL::GetSILCollection(int index) const
{
    if (index < collections.size())
    {
        return collections[index];
    }

    int tmp_index = index - collections.size();
    int nmatrices = matrices.size();
    for (int i = 0 ; i < nmatrices ; i++)
    {
         if (tmp_index < matrices[i]->GetNumCollections())
         {
             return matrices[i]->GetSILCollection(tmp_index);
         }
         tmp_index -= matrices[i]->GetNumCollections();
    }

    EXCEPTION2(BadIndexException, index, GetNumCollections());
}


// ****************************************************************************
//  Method: avtSIL::GetSetIndex
//
//  Purpose:
//      Returns the set index of a set.
//
//  Arguments:
//      name    The name of the set.
//
//  Returns:    The index of the set.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
// ****************************************************************************

int
avtSIL::GetSetIndex(std::string name) const
{
    int index = -1;
    int size = sets.size();
    for (int i = 0 ; i < size ; i++)
    {
        avtSILSet_p set = sets[i];
        if (set->GetName() == name)
        {
            index = i;
            break;
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
//      name    The category of the collection.
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
// ****************************************************************************

int
avtSIL::GetCollectionIndex(std::string name, int superset) const
{
    int index = -1;
    int size = collections.size();
    for (int i = 0 ; i < size ; i++)
    {
        avtSILCollection_p coll = collections[i];
        if (coll->GetCategory() == name && coll->GetSupersetIndex() == superset)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    return index;
}


// ****************************************************************************
//  Method: avtSIL::TranslateCollectionInfo
//
//  Purpose:
//      Translates the collection information to something relative to a SIL
//      matrix.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2002
//
// ****************************************************************************

void
avtSIL::TranslateCollectionInfo(int index, avtSILMatrix_p &mat, int &out_ind)
{
    if (index < collections.size())
    {
        EXCEPTION0(ImproperUseException);
    }

    int tmp_index = index - collections.size();
    int nmatrices = matrices.size();
    for (int i = 0 ; i < nmatrices ; i++)
    {
         if (tmp_index < matrices[i]->GetNumCollections())
         {
             mat = matrices[i];
             out_ind = tmp_index;
             return;
         }
         tmp_index -= matrices[i]->GetNumCollections();
    }

    EXCEPTION0(ImproperUseException);
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
// ****************************************************************************

SILAttributes *
avtSIL::MakeSILAttributes(void) const
{
    int   i;

    SILAttributes *rv = new SILAttributes;

    //
    // Add the sets.
    //
    int nSets = sets.size();
    rv->SetNSets(nSets);
    vector<string> names;
    vector<int>    iw;
    vector<int>    ids;
    for (i = 0 ; i < nSets ; i++)
    {
        avtSILSet_p s = sets[i];
        names.push_back(s->GetName());
        int iw_val = (isWhole[i] ? 0 : 1);
        iw.push_back(iw_val);
        ids.push_back(s->GetIdentifier());
    }
    rv->SetSetNames(names);
    rv->SetIsWhole(iw);
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
        rv->AddNamespaceAttributes(*na);
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
        rv->AddSILMatrixAttributes(*atts);
        delete atts;
    }

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
        if (isWhole[i])
        {
            out << "Whole Set ";
        }
        else
        {
            out << "Subset ";
        }
        out << i << " " << (useInfo ? perSetInfo[i].c_str() : "") << endl;
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


// ****************************************************************************
//  Method: avtSIL::GetSetIndex
//
//  Purpose:
//    Returns the set index of a set.
//
//  Arguments:
//    name      The name of the set.
//    collID    The ID of the collection for this set.
//
//  Returns:    The index of the set.
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 26, 2005 
//
// ****************************************************************************

int
avtSIL::GetSetIndex(std::string name, int collId) const
{
    int index = -1;
    int size = sets.size();
    for (int i = 0 ; i < size ; i++)
    {
        avtSILSet_p set = sets[i];
        intVector mapsIn = set->GetMapsIn();
        if (set->GetName() == name &&
            std::find(mapsIn.begin(), mapsIn.end(), collId) != mapsIn.end())
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    return index;
}


