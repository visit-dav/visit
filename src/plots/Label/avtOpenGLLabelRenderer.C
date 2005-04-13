#ifndef avtOpenGLLabelRenderer
#include <avtOpenGLLabelRenderer.h>
#else
#include <Init.h>
#endif
#include <arial.h>
#include <GL/gl.h>

#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkUnsignedIntArray.h>

#include <snprintf.h>

#include <DebugStream.h>
#include <TimingsManager.h>

//
// Include the vector quantization table
//
#include <quant_vector_lookup.C>

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::avtOpenGLLabelRenderer
//
// Purpose: 
//   Constructor for the avtOpenGLLabelRenderer class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:14:10 PDT 2004
//
// Modifications:
//
// ****************************************************************************

avtOpenGLLabelRenderer::avtOpenGLLabelRenderer() : avtLabelRenderer()
{
    x_scale = 1.;
    y_scale = 1.;

    characterDisplayListsCreated = false;
    for(int i = 0; i < 256; ++i)
        characterDisplayListIndices[i] = -1;
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::~avtOpenGLLabelRenderer
//
// Purpose: 
//   Destructor for the avtOpenGLLabelRenderer class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:14:32 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 16:12:27 PST 2004
//   Changed the routine to release the display lists.
//
// ****************************************************************************

avtOpenGLLabelRenderer::~avtOpenGLLabelRenderer()
{
    ClearCharacterDisplayLists();
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::ReleaseGraphicsResources
//
// Purpose: 
//   Clears the character display lists.
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
avtOpenGLLabelRenderer::ReleaseGraphicsResources()
{
    ClearLabelCaches();
    ClearCharacterDisplayLists();
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::SetupGraphicsLibrary
//
// Purpose: 
//   This method creates display lists to be used for drawing the letters.
//
// Note:       There is partial code for texture mapped fonts but the textures
//             have not been created.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:14:59 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

#ifdef TEXTURED_FONT
#include <six_alpha.h>
#endif

void
avtOpenGLLabelRenderer::SetupGraphicsLibrary()
{
    if(!characterDisplayListsCreated)
    {
        characterDisplayListsCreated = true;

#ifndef TEXTURED_FONT
        // Triangle based font.
        for(int i = 0; i < 256; ++i)
        {
            if(arial_triangle_ntriangles[i] > 0)
            {
                int listIndex = glGenLists(1);

                if(listIndex == 0)
                    continue;

                characterDisplayListIndices[i] = listIndex;
                glNewList(characterDisplayListIndices[i], GL_COMPILE);
                glBegin(GL_TRIANGLES);
  
                int triangleStart = arial_triangle_start[i];

                unsigned char *yIndex = arial_triangle_y_index + triangleStart;
                unsigned char *xIndex = arial_triangle_x_index + triangleStart;
                for(int tri = 0; tri < arial_triangle_ntriangles[i]; ++tri)
                {
                    float v[2];
                    v[0] = arial_font_x[xIndex[0]];
                    v[1] = arial_font_y[yIndex[0]];
                    glVertex2fv(v);

                    v[0] = arial_font_x[xIndex[1]];
                    v[1] = arial_font_y[yIndex[1]];
                    glVertex2fv(v);

                    v[0] = arial_font_x[xIndex[2]];
                    v[1] = arial_font_y[yIndex[2]];
                    glVertex2fv(v);
                    yIndex += 3;
                    xIndex += 3;
                }

                glEnd();
                glEndList();
            }
        }
#else
        // Texture based font.

        // This part would also go into the loop if I had more than
        // 1 texture.
        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA8,
                     64,64,0, GL_ALPHA, GL_UNSIGNED_BYTE, six_alpha);

        for(int i = 0; i < 256; ++i)
        {
            if(arial_triangle_ntriangles[i] > 0)
            {
                int listIndex = glGenLists(1);

                if(listIndex == 0)
                    continue;

                characterDisplayListIndices[i] = listIndex;

                //
                // Try creating a textured font drawn on a quad.
                //
                glNewList(characterDisplayListIndices[i], GL_COMPILE);
                glBindTexture(GL_TEXTURE_2D, texId);
                glBegin(GL_QUADS);

                glTexCoord2f(0,1);
                glVertex2f(0,0);

                glTexCoord2f(1,1);
                glVertex2f(arial_triangle_spacing[i], 0);

                glTexCoord2f(1,0);
                glVertex2f(arial_triangle_spacing[i], 1);

                glTexCoord2f(0,0);
                glVertex2f(0, 1);

                glEnd();
                glEndList();
            }
        }
#endif
    }
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::ClearCharacterDisplayLists
//
// Purpose: 
//   Clears the display lists being used for the letters.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:16:30 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 15:54:59 PST 2004
//   Prevented the display lists from being deleted if they were not created.
//
// ****************************************************************************

void
avtOpenGLLabelRenderer::ClearCharacterDisplayLists()
{
    if (VTKRen == NULL)
        return;

    if(characterDisplayListsCreated)
    {
        VTKRen->GetRenderWindow()->MakeCurrent();

        for(int i = 0; i < 256; ++i)
        {
            if(characterDisplayListIndices[i] != -1)
            {
                glDeleteLists(characterDisplayListIndices[i],1);
                characterDisplayListIndices[i] = -1;
            }
        }
        characterDisplayListsCreated = false;
    }
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::DrawLabel
//
// Purpose: 
//   Draws the labels in normalized display space, which assumes that the
//   points have been transformed on the CPU.
//
// Arguments:
//   screenPoint : The point in normalized display space.
//   label       : The label to be drawn.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:17:02 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLLabelRenderer::DrawLabel(const float *screenPoint, const char *label)
{
    glLoadIdentity();

//debug4 << "label: " << label << " at (" << screenPoint[0] << ", "
//       << screenPoint[1] << ", " << screenPoint[2] << ")" << endl;

    // Translate the text to the screen location
    glTranslatef(screenPoint[0], screenPoint[1], screenPoint[2]);

    // Scale the text
    glScalef(x_scale, y_scale, 1);    

    //
    // Compute the width of the label. Cache this??
    //
    float width = 0;
    const char *cptr = label;
    for(cptr = label; *cptr != '\0'; ++cptr)
    {
        unsigned int cIndex = (unsigned int)(*cptr);
        width += arial_triangle_spacing[cIndex];
    }

    float dx = 0.f;
    float dy = -0.3f;

    if(atts.GetHorizontalJustification() == LabelAttributes::HCenter)
        dx = -width * 0.5f;
    else if(atts.GetHorizontalJustification() == LabelAttributes::Right)
        dx = -width;

    if(atts.GetVerticalJustification() == LabelAttributes::Top)
        dy -= 0.5f;
    else if(atts.GetVerticalJustification() == LabelAttributes::Bottom)
        dy += 0.5f;

    glTranslatef(dx, dy, 0);

    for(cptr = label; *cptr != '\0'; ++cptr)
    {
        unsigned int cIndex = (unsigned int)(*cptr);
        if(characterDisplayListIndices[cIndex] != -1)
        {
            glCallList(characterDisplayListIndices[cIndex]);
        }

        // To the next character location
        glTranslatef(arial_triangle_spacing[cIndex],0,0);
    }
}

//
//
// ****************************************************************************
// Method: avtOpenGLLabelRenderer::DrawLabel2
//
// Purpose: 
//   Draws the label in 2D world space, which is faster since the points don't
//   have to be transformed in the CPU.
//   
// Arguments:
//   screenPoint : The 2D point in world space.
//   label       : The label to be drawn.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:18:01 PDT 2004
//
// Modifications:
//   Brad Whitlock, Wed Nov 3 09:23:01 PDT 2004
//   Fixed for win32.
//
// ****************************************************************************

void
avtOpenGLLabelRenderer::DrawLabel2(const float *screenPoint, const char *label)
{
#ifdef DEBUG_MATRICES
    glBegin(GL_POINTS);
    float v[] = {screenPoint[0], screenPoint[1], 0.};
    glVertex3fv(v);
    glEnd();
#else
    glPushMatrix();
    //
    // Compute the width of the label. Cache this??
    //
    float width = 0;
    const char *cptr = label;
    for(cptr = label; *cptr != '\0'; ++cptr)
    {
        unsigned int cIndex = (unsigned int)(*cptr);
        width += arial_triangle_spacing[cIndex];
    }

    float dx = 0.f;
    float dy = -0.3f;

    if(atts.GetHorizontalJustification() == LabelAttributes::HCenter)
        dx = -width * 0.5f;
    else if(atts.GetHorizontalJustification() == LabelAttributes::Right)
        dx = -width;

    if(atts.GetVerticalJustification() == LabelAttributes::Top)
        dy -= 0.5f;
    else if(atts.GetVerticalJustification() == LabelAttributes::Bottom)
        dy += 0.5f;

    // Translate the text to the screen location
    glTranslatef(screenPoint[0]*x_scale + dx, screenPoint[1]*y_scale + dy, 0);

    for(cptr = label; *cptr != '\0'; ++cptr)
    {
        unsigned int cIndex = (unsigned int)(*cptr);
        if(characterDisplayListIndices[cIndex] != -1)
        {
            glCallList(characterDisplayListIndices[cIndex]);
        }

        // To the next character location
        glTranslatef(arial_triangle_spacing[cIndex], 0.f, 0.f);
    }

    glPopMatrix();
#endif
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::RenderLabels
//
// Purpose: 
//   Renders the labels.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:18:49 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 16:16:16 PST 2004
//   Added code to clear graphics resources when scalable rendering.
//
// ****************************************************************************

void
avtOpenGLLabelRenderer::RenderLabels()
{
    //
    // Make the OpenGL calls to get it into the desired state.
    //
    SetupGraphicsLibrary();

    if(varname == 0)
        return;

    //
    // Set up the OpenGL state
    //
#ifdef TEXTURED_FONT
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif

    //
    // Turn off lighting. -- Really we should check VTK somehow to see
    //                       if lighting is enabled before we go and
    //                       turn it off because we need that check before
    //                       we go turning it on.
    //
    glDisable(GL_LIGHTING);

    //
    // Set the color of the labels.
    //
    if(atts.GetUseForegroundTextColor())
    {
        glColor4dv(fgColor);
    }
    else
    {
        ColorAttribute textColor(atts.GetTextColor());
        textColor.SetAlpha(255);
        glColor4ubv(textColor.GetColor());
    }

    // Disable depth testing
    bool enableDepthTest = true;
    glDisable(GL_DEPTH_TEST);

    if(renderLabels3D)
    {
        DrawLabels3D();
    }
    else 
    {
        DrawLabels2D();
    }

    // Enable depth testing if it was on.
    if(enableDepthTest)
        glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);

#ifdef TEXTURED_FONT
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
#endif

#ifdef avtOpenGLLabelRenderer
    //
    // Free up graphics resources if using the Mesa renderer.
    //
    ClearCharacterDisplayLists();
#endif
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::DrawLabels2D
//
// Purpose: 
//   Draws the labels in 2D according to the options specified in the label plot.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:19:08 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Oct 25 16:27:22 PST 2004
//   I made it use VTKRen.
//
// ****************************************************************************

void
avtOpenGLLabelRenderer::DrawLabels2D()
{
    vtkDataArray *pointData = input->GetPointData()->GetArray(varname);
    vtkDataArray *cellData = input->GetCellData()->GetArray(varname);
    bool haveNodeData = pointData != 0;
    bool haveCellData = cellData != 0;

    //
    // Populate the label caches.
    //
    bool createNodeLabels = false;
    bool createCellLabels = false;
    if(haveNodeData || haveCellData)
    {
        // The variable must have been a scalar or vector.
        if(haveNodeData)
        {
            CreateCachedNodeLabels();
            createNodeLabels = true;
        }

        if(haveCellData)
        {
            CreateCachedCellLabels();
            createCellLabels = true;
        }
    }
    else
    {
        // The variable must have been a mesh
        if(atts.GetShowNodes() && !atts.GetShowSingleNode())
        {
            CreateCachedNodeLabels();
            createNodeLabels = true;
        }

        if(atts.GetShowCells() && !atts.GetShowSingleCell())
        {
            CreateCachedCellLabels();
            createCellLabels = true;
        }
    }

    //
    // Now render the labels.
    //
#ifndef DEBUG_MATRICES
    // Try and devise an alternate scaling so the letters are drawn
    // in the right size when in fullframe mode.
    //
    // Here we transform 2 points, which are the opposite corners
    // of a letter given as a fraction of the normalized display height
    // into world space so we know how to scale the 1x1 box in which the
    // actual letters are defined.
    //
    float pt1[] = {0,0,0};
    VTKRen->NormalizedDisplayToViewport(pt1[0], pt1[1]);
    VTKRen->ViewportToNormalizedViewport(pt1[0], pt1[1]);
    VTKRen->NormalizedViewportToView(pt1[0], pt1[1], pt1[2]);
    VTKRen->ViewToWorld(pt1[0], pt1[1], pt1[2]);

    float pt2[] = {atts.GetTextHeight()*0.8, atts.GetTextHeight(),0};
    VTKRen->NormalizedDisplayToViewport(pt2[0], pt2[1]);
    VTKRen->ViewportToNormalizedViewport(pt2[0], pt2[1]);
    VTKRen->NormalizedViewportToView(pt2[0], pt2[1], pt2[2]);
    VTKRen->ViewToWorld(pt2[0], pt2[1], pt2[2]);
    float charWidth  = pt2[0] - pt1[0];
    float charHeight = pt2[1] - pt1[1];
    x_scale = charWidth;
    y_scale = charHeight;

    // Scale the text
    glPushMatrix();
    glScalef(x_scale, y_scale, 1);

    // store the reciprocals so the translations get done correctly
    // in DrawLabel2.
    x_scale = 1. / x_scale;
    y_scale = 1. / y_scale;
#endif
    if(atts.GetRestrictNumberOfLabels())
    {
        DrawDynamicallySelectedLabels2D(createNodeLabels, createCellLabels);
    }
    else
    {
        DrawAllLabels2D(createNodeLabels, createCellLabels);
    }

    //
    // Draw the single cell.
    //
    if(atts.GetShowSingleCell())
    {
        SetupSingleCellLabel();
        if(singleCellInfo.label != 0)
            DrawLabel2(singleCellInfo.screenPoint, singleCellInfo.label);
    }

    //
    // Draw the single node.
    //
    if(atts.GetShowSingleNode())
    {
        SetupSingleNodeLabel();
        if(singleNodeInfo.label != 0)
            DrawLabel2(singleNodeInfo.screenPoint, singleNodeInfo.label);
    }
#ifndef DEBUG_MATRICES
    // Restore the original view matrix.
    glPopMatrix();
#endif
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::DrawAllLabels2D
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
// ****************************************************************************

void
avtOpenGLLabelRenderer::DrawAllLabels2D(bool drawNodeLabels, bool drawCellLabels)
{
    //
    // Draw all the node labels.
    //
    if(drawNodeLabels)
    {
        vtkPoints *p = input->GetPoints();
        const char *labelPtr = nodeLabelsCacheMap[input];
        int nodeLabelsCacheSize = nodeLabelsCacheSizeMap[input];
        for(int i = 0; i < nodeLabelsCacheSize; ++i)
        {
            const float *vert = p->GetPoint(i);
            DrawLabel2(vert, labelPtr);
            labelPtr += MAX_LABEL_SIZE;
        }
    }

    //
    // Draw all the cell labels.
    //
    vtkFloatArray *cellCenters = GetCellCenterArray();
    if(drawCellLabels && cellCenters != 0)
    {
        const char *labelPtr = cellLabelsCacheMap[input];
        int cellLabelsCacheSize = cellLabelsCacheSizeMap[input];
        for(int i = 0; i < cellLabelsCacheSize; ++i)
        {
            const float *vert = cellCenters->GetTuple3(i);
            DrawLabel2(vert, labelPtr);
            labelPtr += MAX_LABEL_SIZE;
        }
    }
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::DrawDynamicallySelectedLabels2D
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
// ****************************************************************************

void
avtOpenGLLabelRenderer::DrawDynamicallySelectedLabels2D(bool drawNodeLabels,
    bool drawCellLabels)
{
    //
    // Figure out the world coordinates of the window that is being displayed.
    //
    float lowerleft[3] = {0., 0., 0.};
    VTKRen->NormalizedViewportToView(lowerleft[0], lowerleft[1], lowerleft[2]);
    VTKRen->ViewToWorld(lowerleft[0], lowerleft[1], lowerleft[2]);

    float upperright[3] = {1., 1., 0.};
    VTKRen->NormalizedViewportToView(upperright[0], upperright[1], upperright[2]);
    VTKRen->ViewToWorld(upperright[0], upperright[1], upperright[2]);

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
    int LL = (maxLabelLength < 1) ? 1 : maxLabelLength;
    double bin_aspect = LL * char_aspect;
    double nx_target = sqrt (atts.GetNumberOfLabels() * win_aspect / bin_aspect);
    double ny_target = sqrt (atts.GetNumberOfLabels() * bin_aspect / win_aspect);

    //
    // Compute the cell size in such a way as to have at least the target
    // number of cells in each direction but at the same time to be an
    // approximation so that cells don't change size too often when the
    // window is zoomed.
    //
    const double base = 2.;
    double bin_x_size, bin_y_size;
    if (win_aspect >= 1.0)
    {
        // The X axis is the long axis of the view window.
        double power = (log (win_dx) - log (nx_target)) / log (base);
        bin_x_size = pow (base, floor (power));
        bin_y_size = bin_x_size / bin_aspect;
    }
    else
    {
        // The Y axis is the long axis of the view window.
        double power = (log (win_dy) - log (ny_target)) / log (base);
        bin_y_size = pow (base, floor (power));
        bin_x_size = bin_y_size * bin_aspect;
    }

    //
    // Compute the offset to the first cell and the number of cells in
    // each dimension.
    //
    float minMeshX = spatialExtents[0];
    float minMeshY = spatialExtents[2];
    double bin_x_offset = floor ((lowerleft[0] - minMeshX) / bin_x_size) *
                    bin_x_size + minMeshX;
    double bin_y_offset = floor ((lowerleft[1] - minMeshY) / bin_y_size) *
                    bin_y_size + minMeshY;
    int bin_x_n = int(ceil (win_dx / bin_x_size)) + 1;
    int bin_y_n = int(ceil (win_dy / bin_y_size)) + 1;
    int bin_x_y = bin_x_n * bin_y_n;

    //
    // Create an array to record whether or not we've drawn a label
    // for a bin.
    //
    char  *bins = new char[bin_x_y];
    memset (bins, -1, bin_x_y * sizeof (char)); /* -1 */

    //
    // Iterate through the node labels and draw them if they fit into bins.
    //
    if(drawNodeLabels)
    {
        char *labelPtr = nodeLabelsCacheMap[input];
        int nodeLabelsCacheSize = nodeLabelsCacheSizeMap[input];
        vtkPoints *p = input->GetPoints();
        for(int i = 0; i < nodeLabelsCacheSize; ++i, labelPtr += MAX_LABEL_SIZE)
        {
            //
            // Which cell does the label fall within?
            //
            const float *labelVert = p->GetPoint(i);
            float label_x = labelVert[0];
            float label_y = labelVert[1];
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
            if (bins[binIndex] >= 0)
                continue; /*cell has label*/

            // Mark that the cell has a label
            bins[binIndex] = 1;

            // Draw the label in the cell.
            DrawLabel2(labelVert, labelPtr);
        }
    }

    //
    // Iterate through the cell labels and draw them if they fit into bins.
    //
    vtkFloatArray *cellCenters = GetCellCenterArray();
    if(drawCellLabels && cellCenters != 0)
    {
        const char *labelPtr = cellLabelsCacheMap[input];
        int cellLabelsCacheSize = cellLabelsCacheSizeMap[input];
        for(int i = 0; i < cellLabelsCacheSize; ++i, labelPtr += MAX_LABEL_SIZE)
        {
            //
            // Which bin does the label fall within?
            //
            const float *labelVert = cellCenters->GetTuple3(i);
            float label_x = labelVert[0];
            float label_y = labelVert[1];
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
            if (bins[binIndex] >= 0)
                continue; /*cell has label*/

            // Mark that the cell has a label
            bins[binIndex] = 1;

            // Draw the label in the cell.
            DrawLabel2(labelVert, labelPtr);
        }
    }

    delete [] bins;
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::DrawLabels3D
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
// ****************************************************************************

void
avtOpenGLLabelRenderer::DrawLabels3D()
{
    vtkDataArray *pointData = input->GetPointData()->GetArray(varname);
    vtkDataArray *cellData = input->GetCellData()->GetArray(varname);
    bool haveNodeData = pointData != 0;
    bool haveCellData = cellData != 0;

    int total = visitTimer->StartTimer();
    int stageTimer = visitTimer->StartTimer();

    //
    // Populate the label caches.
    //
    if(haveNodeData || haveCellData)
    {
        // The variable must have been a scalar or vector.
        if(haveNodeData)
            CreateCachedNodeLabels();

        if(haveCellData)
            CreateCachedCellLabels();
    }
    else
    {
        // The variable must have been a mesh
        if(atts.GetShowNodes() && !atts.GetShowSingleNode())
            CreateCachedNodeLabels();

        if(atts.GetShowCells() && !atts.GetShowSingleCell())
            CreateCachedCellLabels();
    }
    visitTimer->StopTimer(stageTimer, "Creating label caches");

    // Push the current matrices onto the stack and temporarily
    // override them so we can draw in screen space.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Set the text scale.
    x_scale = atts.GetTextHeight();
    y_scale = atts.GetTextHeight();

    //
    // Determine which of the quantized vectors are visible given the
    // current camera.
    //
    stageTimer = visitTimer->StartTimer();
    vtkCamera *camera = VTKRen->GetActiveCamera();
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

    if(atts.GetRestrictNumberOfLabels())
    {
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
            // The variable must have been a scalar or vector.
            if(haveNodeData)
                PopulateBinsWithNodeLabels3D();

            if(haveCellData)
                PopulateBinsWithCellLabels3D();
        }
        else
        {
            // The variable must have been a mesh
            if(atts.GetShowNodes() && !atts.GetShowSingleNode())
                PopulateBinsWithNodeLabels3D();

            if(atts.GetShowCells() && !atts.GetShowSingleCell())
                PopulateBinsWithCellLabels3D();
        }
        visitTimer->StopTimer(stageTimer, "Binning 3D labels");

        //
        // Draw the labels.
        //
        stageTimer = visitTimer->StartTimer();
        int n = numXBins * numYBins;
        const LabelInfo *info = labelBins;
        for(int i = 0; i < n; ++i, ++info)
        {
            if(info->label != 0)
                DrawLabel(info->screenPoint, info->label);
        }
        visitTimer->StopTimer(stageTimer, "Drawing binned 3D labels");
    }
    else
    {
        //
        // Draw the labels without doing any kind of reduction.
        //
        stageTimer = visitTimer->StartTimer();
        if(haveNodeData || haveCellData)
        {
            // The variable must have been a scalar or vector.
            if(haveNodeData)
                DrawAllNodeLabels3D();

            if(haveCellData)
                DrawAllCellLabels3D();
        }
        else
        {
            // The variable must have been a mesh
            if(atts.GetShowNodes() && !atts.GetShowSingleNode())
                DrawAllNodeLabels3D();

            if(atts.GetShowCells() && !atts.GetShowSingleCell())
                DrawAllCellLabels3D();
        }
        visitTimer->StopTimer(stageTimer, "Drawing all labels 3D");
    }

#define TRANSFORM_POINT(P) VTKRen->WorldToView(P[0], P[1], P[2]); \
                           VTKRen->ViewToNormalizedViewport(P[0], P[1], P[2]);

    //
    // Draw the single cell.
    //
    stageTimer = visitTimer->StartTimer();
    if(atts.GetShowSingleCell())
    {
        SetupSingleCellLabel();
        if(singleCellInfo.label != 0)
        {
            TRANSFORM_POINT(singleCellInfo.screenPoint);
            DrawLabel(singleCellInfo.screenPoint, singleCellInfo.label);
        }
    }

    //
    // Draw the single node.
    //
    if(atts.GetShowSingleNode())
    {
        SetupSingleNodeLabel();
        if(singleNodeInfo.label != 0)
        {
            TRANSFORM_POINT(singleNodeInfo.screenPoint);
            DrawLabel(singleNodeInfo.screenPoint, singleNodeInfo.label);
        }
    }
    visitTimer->StopTimer(stageTimer, "Drawing single node, cell");

    // Restore the matrices.
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    visitTimer->StopTimer(total, "avtOpenGLLabelRenderer::DrawLabels3D");
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::DrawAllCellLabels3D
//
// Purpose: 
//   Draws transformed text in 3D.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 5 14:45:59 PST 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:16:21 PST 2004
//    Caching is now done on a per-vtk-dataset basis.
//   
// ****************************************************************************

#define BEGIN_LABEL labelString = cellLabelsCache + MAX_LABEL_SIZE*id; if(!cellLabelsCached) {
#define GET_THE_POINT const float *vert = cellCenters->GetTuple3(id);
#define END_LABEL   } VISIBLE_POINT_PREDICATE\
                      { \
                          GET_THE_POINT \
                          float v[4] = {vert[0], vert[1], vert[2], 1.f}, vprime[4]; \
                          M->MultiplyPoint(v, vprime); \
                          if (vprime[3] != 0.) \
                          { \
                              vprime[0] /= vprime[3]; \
                              vprime[1] /= vprime[3]; \
                              vprime[2] /= vprime[3]; \
                          } \
                          DrawLabel(vprime, labelString);\
                      }

void
avtOpenGLLabelRenderer::DrawAllCellLabels3D()
{
    char *labelString = 0;
    char *&cellLabelsCache     = cellLabelsCacheMap[input];
    int    cellLabelsCacheSize = cellLabelsCacheSizeMap[input];
    bool   cellLabelsCached    = cellLabelsCache != NULL;

    //
    // Look for the cell center array that the label filter calculated.
    //
    vtkFloatArray *cellCenters = GetCellCenterArray();
    if(cellCenters == 0)
        return;

    // Resize the cell labels cache.
    if(cellLabelsCache == 0 || input->GetNumberOfCells() != cellLabelsCacheSize)
    {
        delete [] cellLabelsCache;
        cellLabelsCacheSize = input->GetNumberOfCells();
        cellLabelsCache = new char[MAX_LABEL_SIZE * cellLabelsCacheSize + 1];
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
    vtkMatrix4x4 *M = WorldToDisplayMatrix();
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

    M->Delete();
    cellLabelsCached = true;
}
#undef BEGIN_LABEL
#undef GET_THE_POINT 
#define GET_THE_POINT const float *vert = p->GetPoint(id);

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::DrawAllNodeLabels3D
//
// Purpose: 
//   Draws all node labels in 3D.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 5 14:49:59 PST 2004
//
// Modifications:
//    Jeremy Meredith, Mon Nov  8 17:16:21 PST 2004
//    Caching is now done on a per-vtk-dataset basis.
//   
// ****************************************************************************

#define BEGIN_LABEL labelString = nodeLabelsCache + MAX_LABEL_SIZE*id; if(!nodeLabelsCached) {

void
avtOpenGLLabelRenderer::DrawAllNodeLabels3D()
{
    char *labelString = 0;
    char *&nodeLabelsCache     = nodeLabelsCacheMap[input];
    int    nodeLabelsCacheSize = nodeLabelsCacheSizeMap[input];
    bool   nodeLabelsCached    = nodeLabelsCache != NULL;

    // Resize the node labels cache.
    if(nodeLabelsCache == 0 || input->GetPoints()->GetNumberOfPoints() != nodeLabelsCacheSize)
    {
        delete [] nodeLabelsCache;
        nodeLabelsCacheSize = input->GetPoints()->GetNumberOfPoints();
        nodeLabelsCache = new char[MAX_LABEL_SIZE * nodeLabelsCacheSize + 1];
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
    vtkMatrix4x4 *M = WorldToDisplayMatrix();
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
 
    M->Delete();

    nodeLabelsCached = true;
}
#undef BEGIN_LABEL
#undef END_LABEL
