/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#include <PolyhedralSplit.h>
#include <vtkDataArray.h>
#include <vtkUnsignedIntArray.h>

// ****************************************************************************
// Method: PolyhedralSplit::PolyhedralSplit
//
// Purpose: 
//   Constructor
//
// Arguments:
//   nnorm : The number of normal cells.
//   npoly : The number of polyhedral cells.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 15:16:41 PST 2010
//
// Modifications:
//   
// ****************************************************************************

PolyhedralSplit::PolyhedralSplit(int nnorm, int npoly) : nodesForPolyhedralCells()
{
    polyhedralSplit = new int[2 * npoly];
    polyhedralCellCount = npoly;
    normalCellCount = nnorm;
    memset(polyhedralSplit, 0, 2 * polyhedralCellCount * sizeof(int));
}

// ****************************************************************************
// Method: PolyhedralSplit::~PolyhedralSplit
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 15:17:13 PST 2010
//
// Modifications:
//   
// ****************************************************************************

PolyhedralSplit::~PolyhedralSplit()
{
    delete [] polyhedralSplit;
}

// ****************************************************************************
// Method: PolyhedralSplit::Destruct
//
// Purpose: 
//   Static destructor function
//
// Arguments:
//   ptr : Pointer to the PolyhedralSplit object to delete.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 15:17:27 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
PolyhedralSplit::Destruct(void *ptr)
{
    PolyhedralSplit *This = (PolyhedralSplit *)ptr;
    delete This;
}

// ****************************************************************************
// Method: PolyhedralSplit::SetCellSplits
//
// Purpose: 
//   Record the cellid and number of splits for the i'th polyhedral cell.
//
// Arguments:
//   i       : The polyhedral cell index.
//   cellid  : The original cell id of the polyhedral cell.
//   nsplits : The number of splits for the polyhedral cell.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 15:17:55 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
PolyhedralSplit::SetCellSplits(int i, int cellid, int nsplits)
{
    polyhedralSplit[2*i] = cellid;
    polyhedralSplit[2*i+1] = nsplits;
}

// ****************************************************************************
// Method: PolyhedralSplit::AppendPolyhedralNode
//
// Purpose: 
//   Store a polyhedral node id.
//
// Arguments:
//   id : The id of the node to save off.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 16:50:25 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
PolyhedralSplit::AppendPolyhedralNode(int id)
{
    nodesForPolyhedralCells.push_back(id);
}

// ****************************************************************************
// Method: PolyhedralSplit::ExpandDataArray
//
// Purpose: 
//   Return a copy of the input data array with polyhedral cells' data
//   replicated the appropriate number of times.
//
// Arguments:
//   input : The input data array.
//
// Returns:    A new data array
//
// Note:       This just works for zonal data right now.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 15:19:07 PST 2010
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
PolyhedralSplit::ExpandDataArray(vtkDataArray *input, bool zoneCent) const
{
    vtkDataArray *output = input->NewInstance();
    if(zoneCent)
    {
        int bloat = 0;
        for(int i = 0; i < polyhedralCellCount; ++i)
           bloat += (polyhedralSplit[i*2+1] - 1);
        output->SetNumberOfTuples(input->GetNumberOfTuples() + bloat);
        output->SetName(input->GetName());

        vtkIdType out = 0;
        int phIndex = 0;
        for(vtkIdType cellid = 0; cellid < input->GetNumberOfTuples(); ++cellid)
        {
            if(phIndex >= polyhedralCellCount ||
               cellid < polyhedralSplit[phIndex*2])
            {
                output->SetTuple(out++, input->GetTuple(cellid));
            }
            else
            {
                int nrepeats = polyhedralSplit[phIndex*2+1];
                for(int j = 0; j < nrepeats; ++j)
                    output->SetTuple(out++, input->GetTuple(cellid));
                phIndex++;
            }
        }
    }
    else
    {
        output->SetNumberOfTuples(input->GetNumberOfTuples() + polyhedralCellCount);
        output->SetName(input->GetName());
        // Copy all of the original node data
        vtkIdType out = 0;
        for(vtkIdType nodeid = 0; nodeid < input->GetNumberOfTuples(); ++nodeid)
            output->SetTuple(out++, input->GetTuple(nodeid));

        // Create average values for the new nodes.
        double *avg = new double[input->GetNumberOfComponents()];
        for(size_t i = 0; i < nodesForPolyhedralCells.size(); )
        {
            int nnodes = nodesForPolyhedralCells[i++];
            memset(avg, 0, input->GetNumberOfComponents() * sizeof(double));
            for(int j = 0; j < nnodes; ++j)
            {
                int nodeid = nodesForPolyhedralCells[i++];
                double *val = input->GetTuple(nodeid);
                for(int c = 0; c < input->GetNumberOfComponents(); ++c)
                    avg[c] += val[c];
            }
            for(int c = 0; c < input->GetNumberOfComponents(); ++c)
                avg[c] /= double(nnodes);
            output->SetTuple(out++, avg);
        }
        delete [] avg;
    }

    return output;  
}

// ****************************************************************************
// Method: PolyhedralSplit::CreateOriginalCells
//
// Purpose: 
//   Creates an original cells array based on the polyhedral split data.
//
// Arguments:
//   domain          : The current domain number.
//   normalCellCount : The number of normal cells.
//
// Returns:    An original cells array.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 15:20:24 PST 2010
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
PolyhedralSplit::CreateOriginalCells(int domain, int normalCellCount) const
{
    vtkUnsignedIntArray *originalCells = vtkUnsignedIntArray::New();
    originalCells->SetNumberOfComponents(2);
    int bloat = 0;
    for(int i = 0; i < polyhedralCellCount; ++i)
        bloat += polyhedralSplit[i*2+1];
    originalCells->SetNumberOfTuples(normalCellCount + bloat);
    originalCells->SetName("avtOriginalCellNumbers");

    int phIndex = 0;
    unsigned int *oc = (unsigned int *)originalCells->GetVoidPointer(0);
    int allCells = normalCellCount + polyhedralCellCount;
    for(int origCell = 0; origCell < allCells; ++origCell)
    {
        oc[1] = origCell;
        if(phIndex >= polyhedralCellCount)
        {
            *oc++ = domain;
            *oc++ = origCell;
        }
        else if(origCell < polyhedralSplit[phIndex*2])
        {
            *oc++ = domain;
            *oc++ = origCell;
        }
        else
        {
            int nrepeats = polyhedralSplit[phIndex*2+1];
            for(int j = 0; j < nrepeats; ++j)
            {
                *oc++ = domain;
                *oc++ = origCell;
            }
            phIndex++;
        }
    }

    return originalCells;
}

