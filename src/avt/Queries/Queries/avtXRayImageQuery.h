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

// ************************************************************************* //
//                            avtXRayImageQuery.h                            //
// ************************************************************************* //

#ifndef AVT_XRAY_IMAGE_QUERY_H
#define AVT_XRAY_IMAGE_QUERY_H

#include <query_exports.h>
#include <avtDatasetQuery.h>
#include <avtXRayFilter.h>

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
    void                      SetDebugRay(const int &ray);
    void                      SetOutputRayBounds(const bool &flag);
    void                      SetFamilyFiles(const bool &flag);
    void                      SetOutputType(int type);
    void                      SetOutputType(const std::string &type);

  protected:
    bool                      divideEmisByAbsorb;
    double                    backgroundIntensity;
    double                   *backgroundIntensities;
    int                       nBackgroundIntensities;
    int                       debugRay;
    bool                      outputRayBounds;
    bool                      familyFiles;
    static int                iFileFamily;
    int                       outputType;
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

    void                      ConvertOldImagePropertiesToNew();
};


#endif
