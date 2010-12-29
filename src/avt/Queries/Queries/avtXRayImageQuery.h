/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

    int                       numPixels;

    virtual void              Execute(vtkDataSet *, const int);

    virtual void              GetSecondaryVars(std::vector<std::string> &);

  private:
    virtual void              Execute(avtDataTree_p);

    void                      WriteImage(int, int, float*);
    void                      WriteFloats(int, int, float*);
};


#endif
