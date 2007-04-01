// ************************************************************************* //
//                              avtIntervalTree.C                            //
// ************************************************************************* //

#include <stdlib.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtIntervalTree.h>

#include <IntervalTreeNotCalculatedException.h>
#include <BadDomainException.h>

#include <vtkBox.h>

using     std::vector;


//
// Macros
//

#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y)  ((X) > (Y) ? (X) : (Y))


//
// Static prototypes
//

static float    EquationsValueAtPoint(const float *, int, int, int, 
                                      const float *);
static bool     Intersects(const float *, float, int, int, const float *);
static bool     Intersects(float [3], float[3], int, int, const float *);


// ****************************************************************************
//  Method: avtIntervalTree constructor
//
//  Arguments:
//     doms     The number of domains.
//     dims     The number of dimensions for the field.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
//  Modifications:
//
//    Mark C. Miller, Mon Oct 18 14:36:49 PDT 2004
//    Added hasBeenCalculated
//
// ****************************************************************************

avtIntervalTree::avtIntervalTree(int doms, int dims)
{
    nDomains    = doms;
    nDims       = dims;
    hasBeenCalculated = false;

    //
    // A vector for one domain should have the min and max for each dimension.
    // 
    vectorSize  = 2*nDims;

    //
    // Calculate number of nodes needed to make a complete binary tree when
    // there should be nDomains leaf nodes.
    //
    int numCompleteTrees = 1, exp = 1;
    while (2*exp < nDomains)
    {
        numCompleteTrees = 2*numCompleteTrees + 1;
        exp *= 2;
    }
    nNodes = numCompleteTrees + 2 * (nDomains - exp);

    nodeExtents = new float[nNodes*vectorSize];
    nodeIDs     = new int[nNodes];
    for (int i = 0 ; i < nNodes ; i++)
    {
        for (int j = 0 ; j < vectorSize ; j++)
        {
            nodeExtents[i*vectorSize + j] = 0.;
        }
        nodeIDs[i]  = -1;
    }
}


// ****************************************************************************
//  Method: avtIntervalTree destructor
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

avtIntervalTree::~avtIntervalTree()
{
    if (nodeExtents != NULL)
    {
        delete [] nodeExtents;
    }
    if (nodeIDs != NULL)
    {
        delete [] nodeIDs;
    }
}


// ****************************************************************************
//  Method: avtIntervalTree::Destruct
//
//  Purpose:
//      Calls the destructor for an interval tree.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
// ****************************************************************************

void
avtIntervalTree::Destruct(void *i)
{
    avtIntervalTree *itree = (avtIntervalTree *) i;
    delete itree;
}


// ****************************************************************************
//  Method: avtIntervalTree::GetExtents
//
//  Purpose:
//      Sees what the range of values the variable contained by the interval
//      tree has.  This is all contained in the first node.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

void
avtIntervalTree::GetExtents(float *extents) const
{
    if ((nodeExtents == NULL) || (hasBeenCalculated == false))
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    //
    // Copy the root cell into the extents.
    //
    for (int i = 0 ; i < nDims*2 ; i++)
    {
        extents[i] = nodeExtents[i];
    }
}


// ****************************************************************************
//  Method: avtIntervalTree::AddDomain
//
//  Purpose:
//      Adds the extents for one domain.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

void
avtIntervalTree::AddDomain(int domain, float *d)
{
    //
    // Sanity Check
    //
    if (domain < 0 || domain >= nDomains)
    {
        EXCEPTION2(BadDomainException, domain, nDomains);
    }

    for (int i = 0 ; i < vectorSize ; i++)
    {
        nodeExtents[domain*vectorSize + i] = d[i];
    }
}


// ****************************************************************************
//  Method: avtIntervalTree::Calculate
//
//  Purpose:
//      Calculates the interval tree.  This means collecting the information
//      from other processors (if we are in parallel) and constructing the
//      tree.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Dec  5 17:21:53 PST 2001 
//    Added argument to allow for serial usage.
//
//    Mark C. Miller, Mon Oct 18 14:36:49 PDT 2004
//    Added hasBeenCalculated
//
// ****************************************************************************

void
avtIntervalTree::Calculate(bool alreadyCollectedAllInformation)
{
    if (!alreadyCollectedAllInformation)
    {
        CollectInformation();
    }
    ConstructTree();
    hasBeenCalculated = true;
}


// ****************************************************************************
//  Method: avtIntervalTree::CollectInformation
//
//  Purpose:
//      If we are doing the job in parallel, the domains were split across all
//      the nodes.  Collect the information here.
//
//  Note:       Does nothing in the serial case.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

void
avtIntervalTree::CollectInformation(void)
{
#ifdef PARALLEL
    //
    // Extents are spread across all of the processors.  Since nodeExtents was
    // initialized to have value 0., we can do an MPI_SUM and get the correct
    // list on processor 0.
    //
    int totalElements = nDomains*vectorSize;
    float *outBuff = new float[totalElements];
    MPI_Allreduce(nodeExtents, outBuff, totalElements, MPI_FLOAT, MPI_SUM,
               MPI_COMM_WORLD);

    for (int i = 0 ; i < totalElements ; i++)
    {
        nodeExtents[i] = outBuff[i];
    }
    delete [] outBuff;
#endif
}


// ****************************************************************************
//  Method: avtIntervalTree::ConstructTree
//
//  Purpose:
//      Constructs the complete binary tree of the nodes' extents.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

void
avtIntervalTree::ConstructTree(void)
{
    int    i;

    //
    // Make a local copy of the bounds so that we can move them around.
    //
    float  *bounds = new float[nDomains*vectorSize];
    for (i = 0 ; i < nDomains*vectorSize ; i++)
    {
        bounds[i] = nodeExtents[i];
    }

    int      offsetStack[100];   // Only need log nDomains
    int      nodeStack  [100];   // Only need log nDomains
    int      sizeStack  [100];   // Only need log nDomains
    int      depthStack [100];   // Only need log nDomains
    int      stackCount = 0;
    int     *blockID    = new int[nDomains];

    for (i = 0 ; i < nDomains ; i++)
    {
        blockID[i] = i;
    }

    //
    // Initialize the arguments for the stack
    //
    offsetStack[0]  = 0;
    sizeStack  [0]  = nDomains;
    depthStack [0]  = 0;
    nodeStack  [0]  = 0;
    ++stackCount;

    int currentOffset, currentSize, currentDepth, leftSize, currentNode;
    while (stackCount > 0)
    {
        --stackCount;
        currentOffset = offsetStack[stackCount];
        currentSize   = sizeStack  [stackCount];
        currentDepth  = depthStack [stackCount];
        currentNode   = nodeStack  [stackCount];

        if (currentSize <= 1)
        {
            nodeIDs[currentNode] = blockID[currentOffset];
            for (int j = 0 ; j < vectorSize ; j++)
            {
                nodeExtents[currentNode*vectorSize + j] =
                                        bounds[currentOffset*vectorSize + j];
            }
            continue;
        }

        Sort(bounds + currentOffset*vectorSize, blockID + currentOffset,
             currentSize, currentDepth);

        leftSize = SplitSize(currentSize);

        offsetStack[stackCount]  = currentOffset;
        sizeStack  [stackCount]  = leftSize;
        depthStack [stackCount]  = currentDepth + 1;
        nodeStack  [stackCount]  = 2*currentNode + 1;
        ++stackCount;

        offsetStack[stackCount]  = currentOffset + leftSize;
        sizeStack  [stackCount]  = currentSize - leftSize;
        depthStack [stackCount]  = currentDepth + 1;
        nodeStack  [stackCount]  = 2*currentNode + 2;
        ++stackCount;
    }

    SetIntervals();

    delete [] blockID;
    delete [] bounds;
}


// ****************************************************************************
//  Method: avtIntervalTree method Sort
//
//  Purpose:
//      Sorts a vector of bounds with respect to the dimension that
//      corresponds to the depth.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

void
avtIntervalTree::Sort(float *bounds, int *blockID, int size, int depth)
{
    float    temp;
    int      temp2;

    for (int i = 0 ; i < size ; i++)
    {
        for (int j = i + 1 ; j < size ; j++)
        {
            if (Less(bounds + j*vectorSize, bounds + i*vectorSize, depth))
            {
                for (int k = 0 ; k < vectorSize ; k++)
                {
                    temp = bounds[j*vectorSize + k];
                    bounds[j*vectorSize + k] = bounds[i*vectorSize + k];
                    bounds[i*vectorSize + k] = temp;
                }
                temp2 = blockID[j];
                blockID[j] = blockID[i];
                blockID[i] = temp2;
            }
        }
    }
}


// ****************************************************************************
//  Method: avtIntervalTree::Less
//
//  Purpose:
//      An operator that can sort a variable according to any dimension.
//
//  Arguments:
//      A        A vector of variables of size vectorSize.
//      B        A vector of variables of size vectorSize.
//      depth    The level of this node in the interval tree.  This is used
//               to find what dimension to sort across.
//
//  Returns:    True if A < B, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

bool
avtIntervalTree::Less(float *A, float *B, int depth)
{
    int        i;
    const int  N_DIMS_LIMIT = 32;
    float      A2[N_DIMS_LIMIT], B2[N_DIMS_LIMIT];

    //
    // Construct an array of the mid-points of the extents.  The first
    // element of the array will be the first dimension to sort along,
    // the second element will be the second, etc.
    //
    for (i = 0 ; i < nDims ; i++)
    {
        A2[i] = (A[(2*depth + 2*i) % vectorSize]
                                + A[(2*depth+1 + 2*i) % vectorSize]) / 2;
        B2[i] = (B[(2*depth + 2*i) % vectorSize]
                                + B[(2*depth+1 + 2*i) % vectorSize]) / 2;
    }

    //
    // Sort across axis.  If there is a tie, then sort across next axis, etc.
    //
    for (i = 0 ; i < nDims ; i++)
    {
        if (A2[i] < B2[i])
            return true;
        else if (A2[i] > B2[i])
            return false;
    }

    //
    // Bounds are exactly identical.  This can cause problems if you are
    // not careful.
    //
    return false;
}


// ****************************************************************************
//  Method: avtIntervalTree::SetIntervals
//
//  Purpose:
//      Now that the intervals for all of the leaf nodes have been set and put
//      in their proper place in the interval tree, calculate the bounds for
//      all of the parenting nodes.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

void
avtIntervalTree::SetIntervals()
{
    int parent;

    for (int i = nNodes-1 ; i > 0 ; i -= 2)
    {
        parent = (i - 2) / 2;
        for (int k = 0 ; k < nDims ; k++)
        {
            nodeExtents[parent*vectorSize + 2*k] =
                      MIN(nodeExtents[(i-1)*vectorSize + 2*k],
                          nodeExtents[i*vectorSize + 2*k]);
            nodeExtents[parent*vectorSize + 2*k + 1] =
                      MAX(nodeExtents[(i-1)*vectorSize + 2*k + 1],
                          nodeExtents[i*vectorSize + 2*k + 1]);
        }
    }
}


// ****************************************************************************
//  Method: avtIntervalTree::SplitSize
//
//  Purpose:
//      Determine how big the left side of the tree should be to make a
//      complete binary tree.
//
//  Returns:      The proper size of the left side of the child tree.
//
//  Programmer:   Hank Childs
//  Creation:     August 8, 2000
//
// ****************************************************************************

int
avtIntervalTree::SplitSize(int size)
{
    //
    // Decompose size into 2^y + n where 0 <= n < 2^y
    //
    int power = 1;
    while (power*2 <= size)
    {
        power *= 2;
    }
    int n = size - power;

    if (n == 0)
    {
        return power/2;
    }
    if (n < power/2)
    {
        return (power/2 + n);
    }

    return power;
}



// ****************************************************************************
//  Method: avtIntervalTree::GetDomainsList
//
//  Purpose:
//      Takes in a linear equation and determines which domains have values
//      that satisfy the equation.
//      The equation is of the form:  params[0]*x + params[1]*y ... = solution
//
//  Arguments:
//      params        The coefficients of the linear equation.
//      solution      The right hand side (solution) of the linear equation.
//      list          The list of domains that satisfy the linear equation.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 1999
//
//  Modifications:
//
//    Mark C. Miller, Mon Oct 18 14:36:49 PDT 2004
//    Added code to throw an exception of the tree hasn't been calculated
//
// ****************************************************************************

void
avtIntervalTree::GetDomainsList(const float *params, float solution,
                                vector<int> &list) const
{
    if (hasBeenCalculated == false)
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    list.clear();

    int nodeStack[100]; // Only need log amount
    int nodeStackSize = 0;

    //
    // Populate the stack by putting on the root domain.  This domain contains
    // all the other domains in its extents.
    //
    nodeStack[0] = 0;
    nodeStackSize++;

    while (nodeStackSize > 0)
    {
        nodeStackSize--;
        int stackIndex = nodeStack[nodeStackSize];
        if ( Intersects(params, solution, stackIndex, nDims, nodeExtents) )
        {
            //
            // The equation has a solution contained by the current extents.
            //
            if (nodeIDs[stackIndex] < 0)
            {
                //
                // This is not a leaf, so put children on stack
                //
                nodeStack[nodeStackSize] = 2 * stackIndex + 1;
                nodeStackSize++;
                nodeStack[nodeStackSize] = 2 * stackIndex + 2;
                nodeStackSize++;
            }
            else
            {
                //
                // Leaf node, put in list
                //
                list.push_back(nodeIDs[stackIndex]);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtIntervalTree::GetDomainsListFromRange
//
//  Purpose:
//      Takes in a linear equation and determines which domains have values
//      that satisfy the equation.
//      The equation is of the form:  params[0]*x + params[1]*y ... = solution
//
//  Arguments:
//      params        The coefficients of the linear equation.
//      solution      The right hand side (solution) of the linear equation.
//      list          The list of domains that satisfy the linear equation.
//
//  Programmer: Hank Childs
//  Creation:   May 14, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Oct 18 14:36:49 PDT 2004
//    Added code to throw an exception of the tree hasn't been calculated
//
// ****************************************************************************
 
void
avtIntervalTree::GetDomainsListFromRange(const float *min_vec,
                                 const float *max_vec, vector<int> &list) const
{
    if (hasBeenCalculated == false)
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    list.clear();
 
    int nodeStack[100]; // Only need log amount
    int nodeStackSize = 0;
 
    //
    // Populate the stack by putting on the root domain.  This domain contains
    // all the other domains in its extents.
    //
    nodeStack[0] = 0;
    nodeStackSize++;
 
    while (nodeStackSize > 0)
    {
        nodeStackSize--;
        int stackIndex = nodeStack[nodeStackSize];
        bool inBlock = true;
        for (int i = 0 ; i < nDims ; i++)
        {
            float min_extent = min_vec[i];
            float max_extent = max_vec[i];
            float min_node   = nodeExtents[stackIndex*nDims*2 + 2*i];
            float max_node   = nodeExtents[stackIndex*nDims*2 + 2*i+1];
            if (min_node > max_extent)
                inBlock = false;
            if (max_node < min_extent)
                inBlock = false;
            if (!inBlock)
                break;
        }
        if (inBlock)
        {
            //
            // The equation has a solution contained by the current extents.
            //
            if (nodeIDs[stackIndex] < 0)
            {
                //
                // This is not a leaf, so put children on stack
                //
                nodeStack[nodeStackSize] = 2 * stackIndex + 1;
                nodeStackSize++;
                nodeStack[nodeStackSize] = 2 * stackIndex + 2;
                nodeStackSize++;
            }
            else
            {
                //
                // Leaf node, put in list
                //
                list.push_back(nodeIDs[stackIndex]);
            }
        }
    }
}


// ****************************************************************************
//  Function: Intersects
//
//  Purpose:
//      Determine if the range of values for the current node has a solution
//      to the equation specified by params and solution.
//
//  Arguments:
//      params       The coeffecients to the vars in the linear equation.
//      solution     The desired sum of the linear equation.
//      block        The block in the nodeExtents that should be checked for an
//                   intersection.
//      nDims        The number of dimensions of the var.
//      nodeExtents  The extents at each node.
//
//  Returns:    true if there is an intersection, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Mar 30 08:15:46 PST 2004
//    Add some numerical tolerances.
//
// ****************************************************************************

bool
Intersects(const float *params, float solution, int block, int nDims,
           const float *nodeExtents)
{
    int  i;

    int  numEncodings = 1;
    for (i = 0 ; i < nDims ; i++)
    {
        numEncodings *= 2;
    }

    float  valAtMin  = EquationsValueAtPoint(params, block, 0, nDims,
                                             nodeExtents);
    if (fabs(valAtMin-solution) < 1e-12)
    {
        //
        // It happens to be that at the minimum extents the value of the
        // linear equation equals the solution, so we have an intersection.
        //
        return true;
    }

    bool  tooSmall = false;
    if (valAtMin < solution)
    {
        tooSmall = true;
    }

    for (i = 1 ; i < numEncodings ; i++)
    {
        float solutionAtI = EquationsValueAtPoint(params, block, i, nDims,
                                                  nodeExtents);
        if (tooSmall && solutionAtI >= solution)
        {
            //
            // F(Point 0) is too small and F(Point i) is too large.  This
            // means that there is an intersection.  And I thought the eight
            // weeks I spent proving the Intermediate Value Theorem in college
            // were useless!!!
            //
            return true;
        }
        if (!tooSmall && solutionAtI <= solution)
        {
            //
            // F(Point 0) is too large and F(Point i) is too small.  This
            // means that there is an intersection.
            //
            return true;
        }
    }

    //
    // All of the extents were too large or too small, so there was no
    // intersection.
    //
    return false;
}


// ****************************************************************************
//  Function: EquationsValueAtPoint
//
//  Purpose:
//      Determine what a linear equations value is at a given point.
//      The point argument is a number corresponding with a given extreme
//      in the nodeExtents.  For example if ndims = 1, then point 0 would
//      be at the minimum and point 1 would be at the max.  There would
//      only be two valid points.  If ndims = 2, then point 0 would be at
//      the minimum for dimensions 0 and 1, point 1 would be at the minimum
//      for dim. 0 and the max for dim.1 and so on.
//
//  Arguments:
//      params       The coefficients for the linear equation.
//      block        The block to use when calculating the value.  This is
//                   not an index into nodeExtents.  The index into nodeExtents
//                   is block*nDims*2.
//      point        The encoded position in the extents for that block.
//      nDims        The number of dimensions of the var.
//      nodeExtents  The extents at each node.
//
//  Returns:       The value at the specified point.
//
//  Note:          It is assumed that the number of dimensions is not larger
//                 than sizeof(int).  This seems very safe when considering the
//                 algorithm used it would take 1->MAX_INT iterations.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

float
EquationsValueAtPoint(const float *params, int block, int point, int nDims,
                      const float *nodeExtents)
{
    static int  encoding[32];
    static bool firstTimeThrough = true;
    if (firstTimeThrough)
    {
        encoding[0] = 1;
        for (int i = 1 ; i < 32 ; i++)
        {
            encoding[i] = encoding[i-1] << 1;
        }
    }

    float rv = 0;
    for (int i = 0 ; i < nDims ; i++)
    {
        if (point & encoding[i])
        {
            //
            // The encoded point wants us to take the maximum extent for this
            // dimension.
            //
            rv += params[i] * nodeExtents[block*nDims*2 + 2*i + 1];
        }
        else
        {
            //
            // The encoded point wants us to take the minimum extent for this
            // dimension.
            //
            rv += params[i] * nodeExtents[block*nDims*2 + 2*i];
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtIntervalTree::GetLeafExtents
//
//  Purpose:
//      Gets the extents for a leaf node.
//
//  Arguments:
//      leafIndex    The index of the leaf.
//      extents      A place to put the extents of the domain.
//
//  Returns:         The domains this leaf comes from.
//
//  Note:            The input integer is _not_ the domain number.  It is the
//                   leaf number.  This is done to prevent having to search
//                   the nodeIDs list for each domain.  This routine is only
//                   useful when iterating over all domains.
//
//  Programmer:      Hank Childs
//  Creation:        December 15, 2000
//
// ****************************************************************************

int
avtIntervalTree::GetLeafExtents(int leafIndex, float *extents) const
{
    int   nodeIndex = nNodes-nDomains + leafIndex;
    for (int i = 0 ; i < vectorSize ; i++)
    {
        extents[i] = nodeExtents[vectorSize*nodeIndex + i];
    }

    return  nodeIDs[nodeIndex];
}


// ****************************************************************************
//  Method: avtIntervalTree::GetDomainExtents
//
//  Purpose:
//      Gets the extents for a domain.
//
//  Arguments:
//      domainIndex  The index of the domain.
//      extents      A place to put the extents of the domain.
//
//  Programmer:      Hank Childs
//  Creation:        December 15, 2000
//
// ****************************************************************************

void
avtIntervalTree::GetDomainExtents(int domainIndex, float *extents) const
{
    int   startIndex = nNodes-nDomains;
    for (int i = startIndex ; i < nNodes ; i++)
    {
        if (nodeIDs[i] == domainIndex)
        {
            for (int j = 0 ; j < vectorSize ; j++)
            {
                extents[j] = nodeExtents[vectorSize*i + j];
            }
            return;
        }
    }
}


// ****************************************************************************
//  Function: Intersects
//
//  Purpose:
//      Determine if the range of values for the current node is intersected 
//      by the line designated by origin and rayDir. 
//
//  Arguments:
//      origin       The origin of the ray. 
//      rayDir       The xyz components of the ray direction.
//      block        The block in the nodeExtents that should be checked for an
//                   intersection.
//      nDims        The number of dimensions of the var.
//      nodeExtents  The extents at each node.
//
//  Returns:    true if there is an intersection, false otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 19, 2003 
//
// ****************************************************************************

bool
Intersects(float origin[3], float rayDir[3], int block, int nDims,
           const float *nodeExtents)
{
    float bnds[6] = { 0., 0., 0., 0., 0., 0.};
    float coord[3] = { 0., 0., 0.};
    float t;

    for (int i = 0; i < nDims; i++)
    {
        bnds[2*i] = nodeExtents[block*nDims*2 + 2*i];
        bnds[2*i+1] = nodeExtents[block*nDims*2 + 2*i + 1];
    }

    if (vtkBox::IntersectBox(bnds, origin, rayDir, coord, t))
        return true;
    else 
        return false;
}


// ****************************************************************************
//  Method: avtIntervalTree::GetDomainsList
//
//  Purpose:
//      Takes in a ray origin and direction,  and determines which domains have 
//      are intersected by the ray. 
//
//  Notes: Copied from GetDomainLists for an equation, just changed the
//         args and the call to Intersects.
//
//  Arguments:
//      origin        The ray origin. 
//      rayDir        The ray direction.
//      list          The list of domains that satisfy the linear equation.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 19, 2003 
//
//  Modifications:
//
//    Mark C. Miller, Mon Oct 18 14:36:49 PDT 2004
//    Added code to throw an exception of the tree hasn't been calculated
//
// ****************************************************************************

void
avtIntervalTree::GetDomainsList(float origin[3], float rayDir[3],
                                vector<int> &list) const
{
    if (hasBeenCalculated == false)
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    list.clear();

    int nodeStack[100]; // Only need log amount
    int nodeStackSize = 0;

    //
    // Populate the stack by putting on the root domain.  This domain contains
    // all the other domains in its extents.
    //
    nodeStack[0] = 0;
    nodeStackSize++;

    while (nodeStackSize > 0)
    {
        nodeStackSize--;
        int stackIndex = nodeStack[nodeStackSize];
        if ( Intersects(origin, rayDir, stackIndex, nDims, nodeExtents) )
        {
            //
            // The equation has a solution contained by the current extents.
            //
            if (nodeIDs[stackIndex] < 0)
            {
                //
                // This is not a leaf, so put children on stack
                //
                nodeStack[nodeStackSize] = 2 * stackIndex + 1;
                nodeStackSize++;
                nodeStack[nodeStackSize] = 2 * stackIndex + 2;
                nodeStackSize++;
            }
            else
            {
                //
                // Leaf node, put in list
                //
                list.push_back(nodeIDs[stackIndex]);
            }
        }
    }
}
