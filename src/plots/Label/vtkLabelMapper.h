/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                           vtkLabelMapper.h                                //
// ************************************************************************* //

#ifndef VTK_LABEL_MAPPER_H
#define VTK_LABEL_MAPPER_H

#include <vtkLabelMapperBase.h>
#include <vtkSmartPointer.h>

#include <LabelAttributes.h>
#include <avtViewInfo.h>


class vtkRenderer;
class vtkTextMapper;
class vtkTextProperty;

// ****************************************************************************
// Class: vtkLabelMapper
//
// Purpose:  vtk mapper for labels
//   
//
// Programmer: Kathleen Biagas 
// Creation:   April 13, 2017
//
// Modifications:
//
// ****************************************************************************

class vtkLabelMapper : public vtkLabelMapperBase
{
public:
    static vtkLabelMapper *New();
    vtkTypeMacro(vtkLabelMapper, vtkLabelMapperBase);

    void ReleaseGraphicsResources(vtkWindow *) override;

protected:
    vtkLabelMapper();
   ~vtkLabelMapper();

    vtkTimeStamp BuildTime;

    void BuildLabelsInternal(vtkDataSet*, vtkRenderer*) override;

    // from openglrenderer
    void DrawLabels2D(vtkDataSet *, vtkRenderer *);
    void DrawAllLabels2D(vtkDataSet *);
    void DrawDynamicallySelectedLabels2D(vtkDataSet *, vtkRenderer *);

    void DrawLabels3D(vtkDataSet *, vtkRenderer*);
    void DrawAllCellLabels3D(vtkDataSet *, vtkRenderer*);
    void DrawAllNodeLabels3D(vtkDataSet *, vtkRenderer*);

    void PopulateBinsWithCellLabels3D(vtkDataSet *, vtkRenderer *);
    void PopulateBinsWithNodeLabels3D(vtkDataSet *, vtkRenderer *);
    void PopulateBinsHelper(vtkRenderer *, const unsigned char *, 
                            const char *, const double *, vtkIdType, int, 
                            const double *);
    void ClearZBuffer();
    void InitializeZBuffer(vtkDataSet *, vtkRenderer *, bool, bool);

    template <typename T>
    double *TransformPoints(T inputPoints,
                           const unsigned char *quantizedNormalIndices,
                           int nPoints,
                           vtkViewport *,
                           double *);

    double                 pointXForm[4][4];

    int                    zBufferMode;
    float                 *zBuffer;
    int                    zBufferWidth;
    int                    zBufferHeight;
    float                  zTolerance;
    static bool            zBufferWarningIssued;

private:
    vtkLabelMapper(const vtkLabelMapper&) VTK_DELETE_FUNCTION;
    void operator=(const vtkLabelMapper&) VTK_DELETE_FUNCTION;
};

#endif
