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
//                         avtActualZoneCoordsQuery.C                        //
// ************************************************************************* //

#include <avtActualZoneCoordsQuery.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkUnsignedIntArray.h>
#include <vtkVisItUtility.h>


// ****************************************************************************
//  Method: avtActualZoneCoordsQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtActualZoneCoordsQuery::avtActualZoneCoordsQuery()
{
}


// ****************************************************************************
//  Method: avtActualZoneCoordsQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 2, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtActualZoneCoordsQuery::~avtActualZoneCoordsQuery()
{
}


// ****************************************************************************
//  Method: avtActualZoneCoordsQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   June 2, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 20 17:10:21 PDT 2004
//    Use vtkVisItUtility method to compute cell center.
//
//    Kathleen Bonnell, Thu Oct 21 15:51:04 PDT 2004 
//    Look for avtOriginalCellNumbers array in PointData if the topological
//    dimension is 0 (e.g. for Vector Plots).
//    
// ****************************************************************************

void
avtActualZoneCoordsQuery::Execute(vtkDataSet *ds, const int dom)
{
    if (ds == NULL)
    {
        return;
    }
    if (pickAtts.GetDomain() != -1 && pickAtts.GetDomain() != dom)
    {
        return;
    }

    //
    //  PickAtts::ElementNumber contains the picked zone id.
    //  Need to find the zoneId in the current data whose
    //  "original zoneId" is the samed as the picked zone.
    //

    int pickedZone = pickAtts.GetElementNumber();
    if (pickAtts.GetRealElementNumber() != -1 && 
        pickAtts.GetRealElementNumber() != pickedZone)
    {
        // A 'ghosted' id needs to be used here.
        pickedZone = pickAtts.GetRealElementNumber();
    }
    int topoDim = GetInput()->GetInfo().GetAttributes().GetTopologicalDimension();
    vtkUnsignedIntArray *origCells = vtkUnsignedIntArray::SafeDownCast(
        ds->GetCellData()->GetArray("avtOriginalCellNumbers")); 

    if (!origCells && topoDim == 0)
    {
        // 
        // This may seem strange, but for Vector plots, the array
        // gets shifted to the point data.
        // 
        origCells = vtkUnsignedIntArray::SafeDownCast(
            ds->GetPointData()->GetArray("avtOriginalCellNumbers"));
    }

    if (origCells)
    {
        int nc = origCells->GetNumberOfComponents();
        int nt = origCells->GetNumberOfTuples();
        int nels = nt * nc;
        int comp = nc -1;
        unsigned int *cellptr = origCells->GetPointer(0);
        for (int i = comp; i < nels; i+=nc)
        {
            if (cellptr[i] == pickedZone)
            {
                actualId = i/nc;
                break;
            }
        }
    }
    else
    {
        actualId = pickedZone;
    }

    if (actualId != -1)
    {
        vtkCell *cell = ds->GetCell(actualId);
        if (cell == NULL || cell->IsA("vtkEmptyCell"))
        {
            actualId = -1;
            return;
        }
        vtkVisItUtility::GetCellCenter(cell, actualCoords);
    }
}
