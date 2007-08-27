/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                      avtOpenGLParallelAxisRenderer.h                      //
// ************************************************************************* //

#ifndef AVT_OPENGL_PARALLELAXIS_RENDERER_H
#define AVT_OPENGL_PARALLELAXIS_RENDERER_H
#include <avtParallelAxisRenderer.h>

// ****************************************************************************
// Class: avtOpenGLParallelAxisRenderer
//
// Purpose: Renders components of a ParallelAxis plot using OpenGL.
//
// Programmer: Mark Blair
// Creation:   Thu Jul  5 19:06:33 PDT 2007
//
// Modifications:
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
// ****************************************************************************

class avtOpenGLParallelAxisRenderer : public avtParallelAxisRenderer
{
public:
                              avtOpenGLParallelAxisRenderer();
    virtual                  ~avtOpenGLParallelAxisRenderer();
    virtual void              ReleaseGraphicsResources();

protected:
    void		      SetupGraphicsLibrary();
    void		      RenderPlotComponents();

    void		      DrawDataCurves(int pointCount, bool sameInput,
                                  int processorRank, int partitionSize);
    void                      DrawAxesAndAxisLabels(int pointCount);
    void                      DrawAxisTitles(int pointCount);
    void                      DrawContextQuads(int pointCount, int brightLevel);
    bool                      NeedToFindDrawableCurves(
                                  bool sameInput, int processorRank);
    void                      InitializeCurveComparisonAtts(int partitionSize);
    void                      ComputeDrawableCurveTemplates();
    void                      FindDrawableCurves(
                                  int pointCount, int processorRank);
    void                      ComputeWorldCoordinateExtents();

    int			      dataCurveLength;
    int                       testAxisOffset;
    int			      leftTestAxisID;
    int			      rightTestAxisID;

    std::vector<stringVector> procOrderedAxisNames;
    std::vector<doubleVector> procExtentMinima;
    std::vector<doubleVector> procExtentMaxima;
    std::vector<intVector>    procAxisInfoFlagSets;

    std::vector<unsigned int> firstDrawCoordIndexs;
    floatVector               drawableCoord2s;
    floatVector               drawableCoord5s;

    bool                      applyExtents;
    doubleVector              worldExtentMinima;
    doubleVector              worldExtentMaxima;
};

#endif
