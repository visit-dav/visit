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
//                    avtMultiResolutionPartitionStrategy.C                  //
// ************************************************************************* //

#include <avtMultiResolutionPartitionStrategy.h>

#include <visitstream.h>


// ****************************************************************************
//  Method: avtMultiResolutionPartitionStrategy constructor
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2004
//
// ****************************************************************************

avtMultiResolutionPartitionStrategy::avtMultiResolutionPartitionStrategy()
{
    nLevels = 0;
    reg_desc = NULL;
}


// ****************************************************************************
//  Method: avtMultiResolutionPartitionStrategy destructor
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2004
//
// ****************************************************************************

avtMultiResolutionPartitionStrategy::~avtMultiResolutionPartitionStrategy()
{
    FreeUpMemory();
}


// ****************************************************************************
//  Method: avtMultiResolutionPartitionStrategy::FreeUpMemory
//
//  Purpose:
//      Frees up memory.
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2005
//
// ****************************************************************************

void
avtMultiResolutionPartitionStrategy::FreeUpMemory(void)
{
    if (reg_desc != NULL)
    {
        for (int i = 0 ; i < nLevels ; i++)
            delete [] reg_desc[i];
        delete [] reg_desc;
    }
    reg_desc = NULL;
    tmpBox.clear();
}


// ****************************************************************************
//  Method: avtMultiResolutionPartitionStrategy::ConstructPartition
//
//  Purpose:
//      Constructs a partition for the structured mesh chunker, using a 
//      multiple resolution strategy.
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2005
//
// ****************************************************************************

void
avtMultiResolutionPartitionStrategy::ConstructPartition(const int *cell_dims,
                             avtStructuredMeshChunker::ZoneDesignation *d_plus,
                             std::vector<int> &boxes)
{
    FreeUpMemory();  // In case we called this module before.
    Level0Initialize(cell_dims, d_plus);
    SearchFromCorners();
    FinalInitialize();

    int cell = 0;
    FindBoxes(nLevels-1, cell);
    boxes = tmpBox;

    FreeUpMemory();  // No reason to hold onto this memory.
}

bool
avtMultiResolutionPartitionStrategy::GrowBox(int *box, int axis, int direction)
{
    if (axis == 0)
    {
        int newI = (direction < 0 ? box[0] - 1 : box[1] + 1);
        for (int k = box[4] ; k <= box[5] ; k++)
            for (int j = box[2] ; j <= box[3] ; j++)
            {
                int index = k*full_dim_size*full_dim_size + j*full_dim_size
                          + newI;
                if (!reg_desc[0][index].allOn)
                    return false;
            }
        if (direction < 0)
            box[0] -= 1;
        else
            box[1] += 1;
    }
    else if (axis == 1)
    {
        int newJ = (direction < 0 ? box[2] - 1 : box[3] + 1);
        for (int k = box[4] ; k <= box[5] ; k++)
            for (int i = box[0] ; i <= box[1] ; i++)
            {
                int index = k*full_dim_size*full_dim_size 
                          + newJ*full_dim_size + i;
                if (!reg_desc[0][index].allOn)
                    return false;
            }
        if (direction < 0)
            box[2] -= 1;
        else
            box[3] += 1;
    }
    else if (axis == 2)
    {
        int newK = (direction < 0 ? box[4] - 1 : box[5] + 1);
        for (int j = box[2] ; j <= box[3] ; j++)
            for (int i = box[0] ; i <= box[1] ; i++)
            {
                int index = newK*full_dim_size*full_dim_size 
                          + j*full_dim_size + i;
                if (!reg_desc[0][index].allOn)
                    return false;
            }
        if (direction < 0)
            box[4] -= 1;
        else
            box[5] += 1;
    }

    return true;
}


void
avtMultiResolutionPartitionStrategy::SearchFromCorners(void)
{
    int  c, d, i, j, k;
    for (c = 0 ; c < 8 ; c++)
    {
        int i_step = 0;
        int j_step = 0;
        int k_step = 0;
        int i_start = 0;
        int j_start = 0;
        int k_start = 0;
        if (c & 1)
        {
            i_step = -1;
            i_start = full_dim_size-1;
        }
        else
        {
            i_step = +1;
            i_start = 0;
        }
        if (c & 2)
        {
            j_step = -1;
            j_start = full_dim_size-1;
        }
        else
        {
            j_step = +1;
            j_start = 0;
        }
        if (c & 4)
        {
            k_step = -1;
            k_start = full_dim_size-1;
        }
        else
        {
            k_step = +1;
            k_start = 0;
        }

        int index = k_start * full_dim_size*full_dim_size 
                    + j_start*full_dim_size + i_start;
        if (!reg_desc[0][index].allOn)
            continue;

        int box[6];
        box[0] = i_start;
        box[1] = i_start;
        box[2] = j_start;
        box[3] = j_start;
        box[4] = k_start;
        box[5] = k_start;
        bool canGrowI = true;
        bool canGrowJ = true;
        bool canGrowK = true;
        bool canGrow = true;
        for (d = 0 ; d < full_dim_size-1 && canGrow ; d++)
        {
            if (canGrowI)
                canGrowI = GrowBox(box, 0, i_step);
            if (canGrowJ)
                canGrowJ = GrowBox(box, 1, j_step);
            if (canGrowK)
                canGrowK = GrowBox(box, 2, k_step);
            canGrow = canGrowI || canGrowJ || canGrowK;
        }
        int box_size = (box[1] - box[0] + 1) * (box[3] - box[2] + 1)
                     * (box[5] - box[4] + 1);
        if (box_size >= 1)
        {
            for (i = box[0] ; i <= box[1] ; i++)
                for (j = box[2] ; j <= box[3] ; j++)
                    for (k = box[4] ; k <= box[5] ; k++)
                    {
                        int index = k * full_dim_size*full_dim_size 
                                    + j*full_dim_size + i;
                        reg_desc[0][index].someOn = false;
                        reg_desc[0][index].allOn = false;
                    }
            box[1] = (box[1]+1 > orig_dims[0] ? orig_dims[0] : box[1]+1);
            box[3] = (box[3]+1 > orig_dims[1] ? orig_dims[1] : box[3]+1);
            box[5] = (box[5]+1 > orig_dims[2] ? orig_dims[2] : box[5]+1);
            AddBox(box);
        }
    }
}

void
avtMultiResolutionPartitionStrategy::GetSubCells(int level, int cell,
                                                 int *subcells)
{
    int d = dims_per_level[level];
    int cI = cell % d;
    int cJ = (cell / d) % d;
    int cK = cell / (d * d);
    int d2 = dims_per_level[level-1];
    for (int i = 0 ; i < 8 ; i++)
    {
        int cI2 = (i & 1 ? 2*cI + 1 : 2*cI);
        int cJ2 = (i & 2 ? 2*cJ + 1 : 2*cJ);
        int cK2 = (i & 4 ? 2*cK + 1 : 2*cK);
        subcells[i] = cK2*d2*d2 + cJ2*d2 + cI2;
    }
}


void
avtMultiResolutionPartitionStrategy::GetExtents(int level, int cell,
                                                int *extents)
{
    int d = dims_per_level[level];
    int cI = cell % d;
    int cJ = (cell / d) % d;
    int cK = cell / (d * d);
    int r = cells_represented[level];
    extents[0] = cI*r;
    extents[1] = (cI+1)*r;
    extents[1] = (extents[1] > orig_dims[0] ? orig_dims[0] : extents[1]);
    extents[2] = cJ*r;
    extents[3] = (cJ+1)*r;
    extents[3] = (extents[3] > orig_dims[1] ? orig_dims[1] : extents[3]);
    extents[4] = cK*r;
    extents[5] = (cK+1)*r;
    extents[5] = (extents[5] > orig_dims[2] ? orig_dims[2] : extents[5]);
}


void 
avtMultiResolutionPartitionStrategy::Make2x1Line(int level, int c1, int c2)
{
    int extents[6];
    GetExtents(level, c1, extents);

    int e2[6];
    GetExtents(level, c2, e2);
    extents[0] = (extents[0] < e2[0] ? extents[0] : e2[0]);
    extents[1] = (extents[1] > e2[1] ? extents[1] : e2[1]);
    extents[2] = (extents[2] < e2[2] ? extents[2] : e2[2]);
    extents[3] = (extents[3] > e2[3] ? extents[3] : e2[3]);
    extents[4] = (extents[4] < e2[4] ? extents[4] : e2[4]);
    extents[5] = (extents[5] > e2[5] ? extents[5] : e2[5]);

    AddBox(extents);
}


void 
avtMultiResolutionPartitionStrategy::Make2x2Box(int level, int *cellids)
{
    int extents[6];
    GetExtents(level, cellids[0], extents);
    for (int i = 1 ; i < 4 ; i++)
    {
        int e2[6];
        GetExtents(level, cellids[i], e2);
        extents[0] = (extents[0] < e2[0] ? extents[0] : e2[0]);
        extents[1] = (extents[1] > e2[1] ? extents[1] : e2[1]);
        extents[2] = (extents[2] < e2[2] ? extents[2] : e2[2]);
        extents[3] = (extents[3] > e2[3] ? extents[3] : e2[3]);
        extents[4] = (extents[4] < e2[4] ? extents[4] : e2[4]);
        extents[5] = (extents[5] > e2[5] ? extents[5] : e2[5]);
    }
    AddBox(extents);
}

void
avtMultiResolutionPartitionStrategy::SearchFor2x1Line(bool *allOn, bool *someOn,
                                                      int level, int *cellids)
{
    int  i;

    // Start off by identifying all the lines that go from left to right,
    // then top to bottom, then front to back.
    for (i = 0 ; i < 4 ; i++)
    {
        int i1 = 2*i;
        int i2 = 2*i+1;
        if (allOn[i1] && allOn[i2])
        {
            allOn[i1] = false;
            allOn[i2] = false;
            someOn[i1] = false;
            someOn[i2] = false;
            Make2x1Line(level, cellids[i1], cellids[i2]);
        }
    }
    for (i = 0 ; i < 4 ; i++)
    {
        int i1 = i;
        int i2 = i+4;
        if (allOn[i1] && allOn[i2])
        {
            allOn[i1] = false;
            allOn[i2] = false;
            someOn[i1] = false;
            someOn[i2] = false;
            Make2x1Line(level, cellids[i1], cellids[i2]);
        }
    }
    for (i = 0 ; i < 4 ; i++)
    {
        int i1 = (i < 2 ? i : i+2);
        int i2 = i1+2;
        if (allOn[i1] && allOn[i2])
        {
            allOn[i1] = false;
            allOn[i2] = false;
            someOn[i1] = false;
            someOn[i2] = false;
            Make2x1Line(level, cellids[i1], cellids[i2]);
        }
    }
}

void
avtMultiResolutionPartitionStrategy::SearchFor2x2Box(bool *allOn, bool *someOn,
                                                     int level, int *cellids)
{
    int  i;

    // Look for planes of 2x2.
    int leftPlane = 0, rightPlane = 0, bottomPlane = 0, topPlane = 0,
        frontPlane = 0, backPlane = 0;
    for (i = 0 ; i < 8 ; i++)
    {
        if (allOn[i])
        {
            if (i & 1)
                rightPlane++;
            else
                leftPlane++;
            if (i & 2)
                topPlane++;
            else
                bottomPlane++;
            if (i & 4)
                backPlane++;
            else
                frontPlane++;
        }
    }

    // We can only declare 1 plane of 2x2 the winner.  Note that there cannot
    // be two non-overlapping winners, since then the original cell would be
    // "all on".
    int ids[6][4] = { { 0, 2, 4, 6 }, { 1, 3, 5, 7 },
                      { 0, 1, 4, 5 }, { 2, 3, 6, 7 },
                      { 0, 1, 2, 3 }, { 4, 5, 6, 7 } };
    int idx = -1;
    if (leftPlane == 4)
        idx = 0;
    else if (rightPlane == 4)
        idx = 1;
    else if (bottomPlane == 4)
        idx = 2;
    else if (topPlane == 4)
        idx = 3;
    else if (frontPlane == 4)
        idx = 4;
    else if (backPlane == 4)
        idx = 5;
    if (idx >= 0)
    {
        int realid[4];
        for (i = 0 ; i < 4 ; i++)
        {
            allOn[ids[idx][i]] = false;
            someOn[ids[idx][i]] = false;
            realid[i] = cellids[ids[idx][i]];
        }
        
        Make2x2Box(level, realid);
    }
}

void
avtMultiResolutionPartitionStrategy::AddBox(int *extents)
{
    int box_size = (extents[1] - extents[0]) * (extents[3] - extents[2])
                 * (extents[5] - extents[4]);
    if (box_size < minimumSize)
        return;

    int s = tmpBox.size();
    tmpBox.resize(s+6);
    tmpBox[s]   = extents[0];
    tmpBox[s+1] = extents[1]-1;
    tmpBox[s+2] = extents[2];
    tmpBox[s+3] = extents[3]-1;
    tmpBox[s+4] = extents[4];
    tmpBox[s+5] = extents[5]-1;
}

void
avtMultiResolutionPartitionStrategy::FindBoxes(int level, int cell)
{
    int  i;

    RegionDescription &rd = reg_desc[level][cell];
    if (rd.allOn)
    {
        int extents[6];
        GetExtents(level, cell, extents);
        AddBox(extents);
    }
    else if (rd.someOn)
    {
        // Note: assume level > 0, because level 0 cells are always all on 
        // or all off.
        int subcells[8];
        GetSubCells(level, cell, subcells);
        bool allOn[8];
        bool someOn[8];
        for (i = 0 ; i < 8 ; i++)
        {
            allOn[i]  = reg_desc[level-1][subcells[i]].allOn;
            someOn[i] = reg_desc[level-1][subcells[i]].someOn;
        }

        SearchFor2x2Box(allOn, someOn, level-1, subcells);
        SearchFor2x1Line(allOn, someOn, level-1, subcells);
        for (i = 0 ; i < 8 ; i++)
        {
            if (someOn[i])
                FindBoxes(level-1, subcells[i]);
        }
    }
}


// ****************************************************************************
//  Method: avtMultiResolutionPartitionStrategy::Initialize
//
//  Purpose:
//      Initialize the region descriptions.
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2005
//
// ****************************************************************************

void
avtMultiResolutionPartitionStrategy::Level0Initialize(const int *dims,
                                  avtStructuredMeshChunker::ZoneDesignation *d)
{
    int  i, j, k;

    orig_dims[0] = dims[0];
    orig_dims[1] = dims[1];
    orig_dims[2] = dims[2];

    int max_dim = dims[0];
    max_dim = (max_dim > dims[1] ? max_dim : dims[1]);
    max_dim = (max_dim > dims[2] ? max_dim : dims[2]);
    
    nLevels = 0;
    int tmp = max_dim;
    while (tmp > 0)
    {
        tmp /= 2;
        nLevels++;
    }

    // See if we have a remainder issue.
    tmp = 1;
    for (i = 0 ; i < nLevels-1 ; i++)
        tmp *= 2;
    if (tmp != max_dim)
        nLevels++;

    reg_desc = new RegionDescription*[nLevels];
    int rsize = 1;
    int dim_size = 1;
    for (i = nLevels-1 ; i >= 0 ; i--) 
    {
         reg_desc[i] = new RegionDescription[rsize];
         rsize *= 8;
         dim_size *= 2;
    }
    if (nLevels-1 >= 0)
        // We got an extra factor of 2 in.
        dim_size /= 2;
    full_dim_size = dim_size;

    cells_represented.clear();
    dims_per_level.clear();
    cells_represented.resize(nLevels);
    dims_per_level.resize(nLevels);
    cells_represented[0] = 1;
    for (i = 1 ; i < nLevels ; i++)
        cells_represented[i] = 2*cells_represented[i-1];
    dims_per_level[nLevels-1] = 1;
    for (i = nLevels-2 ; i >= 0 ; i--)
         dims_per_level[i] = 2*dims_per_level[i+1];

    // Initialize the first level using 'd'.
    for (k = 0 ; k < dim_size ; k++)
    {
        for (j = 0 ; j < dim_size ; j++)
        {
            for (i = 0 ; i < dim_size ; i++)
            {
                int index_new = k*dim_size*dim_size + j*dim_size + i;
                bool val = false;

                // If this is a fake point, then snap it to the closest real
                // point.
                int i2 = (i < dims[0] ? i : dims[0]-1);
                int j2 = (j < dims[1] ? j : dims[1]-1);
                int k2 = (k < dims[2] ? k : dims[2]-1);

                int index_orig = k2*dims[0]*dims[1] + j2*dims[0] + i2;
                val = (d[index_orig] == avtStructuredMeshChunker::RETAIN);

                reg_desc[0][index_new].allOn = val;
                reg_desc[0][index_new].someOn = val;
            }
        }
    }
}

void
avtMultiResolutionPartitionStrategy::FinalInitialize(void)
{
    int i, j, k, l;

    int dim_size = full_dim_size;

    // Now initialize the following levels using the previous level.
    for (l = 1 ; l < nLevels ; l++)
    {
         dim_size /= 2;
         for (k = 0 ; k < dim_size ; k++)
         {
             for (j = 0 ; j < dim_size ; j++)
             {
                 for (i = 0 ; i < dim_size ; i++)
                 {
                     int index = k*dim_size*dim_size + j*dim_size + i;
                     bool allOn = true;
                     bool someOn = false;
                     for (int s = 0 ; s < 8 ; s++)
                     {
                         int i2 = (s & 1 ? 2*i+1 : 2*i);
                         int j2 = (s & 2 ? 2*j+1 : 2*j);
                         int k2 = (s & 4 ? 2*k+1 : 2*k);
                         int old_ds = dim_size*2;
                         int idx2 = k2*old_ds*old_ds + j2*old_ds + i2;
                         allOn  = (!reg_desc[l-1][idx2].allOn ? false : allOn);
                         someOn = reg_desc[l-1][idx2].someOn || someOn;
                     }
                     reg_desc[l][index].allOn = allOn;
                     reg_desc[l][index].someOn = someOn;
                 }
             }
         }
    }
}
