// ************************************************************************* //
//                             avtLabelRenderer.h                            //
// ************************************************************************* //

#ifndef AVT_VOLUME_RENDERER_H
#define AVT_VOLUME_RENDERER_H

#include <avtCustomRenderer.h>
#include <LabelAttributes.h>
#include <vtkSystemIncludes.h>
#include <map>
#include <string>

class vtkDataArray;
class vtkFloatArray;
class vtkMatrix4x4;
class vtkPoints;
class vtkPolyData;

#define RENDERER_ACTION_NOTHING       0
#define RENDERER_ACTION_INIT_ZBUFFER  1
#define RENDERER_ACTION_FREE_ZBUFFER  2

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
//    Jeremy Meredith, Mon Nov  8 17:29:27 PST 2004
//    When running with a parallel engine, multiple passes are made using
//    the same avtCustomRenderer.  This was wreaking havoc with the caching,
//    so I made it do caching on a per-vtkDataset basis.  This renderer is
//    freed when significant changes happen to the dataset, so there should
//    be no possibility for major leaks.
//
//    Hank Childs, Thu Jul 21 14:45:04 PDT 2005
//    Make MAX_LABEL_SIZE be a dynamic quantity based on the size of the
//    variable being rendered (ie scalar, vector, tensor, array, mesh).
//
//    Brad Whitlock, Tue Aug 2 15:24:36 PST 2005
//    I removed single cell/node stuff. I also added support for specifying
//    whether a binned label came from nodes or cells.
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
    void                    SetGlobalLabel(const std::string &L);
    void                    SetUseGlobalLabel(bool val);
    void                    SetRendererAction(int);

protected:
    struct LabelInfo
    {
        LabelInfo(); 
        ~LabelInfo();

        double screenPoint[3];
        int   type;
        const char *label;
    };

    virtual void RenderLabels() = 0;

    void CreateCachedCellLabels();
    void CreateCachedNodeLabels();
    void ClearLabelCaches();

    void ResetLabelBins();
    bool AllowLabelInBin(const float *vert, const char *labelString, int t);
    bool DepthTestPoint(float screenPoint[3]) const;

    vtkFloatArray *GetCellCenterArray();
    float *TransformPoints(const float *inputPoints,
                           const unsigned char *quantizedNormalIndices,
                           int nPoints);
    vtkMatrix4x4 *WorldToDisplayMatrix() const;

    LabelAttributes        atts;
    int                    MAX_LABEL_SIZE;

    std::map<vtkDataSet*,vtkPolyData*> inputMap;

    vtkPolyData           *input;
    char                  *varname;
    bool                   treatAsASCII;
    bool                   renderLabels3D;
    double                 fgColor[4];
    float                  spatialExtents[6];
    std::string            globalLabel;
    bool                   useGlobalLabel;
    int                    rendererAction;

    int                    numXBins, numYBins;
    LabelInfo             *labelBins;
    int                    maxLabelLength;
    int                    maxLabelRows;

    //
    // Cache the label strings for cell and node variables.
    //
    std::map<vtkPolyData*,char*> cellLabelsCacheMap;
    std::map<vtkPolyData*,int>   cellLabelsCacheSizeMap;
    std::map<vtkPolyData*,char*> nodeLabelsCacheMap;
    std::map<vtkPolyData*,int>   nodeLabelsCacheSizeMap;

    //
    // Visible point lookup
    //
    bool                    visiblePoint[256];
};


typedef ref_ptr<avtLabelRenderer> avtLabelRenderer_p;


#endif


