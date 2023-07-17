// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//   avtVolumeResampleFilter.C
// ****************************************************************************

#include <avtVolumeResampleFilter.h>
#include <avtCallback.h>
#include <avtParallel.h>
#include <avtResampleFilter.h>
#include <InvalidDimensionsException.h>
#include <vtkDataSet.h>

enum ResampleReason // Must match avtVolumePLot.h
{
    NoResampling       = 0x0,
    MultipleDatasets   = 0x1,
    NonRectilinearGrid = 0x2,
    DifferentCentering = 0x4,
    ImplicitTransform  = 0x8
};


// ****************************************************************************
//  Method: avtVolumeResampleFilter constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   July 11, 2023
//
//  Modifications:
//
// ****************************************************************************

avtVolumeResampleFilter::avtVolumeResampleFilter(const VolumeAttributes &a)
{
    atts = a;
    resampleFilter = nullptr;
}


// ****************************************************************************
//  Method: avtVolumeResampleFilter destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   July 11, 2023
//
//  Modifications:
//
// ****************************************************************************

avtVolumeResampleFilter::~avtVolumeResampleFilter()
{
    if(resampleFilter)
        delete resampleFilter;
}

// ****************************************************************************
//  Method: avtVolumeResampleFilter::Execute
//
//  Purpose:
//    Performs resampling if necessary.
//
//  Notes:  moved from avtVolumePlot::ApplyRenderingTransformation
//
//  Programmer: Kathleen Biagas
//  Creation:   July 11, 2023
//
//  Modifications:
//
// ****************************************************************************

void
avtVolumeResampleFilter::Execute(void)
{
    // Copy our input to the output
    avtDataObject_p input = GetInput();

    // in case resampling isn't needed
    GetOutput()->Copy(*input);

    if (atts.GetRendererType() == VolumeAttributes::Serial ||
        atts.GetRendererType() == VolumeAttributes::Parallel)
    {
        const avtDataAttributes &datts = input->GetInfo().GetAttributes();
        std::string activeVariable = datts.GetVariableName();
        // Only check for required resampling if there is no user
        // resampling.
        int userResample =
          (atts.GetResampleType() == VolumeAttributes::SingleDomain ||
           atts.GetResampleType() == VolumeAttributes::ParallelRedistribute ||
           atts.GetResampleType() == VolumeAttributes::ParallelPerRank);

        int mustResample =
          userResample ? NoResampling : DataMustBeResampled(input);
        if (atts.GetRendererType() == VolumeAttributes::Serial &&
            (atts.GetResampleType() == VolumeAttributes::ParallelRedistribute ||
             atts.GetResampleType() == VolumeAttributes::ParallelPerRank))
        {
            avtCallback::IssueWarning("Performing 'Serial Rendering' but a "
                "parallel resampling was selected.  "
                "Single domain sampling will be performed.");
        }

        else if(atts.GetRendererType() == VolumeAttributes::Parallel &&
                PAR_Size() > 1 &&
                atts.GetResampleType() == VolumeAttributes::SingleDomain)
        {
            avtCallback::IssueWarning("Performing 'Parallel Rendering' but "
                "single domain resampling was selected.  "
                "Parallel resampling should be used.");
        }

        else if(PAR_Size() == 1 &&
                (atts.GetResampleType() == VolumeAttributes::ParallelRedistribute ||
                 atts.GetResampleType() == VolumeAttributes::ParallelPerRank))
        {
            avtCallback::IssueWarning("Running in serial but parallel "
                "resampling was selected. 'Single domain' sampling will"
                " be performed.");
        }

        // If the data must be resampled and user did not request
        // resampling report a warning so the user knows the data has
        // been resampled.
        if( atts.GetResampleType() != VolumeAttributes::NoResampling &&
            (mustResample && !userResample) )
        {
            std::string msg("The data was resampled because ");

            if( mustResample & MultipleDatasets )
            {
                msg += "one or more ranks has more than one domain";

                if( mustResample & (NonRectilinearGrid | DifferentCentering | ImplicitTransform ))
                    msg += ", and ";
            }

            if( mustResample & NonRectilinearGrid )
            {
                msg += "the data is not on a rectilinear grid";

                if( mustResample & (DifferentCentering | ImplicitTransform))
                    msg += ", and ";
            }

            if( mustResample & DifferentCentering)
            {
                msg += "the data and opacity have different centering";
                if( mustResample & ImplicitTransform)
                    msg += ", and ";
            }

            if( mustResample & ImplicitTransform )
            {
                msg += "the data is on a rectilinear grid with an implicit transform applied";
            }

            msg += ". The data and if needed the opacity have been resampled "
              "on to a rectilinear grid";

            if( PAR_Size() > 1 )
                msg += " and distributed across all ranks";
            msg += ".";
            msg += "\nIf results are not satisfactory, modify the resampling options.";

            avtCallback::IssueWarning(msg.c_str());
        }

        if( atts.GetResampleType() == VolumeAttributes::NoResampling &&
            mustResample )
        {
            avtCallback::IssueWarning("'No resampling' was selected but the plot 'thinks' resampling needs to occur. This need may or may not be correct. As such, the rendered results may or may not be correct.");
        }

        // User can force resampling - for the serial renderer
        // everything is sampled on to a single grid.
        if( atts.GetResampleType() != VolumeAttributes::NoResampling &&
            (mustResample || userResample) )
        {
            // If the user selected a specific centering use it.
            // Otherwise use the centering from the color data.
            bool dataCellCentering = false;

            if( atts.GetResampleCentering() != VolumeAttributes::NativeCentering)
            {
                dataCellCentering =
                    atts.GetResampleCentering() == VolumeAttributes::ZonalCentering;
            }
            else
            {
                if (datts.ValidVariable(activeVariable.c_str()))
                {
                    dataCellCentering =
                      (datts.GetCentering(activeVariable.c_str()) == AVT_ZONECENT);
                }
            }

            //
            // Resample the data
            //
            InternalResampleAttributes resampleAtts;

            // User requested resampling. If the type is 1 then resample
            // on to a single domain. Otherwise resample in parallel
            // (ignored if running in serial).
            if (atts.GetRendererType() == VolumeAttributes::Parallel && PAR_Size() > 1)
            {
                if( userResample )
                {
                    if( atts.GetResampleType() == VolumeAttributes::ParallelRedistribute )
                        resampleAtts.SetDistributedResample(true);
                    else if( atts.GetResampleType() == VolumeAttributes::ParallelPerRank )
                        resampleAtts.SetPerRankResample(true);
                }
                // Must resample but the user selected 'only if required' so
                // do a distributed resample.
                else //if( mustResample )
                {
                    resampleAtts.SetDistributedResample(true);
                }
            }

            resampleAtts.SetTargetVal(atts.GetResampleTarget());
            resampleAtts.SetPrefersPowersOfTwo(true);
            resampleAtts.SetUseTargetVal(true);

            resampleFilter = new avtResampleFilter(&resampleAtts);
            resampleFilter->SetInput(input);
            resampleFilter->MakeOutputCellCentered( dataCellCentering );
            resampleFilter->GetOutput()->SetTransientStatus(false);
            resampleFilter->Update(GetGeneralContract());
            avtDataObject_p dob = resampleFilter->GetOutput();
            GetOutput()->Copy(*dob);
        }
    }
}

// ****************************************************************************
//  Method: avtVolumePlot::DataMustBeResampled
//
//  Purpose:
//      Some types of data MUST be resampled or they cannot be rendered.
//
//  Notes:  Moved from avtVolumePlot by ksb, July 17, 2023.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    bool
//
//  Programmer: Cameron Christensen
//  Creation:   Thursday, September 05, 2013
//
//  Modifications:
//    Burlen Loring, Mon Oct  5 06:42:17 PDT 2015
//    Catch the exception that can occur when using an operator that produces
//    a variable that doesn't exist in the database.
//
// ****************************************************************************

int
avtVolumeResampleFilter::DataMustBeResampled(avtDataObject_p input)
{
    int resampling = NoResampling;

    const avtDataAttributes &datts = input->GetInfo().GetAttributes();

    if(datts.GetRectilinearGridHasTransform())
        resampling |= ImplicitTransform;

    //
    // Unless the input data is a single domain rectilinear mesh,
    // with both the color and opacity data having the same centering,
    // the data must be resampled.
    //
    avtMeshType mt = datts.GetMeshType();
    if (mt != AVT_RECTILINEAR_MESH)
    {
        resampling |= NonRectilinearGrid;
    }

    avtDataset_p ds = GetTypedInput();

    avtDataTree_p tree = ds->GetDataTree();
    // Get the input data.
    int nsets = 0;
    vtkDataSet **leaves = tree->GetAllLeaves(nsets);
    if( nsets > 1 )
        resampling |= MultipleDatasets;

    // When there are more domains than ranks, it is possible for some
    // ranks to have one data set while others have more than one. As
    // such, when this case occurs the resampling must be acorss all
    // ranks so unify the resampling.
    UnifyBitwiseOrValue(resampling);

    for (int i = 0; i < nsets; ++i)
    {
        if(leaves[i] != nullptr &&
           leaves[i]->GetDataObjectType() != VTK_RECTILINEAR_GRID)
        {
            resampling |= NonRectilinearGrid;
        }
    }

    // If the opacity variable is different from the active variable
    // check the centering.
    std::string activeVariable = datts.GetVariableName();
    std::string opacityVariable = atts.GetOpacityVariable();

    if (opacityVariable != "default" && opacityVariable != activeVariable)
    {
        if (datts.ValidVariable(activeVariable.c_str()) &&
            datts.ValidVariable(opacityVariable.c_str()))
        {
            if(datts.GetCentering(activeVariable.c_str()) !=
               datts.GetCentering(opacityVariable.c_str()))
            resampling |= DifferentCentering;
        }
        else
        {
            if (atts.GetResampleType() != VolumeAttributes::NoResampling)
            {
                std::string msg("Could not determine the variable centering for ");

                msg += activeVariable + " and/or " +
                  opacityVariable + " so resampling.";

                avtCallback::IssueWarning(msg.c_str());
            }

            resampling |= DifferentCentering;
        }
    }
    return resampling;
}


// ****************************************************************************
//  Method: avtVolumeResampleFilter::ModifyContract
//
//  Purpose:
//    No-op.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 11, 2023
//
//  Modifications:
//
// ****************************************************************************

avtContract_p
avtVolumeResampleFilter::ModifyContract(avtContract_p contract)
{
    return contract;
}


// ****************************************************************************
//  Method: avtVolumeResampleFilter::VerifyInput
//
//  Purpose:
//      Verifies that the input is 3D data, throws an exception if not.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 11, 2023
//
// ****************************************************************************

void
avtVolumeResampleFilter::VerifyInput(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() != 3)
    {
        EXCEPTION2(InvalidDimensionsException, "Volume", "3D");
    }
}

