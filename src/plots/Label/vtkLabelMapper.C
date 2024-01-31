// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <vtkLabelMapper.h>


#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkUnsignedIntArray.h>

#include <DebugStream.h>
#include <TimingsManager.h>
#include <vtkVisItUtility.h>

#define ZBUFFER_USE_PROVIDED 0
#define ZBUFFER_QUERY        1
#define ZBUFFER_DONT_USE     2
#define ZBUFFER_QUERY_CUTOFF 100

//
// Include the vector quantization table
//
#include <quant_vector_lookup.C>

#ifndef vtkLabelMapper
#include <avtCallback.h>
bool vtkLabelMapper::zBufferWarningIssued = false;
#endif

static bool createCellLabels = false;
static bool createNodeLabels = false;
//
// Inline matrix operations
//
inline void
matrix_mul(double retval[4][4], const double a[4][4], const double b[4][4])
{
    for(int i = 0; i < 4; ++i)
    {
        retval[i][0] = a[i][0] * b[0][0] +
                       a[i][1] * b[1][0] +
                       a[i][2] * b[2][0] +
                       a[i][3] * b[3][0];
        retval[i][1] = a[i][0] * b[0][1] +
                       a[i][1] * b[1][1] +
                       a[i][2] * b[2][1] +
                       a[i][3] * b[3][1];
        retval[i][2] = a[i][0] * b[0][2] +
                       a[i][1] * b[1][2] +
                       a[i][2] * b[2][2] +
                       a[i][3] * b[3][2];
        retval[i][3] = a[i][0] * b[0][3] +
                       a[i][1] * b[1][3] +
                       a[i][2] * b[2][3] +
                       a[i][3] * b[3][3];
    }
}

inline void
matrix_mul_point(double out[4], const double M[4][4], const double pt[4])
{
    out[0] = pt[0]*M[0][0] + pt[1]*M[1][0] + pt[2]*M[2][0] + pt[3]*M[3][0];
    out[1] = pt[0]*M[0][1] + pt[1]*M[1][1] + pt[2]*M[2][1] + pt[3]*M[3][1];
    out[2] = pt[0]*M[0][2] + pt[1]*M[1][2] + pt[2]*M[2][2] + pt[3]*M[3][2];
    out[3] = pt[0]*M[0][3] + pt[1]*M[1][3] + pt[2]*M[2][3] + pt[3]*M[3][3];
}

//vtkStandardNewMacro(vtkLabelMapper);
#if 1
vtkLabelMapper *
vtkLabelMapper::New()
{
    vtkLabelMapper *result = new vtkLabelMapper;
    result->InitializeObjectBase();
    return result;
}
#endif

// ****************************************************************************
// Method: vtkLabelMapper::vtkLabelMapper
//
// Purpose: 
//   Constructor for the vtkLabelMapper class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:14:10 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Aug 8 17:35:11 PST 2005
//   Added zbuffer stuff.
//
// ****************************************************************************

vtkLabelMapper::vtkLabelMapper()  : vtkLabelMapperBase()
{
    zBuffer = 0;
    zBufferMode = ZBUFFER_DONT_USE;
    zBufferWidth = 0;
    zBufferHeight = 0;
    zTolerance = 0.;
}

// ****************************************************************************
// Method: vtkLabelMapper::~vtkLabelMapper
//
// Purpose: 
//   Destructor for the vtkLabelMapper class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:14:32 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 16:12:27 PST 2004
//   Changed the routine to release the display lists.
//
// ****************************************************************************

vtkLabelMapper::~vtkLabelMapper()
{
    ClearZBuffer();
}



// ****************************************************************************
// Method: vtkLabelMapper::ReleaseGraphicsResources
//
// Purpose: 
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 16:00:15 PST 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 16:10:03 PST 2004
//   I made it clear the label cache and release the graphical resources.
//
// ****************************************************************************

void
vtkLabelMapper::ReleaseGraphicsResources(vtkWindow *win)
{
    this->Superclass::ReleaseGraphicsResources(win);
    ClearZBuffer();
}


//--------------------------------------------------------------------------
void
vtkLabelMapper::BuildLabelsInternal(vtkDataSet *input, vtkRenderer *ren)
{
    //
    // Clear out old informaton
    //
    this->TextMappers.clear();
    this->LabelPositions.clear();
    // 
    // Create an appropriate this->MaxLabelSize for the data.
    //
    vtkDataArray *pointData = input->GetPointData()->GetArray(this->VarName.c_str());
    vtkDataArray *cellData = input->GetCellData()->GetArray(this->VarName.c_str());
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
    this->MaxLabelSize = 25*biggest;


    bool haveNodeData = pointData != NULL;
    bool haveCellData = cellData != NULL;
    createNodeLabels = false;
    createCellLabels = false;

    if (haveNodeData || haveCellData)
    {
        // The variable must have been a scalar or vector.
        if(haveNodeData)
        {
            CreateCachedNodeLabels(input);
            createNodeLabels = true;
        }

        if(haveCellData)
        {
            CreateCachedCellLabels(input);
            createCellLabels = true;
        }
    }
    else
    {
       bool notSubsetOrMaterial = 
            atts.GetVarType() != LabelAttributes::LABEL_VT_SUBSET &&
            atts.GetVarType() != LabelAttributes::LABEL_VT_MATERIAL;

        // The variable must have been a mesh
        if(notSubsetOrMaterial && atts.GetShowNodes())
        {
            CreateCachedNodeLabels(input);
            createNodeLabels = true;
        }

        if(!notSubsetOrMaterial || atts.GetShowCells())
        {
            CreateCachedCellLabels(input);
            createCellLabels = true;
        }

    }

    if (RenderLabels3D)
        DrawLabels3D(input, ren);
    else
        DrawLabels2D(input, ren);
}




// ****************************************************************************
// Method: vtkLabelMapper::DrawLabels2D
//
// Purpose: 
//   Draws the labels in 2D according to the options specified in the label plot.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:19:08 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 16:27:22 PST 2004
//   I made it use ren.
//
//   Brad Whitlock, Tue Aug 2 15:27:44 PST 2005
//   I removed the single cell/node stuff. I also moved some coding to set
//   the size into new methods so we can have individual sizes for node
//   and cell labels.
//
// ****************************************************************************

void
vtkLabelMapper::DrawLabels2D(vtkDataSet *input, vtkRenderer *ren)
{
    //
    // Now render the labels.
    //
    if(atts.GetRestrictNumberOfLabels())
    {
        DrawDynamicallySelectedLabels2D(input, ren);
    }
    else
    {
        DrawAllLabels2D(input);
    }
}


// ****************************************************************************
// Method: vtkLabelMapper::DrawAllLabels2D
//
// Notes:  Taken from avtLabelRenderer
//
// Purpose: 
//   Draws all of the 2D labels using the label cache.
//
// Arguments:
//   drawNodeLabels : Whether to draw node labels.
//   drawCellLables : Whether to draw cell labels.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:19:43 PDT 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:16:21 PST 2004
//    Caching is now done on a per-vtk-dataset basis.
//
//    Brad Whitlock, Thu Aug 4 10:24:48 PDT 2005
//    Added the ability to have different colors and sizes for node vs.
//    cell labels.
//
//    Alister Maguire, Mon May 24 10:06:23 PDT 2021
//    If we're in full frame mode, we need to perform some scaling.
//
// ****************************************************************************

void
vtkLabelMapper::DrawAllLabels2D(vtkDataSet *input)
{
    double positionScale[3];
    GetPositionScale(positionScale);

    //
    // Draw all the node labels.
    //
    size_t index = this->TextMappers.size();
    if(createNodeLabels)
    {
        vtkPoints *p = vtkVisItUtility::GetPoints(input);
        const char *labelPtr = this->NodeLabelsCache;
        for(int i = 0; i < this->NodeLabelsCacheSize; ++i, index++)
        {
            this->TextMappers.push_back(vtkSmartPointer<vtkTextMapper>::New());
            this->TextMappers[index]->SetTextProperty(this->NodeLabelProperty);
            this->TextMappers[index]->SetInput(labelPtr);
            labelPtr += this->MaxLabelSize;
            const double *vert = p->GetPoint(i);
            this->LabelPositions.push_back(vert[0] * positionScale[0]);
            this->LabelPositions.push_back(vert[1] * positionScale[1]);
            this->LabelPositions.push_back(vert[2] * positionScale[2]);
        }
        p->Delete();
    }

    //
    // Draw all the cell labels.
    //
    vtkDoubleArray *cellCenters = GetCellCenterArray(input);
    if(createCellLabels && cellCenters != 0)
    {
        const char *labelPtr = this->CellLabelsCache;
        for(int i = 0; i < this->CellLabelsCacheSize; ++i, index++)
        {
            this->TextMappers.push_back(vtkSmartPointer<vtkTextMapper>::New());
            this->TextMappers[index]->SetTextProperty(this->CellLabelProperty);
            this->TextMappers[index]->SetInput(labelPtr);
            labelPtr += this->MaxLabelSize;
            const double *vert = cellCenters->GetTuple3(i);
            this->LabelPositions.push_back(vert[0] * positionScale[0]);
            this->LabelPositions.push_back(vert[1] * positionScale[1]);
            this->LabelPositions.push_back(vert[2] * positionScale[2]);
        }
    }
}

// ****************************************************************************
// Method: vtkLabelMapper::DrawDynamicallySelectedLabels2D
//
// Notes:  Taken from avtLabelRenderer
//
// Purpose: 
//   Bins up 2D world space to make sure that only a subset of the 2D labels
//   are drawn.
//
// Arguments:
//   drawNodeLabels : Whether to draw node labels.
//   drawCellLables : Whether to draw cell labels.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:20:29 PDT 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:16:21 PST 2004
//    Caching is now done on a per-vtk-dataset basis.
//
//    Brad Whitlock, Wed Apr 13 12:01:21 PDT 2005
//    I fixed a problem with how the bins array was allocated.
//
//    Brad Whitlock, Thu Aug 4 10:24:48 PDT 2005
//    Added the ability to have different colors and sizes for node vs.
//    cell labels. I also changed the scaling for labels so the calculation of
//    bin sizes works better. Finally, I added some code for debugging that
//    lets us visualize the bins.
//
//    Brad Whitlock, Wed Nov 2 14:44:48 PST 2005
//    I changed a char array to bool and removed the use of memset to avoid
//    a problem where labels would not show up on versions built with xlC.
//
//    Brad Whitlock, Sat Apr 21 21:40:29 PDT 2012
//    Favor double instead of float.
//
//    Alister Maguire, Mon May 24 10:06:23 PDT 2021
//    If we're in full frame mode, we need to perform some scaling.
//
// ****************************************************************************

void
vtkLabelMapper::DrawDynamicallySelectedLabels2D(vtkDataSet *input,
    vtkRenderer *ren)
{
    double positionScale[3];
    GetPositionScale(positionScale);

    //
    // Figure out the world coordinates of the window that is being displayed.
    //
    double lowerleft[3] = {0., 0., 0.};
    ren->NormalizedViewportToView(lowerleft[0], lowerleft[1], lowerleft[2]);
    ren->ViewToWorld(lowerleft[0], lowerleft[1], lowerleft[2]);

    double upperright[3] = {1., 1., 0.};
    ren->NormalizedViewportToView(upperright[0], upperright[1], upperright[2]);
    ren->ViewToWorld(upperright[0], upperright[1], upperright[2]);

    //
    // figure out the size and aspect of the window in world coordinates.
    //
    double win_dx = upperright[0] - lowerleft[0];
    double win_dy = upperright[1] - lowerleft[1];
    double win_aspect = win_dx / win_dy;

    //
    // Find the target number of cells in each direction.  This
    // is a real number used as a basis to compute the integer number
    // of cells in each direction.  We handle only the 2d case here.
    // The cell aspect ratio is computed from the longest label.
    //
    const double char_aspect = 0.8;
    int LL = (maxLabelLength < 1) ? 1 : (maxLabelLength / maxLabelRows);
    double bin_aspect = LL * char_aspect * double(maxLabelRows);
    int NL = atts.GetNumberOfLabels();
    double nx_target = sqrt(NL * win_aspect / bin_aspect);
    double ny_target = sqrt(NL * bin_aspect / win_aspect);

    //
    // Compute the cell size in such a way as to have at least the target
    // number of cells in each direction but at the same time to be an
    // approximation so that cells don't change size too often when the
    // window is zoomed.
    //
    const double base = 2.;
    double bin_x_size, bin_y_size;
    double textScale = atts.GetTextFont1().GetScale() * 0.25;
    if (win_aspect >= 1.0)
    {
        // The X axis is the long axis of the view window.
        double power = (log (win_dx) - log (nx_target)) / log (base);
        bin_x_size = pow (base, floor (power));
        bin_y_size = (bin_x_size / bin_aspect);

        if(maxLabelRows > 1)
            bin_y_size *= double(2. * maxLabelRows*maxLabelRows);
    }
    else
    {
        // The Y axis is the long axis of the view window.
        double power = (log (win_dy) - log (ny_target)) / log (base);
        bin_y_size = pow (base, floor (power));
        bin_x_size = bin_y_size * bin_aspect;

        if(maxLabelRows > 1)
            bin_y_size *= double(2. * maxLabelRows*maxLabelRows);
    }

    bin_x_size *= textScale;
    bin_y_size *= textScale;

    //
    // Compute the offset to the first cell and the number of cells in
    // each dimension.
    //
    double minMeshX = this->SpatialExtents[0];
    double minMeshY = this->SpatialExtents[2];
    double bin_x_offset = floor ((lowerleft[0] - minMeshX) / bin_x_size) *
                    bin_x_size + minMeshX;
    double bin_y_offset = floor ((lowerleft[1] - minMeshY) / bin_y_size) *
                    bin_y_size + minMeshY;
    int bin_x_n = int(ceil (win_dx / bin_x_size)) + 1;
    int bin_y_n = int(ceil (win_dy / bin_y_size)) + 1;

    //
    // If we're using full frame scaling, we need to adjust the number of
    // cells in the Y direction. Scaling in the X direction is not needed
    // (I believe this is because VisIt is sneaky and only scales the Y
    // axis).
    //
    if (UseFullFrameScaling)
    {
        bin_y_n /= FullFrameScaling[1];
    }

    int bin_x_y = bin_x_n * bin_y_n;

    //
    // Create an array to record whether or not we've drawn a label
    // for a bin.
    //
    bool *bins = new bool[bin_x_y];
    for(int b = 0; b < bin_x_y; ++b)
        bins[b] = false;

    //
    // Iterate through the node labels and draw them if they fit into bins.
    //
    if(createNodeLabels)
    {
        char *labelPtr = this->NodeLabelsCache;
        vtkPoints *p = vtkVisItUtility::GetPoints(input);
        size_t index = this->TextMappers.size();
        for(int i = 0; i < this->NodeLabelsCacheSize; ++i, labelPtr += this->MaxLabelSize)
        {
            //
            // Which cell does the label fall within?
            //
            const double *labelVert = p->GetPoint(i);
            double label_x = labelVert[0];
            double label_y = labelVert[1];
            int bin_i = int((label_x - bin_x_offset) / bin_x_size);
            if (bin_i < 0 || bin_i >= bin_x_n)
                continue; /*out of bounds*/
            int bin_j = int((label_y - bin_y_offset) / bin_y_size);
            if (bin_j < 0 || bin_j >= bin_y_n)
                continue; /*out of bounds*/

            //
            // If this bin doesn't already have a label, then
            // add the label to that bin.
            //
            int binIndex = bin_j*bin_x_n+bin_i;
            if (bins[binIndex])
                continue; /*cell has label*/

            // Mark that the cell has a label
            bins[binIndex] = true;

            // Add this label.
            this->TextMappers.push_back(vtkSmartPointer<vtkTextMapper>::New());
            this->TextMappers[index]->SetTextProperty(this->NodeLabelProperty);
            this->TextMappers[index]->SetInput(labelPtr);
            index++;
            this->LabelPositions.push_back(labelVert[0] * positionScale[0]);
            this->LabelPositions.push_back(labelVert[1] * positionScale[1]);
            this->LabelPositions.push_back(labelVert[2] * positionScale[2]);
        }
        p->Delete();
    }

    //
    // Iterate through the cell labels and draw them if they fit into bins.
    //
    vtkDoubleArray *cellCenters = GetCellCenterArray(input);
    if(createCellLabels && cellCenters != 0)
    {
        const char *labelPtr = this->CellLabelsCache;
        size_t index = this->TextMappers.size();
        for(int i = 0; i < this->CellLabelsCacheSize; ++i, labelPtr += this->MaxLabelSize)
        {
            //
            // Which bin does the label fall within?
            //
            const double *labelVert = cellCenters->GetTuple3(i);
            double label_x = labelVert[0];
            double label_y = labelVert[1];
            int bin_i = int((label_x - bin_x_offset) / bin_x_size);
            if (bin_i < 0 || bin_i >= bin_x_n)
                continue; /*out of bounds*/
            int bin_j = int((label_y - bin_y_offset) / bin_y_size);
            if (bin_j < 0 || bin_j >= bin_y_n)
                continue; /*out of bounds*/

            //
            // If this bin doesn't already have a label, then
            // add the label to that bin.
            //
            int binIndex = bin_j*bin_x_n+bin_i;
            if (bins[binIndex])
                continue; /*cell has label*/

            // Mark that the cell has a label
            bins[binIndex] = true;

            // Add this label.
            this->TextMappers.push_back(vtkSmartPointer<vtkTextMapper>::New());
            this->TextMappers[index]->SetTextProperty(this->CellLabelProperty);
            this->TextMappers[index]->SetInput(labelPtr);
            index++;
            this->LabelPositions.push_back(labelVert[0] * positionScale[0]);
            this->LabelPositions.push_back(labelVert[1] * positionScale[1]);
            this->LabelPositions.push_back(labelVert[2] * positionScale[2]);
        }
    }

    delete [] bins;
}

// ****************************************************************************
// Method: vtkLabelMapper::TransformPoints
//
// Notes:  Taken from avtLabelRenderer
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
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:05:57 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Apr 25 10:20:49 PDT 2006
//   I changed the code so it uses an internal transformation matrix
//   (pointXForm) instead of using VTK matrices. This helps us to fix
//   the problem with point locations and zooming since our new matrix is
//   derived from information available in  instead of VTK.
//
//   Brad Whitlock, Fri Apr 20 15:27:39 PDT 2012
//   Make arg1 template.
//
// ****************************************************************************

template <typename T>
double *
vtkLabelMapper::TransformPoints(T inputPoints,
    const unsigned char *quantizedNormalIndices, int nPoints, vtkViewport *vp,
    double *realPoints)
{
     double *xformedpts = new double[3 * nPoints];

     double *destPoints = xformedpts;
     double *rp = realPoints;
     T pts = inputPoints;
     if(quantizedNormalIndices == 0)
     {
         //
         // We don't have information about the point to see whether it is
         // visible so we transform all points.
         //
         double p1[4] = {0.,0.,0.,1.}; // set homogenous to 1.0
         double p2[4] = {0.,0.,0.,1.};
         for (int j = 0; j < nPoints; ++j)
         {
             rp[0] = p1[0] = *pts++;
             rp[1] = p1[1] = *pts++;
             rp[2] = p1[2] = *pts++;

             matrix_mul_point(p2, pointXForm, p1);
             if (p2[3] != 0)
             {
                 *destPoints++ = (p2[0]/p2[3]);
                 *destPoints++ = (p2[1]/p2[3]);
                 *destPoints++ = (p2[2]/p2[3]);
             }
             rp+=3;
         }
     }
     else
     {
         // Here we have a bunch of quantized normals and if the normal
         // points away from the camera then we won't transform the point
         // since it would be wasteful.
         double p1[4] = {0.,0.,0.,1.}; // set homogenous to 1.0
         double p2[4] = {0.,0.,0.,1.};
         for (int j = 0; j < nPoints; ++j)
         {
             if(visiblePoint[quantizedNormalIndices[j]])
             {
                 rp[0] = p1[0] = *pts++;
                 rp[1] = p1[1] = *pts++;
                 rp[2] = p1[2] = *pts++;

                 matrix_mul_point(p2, pointXForm, p1);
                 if (p2[3] != 0)
                 {
                     *destPoints++ = (p2[0]/p2[3]);
                     *destPoints++ = (p2[1]/p2[3]);
                     *destPoints++ = (p2[2]/p2[3]);
                 }
             }
             else
             {
                 // Store the origin for the point
                 *destPoints++ = rp[0] = 0.;
                 *destPoints++ = rp[1] = 0.;
                 *destPoints++ = rp[2] = 0.;
                 pts += 3;
             }
             rp+=3;
         }
     }

     return xformedpts;
}

// ****************************************************************************
// Method: PopulateBinsHelper
//
// Notes:  Taken from avtLabelRenderer
//
// Purpose: 
//   Helps the PopulateBinsWithNodeLabels3D and PopulateBinsWithCellLabels3D
//   functions.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 8 09:47:56 PDT 2005
//
// Modifications:
//   Brad Whitlock, Thu Dec 13 15:46:37 PST 2007
//   Pass the type of the label in (whether it's for a node or zone).
//
//   Brad Whitlock, Sat Apr 21 21:32:21 PDT 2012
//   Pass points as doubles.
//
// ****************************************************************************

void
vtkLabelMapper::PopulateBinsHelper(vtkRenderer *ren, const unsigned char *
    quantizedNormalIndices, const char *currentLabel,
    const double *transformedPoint, vtkIdType n, int t,
    const double *realPoint)
{
    if(zBufferMode == ZBUFFER_USE_PROVIDED)
    {
        if(quantizedNormalIndices != 0)
        {
            //
            // Here we only allow visible points in the bins.
            //
            for(vtkIdType i = 0; i < n; ++i)
            {
                if(visiblePoint[quantizedNormalIndices[i]])
                {
                    int sx = int(double(zBufferWidth) * transformedPoint[0]);
                    int sy = int(double(zBufferHeight) * transformedPoint[1]);

                    if(sx >= 0 && sx < zBufferWidth &&
                       sy >= 0 && sy < zBufferHeight &&
                       transformedPoint[2] <= zBuffer[sy * zBufferWidth + sx]+zTolerance)
                    {
                        AllowLabelInBin(transformedPoint, currentLabel, t, realPoint);
                    }
                }
                transformedPoint += 3;
                realPoint += 3;
                currentLabel += this->MaxLabelSize;
            }
        }
        else
        {
            //
            // Here we only allow visible points in the bins.
            //
            for(vtkIdType i = 0; i < n; ++i)
            {
                int sx = int(double(zBufferWidth) * transformedPoint[0]);
                int sy = int(double(zBufferHeight) * transformedPoint[1]);
                if(sx >= 0 && sx < zBufferWidth &&
                   sy >= 0 && sy < zBufferHeight &&
                  transformedPoint[2] <= zBuffer[sy * zBufferWidth + sx]+zTolerance)
                {
                    AllowLabelInBin(transformedPoint, currentLabel, t, realPoint);
                }
                transformedPoint += 3;
                realPoint += 3;
                currentLabel += this->MaxLabelSize;
            }
        }
    }
    else if(zBufferMode == ZBUFFER_QUERY)
    {
        if(quantizedNormalIndices != 0)
        {
            //
            // Here we only allow visible points in the bins.
            //
            for(vtkIdType i = 0; i < n; ++i)
            {
                if(visiblePoint[quantizedNormalIndices[i]])
                {
                    int sx = int(double(zBufferWidth) * transformedPoint[0]);
                    int sy = int(double(zBufferHeight) * transformedPoint[1]);
                    if(sx >= 0 && sx < zBufferWidth &&
                       sy >= 0 && sy < zBufferHeight)
                    {
                        float Z = ren->GetZ(sx, sy);
                        if(transformedPoint[2] <= Z+zTolerance)
                            AllowLabelInBin(transformedPoint, currentLabel, t, realPoint);
                    }
                }
                transformedPoint += 3;
                realPoint += 3;
                currentLabel += this->MaxLabelSize;
            }
        }
        else
        {
            //
            // Here we only allow visible points in the bins.
            //
            for(vtkIdType i = 0; i < n; ++i)
            {
                int sx = int(double(zBufferWidth) * transformedPoint[0]);
                int sy = int(double(zBufferHeight) * transformedPoint[1]);
                if(sx >= 0 && sx < zBufferWidth &&
                   sy >= 0 && sy < zBufferHeight)
                {
                    float Z = ren->GetZ(sx,sy);
                    if(transformedPoint[2] <= Z+zTolerance)
                        AllowLabelInBin(transformedPoint, currentLabel, t, realPoint);
                }
                transformedPoint += 3;
                realPoint += 3;
                currentLabel += this->MaxLabelSize;
            }
        }
    }
    else
    {
        if(quantizedNormalIndices != 0)
        {
            //
            // Here we only allow visible points in the bins.
            //
            for(vtkIdType i = 0; i < n; ++i)
            {
                if(visiblePoint[quantizedNormalIndices[i]])
                    AllowLabelInBin(transformedPoint, currentLabel, t, realPoint);
                transformedPoint += 3;
                realPoint += 3;
                currentLabel += this->MaxLabelSize;
            }
        }
        else
        {
            //
            // Here we only allow visible points in the bins.
            //
            for(vtkIdType i = 0; i < n; ++i)
            {
                AllowLabelInBin(transformedPoint, currentLabel, t, realPoint);
                transformedPoint += 3;
                realPoint += 3;
                currentLabel += this->MaxLabelSize;
            }
        }
    }
}

// ****************************************************************************
// Method: vtkLabelMapper::PopulateBinsWithNodeLabels3D
//
// Notes:  Taken from avtLabelRenderer
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
//    Brad Whitlock, Thu Aug 4 11:22:46 PDT 2005
//    I changed the interface to AllowLabelInBin and moved some code into
//    PopulateBinsHelper.
//
//    Brad Whitlock, Thu Dec 13 15:47:55 PST 2007
//    Pass 0 to PopulateBinsHelper to indicate that we're inserting
//    node labels.
//
//    Brad Whitlock, Sat Apr 21 21:32:44 PDT 2012
//    Support double coordinates.
//
// ****************************************************************************

void
vtkLabelMapper::PopulateBinsWithNodeLabels3D(vtkDataSet *input, vtkRenderer *ren)
{
    const char *mName = "vtkLabelMapper::PopulateBinsWithNodeLabels3D: ";
    vtkPoints *inputPoints = vtkVisItUtility::GetPoints(input);

    int total = visitTimer->StartTimer();

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
    double *xformedPoints = NULL;
    double *realPoints = new double[inputPoints->GetNumberOfPoints()*3]; 
    if(inputPoints->GetDataType() == VTK_DOUBLE)
    {
        debug4 << mName << "TransformPoints with doubles" << endl;
        int stageTimer = visitTimer->StartTimer();
        xformedPoints = TransformPoints((const double *)inputPoints->GetVoidPointer(0),
                                        quantizedNormalIndices,
                                        inputPoints->GetNumberOfPoints(),
                                        ren,
                                        realPoints);
        visitTimer->StopTimer(stageTimer, "Transforming points");
    }
    else if(inputPoints->GetDataType() == VTK_FLOAT)
    {
        debug4 << mName << "TransformPoints with floats" << endl;
        int stageTimer = visitTimer->StartTimer();
        xformedPoints = TransformPoints((const float *)inputPoints->GetVoidPointer(0),
                                        quantizedNormalIndices,
                                        inputPoints->GetNumberOfPoints(),
                                        ren,
                                        realPoints);
        visitTimer->StopTimer(stageTimer, "Transforming points");
    }
    else
    {
        inputPoints->Delete();
        debug4 << mName << "TransformPoints not supported type" << endl;
        return;
    }

    //
    // Here we use the label cache.
    //
    int stageTimer = visitTimer->StartTimer();
    vtkIdType n = inputPoints->GetNumberOfPoints();
    double *transformedPoint = xformedPoints;
    double *realP = realPoints;
    const char *currentLabel = this->NodeLabelsCache;
    PopulateBinsHelper(ren, quantizedNormalIndices, currentLabel, 
                       transformedPoint, n, 0, realP);
    visitTimer->StopTimer(stageTimer, "Binning the 3D node labels");

    delete [] xformedPoints;
    delete [] realPoints;

    inputPoints->Delete();
    visitTimer->StopTimer(total, "PopulateBinsWithNodeLabels3D");
}

// ****************************************************************************
// Method: vtkLabelMapper::PopulateBinsWithCellLabels3D
//
// Notes:  Taken from avtLabelRenderer
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
//    Brad Whitlock, Thu Aug 4 11:22:46 PDT 2005
//    I changed the interface to AllowLabelInBin and moved some code into
//    PopulateBinsHelper.
//
//    Brad Whitlock, Thu Dec 13 15:47:55 PST 2007
//    Pass 1 to PopulateBinsHelper to indicate that we're inserting
//    cell labels.
//
//    Brad Whitlock, Sat Apr 21 21:37:47 PDT 2012
//    Change cell centers to double.
//
// ****************************************************************************

void
vtkLabelMapper::PopulateBinsWithCellLabels3D(vtkDataSet *input, vtkRenderer *ren)
{
    //
    // Get the cell centers.
    //
    vtkDoubleArray *cellCenters = GetCellCenterArray(input);
    if(cellCenters == 0)
    {
        return;
    }
    const double *pts = (const double *)cellCenters->GetVoidPointer(0);

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
    double *realPoints = new double[input->GetNumberOfCells() *3];
    double *xformedPoints = TransformPoints(pts, quantizedNormalIndices,
        input->GetNumberOfCells(), ren, realPoints);
    visitTimer->StopTimer(stageTimer, "Transforming points");

    //
    // Here we use the label cache.
    //
    stageTimer = visitTimer->StartTimer();
    double *transformedPoint = xformedPoints;
    double *realPoint = realPoints;
    vtkIdType n = cellCenters->GetNumberOfTuples();
    const char *currentLabel = this->CellLabelsCache;
    PopulateBinsHelper(ren, quantizedNormalIndices, currentLabel,
                       transformedPoint, n, 1, realPoint);
    visitTimer->StopTimer(stageTimer, "Binning the 3D cell labels");
  
    delete [] xformedPoints;
    delete [] realPoints;

    visitTimer->StopTimer(total, "PopulateBinsWithCellLabels3D");
}

// ****************************************************************************
// Method: vtkLabelMapper::DrawLabels3D
//
// Notes:  Taken from avtLabelRenderer
//
// Purpose: 
//   Draws the labels in 3D.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:21:16 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Nov 29 16:29:35 PST 2004
//   Changed visible point lookup a little.
//
//   Brad Whitlock, Tue Aug 2 15:28:22 PST 2005
//   I removed the single cell/node stuff. I also added the ability to have
//   different colors and sizes for node vs. cell labels. Finally, I added
//   optional z-buffering to the renderer.
//
//   Brad Whitlock, Tue Apr 25 10:13:42 PDT 2006
//   I added code to get the current modelview and projection matrices so we
//   can construct a point transformation matrix that we know will always be
//   correct, regardless of how VTK camera has been set.
//
//   Alister Maguire, Mon May 24 10:06:23 PDT 2021
//   If we're in full frame mode, we need to perform some scaling.
//
// ****************************************************************************

void
vtkLabelMapper::DrawLabels3D(vtkDataSet *input, vtkRenderer *ren)
{
    const char *mName = "vtkLabelMapper::DrawLabels3D: ";
    vtkDataArray *pointData = input->GetPointData()->GetArray(this->VarName.c_str());
    vtkDataArray *cellData = input->GetCellData()->GetArray(this->VarName.c_str());
    bool haveNodeData = pointData != 0;
    bool haveCellData = cellData != 0;

    int total = visitTimer->StartTimer();
    int stageTimer = visitTimer->StartTimer();

    bool notSubsetOrMaterial = 
         atts.GetVarType() != LabelAttributes::LABEL_VT_SUBSET &&
         atts.GetVarType() != LabelAttributes::LABEL_VT_MATERIAL;
    visitTimer->StopTimer(stageTimer, "Creating label caches");

    //
    // Determine which of the quantized vectors are visible given the
    // current camera.
    //
    stageTimer = visitTimer->StartTimer();
    vtkCamera *camera = ren->GetActiveCamera();
    const double *pos = camera->GetPosition();
    const double *focus = camera->GetFocalPoint();
    avtVector camvec(pos[0]-focus[0],pos[1]-focus[1],pos[2]-focus[2]);
    camvec.normalize();
    for(int j = 0; j < 256; ++j)
    {
        if(j < MAX_QUANT_VECTORS)
        {
            avtVector qvNormal((const float *)quant_vector_lookup[j]);
            double dot = camvec * qvNormal;
            if(atts.GetDrawLabelsFacing() == LabelAttributes::Front)
                visiblePoint[j] = (dot >= 0.);
            else if(atts.GetDrawLabelsFacing() == LabelAttributes::Back)
                visiblePoint[j] = (dot < 0.);
            else
                visiblePoint[j] = true;
        }
        else
            visiblePoint[j] = false;
    }
    visitTimer->StopTimer(stageTimer, "Determining visible vectors");

    //
    // Initialize the ZBuffer.
    //
    if((this->RendererAction & RENDERER_ACTION_INIT_ZBUFFER) != 0)
        InitializeZBuffer(input, ren, haveNodeData, haveCellData);

    //
    // Initialize the transformation matrix that we'll use to transform points
    // into normalized device space.
    //
    double modelview[4][4], projection[4][4], mtmp[4][4];
    vtkMatrix4x4 *mvtm = ren->GetActiveCamera()->GetModelViewTransformMatrix();
    vtkMatrix4x4 *ptm = ren->GetActiveCamera()->GetProjectionTransformMatrix(ren);
    if (mvtm)
    {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                modelview[i][j] = mvtm->GetElement(j, i);
                // VTK's modelview matrix seems inverted from what we used
                // to get from GL, that's why we reverse i and j here.
    }

    if (ptm)
    {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                projection[i][j] = ptm->GetElement(i, j);
    }
    const double tonormdev[4][4] = {{0.5,0,0,0},{0,0.5,0,0},{0,0,0.5,0},{0.5,0.5,0.5,1}};
    matrix_mul(mtmp, modelview, projection);
    matrix_mul(pointXForm, mtmp, tonormdev);


    if(atts.GetRestrictNumberOfLabels())
    {
        debug4 << mName << "Restricting number of labels" << endl;

        // 
        // Reset the label bins so we have to repopulate them with the labels,
        // taking into account the current view.
        //
        ResetLabelBins();

        //
        // Populate the label bins.
        //
        stageTimer = visitTimer->StartTimer();
        if(haveNodeData || haveCellData)
        {
            // The variable must have been a scalar, vector, tensor, label.
            if(haveNodeData)
            {
                PopulateBinsWithNodeLabels3D(input, ren);
            }

            if(haveCellData)
            {
                PopulateBinsWithCellLabels3D(input, ren);
            }
        }
        else
        {
            // The variable must have been a mesh, subset, or material.
            if(notSubsetOrMaterial && atts.GetShowNodes())
            {
                PopulateBinsWithNodeLabels3D(input, ren);
            }

            if(!notSubsetOrMaterial || atts.GetShowCells())
            {
                PopulateBinsWithCellLabels3D(input, ren);
            }
        }
        visitTimer->StopTimer(stageTimer, "Binning 3D labels");

        double positionScale[3];
        GetPositionScale(positionScale);

        //
        // Draw the labels that came from nodes.
        //
        stageTimer = visitTimer->StartTimer();
        int n = numXBins * numYBins;
        const LabelInfo *info = this->LabelBins;
        debug4 << mName << "Drawing labels" << endl;
        size_t index = this->TextMappers.size();
        for(int i = 0; i < n; ++i, ++info)
        {
            if(info->label == 0)
                continue;

            this->TextMappers.push_back(vtkSmartPointer<vtkTextMapper>::New());
            this->TextMappers[index]->SetInput(info->label);
            if (info->type == 1)
                this->TextMappers[index]->SetTextProperty(this->CellLabelProperty);
            else
                this->TextMappers[index]->SetTextProperty(this->NodeLabelProperty);
            index++;
            this->LabelPositions.push_back(info->realPoint[0] * positionScale[0]);
            this->LabelPositions.push_back(info->realPoint[1] * positionScale[1]);
            this->LabelPositions.push_back(info->realPoint[2] * positionScale[2]);
        }
        visitTimer->StopTimer(stageTimer, "Drawing binned 3D labels");
    }
    else
    {
        debug4 << mName << "Drawing all labels" << endl;

        //
        // Draw the labels without doing any kind of reduction.
        //
        stageTimer = visitTimer->StartTimer();
        if(haveNodeData || haveCellData)
        {
            // The variable must have been a scalar, vector, tensor, etc.
            if(haveNodeData)
            {
                debug4 << mName << "    Drawing all node labels" << endl;
                DrawAllNodeLabels3D(input, ren);
            }

            if(haveCellData)
            {
                debug4 << mName << "    Drawing all cell labels" << endl;
                DrawAllCellLabels3D(input, ren);
            }
        }
        else
        {
            // The variable must have been a mesh, subset, or material.
            if(notSubsetOrMaterial && atts.GetShowNodes())
            {
                debug4 << mName << "    Drawing all node# labels" << endl;
                DrawAllNodeLabels3D(input, ren);
            }

            if(!notSubsetOrMaterial || atts.GetShowCells())
            {
                debug4 << mName << "    Drawing all cell# labels" << endl;
                DrawAllCellLabels3D(input, ren);
            }
        }
        visitTimer->StopTimer(stageTimer, "Drawing all labels 3D");
    }

    // If the renderAction flag permits us to delete the zbuffer,
    // do it now.
    if((this->RendererAction & RENDERER_ACTION_FREE_ZBUFFER) != 0)
        ClearZBuffer();

    visitTimer->StopTimer(total, "vtkLabelMapper::DrawLabels3D");
}

// ****************************************************************************
// Method: vtkLabelMapper::ClearZBuffer
//
// Notes:  Taken from avtLabelRenderer
//
// Purpose: 
//   Deletes the allocated z-buffer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 9 09:52:35 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
vtkLabelMapper::ClearZBuffer()
{
    if(zBuffer != 0)
    {
        debug4 << "vtkLabelMapper::ClearZBuffer: Deleting z buffer\n";
        delete [] zBuffer;
        zBuffer = 0;
    }
}

// ****************************************************************************
// Method: vtkLabelMapper::InitializeZBuffer
//
// Notes:  Taken from avtLabelRenderer
//
// Purpose: 
//   Initializes the z-buffer array (if necessary).
//
// Arguments:
//   haveNodeData : True if we have nodal data.
//   haveCellData : True if we have cell data.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 9 09:52:56 PDT 2005
//
// Modifications:
//   Tom Fogal, Mon Aug  2 11:34:52 MDT 2010
//   Replaced Mesa compile-time check with a runtime check.
//
//   Brad Whitlock, Sat Apr 21 22:51:33 PDT 2012
//   Change the zTolerance calculation to match the updated method in the 
//   mesh renderer.
//
//   Eric Brugger, Wed Apr 10 14:03:26 PDT 2013
//   Changed the check against vtkMesaRenderWindow to vtkOSMesaGLRenderWindow
//   because of the change to VTK-6.
//
// ****************************************************************************

void
vtkLabelMapper::InitializeZBuffer(vtkDataSet *input, vtkRenderer *ren,
    bool haveNodeData, bool haveCellData)
{
    const char *mName = "vtkLabelMapper::InitializeZBuffer: ";
    //
    // Figure out if we need the zbuffer for anything. If we need it then
    // read it back into a buffer that we allocated.
    //
    zBufferMode = ZBUFFER_DONT_USE;
    zTolerance = 0.f;

    if(atts.GetDepthTestMode() != LabelAttributes::LABEL_DT_NEVER)
    {
        zBufferWidth  = ren->GetVTKWindow()->GetSize()[0];
        zBufferHeight = ren->GetVTKWindow()->GetSize()[1];
        if (ren->GetVTKWindow()->IsA("vtkOSMesaGLRenderWindow"))
        {
            // If we're using Mesa then let's just query since we already have
            // the zbuffer in memory.
            zBufferMode = ZBUFFER_QUERY;
            zBuffer = 0;
        }
        else
        {
            bool readZBuffer = false;
            if(atts.GetDepthTestMode() == LabelAttributes::LABEL_DT_ALWAYS)
            {
                readZBuffer = true;
            }
            else // LABEL_DT_AUTO
            {
                if(haveNodeData && haveCellData)
                {
                    if(input->GetNumberOfCells() + 
                       input->GetNumberOfPoints() < ZBUFFER_QUERY_CUTOFF)
                    {
                        zBufferMode = ZBUFFER_QUERY;
                    }
                }
                else if(haveNodeData)
                {
                    if(input->GetNumberOfPoints() < ZBUFFER_QUERY_CUTOFF)
                        zBufferMode = ZBUFFER_QUERY;
                }
                else if(haveCellData)
                {
                    if(input->GetNumberOfCells() < ZBUFFER_QUERY_CUTOFF)
                        zBufferMode = ZBUFFER_QUERY;
                }

                // If we're not going to try and query the zbuffer later then
                // read the whole thing now if we're direct.
                if(zBufferMode == ZBUFFER_DONT_USE)
                {
                    if(ren->GetVTKWindow()->IsA("vtkRenderWindow"))
                    {
                        vtkRenderWindow *renWin = (vtkRenderWindow*)ren->GetVTKWindow();
                        if(renWin->IsDirect())
                            readZBuffer = true;
                        else if(!zBufferWarningIssued)
                        {
                            zBufferWarningIssued = true;
                            avtCallback::IssueWarning("VisIt is not running on "
                                "a direct display so the z-buffer will not be "
                                "read back to aid in depth testing to "
                                "determine which labels should not be drawn. "
                                "If you want to enable depth testing, set the "
                                "Label plot's depth test flag to Always.");
                        }
                    }
                }
            }

            // Read the z-buffer.
            if(readZBuffer)
            {
                int getZ = visitTimer->StartTimer();
                // vtk's GetZbufferData use width+1 so ensure buffer
                // size is correct, may need to adjust something else as well?
                int zBufferSize = (zBufferWidth+1) * (zBufferHeight+1);

                debug4 << mName << "Allocated z-buffer" << endl;
                zBuffer = new float[zBufferSize];
                if(zBuffer != 0)
                {
                    ren->GetRenderWindow()->GetZbufferData(
                        0, 0, zBufferWidth, zBufferHeight, zBuffer);
                    zBufferMode = ZBUFFER_USE_PROVIDED;
                }
                visitTimer->StopTimer(getZ, "Reading back Z-buffer");
            }
        }

        double nearPlane;
        double farPlane;
        ren->GetActiveCamera()->GetClippingRange(nearPlane, farPlane);

        // compute a shift based upon total range in Z
        double zShift1 = (farPlane - nearPlane) / 1.0e+4;

        // compute a shift based upon distance between eye and near clip
        double zShift2 = nearPlane / 2.0;

        // use whatever shift is smaller
        double zShift = zShift1 < zShift2 ? zShift1 : zShift2;

        // Multiply it by a little so the labels win over the lines.
        zTolerance = float(zShift * 1.001);
    }

    if(zBufferMode == ZBUFFER_DONT_USE)
    {
        debug4 << mName << "zBufferMode = ZBUFFER_DONT_USE" << endl;
    }
    else if(zBufferMode == ZBUFFER_USE_PROVIDED)
    {
        debug4 << mName << "zBufferMode = ZBUFFER_USE_PROVIDED" << endl;
    }
    else if(zBufferMode == ZBUFFER_QUERY)
    {
        debug4 << mName << "zBufferMode = ZBUFFER_QUERY" << endl;
    }
}

// ****************************************************************************
// Method: vtkLabelMapper::GetPositionScale
//
// Purpose:
//     Get a scale to apply to our position. Currently, this will use
//     the full frame scaling if it's enabled.
//
// Arguments:
//     scale    A pointer to a 3D array. We will put the scale factor here.
//
// Programmer: Alister Maguire
// Creation:   May 20, 2021
//
// Modifications:
//
// ****************************************************************************

void
vtkLabelMapper::GetPositionScale(double *scale)
{
    if (UseFullFrameScaling)
    {
        for (int i = 0; i < 3; ++i)
        {
            scale[i] = FullFrameScaling[i];
        }
    }
    else
    {
        for (int i = 0; i < 3; ++i)
        {
            scale[i] = 1.;
        }
    }
}


// ****************************************************************************
// Method: vtkLabelMapper::DrawAllCellLabels3D
//
// Notes:  Taken from avtLabelRenderer
//
// Purpose: 
//   Draws transformed text in 3D.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 5 14:45:59 PST 2004
//
// Modifications:
//   Jeremy Meredith, Mon Nov  8 17:16:21 PST 2004
//   Caching is now done on a per-vtk-dataset basis.
//
//   Brad Whitlock, Fri Aug 5 14:13:02 PST 2005
//   Added coding to use the zbuffer to further restrict the labels that
//   get rendered.
//
//   Brad Whitlock, Tue Apr 25 10:26:22 PDT 2006
//   I made it use our own transformation matrix to transform the points.
//
//   Alister Maguire, Mon May 24 10:06:23 PDT 2021
//   If we're in full frame mode, we need to perform some scaling.
//
// ****************************************************************************


#define BEGIN_LABEL labelString = this->CellLabelsCache + this->MaxLabelSize*id; if(!cellLabelsCached) {
#define GET_THE_POINT const double *vert = cellCenters->GetTuple3(id);
#define TEXT_PROPERTY this->CellLabelProperty 

#define END_LABEL   } VISIBLE_POINT_PREDICATE\
    { \
    GET_THE_POINT \
    double v[4] = {vert[0], vert[1], vert[2], 1.f}; \
    double vprime[4]; \
    matrix_mul_point(vprime, pointXForm, v);\
    if (vprime[3] != 0.) \
    { \
    vprime[0] /= vprime[3]; \
    vprime[1] /= vprime[3]; \
    vprime[2] /= vprime[3]; \
    } \
    double positionScale[3]; \
    GetPositionScale(positionScale); \
    ZBUFFER_PREDICATE_START \
      this->TextMappers.push_back(vtkSmartPointer<vtkTextMapper>::New()); \
      this->TextMappers[index]->SetInput(labelString); \
      this->TextMappers[index]->SetTextProperty(TEXT_PROPERTY); \
      index++; \
      this->LabelPositions.push_back(vert[0] * positionScale[0]); \
      this->LabelPositions.push_back(vert[1] * positionScale[1]); \
      this->LabelPositions.push_back(vert[2] * positionScale[2]); \
    ZBUFFER_PREDICATE_END \
    }

void
vtkLabelMapper::DrawAllCellLabels3D(vtkDataSet *input, vtkRenderer *ren)
{
    char *labelString = 0;
    bool   cellLabelsCached    = this->CellLabelsCache != NULL;

    //
    // Look for the cell center array that the label filter calculated.
    //
    vtkDoubleArray *cellCenters = GetCellCenterArray(input);
    if(cellCenters == 0)
        return;

    // Resize the cell labels cache.
    if(this->CellLabelsCache == 0 || input->GetNumberOfCells() != this->CellLabelsCacheSize)
    {
        delete [] this->CellLabelsCache;
        this->CellLabelsCacheSize = input->GetNumberOfCells();
        this->CellLabelsCache = new char[this->MaxLabelSize * this->CellLabelsCacheSize + 1];
    }

    //
    // See if the dataset has quantized node normals. If so, use them to
    // do backface culling on the labels that are facing away from the
    // camera.
    //
    vtkUnsignedCharArray *qcna = (vtkUnsignedCharArray *)input->
        GetCellData()->GetArray("LabelFilterQuantizedCellNormals");
    const unsigned char *quantizedNormalIndices = (qcna != 0) ?
        (const unsigned char *)qcna->GetVoidPointer(0): 0;

    //
    // Include the method body with BEGIN_LABEL, END_LABEL macros defined
    // such that we immediately draw the labels without first transforming
    // them.
    //
    size_t index = this->TextMappers.size();
    if(zBufferMode == ZBUFFER_USE_PROVIDED)
    {
    //
    // Use the provided z buffer to determine help determine whether a label
    // should be plotted.
    //
#define ZBUFFER_PREDICATE_START \
                          double sx = vprime[0];\
                          double sy = vprime[1];\
                          ren->NormalizedDisplayToDisplay(sx, sy);\
                          int isx = int(sx);\
                          int isy = int(sy);\
                          if(isx >= 0 && isx < zBufferWidth &&\
                             isy >= 0 && isy < zBufferHeight &&\
                             vprime[2] <= zBuffer[isy*zBufferWidth+isx]+zTolerance)\
                          {
#define ZBUFFER_PREDICATE_END }
        if(quantizedNormalIndices == 0)
        {
#define VISIBLE_POINT_PREDICATE
#include <CellLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }
        else
        {
#define VISIBLE_POINT_PREDICATE if(visiblePoint[quantizedNormalIndices[id]])
#include <CellLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }
#undef ZBUFFER_PREDICATE_START
#undef ZBUFFER_PREDICATE_END
    }
    else if(zBufferMode == ZBUFFER_QUERY)
    {
    //
    // Query the zbuffer one pixel at a time to avoid having to read it
    // all at once.
    //
#define ZBUFFER_PREDICATE_START float Z = 0.;\
                          double sx = vprime[0];\
                          double sy = vprime[1];\
                          ren->NormalizedDisplayToDisplay(sx, sy);\
                          int isx = int(sx);\
                          int isy = int(sy);\
                          if(isx >= 0 && isx < zBufferWidth &&\
                             isy >= 0 && isy < zBufferHeight)\
                          {\
                              Z = ren->GetZ(isx, isy); \
                              if(vprime[2] <= Z+zTolerance)\
                              {
#define ZBUFFER_PREDICATE_END }}
        if(quantizedNormalIndices == 0)
        {
#define VISIBLE_POINT_PREDICATE
#include <CellLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }
        else
        {
#define VISIBLE_POINT_PREDICATE if(visiblePoint[quantizedNormalIndices[id]])
#include <CellLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }

#undef ZBUFFER_PREDICATE_START
#undef ZBUFFER_PREDICATE_END
    }
    else
    {
    //
    // Don't use the z buffer at all.
    //
#define ZBUFFER_PREDICATE_START
#define ZBUFFER_PREDICATE_END
        if(quantizedNormalIndices == 0)
        {
#define VISIBLE_POINT_PREDICATE
#include <CellLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }
        else
        {
#define VISIBLE_POINT_PREDICATE if(visiblePoint[quantizedNormalIndices[id]])
#include <CellLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }
#undef ZBUFFER_PREDICATE_START
#undef ZBUFFER_PREDICATE_END
    }

    cellLabelsCached = true;
}
#undef BEGIN_LABEL
#undef GET_THE_POINT 
#define GET_THE_POINT const double *vert = p->GetPoint(id);
#undef TEXT_PROPERTY 
#define TEXT_PROPERTY this->NodeLabelProperty

// ****************************************************************************
// Method: vtkLabelMapper::DrawAllNodeLabels3D
//
// Notes:  Taken from avtLabelRenderer
//
// Purpose: 
//   Draws all node labels in 3D.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 5 14:49:59 PST 2004
//
// Modifications:
//   Jeremy Meredith, Mon Nov  8 17:16:21 PST 2004
//   Caching is now done on a per-vtk-dataset basis.
//
//   Brad Whitlock, Fri Aug 5 14:13:02 PST 2005
//   Added coding to use the zbuffer to further restrict the labels that
//   get rendered.
//
//   Brad Whitlock, Tue Apr 25 10:26:22 PDT 2006
//   I made it use our own transformation matrix to transform the points.
//
// ****************************************************************************

#define BEGIN_LABEL labelString = this->NodeLabelsCache + this->MaxLabelSize*id; if(!nodeLabelsCached) {

void
vtkLabelMapper::DrawAllNodeLabels3D(vtkDataSet *input, vtkRenderer *ren)
{
    const char *mName = "vtkLabelMapper::DrawAllNodeLabels3D: ";
    char *labelString = 0;
    bool   nodeLabelsCached    = this->NodeLabelsCache != NULL;

    // Resize the node labels cache.
    if(this->NodeLabelsCache == 0 || 
       input->GetNumberOfPoints() != this->NodeLabelsCacheSize)
    {
        delete [] this->NodeLabelsCache;
        this->NodeLabelsCacheSize = input->GetNumberOfPoints();
        this->NodeLabelsCache = new char[this->MaxLabelSize * this->NodeLabelsCacheSize + 1];
    }

    //
    // See if the dataset has quantized node normals. If so, use them to
    // do backface culling on the labels that are facing away from the
    // camera.
    //
    vtkUnsignedCharArray *qnna = (vtkUnsignedCharArray *)input->
        GetPointData()->GetArray("LabelFilterQuantizedNodeNormals");
    const unsigned char *quantizedNormalIndices = (qnna != 0) ?
        (const unsigned char *)qnna->GetVoidPointer(0): 0;

    //
    // Include the method body with BEGIN_LABEL, END_LABEL macros defined
    // such that we immediately draw the labels without first transforming
    // them.
    //

    size_t index = this->TextMappers.size();
    if(zBufferMode == ZBUFFER_USE_PROVIDED)
    {
        debug4 << mName << "zBufferMode=ZBUFFER_USE_PROVIDED" << endl;
        //
        // Use the provided z buffer to determine help determine whether a label
        // should be plotted.
        //
#define ZBUFFER_PREDICATE_START \
                          double sx = vprime[0];\
                          double sy = vprime[1];\
                          ren->NormalizedDisplayToDisplay(sx, sy);\
                          int isx = int(sx);\
                          int isy = int(sy);\
                          if(isx >= 0 && isx < zBufferWidth &&\
                             isy >= 0 && isy < zBufferHeight &&\
                             vprime[2] <= zBuffer[isy*zBufferWidth+isx]+zTolerance)\
                          {
#define ZBUFFER_PREDICATE_END }
        if(quantizedNormalIndices == 0)
        {
#define VISIBLE_POINT_PREDICATE
#include <NodeLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }
        else
        {
#define VISIBLE_POINT_PREDICATE if(visiblePoint[quantizedNormalIndices[id]])
#include <NodeLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }

#undef ZBUFFER_PREDICATE_START
#undef ZBUFFER_PREDICATE_END
    }
    else if(zBufferMode == ZBUFFER_QUERY)
    {
        debug4 << mName << "zBufferMode=ZBUFFER_QUERY" << endl;
        //
        // Query the zbuffer one pixel at a time to avoid having to read it
        // all at once.
        //
#define ZBUFFER_PREDICATE_START float Z = 0.;\
                          double sx = vprime[0];\
                          double sy = vprime[1];\
                          ren->NormalizedDisplayToDisplay(sx, sy);\
                          int isx = int(sx);\
                          int isy = int(sy);\
                          if(isx >= 0 && isx < zBufferWidth &&\
                             isy >= 0 && isy < zBufferHeight)\
                          {\
                              Z=ren->GetZ(isx, isy);\
                              if(vprime[2] <= Z+zTolerance)\
                              {
#define ZBUFFER_PREDICATE_END }}
        if(quantizedNormalIndices == 0)
        {
#define VISIBLE_POINT_PREDICATE
#include <NodeLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }
        else
        {
#define VISIBLE_POINT_PREDICATE if(visiblePoint[quantizedNormalIndices[id]])
#include <NodeLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }

#undef ZBUFFER_PREDICATE_START
#undef ZBUFFER_PREDICATE_END
    }
    else
    {
        debug4 << mName << "zBufferMode=ZBUFFER_DONT_USE" << endl;
        //
        // Don't use the z buffer at all.
        //
#define ZBUFFER_PREDICATE_START
#define ZBUFFER_PREDICATE_END
        if(quantizedNormalIndices == 0)
        {
            debug4 << mName << "No quantized normals" << endl;
#define VISIBLE_POINT_PREDICATE
#include <NodeLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }
        else
        {
            debug4 << mName << "Using quantized normals" << endl;
#define VISIBLE_POINT_PREDICATE if(visiblePoint[quantizedNormalIndices[id]])
#include <NodeLabels_body.C>
#undef VISIBLE_POINT_PREDICATE
        }
#undef ZBUFFER_PREDICATE_START
#undef ZBUFFER_PREDICATE_END
    }

    nodeLabelsCached = true;
}
#undef BEGIN_LABEL
#undef END_LABEL


