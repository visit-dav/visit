// ************************************************************************* //
//                              avtDataTree.C                                //
// ************************************************************************* //

#include <avtDataTree.h>

#include <float.h>

#include <vtkAppendFilter.h>
#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>

#include <avtDataRepresentation.h>
#include <avtCommonDataFunctions.h>

#include <BadIndexException.h>
#include <InvalidMergeException.h>
#include <ImproperUseException.h>
#include <NoInputException.h>
#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDataTree constructor (empty tree)
//
//  Programmer: Hank Childs
//  Creation:   July 17, 2001
//
// ****************************************************************************

avtDataTree::avtDataTree()
{
    //
    // The best way to make an empty data tree is to pretend it has children,
    // and then make those children NULL.
    //
    nChildren   = 1;
    children    = new avtDataTree_p[1];
    children[0] = NULL;
    dataRep     = NULL;
}



// ****************************************************************************
//  Method: avtDataTree constructor (leaf)
//
//  Arguments:
//     ds       The vtkDataSet that sets this avtDataTree up as a leaf.
//     index    The index of the data set.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 1, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Feb 15 10:22:12 PST 2001
//    Added check for null argument.
//
//    Hank Childs, Tue Jun  5 08:59:06 PDT 2001
//    Associate an index with the dataset.
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001 
//    Associate a label (emtpy) with the dataset.
//
// ****************************************************************************

avtDataTree::avtDataTree(vtkDataSet *ds, int index)
{
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }
    nChildren = 0;
    children  = NULL;
    std::string l;
    dataRep   = new avtDataRepresentation(ds, index, l);
}


// ****************************************************************************
//  Method: avtDataTree constructor (leaf)
//
//  Arguments:
//     ds       The vtkDataSet that sets this avtDataTree up as a leaf.
//     index    The index of the data set.
//     s        The label to associate with the data set.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 19, 2001 
//
// ****************************************************************************

avtDataTree::avtDataTree(vtkDataSet *ds, int index, string s)
{
    if (ds == NULL)
    {
        EXCEPTION0(NoInputException);
    }
    nChildren = 0;
    children  = NULL;
    dataRep   = new avtDataRepresentation(ds, index, s);
}


// ****************************************************************************
//  Method: avtDataTree constructor (leaf)
//
//  Arguments:
//    ds        The avtDataRepresentation that sets this 
//              avtDataTree up as a leaf.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 1, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 15 10:22:12 PST 2001
//    Added check for invalid argument.
//
// ****************************************************************************

avtDataTree::avtDataTree(avtDataRepresentation &ds)
{
    if (! ds.Valid())
    {
        EXCEPTION0(NoInputException);
    }
    nChildren = 0;
    children  = NULL;
    dataRep   = new avtDataRepresentation(ds);
}
 

// ****************************************************************************
//  Method: avtDataTree constructor
//
//  Arguments:
//    n       The number of children for this tree. 
//    ds      The vtkDataSets that are this tree's leaves. 
//    ind     The indices for ds.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 1, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Feb 15 10:22:12 PST 2001
//    Added check for null argument, and null children.
//
//    Hank Childs, Thu Mar  1 10:56:46 PST 2001
//    Initialize children in all cases.
//
//    Hank Childs, Tue Jun  5 08:59:06 PDT 2001
//    Associate indices with the datasets.
//
// ****************************************************************************

avtDataTree::avtDataTree(int n, vtkDataSet **ds, int *ind)
{
    if (ds == NULL || ind == NULL || n == 0)
    {
        EXCEPTION0(NoInputException);
    }
    nChildren = n;
    children = new avtDataTree_p [nChildren];
    for (int i = 0; i < nChildren; i++)
    {
        if (ds[i] != NULL) 
        {
            children[i] = new avtDataTree(ds[i], ind[i]);
        }
        else
        {
            children[i] = NULL;
        }
    }
    dataRep = NULL;
}


// ****************************************************************************
//  Method: avtDataTree constructor
//
//  Arguments:
//    n       The number of children for this tree. 
//    ds      The vtkDataSets that are this tree's leaves. 
//    ind     The indices for ds.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

avtDataTree::avtDataTree(int n, vtkDataSet **ds, vector<int> &ind)
{
    if (ds == NULL || ind.size() != n || n == 0)
    {
        EXCEPTION0(NoInputException);
    }
    nChildren = n;
    children = new avtDataTree_p [nChildren];
    for (int i = 0; i < nChildren; i++)
    {
        if (ds[i] != NULL) 
        {
            children[i] = new avtDataTree(ds[i], ind[i]);
        }
        else
        {
            children[i] = NULL;
        }
    }
    dataRep = NULL;
}

// ****************************************************************************
//  Method: avtDataTree constructor
//
//  Arguments:
//    n       The number of children for this tree. 
//    ds      The vtkDataSets that are this tree's leaves. 
//    ind     The index of the data chunk for all of the leaves.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

avtDataTree::avtDataTree(int n, vtkDataSet **ds, int ind)
{
    if (ds == NULL || n == 0)
    {
        EXCEPTION0(NoInputException);
    }
    nChildren = n;
    children = new avtDataTree_p [nChildren];
    for (int i = 0; i < nChildren; i++)
    {
        if (ds[i] != NULL) 
        {
            children[i] = new avtDataTree(ds[i], ind);
        }
        else
        {
            children[i] = NULL;
        }
    }
    dataRep = NULL;
}



// ****************************************************************************
//  Method: avtDataTree constructor
//
//  Arguments:
//    n       The number of children for this tree. 
//    ds      The vtkDataSets that are this tree's leaves. 
//    ind     The index of the data chunk for all of the leaves.
//    l       The list of labels for each of the leaves.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
//  Modifications:
// 
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001
//    Added labels to the argument list.
//
// ****************************************************************************

avtDataTree::avtDataTree(int n, vtkDataSet **ds, int ind, vector<string> &l)
{
    if (ds == NULL || n == 0)
    {
        EXCEPTION0(NoInputException);
    }
    nChildren = n;
    children = new avtDataTree_p [nChildren];
    for (int i = 0; i < nChildren; i++)
    {
        if (ds[i] != NULL) 
        {
            children[i] = new avtDataTree(ds[i], ind, l[i]);
        }
        else
        {
            children[i] = NULL;
        }
    }
    dataRep = NULL;
}


// ****************************************************************************
//  Method: avtDataTree constructor
//
//  Arguments:
//    n       The number of children for this tree. 
//    ds      The vtkDataSets that are this tree's leaves. 
//    ind     The index of the data chunk for all of the leaves.
//    l       The label for each of the leaves.
//
//  Programmer: Hank Childs
//  Creation:   March 8, 2002
//
// ****************************************************************************

avtDataTree::avtDataTree(int n, vtkDataSet **ds, int ind, string &l)
{
    if (ds == NULL || n == 0)
    {
        EXCEPTION0(NoInputException);
    }
    nChildren = n;
    children = new avtDataTree_p [nChildren];
    for (int i = 0; i < nChildren; i++)
    {
        if (ds[i] != NULL) 
        {
            children[i] = new avtDataTree(ds[i], ind, l);
        }
        else
        {
            children[i] = NULL;
        }
    }
    dataRep = NULL;
}


// ****************************************************************************
//  Method: avtDataTree constructor
//
//  Arguments:
//    n       The number of children for this tree. 
//    dom     The avtDataRepresentations that are this tree's leaves. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 1, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 15 10:22:12 PST 2001
//    Added check for null argument, and invalid dataRep in list.
//
// ****************************************************************************

avtDataTree::avtDataTree(int n, avtDataRepresentation *drep)
{
    if (drep == NULL)
    {
        EXCEPTION0(NoInputException);
    }
    nChildren = n;
    children = new avtDataTree_p [nChildren];
    for (int i = 0; i < nChildren; i++)
    {
        if (drep[i].Valid())
        {
            children[i] = new avtDataTree(drep[i]);
        }
    }
    dataRep = NULL;
}


// ****************************************************************************
//  Method: avtDataTree copy constructor
//
//  Arguments:
//    dst     The avtDataTree from which to copy. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 1, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 15 10:22:12 PST 2001
//    Added check for null argument, and invalid children in tree.
//
//    Mark C. Miller, 22Apr03
//    Added option to create a tree without the data at the leaves
//
// ****************************************************************************

avtDataTree::avtDataTree(avtDataTree_p dt, bool dontCopyData)
{

    if (*dt == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    nChildren = dt->nChildren;
    if (nChildren > 0)
    {
        children = new avtDataTree_p [nChildren];
        for (int i = 0; i < nChildren; i++)
        {
            if (dt->ChildIsPresent(i)) 
            {
                children[i] = new avtDataTree(dt->children[i], dontCopyData);
            }
        }
        dataRep = NULL;
    } 
    else
    {
        children = NULL;
        if (dontCopyData)
        {
            avtDataRepresentation& oldRep = dt->GetDataRepresentation();
            dataRep  = new avtDataRepresentation(NULL, oldRep.GetDomain(),
                                                       oldRep.GetLabel(),
                                                       dontCopyData);
        }
        else
        {
            if (dt->dataRep != NULL)
               dataRep  = new avtDataRepresentation( dt->GetDataRepresentation() );
            else
               dataRep = NULL;
        }
    }

}


// ****************************************************************************
//  Method: avtDataTree constructor
//
//  Arguments:
//    n       The number of children for this tree.
//    dst     The avtDataTrees which will be this tree's children. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 1, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 15 10:22:12 PST 2001
//    Added check for null argument, and invalid children in tree.
//
// ****************************************************************************

avtDataTree::avtDataTree(int n, avtDataTree_p *dst)
{
    if (dst == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    nChildren = n;
    children = new avtDataTree_p [nChildren];
    for (int i = 0; i < nChildren; i++)
    {
        if (*dst[i] != NULL)
        {
            children[i] = dst[i]; 
        }
    }
    dataRep = NULL;
}


// ****************************************************************************
//  Method: avtDataTree destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 1, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 15 10:22:12 PST 2001
//    Removed deletion of reference pointers. 
//
//    Hank Childs, Sun Sep 16 14:33:43 PDT 2001
//    Fix memory leak.
//
// ****************************************************************************

avtDataTree::~avtDataTree()
{
    if (dataRep != NULL)
    {
        delete dataRep;
        dataRep = NULL;
    }
    if (children != NULL)
    {
        delete [] children;
        children = NULL;
    }
}


// ****************************************************************************
//  Method: avtDataTree assignment operator
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 1, 2001 
//
//  Modifications:
//
//    Hank Childs, Thu Sep 20 12:47:36 PDT 2001
//    Properly set array to NULL when it was deleted.  One case through logic
//    allowed it the array to be deleted again in the destructor.
//
// ****************************************************************************

avtDataTree&
avtDataTree::operator=(avtDataTree &rhs)
{
    if (this != &rhs)
    {
        if (dataRep != NULL) 
        {
            delete dataRep;
            dataRep = NULL;
        }

        if (children != NULL)
        {
            for (int i = 0; i < nChildren; i++)
            {
               children[i] = NULL; 
            }
            delete [] children;
            children = NULL;
        }

        nChildren = rhs.nChildren;
        if (nChildren > 0)
        {
            children = new avtDataTree_p [nChildren];
            for (int i = 0; i < nChildren; i++)
               children[i] = rhs.children[i];
        } 
        else
        {
            dataRep = new avtDataRepresentation(*(rhs.dataRep)); 
        } 
    }
    return *this;
}



// ****************************************************************************
//  Method: avtDataTree::GetChild
//
//  Purpose:
//    Gets a child node.
//
//  Arguments:
//    n      The child node that is to be returned.
//
//  Returns:
//    The avtDataTree residing at node n. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 1, 2001. 
//
// ****************************************************************************

avtDataTree_p 
avtDataTree::GetChild(int n)
{
    if (n < 0 || n >= nChildren)
    {
        EXCEPTION2(BadIndexException, n, nChildren);
    }

    return children[n];
}


// ****************************************************************************
//  Method: avtDataTree::GetDataRepresentation
//
//  Purpose:
//    To fetch the vtkDataSet representation of this tree. 
//
//  Returns:
//    The vtkDataSet representation wrapped as an avtDataRepresentation. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 1, 2001. 
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Apr  9 14:31:41 PDT 2001
//    Made this method only functional from leaf nodes.  No longer will
//    an intermediate dataRep be created for internal nodes.
//
// ****************************************************************************

avtDataRepresentation &
avtDataTree::GetDataRepresentation()
{
    if (dataRep == NULL)
    {
        debug1 << "Attempting to retrieve data representation from "
               << "an internal node." << endl;
        EXCEPTION0(ImproperUseException);
    }

    return *dataRep;
}

// ****************************************************************************
//  Method: avtDataTree::GetNumberOfCells
//
//  Purpose:
//    To count the number of cells (polys, elements, etc.) of all datasets
//    in tree 
//
//  Programmer: Mark C. Miller
//  Creation:   19Aug03 
//
// ****************************************************************************

int
avtDataTree::GetNumberOfCells(void) const
{
    if (nChildren > 0)
    {
        int sum = 0;
        for (int i = 0; i < nChildren; i++)
        {
            if (*children[i] != NULL)
                sum += children[i]->GetNumberOfCells();
        }
        return sum;
    }
    else if (dataRep != NULL)
    {
        return dataRep->GetNumberOfCells();
    }

    return 0;
}


// ****************************************************************************
//  Method: avtDataTree::GetAllLeaves
//
//  Purpose:
//    Returns as vtkDataSet * all leaves of this tree. 
//
//  Arguments:
//    numLeaves     How many leaves were stored. 
//
//  Returns:    The vtkDataSet * in which to return the leaves 
//
//  Notes:      The calling function must free the array where the dataset is
//              held.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 1, 2001. 
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 19 11:47:55 PdT 2001
//    Reflect method CalcTotalLeaves renamed as GetNumberOfLeaves. 
//
//    Hank Childs, Tue Jun 19 17:31:06 PDT 2001
//    More lee-way for bad data trees.
//
// ****************************************************************************

vtkDataSet**
avtDataTree::GetAllLeaves(int &numLeaves)
{
    int leavesGuess = GetNumberOfLeaves();
    vtkDataSet **leaves = new vtkDataSet * [leavesGuess];

    int n = 0;
    AddLeafToList(leaves, n);

    numLeaves = n;
    return leaves;
}


// ****************************************************************************
//  Method: avtDataTree::GetNumberOfLeaves
//
//  Purpose:
//    Calculates how many leaves this tree has. 
//
//  Returns:  the number of leave in this tree.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 1, 2001. 
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Feb 15 10:34:00 PST 2001
//    Added check for null child.
//
//    Kathleen Bonnell, Thu Apr 19 11:47:55 PdT 2001
//    Renamed as GetNumberOfLeaves. 
//
//    Hank Childs, Mon Nov 12 14:27:03 PST 2001
//    Account for degenerate cases.
//
// ****************************************************************************

int
avtDataTree::GetNumberOfLeaves()
{
    if (nChildren > 0)
    {
        int sum = 0;
        for (int i = 0; i < nChildren; i++)
        {
            if (*children[i] != NULL)
                sum += children[i]->GetNumberOfLeaves();
        }
        return sum;
    }
    else if (dataRep != NULL)
    {
        return 1;
    }

    return 0;
}


// ****************************************************************************
//  Method: avtDataTree::GetSingleLeaf
//
//  Purpose:
//      Determines how many leaves there are and returns the first one if there
//      is exactly one leaf.  Otherwise returns NULL.
//
//  Returns:    The first leaf provided there is exactly one, otherwise NULL.
//
//  Programmer: Hank Childs
//  Creation:   November 12, 2001
//
// ****************************************************************************

vtkDataSet *
avtDataTree::GetSingleLeaf(void)
{
    int nLeaves = GetNumberOfLeaves();
    if (nLeaves != 1)
    {
        return NULL;
    }

    vtkDataSet **ds = GetAllLeaves(nLeaves);
    if (nLeaves != 1)
    {
        // Hmmm... we were earlier quoted one leaf, but now we got more than
        // one.
        debug1 << "avtDataTree::GetSingleLeaf ... unexpected case."
               << "  investigate." << endl;
        return NULL;
    }

    vtkDataSet *rv = ds[0];
    delete [] ds;

    return rv;
}


// ****************************************************************************
//  Method: avtDataTree::AddLeafToList
//
//  Purpose:
//    Recursive method to find all leaves and add them to the leaf list.
//
//  Arguments:  
//    leafList   the vtkDataSets in which to store the leaf when it is found
//    index      the index into the list that indicates where the leaf
//               is to be stored. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 1, 2001. 
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 15 10:34:00 PST 2001
//    Added check for null child.
//
//    Hank Childs, Tue Jun 19 17:31:06 PDT 2001
//    Allowed for invalid data trees.
//
// ****************************************************************************

void
avtDataTree::AddLeafToList(vtkDataSet** leafList, int & index)
{
    if (nChildren > 0)
    {
        for (int i = 0; i < nChildren; i++)
        {
            if (*children[i] != NULL)
                children[i]->AddLeafToList(leafList, index);
        }
    }
    else if (dataRep != NULL)
    {
        leafList[index] = dataRep->GetDataVTK();
        index++;
    }
}


// ****************************************************************************
//  Method: avtDataTree::Merge
//
//  Purpose:
//    Merges the argument data tree with this one. 
//
//  Arguments:  
//    rhs       The data tree to merge with.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 5, 2001 
//
//  Modifications:
//
//    Hank Childs, Sun Jun 17 14:16:37 PDT 2001
//    Re-wrote routine to merge arbitrary trees instead of only those with
//    similar structures.
//
//    Hank Childs, Tue Jul 31 10:00:09 PDT 2001
//    Put in check for merging into itself.
//
// ****************************************************************************

void
avtDataTree::Merge(avtDataTree_p rhs)
{
    if (*rhs == this)
    {
        debug1 << "Dissallowing merge since it would cause an infinite tree."
               << endl;
        EXCEPTION0(ImproperUseException);
    }

    avtDataTree_p my_new_root = NULL;
    if (nChildren == 0)
    {
        avtDataTree_p lhs = new avtDataTree(*dataRep);
        avtDataTree_p list[2];
        list[0] = lhs;
        list[1] = rhs;
        my_new_root = new avtDataTree(2, list);
    }
    else
    {
        avtDataTree_p *list = new avtDataTree_p[nChildren+1];
        for (int i = 0 ; i < nChildren ; i++)
        {
            list[i] = children[i];
        }
        list[nChildren] = rhs;
        my_new_root = new avtDataTree(nChildren+1, list);
        delete [] list;
    }
    *this = *(*my_new_root);
}


// ****************************************************************************
//  Method: avtDataTree::Traverse
//
//  Purpose:
//    Walks through the tree, calling the passed function on leaf nodes. 
//
//  Arguments:  
//    func      The function to be performed on the leaf node. 
//    flag      Indicates success or failure of the operation. 
//    arg       The argument to be passed to the function.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 13, 2001 
//
//  Modifications:
//
//    Hank Childs, Tue Jun 19 17:34:16 PDT 2001
//    Allow for bad data trees.
//
// ****************************************************************************

void 
avtDataTree::Traverse(TraverseFunc func, void *arg, bool &flag)
{
    if (nChildren == 0)
    {
        if (dataRep == NULL)
        {
            return;
        }
        func(*dataRep, arg, flag); 
    }
    else
    {
        for (int i = 0; i < nChildren; i++)
        {
            if (ChildIsPresent(i))
            {
                children[i]->Traverse(func, arg, flag);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtDataTree::PruneTree
//
//  Purpose:
//    Prunes unnecessary leaves from the tree, based on the ids in list.
//
//  Returns:
//    The pruned tree.
//      
//  Arguments:
//    list      The chunk ids that should be in the output tree. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 19, 2001 
//
//  Modifications:
//
//    Hank Childs, Sat Jan  5 10:36:36 PST 2002
//    Fix memory leak.
//
//    Hank Childs, Wed Jan  9 09:33:27 PST 2002
//    Made pmap an automatic variable (fixes memory leak).
//
//    Kathleen Bonnell, Mon May  6 16:42:55 PDT 2002 
//    Sometimes the chunkId is set to -1, meaning that the domain
//    for the dataset is unknown, or the dataset is a conglomeration
//    of domains.  Allow for such designated datasets to be included in
//    the pruned tree. 
//
// ****************************************************************************

avtDataTree_p
avtDataTree::PruneTree(const vector<int> &list)
{
    struct map
    {
        int            chunkId;
        avtDataTree_p  subtree;
    } pmap;

    bool success;

    avtDataTree_p *treelist = new avtDataTree_p[list.size()+1];
    int count = 0;
    for (int i = 0 ; i < list.size() ; i++)
    {
        success = false;
        pmap.chunkId = list[i];
        pmap.subtree = new avtDataTree();
        Traverse(CGetChunkByDomain, &pmap, success);

        if (success)
        {
            treelist[count] = pmap.subtree;
            count++;
        }
    }

    //
    //   Add a subtree for datasets with 'unknown' domains.
    //   (chunkId == -1). 
    //
    success = false;
    pmap.chunkId = -1;
    pmap.subtree = new avtDataTree();
    Traverse(CGetChunkByDomain, &pmap, success);
    if (success)
    {
        debug5 << "Found leaves with id == -1, including them "
               << "in the pruned tree." << endl;
        treelist[count] = pmap.subtree;
        count++;
    }


    avtDataTree_p rv = NULL;
    if (count <= 0)
    {
        rv =  new avtDataTree();
    }
    else
    {
        rv =  new avtDataTree(count, treelist);
    }
    delete [] treelist;
    return rv;
}


// ****************************************************************************
//  Method: avtDataTree::PruneTree
//
//  Purpose:
//    Prunes unnecessary leaves from the tree, based on the ids in list.
//
//  Returns:
//    The pruned tree.
//      
//  Arguments:
//    list      The chunk ids that should be in the output tree. 
//    goodIds   The chunk Ids that are part of the pruned tree.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 19, 2001 
//
//  Modifications:
//
//    Hank Childs, Sat Jan  5 10:36:36 PST 2002
//    Fix memory leak.
//
//    Hank Childs, Wed Jan  9 09:33:27 PST 2002
//    Made pmap an automatic variable (fixes memory leak).
//
// ****************************************************************************

avtDataTree_p
avtDataTree::PruneTree(const vector<int> &list, vector<int> &goodIds)
{
    if (!goodIds.empty())
    {
        goodIds.clear();
    }

    struct map
    {
        int            chunkId;
        avtDataTree_p  subtree;
    } pmap;

    bool success;

    avtDataTree_p *treelist = new avtDataTree_p[list.size()];
    int count = 0;
    for (int i = 0 ; i < list.size() ; i++)
    {
        success = false;
        pmap.chunkId = list[i];
        pmap.subtree = new avtDataTree();
        Traverse(CGetChunkByDomain, &pmap, success);

        if (success)
        {
            treelist[count] = pmap.subtree;
            goodIds.push_back(list[i]);
            count++;
        }
    }
    avtDataTree_p rv = NULL;
    if (count <= 0)
    {
        rv = new avtDataTree();
    }
    else
    {
        rv = new avtDataTree(count, treelist);
    }
    delete [] treelist;
    return rv;
}


// ****************************************************************************
//  Method: avtDataTree::PruneTree
//
//  Purpose:
//    Prunes unnecessary leaves from the tree, based on the labels in list.
//
//  Returns:
//    The pruned tree.
//      
//  Arguments:
//    labels      The labels that we want to include in the pruned tree.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 19, 2001 
//
//  Modifications:
//
//    Hank Childs, Sat Jan  5 10:36:36 PST 2002
//    Fix memory leak.
//
//    Hank Childs, Wed Jan  9 09:33:27 PST 2002
//    Made pmap an automatic variable (fixes memory leak).
//
// ****************************************************************************

avtDataTree_p
avtDataTree::PruneTree(const vector<string> &labels)
{
    struct map
    {
        string         label;
        avtDataTree_p  subtree;
    } pmap;

    bool success;

    avtDataTree_p *treelist = new avtDataTree_p[labels.size()];
    int count = 0;
    for (int i = 0 ; i < labels.size() ; i++)
    {
        success = false;
        pmap.label = labels[i];
        pmap.subtree = new avtDataTree();
        Traverse(CGetChunkByLabel, &pmap, success);

        if (success)
        {
            treelist[count] = pmap.subtree;
            count++;
        }
    }
    avtDataTree_p rv = NULL;
    if (count <= 0)
    {
        rv = new avtDataTree();
    }
    else
    {
        rv = new avtDataTree(count, treelist);
    }
    delete [] treelist;
    return rv;
}


// ****************************************************************************
//  Method: avtDataTree::PruneTree
//
//  Purpose:
//    Prunes unnecessary leaves from the tree, based on the labels in list.
//
//  Returns:
//    The pruned tree.
//      
//  Arguments:
//    labels      The labels that we want to include in the pruned tree.
//    goodLabels  The actual labels that are part of the pruned tree.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 19, 2001 
//
//  Modifications:
//
//    Hank Childs, Sat Jan  5 10:36:36 PST 2002
//    Fix memory leak.
//
//    Hank Childs, Wed Jan  9 09:33:27 PST 2002
//    Made pmap an automatic variable (fixes memory leak).
//
// ****************************************************************************

avtDataTree_p
avtDataTree::PruneTree(const vector<string> &labels, vector<string> &goodLabels)
{
    if (!goodLabels.empty())
    {
        goodLabels.clear();
    }

    struct map
    {
        string         label;
        avtDataTree_p  subtree;
    } pmap;

    bool success;

    avtDataTree_p *treelist = new avtDataTree_p[labels.size()];
    int count = 0;
    for (int i = 0 ; i < labels.size() ; i++)
    {
        success = false;
        pmap.label = labels[i];
        pmap.subtree = new avtDataTree();
        Traverse(CGetChunkByLabel, &pmap, success);

        if (success)
        {
            treelist[count] = pmap.subtree;
            goodLabels.push_back(labels[i]);
            count++;
        }
    }
    avtDataTree_p rv = NULL;
    if (count <= 0)
    {
        rv = new avtDataTree();
    }
    else
    {
        rv = new avtDataTree(count, treelist);
    }
    delete [] treelist;
    return rv;
}

// ****************************************************************************
//  Method: avtDataTree::WriteTreeStructure
//
//  Purpose:
//      Peforms a pre-order traversal, writing this tree's structure 
//      to the desired output stream.
//
//      For each node, lists number of children and for each child
//      whether it is present (not null) or absent (null).  When a 
//      leaf is reached, lists whether the data repesentation is
//      null or gives the address.
//
//  Arguments:
//      os      The output stream.
//      indent  The (optional) starting indent.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 13, 2001
//
// ****************************************************************************

void
avtDataTree::WriteTreeStructure(ostream &os, int indent)
{
    os << nChildren ;

    if (nChildren > 0)
    {
        indent += 5;
        for (int i = 0; i < nChildren; ++i)
        {
           os << "\n";
           for (int j = 0; j < indent; j++)
               os << " "; 
            if ( *(children[i]) != NULL )
            {
                os << i << "P " ;
                children[i]->WriteTreeStructure(os, indent);
            }
            else
            {
                os << i << "A " ;
            }
        }
        indent -= 5;
    }
    else
    {
        if (dataRep == NULL)
            os << " DR=NULL " ;
        else
        {
            os << " DR="  << dataRep << " ";
            os << "  domain: " << dataRep->GetDomain() << " "; 
            os << "  label:  " << dataRep->GetLabel().c_str() << " "; 
        }
    }
    os << "\n";
}


// ****************************************************************************
//  Method: avtDataTree::GetAllLabels
//
//  Purpose:
//    Recursive method to retrieve labels. 
//
//  Arguments:  
//    labels    A place to store the retrieved labels.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 24, 2001 
//
// ****************************************************************************

void
avtDataTree::GetAllLabels(vector<string> &labels)
{
    if (nChildren > 0)
    {
        for (int i = 0; i < nChildren; i++)
        {
            if (*children[i] != NULL)
                children[i]->GetAllLabels(labels);
        }
    }
    else if (dataRep != NULL)
    {
        if (!dataRep->GetLabel().empty())
        {
            labels.push_back(dataRep->GetLabel());
        }
    }
}

// ****************************************************************************
//  Method: avtDataTree::IsEmpty
//
//  Purpose:
//    Returns true if this tree is empty 
//    as constructed by default 'empty' constructor. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 1, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Tue Sep 16 13:21:42 PDT 2003
//    Not all 'empty' trees have been created from the default 'empty' 
//    constructor, some have been passed an empty 'child' tree.  Account
//    for that in this test.
//
// ****************************************************************************

bool
avtDataTree::IsEmpty()
{
    bool empty = (nChildren == 1 && *children[0] == NULL);
    if (!empty && nChildren == 1)
    {
        empty = children[0]->IsEmpty();
    }
    return empty;
}


// ****************************************************************************
//  Method: avtDataTree::GetAllUniqueLabels
//
//  Purpose:
//    User-visible method to retrieve labels, ensuring that all elements
//    of the list are unique.   Calls the recursive method.
//
//  Arguments:  
//    labels    A place to store the retrieved labels.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 29, 2002 
//
// ****************************************************************************

void
avtDataTree::GetAllUniqueLabels(vector<string> &labels)
{
    labels.clear();
    set < string > labelSet;

    GetUniqueLabels(labels, labelSet);
}

// ****************************************************************************
//  Method: avtDataTree::GetUniqueLabels
//
//  Purpose:
//    Recursive method to retrieve labels, ensuring that all elements
//    of the list are unique.. 
//
//  Arguments:  
//    labels    A place to store the retrieved labels.
//    labelSet  A place to store the set used for determining uniqueness. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 29, 2002 
//
// ****************************************************************************

void
avtDataTree::GetUniqueLabels(vector<string> &labels, set<string> &labelSet)
{
    if (nChildren > 0)
    {
        for (int i = 0; i < nChildren; i++)
        {
            if (*children[i] != NULL)
                children[i]->GetUniqueLabels(labels, labelSet);
        }
    }
    else if (dataRep != NULL)
    {
        if (!dataRep->GetLabel().empty())
        {
            string aLabel = dataRep->GetLabel();
            if ((labelSet.count(aLabel)) == 0)
            {
                labels.push_back(aLabel);
                labelSet.insert(aLabel);
            }
        }
    }
}

