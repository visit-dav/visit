#ifndef avtOpenGLLabelRenderer
#include <avtOpenGLLabelRenderer.h>
#else
#include <Init.h>
#endif
#include <arial.h>

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

#ifndef VTK_IMPLEMENT_MESA_CXX
#if defined(__APPLE__) && (defined(VTK_USE_CARBON) || defined(VTK_USE_COCOA))
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#else
#include <GL/gl.h>
#endif

#define ZBUFFER_USE_PROVIDED 0
#define ZBUFFER_QUERY        1
#define ZBUFFER_DONT_USE     2
#define ZBUFFER_QUERY_CUTOFF 100

//
// Include the vector quantization table
//
#include <quant_vector_lookup.C>

#ifndef avtOpenGLLabelRenderer
#include <avtCallback.h>
bool avtOpenGLLabelRenderer::zBufferWarningIssued = false;
#endif

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
//   Brad Whitlock, Mon Aug 8 17:35:11 PST 2005
//   Added zbuffer stuff.
//
// ****************************************************************************

avtOpenGLLabelRenderer::avtOpenGLLabelRenderer() : avtLabelRenderer()
{
    x_scale = 1.;
    y_scale = 1.;
    zBuffer = 0;
    zBufferMode = ZBUFFER_DONT_USE;
    zBufferWidth = 0;
    zBufferHeight = 0;
    zTolerance = 0.;

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
    ClearZBuffer();
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
    ClearZBuffer();
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
//   Hank Childs, Thu Jul 21 08:58:01 PDT 2005
//   Added support for newlines ('\n').
//
//   Brad Whitlock, Fri Aug 5 09:38:52 PDT 2005
//   Improved newline support.
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
    for(cptr = label; *cptr != '\0' && *cptr != '\n'; ++cptr)
    {
        unsigned int cIndex = (unsigned int)(*cptr);
        width += arial_triangle_spacing[cIndex];
    }

    float dx = 0.f;
    float dy = -0.3f;
    float off_y = (maxLabelRows > 1) ? (float(maxLabelRows) * 0.3f) : 0.f;

    if(atts.GetHorizontalJustification() == LabelAttributes::HCenter)
        dx = -width * 0.5f;
    else if(atts.GetHorizontalJustification() == LabelAttributes::Right)
        dx = -width;

    if(atts.GetVerticalJustification() == LabelAttributes::Top)
        dy -= 0.5f;
    else if(atts.GetVerticalJustification() == LabelAttributes::Bottom)
        dy += 0.5f;

    glTranslatef(dx, dy + off_y, 0);

    double total_translate = 0.;
    for(cptr = label; *cptr != '\0'; ++cptr)
    {
        unsigned int cIndex = (unsigned int)(*cptr);
        if (cIndex == '\n')
        {
            glTranslatef(-total_translate, 3*dy, 0);
            total_translate = 0.;
        }
        else
        {
            if(characterDisplayListIndices[cIndex] != -1)
            {
                glCallList(characterDisplayListIndices[cIndex]);
            }
    
            // To the next character location
            total_translate += arial_triangle_spacing[cIndex];
            glTranslatef(arial_triangle_spacing[cIndex],0,0);
        }
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
//   Hank Childs, Thu Jul 21 08:58:01 PDT 2005
//   Added support for newlines ('\n').
//
//   Brad Whitlock, Fri Aug 5 09:40:36 PDT 2005
//   Improved newline support.
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
    for(cptr = label; *cptr != '\0' && *cptr != '\n'; ++cptr)
    {
        unsigned int cIndex = (unsigned int)(*cptr);
        width += arial_triangle_spacing[cIndex];
    }

    float dx = 0.f;
    float dy = -0.3f;
    float off_y = (maxLabelRows > 1) ? (float(maxLabelRows) * 0.3f) : 0.f;

    if(atts.GetHorizontalJustification() == LabelAttributes::HCenter)
        dx = -width * 0.5f;
    else if(atts.GetHorizontalJustification() == LabelAttributes::Right)
        dx = -width;

    if(atts.GetVerticalJustification() == LabelAttributes::Top)
        dy -= 0.5f;
    else if(atts.GetVerticalJustification() == LabelAttributes::Bottom)
        dy += 0.5f;

    // Translate the text to the screen location
    glTranslatef(screenPoint[0]*x_scale + dx, screenPoint[1]*y_scale + dy + off_y, 0);

    double total_translate = 0.;
    for(cptr = label; *cptr != '\0'; ++cptr)
    {
        unsigned int cIndex = (unsigned int)(*cptr);
        if (cIndex == '\n')
        {
            glTranslatef(-total_translate, 3*dy, 0);
            total_translate = 0.;
        }
        else
        {
            if(characterDisplayListIndices[cIndex] != -1)
            {
                glCallList(characterDisplayListIndices[cIndex]);
            }
    
            // To the next character location
            total_translate += arial_triangle_spacing[cIndex];
            glTranslatef(arial_triangle_spacing[cIndex],0.f,0.f);
        }
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
//   Brad Whitlock, Thu Aug 4 10:32:20 PDT 2005
//   I removed the code to set the colors because I moved it into the new
//   SetColor method.
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
// Method: avtOpenGLLabelRenderer::SetColor
//
// Purpose: 
//   Sets the color based on the type of variable being plotted and the
//   desired node or cell coloring.
//
// Arguments:
//   index : If it is 0 then do node coloring, else cell coloring.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 4 10:33:05 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLLabelRenderer::SetColor(int index)
{
    if(atts.GetVarType() == LabelAttributes::LABEL_VT_MESH)
    {
        if(index == 0)
        {
            // Node color
            if(atts.GetSpecifyTextColor2())
            {
                ColorAttribute textColor(atts.GetTextColor2());
                textColor.SetAlpha(255);
                glColor4ubv(textColor.GetColor());
            }
            else
            {
                glColor4dv(fgColor);
            }
        }
        else
        {
            // Cell color
            if(atts.GetSpecifyTextColor1())
            {
                ColorAttribute textColor(atts.GetTextColor1());
                textColor.SetAlpha(255);
                glColor4ubv(textColor.GetColor());
            }
            else
            {
                glColor4dv(fgColor);
            }
        }
    }
    else
    {
        //
        // Set the color of the labels.
        //
        if(atts.GetSpecifyTextColor1())
        {
            ColorAttribute textColor(atts.GetTextColor1());
            textColor.SetAlpha(255);
            glColor4ubv(textColor.GetColor());
        }
        else
        {
            glColor4dv(fgColor);
        }
    }
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
//   Brad Whitlock, Tue Aug 2 15:27:44 PST 2005
//   I removed the single cell/node stuff. I also moved some coding to set
//   the size into new methods so we can have individual sizes for node
//   and cell labels.
//
// ****************************************************************************

void
avtOpenGLLabelRenderer::DrawLabels2D()
{
    const char *mName = "avtOpenGLLabelRenderer::DrawLabels2D: ";
    vtkDataArray *pointData = input->GetPointData()->GetArray(varname);
    vtkDataArray *cellData = input->GetCellData()->GetArray(varname);

    if(atts.GetVarType() == LabelAttributes::LABEL_VT_VECTOR_VAR)
    {
        if(pointData == 0)
            pointData = input->GetPointData()->GetVectors();
        if(cellData == 0)
            cellData = input->GetCellData()->GetVectors();
    }

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
        bool notSubsetOrMaterial = 
            atts.GetVarType() != LabelAttributes::LABEL_VT_SUBSET &&
            atts.GetVarType() != LabelAttributes::LABEL_VT_MATERIAL;

        // The variable must have been a mesh
        if(notSubsetOrMaterial && atts.GetShowNodes())
        {
            CreateCachedNodeLabels();
            createNodeLabels = true;
        }

        if(atts.GetShowCells())
        {
            CreateCachedCellLabels();
            createCellLabels = true;
        }
    }

    debug4 << mName << "varname=" << varname << endl;
    debug4 << mName << "haveNodeData=" << (haveNodeData?"true":"false") << endl;
    debug4 << mName << "haveCellData=" << (haveCellData?"true":"false") << endl;
    debug4 << mName << "createNodeLabels=" << (createNodeLabels?"true":"false") << endl;
    debug4 << mName << "createCellLabels=" << (createCellLabels?"true":"false") << endl;

    //
    // Now render the labels.
    //
    if(atts.GetRestrictNumberOfLabels())
    {
        DrawDynamicallySelectedLabels2D(createNodeLabels, createCellLabels);
    }
    else
    {
        DrawAllLabels2D(createNodeLabels, createCellLabels);
    }
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::BeginSize2D
//
// Purpose: 
//   Sets the text size based on whether we're doing node or cell labels if
//   we're plotting a mesh.
//
// Arguments:
//   index : If it is 0 then do node sizing, else cell sizing.
// 
// Programmer: Brad Whitlock
// Creation:   Thu Aug 4 10:36:31 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLLabelRenderer::BeginSize2D(int index)
{
    float h = atts.GetTextHeight1();

    if(atts.GetVarType() == LabelAttributes::LABEL_VT_MESH && index == 0)
        h = atts.GetTextHeight2();

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

    float pt2[] = {h*0.8, h, 0};
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
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::EndSize2D
//
// Purpose: 
//   Pops the matrix that we pushed in BeginSize2D.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 4 10:37:35 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLLabelRenderer::EndSize2D()
{
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
//    Brad Whitlock, Thu Aug 4 10:24:48 PDT 2005
//    Added the ability to have different colors and sizes for node vs.
//    cell labels.
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
        SetColor(0);
        BeginSize2D(0);
        for(int i = 0; i < nodeLabelsCacheSize; ++i)
        {
            const float *vert = p->GetPoint(i);
            DrawLabel2(vert, labelPtr);
            labelPtr += MAX_LABEL_SIZE;
        }
        EndSize2D();
    }

    //
    // Draw all the cell labels.
    //
    vtkFloatArray *cellCenters = GetCellCenterArray();
    if(drawCellLabels && cellCenters != 0)
    {
        const char *labelPtr = cellLabelsCacheMap[input];
        int cellLabelsCacheSize = cellLabelsCacheSizeMap[input];
        SetColor(1);
        BeginSize2D(1);
        for(int i = 0; i < cellLabelsCacheSize; ++i)
        {
            const float *vert = cellCenters->GetTuple3(i);
            DrawLabel2(vert, labelPtr);
            labelPtr += MAX_LABEL_SIZE;
        }
        EndSize2D();
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
    float textScale = atts.GetTextHeight1() / 0.02;
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
    bool *bins = new bool[bin_x_y];
    for(int b = 0; b < bin_x_y; ++b)
        bins[b] = false;

#ifdef VISUALIZE_DYNAMIC_BINS
    glColor3f(1.,0.,0.);
    glBegin(GL_LINES);
    for(int i = 0; i < bin_x_n; ++i)
    {
        float v0[3], v1[3];
        v0[0] = bin_x_offset + float(i) * bin_x_size;
        v0[1] = bin_y_offset;
        v0[2] = 0.;

        v1[0] = bin_x_offset + float(i) * bin_x_size;
        v1[1] = bin_y_offset + bin_y_n * bin_y_size;
        v1[2] = 0.;

        glVertex3fv(v0);
        glVertex3fv(v1);
    }
    for(int i = 0; i < bin_y_n; ++i)
    {
        float v0[3], v1[3];
        v0[0] = bin_x_offset;
        v0[1] = bin_y_offset + float(i) * bin_y_size;
        v0[2] = 0.;

        v1[0] = bin_x_offset + bin_x_n * bin_x_size;
        v1[1] = bin_y_offset + float(i) * bin_y_size;
        v1[2] = 0.;

        glVertex3fv(v0);
        glVertex3fv(v1);
    }
    glEnd();
#endif

    //
    // Iterate through the node labels and draw them if they fit into bins.
    //
    if(drawNodeLabels)
    {
        char *labelPtr = nodeLabelsCacheMap[input];
        int nodeLabelsCacheSize = nodeLabelsCacheSizeMap[input];
        vtkPoints *p = input->GetPoints();
        SetColor(0);
        BeginSize2D(0);
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
            if (bins[binIndex])
                continue; /*cell has label*/

            // Mark that the cell has a label
            bins[binIndex] = true;

            // Draw the label in the cell.
            DrawLabel2(labelVert, labelPtr);
        }
        EndSize2D();
    }

    //
    // Iterate through the cell labels and draw them if they fit into bins.
    //
    vtkFloatArray *cellCenters = GetCellCenterArray();
    if(drawCellLabels && cellCenters != 0)
    {
        const char *labelPtr = cellLabelsCacheMap[input];
        int cellLabelsCacheSize = cellLabelsCacheSizeMap[input];
        SetColor(1);
        BeginSize2D(1);
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
            if (bins[binIndex])
                continue; /*cell has label*/

            // Mark that the cell has a label
            bins[binIndex] = true;

            // Draw the label in the cell.
            DrawLabel2(labelVert, labelPtr);
        }
        EndSize2D();
    }

    delete [] bins;
}

// ****************************************************************************
// Method: PopulateBinsHelper
//
// Purpose: 
//   Helps the PopulateBinsWithNodeLabels3D and PopulateBinsWithCellLabels3D
//   functions.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 8 09:47:56 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLLabelRenderer::PopulateBinsHelper(const unsigned char *
    quantizedNormalIndices, const char *currentLabel,
    const float *transformedPoint, int n)
{
    if(zBufferMode == ZBUFFER_USE_PROVIDED)
    {
        if(quantizedNormalIndices != 0)
        {
            //
            // Here we only allow visible points in the bins.
            //
            for(int i = 0; i < n; ++i)
            {
                if(visiblePoint[quantizedNormalIndices[i]])
                {
                    int sx = int(float(zBufferWidth) * transformedPoint[0]);
                    int sy = int(float(zBufferHeight) * transformedPoint[1]);
                    if(sx >= 0 && sx < zBufferWidth &&
                       sy >= 0 && sy < zBufferHeight &&
                       transformedPoint[2] <= zBuffer[sy * zBufferWidth + sx]+zTolerance)
                    {
                        AllowLabelInBin(transformedPoint, currentLabel, 0);
                    }
                }
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
                int sx = int(float(zBufferWidth) * transformedPoint[0]);
                int sy = int(float(zBufferHeight) * transformedPoint[1]);
                if(sx >= 0 && sx < zBufferWidth &&
                   sy >= 0 && sy < zBufferHeight &&
                  transformedPoint[2] <= zBuffer[sy * zBufferWidth + sx]+zTolerance)
                {
                    AllowLabelInBin(transformedPoint, currentLabel, 0);
                }
                transformedPoint += 3;
                currentLabel += MAX_LABEL_SIZE;
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
            for(int i = 0; i < n; ++i)
            {
                if(visiblePoint[quantizedNormalIndices[i]])
                {
                    int sx = int(float(zBufferWidth) * transformedPoint[0]);
                    int sy = int(float(zBufferHeight) * transformedPoint[1]);
                    if(sx >= 0 && sx < zBufferWidth &&
                       sy >= 0 && sy < zBufferHeight)
                    {
                        float Z = 0.;
                        glReadPixels(sx, sy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, 
                                     (GLvoid*)&Z);
                        if(transformedPoint[2] <= Z+zTolerance)
                            AllowLabelInBin(transformedPoint, currentLabel, 0);
                    }
                }
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
                int sx = int(float(zBufferWidth) * transformedPoint[0]);
                int sy = int(float(zBufferHeight) * transformedPoint[1]);
                if(sx >= 0 && sx < zBufferWidth &&
                   sy >= 0 && sy < zBufferHeight)
                {
                    float Z = 0.;
                    glReadPixels(sx, sy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, 
                                 (GLvoid*)&Z);
                    if(transformedPoint[2] <= Z+zTolerance)
                        AllowLabelInBin(transformedPoint, currentLabel, 0);
                }
                transformedPoint += 3;
                currentLabel += MAX_LABEL_SIZE;
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
            for(int i = 0; i < n; ++i)
            {
                if(visiblePoint[quantizedNormalIndices[i]])
                    AllowLabelInBin(transformedPoint, currentLabel, 0);
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
                AllowLabelInBin(transformedPoint, currentLabel, 0);
                transformedPoint += 3;
                currentLabel += MAX_LABEL_SIZE;
            }
        }
    }
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::PopulateBinsWithNodeLabels3D
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
// ****************************************************************************

void
avtOpenGLLabelRenderer::PopulateBinsWithNodeLabels3D()
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
    PopulateBinsHelper(quantizedNormalIndices, currentLabel, transformedPoint,
                       n);
    visitTimer->StopTimer(stageTimer, "Binning the 3D node labels");

    delete [] xformedPoints;

    visitTimer->StopTimer(total, "PopulateBinsWithNodeLabels3D");
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::PopulateBinsWithCellLabels3D
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
// ****************************************************************************

void
avtOpenGLLabelRenderer::PopulateBinsWithCellLabels3D()
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
    PopulateBinsHelper(quantizedNormalIndices, currentLabel, transformedPoint,
                       n);
    visitTimer->StopTimer(stageTimer, "Binning the 3D cell labels");
  
    delete [] xformedPoints;

    visitTimer->StopTimer(total, "PopulateBinsWithCellLabels3D");
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
//   Brad Whitlock, Tue Aug 2 15:28:22 PST 2005
//   I removed the single cell/node stuff. I also added the ability to have
//   different colors and sizes for node vs. cell labels. Finally, I added
//   optional z-buffering to the renderer.
//
// ****************************************************************************

void
avtOpenGLLabelRenderer::DrawLabels3D()
{
    const char *mName = "avtOpenGLLabelRenderer::DrawLabels3D: ";
    vtkDataArray *pointData = input->GetPointData()->GetArray(varname);
    vtkDataArray *cellData = input->GetCellData()->GetArray(varname);
    bool haveNodeData = pointData != 0;
    bool haveCellData = cellData != 0;

    int total = visitTimer->StartTimer();
    int stageTimer = visitTimer->StartTimer();

    //
    // Populate the label caches.
    //
    bool notSubsetOrMaterial = 
         atts.GetVarType() != LabelAttributes::LABEL_VT_SUBSET &&
         atts.GetVarType() != LabelAttributes::LABEL_VT_MATERIAL;
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
        if(notSubsetOrMaterial && atts.GetShowNodes())
            CreateCachedNodeLabels();

        if(!notSubsetOrMaterial || atts.GetShowCells())
            CreateCachedCellLabels();
    }
    visitTimer->StopTimer(stageTimer, "Creating label caches");

    debug4 << mName << "varname = " << varname << endl;
    debug4 << mName << "haveNodeData = " << (haveNodeData?"true":"false") << endl;
    debug4 << mName << "haveCellData = " << (haveCellData?"true":"false") << endl;
    debug4 << mName << "notSubsetOrMaterial = " << (notSubsetOrMaterial?"true":"false") << endl;

    // Select the text scale.
    float node_x_scale, node_y_scale;
    float cell_x_scale, cell_y_scale;
    if(atts.GetVarType() == LabelAttributes::LABEL_VT_MESH)
    {
        node_x_scale = atts.GetTextHeight2();
        node_y_scale = atts.GetTextHeight2();
        cell_x_scale = atts.GetTextHeight1();
        cell_y_scale = atts.GetTextHeight1();
    }
    else
    {
        node_x_scale = atts.GetTextHeight1();
        node_y_scale = atts.GetTextHeight1();
        cell_x_scale = atts.GetTextHeight1();
        cell_y_scale = atts.GetTextHeight1();
    }

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

    //
    // Initialize the ZBuffer.
    //
    if((rendererAction & RENDERER_ACTION_INIT_ZBUFFER) != 0)
        InitializeZBuffer(haveNodeData, haveCellData);

    // Push the current matrices onto the stack and temporarily
    // override them so we can draw in screen space.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

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
            // The variable must have been a scalar, vector, tensor, label.
            if(haveNodeData)
            {
                PopulateBinsWithNodeLabels3D();
            }

            if(haveCellData)
            {
                PopulateBinsWithCellLabels3D();
            }
        }
        else
        {
            // The variable must have been a mesh, subset, or material.
            if(notSubsetOrMaterial && atts.GetShowNodes())
            {
                PopulateBinsWithNodeLabels3D();
            }

            if(!notSubsetOrMaterial || atts.GetShowCells())
            {
                PopulateBinsWithCellLabels3D();
            }
        }
        visitTimer->StopTimer(stageTimer, "Binning 3D labels");

        //
        // Draw the labels that came from nodes.
        //
        stageTimer = visitTimer->StartTimer();
        int n = numXBins * numYBins;
        const LabelInfo *info = labelBins;
        x_scale = node_x_scale;
        y_scale = node_y_scale;
        SetColor(0);
        int i;
        for(i = 0; i < n; ++i, ++info)
        {
            if(info->label != 0 && info->type == 0)
                DrawLabel(info->screenPoint, info->label);
        }

        //
        // Draw the labels that came from cells.
        //
        x_scale = cell_x_scale;
        y_scale = cell_y_scale;
        SetColor(1);
        info = labelBins;
        for(i = 0; i < n; ++i, ++info)
        {
            if(info->label != 0 && info->type == 1)
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
            // The variable must have been a scalar, vector, tensor, etc.
            if(haveNodeData)
            {
                x_scale = node_x_scale;
                y_scale = node_y_scale;
                SetColor(0);
                DrawAllNodeLabels3D();
            }

            if(haveCellData)
            {
                x_scale = cell_x_scale;
                y_scale = cell_y_scale;
                SetColor(1);
                DrawAllCellLabels3D();
            }
        }
        else
        {
            // The variable must have been a mesh, subset, or material.
            if(notSubsetOrMaterial && atts.GetShowNodes())
            {
                x_scale = node_x_scale;
                y_scale = node_y_scale;
                SetColor(0);
                DrawAllNodeLabels3D();
            }

            if(!notSubsetOrMaterial || atts.GetShowCells())
            {
                x_scale = cell_x_scale;
                y_scale = cell_y_scale;
                SetColor(1);
                DrawAllCellLabels3D();
            }
        }
        visitTimer->StopTimer(stageTimer, "Drawing all labels 3D");
    }

#define TRANSFORM_POINT(P) VTKRen->WorldToView(P[0], P[1], P[2]); \
                           VTKRen->ViewToNormalizedViewport(P[0], P[1], P[2]);

    // Restore the matrices.
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // If the renderAction flag permits us to delete the zbuffer,
    // do it now.
    if((rendererAction & RENDERER_ACTION_FREE_ZBUFFER) != 0)
        ClearZBuffer();

    visitTimer->StopTimer(total, "avtOpenGLLabelRenderer::DrawLabels3D");
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::ClearZBuffer
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
avtOpenGLLabelRenderer::ClearZBuffer()
{
    if(zBuffer != 0)
    {
        debug4 << "avtOpenGLLabelRenderer::ClearZBuffer: Deleting z buffer\n";
        delete [] zBuffer;
        zBuffer = 0;
    }
}

// ****************************************************************************
// Method: avtOpenGLLabelRenderer::InitializeZBuffer
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
//   
// ****************************************************************************

void
avtOpenGLLabelRenderer::InitializeZBuffer(bool haveNodeData,
    bool haveCellData)
{
    const char *mName = "avtOpenGLRenderer::InitializeZBuffer: ";
    //
    // Figure out if we need the zbuffer for anything. If we need it then
    // read it back into a buffer that we allocated.
    //
    zBufferMode = ZBUFFER_DONT_USE;
    zTolerance = 0.f;

    if(atts.GetDepthTestMode() != LabelAttributes::LABEL_DT_NEVER)
    {
        zBufferWidth  = VTKRen->GetVTKWindow()->GetSize()[0];
        zBufferHeight = VTKRen->GetVTKWindow()->GetSize()[1];

#ifdef avtOpenGLLabelRenderer
        // If we're using Mesa then let's just query since we already have
        // the zbuffer in memory.
        zBufferMode = ZBUFFER_QUERY;
        zBuffer = 0;
#else
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
                if(VTKRen->GetVTKWindow()->IsA("vtkRenderWindow"))
                {
                    vtkRenderWindow *renWin = (vtkRenderWindow*)VTKRen->GetVTKWindow();
                    if(renWin->IsDirect())
                        readZBuffer = true;
                    else if(!zBufferWarningIssued)
                    {
                        zBufferWarningIssued = true;
                        avtCallback::IssueWarning("VisIt is not running on a direct "
                           "display so the z-buffer will not be read back to aid in "
                           "depth testing to determine which labels should not be "
                           "drawn. If you want to enable depth testing, set the "
                           "Label plot's depth test flag to Always.");
                    }
                }
            }
        }

        // Read the z-buffer.
        if(readZBuffer)
        {
            int getZ = visitTimer->StartTimer();
            int zBufferSize = zBufferWidth * zBufferHeight;

            debug4 << mName << "Allocated z-buffer" << endl;
            zBuffer = new float[zBufferSize];
            if(zBuffer != 0)
            {
                glReadPixels(0, 0, zBufferWidth, zBufferHeight,
                             GL_DEPTH_COMPONENT, GL_FLOAT,
                             (GLvoid*)zBuffer);
                zBufferMode = ZBUFFER_USE_PROVIDED;
            }
            visitTimer->StopTimer(getZ, "Reading back Z-buffer");
        }
#endif
 
        //
        // Set zTolerance using the projection matrix. We have to do this
        // to make the depth testing play well with mesh plots, and wireframe
        // surface plots, which shift the plot in Z to ensure that the lines
        // are drawn in front of surface primitives.
        //
        // Examine the current projection matrix to compute a zShift 
        // see documentation for glFrustum for what C, D and r are
        //
        float pmatrix[16];
        glGetFloatv(GL_PROJECTION_MATRIX, pmatrix);
        double C = pmatrix[10];
        double D = pmatrix[14];
        double r = (C-1.)/(C+1.);
        double farPlane = D * (1.-r)/2.;
        double nearPlane = farPlane / r;

        // compute a shift based upon total range in Z
        double zShift1 = (farPlane - nearPlane) / 1.0e+4;

        // compute a shift based upon distance between eye and near clip
        double zShift2 = nearPlane / 2.0;

        // use whatever shift is smaller
        double ZT = double(zShift1 < zShift2 ? zShift1 : zShift2);
        // Multiply it by a little so the labels win over the lines.
        zTolerance = float(ZT * 1.001);
    }

    if(zBufferMode == ZBUFFER_DONT_USE)
        debug4 << mName << "zBufferMode = ZBUFFER_DONT_USE" << endl;
    else if(zBufferMode == ZBUFFER_USE_PROVIDED)
        debug4 << mName << "zBufferMode = ZBUFFER_USE_PROVIDED" << endl;
    else if(zBufferMode == ZBUFFER_QUERY)
        debug4 << mName << "zBufferMode = ZBUFFER_QUERY" << endl;
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
//   Jeremy Meredith, Mon Nov  8 17:16:21 PST 2004
//   Caching is now done on a per-vtk-dataset basis.
//
//   Brad Whitlock, Fri Aug 5 14:13:02 PST 2005
//   Added coding to use the zbuffer to further restrict the labels that
//   get rendered.
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
                          ZBUFFER_PREDICATE_START \
                              DrawLabel(vprime, labelString);\
                          ZBUFFER_PREDICATE_END \
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
    if(zBufferMode == ZBUFFER_USE_PROVIDED)
    {
    //
    // Use the provided z buffer to determine help determine whether a label
    // should be plotted.
    //
#define ZBUFFER_PREDICATE_START float pix = 0.;\
                          float sx = vprime[0];\
                          float sy = vprime[1];\
                          VTKRen->NormalizedDisplayToDisplay(sx, sy);\
                          int isx = int(sx);\
                          int isy = int(sy);\
                          if(sx >= 0 && sx < zBufferWidth &&\
                             sy >= 0 && sy < zBufferHeight &&\
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
                          float sx = vprime[0];\
                          float sy = vprime[1];\
                          VTKRen->NormalizedDisplayToDisplay(sx, sy);\
                          int isx = int(sx);\
                          int isy = int(sy);\
                          if(sx >= 0 && sx < zBufferWidth &&\
                             sy >= 0 && sy < zBufferHeight)\
                          {\
                              glReadPixels(int(sx), int(sy), 1, 1,\
                                           GL_DEPTH_COMPONENT, GL_FLOAT,\
                                           (GLvoid*)&Z);\
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
//   Jeremy Meredith, Mon Nov  8 17:16:21 PST 2004
//   Caching is now done on a per-vtk-dataset basis.
//
//   Brad Whitlock, Fri Aug 5 14:13:02 PST 2005
//   Added coding to use the zbuffer to further restrict the labels that
//   get rendered.
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
    if(zBufferMode == ZBUFFER_USE_PROVIDED)
    {
    //
    // Use the provided z buffer to determine help determine whether a label
    // should be plotted.
    //
#define ZBUFFER_PREDICATE_START float pix = 0.;\
                          float sx = vprime[0];\
                          float sy = vprime[1];\
                          VTKRen->NormalizedDisplayToDisplay(sx, sy);\
                          int isx = int(sx);\
                          int isy = int(sy);\
                          if(sx >= 0 && sx < zBufferWidth &&\
                             sy >= 0 && sy < zBufferHeight &&\
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
    //
    // Query the zbuffer one pixel at a time to avoid having to read it
    // all at once.
    //
#define ZBUFFER_PREDICATE_START float Z = 0.;\
                          float sx = vprime[0];\
                          float sy = vprime[1];\
                          VTKRen->NormalizedDisplayToDisplay(sx, sy);\
                          int isx = int(sx);\
                          int isy = int(sy);\
                          if(sx >= 0 && sx < zBufferWidth &&\
                             sy >= 0 && sy < zBufferHeight)\
                          {\
                              glReadPixels(int(sx), int(sy), 1, 1,\
                                           GL_DEPTH_COMPONENT, GL_FLOAT,\
                                           (GLvoid*)&Z);\
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
    //
    // Don't use the z buffer at all.
    //
#define ZBUFFER_PREDICATE_START
#define ZBUFFER_PREDICATE_END
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
 
    M->Delete();

    nodeLabelsCached = true;
}
#undef BEGIN_LABEL
#undef END_LABEL
