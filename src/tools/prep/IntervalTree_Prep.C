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
//                          IntervalTree_Prep.C                              //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#ifdef PARALLEL
    #include <mpi.h>
#endif

#include <SiloObjLib.h>
#include <IntervalTree_Prep.h>


//
// Macros
//

#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y)  ((X) > (Y) ? (X) : (Y))


// 
// Declaration of static consts.  They are not allowed as initializers in
// class definitions.
//

char * const   IntervalTree_Prep::SILO_LOCATION           = "/";


// ****************************************************************************
//  Method: IntervalTree_Prep constructor
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

IntervalTree_Prep::IntervalTree_Prep()
{
    setNDims    = false;
    setNDomains = false;
    vectorSize  = 0;
}


// ****************************************************************************
//  Method: IntervalTree_Prep destructor
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

IntervalTree_Prep::~IntervalTree_Prep()
{
    // Do Nothing
}


// ****************************************************************************
//  Method: IntervalTree_Prep::SetFieldName
//
//  Purpose:
//      Sets the field name.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::SetFieldName(char *f)
{
    name = CXX_strdup(f);
}


// ****************************************************************************
//  Method: IntervalTree_Prep::SetNDims
//
//  Purpose:
//      Sets the number of dimensions for the variable.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::SetNDims(int n)
{
    if (setNDims == true)
    {
        return;
    }

    nDims      = n;
    vectorSize = 2*nDims;
    setNDims   = true;
    ConstructNodeExtents();
}


// ****************************************************************************
//  Method: IntervalTree_Prep::SetNDomains
//
//  Purpose:
//      Set the number of domains that the interval tree will be dealing with;
//      initialize data members.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::SetNDomains(int n)
{
    if (setNDomains == true)
    {
        return;
    }

    nDomains    = n;
    setNDomains = true;

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

    ConstructNodeExtents();
}


// ****************************************************************************
//  Method: ConstructNodeExtents
//
//  Purpose:
//      Constructs the nodeExtents data member which depends on both the 
//      nDomains and nDims data members.  
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::ConstructNodeExtents(void)
{
    if (setNDims == false || setNDomains == false)
    {
        return;
    }

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
//  Method: IntervalTree_Prep::AddVar
//
//  Purpose:
//      Adds the extents for one domain.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::AddVar(int domain, float *d)
{
    //
    // Sanity Check
    //
    if (domain < 0 || domain >= nDomains)
    {
        cerr << "Invalid domain: " << domain << endl;
        return;
    }

    for (int i = 0 ; i < vectorSize ; i++)
    {
        nodeExtents[domain*vectorSize + i] = d[i];
    }
}


// ****************************************************************************
//  Method: IntervalTree_Prep::WrapUp
//
//  Purpose:
//      Allows the interval tree to "wrap up" now that information about every
//      domain has been collected.  This primarily calls a method to construct
//      the tree.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::WrapUp(void)
{
    if (setNDims == false || setNDomains == false)
    {
        //
        // The interval tree has been declared, but has not been given data.
        // This is often the case when the variable name for the multivar 
        // and the names for the normal vars that correspond to it do not
        // match.
        //
        cerr << "Cannot process \"";
        if (name != NULL)
        {
            cerr << name;
        }
        else
        {
            cerr << "(nil)" ;
        }
        cerr << "\", there is not enough information regarding it." << endl;
        exit(EXIT_FAILURE);
    }

    CollectInformation();
    ConstructTree();
}


// ****************************************************************************
//  Method: IntervalTree_Prep::CollectInformation
//
//  Purpose:
//      If there is more than one processor, send all of the domains that were
//      read in to processor 0 so that they can be processed there.
//
//  Note:       Does nothing in the serial case.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::CollectInformation(void)
{
#ifdef PARALLEL
    // 
    // Extents are spread across all of the processors.  Since nodeExtents was
    // initialized to have value 0., we can do an MPI_SUM and get the correct
    // list on processor 0.
    //
    int totalElements = nDomains*vectorSize;
    float *outBuff = new float[totalElements];
    MPI_Reduce(nodeExtents, outBuff, totalElements, MPI_FLOAT, MPI_SUM, 0, 
               MPI_COMM_WORLD);
    
    for (int i = 0 ; i < totalElements ; i++)
    {
        nodeExtents[i] = outBuff[i];
    }
    delete [] outBuff;
#endif
}


// ****************************************************************************
//  Method: IntervalTree_Prep::ConstructTree
//
//  Purpose:
//      Constructs the complete binary tree of the nodes' extents.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::ConstructTree(void)
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
//  Method: IntervalTree_Prep method Sort
//
//  Purpose:
//      Sorts a vector of bounds with respect to the dimension that 
//      corresponds to the depth.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::Sort(float *bounds, int *blockID, int size, int depth)
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
//  Method: IntervalTree_Prep::Less
//
//  Purpose:
//      An operator that can sort a variable according to any dimension.
//
//  Arguments:
//      A     -  A vector of variables of size vectorSize.
//      B     -  A vector of variables of size vectorSize.
//      depth -  The level of this node in the interval tree.  This is used
//               to find what dimension to sort across.
//      
//  Returns:    True if A < B, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
// ****************************************************************************

bool
IntervalTree_Prep::Less(float *A, float *B, int depth)
{
    int        i;
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
//  Method: IntervalTree_Prep::SetIntervals
//
//  Purpose:
//      Now that the intervals for all of the leaf nodes have been set and put
//      in their proper place in the interval tree, calculate the bounds for
//      all of the parenting nodes.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::SetIntervals()
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
//  Method: IntervalTree_Prep::SplitSize
//
//  Purpose:
//      Determine how big the left side of the tree should be to make a 
//      complete binary tree.
//
//  Returns:      The proper size of the left side of the child tree.
//
//  Programmer:   Hank Childs
//  Creation:     December 15, 1999
//
// ****************************************************************************

int
IntervalTree_Prep::SplitSize(int size)
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
//  Method: IntervalTree_Prep::ReadVar (meshvar)
//
//  Purpose:
//      Reads a variable into the interval tree.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::ReadVar(DBmeshvar *mv, int domain)
{
    //
    // ReadVar allows for vals to have multiple array lengths.  Variables
    // do not have this, so we will just create an array that contains all
    // equal lengths.
    //
    int   nelsA[N_DIMS_LIMIT];
    for (int i = 0 ; i < mv->nvals ; i++)
    {
        nelsA[i] = mv->nels;
    }
    ReadVar(mv->vals, mv->nvals, nelsA, domain);
}


// ****************************************************************************
//  Method: IntervalTree_Prep::ReadVar (quadvar)
//
//  Purpose:
//      Reads a variable into the interval tree.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::ReadVar(DBquadvar *qv, int domain)
{
    //
    // ReadVar allows for vals to have multiple array lengths.  Variables
    // do not have this, so we will just create an array that contains all
    // equal lengths.
    //
    int   nelsA[N_DIMS_LIMIT];
    for (int i = 0 ; i < qv->nvals ; i++)
    {
        nelsA[i] = qv->nels;
    }
    ReadVar(qv->vals, qv->nvals, nelsA, domain);
}


// ****************************************************************************
//  Method: IntervalTree_Prep::ReadVar (ucdvar)
//
//  Purpose:
//      Reads a variable into the interval tree.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::ReadVar(DBucdvar *uv, int domain)
{
    //
    // ReadVar allows for vals to have multiple array lengths.  Variables
    // do not have this, so we will just create an array that contains all
    // equal lengths.
    //
    int   nelsA[N_DIMS_LIMIT];
    for (int i = 0 ; i < uv->nvals ; i++)
    {
        nelsA[i] = uv->nels;
    }
    ReadVar(uv->vals, uv->nvals, nelsA, domain);
}


// ****************************************************************************
//  Method: IntervalTree_Prep::ReadVar (pointmesh)
//
//  Purpose:
//      Reads a pointmesh into the interval tree.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::ReadVar(DBpointmesh *pm, int domain)
{
    //
    // ReadVar allows for vals to have multiple array lengths.  Pointmeshes
    // have sets of points, so number(x vals) = number(y vals), etc.  We will
    // just create an array that contains all equal lengths.
    //
    int   nelsA[N_DIMS_LIMIT];
    for (int i = 0 ; i < pm->nels ; i++)
    {
        nelsA[i] = pm->nels;
    }
    ReadVar(pm->coords, pm->ndims, nelsA, domain);
} 


// ****************************************************************************
//  Method: IntervalTree_Prep::ReadVar (quadmesh)
//
//  Purpose:
//      Reads a quadmesh into the interval tree.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::ReadVar(DBquadmesh *qm, int domain)
{
    //
    // The size of each of the coordinate arrays depends on the coordtype.
    // If it is non-collinear, then we have sets of points and each of the
    // coordinate arrays have the same length.  If it is collinear, then
    // the coordinate arrays have the lengths denoted by dims.
    //
    if (qm->coordtype == DB_NONCOLLINEAR)
    {
        //
        // Create an array that contains the size of the coordinate arrays.
        //
        int   nelsA[N_DIMS_LIMIT];
        for (int i = 0 ; i < qm->ndims ; i++)
        {
            nelsA[i] = qm->nnodes;
        }
        ReadVar(qm->coords, qm->ndims, nelsA, domain);
    }
    else
    {
        ReadVar(qm->coords, qm->ndims, qm->dims, domain);
    }
} 


// ****************************************************************************
//  Method: IntervalTree_Prep::ReadVar (ucdmesh)
//
//  Purpose:
//      Reads a ucdmesh into the interval tree.
//
//  Programmer: Hank Childs
//  Creation:   December 20, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::ReadVar(DBucdmesh *um, int domain)
{
    //
    // ReadVar allows for vals to have multiple array lengths.  Ucdmeshes
    // have equal length coordinate arrays, so we will just create an array
    // that contains all equal lengths.
    //
    int   nelsA[N_DIMS_LIMIT];
    for (int i = 0 ; i < um->ndims ; i++)
    {
        nelsA[i] = um->nnodes;
    }
    ReadVar(um->coords, um->ndims, nelsA, domain);
} 


// ****************************************************************************
//  Method: IntervalTree_Prep::ReadVar
//
//  Purpose:
//      Reads a variable into the interval tree.
//
//  Arguments:
//      vals    -  An array of arrays of floats.  
//      nvals   -  The number of arrays of floats for vals.
//      nels    -  An arrays of size nvals.  Each entry contains the length
//                 of the corresponding entry in vals.
//      domain  -  The domain that vals is associated with.
//
//  Local Variables:
//      var     -  An array of 2*N_DIMS_LIMIT.  Element 2*i contains the 
//                 minimum value for the variable in dimension i and element
//                 2*i+1 contains the maximum value for the variable in 
//                 dimension i.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::ReadVar(float **vals, int nvals, int *nels, int domain)
{
    // 
    // This may be the first time we see ndims, so make sure it is set.
    //
    SetNDims(nvals);

    float   var[N_DIMS_LIMIT*2];

    for (int dim = 0 ; dim < nvals ; dim++)
    {
        float min = vals[dim][0];
        float max = vals[dim][0];
        for (int i = 0 ; i < nels[dim] ; i++)
        {
            if (vals[dim][i] < min)
            {
                min = vals[dim][i];
            }
            if (vals[dim][i] > max)
            {
                max = vals[dim][i];
            }
        }
        var[2*dim]   = min;
        var[2*dim+1] = max;
    }
  
    AddVar(domain, var);
}


// ****************************************************************************
//  Method: IntervalTree_Prep::Write
//
//  Purpose:
//      Writes out the interval tree to a SILO file.
//
//  Programmer: Hank Childs
//  Creation:   December 15, 1999
//
// ****************************************************************************

void
IntervalTree_Prep::Write(DBfile *dbfile)
{
#ifdef PARALLEL
  extern int my_rank;
  if (my_rank != 0)
  {
      return;
  }
#endif

    // 
    // Set the directory in the file to be correct.
    //
    if (DBSetDir(dbfile, SILO_LOCATION) < 0)
    {
        cerr << "Unable to change directories in the silo file to "
             << SILO_LOCATION << "." << endl;
        exit(EXIT_FAILURE);
    }

    IntervalTree::Write(dbfile);
}


// ****************************************************************************
//  Method: IntervalTree_Prep::GetRootExtents
//
//  Purpose:
//      Gets the extents over all domains.
//
//  Arguments:
//      extents    An array to copy the extents into.
//
//  Programmer:    Hank Childs
//  Creation:      September 8, 2000
//
// ****************************************************************************

void
IntervalTree_Prep::GetRootExtents(float *extents)
{
    for (int i = 0 ; i < vectorSize ; i++)
    {
        extents[i] = nodeExtents[i];
    }
}


