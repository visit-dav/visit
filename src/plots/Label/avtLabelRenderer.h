// ************************************************************************* //
//                             avtLabelRenderer.h                            //
// ************************************************************************* //

#ifndef AVT_VOLUME_RENDERER_H
#define AVT_VOLUME_RENDERER_H

#include <avtCustomRenderer.h>
#include <LabelAttributes.h>
#include <vtkSystemIncludes.h>

class vtkDataArray;
class vtkFloatArray;
class vtkMatrix4x4;
class vtkPoints;
class vtkPolyData;

#define MAX_LABEL_SIZE 36

// ****************************************************************************
//  Class: avtLabelRenderer
//
//  Purpose:
//      An implementation of an avtCustomRenderer for a Label plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jan 12 09:22:24 PDT 2004
//
//  Modifications:
//    Brad Whitlock, Mon Oct 25 15:57:39 PST 2004
//    Changed a couple things so it works with scalable rendering.
//
// ****************************************************************************

class avtLabelRenderer : public avtCustomRenderer
{
public:
                            avtLabelRenderer();
    virtual                ~avtLabelRenderer();

    virtual void            Render(vtkDataSet *);
    virtual void            ReleaseGraphicsResources();

    void                    SetAtts(const AttributeGroup*);
    bool                    SetForegroundColor(const double *);
    void                    SetVariable(const char *);
    void                    SetTreatAsASCII(bool);
    void                    Set3D(bool val);
    void                    SetExtents(const float *ext);
protected:
    struct LabelInfo
    {
        LabelInfo(); 
        ~LabelInfo();

        float screenPoint[3];
        const char *label;
    };

    virtual void RenderLabels() = 0;

    void CreateCachedCellLabels();
    void CreateCachedNodeLabels();
    void ClearLabelCaches();

    void ResetLabelBins();
    void PopulateBinsWithCellLabels3D();
    void PopulateBinsWithNodeLabels3D();

    void SetupSingleCellLabel();
    void SetupSingleNodeLabel();

    bool AllowLabelInBin(const float *vert, const char *labelString);

    vtkFloatArray *GetCellCenterArray();
    float *TransformPoints(const float *inputPoints,
                           const unsigned char *quantizedNormalIndices,
                           int nPoints);
    vtkMatrix4x4 *WorldToDisplayMatrix() const;

    LabelAttributes        atts;

    vtkPolyData           *input;
    char                  *varname;
    bool                   treatAsASCII;
    bool                   renderLabels3D;
    double                 fgColor[4];
    float                  spatialExtents[6];

    int                    numXBins, numYBins;
    LabelInfo             *labelBins;
    LabelInfo              singleCellInfo;
    LabelInfo              singleNodeInfo;
    int                    singleCellIndex;
    int                    singleNodeIndex;
    int                    maxLabelLength;
    //
    // Cache the label strings for cell variables.
    //
    char                  *cellLabelsCache;
    bool                   cellLabelsCached;
    int                    cellLabelsCacheSize;
    //
    // Cache the label strings for node variables.
    //
    char                  *nodeLabelsCache;
    bool                   nodeLabelsCached;
    int                    nodeLabelsCacheSize;
    //
    // Visible point lookup
    //
    bool                    visiblePoint[256];
};


typedef ref_ptr<avtLabelRenderer> avtLabelRenderer_p;


#endif


