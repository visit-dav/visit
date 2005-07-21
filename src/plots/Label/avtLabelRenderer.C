// ************************************************************************* //
//                             avtLabelRenderer.C                            //
// ************************************************************************* //

#include "avtLabelRenderer.h"

#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkGeometryFilter.h>
#include <vtkIntArray.h>

#include <vtkMath.h>
#include <vtkMatrix4x4.h>

#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkUnsignedIntArray.h>

#include <avtCallback.h>
#include <avtVector.h>

#include <snprintf.h>

#include <DebugStream.h>
#include <TimingsManager.h>


// ****************************************************************************
// Constructor:  avtLabelRenderer::avtLabelRenderer
//
// Programmer:  Brad Whitlock
// Creation:    October 1, 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis, and we no longer
//    keep track of whether the cache is invalidated independent of whether
//    it was deallocated.
//
//    Hank Childs, Thu Jul 21 14:45:04 PDT 2005
//    Initialize MAX_LABEL_SIZE.
//
// ****************************************************************************

avtLabelRenderer::avtLabelRenderer() : avtCustomRenderer(), singleCellInfo(),
    singleNodeInfo()
{
    input = 0;
    varname = 0;
    numXBins = 10;
    numYBins = 10;
    labelBins = 0;
    singleCellIndex = -1;
    singleNodeIndex = -1;
    maxLabelLength = 0;
    MAX_LABEL_SIZE = 36;

    fgColor[0] = 1.;
    fgColor[1] = 1.;
    fgColor[2] = 1.;
    fgColor[3] = 1.;

    treatAsASCII = false;
    renderLabels3D = false;

    spatialExtents[0] = 0.;
    spatialExtents[1] = 0.;
    spatialExtents[2] = 0.;
    spatialExtents[3] = 0.;
    spatialExtents[4] = 0.;
    spatialExtents[5] = 0.;

    for(int i = 0; i < 256; ++i)
        visiblePoint[i] = false;
}

// ****************************************************************************
// Destructor:  avtLabelRenderer::~avtLabelRenderer
//
// Programmer:  Brad Whitlock
// Creation:    October 1, 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 16:04:38 PST 2004
//   Changed the call to clear the label caches.
//
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis, and we no longer
//    keep track of whether the cache is invalidated independent of whether
//    it was deallocated.
//
// ****************************************************************************

avtLabelRenderer::~avtLabelRenderer()
{
    for (std::map<vtkDataSet*,vtkPolyData*>::iterator it = inputMap.begin();
         it != inputMap.end();
         it++)
    {
        it->second->Delete();
    }
    inputMap.clear();

    if(labelBins != 0)
        delete [] labelBins;

    if(varname != 0)
        delete [] varname;

    ClearLabelCaches();
}

// ****************************************************************************
// Method: avtLabelRenderer::SetForegroundColor
//
// Purpose: 
//   Sets the foreground color to be used by the labels if they are being
//   drawn in the foreground color.
//
// Arguments:
//   fg : The foreground color.
//
// Returns:    True if the color was set; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:55:14 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
avtLabelRenderer::SetForegroundColor(const double *fg)
{
    bool retVal = false;

    if (atts.GetUseForegroundTextColor())
    {
       if (fgColor[0] != fg[0] || fgColor[1] != fg[1] || fgColor[2] != fg[2])
       {
           retVal = true; 
       }
    }

    fgColor[0] = fg[0];
    fgColor[1] = fg[1];
    fgColor[2] = fg[2];

    return retVal;
}

// ****************************************************************************
// Method:  avtLabelRenderer::Render
//
// Purpose:
//   Set up things necessary to call the renderer implentation.  Make a new
//   implementation object if things have changed.
//
// Arguments:
//   ds         the dataset to render
//
// Programmer:  Brad Whitlock
// Creation:    Thu Jan 22 11:16:18 PDT 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis.  Also, added a call
//    to free the geometry filter when we were done with it.
//
//    Hank Childs, Thu Jul 21 16:40:13 PDT 2005
//    Initialize MAX_LABEL_SIZE based on variable type.
//
// ****************************************************************************

void
avtLabelRenderer::Render(vtkDataSet *ds)
{
    if (ds->GetNumberOfPoints() == 0)
    {
        debug1 << "avtLabelRenderer::Render: The input dataset had no points!" << endl;
        return;
    }

    //
    // For efficiency: if input type is vtkPolyData, there's no 
    // need to pass it through the geometry filter.
    //

    if (!inputMap[ds])
    {
        // we haven't seen this input yet
        if (ds->GetDataObjectType() != VTK_POLY_DATA) 
        {
            vtkGeometryFilter *gf = vtkGeometryFilter::New();
            gf->SetInput(ds);
            input = vtkPolyData::New();
            gf->SetOutput(input);
            gf->Update();
            gf->Delete();
        }
        else 
        {
            input = (vtkPolyData *)ds;
            input->Register(NULL);
        }

        inputMap[ds] = input;
    }
    else
    {
        // This one is a re-run, so it's safe to re-use whatever
        // polydata we decided to use for it last time.
        input = inputMap[ds];
    }

    // 
    // Create an appropriate MAX_LABEL_SIZE for the data.
    //
    vtkDataArray *pointData = input->GetPointData()->GetArray(varname);
    vtkDataArray *cellData = input->GetCellData()->GetArray(varname);
    int biggest = 0;
    if (pointData)
        biggest = (biggest > pointData->GetNumberOfComponents() ? biggest
                                       : pointData->GetNumberOfComponents());
    if (cellData)
        biggest = (biggest > cellData->GetNumberOfComponents() ? biggest
                                       : cellData->GetNumberOfComponents());
    // Check for Mesh plot
    if (biggest == 0)
        biggest = 3;
    MAX_LABEL_SIZE = 12*biggest;

    //
    // Render the labels using the derived type's RenderLabels method.
    //
    RenderLabels();
}

// ****************************************************************************
// Method: avtLabelRenderer::ReleaseGraphicsResources
//
// Purpose: 
//   This method is called when the renderer must release its graphical
//   resources.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 16:05:29 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::ReleaseGraphicsResources()
{
    ClearLabelCaches();
}

// ****************************************************************************
// Method: avtLabelRenderer::ResetLabelBins
//
// Purpose: 
//   Resets the label bins used by the 3D binning algorithm so all of the
//   bins are empty.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:56:11 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::ResetLabelBins()
{
    int n = numXBins * numYBins;
    if(labelBins == 0)
        labelBins = new LabelInfo[n];

    for(int i = 0; i < n; ++i)
        labelBins[i].label = 0;
}

// ****************************************************************************
// Method: avtLabelRenderer::SetVariable
//
// Purpose: 
//   Sets the name of the variable that is being rendered.
//
// Arguments:
//   name : The name of the plotted variable.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:56:56 PDT 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:19:21 PST 2004
//    The caching code changed a bit, so I decided to change the
//    invalidation to actually deallocate the memory as well.  It was
//    not a big price to pay, and the code winds up a bit simpler.
//   
// ****************************************************************************

void
avtLabelRenderer::SetVariable(const char *name)
{
    //
    // If the variable is different then clear the flags indicating that
    // we have valid cached node and cell labels.
    //
    if(varname != 0 && name != 0 && strcmp(name, varname) != 0)
    {
        ClearLabelCaches();
    }

    // Store the new variable name.
    delete [] varname;
    varname = new char[strlen(name) + 1];
    strcpy(varname, name);
}

// ****************************************************************************
// Method: avtLabelRenderer::SetTreatAsASCII
//
// Purpose: 
//   Causes the renderer to draw character data as characters.
//
// Arguments:
//   val : Whether to draw char data as characters.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:59:18 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::SetTreatAsASCII(bool val)
{
    treatAsASCII = val;
}

// ****************************************************************************
// Method: avtLabelRenderer::Set3D
//
// Purpose: 
//   Tells the renderer whether to draw the labels as 3D.
//
// Arguments:  
//   val : Whether to draw in 3D.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 08:59:55 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::Set3D(bool val)
{
    renderLabels3D = val;
}

// ****************************************************************************
// Method: avtLabelRenderer::SetExtents
//
// Purpose: 
//   Sets the spatial extents into the renderer.
//
// Arguments:
//   ext : The spatial extents.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:00:32 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtLabelRenderer::SetExtents(const float *ext)
{
    spatialExtents[0] = ext[0];
    spatialExtents[1] = ext[1];
    spatialExtents[2] = ext[2];
    spatialExtents[3] = ext[3];
    spatialExtents[4] = ext[4];
    spatialExtents[5] = ext[5];
}

// ****************************************************************************
// Method: avtLabelRenderer::GetCellCenterArray
//
// Purpose: 
//   Returns a pointer to the cell center array.
//
// Returns:    A pointer to the cell center array that the label filter 
//             calculated.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:00:58 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

vtkFloatArray *
avtLabelRenderer::GetCellCenterArray()
{
    vtkFloatArray *cellCenters = 0;
    //
    // Look for the cell center array that the label filter calculated.
    //
    vtkDataArray *data = input->GetCellData()->GetArray("LabelFilterCellCenters");
    if(data == 0)
    {
        debug3 << "The avtLabelRenderer was not able to find the LabelFilterCellCenters array!" << endl;
    }
    else if(!data->IsA("vtkFloatArray"))
    {
        debug3 << "The avtLabelRenderer found the LabelFilterCellCenters array but it was not a vtkFloatArray.!" << endl;
    }
    else
        cellCenters = (vtkFloatArray *)data;

    return cellCenters;
}

// ****************************************************************************
// Method: avtLabelRenderer::SetupSingleCellLabel
//
// Purpose: 
//   Sets up the label that will be used when drawing a single cell.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:01:34 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr 13 13:08:36 PST 2005
//   I removed a check that prevented the single cell label from showing up
//   when a mesh is sliced.
//
// ****************************************************************************

void
avtLabelRenderer::SetupSingleCellLabel()
{
    //
    // Figure out where the single cell, if specified, should be plotted.
    //
    vtkIdType nCells = input->GetNumberOfCells();
    if(singleCellIndex >= 0)
    {
        //
        // Look for the cell center array that the label filter calculated.
        //
        vtkDataArray *data = input->GetCellData()->GetArray("LabelFilterCellCenters");
        if(data == 0)
        {
            debug3 << "The avtLabelRenderer was not able to find the LabelFilterCellCenters array!" << endl;
            return;
        }
        else if(!data->IsA("vtkFloatArray"))
        {
            debug3 << "The avtLabelRenderer found the LabelFilterCellCenters array but it was not a vtkFloatArray.!" << endl;
            return;
        }
        vtkFloatArray *cellCenters = (vtkFloatArray *)data;

        //
        // Look for the original cell number array.
        //
        data = input->GetCellData()->GetArray("LabelFilterOriginalCellNumbers");
        unsigned int realCellIndex = singleCellIndex;
        if(data != 0 && data->IsA("vtkUnsignedIntArray"))
        {
            vtkUnsignedIntArray *originalCells = (vtkUnsignedIntArray *)data;
            // Look through the original cell indices for the real index to use
            // when getting the cell center.
            for(vtkIdType cellid = 0; cellid < nCells; ++cellid)
            {
                unsigned int realCellId = originalCells->GetValue(cellid);
                if(realCellId == realCellIndex)
                {
                    realCellIndex = (unsigned int)cellid;
                    break;
                }
            }
        }

        //
        // Figure out where the point projects on the screen.
        //
        if(realCellIndex < nCells)
        {
            float *vert = cellCenters->GetTuple3(realCellIndex);
            singleCellInfo.screenPoint[0] = vert[0];
            singleCellInfo.screenPoint[1] = vert[1];
            singleCellInfo.screenPoint[2] = vert[2];
            singleCellInfo.label = atts.GetTextLabel().c_str();
        }
        else
        {
            debug4 << "realCellIndex=" << realCellIndex
                   << " but it must be smaller than nCells=" << nCells
                   << endl;
        }
    }
}

// ****************************************************************************
// Method: avtLabelRenderer::SetupSingleNodeLabel
//
// Purpose: 
//   Figures out the single node label.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:02:31 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Apr 13 13:07:31 PST 2005
//   Removed a check that prevented the single node label from showing up
//   when a mesh is sliced.
//
// ****************************************************************************

void
avtLabelRenderer::SetupSingleNodeLabel()
{
    //
    // Figure out where the single node, if specified, should be plotted.
    //
    vtkPoints *p = input->GetPoints();
    vtkIdType npts = p ? p->GetNumberOfPoints() : 0;
    if(singleNodeIndex >= 0)
    {
        //
        // Look for the original cell number array.
        //
        vtkDataArray *data = input->GetPointData()->GetArray("LabelFilterOriginalNodeNumbers");
        int realNodeIndex = singleNodeIndex;
        if(data != 0 && data->IsA("vtkUnsignedIntArray"))
        {
            vtkUnsignedIntArray *originalNodes = (vtkUnsignedIntArray *)data;

            // Look through the original cell indices for the real index to use
            // when getting the cell center.
            for(int i = 0; i < npts; ++i)
            {
                int realNodeId(originalNodes->GetValue(i));
                if(realNodeId == realNodeIndex)
                {
                    realNodeIndex = i;
                    break;
                }
            }
        }

        //
        // Figure out where the point projects on the screen.
        //
        if(realNodeIndex < npts)
        {           
            const float *vert = p->GetPoint(realNodeIndex);
            singleNodeInfo.screenPoint[0] = vert[0];
            singleNodeInfo.screenPoint[1] = vert[1];
            singleNodeInfo.screenPoint[2] = vert[2];
            singleNodeInfo.label = atts.GetTextLabel().c_str();
        }
        else
        {
            debug4 << "realNodeIndex=" << realNodeIndex 
                   << " but it must be smaller than npts=" << npts << endl;
        }
    }
}

// ****************************************************************************
// Method: avtLabelRenderer::CreateCachedCellLabels
//
// Purpose: 
//   Creates a cache of cell labels
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 5 14:45:59 PST 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis, and we no longer
//    keep track of whether the cache is invalidated independent of whether
//    it was deallocated.
//   
//    Hank Childs, Thu Jul 21 09:33:42 PDT 2005
//    Modify BEGIN/CREATE/END _LABEL macros to accomodate multi-step label
//    creation (necessary for arrays).
//
// ****************************************************************************

#define BEGIN_LABEL  int L = 0;
#define CREATE_LABEL L += SNPRINTF
#define END_LABEL    L = (L == -1) ? MAX_LABEL_SIZE : L; \
                     maxLabelLength = (L > maxLabelLength) ? L : maxLabelLength; \
                     labelString += MAX_LABEL_SIZE;

void
avtLabelRenderer::CreateCachedCellLabels()
{
    char *&cellLabelsCache = cellLabelsCacheMap[input];

    // Resize the cell labels cache.
    if (cellLabelsCache == NULL)
    {
        int cellLabelsCacheSize = input->GetNumberOfCells();
        cellLabelsCacheSizeMap[input] = cellLabelsCacheSize;
        cellLabelsCache = new char[MAX_LABEL_SIZE * cellLabelsCacheSize + 1];

        //
        // Include the method body with BEGIN_LABEL, END_LABEL macros defined
        // such that we immediately draw the labels without first transforming
        // them.
        //
        char *labelString = cellLabelsCache;
#include <CellLabels_body.C>
    }
}
#undef BEGIN_LABEL
#undef END_LABEL

// ****************************************************************************
// Method: avtLabelRenderer::CreateCachedNodeLabels
//
// Purpose: 
//   Creates a cache of all node labels.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 5 14:49:59 PST 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis, and we no longer
//    keep track of whether the cache is invalidated independent of whether
//    it was deallocated.
//
//    Hank Childs, Thu Jul 21 09:33:42 PDT 2005
//    Modify BEGIN/CREATE/END _LABEL macros to accomodate multi-step label
//    creation (necessary for arrays).
//
// ****************************************************************************

#define BEGIN_LABEL  int L = 0;
#define CREATE_LABEL L += SNPRINTF
#define END_LABEL    L = (L == -1) ? MAX_LABEL_SIZE : L; \
                     maxLabelLength = (L > maxLabelLength) ? L : maxLabelLength; \
                     labelString += MAX_LABEL_SIZE;
void
avtLabelRenderer::CreateCachedNodeLabels()
{
    char *&nodeLabelsCache = nodeLabelsCacheMap[input];

    // Resize the node labels cache.
    if(nodeLabelsCache == NULL)
    {
        int nodeLabelsCacheSize = input->GetPoints()->GetNumberOfPoints();
        nodeLabelsCacheSizeMap[input] = nodeLabelsCacheSize;
        nodeLabelsCache = new char[MAX_LABEL_SIZE * nodeLabelsCacheSize + 1];

        //
        // Include the method body with BEGIN_LABEL, END_LABEL macros defined
        // such that we immediately draw the labels without first transforming
        // them.
        //
        char *labelString = nodeLabelsCache;
#include <NodeLabels_body.C>
    }
}
#undef BEGIN_LABEL
#undef END_LABEL
#undef CREATE_LABEL

// ****************************************************************************
// Method: avtLabelRenderer::ClearLabelCaches
//
// Purpose: 
//   Clear out the label caches.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 16:03:33 PST 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:20:43 PST 2004
//    Caching is now done on a per-vtk-dataset basis, and we no longer
//    keep track of whether the cache is invalidated independent of whether
//    it was deallocated.
//   
// ****************************************************************************

void
avtLabelRenderer::ClearLabelCaches()
{
    //
    // Delete the label caches.
    //
    maxLabelLength = 0;

    std::map<vtkPolyData*,char*>::iterator it;
    for (it=cellLabelsCacheMap.begin(); it != cellLabelsCacheMap.end(); it++)
    {
        delete[] it->second;
    }
    cellLabelsCacheMap.clear();

    for (it=nodeLabelsCacheMap.begin(); it != nodeLabelsCacheMap.end(); it++)
    {
        delete[] it->second;
    }
    nodeLabelsCacheMap.clear();
}

// ****************************************************************************
// Method: avtLabelRenderer::AllowLabelInBin
//
// Purpose: 
//   This method is used in 3D label binning and it determines whether a given
//   label should be allowed in a bin.
//
// Arguments:
//   screenPoint : The label's location in normalized display space [0,1], [0,1].
//   labelString : A pointer to the label being considered.
//
// Returns:    True if the label is allowed to be in the cell.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:03:34 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
avtLabelRenderer::AllowLabelInBin(const float *screenPoint, const char *labelString)
{
    bool retval = false;

    //
    // If the point is on the screen then consider it for binning.
    //
    int binx = int(screenPoint[0] * numXBins);
    int biny = int(screenPoint[1] * numYBins);

    if(binx >= 0 && binx < numXBins &&
       biny >= 0 && biny < numYBins)
    {
        int index = biny * numXBins + binx;
        LabelInfo *info = labelBins + index;

        //
        // The point is on the screen but is it closer than the point that is
        // already in the bin?
        //
        if(screenPoint[2] < info->screenPoint[2] || info->label == 0)
        {
            info->screenPoint[0] = screenPoint[0];
            info->screenPoint[1] = screenPoint[1];
            info->screenPoint[2] = screenPoint[2];
            retval = true;
            info->label = labelString;
        }
    }
    else
    {
        debug5 << "BAD binx or biny. binx=" << binx << ", biny=" << biny << endl;
    }

    return retval;
}

// ****************************************************************************
// Method: avtLabelRenderer::WorldToDisplayMatrix
//
// Purpose: 
//   Creates a VTK matrix that can be used to calculate the label points
//   in normalized display space
//
// Returns:    The transformation matrix.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:04:55 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

vtkMatrix4x4 *
avtLabelRenderer::WorldToDisplayMatrix() const
{
     // Get world->view matrix
     vtkMatrix4x4 *M1 = vtkMatrix4x4::New();
     M1->DeepCopy(VTKRen->GetActiveCamera()->GetCompositePerspectiveTransformMatrix(1,0,1));

     // Set up view->display matrix
     vtkMatrix4x4 *M2 = vtkMatrix4x4::New();
     {
         float *v = VTKRen->GetViewport();
         float *a = VTKRen->GetAspect();

         M2->Identity();
         M2->Element[0][0] = (v[2]-v[0])/(2. * a[0]);
         M2->Element[1][1] = (v[3]-v[1])/(2. * a[1]);
         M2->Element[0][3] = (v[2]+v[0])/2.;
         M2->Element[1][3] = (v[3]+v[1])/2.;
     }

     // Compose world->display matrix
     vtkMatrix4x4 *M3 = vtkMatrix4x4::New();
     vtkMatrix4x4::Multiply4x4(M2,M1,M3);

     M1->Delete();
     M2->Delete();

     return M3;
}

// ****************************************************************************
// Method: avtLabelRenderer::TransformPoints
//
// Purpose: 
//   Transforms an array of points from world space to normalized display
//   space and returns a pointer to the transformed points.
//
// Arguments:
//   inputPoints            : The input points.
//   quantizedNormalIndices : Quantized normal indices so we know which
//                            points are going to be visible. This way,
//                            we don't transform points that won't be visible
//                            anyway.
//   nPoints                : The number of points to transform.
//
// Returns:    An array containing the transformed points.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:05:57 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

float *
avtLabelRenderer::TransformPoints(const float *inputPoints,
    const unsigned char *quantizedNormalIndices, int nPoints)
{
     float *xformedpts = new float[3 * nPoints];

     vtkMatrix4x4 *M = WorldToDisplayMatrix();

     float *destPoints = xformedpts;
     const float *pts = inputPoints;
     if(quantizedNormalIndices == 0)
     {
         //
         // We don't have information about the point to see whether it is
         // visible so we transform all points.
         //
         for (int j = 0; j < nPoints; ++j)
         {
             float p1[4] = {0,0,0,1}; // set homogenous to 1.0
             p1[0] = *pts++;
             p1[1] = *pts++;
             p1[2] = *pts++;
             float p2[4];

             M->MultiplyPoint(p1, p2);
             if (p2[3] != 0)
             {
                 *destPoints++ = p2[0]/p2[3];
                 *destPoints++ = p2[1]/p2[3];
                 *destPoints++ = p2[2]/p2[3];
             }
         }
     }
     else
     {
         // Here we have a bunch of quantized normals and if the normal
         // points away from the camera then we won't transform the point
         // since it would be wasteful.
         for (int j = 0; j < nPoints; ++j)
         {
             if(visiblePoint[quantizedNormalIndices[j]])
             {
                 float p1[4] = {0,0,0,1}; // set homogenous to 1.0
                 p1[0] = *pts++;
                 p1[1] = *pts++;
                 p1[2] = *pts++;
                 float p2[4];

                 M->MultiplyPoint(p1, p2);
                 if (p2[3] != 0)
                 {
                     *destPoints++ = p2[0]/p2[3];
                     *destPoints++ = p2[1]/p2[3];
                     *destPoints++ = p2[2]/p2[3];
                 }
             }
             else
             {
                 // Store the origin for the point
                 *destPoints++ = 0.f;
                 *destPoints++ = 0.f;
                 *destPoints++ = 0.f;
                 pts += 3;
             }
         }
     }

     M->Delete();

     return xformedpts;
}

// ****************************************************************************
// Method: avtLabelRenderer::PopulateBinsWithNodeLabels3D
//
// Purpose: 
//   Adds node labels to the 3D label bins.
//
// Note:       The transformed points are stored in the bins.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:08:35 PDT 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:25:19 PST 2004
//    Caching is now on a per-dataset basis.
//
// ****************************************************************************

void
avtLabelRenderer::PopulateBinsWithNodeLabels3D()
{
    vtkPoints *inputPoints = input->GetPoints();

    if(!inputPoints->GetData()->IsA("vtkFloatArray"))
    {
        debug4 << "The points array is not vtkFloatArray!" << endl;
        return;
    }

    int total = visitTimer->StartTimer();

    //
    // Get the dataset's input points.
    //
    vtkFloatArray *fa = (vtkFloatArray *)inputPoints->GetData();
    const float *pts = (const float *)fa->GetVoidPointer(0);

    //
    // See if the dataset has quantized node normals. If so, use them to
    // do backface culling on the labels that are facing away from the
    // camera.
    //
    const unsigned char *quantizedNormalIndices = 0;
    if(atts.GetDrawLabelsFacing() != LabelAttributes::FrontAndBack)
    {
        vtkUnsignedCharArray *qnna = (vtkUnsignedCharArray *)input->
            GetPointData()->GetArray("LabelFilterQuantizedNodeNormals");
        quantizedNormalIndices = (qnna != 0) ?
            (const unsigned char *)qnna->GetVoidPointer(0): 0;
    }

    //
    // Transform the points that face the camera.
    //
    int stageTimer = visitTimer->StartTimer();
    float *xformedPoints = TransformPoints(pts, quantizedNormalIndices,
        inputPoints->GetNumberOfPoints());
    visitTimer->StopTimer(stageTimer, "Transforming points");

    //
    // Here we use the label cache.
    //
    stageTimer = visitTimer->StartTimer();
    int n = fa->GetNumberOfTuples();
    float *transformedPoint = xformedPoints;
    const char *currentLabel = nodeLabelsCacheMap[input];
    if(quantizedNormalIndices != 0)
    {
        //
        // Here we only allow visible points in the bins.
        //
        for(int i = 0; i < n; ++i)
        {
            if(visiblePoint[quantizedNormalIndices[i]])
                AllowLabelInBin(transformedPoint, currentLabel);
            transformedPoint += 3;
            currentLabel += MAX_LABEL_SIZE;
        }
    }
    else
    {
        //
        // Here we only allow visible points in the bins.
        //
        for(int i = 0; i < n; ++i)
        {
            AllowLabelInBin(transformedPoint, currentLabel);
            transformedPoint += 3;
            currentLabel += MAX_LABEL_SIZE;
        }
    }
    visitTimer->StopTimer(stageTimer, "Binning the 3D node labels");

    delete [] xformedPoints;

    visitTimer->StopTimer(total, "PopulateBinsWithNodeLabels3D");
}

// ****************************************************************************
// Method: avtLabelRenderer::PopulateBinsWithCellLabels3D
//
// Purpose: 
//   Adds cell labels to the 3D label bins.
//
// Note:       The transformed points are stored in the bins.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:08:35 PDT 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:25:19 PST 2004
//    Caching is now on a per-dataset basis.
//
// ****************************************************************************

void
avtLabelRenderer::PopulateBinsWithCellLabels3D()
{
    //
    // Get the cell centers.
    //
    vtkFloatArray *cellCenters = GetCellCenterArray();
    if(cellCenters == 0)
        return;
    const float *pts = (const float *)cellCenters->GetVoidPointer(0);

    int total = visitTimer->StartTimer();

    //
    // See if the dataset has quantized node normals. If so, use them to
    // do backface culling on the labels that are facing away from the
    // camera.
    //
    const unsigned char *quantizedNormalIndices = 0;
    if(atts.GetDrawLabelsFacing() != LabelAttributes::FrontAndBack)
    {
        vtkUnsignedCharArray *qcna = (vtkUnsignedCharArray *)input->
            GetCellData()->GetArray("LabelFilterQuantizedCellNormals");
        quantizedNormalIndices = (qcna != 0) ?
            (const unsigned char *)qcna->GetVoidPointer(0): 0;
    }

    //
    // Transform the points that face the camera.
    //
    int stageTimer = visitTimer->StartTimer();
    float *xformedPoints = TransformPoints(pts, quantizedNormalIndices,
        input->GetNumberOfCells());
    visitTimer->StopTimer(stageTimer, "Transforming points");

    //
    // Here we use the label cache.
    //
    stageTimer = visitTimer->StartTimer();
    float *transformedPoint = xformedPoints;
    int n = cellCenters->GetNumberOfTuples();
    const char *currentLabel = cellLabelsCacheMap[input];
    if(quantizedNormalIndices != 0)
    {
        //
        // Here we only allow visible points in the bins.
        //
        for(int i = 0; i < n; ++i)
        {
            if(visiblePoint[quantizedNormalIndices[i]])
                AllowLabelInBin(transformedPoint, currentLabel);
            transformedPoint += 3;
            currentLabel += MAX_LABEL_SIZE;
        }
    }
    else
    {
        //
        // Here we only allow visible points in the bins.
        //
        for(int i = 0; i < n; ++i)
        {
            AllowLabelInBin(transformedPoint, currentLabel);
            transformedPoint += 3;
            currentLabel += MAX_LABEL_SIZE;
        }
    }
    visitTimer->StopTimer(stageTimer, "Binning the 3D cell labels");
  
    delete [] xformedPoints;

    visitTimer->StopTimer(total, "PopulateBinsWithCellLabels3D");
}

// ****************************************************************************
//  Method:  avtLabelRenderer::SetAtts
//
//  Purpose:
//    Set the attributes
//
//  Arguments:
//    a       : the new attributes
//
//  Programmer:  Brad Whitlock
//  Creation:    October 1, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Nov  8 17:19:21 PST 2004
//    The caching code changed a bit, so I decided to change the
//    invalidation to actually deallocate the memory as well.  It was
//    not a big price to pay, and the code winds up a bit simpler because
//    it is now just a single function call.
//
// ****************************************************************************
void
avtLabelRenderer::SetAtts(const AttributeGroup *a)
{
    const LabelAttributes *newAtts = (const LabelAttributes*)a;

    //
    // See if the label display format changed.
    //
    bool labelDisplayFormatChanged = atts.GetLabelDisplayFormat() !=
                                     newAtts->GetLabelDisplayFormat();

    atts = *newAtts;

    //
    // Resize the number of bins based on the number of labels that we want to display.
    //
    int nLabels = (atts.GetNumberOfLabels() < 1) ? 1 : atts.GetNumberOfLabels();
    int nBins = int(sqrt(double(nLabels)));
    if(nBins * nBins < nLabels)
        ++nBins;
    if(nBins * nBins != numXBins * numYBins)
    {
        delete [] labelBins;
        labelBins = new LabelInfo[nBins * nBins];
        numXBins = numYBins = nBins;
    }

    debug4 << "atts.GetSingleCellIndex() = " << atts.GetSingleCellIndex() << endl;
    debug4 << "singleCellIndex = " << singleCellIndex << endl;

    //
    // Save the single cell index so that can be drawn.
    //
    if(atts.GetShowSingleCell() && atts.GetSingleCellIndex() >= 0)
    {
        if(singleCellIndex < 0 || singleCellIndex != atts.GetSingleCellIndex())
            singleCellIndex = atts.GetSingleCellIndex();
    }
    else
    {
        singleCellInfo.label = 0;
        singleCellIndex = -1;
    }

    //
    // Save the single node index so that can be drawn.
    //
    if(atts.GetShowSingleNode() && atts.GetSingleNodeIndex() >= 0)
    {
        if(singleNodeIndex < 0 || singleNodeIndex != atts.GetSingleNodeIndex())
            singleNodeIndex = atts.GetSingleNodeIndex();
    }
    else
    {
        singleNodeInfo.label = 0;
        singleNodeIndex = -1;
    }

    if(singleCellInfo.label != 0 &&
       atts.GetTextLabel() != std::string(singleCellInfo.label))
    {
        singleCellInfo.label = 0;
        singleNodeInfo.label = 0;
    }

    //
    // If we're changing the display format then we have to clear the
    // label cache.
    //
    if(labelDisplayFormatChanged)
    {
        ClearLabelCaches();
    }
}

//
// LabelInfo constructor
//
avtLabelRenderer::LabelInfo::LabelInfo()
{
     screenPoint[0] = 0.f;
     screenPoint[1] = 0.f;
     screenPoint[2] = 0.f;
     label = 0;
}

avtLabelRenderer::LabelInfo::~LabelInfo()
{
}
