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
//                           IntervalTree_VTK.C                              //
// ************************************************************************* //

#include <visitstream.h>

#include <IntervalTree_VTK.h>
#include <SiloObjLib.h>


//
// Static prototypes
//

bool     Intersects(const float *, float, int, int, const float *);
float    EquationsValueAtPoint(const float *, int, int, int, const float *);


// ****************************************************************************
//  Method: IntervalTree_VTK::GetDomainsList
//
//  Purpose:
//      Takes in a linear equation and determines which domains have values
//      that satisfy the equation.
//      The equation is of the form:  params[0]*x + params[1]*y ... = solution
//
//  Returns:    The size of the list created.
//
//  Arguments:
//      params        The coefficients of the linear equation.
//      solution      The right hand side (solution) of the linear equation.
//      domainList    The list of domains that satisfy the linear equation.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 1999
//
//  Modifications:
// 
//      Hank Childs, Mon Apr  3 17:43:42 PDT 2000
//      Pushed sorting of domain list into this routine.
//
// ****************************************************************************

int
IntervalTree_VTK::GetDomainsList(const float *params, float solution,
                                 int **domainList) const
{
    *domainList = new int[nDomains];
    int domainListOffset = 0;

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
                // Leaf node, put on domainList
                //
                (*domainList)[domainListOffset] = nodeIDs[stackIndex];
                domainListOffset++;
            }
        }
    }

    //
    // Other routines need this list sorted, so do it here.
    //
    InsertionSort(*domainList, domainListOffset);

    return domainListOffset;
}


// ****************************************************************************
//  Method: IntervalTree_VTK::GetMin
//
//  Purpose:
//      Returns the minimum value in the dimension specified.
//
//  Arguments:
//      dim     The dimension to find the minimum in.
//
//  Programmer: Hank Childs
//  Creation:   April 12, 2000
//
// ****************************************************************************

float
IntervalTree_VTK::GetMin(int dim) const
{
    // 
    // Want to find the minimum at the root node, which is the 0th node in
    // the nodeExtents.
    //
    return nodeExtents[dim*2];
}


// ****************************************************************************
//  Method: IntervalTree_VTK::GetMax
//
//  Purpose:
//      Returns the maximum value in the dimension specified.
//
//  Arguments:
//      dim     The dimension to find the maximum in.
//
//  Programmer: Hank Childs
//  Creation:   April 12, 2000
//
// ****************************************************************************

float
IntervalTree_VTK::GetMax(int dim) const
{
    // 
    // Want to find the maximum at the root node, which is the 0th node in
    // the nodeExtents.
    //
    return nodeExtents[dim*2 + 1];
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
    if (valAtMin == solution)
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


