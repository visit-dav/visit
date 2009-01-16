/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              avtLowerResolutionVolumeFilter.C             //
// ************************************************************************* //

#include <avtLowerResolutionVolumeFilter.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>

#include <StackTimer.h>
#include <PlotInfoAttributes.h>

#include <DebugStream.h>

#include <VolumeFunctions.h>
#include <VolumeRLEFunctions.h>

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
    hist = 0;
    hist2 = 0;
    hist_size = 256;
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
    if(hist != 0)
    {
        delete [] hist;
        hist = 0;
    }
    if(hist2 != 0)
    {
        delete [] hist2;
        hist2 = 0;
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
    VolumeAttributes *v = (VolumeAttributes *)a;
    atts = *v;
}

// ****************************************************************************
// Method: avtLowerResolutionVolumeFilter::CalculateHistograms
//
// Purpose: 
//   Calculates the histogram data that we'll later put into the plot info.
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
    vtkDataArray *data = 0, *opac = 0;
    if(VolumeGetScalars(atts, ds, data, opac))
    {
        debug5 << mName << "Computing histograms" << endl;
        int nels = data->GetNumberOfTuples();

        // Get the opacity variable's extents.
        float omin = 0.f, omax = 0.f, osize = 0.f;
        VolumeGetOpacityExtents(atts, opac, omin, omax, osize);
        float ghostval = omax+osize;

        //
        // In this mode, we calculate "gm" so we can do the histogram and then
        // we throw away "gm". It's not that big a deal anymore because the
        // gradient calculation is much faster than it used to be.
        //
        vtkFloatArray *gm = vtkFloatArray::New();
        gm->SetNumberOfTuples(nels);
        gm->SetName("gm");
        VolumeCalculateGradient(atts, (vtkRectilinearGrid *)ds, opac, 
                                0, // gx
                                0, // gy
                                0, // gz
                                (float *)gm->GetVoidPointer(0),
                                0, // gmn
                                ghostval);

        if(hist2 != 0)
            delete [] hist2;
        hist2 = new float[hist_size * hist_size];
        if(hist == 0)
            delete [] hist;
        hist = new float[hist_size];
        VolumeHistograms(atts, data, gm, hist, hist2, hist_size);
        gm->Delete();

        data->Delete();
        opac->Delete();
    }
    else
    {
         debug5 << mName << "Could not get scalars or opacity needed to "
                            "calculate the histogram"
                << endl;
    }
}

// ****************************************************************************
// Method: avtLowerResolutionVolumeFilter::ExecuteData
//
// Purpose: 
//   ds
//
// Arguments:
//   ds : The dataset on which we'll operate.
//
// Returns:    The input dataset.
//
// Note:       This filter assumes that there will only be 1 domain. This is
//             fine because we call it after the resample filter.
// 
// Programmer: Brad Whitlock
// Creation:   Thu Dec 18 14:13:43 PST 2008
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtLowerResolutionVolumeFilter::ExecuteData(vtkDataSet *ds, int, std::string)
{
    StackTimer t("avtLowerResolutionVolumeFilter::ExecuteData");

    vtkDataSet *rv = ds;
    // If we're not doing linear scaling then we have to create a copy dataset
    // whose scalars are transformed by the appropriate scaling rule.
    if(atts.GetScaling() != VolumeAttributes::Linear)
    {
        // Get the array that we're "modifying".
        vtkDataArray *src = VolumeGetScalar(atts, ds);
        if(src == 0)
        {
            EXCEPTION0(ImproperUseException);
        }

        // Create a dataset copy and a new data array that we can store
        // the transformed values in.
        rv = ds->NewInstance();
        rv->ShallowCopy(ds);
        vtkDataArray *dest = src->NewInstance();
        dest->SetNumberOfTuples(src->GetNumberOfTuples());
        dest->SetName(src->GetName());

        // Transform the data.
        if (atts.GetScaling() == VolumeAttributes::Log10)
        {
            TRY
            {
                VolumeLogTransform(atts, src, dest);
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
            VolumeSkewTransform(atts, src, dest);

        // Add the new data to the return dataset's point data, replacing
        // the old version.
        rv->GetPointData()->AddArray(dest);
        dest->Delete();
    }

    CalculateHistograms(rv);

    return rv;
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

    if(hist == 0 || hist2 == 0)
        return;

    floatVector        h1;
    unsignedCharVector h2;
    h1.reserve(hist_size);
    h2.reserve(hist_size * hist_size);
    for(int i = 0; i < hist_size; ++i)
        h1.push_back(hist[i]);

    // Convert the 2D hist to uchar and RLE compress it.
    for(int i = 0; i < hist_size * hist_size; ++i)
        h2.push_back((unsigned char)(int)(hist2[i] * 255.));
    unsignedCharVector compressedbuf;
    VolumeRLECompress(h2, compressedbuf);

    MapNode vhist;
    vhist["histogram_size"] = hist_size;
    vhist["histogram_1d"] = h1;
    vhist["histogram_2d"] = compressedbuf;

    GetOutput()->GetInfo().GetAttributes().AddPlotInformation("VolumeHistogram", vhist);
}

// ****************************************************************************
//  Method:  avtLowerResolutionVolumeFilter::FilterUnderstandsTransformedRectMesh
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

