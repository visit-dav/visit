// ************************************************************************* //
//                         avtActualZoneCoordsQuery.C                        //
// ************************************************************************* //

#include <avtActualZoneCoordsQuery.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkUnsignedIntArray.h>


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
    //  PickAtts::ElementNumber contains the picked node id.
    //  Need to find the nodeId in the current data whose
    //  "original nodeId" is the samed as the picked node.
    //

    int pickedZone = pickAtts.GetElementNumber();

    if (pickAtts.GetRealElementNumber() != -1 && 
        pickAtts.GetRealElementNumber() != pickedZone)
    {
        // A 'ghosted' id needs to be used here.
        pickedZone = pickAtts.GetRealElementNumber();
    }

    vtkUnsignedIntArray *origCells = vtkUnsignedIntArray::SafeDownCast(
        ds->GetCellData()->GetArray("avtOriginalCellNumbers")); 

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
        if (cell == NULL)
        {
            actualId = -1;
            return;
        }

        float parametricCenter[3];
        float weights[28];
        int subId = cell->GetParametricCenter(parametricCenter);
        cell->EvaluateLocation(subId, parametricCenter, actualCoords, weights);
    }
}
