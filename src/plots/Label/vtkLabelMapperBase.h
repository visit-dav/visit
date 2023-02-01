// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       vtkLabelMapperBase.h                                //
// ************************************************************************* //

#ifndef VTK_LABEL_MAPPER_BASE_H
#define VTK_LABEL_MAPPER_BASE_H

#include <vtkVisItFullFrameMapper2D.h>
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
//     Alister Maguire, Fri May 21 15:11:53 PDT 2021
//     Inherit from vtkVistFullFrameMapper2d instead of vtkMapper2D. This
//     allows us to easily interact with full frame mode in the VisIt
//     pipeline.
//
// ****************************************************************************

class vtkLabelMapperBase : public vtkVisItFullFrameMapper2D
{
public:
    vtkTypeMacro(vtkLabelMapperBase, vtkVisItFullFrameMapper2D);
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
    vtkLabelMapperBase(const vtkLabelMapperBase&) = delete;
    void operator=(const vtkLabelMapperBase&) = delete;
};

#endif
