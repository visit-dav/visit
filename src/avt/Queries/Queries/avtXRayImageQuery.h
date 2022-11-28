// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtXRayImageQuery.h                            //
// ************************************************************************* //

#ifndef AVT_XRAY_IMAGE_QUERY_H
#define AVT_XRAY_IMAGE_QUERY_H

#include <query_exports.h>
#include <avtDatasetQuery.h>
#include <avtXRayFilter.h>

#ifdef HAVE_CONDUIT
    #include <conduit.hpp>
    #include <conduit_blueprint.hpp>
    #include <conduit_relay.hpp>
#endif

#include <string>
#include <vector>

// ****************************************************************************
//  Class: avtXRayImageQuery
//
//  Purpose:
//    This query calculates an x-ray image for multiple radiation bins.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
//
//  Modifications:
//    Eric Brugger, Tue Sep  7 16:15:13 PDT 2010
//    I added logic to detect RZ meshes with negative R values up front
//    before any processing to avoid more complex error handling during
//    pipeline execution.
//
//    Eric Brugger, Tue Dec 28 14:38:47 PST 2010
//    I moved all the logic for doing the ray integration and creating the
//    image in chunks to avtXRayFilter.
//
//    Kathleen Biagas, Fri Jun 17 15:26:06 PDT 2011
//    Added SetInputParams. Changed args to Set* methods to match what
//    is stored in the map node.
//
//    Kathleen Biagas, Fri Jul 15 16:01:19 PDT 2011
//    Added GetDefaultInputParams.
//
//    Eric Brugger, Mon May 14 10:22:29 PDT 2012
//    I added the bov output type.
//
//    Kathleen Biagas, Wed Oct 17 12:10:13 PDT 2012
//    Added upVector.
//
//    Kathleen Biagas, Wed Oct 17 14:13:26 PDT 2012
//    Added useSpecifiedUpVector, to allow support for old-style cli queries,
//    where the upVector wasn't available.
//
//    Gunther H. Weber, Wed Jan 23 15:27:53 PST 2013
//    Added support for specifying background intensity entering volume.
//
//    Eric Brugger, Thu Nov 20 16:54:28 PST 2014
//    Added a new way to specify the view that matches the way the view is
//    specified for plots.
//
//    Eric Brugger, Thu Jan 15 13:26:26 PST 2015
//    I added support for specifying background intensities on a per bin
//    basis.
//
//    Eric Brugger, Thu May 21 12:15:18 PDT 2015
//    I added support for debugging a ray.
//
//    Eric Brugger, Wed May 27 10:45:28 PDT 2015
//    I modified the query to also output the path length field when
//    outputting in bof or bov format.
//
//    Eric Brugger, Wed May 27 13:19:12 PDT 2015
//    I added an option to family output files.
//
//    Eric Brugger, Thu Jun  4 16:07:06 PDT 2015
//    I added an option to enable outputting the ray bounds to a vtk file.
// 
//    Justin Privitera, Tue Jun 14 10:21:03 PDT 2022
//    Added conduit include here, added output dir field + setter, 
//    added write arrays method for writing conduit blueprint output.
// 
//    Justin Privitera, Tue Nov 15 11:44:01 PST 2022
//    Added WriteBlueprintImagingPlane function if conduit is defined
// 
//    Justin Privitera, Tue Nov 22 14:56:04 PST 2022
//    Added energy group bin variables and a setter.
//
// ****************************************************************************

class QUERY_API avtXRayImageQuery : public avtDatasetQuery
{
  public:
                              avtXRayImageQuery();
    virtual                  ~avtXRayImageQuery();

    virtual const char       *GetType(void) 
                                 { return "avtXRayImageQuery"; }
    virtual const char       *GetDescription(void)
                                 { return "Calculating x ray images"; }

    virtual void              SetInputParams(const MapNode &);
    static void               GetDefaultInputParams(MapNode &);

    void                      SetVariableNames(const stringVector &names);
    void                      SetOrigin(const doubleVector &_origin);
    void                      SetOrigin(const intVector &_origin);
    void                      SetUpVector(const doubleVector &_upvector);
    void                      SetUpVector(const intVector &_upvector);
    void                      SetTheta(const double &thetaInDegrees);
    void                      SetPhi(const double &phiInDegrees);
    void                      SetWidth(const double &size);
    void                      SetHeight(const double &size);
    void                      SetImageSize(const intVector &size);
    void                      SetDivideEmisByAbsorb(const bool &flag);
    void                      SetBackgroundIntensity(const double &intensity);
    void                      SetBackgroundIntensities(
                                  const doubleVector &intensities);
    void                      SetEnergyGroupBounds(
                                  const doubleVector &bins);
    void                      SetDebugRay(const int &ray);
    void                      SetOutputRayBounds(const bool &flag);
    void                      SetFamilyFiles(const bool &flag);
    void                      SetOutputType(int type);
    void                      SetOutputType(const std::string &type);
    void                      SetOutputDir(const std::string &dir);

  protected:
    bool                      divideEmisByAbsorb;
    double                    backgroundIntensity;
    double                   *backgroundIntensities;
    int                       nBackgroundIntensities;
    double                   *energyGroupBounds;
    int                       nEnergyGroupBounds;
    int                       debugRay;
    bool                      outputRayBounds;
    bool                      familyFiles;
    static int                iFileFamily;
    int                       outputType;
    std::string               outputDir;
    bool                      useSpecifiedUpVector;
    bool                      useOldView;
    // The new view specification
    double                    normal[3];
    double                    focus[3];
    double                    viewUp[3];
    double                    viewAngle;
    double                    parallelScale;
    double                    nearPlane;
    double                    farPlane;
    double                    imagePan[2];
    double                    imageZoom;
    bool                      perspective;
    int                       imageSize[2];
    // The old view specification
    double                    origin[3];
    double                    upVector[3];
    double                    theta, phi;
    double                    width, height;
    int                       nx, ny;

    std::string               absVarName;  //e.g. "absorbtivity"
    std::string               emisVarName; //e.g. "emissivity"

    int                       numPixels;

    virtual void              Execute(vtkDataSet *, const int);

    virtual void              GetSecondaryVars(std::vector<std::string> &);

  private:
    virtual void              Execute(avtDataTree_p);

    template <typename T>
    void                      WriteImage(const char *, int, int, T*);
    template <typename T>
    void                      WriteFloats(const char *, int, int, T*);
    void                      WriteBOVHeader(const char *, const char *,
                                  int, int, int, const char *);
#ifdef HAVE_CONDUIT
    template <typename T>
    void                      WriteArrays(vtkDataSet **leaves, 
                                          conduit::float64 *intensity_vals,
                                          conduit::float64 *depth_vals,
                                          int numBins);
    void                      WriteBlueprintImagingPlane(conduit::Node &data_out,
                                                         const std::string plane_name,
                                                         const double width,
                                                         const double height,
                                                         const double center[3]);
#endif
    void                      ConvertOldImagePropertiesToNew();
    void                      CheckData(vtkDataSet **, const int);
};


#endif
