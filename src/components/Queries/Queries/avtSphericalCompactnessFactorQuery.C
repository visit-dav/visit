// ************************************************************************* //
//                      avtSphericalCompactnessFactorQuery.C                 //
// ************************************************************************* //

#include <avtSphericalCompactnessFactorQuery.h>

#include <vtkCell.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtParallel.h>
#include <avtRevolvedVolume.h>
#include <avtSourceFromAVTDataset.h>
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
//  Method: avtSphericalCompactnessFactorQuery constructor
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2005
//
// ****************************************************************************

avtSphericalCompactnessFactorQuery::avtSphericalCompactnessFactorQuery()
{
    rev_volume = new avtRevolvedVolume;
    volume = new avtVMetricVolume;
}


// ****************************************************************************
//  Method: avtSphericalCompactnessFactorQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2005
//
// ****************************************************************************

avtSphericalCompactnessFactorQuery::~avtSphericalCompactnessFactorQuery()
{
    delete volume;
    delete rev_volume;
}


// ****************************************************************************
//  Method: avtSphericalCompactnessFactorQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2005
//
// ****************************************************************************

void
avtSphericalCompactnessFactorQuery::PreExecute(void)
{
    for (int i = 0 ; i < 3 ; i++)
        centroid[i] = 0.;
    total_volume = 0.;
}


// ****************************************************************************
//  Method: avtSphericalCompactnessFactorQuery::MidExecute
//
//  Purpose:
//      This is called after Execute1 and before Execute2.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2005
//
// ****************************************************************************

void
avtSphericalCompactnessFactorQuery::MidExecute(void)
{
    SumDoubleAcrossAllProcessors(total_volume);
    double C_tmp[3] = { 0, 0, 0 };
    SumDoubleArrayAcrossAllProcessors(centroid, C_tmp, 3);
    if (total_volume != 0.)
    {
        C_tmp[0] /= total_volume;
        C_tmp[1] /= total_volume;
        C_tmp[2] /= total_volume;
    }
    centroid[0] = C_tmp[0];
    centroid[1] = C_tmp[1];
    centroid[2] = C_tmp[2];

    volume_inside = 0.;
    radius = pow(total_volume*0.75/M_PI, 0.3333333);
    if (is2D)
    {
        sphere_center[0] = centroid[0];
        sphere_center[1] = 0.;
        sphere_center[2] = 0.;
    }
    else
    {
        sphere_center[0] = centroid[0];
        sphere_center[1] = centroid[1];
        sphere_center[2] = centroid[2];
    }
}


// ****************************************************************************
//  Method: avtSphericalCompactnessFactorQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2005
//
// ****************************************************************************

void
avtSphericalCompactnessFactorQuery::PostExecute(void)
{
    SumDoubleAcrossAllProcessors(volume_inside);
    if (total_volume == 0.)
    {
        total_volume = 1;
    }

    //
    //  Parent class uses this message to set the Results message
    //  in the Query Attributes that is sent back to the viewer.
    //  That is all that is required of this query.
    //
    char msg[4096];
    SNPRINTF(msg, 4096, "Spherical Compactness Factor = %f.  Using centroid "
                        "for sphere origin.  Centroid used was (%f, %f, %f)."
                        "  Radius was %f",
                         volume_inside / total_volume, 
                         sphere_center[0], sphere_center[1], sphere_center[2],
                         radius);
    SetResultMessage(msg);
    SetResultValue(volume_inside / total_volume);
}


// ****************************************************************************
//  Method: avtSphericalCompactnessFactorQuery::Execute1
//
//  Purpose:
//      Processes a single domain.  This will calculate the total volume and
//      the centroid on this pass.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2005
//
// ****************************************************************************

void
avtSphericalCompactnessFactorQuery::Execute1(vtkDataSet *ds, const int dom)
{
    int nCells = ds->GetNumberOfCells();
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *var = ds->GetCellData()->GetArray("avt_volume");
    if (var == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }
    for (int i = 0 ; i < nCells ; i++)
    {
        if (ghosts != NULL && ghosts->GetTuple1(i) != 0.)
            continue;
        vtkCell *cell = ds->GetCell(i);
        float center[3];
        vtkVisItUtility::GetCellCenter(cell, center);
        float volume = var->GetTuple1(i);
        volume = (volume < 0 ? -volume : volume);
        centroid[0] += volume*center[0];
        centroid[1] += volume*center[1];
        centroid[2] += volume*center[2];
        total_volume += volume;
    }
}


// ****************************************************************************
//  Method: avtSphericalCompactnessFactorQuery::Execute2
//
//  Purpose:
//      Processes a single domain.  This will calculate the portion inside
//      the sphere on this pass.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2005
//
// ****************************************************************************

void
avtSphericalCompactnessFactorQuery::Execute2(vtkDataSet *ds, const int dom)
{
    int nCells = ds->GetNumberOfCells();
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *var = ds->GetCellData()->GetArray("avt_volume");
    if (var == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }
    float rad_squared = radius*radius;
    for (int i = 0 ; i < nCells ; i++)
    {
        if (ghosts != NULL && ghosts->GetTuple1(i) != 0.)
            continue;
        vtkCell *cell = ds->GetCell(i);
        float center[3];
        vtkVisItUtility::GetCellCenter(cell, center);
        float dist = (center[0]-sphere_center[0])*(center[0]-sphere_center[0])
                   + (center[1]-sphere_center[1])*(center[1]-sphere_center[1])
                   + (center[2]-sphere_center[2])*(center[2]-sphere_center[2]);
        if (dist > rad_squared)
            continue;  // Not inside.
        float volume = var->GetTuple1(i);
        volume = (volume < 0 ? -volume : volume);
        volume_inside += volume;
    }
}


// ****************************************************************************
//  Method: avtSphericalCompactnessFactorQuery::ApplyFilters
//
//  Purpose:
//      Applies the filters to the input.
//
//  Programmer: Hank Childs
//  Creation:   July 14, 2005
//
// ****************************************************************************

avtDataObject_p
avtSphericalCompactnessFactorQuery::ApplyFilters(avtDataObject_p inData)
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

    avtExpressionFilter *vf = NULL;
    is2D = (dob->GetInfo().GetAttributes().GetSpatialDimension() != 3);
    if (is2D)
        vf = rev_volume;
    else
        vf = volume;

    vf->SetOutputVariableName("avt_volume");
    vf->SetInput(dob);
    dob = vf->GetOutput();
 
    //
    // Cause our artificial pipeline to execute.
    //
    avtPipelineSpecification_p pspec =
        inData->GetTerminatingSource()->GetGeneralPipelineSpecification();
    dob->Update(pspec);
    return dob;
}


