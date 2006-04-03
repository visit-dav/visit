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

#include <avtAbsValFilter.h>
#include <avtBinaryMultiplyFilter.h>
#include <avtCallback.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtVMetricArea.h>
#include <avtVMetricVolume.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <InvalidDimensionsException.h>
#include <NonQueryableInputException.h>

#include <snprintf.h>
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
    absval = new avtAbsValFilter;
    multiply = new avtBinaryMultiplyFilter;
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
// ****************************************************************************

void
avtCentroidQuery::PreExecute(void)
{
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
    SNPRINTF(msg, 4096, "Centroid = (%f, %f, %f)",C_tmp[0],C_tmp[1],C_tmp[2]);
    SetResultMessage(msg);
    doubleVector C_vec(3);
    for (int i = 0 ; i < 3 ; i++)
        C_vec[i] = C_tmp[i];
    SetResultValues(C_vec);
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
    int nCells = ds->GetNumberOfCells();
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *var = ds->GetCellData()->GetArray("avt_mass");
    if (var == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }
    for (int i = 0 ; i < nCells ; i++)
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
    avtDataSpecification_p dspec = GetInput()->GetTerminatingSource()
                                     ->GetFullDataSpecification();
    string varname = dspec->GetVariable();
    bool useVar = false;
    if (GetInput()->GetInfo().GetAttributes().ValidVariable(varname))
    {
        int dim = GetInput()->GetInfo().GetAttributes().
                                         GetVariableDimension(varname.c_str());
        if (dim == 1)
            useVar = true;
    }

    avtVerdictFilter *vf = NULL;
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
    avtPipelineSpecification_p pspec =
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();
    dob->Update(pspec);
    return dob;
}


