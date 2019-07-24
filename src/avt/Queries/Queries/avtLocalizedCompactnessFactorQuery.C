// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtLocalizedCompactnessFactorQuery.C                 //
// ************************************************************************* //

#include <avtLocalizedCompactnessFactorQuery.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtConstantCreatorExpression.h>
#include <avtDatasetExaminer.h>
#include <avtLocalizedCompactnessExpression.h>
#include <avtParallel.h>
#include <avtResampleFilter.h>
#include <avtSourceFromAVTDataset.h>

#include <DebugStream.h>
#include <ImproperUseException.h>

#include <snprintf.h>
#include <float.h>
#include <math.h>

using     std::string;
using     std::vector;

#if !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery constructor
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
// ****************************************************************************

avtLocalizedCompactnessFactorQuery::avtLocalizedCompactnessFactorQuery()
{
}


// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
// ****************************************************************************

avtLocalizedCompactnessFactorQuery::~avtLocalizedCompactnessFactorQuery()
{
}


// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtLocalizedCompactnessFactorQuery::PreExecute(void)
{
    avtDatasetQuery::PreExecute();

    sum = 0.;
    numEntries = 0;
}


// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
// ****************************************************************************

void
avtLocalizedCompactnessFactorQuery::PostExecute(void)
{
    double fullSum;
    SumDoubleArrayAcrossAllProcessors(&sum, &fullSum, 1);
    sum = fullSum;

    int totalNumEntries;
    SumIntArrayAcrossAllProcessors(&numEntries, &totalNumEntries, 1);
    numEntries = totalNumEntries;

    double factor = (totalNumEntries > 0 ? fullSum / totalNumEntries : 0);

    char buff[1024];
    string format = "The localized compactness factor is "
                    + queryAtts.GetFloatFormat() + "\n";
    SNPRINTF(buff, 1024,format.c_str() , factor);
    SetResultMessage(buff);
    SetResultValue(factor);
}


// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery::Execute
//
//  Purpose:
//      Processes a single domain.  This will calculate the average compactness
//      over that domain.
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
// ****************************************************************************

void
avtLocalizedCompactnessFactorQuery::Execute(vtkDataSet *ds, const int dom)
{
    vtkDataArray *varF = ds->GetPointData()->GetArray("lce"); // F = factor
    vtkDataArray *varQ = ds->GetPointData()->GetArray("is_material");
                  // Q = qualifier.  It helps us answer the question: 
                  // Should we even count this?

    if (varF == NULL || varQ == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }
    int nPts = ds->GetNumberOfPoints();
    for (int i = 0 ; i < nPts ; i++)
    {
        if (varQ->GetTuple1(i) != 0.)
        {
            sum += varF->GetTuple1(i);
            numEntries++;
        }
    }
}


// ****************************************************************************
//  Method: avtLocalizedCompactnessFactorQuery::ApplyFilters
//
//  Purpose:
//      Applies the filters to the input.
//
//  Programmer: Hank Childs
//  Creation:   April 29, 2006
//
//  Modifications:
//
//    Hank Childs, Mon May 22 15:44:46 PDT 2006
//    Make the resample region be only the region where there is actually data.
//
//    Hank Childs, Wed Dec 31 13:46:14 PST 2008
//    Change the ResampleAtts to InternalResampleAtts.
//
// ****************************************************************************

avtDataObject_p
avtLocalizedCompactnessFactorQuery::ApplyFilters(avtDataObject_p inData)
{
    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    double extents[6] = { 0, 0, 0, 0, 0, 0 };
    avtDatasetExaminer::GetSpatialExtents(ds, extents);

    avtConstantCreatorExpression ccf;
    ccf.SetValue(1.0);
    ccf.SetInput(dob);
    ccf.SetOutputVariableName("is_material");
    dob = ccf.GetOutput();
   
    InternalResampleAttributes res_atts;
    res_atts.SetDefaultVal(0.);
    bool is2D = (dob->GetInfo().GetAttributes().GetSpatialDimension() < 3);
    int res = (is2D ? 250000 : 2000000);
    res_atts.SetTargetVal(res);
    res_atts.SetUseTargetVal(true);
    res_atts.SetUseBounds(true);
    res_atts.SetMinX(extents[0]);
    res_atts.SetMaxX(extents[1]);
    res_atts.SetMinY(extents[2]);
    res_atts.SetMaxY(extents[3]);
    res_atts.SetMinZ(extents[4]);
    res_atts.SetMaxZ(extents[5]);
    avtResampleFilter resf(&res_atts);
    resf.SetInput(dob);
    dob = resf.GetOutput();
    
    avtLocalizedCompactnessExpression lce;
    lce.SetInput(dob);
    lce.SetOutputVariableName("lce");
    lce.AddInputVariableName("is_material");
    dob = lce.GetOutput();

    //
    // Cause our artificial pipeline to execute.
    //
    avtContract_p contract =
        inData->GetOriginatingSource()->GetGeneralContract();
    contract->GetDataRequest()->AddSecondaryVariable("is_material");
    dob->Update(contract);
    return dob;
}


