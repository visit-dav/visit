/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                     avtEllipticalCompactnessFactorQuery.C                 //
// ************************************************************************* //

#include <avtEllipticalCompactnessFactorQuery.h>

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
//  Method: avtEllipticalCompactnessFactorQuery constructor
//
//  Programmer: Hank Childs
//  Creation:   May 8, 2006
//
// ****************************************************************************

avtEllipticalCompactnessFactorQuery::avtEllipticalCompactnessFactorQuery()
{
    rev_volume = new avtRevolvedVolume;
    volume = new avtVMetricVolume;
}


// ****************************************************************************
//  Method: avtEllipticalCompactnessFactorQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   May 8, 2006
//
// ****************************************************************************

avtEllipticalCompactnessFactorQuery::~avtEllipticalCompactnessFactorQuery()
{
    delete volume;
    delete rev_volume;
}


// ****************************************************************************
//  Method: avtEllipticalCompactnessFactorQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   May 8, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtEllipticalCompactnessFactorQuery::PreExecute(void)
{
    avtTwoPassDatasetQuery::PreExecute();

    for (int i = 0 ; i < 3 ; i++)
        centroid[i] = 0.;
    total_volume = 0.;

    bounds[0] = +FLT_MAX;
    bounds[1] = -FLT_MAX;
    bounds[2] = +FLT_MAX;
    bounds[3] = -FLT_MAX;
    bounds[4] = +FLT_MAX;
    bounds[5] = -FLT_MAX;
}


// ****************************************************************************
//  Method: avtEllipticalCompactnessFactorQuery::MidExecute
//
//  Purpose:
//      This is called after Execute1 and before Execute2.
//
//  Programmer: Hank Childs
//  Creation:   May 8, 2006
//
// ****************************************************************************

void
avtEllipticalCompactnessFactorQuery::MidExecute(void)
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

    for (int i = 0 ; i < numGuesses ; i++)
        volume_inside[i] = 0.;

    //
    // Now we have to choose the lengths of the major and minor axes, which
    // is hard.  One choice would be to let them vary from 0 to infinity,
    // but we only have a few steps in between, so the steps would be too 
    // coarse.  So we need to narrow the range.  Using the bounds of the
    // data seems to be a good place to start.  But, we don't want to set
    // the length of the axes to be based on these axes, because this
    // won't cover the things at the extrema.  Restated, if X spans (-a,a)
    // and Y spans (0,b) (cylindrical), then choosing a and b as axes lengths
    // would give us an ellipsoid with volume 4/3pi(a)(b)(b), but the cylinder
    // that volume covers would be 2pi(a)(b)(b).  So let's lengthen out a and
    // b by (50%)^1/3.  Then the ellipsoid would have a matching volume.
    //
    if (is2D)
    {
        float Amax = ((bounds[1] - bounds[0]) / 2.)*1.5;
        float Bmax = (bounds[3])*1.5;
        float Bmin = sqrt(total_volume * 0.75 / (M_PI*Amax));
        for (int i = 0 ; i < numGuesses ; i++)
        {
            y_radius[i] = Bmin + (Bmax-Bmin)*((float)i/(float)numGuesses);
            z_radius[i] = y_radius[i];
            x_radius[i] = total_volume*0.75 / (M_PI*y_radius[i]*z_radius[i]);
        }
    }
    else
    {
        float Amax = ((bounds[1] - bounds[0]) / 2.)*1.5;
        float Bmax = ((bounds[3] - bounds[2]) / 2.)*1.5;
        float Cmax = ((bounds[5] - bounds[4]) / 2.)*1.5;
        float Amin = sqrt(total_volume * 0.75 / (M_PI*Bmax*Cmax));
        float Bmin = sqrt(total_volume * 0.75 / (M_PI*Amax*Cmax));
        float Cmin = sqrt(total_volume * 0.75 / (M_PI*Amax*Bmax));
        // Get integer square root.
        int dims = (int) ceil(sqrt(numGuesses-0.1));
        for (int i = 0 ; i < dims ; i++)
        {
            float A = Amin + (Amax-Amin)*((float)i/(float)dims);
            for (int j = 0 ; j < dims ; j++)
            {
                float B = Bmin + (Bmax-Bmin)*((float)j/(float)dims);
                float C = total_volume*0.75 / (M_PI*A*B);
                int index = i*dims+j;
                x_radius[index] = A;
                y_radius[index] = B;
                z_radius[index] = C;
            }
        }
    }

    if (is2D)
    {
        ellipse_center[0] = centroid[0];
        ellipse_center[1] = 0.;
        ellipse_center[2] = 0.;
    }
    else
    {
        ellipse_center[0] = centroid[0];
        ellipse_center[1] = centroid[1];
        ellipse_center[2] = centroid[2];
    }
}


// ****************************************************************************
//  Method: avtEllipticalCompactnessFactorQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   May 8, 2006
//
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
// ****************************************************************************

void
avtEllipticalCompactnessFactorQuery::PostExecute(void)
{
    double vi_tmp[numGuesses];
    SumDoubleArrayAcrossAllProcessors(volume_inside, vi_tmp, numGuesses);
    int biggest = 0;
    double biggestVal = vi_tmp[0];
    for (int i = 1 ; i < numGuesses ; i++)
    {
        if (vi_tmp[i] > biggestVal)
        {
            biggestVal = vi_tmp[i];
            biggest    = i;
        }
    }

    //
    //  Parent class uses this message to set the Results message
    //  in the Query Attributes that is sent back to the viewer.
    //  That is all that is required of this query.
    //
    char msg[4096];
    string floatFormat = queryAtts.GetFloatFormat();
    string format = "Elliptical Compactness Factor = " + floatFormat
                      + ".  Using centroid for ellipse origin."
                      "  Centroid used was (" + floatFormat + ", "
                                              + floatFormat + ", "
                                              + floatFormat + ")."
                      "  Best fitting axes were "  + floatFormat + ","
                                                   + floatFormat + ","
                                                   + floatFormat + ".";
    SNPRINTF(msg, 4096,format.c_str(),
                       biggestVal / total_volume, 
                       ellipse_center[0], ellipse_center[1], ellipse_center[2],
                       x_radius[biggest], y_radius[biggest], z_radius[biggest]);
    SetResultMessage(msg);
    SetResultValue(biggestVal / total_volume);
}


// ****************************************************************************
//  Method: avtEllipticalCompactnessFactorQuery::Execute1
//
//  Purpose:
//      Processes a single domain.  This will calculate the total volume and
//      the centroid on this pass.
//
//  Programmer: Hank Childs
//  Creation:   May 8, 2006
//
// ****************************************************************************

void
avtEllipticalCompactnessFactorQuery::Execute1(vtkDataSet *ds, const int dom)
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
        double center[3];
        vtkVisItUtility::GetCellCenter(cell, center);
        float volume = var->GetTuple1(i);
        volume = (volume < 0 ? -volume : volume);
        centroid[0] += volume*center[0];
        centroid[1] += volume*center[1];
        centroid[2] += volume*center[2];
        bounds[0] = (center[0] < bounds[0] ? center[0] : bounds[0]);
        bounds[1] = (center[0] > bounds[1] ? center[0] : bounds[1]);
        bounds[2] = (center[1] < bounds[2] ? center[1] : bounds[2]);
        bounds[3] = (center[1] > bounds[3] ? center[1] : bounds[3]);
        bounds[4] = (center[2] < bounds[4] ? center[2] : bounds[4]);
        bounds[5] = (center[2] > bounds[5] ? center[2] : bounds[5]);
        total_volume += volume;
    }
}


// ****************************************************************************
//  Method: avtEllipticalCompactnessFactorQuery::Execute2
//
//  Purpose:
//      Processes a single domain.  This will calculate the portion inside
//      the sphere on this pass.
//
//  Programmer: Hank Childs
//  Creation:   May 8, 2006
//
// ****************************************************************************

void
avtEllipticalCompactnessFactorQuery::Execute2(vtkDataSet *ds, const int dom)
{
    int nCells = ds->GetNumberOfCells();
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *var = ds->GetCellData()->GetArray("avt_volume");
    if (var == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }
    float d[3];
    for (int i = 0 ; i < nCells ; i++)
    {
        if (ghosts != NULL && ghosts->GetTuple1(i) != 0.)
            continue;
        vtkCell *cell = ds->GetCell(i);
        double center[3];
        vtkVisItUtility::GetCellCenter(cell, center);
        d[0] = center[0] - ellipse_center[0];
        d[1] = center[1] - ellipse_center[1];
        d[2] = center[2] - ellipse_center[2];
        float volume = var->GetTuple1(i);
        volume = (volume < 0 ? -volume : volume);
        for (int j = 0 ; j < numGuesses ; j++)
        {
            float dist = 0;
            dist += d[0]*d[0] / (x_radius[j]*x_radius[j]);
            dist += d[1]*d[1] / (y_radius[j]*y_radius[j]);
            dist += d[2]*d[2] / (z_radius[j]*z_radius[j]);
            if (dist < 1.)
            {
                volume_inside[j] += volume;
            }
        }
    }
}


// ****************************************************************************
//  Method: avtEllipticalCompactnessFactorQuery::ApplyFilters
//
//  Purpose:
//      Applies the filters to the input.
//
//  Programmer: Hank Childs
//  Creation:   May 8, 2006
//
// ****************************************************************************

avtDataObject_p
avtEllipticalCompactnessFactorQuery::ApplyFilters(avtDataObject_p inData)
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


