// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtLowerResolutionVolumeFilter.C             //
// ************************************************************************* //

#include <avtLowerResolutionVolumeFilter.h>

#include <InvalidVariableException.h>
#include <InvalidLimitsException.h>

#include <StackTimer.h>

#include <DebugStream.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkSkew.h>

#define NO_DATA_VALUE -1e+37

// ****************************************************************************
//  Method: avtLowerResolutionVolumeFilter constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Dec 18 14:13:27 PST 2008
//
//  Modifications:
//
// ****************************************************************************

avtLowerResolutionVolumeFilter::avtLowerResolutionVolumeFilter() : avtPluginDataTreeIterator()
{
}

// ****************************************************************************
//  Method: avtLowerResolutionVolumeFilter destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Dec 18 14:13:27 PST 2008
//
//  Modifications:
//
// ****************************************************************************

avtLowerResolutionVolumeFilter::~avtLowerResolutionVolumeFilter()
{
    if(hist != nullptr)
    {
        delete [] hist;
    }
}

// ****************************************************************************
//  Method: avtLowerResolutionVolumeFilter::SetAtts
//
//  Purpose:
//      Sets the attributes of the software override filter.
//
//  Arguments:
//      a       The attributes for the filter.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Dec 18 14:13:27 PST 2008
//
// ****************************************************************************

void
avtLowerResolutionVolumeFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const VolumeAttributes*)a;
}

// ****************************************************************************
// Method: LogTransform
//
// Purpose:
//   Computes log on one data array, storing it into another data array.
//
// Arguments:
//   linear : The linear values.
//   skew   : The computed log values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 19 14:02:29 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
avtLowerResolutionVolumeFilter::LogTransform(vtkDataArray *linear,
                                             vtkDataArray *log)
{
    StackTimer t("VolumeLogTransform");
    double *r = linear->GetRange();
    float range[2];
    range[0] = r[0];
    range[1] = r[1];

    if (atts.GetUseColorVarMin())
    {
        range[0] = atts.GetColorVarMin();
    }
    if (atts.GetUseColorVarMax())
    {
        range[1] = atts.GetColorVarMax();
    }
    if (range[0] <= 0. || range[1] <= 0.)
    {
        EXCEPTION1(InvalidLimitsException, true);
    }
    if(linear->GetDataType() == VTK_FLOAT &&
       log->GetDataType() == VTK_FLOAT)
    {
        const float *src = (const float *)linear->GetVoidPointer(0);
        const float *end = src + linear->GetNumberOfTuples();
        float *dest = (float *)log->GetVoidPointer(0);
        while(src < end)
        {
            float f = *src++;
            if (f > 0)
                f = log10(f);
            else if (f > NO_DATA_VALUE)
                f = log10(range[0]);
            *dest++ = f;
        }
    }
    else
    {
        for (int i = 0 ; i < linear->GetNumberOfTuples() ; i++)
        {
            double f = linear->GetTuple1(i);
            if (f > 0.)
                f = log10(f);
            else if (f > NO_DATA_VALUE)
                f = log10(range[0]);
            log->SetTuple1(i, f);
        }
    }
}

// ****************************************************************************
// Method: SkewTransform
//
// Purpose:
//   Computes skew on one data array, storing it into another data array.
//
// Arguments:
//   linear : The linear values.
//   skew   : The computed skew values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 19 14:02:29 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
avtLowerResolutionVolumeFilter::SkewTransform( vtkDataArray *linear,
                                               vtkDataArray *skew)
{
    StackTimer t("VolumeSkewTransform");
    double *r = linear->GetRange();
    float range[2];
    range[0] = (float)r[0];
    range[1] = (float)r[1];

    if (atts.GetUseColorVarMin())
    {
        range[0] = atts.GetColorVarMin();
    }
    if (atts.GetUseColorVarMax())
    {
        range[1] = atts.GetColorVarMax();
    }
    float skewFactor = atts.GetSkewFactor();
    if(linear->GetDataType() == VTK_FLOAT &&
       skew->GetDataType() == VTK_FLOAT)
    {
        const float *src = (const float *)linear->GetVoidPointer(0);
        const float *end = src + linear->GetNumberOfTuples();
        float *dest = (float *)skew->GetVoidPointer(0);
        while(src < end)
            *dest++ = vtkSkewValue(*src++, range[0], range[1], skewFactor);
    }
    else
    {
        for (int i = 0 ; i < linear->GetNumberOfTuples() ; i++)
        {
            float f = linear->GetTuple1(i);
            skew->SetTuple1(i, (float)vtkSkewValue(f, range[0], range[1], skewFactor));
        }
    }
}

// ****************************************************************************
// Method: avtLowerResolutionVolumeFilter::CalculateHistograms
//
// Purpose:
//   Calculates the histogram data used in the the plot info.
//
// Arguments:
//   ds : The dataset that contains the variables of interest.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 19 14:08:43 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
avtLowerResolutionVolumeFilter::CalculateHistograms(vtkDataSet *ds)
{
    const char *mName = "avtLowerResolutionVolumeFilter::CalculateHistograms: ";

    vtkDataArray *data = ds->GetPointData()->GetScalars();

    if( data == nullptr )
    {
        data = ds->GetCellData()->GetScalars();

        if( data == nullptr )
        {
            EXCEPTION1(InvalidVariableException, "");
        }
    }

    debug5 << mName << "Computing histograms" << std::endl;

    int nTuples = data->GetNumberOfTuples();

    // Initialize the output array.
    if(hist != nullptr)
        delete [] hist;

    hist = new float[hist_size];

    memset(hist, 0, sizeof(float) * hist_size);

    // Get the range for the data var.
    double dataRange[2] = {0., 1.};
    double opacityRange[2] = {0., 1.};

    data->GetRange( dataRange );

    if( atts.GetUseColorVarMin() )
        dataRange[0] = atts.GetColorVarMin();
    if( atts.GetUseColorVarMax() )
        dataRange[1] = atts.GetColorVarMax();

    debug5 << mName << "Var range: "
           << dataRange[0] << ", " << dataRange[1] << std::endl;

    // Populate histograms
    double scale = double(hist_size - 1) / (dataRange[1] - dataRange[1]);

    if(dataRange[1] <= dataRange[1])
        scale = 0.0;

    double hist_max = 0.0;

    for(int index = 0; index < nTuples; ++index)
    {
        double s = data->GetTuple1(index);

        if(s < NO_DATA_VALUE)
            continue;
        if(s < dataRange[0])
            continue;
        if(s > dataRange[1])
            continue;

        int scalar_index = double((s - dataRange[0]) * scale);

        hist[scalar_index] += 1.0;

        if(hist_max < hist[scalar_index])
            hist_max = hist[scalar_index];
    }

    // Normalize the histogram data.
    if(hist_max > 0.0)
    {
        double h_scale = 1.0 / hist_max;

        for (int index = 0; index < hist_size; ++index)
            hist[index] *= h_scale;
    }
}

// ****************************************************************************
// Method: avtLowerResolutionVolumeFilter::ExecuteData
//
// Purpose:
//   ds
//
// Arguments:
//   in_dr :   The input data representation.
//
// Returns:    The output data representation.
//
// Note:       This filter assumes that there will only be 1 domain. Which
//             is fine because it is called after the resample filter.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 18 14:13:43 PST 2008
//
// Modifications:
//   Eric Brugger, Tue Aug 19 14:06:17 PDT 2014
//   Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

avtDataRepresentation *
avtLowerResolutionVolumeFilter::ExecuteData(avtDataRepresentation *in_dr)
{
    StackTimer t("avtLowerResolutionVolumeFilter::ExecuteData");

    // Get the VTK data set.
    vtkDataSet *ds = in_dr->GetDataVTK();
    vtkDataSet *rv = ds;

    // If preforming Log or Skew scaling create a copy of the dataset
    // with scalars that are transformed by the appropriate scaling rule.
    if (atts.GetScaling() == VolumeAttributes::Log ||
	atts.GetScaling() == VolumeAttributes::Skew)
    {
        // Get the array that is being "scaling".
        bool cellData = false;

        vtkDataArray *src = ds->GetPointData()->GetScalars();

        if( src == nullptr )
        {
            src = ds->GetCellData()->GetScalars();

            if( src == nullptr )
            {
                EXCEPTION1(InvalidVariableException, "");
            }

            cellData = true;
        }

        // Create a dataset copy and a new data array so that the
        // transformed values can be stored in it.
        rv = ds->NewInstance();
        rv->ShallowCopy(ds);

        vtkDataArray *dest = src->NewInstance();
        dest->SetNumberOfTuples(src->GetNumberOfTuples());
        dest->SetName(src->GetName());

        // Transform the scalar data.
        if (atts.GetScaling() == VolumeAttributes::Log)
        {
            TRY
            {
                LogTransform(src, dest);
            }
            CATCH(VisItException)
            {
                dest->Delete();
                rv->Delete();
                RETHROW;
            }
            ENDTRY
        }
        else if (atts.GetScaling() == VolumeAttributes::Skew)
            SkewTransform(src, dest);

        // Add the new data to the return dataset's point data, replacing
        // the old version.
        if( cellData )
            rv->GetCellData()->AddArray(dest);
        else
            rv->GetPointData()->AddArray(dest);

        dest->Delete();
    }

    CalculateHistograms(rv);

    avtDataRepresentation *out_dr = new avtDataRepresentation(rv,
        in_dr->GetDomain(), in_dr->GetLabel());

    if (rv != ds)
        rv->Delete();

    return out_dr;
}

// ****************************************************************************
// Method: avtLowerResolutionVolumeFilter::PostExecute
//
// Purpose:
//   This method stores the histogram into the contract's plot info atts.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 18 14:15:14 PST 2008
//
// Modifications:
//   Brad Whitlock, Fri Jan 16 13:54:49 PST 2009
//   Return early if the histogram data does not exist.
//
// ****************************************************************************

void
avtLowerResolutionVolumeFilter::PostExecute()
{
    StackTimer t("avtLowerResolutionVolumeFilter::PostExecute");

    if(hist == 0)
        return;

    floatVector h1;
    h1.reserve(hist_size);

    for(int i = 0; i < hist_size; ++i)
        h1.push_back(hist[i]);

    MapNode vhist;
    vhist["histogram_size"] = hist_size;
    vhist["histogram_1d"] = h1;

    GetOutput()->GetInfo().GetAttributes().AddPlotInformation("VolumeHistogram", vhist);
}

// ****************************************************************************
//  Method: avtLowerResolutionVolumeFilter::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************

bool
avtLowerResolutionVolumeFilter::FilterUnderstandsTransformedRectMesh()
{
    // The resampling and raycasting algorithms now all understand
    // these kinds of grids, so we can now safely return true.
    return true;
}

