// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtCurveQuery.C                             //
// ************************************************************************* //

#include <avtCurveQuery.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <avtCurveConstructorFilter.h>
#include <avtSourceFromAVTDataset.h>
#include <avtOriginatingSource.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtCurveQuery::avtCurveQuery
//
//  Programmer:   Hank Childs
//  Creation:     October 4, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Mar 19 17:45:21 PDT 2009
//    Set flag to force curve construction.
//
//    Kathleen Bonnell, Mon Mar 23 09:55:10 PDT 2009
//    Removed flag to force curve construction, no longer necessary.
//
// ****************************************************************************

avtCurveQuery::avtCurveQuery()
{
    ccf = new avtCurveConstructorFilter;
}


// ****************************************************************************
//  Method: avtCurveQuery::~avtCurveQuery
//
//  Programmer:   Hank Childs
//  Creation:     October 4, 2003
//
// ****************************************************************************

avtCurveQuery::~avtCurveQuery()
{
    if (ccf != NULL)
    {
        delete ccf;
        ccf = NULL;
    }
}


// ****************************************************************************
//  Method: avtCurveQuery::ApplyFilters
//
//  Purpose:
//      This will apply the curve constructor filter.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
// ****************************************************************************

avtDataObject_p
avtCurveQuery::ApplyFilters(avtDataObject_p inData)
{
    avtContract_p contract =
        inData->GetOriginatingSource()->GetGeneralContract();

    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    ccf->SetInput(dob);
    avtDataObject_p objOut = ccf->GetOutput();
    objOut->Update(contract);
    return objOut;
}


// ****************************************************************************
//  Method: avtCurveQuery::Execute
//
//  Purpose:
//      This is the method for the base type to iterate through the data
//      tree.  However, after calling the curve constructor filter, we know
//      there will be exactly one node on the data tree and it will be on
//      processor 0.  So this function will be called exactly one time for
//      the whole curve.  So we can just feed that curve to our derived types.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Jul 27 17:43:38 PDT 2006
//    Curves now represented as 1D RectilinearGrids.
//
// ****************************************************************************

void
avtCurveQuery::Execute(vtkDataSet *ds, const int)
{
    //
    // Construct the curve.  This is heavily assuming that the input is a
    // well-formed curve from the curve constructor filter.
    //
    vtkDataArray *xc = ((vtkRectilinearGrid*)ds)->GetXCoordinates();
    vtkDataArray *sc = ds->GetPointData()->GetScalars();
    int np = xc->GetNumberOfTuples();
    float *x = new float[np];
    float *y = new float[np];
    for (int i = 0 ; i < np ; i++)
    {
         x[i] = xc->GetTuple1(i);
         y[i] = sc->GetTuple1(i);
    }

    //
    // Let the derived type do the actual query.
    //
    double val = CurveQuery(np, x, y);
    std::string msg = CreateMessage(val);

    //
    // Tell the query what the results were.
    //
    SetResultValue(val);
    SetResultMessage(msg);

    delete [] x;
    delete [] y;
}


