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
//                         avtOpenGLStreamlineRenderer.h                       //
// ************************************************************************* //

#ifndef AVT_OPEN_GL_STREAMLINE_RENDERER_H
#define AVT_OPEN_GL_STREAMLINE_RENDERER_H

#include <avtStreamlineRendererImplementation.h>
#include <StreamlineAttributes.h>
#include <string>

#define MAX_DETAIL_LEVELS 5

// ****************************************************************************
//  Class: avtOpenGLStreamlineRenderer
//
//  Purpose:
//      A custom renderer for streamlines.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

class avtOpenGLStreamlineRenderer : public avtStreamlineRendererImplementation
{
  public:
                            avtOpenGLStreamlineRenderer();
    virtual                ~avtOpenGLStreamlineRenderer();

  protected:
    virtual void            Render(vtkPolyData *data,
                                   const StreamlineAttributes&,
                                   bool immediateModeRendering,
                                   double vMin, double vMax,
                                   float ambient_coeff,
                                   float spec_coeff, float spec_power,
                                   float spec_r, float spec_g, float spec_b,
                                   const int *);

    virtual void   InvalidateColors();

    virtual void   SetLevelsLUT(avtLookupTable *);

    unsigned int displaylistid;
    StreamlineAttributes atts;

    float varMin, varMax, varDiff;
    
    float ambient_coeff;
    float spec_coeff;
    float spec_power;
    float spec_r;
    float spec_g;
    float spec_b;
    avtLookupTable *levelsLUT;

    float *spherePts[MAX_DETAIL_LEVELS];

    bool spheres_calculated;
    void CalculateSpherePts();
    void DrawSphereAsQuads(float, float, float, float r, int);

    std::string  colorTableName;
    std::vector<unsigned char> colorTable;

    void DrawStreamlines(vtkPolyData *data);

    void DrawAsLines(vtkPolyData *data);
    void DrawAsTubes(vtkPolyData *data);
    void DrawAsRibbons(vtkPolyData *data);
    
    void DrawSeedPoints(vtkPolyData *data);

    vtkPolyData *MakeNewPolyline(vtkPolyData *data,
                                 int *&segptr);
    void DrawPolyData(vtkPolyData *data);
    bool GetEndPoints(vtkPolyData *data, 
                      int *segptr,
                      int nPts,
                      int &j0,
                      int &j1,
                      double &t0,
                      double &t1);
    
    inline void SetColor(const float &scalar, const float &opacity) const;
    void InitColors();
};


#endif


