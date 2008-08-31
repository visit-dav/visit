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
//                              avtIntegrationRF.h                           //
// ************************************************************************* //

#ifndef AVT_INTEGRATION_RF_H
#define AVT_INTEGRATION_RF_H

#include <pipeline_exports.h>

#include <avtRayFunction.h>


// ****************************************************************************
//  Class: avtIntegrationRF
//
//  Purpose:
//      The derived type of ray function that will perform the integration
//      of $int_0^{maxX} \rho(X) dx$ for each ray.
//
//  Programmer: Hank Childs
//  Creation:   September 8, 2006
//
//  Modifications:
//
//    Hank Childs, Tue Mar 13 16:13:05 PDT 2007
//    Add support for setting distance along the plane.
//
//    Hank Childs, Sun Aug 31 08:42:03 PDT 2008
//    Modify the signature of GetRayValue.
//
// ****************************************************************************

class PIPELINE_API avtIntegrationRF : public avtRayFunction
{
  public:
                        avtIntegrationRF(avtLightingModel *);
    virtual            ~avtIntegrationRF();

    virtual void        GetRayValue(const avtRay *,
                                    unsigned char rgb[3], float);
    virtual bool        NeedPixelIndices(void) { return true; };

    static void         SetWindowSize(int, int);
    static void         OutputRawValues(const char *);

    void                SetRange(double m1, double m2)
                                  { min = m1; max = m2; };
    void                SetDistance(double d) { distance = d; };

  protected:
    static int          windowSize[2];
    static double      *vals;
    static bool         issuedWarning;
    double              min, max;
    double              distance;
};


#endif


