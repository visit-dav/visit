/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                              avtIntervalTree.C                            //
// ************************************************************************* //

#include <stdlib.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtExecutionManager.h>
#include <avtParallel.h>
#include <avtIntervalTree.h>
#include <float.h>

#include <BadDomainException.h>
#include <ImproperUseException.h>
#include <IntervalTreeNotCalculatedException.h>
#include <TimingsManager.h>

#include <vtkCellIntersections.h>
#include <vtkVisItUtility.h>


//
// Macros
//

#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y)  ((X) > (Y) ? (X) : (Y))


//
// Types and globals (needed for qsort/QuickSelect). 
//

typedef union
{
    double f;
    int   b;
} FloatInt;

int globalCurrentDepth;
int globalNDims;

//
// Static prototypes
//

static double   EquationsValueAtPoint(const double *, int, int, int, 
                                      const double *);
static bool     Intersects(const double *, double, int, int, const double *);
static bool     AxiallySymmetricLineIntersection(const double *, const double*,
                                                 int, const double *);
static int      CompareFloatInt(const FloatInt *arg1, const FloatInt *arg2);
static bool     IntersectsWithRay(double [3], double[3], int, int, 
                                  const double *, double[3]);
static bool     IntersectsWithLine(double [3], double[3], int, int, 
                                   const double *, double[3]);
static inline bool LineIntersectsBox2D(double *, double, double,
                                       double, double, int, int);
static inline bool LineIntersectsBox(double *, double, double, double,
                                     double, double, double, int, int, int);


// ****************************************************************************
//  Method: avtIntervalTree constructor
//
//  Arguments:
//     els      The number of elements.
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
//    Kathleen Bonnell, Mon Aug 21 13:34:18 PDT 2006 
//    Add default bool arg -- specifies whether or not this tree
//    will required collective communication. 
//
//    Hank Childs, Mon Sep 13 19:01:26 PDT 2010
//    Initialize new data members for optimizing results.
//
//    Mark C. Miller, Tue Jul 21 12:00:18 PDT 2015
//    Prevent eventual malloc for negative size when/if els<=0.
// ****************************************************************************

avtIntervalTree::avtIntervalTree(int els, int dims, bool rc)
{
    nElements    = els>0?els:1;
    nDims       = dims;
    hasBeenCalculated = false;
    requiresCommunication = rc;
    accelerateSizeQueries = false;
    optimizeForRepeatedQueries = false;
    numElementsBeneathThisNode = NULL;

    //
    // A vector for one element should have the min and max for each dimension.
    // 
    vectorSize  = 2*nDims;

    //
    // Calculate number of nodes needed to make a complete binary tree when
    // there should be nElements leaf nodes.
    //
    int numCompleteTrees = 1, exp = 1;
    while (2*exp < nElements)
    {
        numCompleteTrees = 2*numCompleteTrees + 1;
        exp *= 2;
    }
    nNodes = numCompleteTrees + 2 * (nElements - exp);

    nodeExtents = new double[nNodes*vectorSize];
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
//  Method: avtIntervalTree copy constructor
//
//  Programmer: Hank Childs
//  Creation:   June 12, 2008
//
//  Modifications:
//
//    Hank Childs, Mon Sep 13 19:01:26 PDT 2010
//    Copy over new data members for optimizing performance.
//
// ****************************************************************************

avtIntervalTree::avtIntervalTree(const avtIntervalTree *it)
{
    nElements = it->nElements;
    nNodes = it->nNodes;
    nDims = it->nDims;
    vectorSize = it->vectorSize;
    nodeExtents = new double[nNodes*vectorSize];
    nodeIDs     = new int[nNodes];
    if (it->numElementsBeneathThisNode != NULL)
        numElementsBeneathThisNode = new int[nNodes];
    else
        numElementsBeneathThisNode = NULL;
    for (int i = 0 ; i < nNodes ; i++)
    {
        for (int j = 0 ; j < vectorSize ; j++)
        {
            nodeExtents[i*vectorSize + j] = it->nodeExtents[i*vectorSize + j];
        }
        nodeIDs[i]  = it->nodeIDs[i];
        if (numElementsBeneathThisNode)
            numElementsBeneathThisNode[i] = it->numElementsBeneathThisNode[i];
    }
    hasBeenCalculated = it->hasBeenCalculated;
    requiresCommunication = it->requiresCommunication;
    accelerateSizeQueries = it->accelerateSizeQueries;
    optimizeForRepeatedQueries = it->optimizeForRepeatedQueries;
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
    if (numElementsBeneathThisNode != NULL)
    {
        delete [] numElementsBeneathThisNode;
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
//  Method: avtIntervalTree::AccelerateSizeQueries
//
//  Purpose:
//      Instructs this object to accelerate size queries ... when the method
//      GetNumberOfElementsInRange is called, it can return the answer without
//      descending so far in the tree.
//
//  Programmer: Hank Childs
//  Creation:   September 13, 2010
//
// ****************************************************************************

void
avtIntervalTree::AccelerateSizeQueries(void)
{
    accelerateSizeQueries = true;
    numElementsBeneathThisNode = new int[nNodes];
    for (int i = 0 ; i < nNodes ; i++)
        numElementsBeneathThisNode[i] = 0;
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
avtIntervalTree::GetExtents(double *extents) const
{
    if ((nodeExtents == NULL) || (hasBeenCalculated == false))
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    //
    // Copy the root element into the extents.
    //
    for (int i = 0 ; i < nDims*2 ; i++)
    {
        extents[i] = nodeExtents[i];
    }
}


// ****************************************************************************
//  Method: avtIntervalTree::AddElement
//
//  Purpose:
//      Adds the extents for one element.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

void
avtIntervalTree::AddElement(int element, const double *d)
{
    //
    // Sanity Check
    //
    if (element < 0 || element >= nElements)
    {
        EXCEPTION2(BadDomainException, element, nElements);
    }

    for (int i = 0 ; i < vectorSize ; i++)
    {
        nodeExtents[element*vectorSize + i] = d[i];
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
//    Kathleen Bonnell, Mon Aug 21 13:34:18 PDT 2006 
//    Added requiresCommunication.
//
// ****************************************************************************

void
avtIntervalTree::Calculate(bool alreadyCollectedAllInformation)
{
    if (requiresCommunication && !alreadyCollectedAllInformation)
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
//      If we are doing the job in parallel, the elements were split across all
//      the nodes.  Collect the information here.
//
//  Note:       Does nothing in the serial case.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Dave Pugmire, Tue Jul  8 10:54:28 EDT 2008
//    Changed MPI_FLOAT to MPI_DOUBLE
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
    int totalElements = nElements*vectorSize;
    double *outBuff = new double[totalElements];
    
    MPI_Allreduce(nodeExtents, outBuff, totalElements, MPI_DOUBLE, MPI_SUM,
               VISIT_MPI_COMM);

    for (int i = 0 ; i < totalElements ; i++)
    {
        nodeExtents[i] = outBuff[i];
    }
    delete [] outBuff;
#endif
}


// ****************************************************************************
//  Function: SwapFloatInt
//
//  Purpose:
//      Swaps two FloatInts.
//
//  Programmer: Hank Childs
//  Creation:   September 13, 2010
//
// ****************************************************************************

inline void
SwapFloatInt(FloatInt *ptr, int idx1, int idx2)
{
    int              totalSize = 2*globalNDims+1;
    static int       vecSize = 0;
    static FloatInt *tmp = NULL;
    if (tmp == NULL || vecSize != totalSize)
    {
        if (tmp != NULL)
            delete [] tmp;
        tmp = new FloatInt[totalSize];
        vecSize = totalSize;
    }
    
    memcpy(tmp, ptr+idx1*totalSize, sizeof(FloatInt)*totalSize);
    memcpy(ptr+idx1*totalSize, ptr+idx2*totalSize, sizeof(FloatInt)*totalSize);
    memcpy(ptr+idx2*totalSize, tmp, sizeof(FloatInt)*totalSize);
}


// ****************************************************************************
//  Function: SortAroundPivot
//
//  Purpose:
//      Sorts all members of a list around a pivot.
//
//  Programmer: Hank Childs
//  Creation:   September 13, 2010
//
// ****************************************************************************

static int
SortAroundPivot(FloatInt *list, int size, int pivotindex)
{
    int totalSize = 2*globalNDims+1;
    SwapFloatInt(list, pivotindex, size-1);
    int storeindex = 0;
    for (int i = 0 ; i < size-1 ; i++)
    {
        int idx1=totalSize*i;
        int idx2=totalSize*(size-1);
        if (CompareFloatInt(list+idx1, list+idx2) <= 0)
        {
            SwapFloatInt(list, storeindex, i);
            storeindex++;
        }
    }
    SwapFloatInt(list, size-1, storeindex);
    return storeindex;
}

// ****************************************************************************
//  Function: QuickSelect
//
//  Purpose:
//      Applies the QuickSelect algorithm to a list.  The list is of size 'size'
//      and it finds the 'k'th biggest element.  At the end, everything smaller
//      than the 'k'th element is at smaller indices; everything greater than
//      at bigger indices.
//
//  Programmer: Hank Childs
//  Creation:   September 13, 2010
//
// ****************************************************************************

static void
QuickSelect(FloatInt *list, int size, int k)
{
    int totalSize = 2*globalNDims+1;
    int pc1 = 0;
    int pc2 = size-1;
    int pc3 = (pc1+pc2)/2;
    // To index into "list", we need to multiply each of our candidate
    // indices by the size of an entry.
    pc1 *= totalSize;
    pc2 *= totalSize;
    pc3 *= totalSize;
    int pivotIndex = -1;
    if ((CompareFloatInt(list+pc2,list+pc1) <= 0) &&
        (CompareFloatInt(list+pc1,list+pc3) <= 0))
        pivotIndex = pc1;
    else if ((CompareFloatInt(list+pc3,list+pc1) <= 0) &&
        (CompareFloatInt(list+pc1,list+pc2) <= 0))
        pivotIndex = pc1;
    else if ((CompareFloatInt(list+pc1,list+pc2) <= 0) &&
        (CompareFloatInt(list+pc2,list+pc3) <= 0))
        pivotIndex = pc2;
    else if ((CompareFloatInt(list+pc3,list+pc2) <= 0) &&
        (CompareFloatInt(list+pc2,list+pc1) <= 0))
        pivotIndex = pc2;
    else if ((CompareFloatInt(list+pc1,list+pc3) <= 0) &&
        (CompareFloatInt(list+pc3,list+pc2) <= 0))
        pivotIndex = pc3;
    else
        pivotIndex = pc3;
    
    // I guess we could leave this multiplied, but it seems better to return
    // it to index form.
    pivotIndex /= totalSize;

    int numSmaller = SortAroundPivot(list, size, pivotIndex);
    if (k == numSmaller)
        return;

    if (k < numSmaller)
        QuickSelect(list, numSmaller, k);
    else
        QuickSelect(list+totalSize*(numSmaller+1), size-numSmaller-1, k-numSmaller-1);
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
//  Modifications:
//
//    Hank Childs, Mon Jun 27 09:01:01 PDT 2005
//    Re-written to use qsort.
//
//    Hank Childs, Mon Sep 13 18:54:21 PDT 2010
//    Re-written to use QuickSelect.
//
// ****************************************************************************

void
avtIntervalTree::ConstructTree(void)
{
    int t1 = visitTimer->StartTimer();

    int    i, j;

    //
    // Make a local copy of the bounds so that we can move them around.
    //
    int totalSize = vectorSize+1;
    globalNDims = nDims;
    FloatInt  *bounds = new FloatInt[nElements*totalSize];
    for (i = 0 ; i < nElements ; i++)
    {
        for (j = 0 ; j < vectorSize ; j++)
            bounds[totalSize*i+j].f = nodeExtents[vectorSize*i+j];
        bounds[totalSize*i+(totalSize-1)].b = i;
    }

    int      offsetStack[100];   // Only need log nElements
    int      nodeStack  [100];   // Only need log nElements
    int      sizeStack  [100];   // Only need log nElements
    int      depthStack [100];   // Only need log nElements
    int      stackCount = 0;

    //
    // Initialize the arguments for the stack
    //
    offsetStack[0]  = 0;
    sizeStack  [0]  = nElements;
    depthStack [0]  = 0;
    nodeStack  [0]  = 0;
    ++stackCount;

    int currentOffset, currentSize, currentDepth, leftSize, currentNode;
    int count = 0;
    while (stackCount > 0)
    {
        count++;
        --stackCount;
        currentOffset = offsetStack[stackCount];
        currentSize   = sizeStack  [stackCount];
        currentDepth  = depthStack [stackCount];
        currentNode   = nodeStack  [stackCount];

        if (currentSize <= 1)
        {
            nodeIDs[currentNode] = 
                             bounds[currentOffset*totalSize + (totalSize-1)].b;
            for (int j = 0 ; j < vectorSize ; j++)
                nodeExtents[currentNode*vectorSize + j] =
                                         bounds[currentOffset*totalSize + j].f;
            if (accelerateSizeQueries)
                numElementsBeneathThisNode[currentNode] = 1;
            continue;
        }

        globalCurrentDepth = currentDepth;
        leftSize = SplitSize(currentSize);
        if (optimizeForRepeatedQueries)
            QuickSelect(bounds + currentOffset*totalSize, currentSize, leftSize);

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

    delete [] bounds;
    visitTimer->StopTimer(t1, "Constructing interval tree");
}


// ****************************************************************************
//  Function: CompareFloatInt
//
//  Purpose:
//      Sorts a vector of bounds with respect to the dimension that
//      corresponds to the depth.
//
//  Notes:
//     Adapted from deprecated methods 'Sort' and 'Less' written in 
//     August of 2000.
//
//  Programmer: Hank Childs
//  Creation:   June 27, 2005
//
//  Modifications:
//
//    Mark C. Miller, Wed Aug 23 08:53:58 PDT 2006
//    Changed return values from true/false to 1, -1, 0
//
//    Hank Childs, Mon Sep 13 19:01:26 PDT 2010
//    Rename method; we now use it for QuickSelect, not qsort.
//
// ****************************************************************************

int
CompareFloatInt(const FloatInt *A, const FloatInt *B)
{
    //
    // Walk through the mid-points of the extents, starting with the current
    // depth and moving forward.
    //
    int vectorSize = 2*globalNDims;
    for (int i = 0 ; i < globalNDims ; i++)
    {
        double A_mid = (A[(2*globalCurrentDepth + 2*i) % vectorSize].f
                       + A[(2*globalCurrentDepth+1 + 2*i) % vectorSize].f) / 2;
        double B_mid = (B[(2*globalCurrentDepth + 2*i) % vectorSize].f
                       + B[(2*globalCurrentDepth+1 + 2*i) % vectorSize].f) / 2;
        if (A_mid < B_mid)
            return -1;
        else if (A_mid > B_mid)
            return +1;
    }

    //
    // Bounds are exactly identical.  This can cause problems if you are
    // not careful.
    //
    return 0;
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
//  Modifications:
//
//    Hank Childs, Mon Sep 13 19:01:26 PDT 2010
//    Set up the acceleration arrays if requested.
//
//    Jeremy Brennan, Mon Dec 12 14:51:00 PDT 2016
//    Added mutex lock for threaded operation.
//    nodeExtents is a shared class member, simultaneous writes in threaded
//    operations cause data corruption.
//
// ****************************************************************************

void
avtIntervalTree::SetIntervals()
{
    VisitMutexLock("avtIntervalTree::SetIntervals");
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
        if (accelerateSizeQueries)
            numElementsBeneathThisNode[parent] = 
                  numElementsBeneathThisNode[i-1]+
                  numElementsBeneathThisNode[i];
    }
    VisitMutexUnlock("avtIntervalTree::SetIntervals");
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
//  Method: avtIntervalTree::GetElementsList
//
//  Purpose:
//      Takes in a linear equation and determines which elements have values
//      that satisfy the equation.
//      The equation is of the form:  params[0]*x + params[1]*y ... = solution
//
//  Arguments:
//      params        The coefficients of the linear equation.
//      solution      The right hand side (solution) of the linear equation.
//      list          The list of elements that satisfy the linear equation.
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
avtIntervalTree::GetElementsList(const double *params, double solution,
                                intVector &list) const
{
    if (hasBeenCalculated == false)
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    list.clear();

    int nodeStack[100]; // Only need log amount
    int nodeStackSize = 0;

    //
    // Populate the stack by putting on the root element.  This element contains
    // all the other elements in its extents.
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
//  Method: avtIntervalTree::GetElementsListFromRange
//
//  Purpose:
//      Takes in a linear equation and determines which elements have values
//      that satisfy the equation.
//      The equation is of the form:  params[0]*x + params[1]*y ... = solution
//
//  Arguments:
//      params        The coefficients of the linear equation.
//      solution      The right hand side (solution) of the linear equation.
//      list          The list of elements that satisfy the linear equation.
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
avtIntervalTree::GetElementsListFromRange(const double *min_vec,
                                 const double *max_vec, intVector &list) const
{
    if (hasBeenCalculated == false)
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    list.clear();
 
    int nodeStack[100]; // Only need log amount
    int nodeStackSize = 0;
 
    //
    // Populate the stack by putting on the root element.  This element contains
    // all the other element in its extents.
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
            double min_extent = min_vec[i];
            double max_extent = max_vec[i];
            double min_node   = nodeExtents[stackIndex*nDims*2 + 2*i];
            double max_node   = nodeExtents[stackIndex*nDims*2 + 2*i+1];
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
//  Method: avtIntervalTree::GetNumberOfElementsInRange
//
//  Purpose:
//      Takes in a range and determines how manuy elements have values
//      that are within the range.
//
//  Arguments:
//      params        The coefficients of the linear equation.
//      solution      The right hand side (solution) of the linear equation.
//
//  Programmer: Hank Childs
//  Creation:   August 22, 2010
//
// ****************************************************************************
 
int
avtIntervalTree::GetNumberOfElementsInRange(const double *min_vec,
                                            const double *max_vec) const
{
    int numMatches = 0;

    if (hasBeenCalculated == false)
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    int nodeStack[100]; // Only need log amount
    int nodeStackSize = 0;
 
    //
    // Populate the stack by putting on the root element.  This element contains
    // all the other element in its extents.
    //
    nodeStack[0] = 0;
    nodeStackSize++;
 
    while (nodeStackSize > 0)
    {
        nodeStackSize--;
        int stackIndex = nodeStack[nodeStackSize];
        bool inBlock = true;
        bool fullyInBlock = true;
        for (int i = 0 ; i < nDims ; i++)
        {
            double min_extent = min_vec[i];
            double max_extent = max_vec[i];
            double min_node   = nodeExtents[stackIndex*nDims*2 + 2*i];
            double max_node   = nodeExtents[stackIndex*nDims*2 + 2*i+1];
            if (min_node > max_extent)
                inBlock = false;
            if (max_node < min_extent)
                inBlock = false;
            if (min_node < min_extent)
                fullyInBlock = false;
            if (max_node > max_extent)
                fullyInBlock = false;
            if (!inBlock)
                break;
        }
        if (inBlock)
        {
            if (fullyInBlock && accelerateSizeQueries)
            {
                numMatches += numElementsBeneathThisNode[stackIndex];
            }
            else if (nodeIDs[stackIndex] < 0)
            {
                //
                // The equation has a solution contained by the current extents.
                // But this is not a leaf, so put children on stack
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
                numMatches++;
            }
        }
    }

    return numMatches;
}


// ****************************************************************************
//  Method: avtIntervalTree::GetElementsFromAxiallySymmetricLineIntersection
//
//  Purpose:
//      This test will only work with 2D items and 
//      intersections with a 3D line.  The idea is that the terms are
//      revolved into 3D and we need to figure out which items intersect
//      with a line when revolved into 3D.
//
//  Arguments:
//      P1         A point on the line.
//      D1         The direction of the line.
//      list       The list of items that satisfy the linear equation.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2006
//
// ****************************************************************************

void
avtIntervalTree::GetElementsFromAxiallySymmetricLineIntersection(
                                const double *P1, const double *D1,
                                std::vector<int> &list) const
{
    if (hasBeenCalculated == false)
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    if (nDims != 2)
    {
        EXCEPTION0(ImproperUseException);
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
        if (AxiallySymmetricLineIntersection(P1, D1, stackIndex, nodeExtents))
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
Intersects(const double *params, double solution, int block, int nDims,
           const double *nodeExtents)
{
    int  i;

    int  numEncodings = 1;
    for (i = 0 ; i < nDims ; i++)
    {
        numEncodings *= 2;
    }

    double  valAtMin  = EquationsValueAtPoint(params, block, 0, nDims,
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
        double solutionAtI = EquationsValueAtPoint(params, block, i, nDims,
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
//  Function: AxiallySymmetricLineIntersection
//
//  Purpose:
//      Determine if the 2D bounding box of the current item, if revolved into
//      3D, will intersect the specified line.
//
//  Arguments:
//      P1           A point on the line
//      D1           A direction on the line
//      block        The block in the nodeExtents that should be checked for an
//                   intersection.
//      nodeExtents  The extents at each node.
//
//  Returns:    true if there is an intersection, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2006
//
// ****************************************************************************

bool
AxiallySymmetricLineIntersection(const double *P1, const double *D1,
                                 int block, const double *nodeExtents)
{
    double Zmin = nodeExtents[4*block];
    double Zmax = nodeExtents[4*block+1];
    double Rmin = nodeExtents[4*block+2];
    double Rmax = nodeExtents[4*block+3];

    //
    // We are solving for "t" in two inequalities:
    //  Zmin <= P1[2] + t*D1[2] <= Zmax
    //  Rmin <= (P1[0] + t*D1[0])^2 + (P1[1] + t*D1[1])^2 <= Rmax
    //
    // In both cases, we solve for "t" (sorting out special cases as we go)
    // and see if the line intersects the cylinder.
    //

    //
    // Solve for 't' with the inequality focusing on 'Z'.
    //
    double tRangeFromZ[2];
    if (D1[2] == 0)
    {
        if (Zmin <= P1[2] && P1[2] <= Zmax)
        {
            tRangeFromZ[0] = -10e30;
            tRangeFromZ[1] = +10e30;
        }
        else
            return false;
    }
    else
    {
        double t1 = (Zmin-P1[2]) / D1[2];
        double t2 = (Zmax-P1[2]) / D1[2];
        tRangeFromZ[0] = (t1 < t2 ? t1 : t2);
        tRangeFromZ[1] = (t1 > t2 ? t1 : t2);
    }

    //
    // Solve for 't' with the inequality focusing on 'R'.
    //
    // Expanding out gives:
    //    Rmin^2 <= At^2 + Bt + C <= Rmax^2
    //    with:
    //       A = D1[0]^2 + D1[1]^2
    //       B = 2*P1[0]*D1[0] + 2*P1[1]*D1[1]
    //       C = P1[0]^2 + P1[1]^2
    //
    // Note that there is some subtlety to solving quadratic equations with
    // inequalities:
    //  C0 <= t^2 <= C1
    //  has solutions
    //    C0^0.5 <= t <=  C1^0.5
    //   -C1^0.5 <= t <= -C0^0.5
    //  
    // Here's the game plan for solving the inequalities:
    //  Find the roots for Rmin = At^2 + Bt + C
    //  This will divide the number line into 3 intervals:
    //  (-inf, R0), (R0, R1), (R1, inf)
    //  Then check to see if the intervals meet the inequality or not.
    //  Keep the intervals that meet the inequality, discard those that don't.
    //  If there are no roots, then check the interval (-inf, inf) and see
    //  if that satisfies the inequality.
    //
    //  Then repeat for the other inequality.
    //
    double r1_int1[2] = { 10e30, -10e30 };
    double r1_int2[2] = { 10e30, -10e30 };
    double A = D1[0]*D1[0] + D1[1]*D1[1];
    double B = 2*P1[0]*D1[0] + 2*P1[1]*D1[1];
    double C0 = P1[0]*P1[0] + P1[1]*P1[1];
    double C = C0-Rmin*Rmin;
    double discriminant = B*B - 4*A*C;
    if (A == 0 || discriminant < 0)
    {
        // There are no roots, so the inequality is either always true
        // or always false (for all t).  So evaluate it and see which one.
        // Use T = 0.  And if this inequality can't be true, then there are
        // no solutions, so just return.
        if (C0 >= Rmin*Rmin)
        {
            r1_int1[0] = -10e30;
            r1_int1[1] = +10e30;
        }
        else
            return false;
    }
    else
    {
        double root = sqrt(discriminant);
        double soln1 = (-B + root) / (2*A);
        double soln2 = (-B - root) / (2*A);
        if (soln1 == soln2)
        {
            // This is a parabola with its vertex on the X-axis.  So the
            // vertex is ambiguous, everything else is either one way or 
            // another.  Test a non-solution point and see whether it is
            // valid.  If so, declare the whole range valid.
            double I0 = (soln1 == 0. ? 1. : 0.);
            if (A*I0*I0 + B*I0 + C0 >= Rmin*Rmin)
            {
                r1_int1[0] = -10e30;
                r1_int1[1] = +10e30;
            }
            else if (A*soln1*soln1 + B*soln1 + C0 >= Rmin*Rmin)
            {
                r1_int1[0] = soln1;
                r1_int1[1] = soln1;
            }
        }
        else
        {
            if (soln1 > soln2)
            {
                double tmp = soln1;
                soln1 = soln2;
                soln2 = tmp;
            }

            double half = (soln2-soln1) * 0.5;
            double val = soln1 - half;
            if (A*val*val + B*val + C0 >= Rmin*Rmin)
            {
                r1_int1[0] = -10e30;
                r1_int1[1] = soln1;
                r1_int2[0] = soln2;
                r1_int2[1] = +10e30;
            }
            else
            {
                r1_int1[0] = soln1;
                r1_int1[1] = soln2;
            }
        }
    }

    double r2_int1[2] = { 10e30, -10e30 };
    double r2_int2[2] = { 10e30, -10e30 };
    C = C0-Rmax*Rmax;
    discriminant = B*B - 4*A*C;
    if (A == 0 || discriminant < 0)
    {
        // There are no roots, so the inequality is either always true
        // or always false (for all t).  So evaluate it and see which one.
        // Use T = 0.  And if this inequality can't be true, then there are
        // no solutions, so just return.
        if (C0 <= Rmax*Rmax)
        {
            r2_int1[0] = -10e30;
            r2_int1[1] = +10e30;
        }
        else
            return false;
    }
    else
    {
        double root = sqrt(discriminant);
        double soln1 = (-B + root) / (2*A);
        double soln2 = (-B - root) / (2*A);
        if (soln1 == soln2)
        {
            // This is a parabola with its vertex on the X-axis.  So the
            // vertex is ambiguous, everything else is either one way or 
            // another.  Test a non-solution point and see whether it is
            // valid.  If so, declare the whole range valid.
            double I0 = (soln1 == 0. ? 1. : 0.);
            if (A*I0*I0 + B*I0 + C0 <= Rmax*Rmax)
            {
                r2_int1[0] = -10e30;
                r2_int1[1] = +10e30;
            }
            else if (A*soln1*soln1 + B*soln1 + C0 <= Rmax*Rmax)
            {
                r2_int1[0] = soln1;
                r2_int1[1] = soln1;
            }
        }
        else
        {
            if (soln1 > soln2)
            {
                double tmp = soln1;
                soln1 = soln2;
                soln2 = tmp;
            }

            double half = (soln2-soln1) * 0.5;
            double val = soln1 - half;
            if (A*val*val + B*val + C0 <= Rmax*Rmax)
            {
                r2_int1[0] = -10e30;
                r2_int1[1] = soln1;
                r2_int2[0] = soln2;
                r2_int2[1] = +10e30;
            }
            else
            {
                r2_int1[0] = soln1;
                r2_int1[1] = soln2;
            }
        }
    }


    //
    // Now that we have t-ranges for all the inequalities, see if we
    // can find a 't' that makes them all true.  If so, we are inside
    // the cylinder.  If not, we are outside.
    //
    double t[2];

    // Try Z, R1_int1, R2_int1.
    t[0] = tRangeFromZ[0];
    t[1] = tRangeFromZ[1];
    t[0] = (t[0] < r1_int1[0] ? r1_int1[0] : t[0]);
    t[1] = (t[1] > r1_int1[1] ? r1_int1[1] : t[1]);
    t[0] = (t[0] < r2_int1[0] ? r2_int1[0] : t[0]);
    t[1] = (t[1] > r2_int1[1] ? r2_int1[1] : t[1]);
    if (t[0] <= t[1])
        return true;

    // Try Z, R1_int1, R2_int2.
    t[0] = tRangeFromZ[0];
    t[1] = tRangeFromZ[1];
    t[0] = (t[0] < r1_int1[0] ? r1_int1[0] : t[0]);
    t[1] = (t[1] > r1_int1[1] ? r1_int1[1] : t[1]);
    t[0] = (t[0] < r2_int2[0] ? r2_int2[0] : t[0]);
    t[1] = (t[1] > r2_int2[1] ? r2_int2[1] : t[1]);
    if (t[0] <= t[1])
        return true;

    // Try Z, R1_int2, R2_int1.
    t[0] = tRangeFromZ[0];
    t[1] = tRangeFromZ[1];
    t[0] = (t[0] < r1_int2[0] ? r1_int2[0] : t[0]);
    t[1] = (t[1] > r1_int2[1] ? r1_int2[1] : t[1]);
    t[0] = (t[0] < r2_int1[0] ? r2_int1[0] : t[0]);
    t[1] = (t[1] > r2_int1[1] ? r2_int1[1] : t[1]);
    if (t[0] <= t[1])
        return true;

    // Try Z, R1_int2, R2_int2.
    t[0] = tRangeFromZ[0];
    t[1] = tRangeFromZ[1];
    t[0] = (t[0] < r1_int2[0] ? r1_int2[0] : t[0]);
    t[1] = (t[1] > r1_int2[1] ? r1_int2[1] : t[1]);
    t[0] = (t[0] < r2_int2[0] ? r2_int2[0] : t[0]);
    t[1] = (t[1] > r2_int2[1] ? r2_int2[1] : t[1]);
    if (t[0] <= t[1])
        return true;
    
    //
    // None of the intervals overlap.  No 't' will make this true, so there
    // is no intersection.
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

double
EquationsValueAtPoint(const double *params, int block, int point, int nDims,
                      const double *nodeExtents)
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

    double rv = 0;
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
//      extents      A place to put the extents of the element.
//
//  Returns:         The element this leaf comes from.
//
//  Note:            The input integer is _not_ the element number.  It is the
//                   leaf number.  This is done to prevent having to search
//                   the nodeIDs list for each element.  This routine is only
//                   useful when iterating over all elements.
//
//  Programmer:      Hank Childs
//  Creation:        December 15, 2000
//
// ****************************************************************************

int
avtIntervalTree::GetLeafExtents(int leafIndex, double *extents) const
{
    int   nodeIndex = nNodes-nElements + leafIndex;
    for (int i = 0 ; i < vectorSize ; i++)
    {
        extents[i] = nodeExtents[vectorSize*nodeIndex + i];
    }

    return  nodeIDs[nodeIndex];
}


// ****************************************************************************
//  Method: avtIntervalTree::GetElementExtents
//
//  Purpose:
//      Gets the extents for a element.
//
//  Arguments:
//      elementIndex  The index of the element.
//      extents      A place to put the extents of the element.
//
//  Programmer:      Hank Childs
//  Creation:        December 15, 2000
//
// ****************************************************************************

void
avtIntervalTree::GetElementExtents(int elementIndex, double *extents) const
{
    int   startIndex = nNodes-nElements;
    for (int i = startIndex ; i < nNodes ; i++)
    {
        if (nodeIDs[i] == elementIndex)
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
//  Method: avtIntervalTree::GetElementsList
//
//  Purpose:
//      Takes in a ray origin and direction, and determines which elements have 
//      are intersected by the ray. 
//
//  Notes: Copied from GetElementLists for an equation, just changed the
//         args and the call to Intersects.
//
//  Arguments:
//      origin        The ray origin. 
//      rayDir        The ray direction.
//      list          The list of elements that satisfy the linear equation.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 19, 2003 
//
//  Modifications:
//
//    Mark C. Miller, Mon Oct 18 14:36:49 PDT 2004
//    Added code to throw an exception of the tree hasn't been calculated
//
//    Eric Brugger, Mon Jul 12 14:11:51 PDT 2010
//    I replaced the code that caluculates line box intersections.
//
// ****************************************************************************

void
avtIntervalTree::GetElementsList(double origin[3], double rayDir[3],
                                intVector &list) const
{
    if (hasBeenCalculated == false)
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    list.clear();

    //
    // Pre-calculate some values used by the line box intersection routine.
    //
    double invDirX = 1 / rayDir[0];
    double invDirY = 1 / rayDir[1];
    double invDirZ = 1 / rayDir[2];
    int signX = invDirX < 0;
    int signY = invDirY < 0;
    int signZ = invDirZ < 0;
    double originX = origin[0];
    double originY = origin[1];
    double originZ = origin[2];

    int nodeStack[100]; // Only need log amount
    int nodeStackSize = 0;

    //
    // Populate the stack by putting on the root element.  This element contains
    // all the other elements in its extents.
    //
    nodeStack[0] = 0;
    nodeStackSize++;

    if (nDims < 3)
    {
        double bounds[4] = {0, 0, 0, 0};
        while (nodeStackSize > 0)
        {
            nodeStackSize--;
            int stackIndex = nodeStack[nodeStackSize];
            for (int i = 0; i < nDims*2; i++)
                bounds[i] = nodeExtents[(stackIndex*nDims*2)+i];
            if (LineIntersectsBox2D(bounds, invDirX, invDirY,
                originX, originY, signX, signY))
            {
                //
                // The line intersects the current extents.
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
    else
    {
        while (nodeStackSize > 0)
        {
            nodeStackSize--;
            int stackIndex = nodeStack[nodeStackSize];
            double *bounds = nodeExtents + (stackIndex*nDims*2);
            if (LineIntersectsBox(bounds, invDirX, invDirY, invDirZ,
                originX, originY, originZ, signX, signY, signZ))
            {
                //
                // The line intersects the current extents.
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
}


// ****************************************************************************
//  Method: avtIntervalTree::GetElementsListFromRay
//
//  Purpose:
//    Takes in a ray origin and direction, and determines which elements 
//    are intersected by the ray. 
//
//  Notes: Copied from GetElementsLists for an equation, just changed the
//         args and the call to Intersects.
//
//  Arguments:
//    origin    The ray origin. 
//    rayDir    The ray direction.
//    list      The list of elements that satisfy the linear equation.
//    pts       The intersection points. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 15, 2006 
//
//  Modifications:
//
// ****************************************************************************

void
avtIntervalTree::GetElementsListFromRay(double origin[3], double rayDir[3],
                          intVector &list, doubleVector &pts) const
{
    if (hasBeenCalculated == false)
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    double isect[3];
    list.clear();

    int nodeStack[100]; // Only need log amount
    int nodeStackSize = 0;

    //
    // Populate the stack by putting on the root element.  This element contains
    // all the other elements in its extents.
    //
    nodeStack[0] = 0;
    nodeStackSize++;

    while (nodeStackSize > 0)
    {
        nodeStackSize--;
        int stackIndex = nodeStack[nodeStackSize];
        if (IntersectsWithRay(origin, rayDir, stackIndex, nDims, nodeExtents,
                              isect))
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
                pts.push_back(isect[0]);
                pts.push_back(isect[1]);
                pts.push_back(isect[2]);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtIntervalTree::GetElementsListFromLine
//
//  Purpose:
//    Takes in line endpoints, determines which elements 
//    are intersected by the line. 
//
//  Notes: Copied from GetElementsLists for an equation, just changed the
//         args and the call to Intersects.
//
//  Arguments:
//    pt1       The first endpoint.
//    pt2       The second endpoint.
//    list      The list of elements that satisfy the linear equation.
//    pts       The intersection points. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 14, 2006 
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Jun 11 08:25:03 PDT 2009
//    Added optional tolerance argument, to be passed to PointsEqual method.
//
// ****************************************************************************

void
avtIntervalTree::GetElementsListFromLine(double pt1[3], double pt2[3],
                          intVector &list, doubleVector &pts, 
                          const double *tol) const
{
    if (hasBeenCalculated == false)
    {
        EXCEPTION0(IntervalTreeNotCalculatedException);
    }

    double isect[3];
    double dir[3] = {pt2[0] - pt1[0], pt2[1]-pt1[1], pt2[2]-pt1[2]};
    list.clear();

    int nodeStack[100]; // Only need log amount
    int nodeStackSize = 0;

    //
    // Populate the stack by putting on the root element.  This element contains
    // all the other elements in its extents.
    //
    nodeStack[0] = 0;
    nodeStackSize++;

    while (nodeStackSize > 0)
    {
        nodeStackSize--;
        int stackIndex = nodeStack[nodeStackSize];
        if (IntersectsWithRay(pt1, dir, stackIndex, nDims, nodeExtents, isect))
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
                double isect2[3];
                if (IntersectsWithLine(pt1, pt2, stackIndex, nDims, 
                                       nodeExtents, isect))
                {  
                    int suc2 = IntersectsWithLine(pt2, pt1, stackIndex, 
                                              nDims, nodeExtents, isect2);

                    if (suc2 && 
                        !vtkVisItUtility::PointsEqual(isect, isect2, tol))
                    {
                        list.push_back(nodeIDs[stackIndex]);
                        pts.push_back(isect[0]);
                        pts.push_back(isect[1]);
                        pts.push_back(isect[2]);
                    } // if 
                } // if 
            } // else
        } // If IntersectsRay
    } // while
}


// ****************************************************************************
//  Function: IntersectsWithRay
//
//  Purpose:
//    Determine if the range of values for the current node is intersected 
//    by the line designated by origin and rayDir. 
//
//  Arguments:
//    origin    The origin of the ray. 
//    rayDir    The xyz components of the ray direction.
//    block     The block in the nodeExtents that should be checked for an
//              intersection.
//    nDims     The number of dimensions of the var.
//    nodeExtents  The extents at each node.
//    isect     A place to store the intersection point of the line with the
//              node. 
//
//  Returns:    true if there is an intersection, false otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 14, 2006 
//
//  Modifications:
//    Kathleen Bonnell, Mon Sep 11 09:31:14 PDT 2006
//    Removed unsued ivars.
//
// ****************************************************************************

bool
IntersectsWithRay(double origin[3], double rayDir[3], int block, 
           int nDims, const double *nodeExtents, double isect[3])
{
    double bnds[6] = { 0., 0., 0., 0., 0., 0.};

    for (int i = 0; i < nDims; i++)
    {
        bnds[2*i] = nodeExtents[block*nDims*2 + 2*i];
        bnds[2*i+1] = nodeExtents[block*nDims*2 + 2*i + 1];
    }

    if (vtkCellIntersections::IntersectBox(bnds, origin, rayDir, isect))
    {
        return true;
    }
    else 
        return false;
}


// ****************************************************************************
//  Function: IntersectsWithLine
//
//  Purpose:
//    Determine if the range of values for the current node is intersected 
//    by the line designated by pt1 and pt2. 
//
//  Arguments:
//    pt1       The first endpoint.
//    pt2       The second endpoint
//    block     The block in the nodeExtents that should be checked for an
//              intersection.
//    nDims     The number of dimensions of the var.
//    nodeExtents  The extents at each node.
//    isect     A place to store the intersection point of the line with the
//              node. 
//
//  Returns:    true if there is an intersection, false otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 14, 2006 
//
//  Modifications:
//    Kathleen Bonnell, Mon Sep 11 09:31:14 PDT 2006
//    Removed unsued ivars.
//
// ****************************************************************************

bool
IntersectsWithLine(double pt1[3], double pt2[3], int block,  int nDims,
           const double *nodeExtents, double isect[3])
{
    double bnds[6] = { 0., 0., 0., 0., 0., 0.};

    for (int i = 0; i < nDims; i++)
    {
        bnds[2*i] = nodeExtents[block*nDims*2 + 2*i];
        bnds[2*i+1] = nodeExtents[block*nDims*2 + 2*i + 1];
    }

    if (vtkCellIntersections::LineIntersectBox(bnds,pt1, pt2, isect))
    {
        return true;
    }
    else 
        return false;
}


// ****************************************************************************
//  Function: LineIntersectsBox2D
//
//  Purpose:
//      Used to determine if a line intersects an axis aligned 2D box.
//
//  Returns:    true if line intersects the box, false otherwise.
//
//  Programmer: Eric Brugger
//  Creation:   July 12, 2010 
//
// ****************************************************************************

bool
LineIntersectsBox2D(double *bounds, double invDirX, double invDirY,
    double originX, double originY, int signX, int signY)
{
    double tmin, tmax, tymin, tymax;

    tmin  = (bounds[signX]   - originX) * invDirX;
    tmax  = (bounds[1-signX] - originX) * invDirX;
    tymin = (bounds[signY+2]   - originY) * invDirY;
    tymax = (bounds[1-signY+2] - originY) * invDirY;
    if ( (tmin > tymax) || (tymin > tmax) )
        return false;
    return true;
}


// ****************************************************************************
//  Function: LineIntersectsBox
//
//  Purpose:
//      Used to determine if a line intersects an axis aligned box.
//
//  Returns:    true if line intersects the box, false otherwise.
//
//  Programmer: Eric Brugger
//  Creation:   July 12, 2010 
//
// ****************************************************************************

bool
LineIntersectsBox(double *bounds,
    double invDirX, double invDirY, double invDirZ,
    double originX, double originY, double originZ,
    int signX, int signY, int signZ)
{
    double tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin  = (bounds[signX]   - originX) * invDirX;
    tmax  = (bounds[1-signX] - originX) * invDirX;
    tymin = (bounds[signY+2]   - originY) * invDirY;
    tymax = (bounds[1-signY+2] - originY) * invDirY;
    if ( (tmin > tymax) || (tymin > tmax) )
        return false;
    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;
    tzmin = (bounds[signZ+4]   - originZ) * invDirZ;
    tzmax = (bounds[1-signZ+4] - originZ) * invDirZ;
    if ( (tmin > tzmax) || (tzmin > tmax) )
        return false;
    return true;
}
