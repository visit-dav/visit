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
#include <map>

#include <DebugStream.h>

// ****************************************************************************
// Method: PolyhedralSplit::PolyhedralSplit
//
// Purpose: 
//   Constructor
//
// Arguments:
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 15:16:41 PST 2010
//
// Modifications:
//   Brad Whitlock, Tue Oct 26 16:27:26 PDT 2010
//   I removed the arguments and made polyhedralSplit into a vector.
//
// ****************************************************************************

PolyhedralSplit::PolyhedralSplit() : polyhedralSplit(), nodesForPolyhedralCells()
{
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
// Method: PolyhedralSplit::AppendCellSplits
//
// Purpose: 
//   Record the cellid and number of splits for the next polyhedral cell.
//
// Arguments:
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
PolyhedralSplit::AppendCellSplits(int cellid, int nsplits)
{
    polyhedralSplit.push_back(cellid);
    polyhedralSplit.push_back(nsplits);
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
//   input        : The input data array.
//   zoneCent     : True if the data is zone centered.
//   averageNodes : If true then average the node data for new polyhedral nodes.
//                  If false, use the dominant value.
//
// Returns:    A new data array
//
// Note:       This just works for zonal data right now.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 15:19:07 PST 2010
//
// Modifications:
//   Brad Whitlock, Tue Oct 26 16:23:45 PDT 2010
//   I turned polyhedralSplit into a vector. Allow for the input to be returned
//   in the case where there are now polyhedral cells. I also added a new case
//   for figuring out the new nodal values for new polyhedral nodes so we can
//   use the dominant value. This lets us handle nodal material data.
//
//   Brad Whitlock, Mon Nov 22 14:14:16 PST 2010
//   Set the component count so it works on vector data too.
//
// ****************************************************************************

vtkDataArray *
PolyhedralSplit::ExpandDataArray(vtkDataArray *input, bool zoneCent, 
    bool averageNodes) const
{
    const char *mName = "PolyhedralSplit::ExpandDataArray: ";
    vtkDataArray *output = NULL;
    int polyhedralCellCount = polyhedralSplit.size() / 2;

    if(polyhedralCellCount == 0)
    {
        debug4 << mName << "Returning input data array " << endl;
        output = input;
        output->Register(NULL);
    }
    else if(zoneCent)
    {
        debug4 << mName << "Expanding zonal data" << endl;
        output = input->NewInstance();
        int bloat = 0;
       
        for(int i = 1; i < polyhedralSplit.size(); i += 2)
           bloat += (polyhedralSplit[i] - 1);
        output->SetNumberOfComponents(input->GetNumberOfComponents());
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
        output = input->NewInstance();
        output->SetNumberOfComponents(input->GetNumberOfComponents());
        output->SetNumberOfTuples(input->GetNumberOfTuples() + polyhedralCellCount);
        output->SetName(input->GetName());
        // Copy all of the original node data
        vtkIdType out = 0;
        for(vtkIdType nodeid = 0; nodeid < input->GetNumberOfTuples(); ++nodeid)
            output->SetTuple(out++, input->GetTuple(nodeid));

        // Create average values for the new nodes.
        if(averageNodes ||
           input->GetNumberOfComponents()>1/*For ease*/)
        {
            debug4 << mName << "Expanding nodal data using averaging for the "
                "new node values" << endl;
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
        else
        {
            debug4 << mName << "Expanding nodal data using dominant value for "
                "the new node values" << endl;

            // Just 1-component values for now.
            for(size_t i = 0; i < nodesForPolyhedralCells.size(); )
            {
                int nnodes = nodesForPolyhedralCells[i++];
                // Count up the unique values.
                std::map<double,int> counts;
                std::map<double,int>::iterator it;
                for(int j = 0; j < nnodes; ++j)
                {
                    int nodeid = nodesForPolyhedralCells[i++];
                    double val = input->GetTuple1(nodeid);
                    it = counts.find(val);
                    if(it == counts.end())
                        counts[val] = 1;
                    else
                        counts[val]++;
                }

                // Figure out which is the most common value.
                it = counts.begin();
                int maxcount = it->second;
                double mostCommon = it->first;
                for(; it != counts.end(); ++it)
                {
                    if(it->second > maxcount)
                    {
                        maxcount = it->second;
                        mostCommon = it->first;
                    }
                }

                // Save the most common value as the new node value
                output->SetTuple1(out++, mostCommon);
            }
        }
    }

    debug4 << mName << "Output data has " << output->GetNumberOfTuples()
           << " tuples" << endl;

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
//   Brad Whitlock, Tue Oct 26 16:23:45 PDT 2010
//   I turned polyhedralSplit into a vector.
//   
// ****************************************************************************

vtkDataArray *
PolyhedralSplit::CreateOriginalCells(int domain, int normalCellCount) const
{
    vtkUnsignedIntArray *originalCells = vtkUnsignedIntArray::New();
    originalCells->SetNumberOfComponents(2);
    int bloat = 0;
    int polyhedralCellCount = polyhedralSplit.size()/2;
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

