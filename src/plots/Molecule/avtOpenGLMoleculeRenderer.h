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
//                         avtOpenGLMoleculeRenderer.h                       //
// ************************************************************************* //

#ifndef AVT_OPEN_GL_MOLECULE_RENDERER_H
#define AVT_OPEN_GL_MOLECULE_RENDERER_H

#include <avtMoleculeRendererImplementation.h>
#include <MoleculeAttributes.h>
#include <string>

#define MAX_DETAIL_LEVELS 4

// ****************************************************************************
//  Class: avtOpenGLMoleculeRenderer
//
//  Purpose:
//      An implementation of a molecule renderer that uses OpenGL calls.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
//  Modifications:
//    Brad Whitlock, Mon Mar 27 16:27:18 PST 2006
//    Added support for imposter rendering.
//
//    Jeremy Meredith, Tue Aug 29 11:28:15 EDT 2006
//    Changed point locations to doubles.
//
// ****************************************************************************

class avtOpenGLMoleculeRenderer : public avtMoleculeRendererImplementation
{
  public:
                            avtOpenGLMoleculeRenderer();
    virtual                ~avtOpenGLMoleculeRenderer();

  protected:
    virtual void            Render(vtkPolyData *data,
                                   const MoleculeAttributes&,
                                   bool immediateModeRendering,
                                   float varmin, float varmax,
                                   float ambient_coeff,
                                   float spec_coeff, float spec_power,
                                   float spec_r, float spec_g, float spec_b,
                                   const int *);

    virtual void   InvalidateColors();

    virtual void   SetLevelsLUT(avtLookupTable *);

    bool  immediatemode;
    unsigned int displaylistid;
    MoleculeAttributes atts_for_displaylist;

    float varmin;
    float varmax;

    float ambient_coeff;
    float spec_coeff;
    float spec_power;
    float spec_r;
    float spec_g;
    float spec_b;
    avtLookupTable *levelsLUT;

    float *sphere_pts[MAX_DETAIL_LEVELS];
    float *cyl_pts[MAX_DETAIL_LEVELS];

    bool spheres_calculated;
    void CalculateSpherePts();
    void DrawSphereAsQuads(float, float, float, float r, int);

    bool cylinders_calculated;
    void CalculateCylPts();
    void DrawCylinderBetweenTwoPoints(double *, double *, float r, int);

    int            numcolors;
    unsigned char *colors;
    std::string    colortablename;
    bool           ct_is_discrete;

    void SetColors(vtkPolyData *data,
                   const MoleculeAttributes&);

    void DrawAtomsAsSpheres(vtkPolyData *data,
                            const MoleculeAttributes&);

    void DrawBonds(vtkPolyData *data,
                   const MoleculeAttributes&);

    void *tex;
};


#endif


