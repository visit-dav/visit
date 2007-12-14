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
//                      avtMesaLabelRenderer.h                               //
// ************************************************************************* //

#ifndef AVT_MESA_LABEL_RENDERER_H
#define AVT_MESA_LABEL_RENDERER_H
#include <avtLabelRenderer.h>

// ****************************************************************************
// Class: avtMesaLabelRenderer
//
// Purpose:
//   Renders the labels using Mesa.
//
// Notes:      The contents of this file are essentially the same as
//             avtOpenGLLabelRenderer.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:46:36 PDT 2004
//
// Modifications:
//   Brad Whitlock, Thu Aug 4 10:35:24 PDT 2005
//   Added some new helper methods.
//
//   Brad Whitlock, Tue Apr 25 10:15:48 PDT 2006
//   Moved some point transformation code from the base class to here.
//
// ****************************************************************************

class avtMesaLabelRenderer : public avtLabelRenderer
{
public:
                            avtMesaLabelRenderer();
    virtual                ~avtMesaLabelRenderer();
    virtual void            ReleaseGraphicsResources();

protected:
    void SetupGraphicsLibrary();
    void RenderLabels();
    void DrawLabel(const double *vert, const char *label);
    void DrawLabel2(const double *vert, const char *label);
    void ClearCharacterDisplayLists();

    void DrawLabels2D();
    void DrawAllLabels2D(bool, bool);
    void DrawDynamicallySelectedLabels2D(bool, bool);

    void DrawLabels3D();
    void DrawAllCellLabels3D();
    void DrawAllNodeLabels3D();

    void PopulateBinsWithCellLabels3D();
    void PopulateBinsWithNodeLabels3D();
    void PopulateBinsHelper(const unsigned char *, const char *, const float *,
                            int, int);
    void ClearZBuffer();
    void InitializeZBuffer(bool, bool);

    void SetColor(int);
    void BeginSize2D(int);
    void EndSize2D();

    float *TransformPoints(const float *inputPoints,
                           const unsigned char *quantizedNormalIndices,
                           int nPoints);

    float                  x_scale;
    float                  y_scale;
    int                    characterDisplayListIndices[256];
    bool                   characterDisplayListsCreated;

    double                 pointXForm[4][4];

    int                    zBufferMode;
    float                 *zBuffer;
    int                    zBufferWidth;
    int                    zBufferHeight;
    float                  zTolerance;
};

#endif
