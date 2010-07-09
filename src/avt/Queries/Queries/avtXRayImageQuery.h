/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <vtkPolyData.h>

// ****************************************************************************
//  Class: avtXRayImageQuery
//
//  Purpose:
//    This query calculates an x-ray image for multiple radiation bins.
//
//  Programmer: Eric Brugger
//  Creation:   June 30, 2010
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

    void                      SetVariableNames(const stringVector &names);
    void                      SetOrigin(float x, float y, float z);
    void                      SetThetaPhi(float thetaInDegrees, float phiInDegrees);
    void                      SetWidthHeight(float w, float h);
    void                      SetImageSize(int nx, int ny);
    void                      SetDivideEmisByAbsorb(bool flag);
    void                      SetOutputType(int type);

  protected:
    float                     origin[3];
    float                     theta, phi;
    float                     width, height;
    int                       nx, ny;
    bool                      divideEmisByAbsorb;
    int                       outputType;

    std::string               absVarName;  //e.g. "absorbtivity"
    std::string               emisVarName; //e.g. "emissivity"
    double *                  radBins;

    int                       numBins;    //Used for radiation bins.
                                          //Number is obtained from the mesh,
                                          //not set by the user
    int                       numPixels;
    int                       numPixelsPerIteration;
    std::string               varname;

    int                       iPass;
    int                       numPasses;

    int                       iFragment;
    int                       nImageFragments;
    int                      *imageFragmentSizes;
    float                   **imageFragments;

    int                       actualPixelsPerIteration;
    int                       pixelsForFirstPass;
    int                       pixelsForLastPass;
    int                       pixelsForFirstPassFirstProc;
    int                       pixelsForFirstPassLastProc;
    int                       pixelsForLastPassFirstProc;
    int                       pixelsForLastPassLastProc;

    virtual void              Execute(vtkDataSet *, const int);

    virtual void              GetSecondaryVars(std::vector<std::string> &);

  private:
    virtual void              Execute(avtDataTree_p);
    virtual void              ExecuteTree(avtDataTree_p);
    virtual void              PreExecute(void);
    virtual void              PostExecute(void);

    void                      IntegrateLines(vtkPolyData *, int*);
    float                    *CollectImages(int, int, int*, float **);
    void                      WriteImage(int, int, int*, float**);
    void                      WriteFloats(int, int, int*, float**);
};


#endif
