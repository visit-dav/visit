/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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
//                             avtLightingModel.h                            //
// ************************************************************************* //

#ifndef AVT_LIGHTING_MODEL
#define AVT_LIGHTING_MODEL

#include <pipeline_exports.h>

#include <LightList.h>

class     avtRay;


// ****************************************************************************
//  Class: avtLightingModel
//
//  Purpose:
//      An abstract type that defines a lighting model.  It is only an
//      interface.
//
//  Programmer: Hank Childs
//  Creation:   November 29, 2000
//
//  Modifications:
//    Brad Whitlock, Wed Apr 24 10:18:48 PDT 2002
//    Added constructor and destructor so the vtable gets into the Windows DLL.
//
// ****************************************************************************

class PIPELINE_API avtLightingModel
{
  public:
                      avtLightingModel();
    virtual          ~avtLightingModel();

    virtual void      AddLighting(int, const avtRay *, unsigned char *) 
                             const = 0;

    void              SetGradientVariableIndex(int gvi)
                             { gradientVariableIndex = gvi; };
    void              SetViewDirection(double *vd)
                             { view_direction[0] = vd[0];
                               view_direction[1] = vd[1];
                               view_direction[2] = vd[2]; };
    void              SetViewUp(double *vu)
                             { view_up[0] = vu[0];
                               view_up[1] = vu[1];
                               view_up[2] = vu[2]; };
    void              SetLightInfo(const LightList &ll)
                             { lights = ll; };
    void              SetSpecularInfo(bool ds, double sc, double sp)
                             { doSpecular = ds; specularCoeff = sc;
                               specularPower = sp; };

  protected:
    int               gradientVariableIndex;
    double            view_direction[3];
    double            view_up[3];
    LightList         lights;
    bool              doSpecular;
    double            specularCoeff;
    double            specularPower;
};


#endif


