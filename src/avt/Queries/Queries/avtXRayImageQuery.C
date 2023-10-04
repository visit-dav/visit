// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtXRayImageQuery.h>

#include <vtkImageData.h>
#include <vtkJPEGWriter.h>
#include <vtkPointData.h>
#include <vtkPNGWriter.h>
#include <vtkTIFFWriter.h>
#include <vtkDataArray.h>
#include <vtkCellData.h>

#include <avtDatasetExaminer.h>
#include <avtOriginatingSource.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtXRayFilter.h>

#include <vectortypes.h>

#include <visitstream.h>

#include <DebugStream.h>
#include <TimingsManager.h>
#include <QueryArgumentException.h>

#include <float.h>
#include <stdio.h>

#include <string>
#include <vector>

int avtXRayImageQuery::iFileFamily = 0;

const int NUMFAMILYFILES = 9999;

// 
// Filename Scheme information and handling
// 

// To add new filename schemes, these are the changes to make:
//    1) increment `NUM_FILENAME_SCHEMES` by however many filename schemes you are planning to add
//    2) add new entries to the `filename_schemes` array
//    3) add new constants for your filename schemes (make sure in the same order as in `filename_schemes`)
//    4) add new cases where necessary (probably just in `avtXRayImageQuery::Execute` baseName setup)
//    5) add them to `src/gui/QvisXRayImageQueryWidget.C` in the constructor.

// a constant for how many valid filename schemes there are
const int NUM_FILENAME_SCHEMES = 3;

// member filenameScheme indexes this array.
const char *filename_schemes[NUM_FILENAME_SCHEMES] = {"none", "family", "cycle"};

// constants for each of the filename schemes
const int NONE = 0;
const int FAMILYFILES = 1;
const int CYCLEFILES = 2;

// a filename type is valid if it is an int in [0,3)
inline bool filenameSchemeValid(int ftype)
{
    return ftype >= 0 && ftype < 3;
}

//
// Output Type information and handling
//

// To add new output types, these are the changes to make:
//    1) increment `NUM_OUTPUT_TYPES` by however many output types you are planning to add
//    2) add new entries to the `file_protocols` and `file_extensions` arrays
//    3) add new constants for your output types (make sure in the same order as in `file_protocols`)
//    4) optional: add new inline functions to check for your output type or types; see below for examples
//    5) add new cases where necessary (probably just in `avtXRayImageQuery::Execute`)
//    6) add them to `src/gui/QvisXRayImageQueryWidget.C` in the constructor.

// a constant for how many valid output types there are
const int NUM_OUTPUT_TYPES = 8;

// member `outputType` indexes these arrays
const char *file_protocols[NUM_OUTPUT_TYPES] = {"jpeg", "png", "tif", "bof", "bov", 
    /*conduit blueprint output types */ "json", "hdf5", "yaml"}; // removed conduit_bin and conduit_json
const char *file_extensions[NUM_OUTPUT_TYPES] = {"jpg", "png", "tif", "bof", "bov", 
    /*conduit blueprint output types */ "root", "root", "root"};

// constants for each of the output types
const int JPEG_OUT = 0;
const int PNG_OUT = 1;
const int TIF_OUT = 2;
const int RAWFLOATS_OUT = 3;
const int BOV_OUT = 4;
const int BLUEPRINT_JSON_OUT = 5;
const int BLUEPRINT_HDF5_OUT = 6;
const int BLUEPRINT_YAML_OUT = 7;

// an output type is valid if it is an int in [0,NUM_OUTPUT_TYPES)
inline bool outputTypeValid(int otype)
{
    return otype >= 0 && otype < NUM_OUTPUT_TYPES;
}

inline bool outputTypeIsJpegPngOrTif(int otype)
{
    return otype == JPEG_OUT || otype == PNG_OUT || otype == TIF_OUT;
}

inline bool outputTypeIsRawfloatsOrBov(int otype)
{
    return otype == RAWFLOATS_OUT || otype == BOV_OUT;
}

inline bool outputTypeIsBlueprint(int otype)
{
    return otype == BLUEPRINT_HDF5_OUT || otype == BLUEPRINT_JSON_OUT || 
        otype == BLUEPRINT_YAML_OUT;
}

inline bool multipleOutputFiles(int otype, int numBins)
{
    return (outputTypeIsJpegPngOrTif(otype) && numBins > 1) || 
        outputTypeIsRawfloatsOrBov(otype);
}

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
//    Justin Privitera, Tue Jun 14 11:30:54 PDT 2022
//    Changed default output type to use constant instead of magic number and
//    added default outdir value.
// 
//    Justin Privitera, Tue Sep 27 10:52:59 PDT 2022
//    Replaced familyfiles with filenamescheme.
//
//    Justin Privitera, Tue Nov 22 14:56:04 PST 2022
//    Set default values for energy group bin variables.
// 
//    Justin Privitera, Mon Nov 28 15:38:25 PST 2022
//    Renamed energy group bins to energy group bounds.
// 
//    Justin Privitera, Wed Nov 30 17:43:48 PST 2022
//    Added default values for units.
// 
//    Justin Privitera, Fri Apr  7 16:39:30 PDT 2023
//    outputRayBounds is now included in the constructor.
// 
//    Justin Privitera, Fri Jun 16 17:17:14 PDT 2023
//    Added view width override and non square pixels.
// 
//    Justin Privitera, Fri Jul 14 17:33:07 PDT 2023
//    Added new view params flag. Turned it on by default and turned old view 
//    params flag off by default.
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
    energyGroupBounds = NULL;
    nEnergyGroupBounds = 0;
    debugRay = -1;
    outputRayBounds = false;
    filenameScheme = NONE;
    outputType = PNG_OUT;
    outputDir = ".";
    useSpecifiedUpVector = true;
    useNewView = true;
    useOldView = false;

    spatialUnits = "no units provided";
    energyUnits = "no units provided";
    absUnits = "no units provided";
    emisUnits = "no units provided";
    intensityUnits = "no units provided";
    pathLengthUnits = "no info provided";

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
    viewWidthOverride = 0.0;
    nonSquarePixels = false;
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
//    Justin Privitera, Tue Nov 22 14:56:04 PST 2022
//    Make sure the energy group bins are deleted.
// 
//    Justin Privitera, Mon Nov 28 15:38:25 PST 2022
//    Renamed energy group bins to energy group bounds.
//
// ****************************************************************************

avtXRayImageQuery::~avtXRayImageQuery()
{
    if (backgroundIntensities != NULL)
        delete [] backgroundIntensities;
    if (energyGroupBounds != NULL)
        delete [] energyGroupBounds;
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
//    Eric Brugger, Thu Jun  4 16:11:47 PDT 2015
//    I added an option to enable outputting the ray bounds to a vtk file.
// 
//    Justin Privitera, Tue Jun 14 11:30:54 PDT 2022
//    Handled sending the output directory through.
// 
//    Justin Privitera, Tue Sep 27 10:52:59 PDT 2022
//    Added filename scheme option. Only call family files option if
//    filename scheme is not present.
//
//    Justin Privitera, Tue Nov 22 14:56:04 PST 2022
//    Logic for energy group bins.
// 
//    Justin Privitera, Mon Nov 28 15:38:25 PST 2022
//    Renamed energy group bins to energy group bounds.
// 
//    Justin Privitera, Wed Nov 30 17:43:48 PST 2022
//    Added logic to handle passing through the units.
// 
//    Justin Privitera, Mon Dec 12 13:28:55 PST 2022
//    Changed path_length_units to path_length_info.
// 
//    Justin Privitera, Fri Jun 16 17:17:14 PDT 2023
//    Added view width override and non square pixels.
// 
//    Justin Privitera, Fri Jul 14 17:33:07 PDT 2023
//    Added logic for triggering the new camera properties.
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

    if (params.HasNumericVectorEntry("energy_group_bounds"))
    {
        doubleVector v;
        params.GetEntry("energy_group_bounds")->ToDoubleVector(v);
        SetEnergyGroupBounds(v);
    }

    // Are you ever going to have just one energy group bound? No.
    // But this is here for helpful error messaging. It is possible
    // to pass just one number in under energy_group_bounds, so with
    // this logic here VisIt will give users sensible error messages
    // embedded within the blueprint metadata.
    if (params.HasNumericEntry("energy_group_bounds"))
    {
        doubleVector v;
        v.push_back(params.GetEntry("energy_group_bounds")->ToDouble());
        SetEnergyGroupBounds(v);
    }

    std::map<std::string, std::string> unitsmap;
    if (params.HasEntry("spatial_units"))
        unitsmap["spatialUnits"] = params.GetEntry("spatial_units")->AsString();
    if (params.HasEntry("energy_units"))
        unitsmap["energyUnits"] = params.GetEntry("energy_units")->AsString();
    if (params.HasEntry("abs_units"))
        unitsmap["absUnits"] = params.GetEntry("abs_units")->AsString();
    if (params.HasEntry("emis_units"))
        unitsmap["emisUnits"] = params.GetEntry("emis_units")->AsString();
    if (params.HasEntry("intensity_units"))
        unitsmap["intensityUnits"] = params.GetEntry("intensity_units")->AsString();
    if (params.HasEntry("path_length_info"))
        unitsmap["pathLengthUnits"] = params.GetEntry("path_length_info")->AsString();
    SetUnits(unitsmap);

    if (params.HasNumericEntry("debug_ray"))
        SetDebugRay(params.GetEntry("debug_ray")->AsInt());

    if (params.HasNumericEntry("output_ray_bounds"))
        SetOutputRayBounds(params.GetEntry("output_ray_bounds")->ToBool());

    if (params.HasEntry("filename_scheme"))
    {
        if (params.GetEntry("filename_scheme")->TypeName() == "int")
            SetFilenameScheme(params.GetEntry("filename_scheme")->AsInt());
        else if (params.GetEntry("filename_scheme")->TypeName() == "string")
            SetFilenameScheme(params.GetEntry("filename_scheme")->AsString());
    }
    else
    {
        if (params.HasNumericEntry("family_files"))
            SetFamilyFiles(params.GetEntry("family_files")->ToBool());
    }

    if (params.HasEntry("output_type"))
    {
        if (params.GetEntry("output_type")->TypeName() == "int")
            SetOutputType(params.GetEntry("output_type")->AsInt());
        else if (params.GetEntry("output_type")->TypeName() == "string")
            SetOutputType(params.GetEntry("output_type")->AsString());
    }

    if (params.HasEntry("output_dir"))
        SetOutputDir(params.GetEntry("output_dir")->AsString());

    // this is not a normal parameter, it is set by the cli when the query
    // is called with the deprecated argument parsing.
    if (params.HasNumericEntry("useUpVector"))
    {
        useSpecifiedUpVector = params.GetEntry("useUpVector")->ToBool();
    }

    useNewView = false;
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
        useNewView = true;
    }

    if (params.HasNumericVectorEntry("focus"))
    {
        doubleVector v;
        params.GetEntry("focus")->ToDoubleVector(v);
        if (v.size() != 3)
            EXCEPTION2(QueryArgumentException, "focus", 3);
        focus[0] = v[0]; focus[1] = v[1]; focus[2] = v[2];
        useNewView = true;
    }

    if (params.HasNumericVectorEntry("view_up"))
    {
        doubleVector v;
        params.GetEntry("view_up")->ToDoubleVector(v);
        if (v.size() != 3)
            EXCEPTION2(QueryArgumentException, "view_up", 3);
        viewUp[0] = v[0]; viewUp[1] = v[1]; viewUp[2] = v[2];
        useNewView = true;
    }

    if (params.HasNumericEntry("view_angle"))
    {
        viewAngle = params.GetEntry("view_angle")->ToDouble();
        useNewView = true;
    }

    if (params.HasNumericEntry("parallel_scale"))
    {
        parallelScale = params.GetEntry("parallel_scale")->ToDouble();
        useNewView = true;
    }

    if (params.HasNumericEntry("view_width"))
    {
        viewWidthOverride = params.GetEntry("view_width")->ToDouble();
        nonSquarePixels = true;
        useNewView = true;
    }

    if (params.HasNumericEntry("near_plane"))
    {
        nearPlane = params.GetEntry("near_plane")->ToDouble();
        useNewView = true;
    }

    if (params.HasNumericEntry("far_plane"))
    {
        farPlane = params.GetEntry("far_plane")->ToDouble();
        useNewView = true;
    }

    if (params.HasNumericVectorEntry("image_pan"))
    {
        doubleVector v;
        params.GetEntry("image_pan")->ToDoubleVector(v);
        if (v.size() != 2)
            EXCEPTION2(QueryArgumentException, "image_pan", 2);
        imagePan[0] = v[0]; imagePan[1] = v[1];
        useNewView = true;
    }

    if (params.HasNumericEntry("image_zoom"))
    {
        imageZoom = params.GetEntry("image_zoom")->ToDouble();
        useNewView = true;
    }

    if (params.HasNumericEntry("perspective"))
    {
        perspective = params.GetEntry("perspective")->ToBool();
        useNewView = true;
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
//  Method: avtXRayImageQuery::SetEnergyGroupBounds
//
//  Purpose:
//    Set the energy group bins.
//
//  Programmer: Justin Privitera
//  Creation:   November 18, 2022
// 
//  Modifications:
//    Justin Privitera, Mon Nov 28 15:38:25 PST 2022
//    Renamed energy group bins to energy group bounds. Changed the function 
//    name.
//
// ****************************************************************************

void
avtXRayImageQuery::SetEnergyGroupBounds(const doubleVector &bins)
{
    if (energyGroupBounds != NULL)
        delete [] energyGroupBounds;

    energyGroupBounds = new double[bins.size()];
    for (int i = 0; i < bins.size(); i++)
        energyGroupBounds[i] = bins[i];
    nEnergyGroupBounds = bins.size();
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetUnits
//
//  Purpose:
//    Set all the unit variables. 
// 
//  Note:
//    Doing them all in one function reduces code bloat with lots of setters, 
//    and they'd all be very similar anyway.
//
//  Programmer: Justin Privitera
//  Creation:   November 30, 2022
//
// ****************************************************************************

void
avtXRayImageQuery::SetUnits(const std::map<std::string, std::string> &unitsmap)
{
    if (! unitsmap.empty())
    {
        if (unitsmap.count("spatialUnits") > 0)
            spatialUnits = unitsmap.at("spatialUnits");
        if (unitsmap.count("energyUnits") > 0)
            energyUnits = unitsmap.at("energyUnits");
        if (unitsmap.count("absUnits") > 0)
            absUnits = unitsmap.at("absUnits");
        if (unitsmap.count("emisUnits") > 0)
            emisUnits = unitsmap.at("emisUnits");
        if (unitsmap.count("intensityUnits") > 0)
            intensityUnits = unitsmap.at("intensityUnits");
        if (unitsmap.count("pathLengthUnits") > 0)
            pathLengthUnits = unitsmap.at("pathLengthUnits");
    }
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
//  Method: avtXRayImageQuery::SetOutputRayBounds
//
//  Purpose:
//    Set the output ray bounds flag.
//
//  Programmer: Eric Brugger
//  Creation:   June 4, 2015
//
// ****************************************************************************

void
avtXRayImageQuery::SetOutputRayBounds(const bool &flag)
{
    outputRayBounds = flag;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetFamilyFiles
//
//  Purpose:
//    Set the filename scheme.
//
//  Programmer: Eric Brugger
//  Creation:   May 27, 2015
//
//  Modifications:
//     Justin Privitera, Thu Sep 22 16:46:46 PDT 2022
//     Instead of setting the familyfiles flag, now it sets the filename scheme
//     appropriately.
// ****************************************************************************

void
avtXRayImageQuery::SetFamilyFiles(const bool &flag)
{
    filenameScheme = flag ? FAMILYFILES : NONE;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetFilenameScheme
//
//  Purpose:
//    Set the filename scheme.
//
//  Programmer: Justin Privitera
//  Creation:   Tue Sep 27 10:52:59 PDT 2022
// 
//  Modifications:
// 
// ****************************************************************************

void
avtXRayImageQuery::SetFilenameScheme(int type)
{
    if (filenameSchemeValid(filenameScheme))
        filenameScheme = type;
    else
    {
        std::ostringstream err_oss;
        err_oss << "Filename scheme " << type << " is invalid.\n";
        EXCEPTION1(VisItException, err_oss.str());
    }
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetFilenameScheme
//
//  Purpose:
//    Set the filename scheme.
//
//  Programmer: Justin Privitera
//  Creation:   Tue Sep 27 10:52:59 PDT 2022
//
//  Modifications:
// 
// ****************************************************************************

void
avtXRayImageQuery::SetFilenameScheme(const std::string &type)
{
    int i = 0;
    while (i < NUM_FILENAME_SCHEMES)
    {
        // the output type indexes the file extensions array
        if (type == filename_schemes[i])
        {
            filenameScheme = i;
            return;
        }
        i ++;
    }
    std::ostringstream err_oss;
    err_oss << "Filename scheme " << type << " is invalid.\n";
    EXCEPTION1(VisItException, err_oss.str());
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
//  Modifications:
//    Justin Privitera, Tue Jun 14 11:30:54 PDT 2022
//    Validity check for output type and error message if it is invalid.
// 
//    Justin Privitera, Wed Jul 20 13:54:06 PDT 2022
//    Use ostringstreams for error messages.
// 
// ****************************************************************************

void
avtXRayImageQuery::SetOutputType(int type)
{
    if (outputTypeValid(outputType))
        outputType = type;
    else
    {
        std::ostringstream err_oss;
        err_oss << "Output type " << type << " is invalid.\n";
        EXCEPTION1(VisItException, err_oss.str());
    }
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
//    Justin Privitera, Tue Jun 14 11:30:54 PDT 2022
//    Output type upgrade across the entire file. Magic numbers for output
//    types are gone, replaced with new constants. Output type indexes the 
//    new file protocols array.
// 
//    Justin Privitera, Wed Jul 20 13:54:06 PDT 2022
//    Use ostringstream for error messages.
// 
// ****************************************************************************

void
avtXRayImageQuery::SetOutputType(const std::string &type)
{
    int i = 0;
    while (i < NUM_OUTPUT_TYPES)
    {
        // the output type indexes the file extensions array
        if (type == file_protocols[i])
        {
            outputType = i;
            return;
        }
        i ++;
    }
    std::ostringstream err_oss;
    err_oss << "Output type " << type << " is invalid.\n";
    EXCEPTION1(VisItException, err_oss.str());
}

// ****************************************************************************
//  Method: avtXRayImageQuery::SetOutputDir
//
//  Purpose:
//    Set the output directory.
//
//  Programmer: Justin Privitera 
//  Creation:   Wed Jun  1 12:30:46 PDT 2022
//
//  Modifications:
// 
// ****************************************************************************

void
avtXRayImageQuery::SetOutputDir(const std::string &dir)
{
    outputDir = dir;
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
//    Eric Brugger, Thu Jun  4 16:11:47 PDT 2015
//    I added an option to enable outputting the ray bounds to a vtk file.
//
//    Kevin Griffin, Tue Sep 27 16:52:14 PDT 2016
//    Ensured that all ranks throw an Exception if any rank throws one
//    for incorrectly centered data and/or missing variables.
//
//    Eric Brugger, Wed Aug  2 09:40:02 PDT 2017
//    I corrected a bug where the type specified in the bov file was
//    incorrectly set to float for the intensities in the case of double
//    and integer output.
// 
//    Justin Privitera, Tue Jun 14 11:30:54 PDT 2022
//     - Output type upgrade across the entire file. Magic numbers for output
//    types are gone, replaced with new constants. There are now functions
//    used to test output type validity and if output type falls into a 
//    specific category. There are also arrays that the output type indexes
//    into that contain file protocols and file extensions.
//     - Added output directory to query.
//     - Conduit checks if the output directory is valid.
//     - baseName and fileName have become stringstreams. New strings have 
//    been made to store results from baseName formatting, and are used 
//    extensively.
//    Blueprint output has been added, with 4 output types.
//    The output messages has been reorganized and refactored.
// 
//    Justin Privitera, Wed Jul 20 13:54:06 PDT 2022
//    Use stringstreams for output messages, use ostringstreams for error
//    messages, and set result messages for error cases.
// 
//    Justin Privitera, Thu Sep  8 16:29:06 PDT 2022
//    Added spatial extents meta data to blueprint outputs.
// 
//    Justin Privitera, Tue Sep 27 10:52:59 PDT 2022
//     - Added check to see if the filename scheme is one of the valid options.
//     - Reworked the file naming system.
//     - Moved calculation of numbins to earlier so that info can be used when
//    deciding on the file naming scheme.
//     - Determined whether or not bmp, jpeg, png, and tif outputs should 
//    write bin info in the filename or exclude it.
//     - Combined rawfloats and bov logic into one block with some 
//    conditionals.
//     - Removed all the filenaming logic that was specific to blueprint.
//     - Passed options node to conduit save_mesh call.
//     - Removed unused code.
//     - Updated output messages to reflect new filenaming schemes.
//     - Moved ifdef conduit guards to reflect desired behavior.
//     - Cleaned up result message handling.
// 
//    Justin Privitera, Thu Sep 29 17:35:07 PDT 2022
//    Added warning message for bmp output in result message and to debug1.
// 
//    Justin Privitera, Wed Oct 12 11:38:11 PDT 2022
//    Removed bmp output type.
// 
//    Justin Privitera, Tue Nov 15 11:44:01 PST 2022
//    Various changes to the blueprint output:
//     - Reorganized metadata into categories
//     - Added new metadata outputs: query parameters and extra data
//     - Added imaging plane topologies
// 
//    Justin Privitera, Tue Nov 22 14:56:04 PST 2022
//    Added logic to output energy group bounds in blueprint output if they are
//    provided; include an info message if not.
// 
//    Justin Privitera, Mon Nov 28 15:38:25 PST 2022
//    Renamed energy group bins to energy group bounds.
// 
//    Justin Privitera, Wed Nov 30 10:41:17 PST 2022
//    Absolute value is applied to detector height and width to ensure
//    sensible values come out of the query.
// 
//    Justin Privitera, Wed Nov 30 17:43:48 PST 2022
//    The units are propagated to the output metadata for blueprint output 
//    types.
// 
//    Justin Privitera, Thu Dec  1 15:29:48 PST 2022
//    Changed where units go in the blueprint output.
// 
//    Justin Privitera, Wed Dec  7 16:16:16 PST 2022
//     - Calculated far plane projection width and height.
//     - Made spatial extents a proper coordset living in the blueprint 
//    metadata.
//     - Pass corner coord containers to my imaging plane calculation methods
//    so those values can be used to calculate the rays.
//     - Added ray corners mesh and rays mesh.
// 
//    Justin Privitera, Mon Dec 12 13:28:55 PST 2022
//    Major refactor of blueprint output logic. Most calculations are pushed
//    down into helpers.
//
//    Justin Privitera, Wed Mar 15 17:51:13 PDT 2023
//    Have blueprint verify failures go to debug1 instead of to console.
//    Consistent error messaging + cleaner error handling.
// 
//    Justin Privitera, Wed Mar 29 13:19:53 PDT 2023
//    Leverage avtXRayFilter::CalculateImagingPlaneDims so that calculations
//    are not duplicated here as well.
//    Send "state" node to metadata write call for conduit blueprint output.
// 
//    Justin Privitera, Fri Jun 16 17:17:14 PDT 2023
//    Added view width override and non square pixels to call to 
//    CalculateImagingPlaneDims().
// 
//    Justin Privitera, Fri Jul 14 17:33:07 PDT 2023
//    New logic to determine if old camera properties are being used.
// 
//    Justin Privitera, Mon Aug  7 15:49:36 PDT 2023
//    Add more context to debug message for blueprint failing to verify.
// 
// ****************************************************************************

void
avtXRayImageQuery::Execute(avtDataTree_p tree)
{
    std::cout << "avtXRayImageQuery::Execute" << std::endl;
    // check validity of output type before proceeding
    if (!outputTypeValid(outputType))
    {
        std::ostringstream err_oss;
        err_oss << "ERROR: Output type " << outputType << " is invalid.\n";
        SetResultMessage(err_oss.str());
        EXCEPTION1(VisItException, err_oss.str());
    }
    // check validity of output type before proceeding
    if (!filenameSchemeValid(filenameScheme))
    {
        std::ostringstream err_oss;
        err_oss << "Filename type " << filenameScheme << " is invalid.\n";
        SetResultMessage(err_oss.str());
        EXCEPTION1(VisItException, err_oss.str());
    }
    // It would be nice to have something that could check the validity of the 
    // output directory without needing conduit.
#ifdef HAVE_CONDUIT
    // check if output directory exists before proceeding
    if (!conduit::utils::is_directory(outputDir))
    {
        std::ostringstream err_oss;
        err_oss << "ERROR: Directory " << outputDir << " does not exist.\n";
        SetResultMessage(err_oss.str());
        EXCEPTION1(VisItException, err_oss.str());
    }
#endif

    avtDataset_p input = GetTypedInput();
    
    int nsets = 0;
    vtkDataSet **dataSets = tree->GetAllLeaves(nsets);
    
    CheckData(dataSets, nsets);

    //
    // If the number of pixels is less than or equal to zero then print
    // an error message and exit.
    //
    if (numPixels <= 0)
    {
        SetResultMessage("ERROR: VisIt is unable to execute this query because "
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
            SetResultMessage("ERROR: VisIt is unable to execute this query "
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

    if (useOldView && !useNewView)
        ConvertOldImagePropertiesToNew();
    filt->SetImageProperties(normal, focus, viewUp, viewAngle, parallelScale,
        viewWidthOverride, nonSquarePixels, nearPlane, farPlane, imagePan, 
        imageZoom, perspective, imageSize);

    filt->SetDivideEmisByAbsorb(divideEmisByAbsorb);
    filt->SetBackgroundIntensity(backgroundIntensity);
    filt->SetBackgroundIntensities(backgroundIntensities,
        nBackgroundIntensities);
    filt->SetDebugRay(debugRay);
    filt->SetOutputRayBounds(outputRayBounds);
    filt->SetVariableNames(absVarName, emisVarName);
    filt->SetInput(dob);

    //
    // Cause our artificial pipeline to execute.
    //
    avtContract_p contract = input->GetOriginatingSource()->GetGeneralContract();
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

            SetResultMessage("ERROR: There must be at least one bin.");
            EXCEPTION1(VisItException, "There must be at least one bin.");
        }

        int numBins = numLeaves / 2;

        //
        // Create the file base name.
        //
        std::stringstream baseName;
        const int cycle = GetInput()->GetInfo().GetAttributes().GetCycle();
        if (filenameScheme == FAMILYFILES)
        {
            bool keepTrying{true};
            while (keepTrying)
            {
                keepTrying = false;
                //
                // Create the file base name and increment the family number.
                //
                baseName.clear();
                baseName.str(std::string());
                baseName << "output" << "." << std::setfill('0') << std::setw(4) << iFileFamily;
                if (iFileFamily < NUMFAMILYFILES) iFileFamily ++;

                //
                // Check if the first file created with the file base name
                // exists. If it does and we aren't at the maximum, try
                // the next file base name in the sequence.
                //
                std::stringstream fileName;
                if (outputDir != ".")
                    fileName << outputDir.c_str() << "/" << baseName.str();
                if (multipleOutputFiles(outputType, numBins))
                    fileName << ".00." << file_extensions[outputType];
                else
                    fileName << "." << file_extensions[outputType];

                ifstream ifile(fileName.str());
                if (!ifile.fail() && iFileFamily < NUMFAMILYFILES)
                    keepTrying = true;
            }
        }
        else if (filenameScheme == CYCLEFILES)
            baseName << "output.cycle_" << std::setfill('0') << std::setw(6) << cycle;
        else
            baseName << "output";

        // does NOT contain the file extension
        std::string out_filename_w_path{(outputDir == "." ? "" : outputDir + "/") + baseName.str()};

        //
        // Write out the intensity and path length. The path length is only
        // put out when the output format is bof or bov.
        //
        vtkDataArray *intensity;
        vtkDataArray *pathLength;
#ifdef HAVE_CONDUIT
        conduit::Node data_out;
#endif
        if (outputTypeIsJpegPngOrTif(outputType))
        {
            const bool write_bin_info_to_filename{numBins > 1};
            for (int i = 0; i < numBins; i++)
            {
                intensity = leaves[i]->GetPointData()->GetArray("Intensity");
                if (intensity->GetDataType() == VTK_FLOAT)
                    WriteImage(out_filename_w_path.c_str(), i, numPixels,
                        (float*) intensity->GetVoidPointer(0), 
                        write_bin_info_to_filename);
                else if (intensity->GetDataType() == VTK_DOUBLE)
                    WriteImage(out_filename_w_path.c_str(), i, numPixels,
                        (double*) intensity->GetVoidPointer(0), 
                        write_bin_info_to_filename);
                else if (intensity->GetDataType() == VTK_INT)
                    WriteImage(out_filename_w_path.c_str(), i, numPixels,
                        (int*) intensity->GetVoidPointer(0), 
                        write_bin_info_to_filename);
            }
        }
        else if (outputTypeIsRawfloatsOrBov(outputType))
        {
            const bool bovOut{outputType == BOV_OUT};
            for (int i = 0; i < numBins; i++)
            {
                intensity = leaves[i]->GetPointData()->GetArray("Intensity");
                pathLength = leaves[numBins+i]->GetPointData()->GetArray("PathLength");
                if (intensity->GetDataType() == VTK_FLOAT)
                {
                    WriteFloats(out_filename_w_path.c_str(), i, numPixels,
                        (float*)intensity->GetVoidPointer(0));
                    if (bovOut)
                        WriteBOVHeader(out_filename_w_path.c_str(), "intensity", i, nx, ny, "FLOAT");
                    WriteFloats(out_filename_w_path.c_str(), numBins+i, numPixels,
                        (float*)pathLength->GetVoidPointer(0));
                    if (bovOut)
                        WriteBOVHeader(out_filename_w_path.c_str(), "path_length", numBins+i,
                            nx, ny, "FLOAT");
                }
                else if (intensity->GetDataType() == VTK_DOUBLE)
                {
                    WriteFloats(out_filename_w_path.c_str(), i, numPixels,
                        (double*)intensity->GetVoidPointer(0));
                    if (bovOut)
                        WriteBOVHeader(out_filename_w_path.c_str(), "intensity", i, nx, ny, "DOUBLE");
                    WriteFloats(out_filename_w_path.c_str(), numBins+i, numPixels,
                        (double*)pathLength->GetVoidPointer(0));
                    if (bovOut)
                        WriteBOVHeader(out_filename_w_path.c_str(), "path_length", numBins+i,
                            nx, ny, "DOUBLE");
                }
                else if (intensity->GetDataType() == VTK_INT)
                {
                    WriteFloats(out_filename_w_path.c_str(), i, numPixels,
                        (int*)intensity->GetVoidPointer(0));
                    if (bovOut)
                        WriteBOVHeader(out_filename_w_path.c_str(), "intensity", i, nx, ny, "INT");
                    WriteFloats(out_filename_w_path.c_str(), numBins+i, numPixels,
                        (int*)pathLength->GetVoidPointer(0));
                    if (bovOut)
                        WriteBOVHeader(out_filename_w_path.c_str(), "path_length", numBins+i,
                            nx, ny, "INT");
                }
            }
        }
        else if (outputTypeIsBlueprint(outputType))
        {
#ifdef HAVE_CONDUIT
            // calculate constants for use in multiple functions
            double nearHeight, nearWidth, viewHeight, viewWidth, farHeight, farWidth;
            avtXRayFilter::CalculateImagingPlaneDims(parallelScale, viewWidthOverride,
                                                     nonSquarePixels, imageSize,
                                                     perspective, viewAngle,
                                                     nearPlane, farPlane, imageZoom,
                                                     nearHeight, nearWidth,
                                                     viewHeight, viewWidth,
                                                     farHeight, farWidth);

            const double detectorWidth{2. * nearWidth}; // near
            const double detectorHeight{2. * nearHeight}; // near
            const double farDetectorWidth{2. * farWidth};
            const double farDetectorHeight{2. * farHeight};

            // The following variables will be assigned values
            // when WriteBlueprintMeshes() is called.
            int numfieldvals;
            conduit::float64 *intensity_vals;
            conduit::float64 *depth_vals;

            // this includes the image mesh and the spatial mesh
            WriteBlueprintMeshes(data_out, detectorWidth, detectorHeight, 
                numBins, leaves, numfieldvals, intensity_vals, depth_vals);

            // all the metadata living under "state" is written in this function
            WriteBlueprintMetadata(data_out["state"], cycle, numBins, 
                detectorWidth, detectorHeight, 
                numfieldvals, intensity_vals, depth_vals);

            // includes imaging planes, ray corners, and rays
            WriteBlueprintImagingMeshes(data_out,
                nearWidth, nearHeight, viewWidth, viewHeight, farWidth, farHeight,
                detectorWidth, detectorHeight, farDetectorWidth, farDetectorHeight);

            // verify
            conduit::Node verify_info;
            if(!conduit::blueprint::mesh::verify(data_out, verify_info))
            {
                debug1 << "X Ray Image Query ERROR: Blueprint Output: failed to verify:\n"
                       << verify_info.to_yaml();
                SetResultMessage("ERROR: Blueprint mesh verification failed!");
                EXCEPTION1(VisItException, "Blueprint mesh verification failed!");
            }

            try
            {
                conduit::Node opts;
                opts["suffix"] = "none";
                // save out
                conduit::relay::io::blueprint::save_mesh(data_out,
                                                         out_filename_w_path.c_str(),
                                                         file_protocols[outputType],
                                                         opts);
            }
            catch (conduit::Error &e)
            {
                std::ostringstream err_oss;
                err_oss <<  "ERROR: Conduit Exception in X Ray Image Query "
                        << "Execute: " << endl
                        << e.message();
                SetResultMessage(err_oss.str());
                EXCEPTION1(VisItException, err_oss.str());
            }
#else
            // this is safe because at the beginning of the function we check that the output type is valid
            std::ostringstream err_oss;
            err_oss << "ERROR: Visit was not installed with conduit, "
                        << "which is needed for output type "
                        << file_protocols[outputType]
                        << "." << std::endl;
            SetResultMessage(err_oss.str());
            EXCEPTION1(VisItException, err_oss.str());
#endif
        }
        else
        {
            // this is safe because at the beginning of the function we check that the output type is valid
            std::ostringstream err_oss;
            err_oss << "ERROR: No logic implemented for output type "
                        << file_protocols[outputType]
                        << "." << std::endl;
            SetResultMessage(err_oss.str());
            EXCEPTION1(VisItException, err_oss.str());
        }


        //
        // Output the result message.
        //

        // the output type is valid; we verified at the beginning of the function
        std::stringstream buf;

        if (outputTypeIsJpegPngOrTif(outputType))
        {
            if (numBins == 1)
                buf << "The x ray image query results were "
                    << "written to the file "
                    << out_filename_w_path << "."
                    << file_extensions[outputType] << "\n";
            else
                buf << "The x ray image query results were "
                    << "written to the files "
                    << out_filename_w_path << ".00."
                    << file_extensions[outputType]
                    << " - " << out_filename_w_path << "."
                    << std::setfill('0') << std::setw(2)
                    << numBins - 1 << "."
                    << file_extensions[outputType] << "\n";
        }
        else if (outputTypeIsRawfloatsOrBov(outputType))
            buf << "The x ray image query results were "
                << "written to the files "
                << out_filename_w_path << ".00."
                << file_extensions[outputType]
                << " - " << out_filename_w_path << "."
                << std::setfill('0') << std::setw(2)
                << 2 * numBins - 1 << "."
                << file_extensions[outputType] << "\n";
#ifdef HAVE_CONDUIT
        else if (outputTypeIsBlueprint(outputType))
        {
            buf << "The x ray image query results were "
                << "written to the file "
                << out_filename_w_path << "."
                << file_extensions[outputType] << "\n";
        }
#endif
        else
        {
            // this is safe because at the beginning of the function we check that the output type is valid
            std::ostringstream err_oss;
            err_oss << "ERROR: No output message implemented for output type "
                        << file_protocols[outputType] << "." << std::endl;
            SetResultMessage(err_oss.str());
            EXCEPTION1(VisItException, err_oss.str());
        }
        SetResultMessage(buf.str());

        // Free the memory from the GetAllLeaves function call.
        delete [] leaves;
    }
    visitTimer->StopTimer(t2, "avtXRayImageQuery::WriteImage");

    visitTimer->StopTimer(t1, "avtXRayImageQuery::ExecutePipeline");

    delete filt;
}

// ****************************************************************************
//  Method: avtXRayImageQuery::CheckData
//
//  Purpose:    Perform error checks on the input datasets.
//
//  Arguments:
//    dataSets: The input datasets
//    nsets:    The number of datasets
//
//  Programmer: Kevin Griffin
//  Creation:   Septemeber 19, 2016
//
//  Modifications:
//    Kevin Griffin, Tue Sep 27 16:52:14 PDT 2016
//    Ensured that all nodes throw an exception when at least one node does.
// 
//    Justin Privitera, Wed Jul 20 13:54:06 PDT 2022
//    Use ostringstreams for error messages.
// 
//    Justin Privitera, Thu Mar 16 14:47:46 PDT 2023
//    Consistent error messaging.
//
// ****************************************************************************
void
avtXRayImageQuery::CheckData(vtkDataSet **dataSets,  const int nsets)
{
    int foundError = 0;     // 0=false, 1=true
    bool isArgException = false;
    bool isAbs = false;
    std::ostringstream msg;
    
    for (int i = 0; i < nsets; i++)
    {
        vtkDataArray *abs  = dataSets[i]->GetCellData()->GetArray(absVarName.c_str());
        vtkDataArray *emis = dataSets[i]->GetCellData()->GetArray(emisVarName.c_str());
        
        if (abs == NULL)
        {
            if (dataSets[i]->GetPointData()->GetArray(absVarName.c_str()) != NULL)
            {
                msg << "Variable " << absVarName << " is node-centered, but "
                    << "it must be zone-centered for this query.\n";
                foundError = 1;
                isAbs = true;
                break;
            }
            else
            {
                foundError = 1;
                isArgException = true;
                isAbs = true;
                break;
            }
        }
        if (emis == NULL)
        {
            if (dataSets[i]->GetPointData()->GetArray(emisVarName.c_str())
                != NULL)
            {
                msg << "Variable " << emisVarName << " is node-centered, but "
                    << "it must be zone-centered for this query.\n";
                foundError = 1;
                break;
            }
            else
            {
                foundError = 1;
                isArgException = true;
                break;
            }
            
        }
    }
    
    // Check if an exception has been raised on any rank
    int maxError = UnifyMaximumValue(foundError);
    if(maxError > 0)
    {
        if(foundError == 1)
        {
            if(isArgException)
            {
                SetResultMessage("ERROR: " + (isAbs ? absVarName : emisVarName));
                EXCEPTION1(QueryArgumentException, isAbs ? absVarName.c_str() : emisVarName.c_str());
            }
            else
            {
                SetResultMessage("ERROR: " + msg.str());
                EXCEPTION1(ImproperUseException, msg.str());
            }
        }
        else
        {
            SetResultMessage("ERROR: Exception encountered on another node");
            EXCEPTION1(VisItException, "Exception encountered on another node");
        }
    }
    
    // Check if no data is available on all ranks
    int maxNsets = UnifyMaximumValue(nsets);
    if(maxNsets <= 0)
    {
        msg << "ERROR: Variables " << absVarName << " and " << emisVarName 
            << " resulted in no data being selected.\n";
        SetResultMessage(msg.str());
        EXCEPTION1(VisItException, msg.str());
    }
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
//    Justin Privitera, Tue Jun 14 11:30:54 PDT 2022
//    Changed magic numbers to their new constants.
// 
//    Justin Privitera, Wed Jul 20 13:54:06 PDT 2022
//    Use stringstreams. Fixes issue with long baseName being written
//    to fixed length buffers.
// 
//    Justin Privitera, Tue Sep 27 10:52:59 PDT 2022
//    Added new arg to control if bin info is written to filenames. It is only
//    written if necessary.
// 
//    Justin Privitera, Wed Oct 12 11:38:11 PDT 2022
//    Removed bmp output type.
//
// ****************************************************************************

template <typename T>
void
avtXRayImageQuery::WriteImage(const char *baseName, int iImage, int nPixels,
    T *fbuf, bool write_bin_info_to_filename)
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

    std::stringstream fileName;
    if (write_bin_info_to_filename)
        fileName << baseName << "." << std::setfill('0') << std::setw(2) << iImage;
    else
        fileName << baseName;

    if (outputType == JPEG_OUT)
    {
        vtkImageWriter *writer = vtkJPEGWriter::New();
        fileName << ".jpg";
        writer->SetFileName(fileName.str().c_str());
        writer->SetInputData(image);
        writer->Write();
        writer->Delete();
    }
    else if (outputType == PNG_OUT)
    {
        vtkImageWriter *writer = vtkPNGWriter::New();
        fileName << ".png";
        writer->SetFileName(fileName.str().c_str());
        writer->SetInputData(image);
        writer->Write();
        writer->Delete();
    }
    else if (outputType == TIF_OUT)
    {
        vtkImageWriter *writer = vtkTIFFWriter::New();
        fileName << ".tif";
        writer->SetFileName(fileName.str().c_str());
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
//    Justin Privitera, Wed Jul 20 13:54:06 PDT 2022
//    Use stringstreams.
// 
//    Justin Privitera, Tue Sep 27 10:52:59 PDT 2022
//    Extra dot added for filenames.
//
// ****************************************************************************

template <typename T>
void
avtXRayImageQuery::WriteFloats(const char *baseName, int iImage, int nPixels,
    T *fbuf)
{
    std::stringstream fileName;
    fileName << baseName << "." << std::setfill('0') << std::setw(2) << iImage << ".bof";
    FILE *file = fopen(fileName.str().c_str(), "w");
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
//    Justin Privitera, Wed Jul 20 13:54:06 PDT 2022
//    Use stringstreams.
// 
//    Justin Privitera, Tue Sep 27 10:52:59 PDT 2022
//    Extra dot added for filenames.
//
// ****************************************************************************

void
avtXRayImageQuery::WriteBOVHeader(const char *baseName, const char *varName,
    int iBin, int nx, int ny, const char *type)
{
    std::stringstream fileName;
    fileName << baseName << "." << std::setfill('0') << std::setw(2) << iBin << ".bov";
    FILE *file = fopen(fileName.str().c_str(), "w");
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
//  Method: avtXRayImageQuery::WriteArrays
//
//  Purpose:
//    Write image to specified conduit arrays.
//
//  Programmer: Justin Privitera
//  Creation:   Sat Jun 11 17:59:32 PDT 2022
//
//  Modifications:
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
template <typename T>
void
avtXRayImageQuery::WriteArrays(vtkDataSet **leaves, 
                               conduit::float64 *intensity_vals,
                               conduit::float64 *depth_vals,
                               int numBins)
{
    vtkDataArray *intensity;
    vtkDataArray *pathLength;
    int field_index = 0;
    for (int i = 0; i < numBins; i ++)
    {
        intensity = leaves[i]->GetPointData()->GetArray("Intensity");
        pathLength = leaves[numBins + i]->GetPointData()->GetArray("PathLength");
        T *intensity_ptr = (T *) intensity->GetVoidPointer(0);
        T *path_length_ptr = (T *) pathLength->GetVoidPointer(0);
        for (int j = 0; j < numPixels; j ++)
        {
            intensity_vals[field_index] = intensity_ptr[j];
            depth_vals[field_index] = path_length_ptr[j];
            field_index ++;
        }
    }
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBlueprintImagingPlane
//
//  Purpose:
//    Calculates imaging plane coords and writes them to blueprint output.
//
//  Programmer: Justin Privitera
//  Creation:   November 14, 2022
// 
//  Modifications:
//    Justin Privitera, Wed Dec  7 16:16:16 PST 2022
//     - Added 5 new args that act as containers for various calculated vector 
//    values.
//     - Use the new Add3 inline function to reduce code lines. 
// 
//    Justin Privitera, Mon Dec 12 13:28:55 PST 2022
//     - Use avtVectors.
//     - Changed order and names of arguments.
//     - Some calculations were lifted out of the function.
//     - Calculated values are sent back up the call stack.
// 
//    Justin Privitera, Wed Mar 29 13:48:51 PDT 2023
//    Leverage conduit's features to make the code more legible.
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
avtXRayImageQuery::WriteBlueprintImagingPlane(conduit::Node &data_out,
                                              const std::string plane_name,
                                              const double planeWidth,
                                              const double planeHeight,
                                              const avtVector &center,
                                              const avtVector &left,
                                              avtVector &llc,
                                              avtVector &lrc,
                                              avtVector &ulc,
                                              avtVector &urc)
{
    // set up imaging plane coords
    conduit::Node &plane_coords = data_out["coordsets"][plane_name + "_coords"];
    plane_coords["type"] = "explicit";
    plane_coords["values/x"].set(conduit::DataType::float64(4));
    plane_coords["values/y"].set(conduit::DataType::float64(4));
    plane_coords["values/z"].set(conduit::DataType::float64(4));
    double *xvals = plane_coords["values/x"].value();
    double *yvals = plane_coords["values/y"].value();
    double *zvals = plane_coords["values/z"].value();

    // set these values and send back up the callstack for use elsewhere
    llc = center + (-1. * planeHeight) * viewUp +     planeWidth     * left;
    lrc = center + (-1. * planeHeight) * viewUp + (-1. * planeWidth) * left;
    ulc = center +     planeHeight     * viewUp +     planeWidth     * left;
    urc = center +     planeHeight     * viewUp + (-1. * planeWidth) * left;
    
    // set x values    // set y values    // set z values
    xvals[0] = llc.x;  yvals[0] = llc.y;  zvals[0] = llc.z;
    xvals[1] = lrc.x;  yvals[1] = lrc.y;  zvals[1] = lrc.z;
    xvals[2] = urc.x;  yvals[2] = urc.y;  zvals[2] = urc.z;
    xvals[3] = ulc.x;  yvals[3] = ulc.y;  zvals[3] = ulc.z;

    // set up imaging plane topo
    conduit::Node &plane_topo = data_out["topologies"][plane_name + "_topo"];
    plane_topo["type"] = "unstructured";
    plane_topo["coordset"] = plane_name + "_coords";
    plane_topo["elements/shape"] = "quad";
    const int num_corners{4};
    plane_topo["elements/connectivity"].set(conduit::DataType::int32(num_corners));
    int *conn = plane_topo["elements/connectivity"].value();
    for (int i = 0; i < num_corners; i ++) { conn[i] = i; }

    // set up imaging plane trivial field
    conduit::Node &plane_field = data_out["fields"][plane_name + "_field"];
    plane_field["topology"] = plane_name + "_topo";
    plane_field["association"] = "element";
    plane_field["volume_dependent"] = "false";
    plane_field["values"].set(conduit::DataType::float64(1));
    conduit::float64 *field_vals = plane_field["values"].value();
    field_vals[0] = 0;
}
#endif
// ****************************************************************************
//  Function: WriteBlueprintRayCornersMesh
//
//  Purpose:
//    This function writes a mesh representing the ray corners used in the 
//    query to the blueprint output.
//
//  Programmer: Justin Privitera
//  Creation:   December 09, 2022
// 
//  Modifications:
//    Justin Privitera, Wed Mar 29 13:48:51 PDT 2023
//    Leverage conduit's features to make the code more legible.
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
WriteBlueprintRayCornersMesh(conduit::Node &data_out,
                             const avtVector &llc_near,
                             const avtVector &llc_far,
                             const avtVector &lrc_near,
                             const avtVector &lrc_far,
                             const avtVector &urc_near,
                             const avtVector &urc_far,
                             const avtVector &ulc_near,
                             const avtVector &ulc_far)
{
    const int num_corners{4};
    const int num_points{8};

    // set up ray coords
    conduit::Node &ray_corners_coords = data_out["coordsets"]["ray_corners_coords"];
    ray_corners_coords["type"] = "explicit";
    ray_corners_coords["values/x"].set(conduit::DataType::float64(num_points));
    ray_corners_coords["values/y"].set(conduit::DataType::float64(num_points));
    ray_corners_coords["values/z"].set(conduit::DataType::float64(num_points));
    double *xvals_ray = ray_corners_coords["values/x"].value();
    double *yvals_ray = ray_corners_coords["values/y"].value();
    double *zvals_ray = ray_corners_coords["values/z"].value();
                
    // set x values             // set y values             // set z values
    xvals_ray[0] = llc_near.x;  yvals_ray[0] = llc_near.y;  zvals_ray[0] = llc_near.z;
    xvals_ray[1] = llc_far.x;   yvals_ray[1] = llc_far.y;   zvals_ray[1] = llc_far.z;
    xvals_ray[2] = lrc_near.x;  yvals_ray[2] = lrc_near.y;  zvals_ray[2] = lrc_near.z;
    xvals_ray[3] = lrc_far.x;   yvals_ray[3] = lrc_far.y;   zvals_ray[3] = lrc_far.z;
    xvals_ray[4] = urc_near.x;  yvals_ray[4] = urc_near.y;  zvals_ray[4] = urc_near.z;
    xvals_ray[5] = urc_far.x;   yvals_ray[5] = urc_far.y;   zvals_ray[5] = urc_far.z;
    xvals_ray[6] = ulc_near.x;  yvals_ray[6] = ulc_near.y;  zvals_ray[6] = ulc_near.z;
    xvals_ray[7] = ulc_far.x;   yvals_ray[7] = ulc_far.y;   zvals_ray[7] = ulc_far.z;

    // set up ray topo
    conduit::Node &ray_corners_topo = data_out["topologies"]["ray_corners_topo"];
    ray_corners_topo["type"] = "unstructured";
    ray_corners_topo["coordset"] = "ray_corners_coords";
    ray_corners_topo["elements/shape"] = "line";
    ray_corners_topo["elements/connectivity"].set(conduit::DataType::int32(num_points));
    int *conn = ray_corners_topo["elements/connectivity"].value();
    for (int i = 0; i < num_points; i ++) { conn[i] = i; }

    // set up ray trivial field
    conduit::Node &ray_corners_field = data_out["fields"]["ray_corners_field"];
    ray_corners_field["topology"] = "ray_corners_topo";
    ray_corners_field["association"] = "element";
    ray_corners_field["volume_dependent"] = "false";
    ray_corners_field["values"].set(conduit::DataType::float64(num_corners));
    conduit::float64 *field_vals = ray_corners_field["values"].value();
    for (int i = 0; i < num_corners; i ++) { field_vals[i] = 0; }
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBlueprintRaysMesh
//
//  Purpose:
//    This function writes a mesh representing the rays used in the query
//    to the blueprint output.
//
//  Programmer: Justin Privitera
//  Creation:   December 09, 2022
// 
//  Modifications:
//    Justin Privitera, Wed Mar 29 13:48:51 PDT 2023
//    Leverage conduit's features to make the code more legible.
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
avtXRayImageQuery::WriteBlueprintRaysMesh(conduit::Node &data_out,
                                          const double detectorWidth,
                                          const double detectorHeight,
                                          const avtVector &lrc_near,
                                          const double farDetectorWidth,
                                          const double farDetectorHeight,
                                          const avtVector &lrc_far,
                                          const avtVector &left)
{
    // calculate points for rays on near plane and far plane

    // set up ray coords
    const int num_lines{nx * ny};
    const int num_points{num_lines * 2};
    conduit::Node &ray_coords = data_out["coordsets"]["ray_coords"];
    ray_coords["type"] = "explicit";
    ray_coords["values/x"].set(conduit::DataType::float64(num_points));
    ray_coords["values/y"].set(conduit::DataType::float64(num_points));
    ray_coords["values/z"].set(conduit::DataType::float64(num_points));
    double *xvals_ray = ray_coords["values/x"].value();
    double *yvals_ray = ray_coords["values/y"].value();
    double *zvals_ray = ray_coords["values/z"].value();

    avtVector scaledunitleft, scaledunitup, lrc;

    for (int i = 0; i < 2; i ++)
    {
        double dx, dy;
        if (i == 0) // 1st iteration is for the near plane
        {
            dx = detectorWidth / nx;
            dy = detectorHeight / ny;
            lrc = lrc_near;
        }
        else // 2nd iteration is for the far plane
        {
            dx = farDetectorWidth / nx;
            dy = farDetectorHeight / ny;
            lrc = lrc_far;
        }
        scaledunitleft = dx * left.normalized();
        scaledunitup   = dy * viewUp.normalized();

        for (int j = 0; j < nx; j ++)
        {
            for (int k = 0; k < ny; k ++)
            {
                avtVector temp = lrc + (0.5 + j) * scaledunitleft + (0.5 + k) * scaledunitup;
                // 3d to 1d conversion
                const int index{i * nx * ny + j * ny + k};
                xvals_ray[index] = temp[0];
                yvals_ray[index] = temp[1];
                zvals_ray[index] = temp[2];
            } 
        }
    }

    // set up ray topo
    conduit::Node &ray_topo = data_out["topologies"]["ray_topo"];
    ray_topo["type"] = "unstructured";
    ray_topo["coordset"] = "ray_coords";
    ray_topo["elements/shape"] = "line";
    ray_topo["elements/connectivity"].set(conduit::DataType::int32(num_points));
    int *conn = ray_topo["elements/connectivity"].value();
    for (int i = 0; i < num_lines; i ++)
    {
        // connect each point in the near plane to a point in the far plane
        conn[i * 2] = i;
        conn[i * 2 + 1] = i + num_lines;
    }

    // set up ray trivial field
    conduit::Node &ray_field = data_out["fields"]["ray_field"];
    ray_field["topology"] = "ray_topo";
    ray_field["association"] = "element";
    ray_field["volume_dependent"] = "false";
    ray_field["values"].set(conduit::DataType::float64(num_lines));
    conduit::float64 *field_vals = ray_field["values"].value();
    for (int i = 0; i < num_lines; i ++) { field_vals[i] = i; }
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBlueprintImagingMeshes
//
//  Purpose:
//    This function writes the various imaging meshes to the blueprint output.
//    These meshes are specifically useful for visualizing where the x ray
//    detector is looking and what it is looking at.
//
//  Programmer: Justin Privitera
//  Creation:   December 09, 2022
// 
//  Modifications:
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
avtXRayImageQuery::WriteBlueprintImagingMeshes(conduit::Node &data_out,
                                               const double nearWidth, 
                                               const double nearHeight, 
                                               const double viewWidth, 
                                               const double viewHeight, 
                                               const double farWidth, 
                                               const double farHeight,
                                               const double detectorWidth,
                                               const double detectorHeight,
                                               const double farDetectorWidth,
                                               const double farDetectorHeight)
{
    // lower left corner, lower right corner, etc. - for near, image, and far planes
    avtVector llc_near,  lrc_near,  ulc_near,  urc_near;
    avtVector llc_image, lrc_image, ulc_image, urc_image;
    avtVector llc_far,   lrc_far,   ulc_far,   urc_far;
    // plane center
    avtVector center;
    // we will use the values to compute the rays to output for visualization

    // calculate left vector by crossing normal with up vector
    avtVector left = viewUp.cross(normal);

    // write the imaging planes

    // write near plane
    center = nearPlane * normal + focus;
    WriteBlueprintImagingPlane(data_out, "near_plane", nearWidth, nearHeight, 
        center, left, llc_near, lrc_near, ulc_near, urc_near);

    // write view plane
    // we also send the focus vector as the center
    WriteBlueprintImagingPlane(data_out, "view_plane", viewWidth, viewHeight, 
        focus, left, llc_image, lrc_image, ulc_image, urc_image);

    // write far plane
    center = farPlane * normal + focus;
    WriteBlueprintImagingPlane(data_out, "far_plane", farWidth, farHeight, 
        center, left, llc_far, lrc_far, ulc_far, urc_far);

    // write the ray meshes

    WriteBlueprintRayCornersMesh(data_out,
        llc_near, llc_far, lrc_near, lrc_far,
        urc_near, urc_far, ulc_near, ulc_far);

    WriteBlueprintRaysMesh(data_out,
        detectorWidth, detectorHeight, lrc_near,
        farDetectorWidth, farDetectorHeight, lrc_far,
        left);
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBlueprintXRayView
//
//  Purpose:
//    This function handles writing view-related information for the blueprint
//    output metadata.
//
//  Programmer: Justin Privitera
//  Creation:   December 09, 2022
// 
//  Modifications:
//    Justin Privitera, Tue Feb 14 17:02:43 PST 2023
//    Change all metadata to use pot_hole_case instead of camelCase.
// 
//    Justin Privitera, Wed Mar 29 13:48:51 PDT 2023
//    Leverage conduit's features to make the code more legible.
// 
//    Justin Privitera, Fri Jun 16 17:17:14 PDT 2023
//    Added view width override and non square pixels.
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
avtXRayImageQuery::WriteBlueprintXRayView(conduit::Node &xray_view)
{
    xray_view["normal/x"] = normal[0];
    xray_view["normal/y"] = normal[1];
    xray_view["normal/z"] = normal[2];
    xray_view["focus/x"] = focus[0];
    xray_view["focus/y"] = focus[1];
    xray_view["focus/z"] = focus[2];
    xray_view["view_up/x"] = viewUp[0];
    xray_view["view_up/y"] = viewUp[1];
    xray_view["view_up/z"] = viewUp[2];
    xray_view["view_angle"] = viewAngle;
    xray_view["parallel_scale"] = parallelScale;
    xray_view["view_width"] = viewWidthOverride;
    xray_view["non_square_pixels"] = (nonSquarePixels ? "yes" : "no");
    xray_view["near_plane"] = nearPlane;
    xray_view["far_plane"] = farPlane;
    xray_view["image_pan/x"] = imagePan[0];
    xray_view["image_pan/y"] = imagePan[1];
    xray_view["image_zoom"] = imageZoom;
    xray_view["perspective"] = perspective;
    xray_view["perspective_str"] = perspective ? "perspective" : "parallel";
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBlueprintXRayQuery
//
//  Purpose:
//    This function handles writing query-related information for the blueprint
//    output metadata.
//
//  Programmer: Justin Privitera
//  Creation:   December 09, 2022
// 
//  Modifications:
//    Justin Privitera, Tue Feb 14 17:02:43 PST 2023
//    Change all metadata to use pot_hole_case instead of camelCase.
//
//    Justin Privitera, Wed Mar 29 13:48:51 PDT 2023
//    Leverage conduit's features to make the code more legible.
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
avtXRayImageQuery::WriteBlueprintXRayQuery(conduit::Node &xray_query, 
                                           const int numBins)
{
    xray_query["divide_emis_by_absorb"] = divideEmisByAbsorb;
    xray_query["divide_emis_by_absorb_str"] = divideEmisByAbsorb ? "yes" : "no";
    xray_query["num_x_pixels"] = nx;
    xray_query["num_y_pixels"] = ny;
    xray_query["num_bins"] = numBins;
    xray_query["abs_var_name"] = absVarName;
    xray_query["emis_var_name"] = emisVarName;
    xray_query["abs_units"] = absUnits;
    xray_query["emis_units"] = emisUnits;
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBlueprintXRayData
//
//  Purpose:
//    This function handles writing general data for the blueprint output 
//    metadata.
//
//  Programmer: Justin Privitera
//  Creation:   December 09, 2022
// 
//  Modifications:
//    Justin Privitera, Fri Dec 16 18:20:51 PST 2022
//    Changed conduit output data types for spatial extents coords to be 
//    consistent.
// 
//    Justin Privitera, Tue Feb 14 17:02:43 PST 2023
//    Add image_topo_order_of_domain_variables to xray_data metadata.
//    Change all metadata to use pot_hole_case instead of camelCase.
// 
//    Justin Privitera, Wed Mar 29 13:48:51 PDT 2023
//    Leverage conduit's features to make the code more legible.
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
avtXRayImageQuery::WriteBlueprintXRayData(conduit::Node &xray_data, 
                                          const double detectorWidth, 
                                          const double detectorHeight,
                                          const int numfieldvals,
                                          const conduit::float64 *intensity_vals,
                                          const conduit::float64 *depth_vals)
{
    // If the near plane is too far back, it can cause the near width
    // and height to be negative. However, the detector height and 
    // width ought to be positive values, hence the absolute value.
    xray_data["detector_width"] = fabs(detectorWidth);
    xray_data["detector_height"] = fabs(detectorHeight);

    // intensity and path length max and mins
    conduit::float64 int_max, int_min, pl_max, pl_min;
    int_max = int_min = pl_max = pl_min = 0;
    if (numfieldvals > 0)
    {
        int_max = int_min = intensity_vals[0];
        pl_max = pl_min = depth_vals[0];
        for (int i = 0; i < numfieldvals; i ++)
        {
            if (int_max < intensity_vals[i])
                int_max = intensity_vals[i];
            if (int_min > intensity_vals[i])
                int_min = intensity_vals[i];
            if (pl_max < depth_vals[i])
                pl_max = depth_vals[i];
            if (pl_min > depth_vals[i])
                pl_min = depth_vals[i];
        }
    }

    xray_data["intensity_max"] = int_max;
    xray_data["intensity_min"] = int_min;
    xray_data["path_length_max"] = pl_max;
    xray_data["path_length_min"] = pl_min;

    xray_data["image_topo_order_of_domain_variables"] = "xyz";
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBlueprintMetadata
//
//  Purpose:
//    This function handles writing metadata for the blueprint output.
//
//  Programmer: Justin Privitera
//  Creation:   December 09, 2022
// 
//  Modifications:
//    Justin Privitera, Wed Mar 29 13:48:51 PDT 2023
//    Leverage conduit's features to make the code more legible.
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
avtXRayImageQuery::WriteBlueprintMetadata(conduit::Node &metadata,
                                          const int cycle,
                                          const int numBins,
                                          const double detectorWidth, 
                                          const double detectorHeight,
                                          const int numfieldvals,
                                          const conduit::float64 *intensity_vals,
                                          const conduit::float64 *depth_vals)
{
    // top level items
    metadata["time"] = GetInput()->GetInfo().GetAttributes().GetTime();
    metadata["cycle"] = cycle;

    WriteBlueprintXRayView(metadata["xray_view"]);
    WriteBlueprintXRayQuery(metadata["xray_query"], numBins);
    WriteBlueprintXRayData(metadata["xray_data"], detectorWidth, detectorHeight, 
                           numfieldvals, intensity_vals, depth_vals);
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBlueprintMeshCoordsets
//
//  Purpose:
//    This function writes a coordset in pixel space and a coordset in space
//    for the blueprint output.
//
//  Programmer: Justin Privitera
//  Creation:   December 09, 2022
// 
//  Modifications:
//    Justin Privitera, Fri Mar 10 19:06:49 PST 2023
//    Added spatial energy reduced coordset.
// 
//    Justin Privitera, Wed Mar 15 17:51:13 PDT 2023
//    Leverage conduit's features to make the code more legible.
//    Added spectra coordset.
// 
//    Justin Privitera, Mon Aug  7 15:49:36 PDT 2023
//    Warn to debug when missing energy group bounds for blueprint output and
//    when provided energy group bounds are not the right size.
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
avtXRayImageQuery::WriteBlueprintMeshCoordsets(conduit::Node &coordsets,
                                               const int x_coords_dim,
                                               const int y_coords_dim,
                                               const int z_coords_dim,
                                               const double detectorWidth, 
                                               const double detectorHeight)
{
    // set up coords
    conduit::Node &image_coords = coordsets["image_coords"];
    image_coords["type"] = "rectilinear";
    image_coords["values/x"].set(conduit::DataType::int32(x_coords_dim));
    int *xvals = image_coords["values/x"].value();
    for (int i = 0; i < x_coords_dim; i ++) { xvals[i] = i; }

    image_coords["values/y"].set(conduit::DataType::int32(y_coords_dim));
    int *yvals = image_coords["values/y"].value();
    for (int i = 0; i < y_coords_dim; i ++) { yvals[i] = i; }

    image_coords["values/z"].set(conduit::DataType::int32(z_coords_dim));
    int *zvals = image_coords["values/z"].value();
    for (int i = 0; i < z_coords_dim; i ++) { zvals[i] = i; }

    image_coords["labels/x"] = "width";
    image_coords["labels/y"] = "height";
    image_coords["labels/z"] = "energy_group";

    image_coords["units/x"] = "pixels";
    image_coords["units/y"] = "pixels";
    image_coords["units/z"] = "bins";

    // calculate spatial extent coords
    // (the physical extents of the image projected on the near plane)

    const double nearDx{detectorWidth  / imageSize[0]};
    const double nearDy{detectorHeight / imageSize[1]};

    // set up spatial extents coords
    conduit::Node &spatial_coords = coordsets["spatial_coords"];
    spatial_coords["type"] = "rectilinear";
    spatial_coords["values/x"].set(conduit::DataType::float64(x_coords_dim));
    double *spatial_xvals = spatial_coords["values/x"].value();
    for (int i = 0; i < x_coords_dim; i ++) { spatial_xvals[i] = i * nearDx; }

    spatial_coords["values/y"].set(conduit::DataType::float64(y_coords_dim));
    double *spatial_yvals = spatial_coords["values/y"].value();
    for (int i = 0; i < y_coords_dim; i ++) { spatial_yvals[i] = i * nearDy; }

    // include energy group bins in blueprint output if they are provided
    if (energyGroupBounds)
    {
        if (z_coords_dim == nEnergyGroupBounds) // only pass them thru if it makes sense to do so
        {
            spatial_coords["values/z"].set(conduit::DataType::float64(nEnergyGroupBounds));
            double *spatial_zvals = spatial_coords["values/z"].value();
            for (int i = 0; i < nEnergyGroupBounds; i ++) { spatial_zvals[i] = energyGroupBounds[i]; }                    
        }
        else
        {
            std::stringstream out;
            out << "Energy group bounds size mismatch: provided " 
                << nEnergyGroupBounds << " bounds, but " 
                << z_coords_dim << " in query results.";
            debug1 << "X Ray Image Query WARNING: Blueprint Output: " << out.str() << "\n";
            spatial_coords["info"] = out.str();
            spatial_coords["values/z"].set(conduit::DataType::float64(z_coords_dim));
            double *zvals = spatial_coords["values/z"].value();
            for (int i = 0; i < z_coords_dim; i ++) { zvals[i] = i; }
        }
    }
    else
    {
        debug1 << "X Ray Image Query WARNING: Blueprint Output: Energy group bounds not provided." << "\n";
        spatial_coords["info"] = "Energy group bounds not provided.";
        spatial_coords["values/z"].set(conduit::DataType::float64(z_coords_dim));
        double *zvals = spatial_coords["values/z"].value();
        for (int i = 0; i < z_coords_dim; i ++) { zvals[i] = i; }
    }

    spatial_coords["units/x"] = spatialUnits;
    spatial_coords["units/y"] = spatialUnits;
    spatial_coords["units/z"] = energyUnits;

    spatial_coords["labels/x"] = "width";
    spatial_coords["labels/y"] = "height";
    spatial_coords["labels/z"] = "energy_group";

    // set up spatial energy reduced coords
    conduit::Node &spatial_energy_reduced_coords = coordsets["spatial_energy_reduced_coords"];
    spatial_energy_reduced_coords["type"] = "rectilinear";
    // copy over the x and y coords from the spatial_coords
    spatial_energy_reduced_coords["values/x"].set(spatial_coords["values/x"]);
    spatial_energy_reduced_coords["values/y"].set(spatial_coords["values/y"]);

    spatial_energy_reduced_coords["units/x"].set(spatial_coords["units/x"]);
    spatial_energy_reduced_coords["units/y"].set(spatial_coords["units/y"]);

    spatial_energy_reduced_coords["labels/x"].set(spatial_coords["labels/x"]);
    spatial_energy_reduced_coords["labels/y"].set(spatial_coords["labels/y"]);

    // set up spectra coords
    conduit::Node &spectra_coords = coordsets["spectra_coords"];
    spectra_coords["type"] = "rectilinear";
    spectra_coords["values/x"].set(spatial_coords["values/z"]);
    spectra_coords["units/x"].set(spatial_coords["units/z"]);
    spectra_coords["labels/x"].set(spatial_coords["labels/z"]);
    if (spatial_coords.has_child("info"))
        spectra_coords["info"].set(spatial_coords["info"]);
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBlueprintMeshTopologies
//
//  Purpose:
//    This function writes two topologies to the blueprint output, one for the 
//    image coords and one for the spatial extents.
//
//  Programmer: Justin Privitera
//  Creation:   December 09, 2022
// 
//  Modifications:
//    Justin Privitera, Fri Mar 10 19:06:49 PST 2023
//    Added spatial energy reduced topo.
// 
//    Justin Privitera, Wed Mar 15 17:51:13 PDT 2023
//    Leverage conduit's features to make the code more legible.
//    Added spectra topo.
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
avtXRayImageQuery::WriteBlueprintMeshTopologies(conduit::Node &topologies)
{
    // set up image topology
    topologies["image_topo/coordset"] = "image_coords";
    topologies["image_topo/type"] = "rectilinear";

    // set up spatial extents topology
    topologies["spatial_topo/coordset"] = "spatial_coords";
    topologies["spatial_topo/type"] = "rectilinear";

    // set up spatial energy reduced topology
    topologies["spatial_energy_reduced_topo/coordset"] = "spatial_energy_reduced_coords";
    topologies["spatial_energy_reduced_topo/type"] = "rectilinear";

    // set up spectra topology
    topologies["spectra_topo/coordset"] = "spectra_coords";
    topologies["spectra_topo/type"] = "rectilinear";
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBlueprintMeshFields
//
//  Purpose:
//    This function writes intensity and path length fields to the blueprint 
//    output twice, once for the image coords and once for the spatial extents.
//
//  Programmer: Justin Privitera
//  Creation:   December 09, 2022
// 
//  Modifications:
//    Justin Privitera, Fri Mar 10 19:06:49 PST 2023
//    Added spatial energy reduced fields and calculations for them.
// 
//    Justin Privitera, Wed Mar 15 17:51:13 PDT 2023
//    Leverage conduit's features to make the code more legible.
//    Added spectra fields and calculations for them.
//    Consistent error messaging.
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
avtXRayImageQuery::WriteBlueprintMeshFields(conduit::Node &fields, 
                                            const int numfieldvals,
                                            const int numBins,
                                            vtkDataSet **leaves,
                                            conduit::float64 *&intensity_vals,
                                            conduit::float64 *&depth_vals)
{
    // 
    // set up fields for image topo
    // 

    // intensities for image topo
    conduit::Node &intensities = fields["intensities"];
    intensities["topology"] = "image_topo";
    intensities["association"] = "element";
    intensities["units"] = intensityUnits;
    // set to float64 regardless of vtk data types
    intensities["values"].set(conduit::DataType::float64(numfieldvals));
    intensity_vals = intensities["values"].value();

    // path length for image topo
    conduit::Node &path_length = fields["path_length"];
    path_length["topology"] = "image_topo";
    path_length["association"] = "element";
    path_length["units"] = pathLengthUnits;
    // set to float64 regardless of vtk data types
    path_length["values"].set(conduit::DataType::float64(numfieldvals));
    depth_vals = path_length["values"].value();

    // write actual field values
    const int datatype{leaves[0]->GetPointData()->GetArray("Intensity")->GetDataType()};
    if (datatype == VTK_FLOAT)
        WriteArrays<float>(leaves, intensity_vals, depth_vals, numBins);
    else if (datatype == VTK_DOUBLE)
        WriteArrays<double>(leaves, intensity_vals, depth_vals, numBins);
    else if (datatype == VTK_INT)
        WriteArrays<int>(leaves, intensity_vals, depth_vals, numBins);
    else
    {
        std::ostringstream err_oss;
        err_oss << "ERROR: VTKDataType " << datatype << " is not supported.\n";
        SetResultMessage(err_oss.str());
        EXCEPTION1(VisItException, err_oss.str());
    }

    // set strides for image topo fields
    intensities["strides"].set(conduit::DataType::int64(3));
    conduit::int64 *stride_ptr = intensities["strides"].value();
    stride_ptr[0] = 1;
    stride_ptr[1] = nx;
    stride_ptr[2] = nx * ny;
    path_length["strides"].set(intensities["strides"]);

    // 
    // set up fields for spatial topo
    // 

    // intensities
    conduit::Node &intensities_spatial = fields["intensities_spatial"];
    // simply copy over the existing intensities data
    intensities_spatial.set(intensities);
    // then modify the topo
    intensities_spatial["topology"] = "spatial_topo";

    // path length
    conduit::Node &path_length_spatial = fields["path_length_spatial"];
    // simply copy over the existing path length data
    path_length_spatial.set(path_length);
    // then modify the topo
    path_length_spatial["topology"] = "spatial_topo";

    // 
    // set up fields for spatial energy reduced topo
    // 

    // intensities
    conduit::Node &intensities_spatial_energy_reduced = fields["intensities_spatial_energy_reduced"];
    intensities_spatial_energy_reduced["topology"] = "spatial_energy_reduced_topo";
    intensities_spatial_energy_reduced["association"] = "element";
    // set to float64 regardless of vtk data types
    intensities_spatial_energy_reduced["values"].set(conduit::DataType::float64(nx * ny));
    conduit::float64 *ser_intensity_vals = intensities_spatial_energy_reduced["values"].value();

    // path_length
    conduit::Node &path_length_spatial_energy_reduced = fields["path_length_spatial_energy_reduced"];
    path_length_spatial_energy_reduced["topology"] = "spatial_energy_reduced_topo";
    path_length_spatial_energy_reduced["association"] = "element";
    // set to float64 regardless of vtk data types
    path_length_spatial_energy_reduced["values"].set(conduit::DataType::float64(nx * ny));
    conduit::float64 *ser_depth_vals = path_length_spatial_energy_reduced["values"].value();

    // sum reduction
    // nx is the number of x ELEMENTS, same for ny
    for (int i = 0; i < nx; i ++)
    {
        for (int j = 0; j < ny; j ++)
        {
            double int_sum, pl_sum;
            int_sum = pl_sum = 0;
            for (int k = 0; k < numBins; k ++)
            {
                double intensity_val = intensity_vals[i + j * nx + k * nx * ny];
                double path_length_val = depth_vals[i + j * nx + k * nx * ny];
                double bin_width;
                if (nEnergyGroupBounds == numBins + 1)
                    bin_width = energyGroupBounds[k + 1] - energyGroupBounds[k];
                else
                    bin_width = 1;
                int_sum += intensity_val * bin_width;
                pl_sum += path_length_val * bin_width;
            }
            ser_intensity_vals[i + j * nx] = int_sum;
            ser_depth_vals[i + j * nx] = pl_sum;
        }
    }

    // set strides for spatial energy reduced fields
    intensities_spatial_energy_reduced["strides"].set(conduit::DataType::int64(3));
    conduit::int64 *ser_stride_ptr = intensities_spatial_energy_reduced["strides"].value();
    ser_stride_ptr[0] = 1;
    ser_stride_ptr[1] = nx;
    path_length_spatial_energy_reduced["strides"].set(intensities_spatial_energy_reduced["strides"]);

    // 
    // set up fields for spectra topo
    // 

    // intensities
    conduit::Node &intensities_spectra = fields["intensities_spectra"];
    intensities_spectra["topology"] = "spectra_topo";
    intensities_spectra["association"] = "element";
    // set to float64 regardless of vtk data types
    intensities_spectra["values"].set(conduit::DataType::float64(numBins));
    conduit::float64 *spec_intensity_vals = intensities_spectra["values"].value();

    // path_length
    conduit::Node &path_length_spectra = fields["path_length_spectra"];
    path_length_spectra["topology"] = "spectra_topo";
    path_length_spectra["association"] = "element";
    // set to float64 regardless of vtk data types
    path_length_spectra["values"].set(conduit::DataType::float64(numBins));
    conduit::float64 *spec_depth_vals = path_length_spectra["values"].value();

    // calculate pixel area from the coordset
    const conduit::Node &spatial_coords = (*(fields.parent()))["coordsets/spatial_coords"];
    const double *spatial_xvals = spatial_coords["values/x"].value();
    const double *spatial_yvals = spatial_coords["values/y"].value();
    // we take the 1st element because it is 1 pixel width (or height) from the origin
    const double spatial_pixel_area = spatial_xvals[1] * spatial_yvals[1];

    // sum reduction
    for (int k = 0; k < numBins; k ++)
    {
        double int_sum, pl_sum;
        int_sum = pl_sum = 0;
        for (int i = 0; i < nx; i ++)
        {
            for (int j = 0; j < ny; j ++)
            {
                double intensity_val = intensity_vals[i + j * nx + k * nx * ny];
                double path_length_val = depth_vals[i + j * nx + k * nx * ny];
                int_sum += intensity_val * spatial_pixel_area;
                pl_sum += path_length_val * spatial_pixel_area;
            }
        }
        spec_intensity_vals[k] = int_sum;
        spec_depth_vals[k] = pl_sum;
    }
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::WriteBlueprintMeshes
//
//  Purpose:
//    This function crafts two blueprint meshes, one representing the output
//    image in image space, and another representing it in physical space with
//    energy group bounds.
//
//  Programmer: Justin Privitera
//  Creation:   December 09, 2022
// 
//  Modifications:
//    Justin Privitera, Wed Mar 15 17:51:13 PDT 2023
//    Now send the smaller pieces of the conduit node to each writer function
//    instead of the whole thing.
//
// ****************************************************************************
#ifdef HAVE_CONDUIT
void
avtXRayImageQuery::WriteBlueprintMeshes(conduit::Node &data_out, 
                                        const double detectorWidth, 
                                        const double detectorHeight,
                                        const int numBins,
                                        vtkDataSet **leaves,
                                        int &numfieldvals,
                                        conduit::float64 *&intensity_vals,
                                        conduit::float64 *&depth_vals)
{
    const int x_coords_dim = nx + 1;
    const int y_coords_dim = ny + 1;
    const int z_coords_dim = numBins + 1;
    
    // this value is needed elsewhere so we send it back up the call chain
    numfieldvals = (x_coords_dim - 1) * (y_coords_dim - 1) * (z_coords_dim - 1);

    // We write one coordset for the image and one for the spatial extents
    WriteBlueprintMeshCoordsets(data_out["coordsets"], 
        x_coords_dim, y_coords_dim, z_coords_dim,
        detectorWidth, detectorHeight);
    
    // Then we duplicate the topologies and fields for both coordsets
    WriteBlueprintMeshTopologies(data_out["topologies"]);    
    WriteBlueprintMeshFields(data_out["fields"], numfieldvals, numBins, 
        leaves, intensity_vals, depth_vals);
}
#endif

// ****************************************************************************
//  Method: avtXRayImageQuery::GetDefaultInputParams
//
//  Purpose:
//    Retrieves default values for input variables. 
// 
//  Note:
//    If someone uses this function to get the default parameters, modifies
//    them, and runs the query, the query will default to using the simplified
//    view specification even if the user only modified the new view params.
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
//    Justin Privitera, Thu Dec  1 11:39:12 PST 2022
//    Added all missing default input parameters.
// 
//    Justin Privitera, Mon Dec 12 13:28:55 PST 2022
//    Changed path_length_units to path_length_info.
// 
//    Justin Privitera, Fri Jul 14 17:33:07 PDT 2023
//    Removed old parameters from default.
//
// ****************************************************************************

void
avtXRayImageQuery::GetDefaultInputParams(MapNode &params)
{
    stringVector v;
    v.push_back("absorbtivity");
    v.push_back("emissivity");
    params["vars"] = v;

    params["background_intensity"] = 0.0;
    params["background_intensities"] = 0.0;
    params["divide_emis_by_absorb"] = 0;
    params["output_type"] = std::string("png");
    params["output_dir"] = std::string(".");
    params["family_files"] = 0;
    params["filename_scheme"] = 0;

    intVector is;
    is.push_back(200);
    is.push_back(200);
    params["image_size"] = is;

    params["debug_ray"] = -1;
    params["output_ray_bounds"] = 0;

    doubleVector egb;
    egb.push_back(0.0);
    egb.push_back(1.0);
    params["energy_group_bounds"] = egb;

    params["spatial_units"] = std::string("spatial units");
    params["energy_units"] = std::string("energy units");
    params["abs_units"] = std::string("abs units");
    params["emis_units"] = std::string("emis units");
    params["intensity_units"] = std::string("intensity units");
    params["path_length_info"] = std::string("path length info");

    //
    // The view parameters.
    //
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

    doubleVector n;
    n.push_back(0.0);
    n.push_back(0.0);
    n.push_back(1.0);
    params["normal"] = n;

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
//  Modifications:
//    Justin Privitera, Fri Jun 16 17:17:14 PDT 2023
//    Added view width override and non square pixels.
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
    viewWidthOverride = width / 2.;
    nonSquarePixels = true;

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
