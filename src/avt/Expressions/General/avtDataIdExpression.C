// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtDataIdExpression.C                            //
// ************************************************************************* //

#include <avtDataIdExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtCallback.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDataIdExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
//  Modifications:
//    Brad Whitlock, Tue Sep 10 16:15:36 PDT 2013
//    Added IJK creation.
//
// ****************************************************************************

avtDataIdExpression::avtDataIdExpression()
{
    doZoneIds = true;
    doGlobalNumbering = false;
    doIJK = false;
    doDomainIds = false;
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtDataIdExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

avtDataIdExpression::~avtDataIdExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDataIdExpression::PreExecute
//
//  Purpose:
//      Reset haveIssuedWarning, so we can issue it only once per execution.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

void
avtDataIdExpression::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtDataIdExpression::DeriveVariable
//
//  Purpose:
//      Assigns the zone ID to each variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     September 23, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Sep 10 16:15:36 PDT 2013
//    Added IJK creation.
//
//    Chris Laganella, Thu Feb  3 18:12:06 EST 2022
//    Fixed an iteration bug with IJKs
//
//    Chris Laganella, Fri Feb  4 19:29:10 EST 2022
//    Added the ability to create domain ids.
// ****************************************************************************

vtkDataArray *
avtDataIdExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkIdType nvals = 0;
    if (doZoneIds)
        nvals = in_ds->GetNumberOfCells();
    else
        nvals = in_ds->GetNumberOfPoints();

    vtkIntArray *rv = vtkIntArray::New();
    rv->SetNumberOfTuples(nvals);

    vtkDataArray *arr = NULL;
    if (doZoneIds && doGlobalNumbering)
        arr = in_ds->GetCellData()->GetArray("avtGlobalZoneNumbers");
    else if (doZoneIds && !doGlobalNumbering)
        arr = in_ds->GetCellData()->GetArray("avtOriginalCellNumbers");
    else if (!doZoneIds && doGlobalNumbering)
        arr = in_ds->GetPointData()->GetArray("avtGlobalNodeNumbers");
    else if (!doZoneIds && !doGlobalNumbering)
        arr = in_ds->GetPointData()->GetArray("avtOriginalNodeNumbers");

    if (arr == NULL)
    {
        for (vtkIdType i = 0 ; i < nvals ; i++)
            rv->SetValue(i, (int)i);
        char standard_msg[1024] = "VisIt was not able to create the requested"
                " ids.  Please see a VisIt developer.";
        char globalmsg[1024] = "VisIt was not able to create global ids, most "
                "likely because ids of this type were not stored in the file.";
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning(
                               (doGlobalNumbering ? globalmsg : standard_msg));
            haveIssuedWarning = true;
        }

        return rv;
    }

    if(doIJK)
    {
        if(doGlobalNumbering)
        {
            rv->SetNumberOfTuples(nvals);
            for (vtkIdType i = 0 ; i < nvals ; i++)
                rv->SetValue(i, (int)i);
            avtCallback::IssueWarning("Construction of global IJK indices is "
                                      "not yet supported.");
            return rv;
        }

        // Just support structured, local numbering for now.

        rv->SetNumberOfComponents(3);
        rv->SetNumberOfTuples(nvals);

        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(in_ds);
        vtkStructuredGrid  *sgrid = vtkStructuredGrid::SafeDownCast(in_ds);
        vtkDataArray *sdims = in_ds->GetFieldData()->GetArray("avtOriginalStructuredDimensions");
        if(sdims != NULL)
        {
            int dims[3] = {1,1,1};
            dims[0] = (int)sdims->GetTuple1(0);
            dims[1] = (int)sdims->GetTuple1(1);
            dims[2] = (int)sdims->GetTuple1(2);
            int origin = 0;
            if(doZoneIds)
            {
                dims[0] = std::max(dims[0]-1, 1);
                dims[1] = std::max(dims[1]-1, 1);
                dims[2] = std::max(dims[2]-1, 1);
                origin = GetInput()->GetInfo().GetAttributes().GetCellOrigin();
            }
            else
                origin = GetInput()->GetInfo().GetAttributes().GetNodeOrigin();

            int ijk[3];
            vtkIdType id, nxy = dims[0] * dims[1];
            for (vtkIdType i = 0 ; i < nvals ; i++)
            {
                vtkIdType id;
                if (arr->GetNumberOfComponents() == 2)
                    // When there are two components, they are typically packed as 
                    // <dom, id>.  We will want the second one.
                    id = (vtkIdType)arr->GetComponent(i, 1);
                else
                    id = (vtkIdType)arr->GetComponent(i, 0);

                vtkIdType offset = id % nxy;
                ijk[0] = (offset % dims[0]) + origin;
                ijk[1] = (offset / dims[0]) + origin;
                ijk[2] = (id / nxy) + origin;
                rv->SetTypedTuple(i, ijk);
            }
        }
        else if(rgrid != NULL || sgrid != NULL)
        {
            // Get the size of the local mesh.
            int dims[3] = {1,1,1};
            if(rgrid != NULL)
                rgrid->GetDimensions(dims);
            else if(sgrid != NULL)
                sgrid->GetDimensions(dims);
            int origin = 0;
            if(doZoneIds)
            {
                dims[0] = std::max(dims[0]-1, 1);
                dims[1] = std::max(dims[1]-1, 1);
                dims[2] = std::max(dims[2]-1, 1);
                origin = GetInput()->GetInfo().GetAttributes().GetCellOrigin();
            }
            else
                origin = GetInput()->GetInfo().GetAttributes().GetNodeOrigin();

            int ijk[3];
            vtkIdType id = 0;
            for(int k = 0; k < dims[2]; ++k)
            {
                ijk[2] = k + origin;
                for(int j = 0; j < dims[1]; ++j)
                {
                    ijk[1] = j + origin;
                    for(int i = 0; i < dims[0]; ++i, ++id)
                    {
                        ijk[0] = i + origin;
                        rv->SetTypedTuple(id, ijk);
                    }
                }
            }
        }
        else
        {
            // If the dataset was once structured, we're not currently going to 
            // figure out the previous IJK indices. That could come later.

            int ijk[3] = {0,0,0};
            for (vtkIdType i = 0 ; i < nvals ; i++)
            {
                if (arr->GetNumberOfComponents() == 2)
                    // When there are two components, they are typically packed as 
                    // <dom, id>.  We will want the second one.
                    ijk[0] = (int)arr->GetComponent(i, 1);
                else
                    ijk[0] = (int)arr->GetComponent(i, 0);
                rv->SetTypedTuple(i, ijk);
            }
        }
    }
    else if(doDomainIds)
    {
        rv->SetNumberOfComponents(1);
        rv->SetNumberOfTuples(nvals);
        if(arr->GetNumberOfComponents() == 2)
        {
            // Data is <dom, id>
            for(vtkIdType i = 0; i < nvals; i++)
            {
                rv->SetValue(i, (int)arr->GetComponent(i, 0));
            }
        }
        else
        {
            // No original domain id, set to 0
            debug2 << "DomainIds requested from avtDataIdExpression "
                << "but the data does not contain domain ids. Assuming 0."
                << std::endl;
            for(vtkIdType i = 0; i < nvals; i++)
            {
                rv->SetValue(i, 0);
            }
        }
    }
    else
    {
        rv->SetNumberOfTuples(nvals);

        for (vtkIdType i = 0 ; i < nvals ; i++)
        {
            if (arr->GetNumberOfComponents() == 2)
                // When there are two components, they are typically packed as 
                // <dom, id>.  We will want the second one.
                rv->SetValue(i, (int)arr->GetComponent(i, 1));
            else
                rv->SetValue(i, (int)arr->GetComponent(i, 0));
        }
    }

    return rv;
}

// ****************************************************************************
//  Method: avtDataIdExpression::GetVariableDimension
//
//  Purpose:
//      Reports the dimension of the output variable.
//
//  Programmer: Chris Laganella
//  Creation:   Mon Jan 31 19:00:15 EST 2022
//
// ****************************************************************************
int
avtDataIdExpression::GetVariableDimension()
{
    return (doIJK) ? 3 : 1;
}


// ****************************************************************************
//  Method: avtDataIdExpression::ModifyContract
//
//  Purpose:
//      Tells the input create zone ids.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

avtContract_p
avtDataIdExpression::ModifyContract(avtContract_p spec)
{
    if (doZoneIds && doGlobalNumbering)
        spec->GetDataRequest()->TurnGlobalZoneNumbersOn();
    else if (doZoneIds && !doGlobalNumbering)
        spec->GetDataRequest()->TurnZoneNumbersOn();
    else if (!doZoneIds && doGlobalNumbering)
        spec->GetDataRequest()->TurnGlobalNodeNumbersOn();
    else if (!doZoneIds && !doGlobalNumbering)
        spec->GetDataRequest()->TurnNodeNumbersOn();

    return spec;
}


