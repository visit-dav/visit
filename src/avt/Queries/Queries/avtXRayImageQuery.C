/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <avtXRayImageQuery.h>

#include <vtkBMPWriter.h>
#include <vtkImageData.h>
#include <vtkJPEGWriter.h>
#include <vtkPointData.h>
#include <vtkPNGWriter.h>
#include <vtkTIFFWriter.h>

#include <avtDatasetExaminer.h>
#include <avtOriginatingSource.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtXRayFilter.h>

#include <vectortypes.h>

#include <visitstream.h>
#include <snprintf.h>

#include <DebugStream.h>
#include <TimingsManager.h>
#include <QueryArgumentException.h>

#include <float.h>
#include <stdio.h>

#include <string>
#include <vector>

int avtXRayImageQuery::iFileFamily = 0;

// ****************************************************************************
//  Method: avtXRayImageQuery::avtXRayImageQuery
//
//  Purpose:
//    Basic constructor
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Sep  7 16:23:06 PDT 2010
//    I reduced the number of pixels per iteration, since it doesn't
//    increase the run time significantly, but reduces the memory
//    requirements significantly.
//
//    Eric Brugger, Tue Dec 28 14:40:43 PST 2010
//    I moved all the logic for doing the ray integration and creating the
//    image in chunks to avtXRayFilter.
//
//    Kathleen Biagas, Tue Jul 26 12:36:11 PDT 2011
//    Changed default nx,ny to 200 (per Eric).
//
//    Kathleen Biagas, Wed Oct 17 12:10:25 PDT 2012
//    Added upVector.
//
//    Kathleen Biagas, Wed Oct 17 14:39:41 PDT 2012
//    Added useSpecifiedUpVector.
//
//    Gunther H. Weber, Wed Jan 23 15:27:53 PST 2013
//    Added support for specifying background intensity entering volume.
//
//    Eric Brugger, Thu Nov 20 16:57:20 PST 2014
//    Added a new way to specify the view that matches the way the view is
//    specified for plots.
//
//    Eric Brugger,Thu Jan 15 13:32:48 PST 2015
//    I added support for specifying background intensities on a per bin
//    basis.
//
//    Eric Brugger, Thu May 21 12:15:59 PDT 2015
//    I added support for debugging a ray.
//
//    Eric Brugger, Wed May 27 14:37:36 PDT 2015
//    I added an option to family output files.
//
// ****************************************************************************

avtXRayImageQuery::avtXRayImageQuery():
    absVarName("absorbtivity"),
    emisVarName("emissivity")
{
    divideEmisByAbsorb = false;
    backgroundIntensity = 0.0;
    backgroundIntensities = NULL;
    nBackgroundIntensities = 0;
    debugRay = -1;
    familyFiles = false;
    outputType = 2; // png
    useSpecifiedUpVector = true;
    useOldView = true;

    //
    // The new view specification
    //
    normal[0] = 0.0;
    normal[1] = 0.0;
    normal[2] = 1.0;
    focus[0] = 0.0;
    focus[1] = 0.0;
    focus[2] = 0.0;
    viewUp[0] = 0.0;
    viewUp[1] = 1.0;
    viewUp[2] = 0.0;
    viewAngle = 30.;
    parallelScale = 0.5;
    nearPlane = -0.5;
    farPlane = 0.5;
    imagePan[0] = 0.0;
    imagePan[1] = 0.0;
    imageZoom = 1.0;
    perspective = true;
    imageSize[0] = 200;
    imageSize[1] = 200;

    //
    // The old view specification
    //
    origin[0] = 0.0;
    origin[1] = 0.0;
    origin[2] = 0.0;
    upVector[0] = 0.0;
    upVector[1] = 1.0;
    upVector[2] = 0.0;
    theta  = 0.0;
    phi    = 0.0;
    width  = 1.0;
    height = 1.0;
    nx     = 200;
    ny     = 200;
    numPixels = nx * ny;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::~avtXRayImageQuery
//
//  Purpose:
//    The destructor.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger,Thu Jan 15 13:32:48 PST 2015
//    I added support for specifying background intensities on a per bin
//    basis.
//
// ****************************************************************************

avtXRayImageQuery::~avtXRayImageQuery()
{
    if (backgroundIntensities != NULL)
        delete [] backgroundIntensities;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetInputParams
//
//  Purpose:
//    Set the input parameters.
//
//  Programmer: Kathleen Biagas 
//  Creation:   May 17, 2011
//
//  Modifications:
//    Kathleen Biagas, Wed Oct 17 12:10:25 PDT 2012
//    Added upVector.
//
//    Kathleen Biagas, Wed Oct 17 14:39:41 PDT 2012
//    Added useSpecifiedUpVector.
//
//    Kathleen Biagas, Thu Jan 10 08:11:20 PST 2013
//    Added error checking.
//
//    Gunther H. Weber, Wed Jan 23 15:27:53 PST 2013
//    Added support for specifying background intensity entering volume.
//
//    Eric Brugger, Thu Nov 20 16:57:20 PST 2014
//    Added a new way to specify the view that matches the way the view is
//    specified for plots.
//
//    Eric Brugger,Thu Jan 15 13:32:48 PST 2015
//    I added support for specifying background intensities on a per bin
//    basis.
//
//    Eric Brugger, Thu May 21 12:15:59 PDT 2015
//    I added support for debugging a ray.
//
//    Eric BBrugger, Tue May 26 11:47:12 PDT 2015
//    I corrected a bug with the processing of image_pan, where the values
//    were being interpreted as integers instead of doubles.
//
//    Eric Brugger, Wed May 27 14:37:36 PDT 2015
//    I added an option to family output files.
//
// ****************************************************************************

void
avtXRayImageQuery::SetInputParams(const MapNode &params)
{
    if (params.HasEntry("vars"))
    {
        stringVector v = params.GetEntry("vars")->AsStringVector();
        if (v.size() != 1 && v.size() != 2)
            EXCEPTION2(QueryArgumentException, "vars", 2);
        SetVariableNames(v);
    }
    else
        EXCEPTION1(QueryArgumentException, "vars");

    if (params.HasNumericEntry("divide_emis_by_absorb"))
        SetDivideEmisByAbsorb(params.GetEntry("divide_emis_by_absorb")->ToBool());

    if (params.HasNumericEntry("background_intensity"))
        SetBackgroundIntensity(params.GetEntry("background_intensity")->ToDouble());

    if (params.HasNumericVectorEntry("background_intensities"))
    {
        doubleVector v;
        params.GetEntry("background_intensities")->ToDoubleVector(v);
        SetBackgroundIntensities(v);
    }

    if (params.HasNumericEntry("debug_ray"))
        SetDebugRay(params.GetEntry("debug_ray")->AsInt());

    if (params.HasNumericEntry("family_files"))
        SetFamilyFiles(params.GetEntry("family_files")->ToBool());

    if (params.HasEntry("output_type"))
    {
        if (params.GetEntry("output_type")->TypeName() == "int")
            SetOutputType(params.GetEntry("output_type")->AsInt());
        else if (params.GetEntry("output_type")->TypeName() == "string")
            SetOutputType(params.GetEntry("output_type")->AsString());
    }

    // this is not a normal parameter, it is set by the cli when the query
    // is called with the deprecated argument parsing.
    if (params.HasNumericEntry("useUpVector"))
    {
        useSpecifiedUpVector = params.GetEntry("useUpVector")->ToBool();
    }

    useOldView = false;

    //
    // The new view parameters.
    //
    if (params.HasNumericVectorEntry("normal"))
    {
        doubleVector v;
        params.GetEntry("normal")->ToDoubleVector(v);
        if (v.size() != 3)
            EXCEPTION2(QueryArgumentException, "normal", 3);
        normal[0] = v[0]; normal[1] = v[1]; normal[2] = v[2];
    }

    if (params.HasNumericVectorEntry("focus"))
    {
        doubleVector v;
        params.GetEntry("focus")->ToDoubleVector(v);
        if (v.size() != 3)
            EXCEPTION2(QueryArgumentException, "focus", 3);
        focus[0] = v[0]; focus[1] = v[1]; focus[2] = v[2];
    }

    if (params.HasNumericVectorEntry("view_up"))
    {
        doubleVector v;
        params.GetEntry("view_up")->ToDoubleVector(v);
        if (v.size() != 3)
            EXCEPTION2(QueryArgumentException, "view_up", 3);
        viewUp[0] = v[0]; viewUp[1] = v[1]; viewUp[2] = v[2];
    }

    if (params.HasNumericEntry("view_angle"))
    {
        viewAngle = params.GetEntry("view_angle")->ToDouble();
    }

    if (params.HasNumericEntry("parallel_scale"))
    {
        parallelScale = params.GetEntry("parallel_scale")->ToDouble();
    }

    if (params.HasNumericEntry("near_plane"))
    {
        nearPlane = params.GetEntry("near_plane")->ToDouble();
    }

    if (params.HasNumericEntry("far_plane"))
    {
        farPlane = params.GetEntry("far_plane")->ToDouble();
    }

    if (params.HasNumericVectorEntry("image_pan"))
    {
        doubleVector v;
        params.GetEntry("image_pan")->ToDoubleVector(v);
        if (v.size() != 2)
            EXCEPTION2(QueryArgumentException, "image_pan", 2);
        imagePan[0] = v[0]; imagePan[1] = v[1];
    }

    if (params.HasNumericEntry("image_zoom"))
    {
        imageZoom = params.GetEntry("image_zoom")->ToDouble();
    }

    if (params.HasNumericEntry("perspective"))
    {
        perspective = params.GetEntry("perspective")->ToBool();
    }

    if (params.HasNumericVectorEntry("image_size"))
    {
        intVector v;
        params.GetEntry("image_size")->ToIntVector(v);
        if (v.size() != 2)
            EXCEPTION2(QueryArgumentException, "image_size", 2);
        imageSize[0] = v[0]; imageSize[1] = v[1];
        SetImageSize(v);
    }

    //
    // The old view parameters.
    //
    if (params.HasNumericVectorEntry("origin"))
    {
        doubleVector v;
        params.GetEntry("origin")->ToDoubleVector(v);
        if (v.size() != 3)
            EXCEPTION2(QueryArgumentException, "origin", 3);
        SetOrigin(v);
        useOldView = true;
    }

    if (params.HasNumericVectorEntry("up_vector"))
    {
        doubleVector v;
        params.GetEntry("up_vector")->ToDoubleVector(v);
        if (v.size() != 3)
            EXCEPTION2(QueryArgumentException, "up_vector", 3);
        SetUpVector(v);
        useOldView = true;
    }

    if (params.HasNumericEntry("theta"))
    {
        SetTheta(params.GetEntry("theta")->ToDouble());
        useOldView = true;
    }

    if (params.HasNumericEntry("phi"))
    {
        SetPhi(params.GetEntry("phi")->ToDouble());
        useOldView = true;
    }

    if (params.HasNumericEntry("width"))
    {
        SetWidth(params.GetEntry("width")->ToDouble());
        useOldView = true;
    }

    if (params.HasNumericEntry("height"))
    {
        SetHeight(params.GetEntry("height")->ToDouble());
        useOldView = true;
    }
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetVariableNames
//
//  Purpose:
//    Set the names of absorbtion and emission variables.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::SetVariableNames(const stringVector &names)
{
    if (names.size() != 1 && names.size() != 2)
        EXCEPTION1(VisItException, "Not able to find the absorption and "
                   "emissivity variable names.");
    absVarName  = names[0];
    if (names.size() == 2)
        emisVarName = names[1];
    else
        // They put in the same name twice.
        emisVarName = names[0];
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetOrigin
//
//  Purpose:
//    Set the origin of the image plane.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Kathleen Biagas, Fri Jun 17 15:17:32 PDT 2011
//    Changed arg to doubleVector to match what is stored in InputParams.
//
// ****************************************************************************

void
avtXRayImageQuery::SetOrigin(const doubleVector &_origin)
{
    if (_origin.size() != 3)
        EXCEPTION1(VisItException, "origin should have 3 elements.");
    origin[0] = _origin[0];
    origin[1] = _origin[1];
    origin[2] = _origin[2];
}

void
avtXRayImageQuery::SetOrigin(const intVector &_origin)
{
    if (_origin.size() != 3)
        EXCEPTION1(VisItException, "origin should have 3 elements.");
    origin[0] = (double)_origin[0];
    origin[1] = (double)_origin[1];
    origin[2] = (double)_origin[2];
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetUpVector
//
//  Purpose:
//    Set the up-vector of the image plane.
//
//  Programmer: Kathleen Biagas
//  Creation:   October 17, 2012
//
//  Modifications:
//
// ****************************************************************************

void
avtXRayImageQuery::SetUpVector(const doubleVector &_upvector)
{
    if (_upvector.size() != 3)
        EXCEPTION1(VisItException, "up_vector should have 3 elements.");
    upVector[0] = _upvector[0];
    upVector[1] = _upvector[1];
    upVector[2] = _upvector[2];
}

void
avtXRayImageQuery::SetUpVector(const intVector &_upvector)
{
    if (_upvector.size() != 3)
        EXCEPTION1(VisItException, "up_vector should have 3 elements.");
    upVector[0] = (double)_upvector[0];
    upVector[1] = (double)_upvector[1];
    upVector[2] = (double)_upvector[2];
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetTheta
//
//  Purpose:
//    Set the theta and phi offsets from the z-axis in 3D, or the x-axis in 2D.
//    In 3D, theta is a rotation about the x-axis, and phi is a rotation about
//    the z-axis.  In 2D, only theta is used.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Kathleen Biagas, Fri Jun 17 15:17:32 PDT 2011
//    Changed arg to doubleVector to match what is stored in InputParams.
//
// ****************************************************************************

void
avtXRayImageQuery::SetTheta(const double &thetaInDegrees)
{
    theta = thetaInDegrees * M_PI / 180.0;
}

void
avtXRayImageQuery::SetPhi(const double &phiInDegrees)
{
    phi   = phiInDegrees * M_PI / 180.0;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetWidth
//
//  Purpose:
//    Set the width of the image.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtXRayImageQuery::SetWidth(const double &size) 
{
    width  = size;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetHeight
//
//  Purpose:
//    Set the width of the image.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//
// ****************************************************************************

void
avtXRayImageQuery::SetHeight(const double &size) 
{
    height = size;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetImageSize
//
//  Purpose:
//    Set the size of the image.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Kathleen Biagas, Fri Jun 17 15:17:32 PDT 2011
//    Changed arg to intVector to match what is stored in InputParams.
//
// ****************************************************************************

void
avtXRayImageQuery::SetImageSize(const intVector &size)
{
    if (size.size() != 2)
        EXCEPTION1(VisItException, "image_size should have 2 elements.");
    nx = size[0];
    ny = size[1];
    numPixels = nx * ny;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetDivideEmisByAbsorb
//
//  Purpose:
//    Set the flag that controls if the emissivity divided by the absorbtivity
//    is used in place of the emissivity.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::SetDivideEmisByAbsorb(const bool &flag)
{
    divideEmisByAbsorb = flag;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetBackgroundIntensity
//
//  Purpose:
//    Set the background intensity entering the volume
//
//  Programmer: Gunther H. Weber
//  Creation:   January 23, 2013
//
// ****************************************************************************

void
avtXRayImageQuery::SetBackgroundIntensity(const double &intensity)
{
    backgroundIntensity = intensity;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetBackgroundIntensities
//
//  Purpose:
//    Set the background intensities entering the volume on a per bin basis.
//
//  Programmer: Eric Brugger
//  Creation:   January 15, 2015
//
// ****************************************************************************

void
avtXRayImageQuery::SetBackgroundIntensities(const doubleVector &intensities)
{
    if (backgroundIntensities != NULL)
        delete [] backgroundIntensities;

    backgroundIntensities = new double[intensities.size()];
    for (int i = 0; i < intensities.size(); i++)
        backgroundIntensities[i] = intensities[i];
    nBackgroundIntensities = intensities.size();
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetDebugRay
//
//  Purpose:
//    Set the id of the debug ray.
//
//  Programmer: Eric Brugger
//  Creation:   May 21, 2015
//
// ****************************************************************************

void
avtXRayImageQuery::SetDebugRay(const int &ray)
{
    debugRay = ray;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetFamilyFiles
//
//  Purpose:
//    Set the family files flag.
//
//  Programmer: Eric Brugger
//  Creation:   May 27, 2015
//
// ****************************************************************************

void
avtXRayImageQuery::SetFamilyFiles(const bool &flag)
{
    familyFiles = flag;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetOutputType
//
//  Purpose:
//    Set the output image type.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::SetOutputType(int type)
{
    outputType = type;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetOutputType
//
//  Purpose:
//    Set the output image type.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 13, 2011
//
//  Modifications:
//    Eric Brugger, Mon May 14 10:35:27 PDT 2012
//    I added the bov output type.
//
// ****************************************************************************

void
avtXRayImageQuery::SetOutputType(const std::string &type)
{
    if      (type == "bmp")
        outputType = 0;
    else if (type == "jpeg")
        outputType = 1;
    else if (type == "png")
        outputType = 2;
    else if (type == "tif")
        outputType = 3;
    else if (type == "rawfloats")
        outputType = 4;
    else if (type == "bov")
        outputType = 5;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::Execute
//
//  Purpose:
//    Dummy routine used to satisfy a pure virtual method.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Dec 28 14:40:43 PST 2010
//    I moved all the logic for doing the ray integration and creating the
//    image in chunks to avtXRayFilter.
//
// ****************************************************************************

void
avtXRayImageQuery::Execute(vtkDataSet *ds, const int chunk)
{
}

// ****************************************************************************
//  Method: avtXRayImageQuery::GetSecondaryVars
//
//  Purpose:
//    Adds the user-specified absorption and emission variables, using those 
//    for the query rather than the currently plotted variable.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
// ****************************************************************************

void
avtXRayImageQuery::GetSecondaryVars(std::vector<std::string> &outVars)
{
    outVars.clear();
    outVars.push_back(absVarName);
    outVars.push_back(emisVarName);
}

// ****************************************************************************
//  Method: avtXRayImageQuery::Execute
//
//  Purpose:
//    Applies the xray filter to the data set and writes out the resulting
//    images.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Fri Jul 16 15:43:35 PDT 2010
//    I modified the query to handle the case where some of the processors
//    didn't have any data sets when executing in parallel.
//
//    Eric Brugger, Tue Sep  7 16:23:06 PDT 2010
//    I added logic to detect RZ meshes with negative R values up front
//    before any processing to avoid more complex error handling during
//    pipeline execution.
//
//    Eric Brugger, Tue Dec 28 14:40:43 PST 2010
//    I moved all the logic for doing the ray integration and creating the
//    image in chunks to avtXRayFilter.
//
//    Eric Brugger, Mon May 14 10:35:27 PDT 2012
//    I added the bov output type.
//
//    Kathleen Biagas, Wed Oct 17 14:41:28 PDT 2012
//    Send upVector to avtXRayFilter.
//
//    Gunther H. Weber, Wed Jan 23 15:27:53 PST 2013
//    Added support for specifying background intensity entering volume.
//
//    Eric Brugger, Thu Nov 20 16:57:20 PST 2014
//    Added a new way to specify the view that matches the way the view is
//    specified for plots.
//
//    Eric Brugger,Thu Jan 15 13:32:48 PST 2015
//    I added support for specifying background intensities on a per bin
//    basis.
//
//    Eric Brugger, Thu May 21 12:15:59 PDT 2015
//    I added support for debugging a ray.
//
//    Eric Brugger, Wed May 27 10:47:29 PDT 2015
//    I modified the query to also output the path length field when
//    outputting in bof or bov format.
//
//    Eric Brugger, Wed May 27 14:37:36 PDT 2015
//    I added an option to family output files.
//
// ****************************************************************************

void
avtXRayImageQuery::Execute(avtDataTree_p tree)
{
    avtDataset_p input = GetTypedInput();

    //
    // If the number of pixels is less than or equal to zero then print
    // an error message and exit.
    //
    if (numPixels <= 0)
    {
        SetResultMessage("VisIt is unable to execute this query because "
                         "the number of pixels specified is less than or "
                         "equal to zero.");
        return;
    }

    //
    // If the grid is 2d and the R coordinates include negative values then
    // print an error message and exit.
    //
    if (input->GetInfo().GetAttributes().GetSpatialDimension() == 2)
    {
        double extents[6] = {0., 0., 0., 0., 0., 0.};
        avtDatasetExaminer::GetSpatialExtents(input, extents);
        UnifyMinMax(extents, 6);
        if (extents[2] < 0.)
        {
            SetResultMessage("VisIt is unable to execute this query "
                             "because it has encountered an RZ mesh with "
                             "negative R values.");
            return;
        }
    }

    int t1 = visitTimer->StartTimer();

    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, input);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    avtXRayFilter *filt = new avtXRayFilter;

    if (useOldView)
        ConvertOldImagePropertiesToNew();
    filt->SetImageProperties(normal, focus, viewUp, viewAngle, parallelScale,
        nearPlane, farPlane, imagePan, imageZoom, perspective, imageSize);

    filt->SetDivideEmisByAbsorb(divideEmisByAbsorb);
    filt->SetBackgroundIntensity(backgroundIntensity);
    filt->SetBackgroundIntensities(backgroundIntensities,
        nBackgroundIntensities);
    filt->SetDebugRay(debugRay);
    filt->SetVariableNames(absVarName, emisVarName);
    filt->SetInput(dob);

    //
    // Cause our artificial pipeline to execute.
    //
    avtContract_p contract =
        input->GetOriginatingSource()->GetGeneralContract();
    filt->GetOutput()->Update(contract);

    //
    // Get the output and write out the image.
    //
    int t2 = visitTimer->StartTimer();
    if (PAR_Rank() == 0)
    {
        //
        // Get the output.
        //
        avtDataTree_p tree = filt->GetTypedOutput()->GetDataTree();

        int numLeaves;
        vtkDataSet **leaves;
        leaves = tree->GetAllLeaves(numLeaves);

        if (numLeaves <= 0)
        {
            // Free the memory from the GetAllLeaves function call.
            delete [] leaves;
            delete filt;

            EXCEPTION1(VisItException, "There must be at least one bin.");
        }

        //
        // Create the file base name.
        //
        const char *exts[6] = {"bmp", "jpeg", "png", "tif", "bof", "bov"};
        char baseName[512];
        bool keepTrying = true;
        while (keepTrying)
        {
            keepTrying = false;
            if (familyFiles)
            {
                //
                // Create the file base name and increment the family number.
                //
                SNPRINTF(baseName, 512, "output%04d.", iFileFamily);
                if (iFileFamily < 9999) iFileFamily++;

                //
                // Check if the first file created with the file base name
                // exists. If it does and we aren't at the maximum, try
                // the next file base name in the sequence.
                //
                char fileName[512];
                SNPRINTF(fileName, 512, "%s00.%s", baseName, exts[outputType]);

                ifstream ifile(fileName);
                if (!ifile.fail() && iFileFamily < 9999)
                {
                    keepTrying = true;
                }
            }
            else
            {
                SNPRINTF(baseName, 512, "output");
            }
        }

        //
        // Write out the intensity and path length. The path length is only
        // put out when the output format is bof or bov.
        //
        int numBins = numLeaves / 2;

        vtkDataArray *intensity;
        vtkDataArray *pathLength;
        if (outputType >= 0 && outputType <=3)
        {
            for (int i = 0; i < numBins; i++)
            {
                intensity= leaves[i]->GetPointData()->GetArray("Intensity");
                if (intensity->GetDataType() == VTK_FLOAT)
                    WriteImage(baseName, i, numPixels,
                        (float*) intensity->GetVoidPointer(0));
                else if (intensity->GetDataType() == VTK_DOUBLE)
                    WriteImage(baseName, i, numPixels,
                        (double*) intensity->GetVoidPointer(0));
                else if (intensity->GetDataType() == VTK_INT)
                    WriteImage(baseName, i, numPixels,
                        (int*) intensity->GetVoidPointer(0));
            }
        }
        else if (outputType == 4)
        {
            for (int i = 0; i < numBins; i++)
            {
                intensity = leaves[i]->GetPointData()->GetArray("Intensity");
                pathLength = leaves[numBins+i]->GetPointData()->GetArray("PathLength");
                if (intensity->GetDataType() == VTK_FLOAT)
                {
                    WriteFloats(baseName, i, numPixels,
                        (float*)intensity->GetVoidPointer(0));
                    WriteFloats(baseName, numBins+i, numPixels,
                        (float*)pathLength->GetVoidPointer(0));
                }
                else if (intensity->GetDataType() == VTK_DOUBLE)
                {
                    WriteFloats(baseName, i, numPixels,
                        (double*)intensity->GetVoidPointer(0));
                    WriteFloats(baseName, numBins+i, numPixels,
                        (double*)pathLength->GetVoidPointer(0));
                }
                else if (intensity->GetDataType() == VTK_INT)
                {
                    WriteFloats(baseName, i, numPixels,
                        (int*)intensity->GetVoidPointer(0));
                    WriteFloats(baseName, numBins+i, numPixels,
                        (int*)pathLength->GetVoidPointer(0));
                }
            }
        }
        else if (outputType == 5)
        {
            for (int i = 0; i < numBins; i++)
            {
                intensity = leaves[i]->GetPointData()->GetArray("Intensity");
                pathLength = leaves[numBins+i]->GetPointData()->GetArray("PathLength");
                if (intensity->GetDataType() == VTK_FLOAT)
                {
                    WriteFloats(baseName, i, numPixels,
                        (float*)intensity->GetVoidPointer(0));
                    WriteBOVHeader(baseName, "intensity", i, nx, ny, "FLOAT");
                    WriteFloats(baseName, numBins+i, numPixels,
                        (float*)pathLength->GetVoidPointer(0));
                    WriteBOVHeader(baseName, "path_length", numBins+i,
                        nx, ny, "FLOAT");
                }
                else if (intensity->GetDataType() == VTK_DOUBLE)
                {
                    WriteFloats(baseName, i, numPixels,
                        (double*)intensity->GetVoidPointer(0));
                    WriteBOVHeader(baseName, "intensity", i, nx, ny, "DOUBLE");
                    WriteFloats(baseName, numBins+i, numPixels,
                        (double*)pathLength->GetVoidPointer(0));
                    WriteBOVHeader(baseName, "path_length", numBins+i,
                        nx, ny, "FLOAT");
                }
                else if (intensity->GetDataType() == VTK_INT)
                {
                    WriteFloats(baseName, i, numPixels,
                        (int*)intensity->GetVoidPointer(0));
                    WriteBOVHeader(baseName, "intensity", i, nx, ny, "INT");
                    WriteFloats(baseName, numBins+i, numPixels,
                        (int*)pathLength->GetVoidPointer(0));
                    WriteBOVHeader(baseName, "path_length", numBins+i,
                        nx, ny, "FLOAT");
                }
            }
        }

        //
        // Output the result message.
        //
        if (outputType >=0 && outputType <= 5)
        {
            std::string msg = "";
            char buf[512];
    
            if (numBins == 1 && outputType < 4)
            {
                SNPRINTF(buf, 512, "The x ray image query results were "
                         "written to the file %s00.%s\n", baseName,
                         exts[outputType]);
            }
            else
                if (outputType < 4)
                {
                    SNPRINTF(buf, 512, "The x ray image query results were "
                        "written to the files %s00.%s - %s%02d.%s\n",
                        baseName, exts[outputType], baseName, numBins - 1,
                        exts[outputType]);
                }
                else
                {
                    SNPRINTF(buf, 512, "The x ray image query results were "
                        "written to the files %s00.%s - %s%02d.%s\n",
                        baseName, exts[outputType], baseName, 2*numBins - 1,
                        exts[outputType]);
                }
            msg += buf;

            SetResultMessage(msg);
        }
        else
        {
            SetResultMessage("No x ray image query results were written "
                             "because the output type was invalid\n");
        }

        // Free the memory from the GetAllLeaves function call.
        delete [] leaves;
    }
    visitTimer->StopTimer(t2, "avtXRayImageQuery::WriteImage");

    visitTimer->StopTimer(t1, "avtXRayImageQuery::ExecutePipeline");

    delete filt;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteImage
//
//  Purpose:
//    Write the image in the appropriate format.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Dec 28 14:40:43 PST 2010
//    I moved all the logic for doing the ray integration and creating the
//    image in chunks to avtXRayFilter.
//
//    Eric Brugger, Wed May 27 14:37:36 PDT 2015
//    I added an option to family output files.
//
// ****************************************************************************

template <typename T>
void
avtXRayImageQuery::WriteImage(const char *baseName, int iImage, int nPixels,
    T *fbuf)
{
    //
    // Determine the range of the data excluding values less than zero.
    //
    double minVal = FLT_MAX;
    double maxVal = -FLT_MAX;
    for (int i = 0; i < nPixels; i++)
    {
        if (fbuf[i] > 0.)
        {
            minVal = fbuf[i] < minVal ?  fbuf[i] : minVal;
            maxVal = fbuf[i] > maxVal ?  fbuf[i] : maxVal;
        }
    }
    double range = maxVal - minVal;
    
    vtkImageData *image = vtkImageData::New();
    image->SetExtent(0, nx-1, 0, ny-1, 0, 0);
    image->SetSpacing(1., 1., 1.);
    image->SetOrigin(0., 0., 0.);
    image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    unsigned char *pixels = (unsigned char *)image->GetScalarPointer(0, 0, 0);

    unsigned char pixel;
    unsigned char *ipixel = pixels;
    for (int i = 0; i < nPixels; i++)
    {
        pixel = (unsigned char)
            ((fbuf[i] < minVal) ? 0 : (254. * ((fbuf[i] - minVal) / range)));
        *ipixel = pixel;
        ipixel++;
        *ipixel = pixel;
        ipixel++;
        *ipixel = pixel;
        ipixel++;
    }

    if (outputType == 0)
    {
        vtkImageWriter *writer = vtkBMPWriter::New();
        char fileName[24];
        sprintf(fileName, "%s%02d.bmp", baseName, iImage);
        writer->SetFileName(fileName);
        writer->SetInputData(image);
        writer->Write();
        writer->Delete();
    }
    else if (outputType == 1)
    {
        vtkImageWriter *writer = vtkJPEGWriter::New();
        char fileName[24];
        sprintf(fileName, "%s%02d.jpg", baseName, iImage);
        writer->SetFileName(fileName);
        writer->SetInputData(image);
        writer->Write();
        writer->Delete();
    }
    else if (outputType == 2)
    {
        vtkImageWriter *writer = vtkPNGWriter::New();
        char fileName[24];
        sprintf(fileName, "%s%02d.png", baseName, iImage);
        writer->SetFileName(fileName);
        writer->SetInputData(image);
        writer->Write();
        writer->Delete();
    }
    else if (outputType == 3)
    {
        vtkImageWriter *writer = vtkTIFFWriter::New();
        char fileName[24];
        sprintf(fileName, "%s%02d.tif", baseName, iImage);
        writer->SetFileName(fileName);
        writer->SetInputData(image);
        writer->Write();
        writer->Delete();
    }
}

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteFloats
//
//  Purpose:
//    Write the image as a block of floats.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Dec 28 14:40:43 PST 2010
//    I moved all the logic for doing the ray integration and creating the
//    image in chunks to avtXRayFilter.
//
//    Eric Brugger, Wed May 27 14:37:36 PDT 2015
//    I added an option to family output files.
//
// ****************************************************************************

template <typename T>
void
avtXRayImageQuery::WriteFloats(const char *baseName, int iImage, int nPixels,
    T *fbuf)
{
    char fileName[512];
    sprintf(fileName, "%s%02d.bof", baseName, iImage);
    FILE *file = fopen(fileName, "w");
    fwrite(fbuf, sizeof(T), nPixels, file);
    fclose(file);
}

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBOVHeader
//
//  Purpose:
//    Write the header file for a brick of values file.
//
//  Programmer: Eric Brugger
//  Creation:   May 14, 2012
//
//  Modifications:
//    Eric Brugger, Wed May 27 10:47:29 PDT 2015
//    I modified the query to also output the path length field when
//    outputting in bof or bov format.
//
//    Eric Brugger, Wed May 27 14:37:36 PDT 2015
//    I added an option to family output files.
//
// ****************************************************************************

void
avtXRayImageQuery::WriteBOVHeader(const char *baseName, const char *varName,
    int iBin, int nx, int ny, const char *type)
{
    char fileName[24];
    sprintf(fileName, "%s%02d.bov", baseName, iBin);
    FILE *file = fopen(fileName, "w");
    fprintf(file, "TIME: 0\n");
    fprintf(file, "DATA_FILE: %s%02d.bof\n", baseName, iBin);
    fprintf(file, "DATA_SIZE: %d %d 1\n", nx, ny);
    fprintf(file, "DATA_FORMAT: %s\n", type);
    fprintf(file, "VARIABLE: %s\n", varName);
    const int one = 1;
    unsigned char *ptr = (unsigned char *)&one;
    if (ptr[0] == 1)
        fprintf(file, "DATA_ENDIAN: LITTLE\n");
    else
        fprintf(file, "DATA_ENDIAN: BIG\n");
    fprintf(file, "CENTERING: zonal\n");
    fprintf(file, "BRICK_ORIGIN: 1 1 1\n");
    fprintf(file, "BRICK_SIZE: %d %d 1\n", nx, ny);
    fclose(file);
}

// ****************************************************************************
//  Method: avtXRayImageQuery::GetDefaultInputParams
//
//  Purpose:
//    Retrieves default values for input variables. 
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 15, 2011
//
//  Modifications:
//    Kathleen Biagas, Wed Oct 17 12:10:25 PDT 2012
//    Added up_vector.
//
//    Eric Brugger, Thu Nov 20 16:57:20 PST 2014
//    Added a new way to specify the view that matches the way the view is
//    specified for plots.
//
//    Eric Brugger, Thu May 21 12:15:59 PDT 2015
//    I added support for debugging a ray.
//
// ****************************************************************************

void
avtXRayImageQuery::GetDefaultInputParams(MapNode &params)
{
    stringVector v;
    v.push_back("absorbtivity");
    v.push_back("emissivity");
    params["vars"] = v;

    params["divide_emis_by_absorb"] = 0;
    params["background_intensity"] = 0.0;
    params["debug_ray"] = -1;
    params["output_type"] = std::string("png");

    //
    // The new view parameters.
    //
    doubleVector n;
    n.push_back(0.0);
    n.push_back(0.0);
    n.push_back(1.0);
    params["normal"] = n;

    doubleVector f;
    f.push_back(0.0);
    f.push_back(0.0);
    f.push_back(0.0);
    params["focus"] = f;

    doubleVector vu;
    vu.push_back(0.0);
    vu.push_back(1.0);
    vu.push_back(0.0);
    params["view_up"] = vu;

    params["view_angle"] = 30.;
    params["parallel_scale"] = 0.5;
    params["near_plane"] = -0.5;
    params["far_plane"] = 0.5;

    doubleVector ip;
    ip.push_back(0.0);
    ip.push_back(0.0);
    params["image_pan"] = ip;

    params["image_zoom"] = 1.;
    params["perspective"] = 1;

    intVector is;
    is.push_back(200);
    is.push_back(200);
    params["image_size"] = is;

    //
    // The old view parameters.
    //
    doubleVector o;
    o.push_back(0.0);
    o.push_back(0.0);
    o.push_back(0.0);
    params["origin"] = o;

    doubleVector uv;
    uv.push_back(0.0);
    uv.push_back(1.0);
    uv.push_back(0.0);
    params["up_vector"] = uv;

    params["theta"] = 0.0;
    params["phi"] = 0.0;

    params["width"] = 1.0;
    params["height"] = 1.0;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::ConvertOldImagePropertiesToNew
//
//  Purpose:
//    Convert the old image specification to the new image specification.
//
//  Programmer: Eric Brugger
//  Creation:   November 20, 2014
//
// ****************************************************************************

void
avtXRayImageQuery::ConvertOldImagePropertiesToNew()
{
    double cosT = cos(theta);
    double sinT = sin(theta);
    double cosP = cos(phi);
    double sinP = sin(phi);

    normal[0] = sinT*cosP;
    normal[1] = sinT*sinP;
    normal[2] = cosT;
    focus[0] = origin[0];
    focus[1] = origin[1];
    focus[2] = origin[2];
    if (useSpecifiedUpVector)
    {
        viewUp[0] = upVector[0];
        viewUp[1] = upVector[1];
        viewUp[2] = upVector[2];
    }
    else
    {
        viewUp[0] = -sinP;
        viewUp[1] = cosP;
        viewUp[2] = 0;
    }
    viewAngle = 30;
    parallelScale = height / 2.;

    //
    // The 10 below is to get the results to match previous results more
    // closely. In theory this constant multiplier shouldn't matter, but
    // it matters along zone boundaries.
    //
    nearPlane = -10 * height;
    farPlane = 10 * height;
    imagePan[0] = 0;
    imagePan[1] = 0;
    imageZoom = 1;
    perspective = false;
    imageSize[0] = nx;
    imageSize[1] = ny;
}
