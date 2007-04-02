/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                               avtSILGenerator.C                           //
// ************************************************************************* //

#include <avtSILGenerator.h>

#include <stdio.h>

#include <avtDatabaseMetaData.h>
#include <avtSILEnumeratedNamespace.h>
#include <avtSIL.h>

#include <ImproperUseException.h>

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
// ****************************************************************************

void
avtSILGenerator::CreateSIL(avtDatabaseMetaData *md, avtSIL *sil)
{
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
                       mesh->blockNames);
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
                           spec->name, spec->species, id);
            }
        }
        matListList.push_back(matList);

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
//     Mark C. Miller, 04Sep03, added cat argument, renamed 2nd arg to 'parent'
//     Mark C. Miller, 14Sep03, added onlyCreateSets argument
//
// ****************************************************************************

void
avtSILGenerator::AddSubsets(avtSIL *sil, int parent, int num, int origin,
                               vector<int> &list, const string &title,
                               const string &unit, const vector<string> &names,
                               SILCategoryRole cat, bool onlyCreateSets)
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

    // sometimes, we only want to create sets and put them in collections later
    if (onlyCreateSets)
       return;
 
    //
    // Create a namespace and a collection.  The collection owns the
    // namespace after it is registered (so no leaks).
    //
    avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(list);
    avtSILCollection_p coll = new avtSILCollection(title, cat, parent, ns);
 
    sil->AddCollection(coll);
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
//      species    The vector of individual species
//      id         An id to be used for all of the sets (-1 okay).
//
//  Notes:      This routine was originally implemented in avtGenericDatabase.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 6, 2002
//
// ****************************************************************************
 
void
avtSILGenerator::AddSpecies(avtSIL *sil, int top,
                               const vector<string> &matnames,
                               const string &name,
                               const vector<avtMatSpeciesMetaData*> &species,
                               int id)
{
    vector<int> list;
    int numMats = matnames.size();
    for (int i = 0 ; i < numMats ; i++)
    {
        const vector<string> &specnames = species[i]->speciesNames;
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
        for (int j = 0; j < thisSIL->collections.size(); j++)
        {
           vector<int> subsetList;
           SILCategoryRole cat;

           avtSILCollectionMetaData *thisCollection = thisSIL->collections[j];

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
              cat = CategoryFromCollectionClassName(thisCollection->GetClassName()); 
           
           if (thisCollection->GetType() == avtSILCollectionMetaData::Class)
           {
              vector<string> names;
              AddSubsets(sil, topIndex, thisCollection->collectionSize, 0, subsetList,
                         thisCollection->GetClassName(), thisCollection->defaultMemberBasename,
                         names, cat);
           }
           else if (thisCollection->GetType() == avtSILCollectionMetaData::PureCollection)
           {
              int mdCollIdOfParent = thisCollection->collectionIdOfParent;
              int mdParentId = thisCollection->indexOfParent;
              int parentIndex = subsetListList[mdCollIdOfParent][mdParentId];

              // build the list of SIL indices in this collection from the 
              // indices of children as they were enumerated in the plugin
              for (int k = 0; k < thisCollection->collectionSize; k++)
              {
                 int mdCollIdOfChildren = thisCollection->collectionIdOfChildren;
                 vector<int> mdIndicesOfChildren = thisCollection->indicesOfChildren;
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
              avtSILCollection_p coll = new avtSILCollection(thisCollection->GetClassName(),
                                                             cat, parentIndex, ns);
              sil->AddCollection(coll);
           }
           else if (thisCollection->GetType() == avtSILCollectionMetaData::CollectionAndSets)
           {
              vector<string> names;
              int mdCollIdOfParent = thisCollection->collectionIdOfParent;
              int mdParentId = thisCollection->indexOfParent;
              int parentIndex = subsetListList[mdCollIdOfParent][mdParentId];

              AddSubsets(sil, parentIndex, thisCollection->collectionSize, 0, subsetList,
                         thisCollection->GetClassName(), thisCollection->defaultMemberBasename,
                         names, cat);
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
