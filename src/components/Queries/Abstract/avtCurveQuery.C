// ************************************************************************* //
//                               avtCurveQuery.C                             //
// ************************************************************************* //

#include <avtCurveQuery.h>

#include <vtkDataSet.h>

#include <avtCurveConstructorFilter.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTerminatingSource.h>

#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtCurveQuery::avtCurveQuery
//
//  Programmer:   Hank Childs
//  Creation:     October 4, 2003
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
    avtPipelineSpecification_p pspec =
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();

    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    ccf->SetInput(dob);
    avtDataObject_p objOut = ccf->GetOutput();
    objOut->Update(pspec);
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
// ****************************************************************************

void
avtCurveQuery::Execute(vtkDataSet *ds, const int)
{
    //
    // Construct the curve.  This is heavily assuming that the input is a
    // well-formed curve from the curve constructor filter.
    //
    int np = ds->GetNumberOfPoints();
    float *x = new float[np];
    float *y = new float[np];
    float pt[3];
    for (int i = 0 ; i < np ; i++)
    {
         ds->GetPoint(i, pt);
         x[i] = pt[0];
         y[i] = pt[1];
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


