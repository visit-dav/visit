// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtCentroidQuery.C                            //
// ************************************************************************* //

#include <avtCentroidQuery.h>

#include <vtkCell.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkVisItUtility.h>

#include <avtAbsValExpression.h>
#include <avtBinaryMultiplyExpression.h>
#include <avtCallback.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtVMetricArea.h>
#include <avtVMetricVolume.h>

#include <DebugStream.h>
#include <ImproperUseException.h>

#include <float.h>
#include <math.h>

using     std::string;
using     std::vector;

#if !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

// ****************************************************************************
//  Method: avtCentroidQuery constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Aug 12 15:25:07 PDT 2005
//    Add absval.
//
// ****************************************************************************

avtCentroidQuery::avtCentroidQuery()
{
    absval = new avtAbsValExpression;
    multiply = new avtBinaryMultiplyExpression;
    area = new avtVMetricArea;
    volume = new avtVMetricVolume;
}


// ****************************************************************************
//  Method: avtCentroidQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Aug 12 15:25:07 PDT 2005
//    Destruct absval.
//
// ****************************************************************************

avtCentroidQuery::~avtCentroidQuery()
{
    delete absval;
    delete multiply;
    delete volume;
    delete area;
}


// ****************************************************************************
//  Method: avtCentroidQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtCentroidQuery::PreExecute(void)
{
    avtDatasetQuery::PreExecute();

    for (int i = 0 ; i < 3 ; i++)
        C[i] = 0.;
    total_mass = 0.;
}

// ****************************************************************************
//  Method: avtCentroidQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
//    Kathleen Biagas, Mon Feb 24 15:53:29 PST 2014
//    Add Xml results.
//
// ****************************************************************************

void
avtCentroidQuery::PostExecute(void)
{
    double C_tmp[3];
    SumDoubleAcrossAllProcessors(total_mass);
    SumDoubleArrayAcrossAllProcessors(C, C_tmp, 3);
    if (total_mass != 0.)
    {
        C_tmp[0] /= total_mass;
        C_tmp[1] /= total_mass;
        C_tmp[2] /= total_mass;
    }

    //
    //  Parent class uses this message to set the Results message
    //  in the Query Attributes that is sent back to the viewer.
    //  That is all that is required of this query.
    //
    char msg[4096];
    string floatFormat = queryAtts.GetFloatFormat();
    string format = "Centroid = (" + floatFormat + ", "
                                   + floatFormat + ", "
                                   + floatFormat + ")";
    snprintf(msg, 4096, format.c_str(),C_tmp[0],C_tmp[1],C_tmp[2]);
    SetResultMessage(msg);
    doubleVector C_vec(3);
    for (int i = 0 ; i < 3 ; i++)
        C_vec[i] = C_tmp[i];
    SetResultValues(C_vec);
    MapNode result_node;
    result_node["centroid"] = C_vec;
    SetXmlResult(result_node.ToXML());
}

// ****************************************************************************
//  Method: avtCentroidQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

void
avtCentroidQuery::Execute(vtkDataSet *ds, const int dom)
{
    vtkIdType nCells = ds->GetNumberOfCells();
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *var = ds->GetCellData()->GetArray("avt_mass");
    if (var == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }
    for (vtkIdType i = 0 ; i < nCells ; i++)
    {
        if (ghosts != NULL && ghosts->GetTuple1(i) != 0.)
            continue;
        vtkCell *cell = ds->GetCell(i);
        double center[3];
        vtkVisItUtility::GetCellCenter(cell, center);
        double mass = var->GetTuple1(i);
        C[0] += mass*center[0];
        C[1] += mass*center[1];
        C[2] += mass*center[2];
        total_mass += (mass < 0. ? -mass : mass);
    }
}


// ****************************************************************************
//  Method: avtCentroidQuery::ApplyFilters
//
//  Purpose:
//      Applies the filters to the input.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Aug 12 15:25:07 PDT 2005
//    Use absolute value of the volume or area.
//
//    Hank Childs, Wed Sep 14 16:01:12 PDT 2005
//    Use topological dimension for weights, not spatial dimension.
//
//    Kathleen Bonnell, Wed Apr  2 12:01:59 PDT 2008 
//    Retrieve the varname from the queryAtts instead of DataRequest, as
//    DataRequest may have the wrong value based on other pipelines sharing
//    the same source. 
//
// ****************************************************************************

avtDataObject_p
avtCentroidQuery::ApplyFilters(avtDataObject_p inData)
{
    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    //
    // Set up our base class so it is ready to sum.
    //
    string varname = queryAtts.GetVariables()[0];
    bool useVar = false;
    if (GetInput()->GetInfo().GetAttributes().ValidVariable(varname))
    {
        int dim = GetInput()->GetInfo().GetAttributes().
                                         GetVariableDimension(varname.c_str());
        if (dim == 1)
            useVar = true;
    }

    avtVerdictExpression *vf = NULL;
    if (dob->GetInfo().GetAttributes().GetTopologicalDimension() == 3)
        vf = volume;
    else
        vf = area;
    vf->SetOutputVariableName("avt_verdict");
    vf->SetInput(dob);
 
    absval->SetInput(vf->GetOutput());
    absval->AddInputVariableName("avt_verdict");
    if (useVar)
        absval->SetOutputVariableName("avt_weights");
    else
        absval->SetOutputVariableName("avt_mass");
    dob = absval->GetOutput();

    if (useVar)
    {
        multiply->SetInput(dob);
        multiply->ClearInputVariableNames();
        multiply->AddInputVariableName(varname.c_str());
        multiply->AddInputVariableName("avt_weights");
        multiply->SetOutputVariableName("avt_mass");
        dob = multiply->GetOutput();
    }

    //
    // Cause our artificial pipeline to execute.
    //
    avtContract_p contract =
        inData->GetOriginatingSource()->GetGeneralContract();
    dob->Update(contract);
    return dob;
}


