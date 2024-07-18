// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//   avtVolumeResampleFilter.C
// ****************************************************************************

#include <avtVolumeResampleFilter.h>
#include <avtCallback.h>
#include <avtParallel.h>
#include <vtkDataSet.h>

enum ResampleReason
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
//    Kathleen Biagas, Monday July 8, 2024
//    Inherit from avtResampleFilter.
//
// ****************************************************************************

avtVolumeResampleFilter::avtVolumeResampleFilter(
    const InternalResampleAttributes *a,
   const VolumeAttributes & va) : avtResampleFilter(a)
{
    volAtts = va;
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


    if (volAtts.GetRendererType() == VolumeAttributes::Serial ||
        volAtts.GetRendererType() == VolumeAttributes::Parallel)
    {
        const avtDataAttributes &datts = input->GetInfo().GetAttributes();
        std::string activeVariable = datts.GetVariableName();
        // Only check for required resampling if there is no user
        // resampling.
        int userResample =
          (volAtts.GetResampleType() == VolumeAttributes::SingleDomain ||
           volAtts.GetResampleType() == VolumeAttributes::ParallelRedistribute ||
           volAtts.GetResampleType() == VolumeAttributes::ParallelPerRank);

        int mustResample =
          userResample ? NoResampling : DataMustBeResampled(input);
        if (volAtts.GetRendererType() == VolumeAttributes::Serial &&
            (volAtts.GetResampleType() == VolumeAttributes::ParallelRedistribute ||
             volAtts.GetResampleType() == VolumeAttributes::ParallelPerRank))
        {
            avtCallback::IssueWarning("Performing 'Serial Rendering' but a "
                "parallel resampling was selected.  "
                "Single domain sampling will be performed.");
        }

        else if(volAtts.GetRendererType() == VolumeAttributes::Parallel &&
                PAR_Size() > 1 &&
                volAtts.GetResampleType() == VolumeAttributes::SingleDomain)
        {
            avtCallback::IssueWarning("Performing 'Parallel Rendering' but "
                "single domain resampling was selected.  "
                "Parallel resampling should be used.");
        }

        else if(PAR_Size() == 1 &&
                (volAtts.GetResampleType() == VolumeAttributes::ParallelRedistribute ||
                 volAtts.GetResampleType() == VolumeAttributes::ParallelPerRank))
        {
            avtCallback::IssueWarning("Running in serial but parallel "
                "resampling was selected. 'Single domain' sampling will"
                " be performed.");
        }

        // If the data must be resampled and user did not request
        // resampling report a warning so the user knows the data has
        // been resampled.
        if( volAtts.GetResampleType() != VolumeAttributes::NoResampling &&
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

        if( volAtts.GetResampleType() == VolumeAttributes::NoResampling &&
            mustResample )
        {
            avtCallback::IssueWarning("'No resampling' was selected but the plot 'thinks' resampling needs to occur. This need may or may not be correct. As such, the rendered results may or may not be correct.");
        }

        // User can force resampling - for the serial renderer
        // everything is sampled on to a single grid.
        if( volAtts.GetResampleType() != VolumeAttributes::NoResampling &&
            (mustResample || userResample) )
        {
            // If the user selected a specific centering use it.
            // Otherwise use the centering from the color data.
            bool dataCellCentering = false;

            if( volAtts.GetResampleCentering() != VolumeAttributes::NativeCentering)
            {
                dataCellCentering =
                    volAtts.GetResampleCentering() == VolumeAttributes::ZonalCentering;
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

            // User requested resampling. If the type is 1 then resample
            // on to a single domain. Otherwise resample in parallel
            // (ignored if running in serial).
            if (volAtts.GetRendererType() == VolumeAttributes::Parallel && PAR_Size() > 1)
            {
                if( userResample )
                {
                    if( volAtts.GetResampleType() == VolumeAttributes::ParallelRedistribute )
                        atts.SetDistributedResample(true);
                    else if( volAtts.GetResampleType() == VolumeAttributes::ParallelPerRank )
                        atts.SetPerRankResample(true);
                }
                // Must resample but the user selected 'only if required' so
                // do a distributed resample.
                else //if( mustResample )
                {
                    atts.SetDistributedResample(true);
                }
            }

            atts.SetTargetVal(volAtts.GetResampleTarget());
            atts.SetPrefersPowersOfTwo(true);
            atts.SetUseTargetVal(true);
            MakeOutputCellCentered( dataCellCentering );
            ResampleInput();
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
//    Kathleen Biagas, Wed Jan  3 13:26:29 PST 2024
//    Retreive value returned by UnifyBitwiseOrValue, and moved the call to
//    the end of the method.
//
//    Kathleen Biagas, Moday July 8, 2024
//    Test individual datasets in the input for RectilinearGrid, don't rely
//    solely on MeshType, as some operators may change their mesh type
//    without actually updating the MeshType in avtDataAttributes.
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
    else
    {
        avtDataTree_p tree = GetTypedInput()->GetDataTree();
        // Get the input data.
        int nsets = 0;
        vtkDataSet **leaves = tree->GetAllLeaves(nsets);
        if( nsets > 1 )
            resampling |= MultipleDatasets;

        // Operators may not set their output mesh types correctly
        for (int i = 0; i < nsets; ++i)
        {
            if(leaves[i] != nullptr &&
               leaves[i]->GetDataObjectType() != VTK_RECTILINEAR_GRID)
            {
                resampling |= NonRectilinearGrid;
                break;
            }
        }
        delete [] leaves;
    }

    // If the opacity variable is different from the active variable
    // check the centering.
    std::string activeVariable = datts.GetVariableName();
    std::string opacityVariable = volAtts.GetOpacityVariable();

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
            if (volAtts.GetResampleType() != VolumeAttributes::NoResampling)
            {
                std::string msg("Could not determine the variable centering for ");

                msg += activeVariable + " and/or " +
                  opacityVariable + " so resampling.";

                avtCallback::IssueWarning(msg.c_str());
            }

            resampling |= DifferentCentering;
        }
    }

    // When there are more domains than ranks, it is possible for some
    // ranks to have one data set while others have more than one. As
    // such, when this case occurs the resampling must be acorss all
    // ranks so unify the resampling.
    resampling = UnifyBitwiseOrValue(resampling);

    return resampling;
}


