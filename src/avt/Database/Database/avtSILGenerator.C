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
//                               avtSILGenerator.C                           //
// ************************************************************************* //

#include <avtSILGenerator.h>

#include <stdio.h>

#include <avtDatabaseMetaData.h>
#include <avtSILEnumeratedNamespace.h>
#include <avtSIL.h>

#include <ImproperUseException.h>
#include <TimingsManager.h>

#include <cstring>
#include <cstdlib>

using std::string;
using std::vector;

// Prototypes
static int GroupSorter(const void *, const void *);
static SILCategoryRole CategoryFromCollectionClassName(string classStr);


// ****************************************************************************
//  Method: avtSILGenerator::CreateSIL
//
//  Purpose:
//      Creates a pretty good SIL based on avtDatabaseMetaData.
//
//  Arguments:
//      md      The database meta-data.
//      sil     The SIL to populate.
//
//  Notes:      This routine was originally implemented in avtGenericDatabase.
//
//  Programmer: Hank Childs
//  Creation:   September 6, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Dec  4 08:21:13 PST 2002
//    Make SIL matrices instead of large SIL objects.
//
//    Hank Childs, Fri Aug  1 21:38:52 PDT 2003
//    Add support for curves.
//
//    Jeremy Meredith, August 25, 2005
//    Added group origin.
//
//    Jeremy Meredith, Mon Aug 28 16:25:07 EDT 2006
//    Added scalar enumeration types.
//
//    Hank Childs, Wed Dec 19 08:39:46 PST 2007
//    Add timing information.
//
//    Dave Bremer, Tue Apr  1 16:43:16 PDT 2008
//    Passed in a flag to AddSubsets to control the use of SIL Arrays.
//    They don't perform well when used with a SIL that has many
//    collections, because when the avtSILSet is created on demand,
//    the SIL's collections need to be examined to see if they contain
//    the set, so maps out can be added to the set.  I disabled their 
//    use if the mesh has any groups, although it might be optimal to
//    allow them for small numbers of groups.
//
//    Mark C. Miller, Mon Apr 14 15:28:11 PDT 2008
//    Changed interface to enumerated scalar
// ****************************************************************************

void
avtSILGenerator::CreateSIL(avtDatabaseMetaData *md, avtSIL *sil)
{
    int t0 = visitTimer->StartTimer();
    int   i;

    // skip to create custom SIL if the md object has any SIL information
    if (md->GetNumSILs())
    {
       CreateCustomSIL(md, sil);
       return;
    }

    //
    // We will need these when we set up the matrices at the end.
    //
    vector< vector<int> > domainListList;
    vector< vector<int> > matListList;

    //
    // Add all of the meshes to the set.
    //
    int numMeshes = md->GetNumMeshes();
    for (i = 0 ; i < numMeshes ; i++)
    {
        //
        // Create the mesh and add it to the SIL.
        //
        const avtMeshMetaData *mesh = md->GetMesh(i);
        int id = (mesh->numBlocks > 1 ? -1 : 0);
        avtSILSet_p set = new avtSILSet(mesh->name, id);
        int topIndex = sil->AddWhole(set);
 
        //
        // Since we have this mesh handy, add all of the subdomains if they
        // exist.
        //
        vector<int> domainList;
        if (mesh->numBlocks > 1)
        {
            AddSubsets(sil, topIndex, mesh->numBlocks, mesh->blockOrigin,
                       domainList, mesh->blockTitle, mesh->blockPieceName,
                       mesh->blockNames, mesh->numGroups==0);
            if (mesh->numGroups > 0)
            {
                AddGroups(sil, topIndex, mesh->numGroups, mesh->groupOrigin,
                          domainList, mesh->groupIds, mesh->groupTitle,
                          mesh->groupPieceName, mesh->blockTitle);
            }
        }
        domainListList.push_back(domainList);

        //
        // Add material related sets if they exist
        //
        const avtMaterialMetaData *mat = md->GetMaterialOnMesh(mesh->name);
        vector<int> matList;
        if (mat != NULL)
        {
            int id = -1;
            if (mesh->numBlocks == 1)
            {
                id = 0;
            }
            AddMaterials(sil, topIndex, mat->name, mat->materialNames,
                         matList, id);
 
            //
            // Add the species if they exist
            //
            const avtSpeciesMetaData *spec = md->GetSpeciesOnMesh(mesh->name);
            if (spec != NULL)
            {
                if (spec->numMaterials != mat->numMaterials)
                    EXCEPTION0(ImproperUseException);
 
                AddSpecies(sil, topIndex, mat->materialNames,
                           spec->name, spec, id);
            }
        }
        matListList.push_back(matList);

        //
        // Add scalar enumerations if they exist
        //

        for (int j=0; j<md->GetNumScalars(); j++)
        {
            const avtScalarMetaData *smd = md->GetScalar(j);
            if (smd->GetEnumerationType() != avtScalarMetaData::None &&
                smd->meshName == mesh->name)
            {
                AddEnumScalars(sil, topIndex, smd);
            }
        }

        //
        // IF WE EVER WANT TO STOP USING SIL MATRICES, THE BELOW CODE CAN BE
        // USED IN LIEU OF THEM (AND COMMENT OUT THE SIL MATRIX CODE).
        //
        //  AddMaterialSubsets(sil, domainList, mesh->numBlocks,
        //                     mesh->blockOrigin, matList, mat->name,
        //                     mesh->blockNames, mat->materialNames);
        //
    }
 
    for (i = 0 ; i < md->GetNumCurves() ; i++)
    {
        const avtCurveMetaData *curve = md->GetCurve(i);
        avtSILSet_p set = new avtSILSet(curve->name, 0);
        sil->AddWhole(set);
    }
    
    //
    // Note that the SIL matrices must be added last, so we have to pull this
    // out as its own 'for' loop.
    //
    for (i = 0 ; i < numMeshes ; i++)
    {
        const avtMeshMetaData *mesh = md->GetMesh(i);
        const avtMaterialMetaData *mat = md->GetMaterialOnMesh(mesh->name);

        if (mat != NULL && mesh->numBlocks > 1)
        {
            avtSILMatrix_p matrix = new avtSILMatrix(domainListList[i],
                                                SIL_DOMAIN, mesh->blockTitle,
                                                matListList[i], SIL_MATERIAL,
                                                mat->name);
            sil->AddMatrix(matrix);
        }
    }

    visitTimer->StopTimer(t0, "SIL generator to execute");
}


// ****************************************************************************
//  Method: avtSILGenerator::AddSubsets
//
//  Purpose:
//      Adds the subsets to a whole.
//
//  Arguments:
//      sil      The sil to add these sets to.
//      parent   The index of parent of all of these subsets.
//      num      The number of subsets.
//      origin   The origin of the subset numbers.
//      list     A list of all of the indices for each of the subsets created.
//      names    A list of each of the block names.
//      title    The title for the subsets.
//      unit     The prefix for each subset name.
//      cat      The category role tag for these subsets in context of parent.
//
//  Notes:      This routine was originally implemented in avtGenericDatabase.
//
//  Programmer:  Hank Childs
//  Creation:    September 6, 2002
//
//  Modifications:
//
//     Mark C. Miller, September 3, 2003
//     Added cat argument, renamed 2nd arg to 'parent'
//
//     Mark C. Miller, September 14, 2003
//     Added onlyCreateSets argument
//
//     Dave Bremer, Mon Feb 12 17:20:43 PST 2007
//     Added support for format strings.
//
//     Dave Bremer, Wed Dec 19 12:18:03 PST 2007
//     Added code to use an avtSILArray in place of a bunch of sets.
//
//     Dave Bremer, Thu Mar 27 16:39:04 PDT 2008
//     Extended the use of avtSILArrays to the case in which the
//     domains all have explicit names.
//
//     Dave Bremer, Tue Apr  1 16:43:16 PDT 2008
//     Added a flag to AddSubsets to control the use of SIL Arrays.
//     They don't perform well when used with a SIL that has many
//     collections, because when the avtSILSet is created on demand,
//     the SIL's collections need to be examined to see if they contain
//     the set, so maps out can be added to the set.
// ****************************************************************************

void
avtSILGenerator::AddSubsets(avtSIL *sil, int parent, int num, int origin,
                            vector<int> &list, const string &title,
                            const string &unit, const vector<string> &names,
                            bool useSILArrays, SILCategoryRole cat, 
                            bool onlyCreateSets)
{
    list.reserve(list.size() + num);
    if (useSILArrays && !onlyCreateSets)
    {
        int iFirstSet = sil->GetNumSets();
        avtSILArray_p  pArray;

        if (names.size() == num)
            pArray = new avtSILArray(names, num, origin, (cat==SIL_DOMAIN),
                                     title, cat, parent);
        else
            pArray = new avtSILArray(unit,  num, origin, (cat==SIL_DOMAIN),
                                     title, cat, parent);
        sil->AddArray(pArray);

        for (int ii = 0; ii < num; ii++)
            list.push_back(iFirstSet + ii);
    }
    else
    {
        for (int i = 0 ; i < num ; i++)
        {
            char name[1024];
            if (names.size() == num)
            {
                strcpy(name, names[i].c_str());
            }
            else
            {
                if (strstr(unit.c_str(), "%") != NULL)
                    sprintf(name, unit.c_str(), i+origin);
                else
                    sprintf(name, "%s%d", unit.c_str(), i+origin);
            }
    
            // determine "identifier" for the set (only "domains" get non -1) 
            int ident = -1;
            if (cat == SIL_DOMAIN)
            ident = i; 
    
            avtSILSet_p set = new avtSILSet(name, ident);
    
            int dIndex = sil->AddSubset(set);
            list.push_back(dIndex);
        }
        if (!onlyCreateSets)
        {
            //
            // Create a namespace and a collection.  The collection owns the
            // namespace after it is registered (so no leaks).
            //
            avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(list);
            avtSILCollection_p coll = new avtSILCollection(title, cat, parent, ns);
        
            sil->AddCollection(coll);
        }
    }
}


// ****************************************************************************
//  Method: avtSILGenerator::AddGroups
//
//  Purpose:
//      Adds the groups to the SIL.  Hook them up with the domains so that
//      turning off a domain turns off all the domains underneath it.
//
//  Arguments:
//      sil      The sil to add these sets to.
//      top      The index of the top level set.
//      domList  The indices to the domains in the SIL.
//      groupIds The group index of each domain.
//      gTitle   The title of the groups.
//      piece    The name of one set in the group.
//      bTitle   The title of the blocks.
//
//  Notes:      This routine was originally implemented in avtGenericDatabase.
//
//  Programmer:  Hank Childs
//  Creation:    September 6, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 28 10:19:35 PST 2003
//    Swapped the order of a conjunctive test to prevent an ABR.
//
//    Jeremy Meredith, Wed Aug 24 12:42:53 PDT 2005
//    Added an origin.
//
//    Dave Bremer, Mon Feb 12 17:20:43 PST 2007
//    Added support for format strings.
//
// ****************************************************************************
 
void
avtSILGenerator::AddGroups(avtSIL *sil, int top, int numGroups, int origin,
                           const vector<int> &domList, const vector<int> &groupIds,
                           const string &gTitle, const string &piece,
                           const string &bTitle)
{
    int  i;
 
    //
    // Start off by adding each group to the SIL.
    //
    vector<int> groupList;
    for (i = 0 ; i < numGroups ; i++)
    {
        char name[1024];
        if (strstr(piece.c_str(), "%") != NULL)
            sprintf(name, piece.c_str(), i+origin);
        else
            sprintf(name, "%s%d", piece.c_str(), i+origin);
 
        avtSILSet_p set = new avtSILSet(name, -1);
 
        int gIndex = sil->AddSubset(set);
        groupList.push_back(gIndex);
    }
 
    //
    // Create a namespace and a collection.  The collection owns the
    // namespace after it is registered (so no leaks).
    //
    avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(groupList);
    avtSILCollection_p coll = new avtSILCollection(gTitle, SIL_BLOCK, top, ns);
    sil->AddCollection(coll);
 
    //
    // Things aren't very well sorted here -- we want all of the domains,
    // sorted by groupId.  Let's try to be efficient and use the qsort routine
    // provided by stdlib to do this.
    //
    int nDoms = domList.size();
    int *records = new int[2*nDoms];
    for (i = 0 ; i < nDoms ; i++)
    {
        records[2*i]   = i;            // The domain.
        records[2*i+1] = groupIds[i];  // The group.
    }
    qsort(records, nDoms, 2*sizeof(int), GroupSorter);
 
    //
    // Now create the collections that fall under each group.
    //
    int current = 0;
    for (i = 0 ; i < numGroups ; i++)
    {
        vector<int> thisGroupsList;
        while (current < nDoms && records[2*current+1] <= i)
        {
            if (records[2*current+1] == i)
            {
                thisGroupsList.push_back(domList[records[2*current]]);
            }
            current++;
        }
 
        if (thisGroupsList.size() > 0)
        {
            avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(
                                                               thisGroupsList);
            avtSILCollection_p coll = new avtSILCollection(bTitle, SIL_DOMAIN,
                                                           groupList[i], ns);
            sil->AddCollection(coll);
        }
    }
 
    delete [] records;
}
 
 
// ****************************************************************************
//  Method: avtSILGenerator::AddMaterials
//
//  Purpose:
//      Adds a material to the top level.
//
//  Arguments:
//      sil        The sil to add the materials to.
//      top        The index of the whole mesh.
//      name       The name of the material (ie "mat1", not "copper")
//      matnames   The name of the individual materials (ie "copper", "steel")
//      list       A place to store the subsets we have created.
//      id         An id to be used for all of the sets (-1 okay).
//
//  Notes:      This routine was originally implemented in avtGenericDatabase.
//
//  Programmer: Hank Childs
//  Creation:   September 6, 2002
//
// ****************************************************************************
 
void
avtSILGenerator::AddMaterials(avtSIL *sil, int top, const string &name,
                                 const vector<string> &matnames,
                                 vector<int> &list, int id)
{
    int numMats = matnames.size();
    for (int i = 0 ; i < numMats ; i++)
    {
        avtSILSet_p set = new avtSILSet(matnames[i], id);
        int mIndex = sil->AddSubset(set);
        list.push_back(mIndex);
    }
 
    //
    // Create a namespace and a collection.  The collection owns the namespace
    // after it is registered (so no leaks).
    //
    avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(list);
    avtSILCollection_p coll = new avtSILCollection(name, SIL_MATERIAL,
                                                   top, ns);
 
    sil->AddCollection(coll);
}
 
 
// ****************************************************************************
//  Method: avtSILGenerator::AddSpecies
//
//  Purpose:
//      Adds species to a material.
//
//  Arguments:
//      sil        The sil to add the species to.
//      top        The index of the whole
//      matnames   The names of all amterials
//      name       The name of the species object (ie "spec1", not "oxygen")
//      species    The species metadata object.
//      id         An id to be used for all of the sets (-1 okay).
//
//  Notes:      This routine was originally implemented in avtGenericDatabase.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 6, 2002
//
//  Modifications:
//    Brad Whitlock, Thu Mar 8 09:45:06 PDT 2007
//    I made it use the automatically generated avtSpeciesMetaData object.
//
// ****************************************************************************
 
void
avtSILGenerator::AddSpecies(avtSIL *sil, int top,
                               const vector<string> &matnames,
                               const string &name,
                               const avtSpeciesMetaData *species,
                               int id)
{
    vector<int> list;
    int numMats = matnames.size();
    for (int i = 0 ; i < numMats ; i++)
    {
        if(i >= 0 && i < species->GetNumSpecies())
        {
            const vector<string> &specnames = species->GetSpecies(i).speciesNames;
            int numSpecs = specnames.size();
            for (int j = 0 ; j < numSpecs; j++)
            {
                char n[1024];
                sprintf(n, "Mat %s, Spec %s", matnames[i].c_str(),
                                              specnames[j].c_str());
                avtSILSet_p set = new avtSILSet(n, id);
                int mIndex = sil->AddSubset(set);
                list.push_back(mIndex);
            }
        }
    }
 
    //
    // Create a namespace and a collection.  The collection owns the namespace
    // after it is registered (so no leaks).
    //
    avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(list);
    avtSILCollection_p coll = new avtSILCollection(name, SIL_SPECIES,
                                                   top, ns);
 
    sil->AddCollection(coll);
}


// ****************************************************************************
//  Method: avtSILGenerator::AddMaterialSubsets
//
//  Purpose:
//      Adds a layer of material-domains and connects them up with the correct
//      domain and material.
//
//  Arguments:
//      sil         The SIL to add the material-domains to.
//      domList     The list of indices for the domains.
//      blocks      The number of blocks for the domains.
//      origin      The origin of the block numbering.
//      matList     The list of indices for the materials.
//      mname       The name of the material (ie "mat1", not "copper").
//      blocknames  A list of each of the block names.
//      matnames    The names of the actual materials (ie "copper", "steel").
//
//  Notes:      This routine was originally implemented in avtGenericDatabase.
//
//  Programmer: Hank Childs
//  Creation:   September 6, 2002
//
// ****************************************************************************
 
void
avtSILGenerator::AddMaterialSubsets(avtSIL *sil, const vector<int> &domList,
                               int blocks, int origin,
                               const vector<int> &matList, const string &mname,
                               const vector<string> &blocknames,
                               const vector<string> &matnames)
{
    int   i, j;
 
    //
    // Start off by creating all of the material domains and adding them to
    // the sil.
    //
    int nmat = matnames.size();
    vector<int>  matdomlist;
    for (i = 0 ; i < blocks ; i++)
    {
        for (j = 0 ; j < nmat ; j++)
        {
            //
            // Create a domain for this domain, this material.
            //
            char matdom_name[1024];
            if (blocknames.size() == blocks)
            {
                sprintf(matdom_name, "Dom=%s,Mat=%s",
                        blocknames[i].c_str(), matnames[j].c_str());
            }
            else
            {
                sprintf(matdom_name, "Dom=%d,Mat=%s",
                        i+origin,matnames[j].c_str());
            }

            //
            // Create the set and add it to the SIL.
            //
            avtSILSet_p set = new avtSILSet(matdom_name, i);
            int mdIndex = sil->AddSubset(set);
 
            //
            // Store the index of the set so we can set up a collection later.
            //
            matdomlist.push_back(mdIndex);
        }
    }
 
    //
    // Set up all of the collections relating the domains to
    // the material-domains.
    //
    for (i = 0 ; i < blocks ; i++)
    {
        //
        // We want all of the materials on block i.
        //
        vector<int> mdlist;
        for (j = 0 ; j < nmat ; j++)
        {
            mdlist.push_back(matdomlist[i*nmat+j]);
        }
 
        //
        // Create a namespace and a collection.  The collection owns the
        // namespace after it is registered (so no leaks).
        //
        avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(mdlist);
        int dl = domList[i];
        avtSILCollection_p coll = new avtSILCollection(mname, SIL_MATERIAL,
                                                       dl, ns);
 
        sil->AddCollection(coll);
    }
 
    //
    // Set up all of the collections relating the materials to
    // the material-domains.
    //
    for (i = 0 ; i < nmat ; i++)
    {
        //
        // We want all of the blocks on material i.
        //
        vector<int> mdlist;
        for (j = 0 ; j < blocks ; j++)
        {
            mdlist.push_back(matdomlist[j*nmat+i]);
        }
 
        //
        // Create a namespace and a collection.  The collection owns the
        // namespace after it is registered (so no leaks).
        //
        avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(mdlist);
        avtSILCollection_p coll = new avtSILCollection("domains", SIL_DOMAIN,
                                                       matList[i], ns);
 
        sil->AddCollection(coll);
    }
}


// ****************************************************************************
//  Method:  avtSILGenerator::AddEnumScalarSubgraph
//
//  Purpose: Add SIL collection structure representing the graph of an
//  enumerated scalar.
//
//  Programmer:  Mark C. Miller 
//  Creation:    March 26, 2008
//
// ****************************************************************************

static void
AddEnumScalarSubgraph(avtSIL *sil,
    int silTop, int enumTop, const string enumTopName,
    const vector<int> &graphEdges, const vector<int> &setIDs)
{
    int i;
    vector<int> childSetIDs;
    vector<int> childEnumIDs;

    //
    // Find all the child sets of the given subgraphTop set.
    //
    if (enumTop == -1)
    {
        //
        // First, find all the top-level enum sets (those that do NOT appear as the
        // 'tail' of an edge). All tails are at the 'odd' indices in the edge list.
        //
        vector<bool> isTopEnum(setIDs.size(), true);
        for (i = 1; i < graphEdges.size(); i+=2)
            isTopEnum[graphEdges[i]] = false;

        for (i = 0; i < setIDs.size(); i++)
        {
            if (isTopEnum[i])
            {
                childSetIDs.push_back(setIDs[i]);
                childEnumIDs.push_back(i);
            }
        }
    }
    else
    {
        for (i = 0; i < graphEdges.size(); i+=2)
        {
            if (graphEdges[i] == enumTop)
            {
                childSetIDs.push_back(setIDs[graphEdges[i+1]]);
                childEnumIDs.push_back(graphEdges[i+1]);
            }
        }
    }

    //
    // Add this collection of children to the SIL 
    //
    if (childSetIDs.size() > 0)
    {
        avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(childSetIDs);
        avtSILCollection_p coll = new avtSILCollection(enumTopName, SIL_ENUMERATION,
                                                       silTop, ns);
        sil->AddCollection(coll);

        //
        // Recruse on the children
        //
        for (i = 0; i < childSetIDs.size(); i++)
        {
            const string name = sil->GetSILSet(childSetIDs[i])->GetName();
            AddEnumScalarSubgraph(sil, childSetIDs[i], childEnumIDs[i], name, 
                                  graphEdges, setIDs);
        }
    }
}


// ****************************************************************************
//  Method:  avtSILGenerator::AddEnumScalars
//
//  Purpose:
//    Adds collections for an enumerated scalar to the SIL.
//
//  Arguments:
//      sil        The sil to add the species to.
//      top        The index of the whole
//      smd        The meta data for the enumerated scalar
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2006
//
//  Modifications:
//
//    Mark C. Miller, Wed Mar 26 16:23:27 PDT 2008
//    Added support for enumerated scalars w/graphs
//
// ****************************************************************************
void
avtSILGenerator::AddEnumScalars(avtSIL *sil, int top,
                                const avtScalarMetaData *smd)
{
    int nEnums = smd->enumNames.size();
    vector<int> enumList;
    for (int k=0; k<nEnums; k++)
    {
        char name[1024];
        sprintf(name, "%s", smd->enumNames[k].c_str());
        avtSILSet_p set = new avtSILSet(name, -1);
        int dIndex = sil->AddSubset(set);
        enumList.push_back(dIndex);
    }

    if (smd->enumGraphEdges.size() > 0)
    {
        AddEnumScalarSubgraph(sil, top, -1, smd->name, smd->enumGraphEdges, enumList);
    }
    else
    {
        avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(enumList);
        avtSILCollection_p coll = new avtSILCollection(smd->name, SIL_ENUMERATION,
                                                       top, ns);
        sil->AddCollection(coll);
    }
}


// ****************************************************************************
//  Function: GroupSorter
//
//  Purpose:
//      This sorts records that have two entries -- a domain number and a group
//      id.  This is a routine that is then fed into qsort.  Its only value
//      added is to provide the >, <, ==.
//
//  Arguments:
//      arg1    The first record.
//      arg2    The second record.
//
//  Returns:    <0 if arg1<arg2, 0 if arg1==arg2, >0 if arg2>arg1.
//
//  Programmer: Hank Childs
//  Creation:   June 24, 2002
//
// ****************************************************************************

int
GroupSorter(const void *arg1, const void *arg2)
{
    const int *r1 = (const int *) arg1;
    const int *r2 = (const int *) arg2;

    if (r1[1] > r2[1])
    {
        return 1;
    }
    else if (r2[1] > r1[1])
    {
        return -1;
    }
    else if (r1[0] > r2[0])
    {
        return 1;
    }
    else if (r2[0] > r1[0])
    {
        return -1;
    }

    return 0;
}



// ****************************************************************************
//  Method: CreateCustomSIL
//
//  Purpose: create a custom SIL using avtSILMetaData information
//
//  Programmer: Mark C. Miller 
//  Creation:   04Sep03
//
//  Modifications:
//    Brad Whitlock, Thu Mar 8 09:50:39 PDT 2007
//    Made it use autogenerated avtSILMetaData.
//
//    Dave Bremer, Tue Apr  1 16:43:16 PDT 2008
//    Passed in a flag to AddSubsets to disable the use of SIL Arrays.
//    They don't perform well when used with a SIL that has many
//    collections, because when the avtSILSet is created on demand,
//    the SIL's collections need to be examined to see if they contain
//    the set, so maps out can be added to the set.  I disabled them here
//    defensively, because turning off SIL Arrays is better in the worst case.
// ****************************************************************************

void
avtSILGenerator::CreateCustomSIL(avtDatabaseMetaData *md, avtSIL *sil)
{
    int numSILs = md->GetNumSILs();
    for (int i = 0 ; i < numSILs; i++)
    {
        //
        // Create the top-level set and add it to the SIL.
        //
        const avtSILMetaData *thisSIL = md->GetSIL(i);
        avtSILSet_p set = new avtSILSet(thisSIL->meshName, -1);
        int topIndex = sil->AddWhole(set);

#if 0
        // figure out the class name of the storage chunk class
        int chunkClassId = thisSIL->theStorageChunkClassId;
        string storageChunkClassName =
           thisSIL->collections[chunkClassId]->classOfCollection;
#endif

        //
        // loop over all collections in avtSILMetaData and create them
        // in the avtSIL object
        //
        vector< vector<int> >  subsetListList;
        for (int j = 0; j < thisSIL->GetNumCollections(); j++)
        {
           vector<int> subsetList;
           SILCategoryRole cat;

           const avtSILCollectionMetaData &thisCollection = thisSIL->GetCollections(j);

           // skip this collection entry if its any of the pre-defined collections
           if (j == 0)
           {
              subsetListList.push_back(subsetList);
              continue;
           }

#if 0
           // determine this collection's category
           if (j == chunkClassId)
              cat = SIL_DOMAIN;
           else
#endif
              cat = CategoryFromCollectionClassName(thisCollection.GetClassName()); 
           
           if (thisCollection.GetType() == avtSILCollectionMetaData::Class)
           {
              vector<string> names;
              AddSubsets(sil, topIndex, thisCollection.collectionSize, 0, subsetList,
                         thisCollection.GetClassName(), thisCollection.defaultMemberBasename,
                         names, false, cat);
           }
           else if (thisCollection.GetType() == avtSILCollectionMetaData::PureCollection)
           {
              int mdCollIdOfParent = thisCollection.collectionIdOfParent;
              int mdParentId = thisCollection.indexOfParent;
              int parentIndex = subsetListList[mdCollIdOfParent][mdParentId];

              // build the list of SIL indices in this collection from the 
              // indices of children as they were enumerated in the plugin
              for (int k = 0; k < thisCollection.collectionSize; k++)
              {
                 int mdCollIdOfChildren = thisCollection.collectionIdOfChildren;
                 vector<int> mdIndicesOfChildren = thisCollection.indicesOfChildren;
                 int indexInSIL;
                 if (mdIndicesOfChildren.size() == 0)
                    indexInSIL = subsetListList[mdCollIdOfChildren][k];
                 else
                    indexInSIL = subsetListList[mdCollIdOfChildren][mdIndicesOfChildren[k]];
                 subsetList.push_back(indexInSIL);
              }

              //
              // Create a namespace and a collection.  The collection owns the
              // namespace after it is registered (so no leaks).
              //
              avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(subsetList);
              avtSILCollection_p coll = new avtSILCollection(thisCollection.GetClassName(),
                                                             cat, parentIndex, ns);
              sil->AddCollection(coll);
           }
           else if (thisCollection.GetType() == avtSILCollectionMetaData::CollectionAndSets)
           {
              vector<string> names;
              int mdCollIdOfParent = thisCollection.collectionIdOfParent;
              int mdParentId = thisCollection.indexOfParent;
              int parentIndex = subsetListList[mdCollIdOfParent][mdParentId];

              AddSubsets(sil, parentIndex, thisCollection.collectionSize, 0, subsetList,
                         thisCollection.GetClassName(), thisCollection.defaultMemberBasename,
                         names, false, cat);
           }
           else
           {
              EXCEPTION0(ImproperUseException);
           }

           subsetListList.push_back(subsetList);

        }
    }
}

// ****************************************************************************
//  Function: CategoryFromCollectionClassName 
//
//  Purpose: guess the intended SIL category from the collection class name.
//
//  Notes: It would be best if the creator of the SIL meta data simply told
//  us the categories. However, that would pull avtSILCollection types into
//  the plugins and is undesireable
//
//  The set of strings searched for here is based on aprior knowledge of what
//  a plugin might like to create.
//
//  Programmer: Mark C. Miller 
//  Creation:   04Sep03
//
// ****************************************************************************
SILCategoryRole
CategoryFromCollectionClassName(string classStr)
{
   if      (classStr.find("dom") != string::npos) // treat 'dom' as domain
      return SIL_DOMAIN;
   else if (classStr.find("mat") != string::npos) // treat 'mat' as material
      return SIL_MATERIAL;
   else if (classStr.find("spec") != string::npos) // treat 'spec' as species
      return SIL_SPECIES;
   else if (classStr.find("proc") != string::npos) // treat 'proc' as processor
      return SIL_PROCESSOR;
   else if (classStr.find("blo") != string::npos) // treat 'blo' as block
      return SIL_BLOCK;
   else if (classStr.find("gro") != string::npos) // treat 'gro' as block
      return SIL_BLOCK;
   else if (classStr.find("pat") != string::npos) // treat patches as domains
      return SIL_DOMAIN;
   else if (classStr.find("lev") != string::npos) // treat levels as assemblies
      return SIL_ASSEMBLY;
   else
      return SIL_USERD;
}
