// ************************************************************************* //
//                       avtActualNodeCoordsQuery.C                          //
// ************************************************************************* //

#include <avtActualNodeCoordsQuery.h>

#include <vtkDataSet.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkVisItUtility.h>



// ****************************************************************************
//  Method: avtActualNodeCoordsQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtActualNodeCoordsQuery::avtActualNodeCoordsQuery()
{
}

// ****************************************************************************
//  Method: avtActualNodeCoordsQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 2, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtActualNodeCoordsQuery::~avtActualNodeCoordsQuery()
{
}

// ****************************************************************************
//  Method: avtActualNodeCoordsQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Kathleen Bonnell  
//  Creation:   June 2, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun 16 13:54:28 PDT 2004
//    avtOriginalNode numbers is now of type vtkIntArray.
//
// ****************************************************************************

void
avtActualNodeCoordsQuery::Execute(vtkDataSet *ds, const int dom)
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

    int pickedNode = pickAtts.GetElementNumber();

    if (pickAtts.GetRealElementNumber() != -1 &&
        pickAtts.GetRealElementNumber() != pickedNode)
    {
        // A 'ghosted' id needs to be used here.
        pickedNode = pickAtts.GetRealElementNumber();
    }

    vtkIntArray *origNodes = vtkIntArray::SafeDownCast(
        ds->GetPointData()->GetArray("avtOriginalNodeNumbers")); 

    if (origNodes)
    {
        int nc = origNodes->GetNumberOfComponents();
        int nt = origNodes->GetNumberOfTuples();
        int nels = nc*nt;
        int comp = nc -1;
        int *nodeptr = origNodes->GetPointer(0);

        for (int i = comp; i < nels; i+=nc)
        {
            if (nodeptr[i] == pickedNode)
            {
                actualId = i/nc;
                break;
            }
        }
    }
    else
    {
        actualId = pickedNode;
    }

    if (actualId >= 0 && actualId < ds->GetNumberOfPoints())
    {
        vtkVisItUtility::GetPoints(ds)->GetPoint(actualId, actualCoords);
    }
    else
    {
        actualId = -1;
    }
}
