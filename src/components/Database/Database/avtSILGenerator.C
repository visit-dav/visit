// ************************************************************************* //
//                               avtSILGenerator.C                           //
// ************************************************************************* //

#include <avtSILGenerator.h>

#include <stdio.h>

#include <avtDatabaseMetaData.h>
#include <avtSILEnumeratedNamespace.h>
#include <avtSIL.h>

#include <ImproperUseException.h>


static int GroupSorter(const void *, const void *);


using std::string;
using std::vector;


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
// ****************************************************************************

void
avtSILGenerator::CreateSIL(avtDatabaseMetaData *md, avtSIL *sil)
{
    int   i;

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
                AddGroups(sil, topIndex, mesh->numGroups, domainList,
                       mesh->groupIds, mesh->groupTitle, mesh->groupPieceName,
                       mesh->blockTitle);
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
//      top      The index of the top level set.
//      num      The number of subsets.
//      origin   The origin of the subset numbers.
//      list     A list of all of the indices for each of the subsets created.
//      names    A list of each of the block names.
//      title    The title for the domains.
//      unit     The prefix for each domain.
//
//  Notes:      This routine was originally implemented in avtGenericDatabase.
//
//  Programmer:  Hank Childs
//  Creation:    September 6, 2002
//
// ****************************************************************************

void
avtSILGenerator::AddSubsets(avtSIL *sil, int top, int num, int origin,
                               vector<int> &list, const string &title,
                               const string &unit, const vector<string> &names)
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
 
        avtSILSet_p set = new avtSILSet(name, i);
 
        int dIndex = sil->AddSubset(set);
        list.push_back(dIndex);
    }
 
    //
    // Create a namespace and a collection.  The collection owns the
    // namespace after it is registered (so no leaks).
    //
    avtSILEnumeratedNamespace *ns = new avtSILEnumeratedNamespace(list);
    avtSILCollection_p coll = new avtSILCollection(title, SIL_DOMAIN,
                                                   top, ns);
 
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
// ****************************************************************************
 
void
avtSILGenerator::AddGroups(avtSIL *sil, int top, int numGroups,
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
        sprintf(name, "%s%d", piece.c_str(), i);
 
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


