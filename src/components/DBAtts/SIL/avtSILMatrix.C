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
//                                  avtSILMatrix.C                           //
// ************************************************************************* //

#include <avtSILMatrix.h>

#include <stdio.h>

#include <SILMatrixAttributes.h>

#include <avtSIL.h>
#include <avtSILEnumeratedNamespace.h>

#include <DebugStream.h>


using std::string;
using std::vector;


// ****************************************************************************
//
//          set2.1   set2.2   set2.3   set2.4  .....   set2.n
//   set1.1   0         1        2        3             n-1       coll0
//   set1.2   n        n+1      n+2      n+3           2n-1       coll1
//   set1.3   2n      2n+1     2n+2     2n+3           3n-1       coll2
//    ...
//  set1.m-1  x         x        x        x            mn-1       coll(m-1)
//
//          coll(m) coll(m+1)         coll(m+3)     coll(m+n-1)
//
// ****************************************************************************


// ****************************************************************************
//  Method: avtSILMatrix constructor
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
// ****************************************************************************

avtSILMatrix::avtSILMatrix(const vector<int> &s1, SILCategoryRole r1,
                           const string &n1, const vector<int> &s2,
                           SILCategoryRole r2, const string &n2)
{
    Initialize(s1, r1, n1, s2, r2, n2);
}


// ****************************************************************************
//  Method: avtSILMatrix constructor
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2002
//
// ****************************************************************************

avtSILMatrix::avtSILMatrix(const SILMatrixAttributes &atts)
{
    Initialize(atts.GetSet1(), (SILCategoryRole) atts.GetRole1(), 
               atts.GetCategory1(), atts.GetSet2(), 
               (SILCategoryRole) atts.GetRole2(), atts.GetCategory2());
}


// ****************************************************************************
//  Method: avtSILMatrix constructor
//
//  Programmer: Hank Childs
//  Creation:   November 18, 2002
//
//  Modifications:
//    Dave Bremer, Thu Dec 20 10:31:43 PST 2007
//    Init setsStartAt and collectionsStartAt.
// ****************************************************************************

avtSILMatrix::avtSILMatrix(const avtSILMatrix *sm)
{
    Initialize(sm->set1, sm->role1, sm->category1, sm->set2, sm->role2,
               sm->category2);

    setsStartAt = sm->setsStartAt;
    collectionsStartAt = sm->collectionsStartAt;
}


// ****************************************************************************
//  Method: avtSILMatrix::Initialize
//
//  Purpose:
//      Since both the public constructors take essentially the same argument,
//      this provides a single point of source to do their initialization.
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2002
//
//  Modifications:
//    Dave Bremer, Thu Dec 20 10:31:43 PST 2007
//    Scan set1 and set2 to see if they are sequential, so that fast queries 
//    for set containment can be performed.
// ****************************************************************************

void
avtSILMatrix::Initialize(const vector<int> &s1, SILCategoryRole r1,
                         const string &n1, const vector<int> &s2,
                         SILCategoryRole r2, const string &n2)
{
    sil                = NULL;
    setsStartAt        = 0;
    collectionsStartAt = 0;

    set1        = s1;
    role1       = r1;
    category1   = n1;

    set2        = s2;
    role2       = r2;
    category2   = n2;

    int ii;
    set1IsSequential = true;
    for (ii = 1; ii < set1.size(); ii++)
    {
        if (set1[ii-1] + 1 != set1[ii])
        {
            set1IsSequential = false;
            break;
        }
    }
    set2IsSequential = true;
    for (ii = 1; ii < set2.size(); ii++)
    {
        if (set2[ii-1] + 1 != set2[ii])
        {
            set2IsSequential = false;
            break;
        }
    }
}


// ****************************************************************************
//  Method: avtSILMatrix::SetSIL
//
//  Purpose:
//      Registers a SIL with this matrix.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
// ****************************************************************************

void
avtSILMatrix::SetSIL(avtSIL *s)
{
    sil = s;
}


// ****************************************************************************
//  Method: avtSILMatrix::MakeAttributes
//
//  Purpose:
//      Makes an attribute subject that represents this object.
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2002
//
// ****************************************************************************

SILMatrixAttributes *
avtSILMatrix::MakeAttributes(void) const
{
    SILMatrixAttributes *rv = new SILMatrixAttributes;

    rv->SetSet1(set1);
    rv->SetRole1(role1);
    rv->SetCategory1(category1);
    rv->SetSet2(set2);
    rv->SetRole2(role2);
    rv->SetCategory2(category2);

    return rv;
}


// ****************************************************************************
//  Method: avtSILMatrix::Print
//
//  Purpose:
//      Prints out a description of what is in this SIL matrix.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
// ****************************************************************************

void
avtSILMatrix::Print(ostream &out) const
{
    out << "Matrix of " << category1.c_str() << " x " << category2.c_str() << endl;
    out << "Dimensions are " << set1.size() << " x " << set2.size() << endl;
    out << "Sets run between " << setsStartAt << " and " 
        << setsStartAt+set1.size()*set2.size() << endl;
}


// ****************************************************************************
//  Method: avtSILMatrix::GetSILSet
//
//  Purpose:
//      Gets a SIL set.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
// ****************************************************************************

avtSILSet_p
avtSILMatrix::GetSILSet(int index) const
{
    int row    = index / set2.size();
    int column = index % set2.size();

    avtSILSet_p s1 = sil->GetSILSet(set1[row]);
    avtSILSet_p s2 = sil->GetSILSet(set2[column]);
    char name[1024];
    sprintf(name, "%s, %s", s1->GetName().c_str(), s2->GetName().c_str());

    avtSILSet_p rv = new avtSILSet(name, s1->GetIdentifier());

    rv->AddMapIn(collectionsStartAt+row);
    rv->AddMapIn(collectionsStartAt+set1.size()+column);

    return rv;
}


// ****************************************************************************
//  Method: avtSILMatrix::GetSILCollection
//
//  Purpose:
//      Gets a SIL collection.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
// ****************************************************************************

avtSILCollection_p
avtSILMatrix::GetSILCollection(int index) const
{
    avtSILEnumeratedNamespace *ens = NULL;
    int topset = 0;
    string name;
    SILCategoryRole role;

    if (index < set1.size())
    {
        int row    = index;

        topset = set1[row];
        name = category2;
        role = role2;

        vector<int> s;
        for (int i = 0 ; i < set2.size() ; i++)
        {
            s.push_back(setsStartAt + row*set2.size() + i);
        }
        ens = new avtSILEnumeratedNamespace(s);
    }
    else
    {
        int column = index - set1.size();

        topset = set2[column];
        name = category1;
        role = role1;

        vector<int> s;
        for (int i = 0 ; i < set1.size() ; i++)
        {
            s.push_back(setsStartAt + i*set2.size() + column);
        }
        ens = new avtSILEnumeratedNamespace(s);
    }

    avtSILCollection_p rv = new avtSILCollection(name, role, topset, ens);
    return rv;
}


// ****************************************************************************
//  Method: avtSILMatrix::GetSetState
//
//  Purpose:
//      Determines what the state of each "real" set is by looking at the
//      matrix row or column underneath it.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2002
//
// ****************************************************************************

SetState
avtSILMatrix::GetSetState(const vector<unsigned char> &useSet, int index) const
{
    int NoneUsedCount = 0;
    int SomeUsedCount = 0;
    int AllUsedCount = 0;

    int set1size = set1.size();
    int set2size = set2.size();

    if (index < set1size)
    {
        int row    = index;
        for (int i = 0 ; i < set2size ; i++)
        {
            int set = setsStartAt + row*set2size + i;
            SetState s = (SetState) useSet[set];
            if (s == NoneUsed)
                NoneUsedCount++;
            else if (s == SomeUsed)
                SomeUsedCount++;
            else 
                AllUsedCount++;
        }
    }
    else
    {
        int column = index - set1size;
        for (int i = 0 ; i < set1size ; i++)
        {
            int set = setsStartAt + i*set2size + column;
            SetState s = (SetState) useSet[set];
            if (s == NoneUsed)
                NoneUsedCount++;
            else if (s == SomeUsed)
                SomeUsedCount++;
            else 
                AllUsedCount++;
        }
    }

    SetState retval;
    if (SomeUsedCount > 0)
        retval = SomeUsed;
    else if (NoneUsedCount == 0 && AllUsedCount != 0)
        retval = AllUsed;
    else if (AllUsedCount == 0 && NoneUsedCount != 0)
        retval = NoneUsed;
    else
        retval = SomeUsed;

    return retval;
}


// ****************************************************************************
//  Method: avtSILMatrix::GetMaterialList
//
//  Purpose:
//      Determines the material list underneath a domain.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2002
//
// ****************************************************************************

bool
avtSILMatrix::GetMaterialList(int index, MaterialList &matlist, 
                              const vector<unsigned char> &useSet) const
{
    int set1size = set1.size();
    int set2size = set2.size();

    int usedOne = 0;
    int didntUseOne = 0;

    if (index < set1size)
    {
        if (role2 != SIL_MATERIAL)
        {
            return false;
        }
        int row    = index;
        for (int i = 0 ; i < set2size ; i++)
        {
            int set = setsStartAt + row*set2size + i;
            if (useSet[set] != NoneUsed)
            {
                usedOne++;
                avtSILSet_p s2 = sil->GetSILSet(set2[i]);
                matlist.push_back(s2->GetName());
            }
            else
            {
                didntUseOne++;
            }
        }
    }
    else
    {
        if (role1 != SIL_MATERIAL)
        {
            return false;
        }
        int column = index - set1size;
        for (int i = 0 ; i < set1size ; i++)
        {
            int set = setsStartAt + i*set2size + column;
            if (useSet[set] != NoneUsed)
            {
                usedOne++;
                avtSILSet_p s1 = sil->GetSILSet(set1[i]);
                matlist.push_back(s1->GetName());
            }
            else
            {
                didntUseOne++;
            }
        }
    }

    return (usedOne != 0 && didntUseOne != 0 ? true : false);
}


// ****************************************************************************
//  Method: avtSILMatrix::TurnSet
//
//  Purpose:
//      Turns the sets in a SIL matrix on or off.
//
//  Programmer: Hank Childs
//  Creation:   December 4, 2002
//
//  Modifications:
// 
//    Hank Childs, Thu Nov 13 16:47:18 PST 2003
//    Added forLoadBalance argument.  This was causing a serious AMR bug 
//    with respect to properly ghosting out coarser zones in parallel.
//
// ****************************************************************************

void
avtSILMatrix::TurnSet(vector<unsigned char> &useSet, int index, 
                      SetState val, bool forLoadBalance) const
{
    int set1size = set1.size();
    int set2size = set2.size();

    if (index < set1size)
    {
        int row    = index;
        for (int i = 0 ; i < set2size ; i++)
        {
            int set = setsStartAt + row*set2size + i;
            if (forLoadBalance && (val==NoneUsed) && 
               ((useSet[set]==AllUsed) || (useSet[set]==AllUsedOtherProc)))
                useSet[set] = AllUsedOtherProc;
            else
                useSet[set] = val;
        }
    }
    else
    {
        int column = index - set1size;
        for (int i = 0 ; i < set1size ; i++)
        {
            int set = setsStartAt + i*set2size + column;
            if (forLoadBalance && (val==NoneUsed) && 
               ((useSet[set]==AllUsed) || (useSet[set]==AllUsedOtherProc)))
                useSet[set] = AllUsedOtherProc;
            else
                useSet[set] = val;
        }
    }
}


// ****************************************************************************
//  Method: avtSILMatrix::GetNumSets
//
//  Purpose:
//      Gets the number of sets in the matrix.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
// ****************************************************************************

int
avtSILMatrix::GetNumSets(void) const
{
    return set1.size()*set2.size();
}


// ****************************************************************************
//  Method: avtSILMatrix::GetNumCollections
//
//  Purpose:
//      Gets the number of collections in the matrix.
//
//  Programmer: Hank Childs
//  Creation:   November 14, 2002
//
// ****************************************************************************

int
avtSILMatrix::GetNumCollections(void) const
{
    return set1.size()+set2.size();
}




// ****************************************************************************
//  Method: avtSILMatrix::SetIsInCollection
//
//  Purpose:
//      Determines whether this set 
//
//  Returns:  A collection index corresponding to the set, or -1 if no match
//      
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 10:31:43 PST 2007
//
// ****************************************************************************
int
avtSILMatrix::SetIsInCollection(int set) const
{
    int ii;
    if (set1IsSequential)
    {
        if (set1[0] <= set && set < set1[0]+set1.size())
            return (GetStartCollection() + set - set1[0]);
    }
    else
    {
        for (ii = 0; ii < set1.size(); ii++)
        {
            if (set1[ii] == set)
                return (GetStartCollection() + ii);
        }
    }
    if (set2IsSequential)
    {
        if (set2[0] <= set && set < set2[0]+set2.size())
            return (GetStartCollection() + set1.size() + set - set2[0]);
    }
    else
    {
        for (ii = 0; ii < set2.size(); ii++)
        {
            if (set2[ii] == set)
                return (GetStartCollection() + set1.size() + ii);
        }
    }
    return -1;
}







