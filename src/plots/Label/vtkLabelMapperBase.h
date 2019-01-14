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
//                       vtkLabelMapperBase.h                                //
// ************************************************************************* //

#ifndef VTK_LABEL_MAPPER_BASE_H
#define VTK_LABEL_MAPPER_BASE_H

#include <vtkMapper2D.h>
#include <vtkSmartPointer.h>

#include <LabelAttributes.h>
#include <avtViewInfo.h>

#include <string>

class vtkDataSet;
class vtkDoubleArray;
class vtkRenderer;
class vtkTextMapper;
class vtkTextProperty;

// ****************************************************************************
// Class: vtkLabelMapperBase
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

class vtkLabelMapperBase : public vtkMapper2D
{
public:
    vtkTypeMacro(vtkLabelMapperBase, vtkMapper2D);
    void PrintSelf(ostream &os, vtkIndent indent) override;


    // VTK Pipeline methods
    virtual void SetInputData(vtkDataObject*);
    vtkDataSet *GetInput();

    void RenderOpaqueGeometry(vtkViewport* viewport, vtkActor2D* actor) override;
    void RenderOverlay(vtkViewport* viewport, vtkActor2D* actor) override;
 
    void ReleaseGraphicsResources(vtkWindow *) override;

    vtkMTimeType GetMTime() override;


    // this Class specific methods
    void SetAtts(LabelAttributes *);

    void SetGlobalLabel(const std::string &s) { this->GlobalLabel = s;}
    vtkSetMacro(UseGlobalLabel, bool);
    enum RenderActions
    {
        RENDERER_ACTION_NOTHING      =0,
        RENDERER_ACTION_INIT_ZBUFFER =1,
        RENDERER_ACTION_FREE_ZBUFFER =2
    };

    vtkSetMacro(RendererAction, int);
    void SetVariable(const std::string &s);
    vtkSetMacro(TreatAsASCII, bool);
    vtkSetMacro(RenderLabels3D, bool);
    vtkSetVector6Macro(SpatialExtents, double);
    vtkSetVector3Macro(ForegroundColor, double);
    vtkSetMacro(CellOrigin, vtkIdType);
    vtkSetMacro(NodeOrigin, vtkIdType);

protected:
    vtkLabelMapperBase();
    virtual ~vtkLabelMapperBase();

    vtkTimeStamp BuildTime;

    int FillInputPortInformation(int, vtkInformation *) override;



    void BuildLabels(vtkRenderer *);
    virtual void BuildLabelsInternal(vtkDataSet*, vtkRenderer*) = 0;

    void SetTextAtts(vtkViewport *);




protected:
    struct LabelInfo
    {
        LabelInfo(); 
        ~LabelInfo();

        double screenPoint[3];
        double realPoint[3];
        int   type;
        const char *label;
    };

    void CreateCachedCellLabels(vtkDataSet *input);
    void CreateCachedNodeLabels(vtkDataSet *input);
    void ClearLabelCaches();
    void ResetLabelBins();
    bool AllowLabelInBin(const double *screenPoint, 
                         const char *labelString, int t,
                         const double *realPoint);

    vtkDoubleArray *GetCellCenterArray(vtkDataSet*);

    LabelAttributes        atts;
    int                    MaxLabelSize;

    std::string            VarName;
    bool                   TreatAsASCII;
    bool                   RenderLabels3D;
    vtkIdType              CellOrigin;
    vtkIdType              NodeOrigin;
    double                 ForegroundColor[3];
    double                 SpatialExtents[6];
    std::string            GlobalLabel;
    bool                   UseGlobalLabel;
    int                    RendererAction;

    int                    numXBins, numYBins;
    LabelInfo             *LabelBins;
    int                    maxLabelLength;
    int                    maxLabelRows;


    //
    // Cache the label strings for cell and node variables.
    //
    char                  *CellLabelsCache;
    int                    CellLabelsCacheSize;
    char                  *NodeLabelsCache;
    int                    NodeLabelsCacheSize;

    //
    // Visible point lookup
    bool                   visiblePoint[256];

    // from vtkLabeledDataMapper
    std::vector<vtkSmartPointer<vtkTextMapper> > TextMappers;
    std::vector<double> LabelPositions;

    vtkSmartPointer<vtkTextProperty> NodeLabelProperty;
    vtkSmartPointer<vtkTextProperty> CellLabelProperty;

    avtViewInfo            visit_view; 

private:
    vtkLabelMapperBase(const vtkLabelMapperBase&) VTK_DELETE_FUNCTION;
    void operator=(const vtkLabelMapperBase&) VTK_DELETE_FUNCTION;
};

#endif
