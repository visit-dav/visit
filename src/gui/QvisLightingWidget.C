/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <QvisLightingWidget.h>
#include <math.h>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QResizeEvent>

#include <mini3D.h>

/*  Settings for the picture elements */

#define ARROW_XDIM  10
#define CONE_XDIM   10
#define SPHERE_XDIM 20
#define SPHERE_YDIM 10

static float const arrow_tip_rad  = 0.8f;
static float const arrow_tip_len  = 1.5f;
static float const arrow_stem_rad = 0.3f;
static float const arrow_stem_len = 1.5f;

static float const cone_rad   = 1.0f;
static float const cone_len   = 3.0f;

static float const bulb_s1 = 2.0f;
static float const bulb_s2 = 1.2f;
static float const bulb_s3 = 0.5f;
static float const bulb_s4 = 0.3f;
static float const bulb_s5 = 2.2f;
static float const bulb_s6 = 2.7f;

static float const sphere_rad = 7.0f;

static float const cube_size  = 10.0f;   /* This is the cube half-side-width */

static color arrow_color = {0.2f, 0.3f, 1.0f}; /* medium blue      */
static color cone_color  = {1.0f, 0.8f, 0.3f}; /* harvest gold     */
static color bulb_color  = {0.7f, 0.6f, 0.2f}; /* yellow-ish       */
static color bulb_color2 = {0.4f, 0.4f, 0.4f}; /* medium gray      */

//
// Static members.
//
bool QvisLightingWidget::sharedElementsCreated = false;
m3d_complex_element QvisLightingWidget::litearrow;
m3d_complex_element QvisLightingWidget::litecone;
m3d_complex_element QvisLightingWidget::litebulb;
m3d_complex_element QvisLightingWidget::sphere;
m3d_complex_element QvisLightingWidget::cube;

// ****************************************************************************
// Method: QvisLightingWidget::QvisLightingWidget
//
// Purpose: 
//   This is the constructor for the QvisLightingWidget class.
//
// Arguments:
//   parent : A pointer to the parent widget.
//   name   : The name to be used for this instance of the widget.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 09:54:18 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 3 13:22:08 PST 2003
//   I initialized the renderer.
//
//   Brad Whitlock, Thu Jun  5 16:20:07 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisLightingWidget::QvisLightingWidget(QWidget *parent) : 
    QWidget(parent), renderer(150,150)
{
    pixmap = 0;
    previewMode = false;
    mouseDown = false;
    needsRedrawPixmap = true;
    rendererCreated = false;
    numberOfLights = 0;

    createSharedElements();
}

// ****************************************************************************
// Method: QvisLightingWidget::~QvisLightingWidget
//
// Purpose: 
//   This is the destructor for the QvisLightingWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 09:55:12 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisLightingWidget::~QvisLightingWidget()
{
    deleteBackingPixmap();
}

// ****************************************************************************
// Method: QvisLightingWidget::createSharedElements
//
// Purpose: 
//   Creates the shared elements that are used by all instances of the widget.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 17:25:33 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWidget::createSharedElements()
{
    // If this is the first instance of this type of widget then create
    // some elements that the mini3D renderer will use.
    if(!sharedElementsCreated)
    {
        initializeCube();
        initializeSphere();
        initializeLitecone();
        initializeLitearrow();
        initializeLitebulb();
        sharedElementsCreated = true;
    }
}

// ****************************************************************************
// Method: QvisLightingWidget::sizeHint
//
// Purpose: 
//   This method returns the widget's preferred size.
//
// Returns:    The widget's preferred size.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 09:56:11 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisLightingWidget::sizeHint() const
{
    return QSize(150, 150);
}

// ****************************************************************************
// Method: QvisLightingWidget::sizePolicy
//
// Purpose: 
//   Returns how the widget allows itself to be resized.
//
// Returns:    How the widget allows itself to be resized.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 09:56:38 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QSizePolicy
QvisLightingWidget::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

// ****************************************************************************
// Method: QvisLightingWidget::setPreviewMode
//
// Purpose: 
//   Sets the widget's preview mode. When in preview mode, all enabled lights
//   are displayed. Otherwise, just the first light is shown.
//
// Arguments:
//   val : The new preview mode.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:41:47 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Oct 25 14:18:41 PST 2001
//   I removed an extra if test to make the widget update properly.
//
// ****************************************************************************

void
QvisLightingWidget::setPreviewMode(bool val)
{
    previewMode = val;

    needsRedrawPixmap = true;
    update();
}

// ****************************************************************************
// Method: QvisLightingWidget::clearLights
//
// Purpose: 
//   Clears all of the lights from the scene.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:41:29 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWidget::clearLights()
{
    numberOfLights = 0;

    needsRedrawPixmap = true;
    update();
}

// ****************************************************************************
// Method: QvisLightingWidget::addLight
//
// Purpose: 
//   Adds a light to the widget.
//
// Arguments:
//   position : An array containing the position.
//   c        : The light color.
//   type     : The type of the light.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:40:52 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWidget::addLight(const double position[3], const QColor &c, int type)
{
    lights[numberOfLights].t = (light_type)type;
    lights[numberOfLights].v.x = float(position[0]);
    lights[numberOfLights].v.y = float(position[1]);
    lights[numberOfLights].v.z = float(position[2]);
    lights[numberOfLights].c.r = float(c.red()) / 255.;
    lights[numberOfLights].c.g = float(c.green()) / 255.;
    lights[numberOfLights].c.b = float(c.blue()) / 255.;
    ++numberOfLights;

    needsRedrawPixmap = true;
    update();
}

// ****************************************************************************
// Method: QvisLightingWidget::setLightColor
//
// Purpose: 
//   Sets the color for the first light.
//
// Arguments:
//   c : The color of the light.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:40:21 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWidget::setLightColor(const QColor &c)
{
    lights[0].c.r = float(c.red()) / 255.;
    lights[0].c.g = float(c.green()) / 255.;
    lights[0].c.b = float(c.blue()) / 255.;

    needsRedrawPixmap = true;
    update();    
}

// ****************************************************************************
// Method: QvisLightingWidget::setLightPosition
//
// Purpose: 
//   Sets the position for the first light.
//
// Arguments:
//   position : An array containing the position.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:39:53 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWidget::setLightPosition(const double position[3])
{
    lights[0].v.x = float(position[0]);
    lights[0].v.y = float(position[1]);
    lights[0].v.z = float(position[2]);

    needsRedrawPixmap = true;
    update();
}

// ****************************************************************************
// Method: QvisLightingWidget::setLightType
//
// Purpose: 
//   Sets the light type for the first light.
//
// Arguments:
//   type : The new light type.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:39:20 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWidget::setLightType(int type)
{
    lights[0].t = (light_type)type;

    needsRedrawPixmap = true;
    update();
}

// ****************************************************************************
// Method: QvisLightingWidget::mousePressEvent
//
// Purpose: 
//   This is an event handler that is called when the mouse button is pressed.
//
// Arguments:
//   e : An object that contains information about the mouse click.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 09:57:11 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        lastX = ( (float)(e->x()*2)/(float)width() - 1.);
        lastY =-( (float)(e->y()*2)/(float)height() - 1.);
        mouseDown = true;
    }
    else if (e->button() == Qt::MidButton)
    {
        view2=m3du_create_identity_matrix();
        renderer.set_view_matrix(mtx_mult(view,view2));
        needsRedrawPixmap = true;
        update();
    }
}

// ****************************************************************************
// Method: QvisLightingWidget::mouseMoveEvent
//
// Purpose: 
//   This is an event handler that is called when the mouse is moved.
//
// Arguments:
//   e : An object that contains information about the mouse movement.
//
// Programmer: Jeremy Meredith, Brad Whitlock
// Creation:   Fri Oct 19 09:57:11 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(mouseDown)
    {
        float x,y;
        matrix4 tb;

        x = ( (float)(e->x()*2)/(float)width() - 1.);
        y =-( (float)(e->y()*2)/(float)height() - 1.);
        tb = m3du_create_trackball_matrix(lastX, lastY, x, y);

        // Are we rotating the light or the view?
        if (!previewMode)
        {
            vector3 dir;

            /* For directional lights, rotations should occur  *
             * with respect to the current view orientation    */
            if (lights[0].t == M3D_LIGHT_DIR)
                tb = mtx_mult(mtx_inverse(view2), mtx_mult(tb, view2));

            dir = vec_normalize(vec_create(lights[0].v.x,
                                           lights[0].v.y,
                                           lights[0].v.z));
            dir = mtx_transform_vertex(tb, dir);

            // Save the light location and emit a signal that tells the
            // outside world about the change in location.
            lights[0].v.x = dir.x;
            lights[0].v.y = dir.y;
            lights[0].v.z = dir.z;

            // Force a re-render and update.
            needsRedrawPixmap = true;
            update();
        }
        else
        {
            view2 = mtx_mult(tb, view2);
            renderer.set_view_matrix(mtx_mult(view, view2));

            // Force a re-render and update.
            needsRedrawPixmap = true;
            update();
        }

        lastX = x;
        lastY = y;
    }
}

// ****************************************************************************
// Method: QvisLightingWidget::mouseReleaseEvent
//
// Purpose: 
//   This is the mouse release event handler.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:38:13 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWidget::mouseReleaseEvent(QMouseEvent *)
{
    emit lightMoved(double(lights[0].v.x),
                    double(lights[0].v.y),
                    double(lights[0].v.z));
    mouseDown = false;
}

// ****************************************************************************
// Method: QvisLightingWidget::paintEvent
//
// Purpose: 
//   This is the paint event handler for the widget.
//
// Arguments:
//   e : Information about the area to be painted.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:37:34 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Jun  5 16:21:22 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisLightingWidget::paintEvent(QPaintEvent *e)
{
    bool clipByRegion = true;

    if(pixmap == 0)
    {
        pixmap = new QPixmap(width(), height());
        needsRedrawPixmap = true;
    }

    if(needsRedrawPixmap)
    {
        QPainter pixpaint(pixmap);

        // Draw the scene into the backing pixmap.
        QBrush b(QColor(16,16,16));
        qDrawShadePanel(&pixpaint, 0, 0, width(), height(), palette(), true,
                        2, &b);
        redrawScene(&pixpaint);
        needsRedrawPixmap = false;
        clipByRegion = false;
    }

    // Blit the pixmap to the screen.
    QPainter paint(this);
    if(clipByRegion && !e->region().isEmpty())
        paint.setClipRegion(e->region());
    paint.drawPixmap(QPoint(0,0), *pixmap);
}

// ****************************************************************************
// Method: QvisLightingWidget::deleteBackingPixmap
//
// Purpose: 
//   This method deletes the backing pixmap.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:37:12 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWidget::deleteBackingPixmap()
{
    if(pixmap != 0)
    {
        delete pixmap;
        pixmap = 0;
    }
}

// ****************************************************************************
// Method: QvisLightingWidget::resizeEvent
//
// Purpose: 
//   This is the resize event handler for the widget.
//
// Arguments:
//   e : An object containing information about the resize.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:36:15 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 3 13:26:04 PST 2003
//   I made the renderer be an object.
//
// ****************************************************************************

void
QvisLightingWidget::resizeEvent(QResizeEvent *e)
{
    deleteBackingPixmap();
    renderer.resize(e->size().width(), e->size().height());
}

// ****************************************************************************
// Method: QvisLightingWidget::redrawScene
//
// Purpose: 
//   Redraws the scene using the specified painter.
//
// Arguments:
//   p : The painter used to draw the scene.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:34:22 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 3 13:26:47 PST 2003
//   I made the renderer be an object.
//
// ****************************************************************************

void
QvisLightingWidget::redrawScene(QPainter *painter)
{
    if(!rendererCreated)
    {
        renderer.resize(width(), height());

        view2= m3du_create_identity_matrix();
        view = m3du_create_view_matrix(vec_create(0,0,50),
                                       vec_create(0,0,0),
                                       vec_create(0,1,0));
        renderer.set_view_matrix(view);

        proj = m3du_create_proj_matrix(1,1000,3.14159f/4.f);
        renderer.set_proj_matrix(proj);
        rendererCreated = true;
    }

    // This redraws the show!
    setupAndDraw(painter);
}

// ****************************************************************************
// Method: QvisLightingWidget::setupAndDraw
//
// Purpose: 
//   This method is a helper for redrawScene which sets up the scene to be
//   rendered using the current list of lights.
//
// Arguments:
//   p : The painter used to draw the scene.
//
// Programmer: Jeremy Meredith, Brad Whitlock
// Creation:   Fri Oct 19 16:33:07 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 3 13:27:21 PST 2003
//   I made the renderer be an object.
//
// ****************************************************************************

void
QvisLightingWidget::setupAndDraw(QPainter *p)
{
    int l;

    // Set up the lighting
    for(l = 0; l < 8; ++l)
    {
        light_type t;
 
        if(previewMode)
            t = (l < numberOfLights) ? lights[l].t : M3D_LIGHT_OFF;
        else
            t = (l == 0) ? lights[l].t : M3D_LIGHT_OFF;

        renderer.set_light(l+1, t, lights[l].v.x, lights[l].v.y, lights[l].v.z,
                           lights[l].c.r, lights[l].c.g, lights[l].c.b);
    }

    renderer.begin_scene(p);

    // Create the icons for the currently selected lights
    int n_amb = 0;
    for(l = 0; l < (previewMode ? numberOfLights : 1); ++l)
    {
        int t = lights[l].t;
        vector3 dir;
        vector3 from;
        vector3 at;
        vector3 up;
        matrix4 world;

        dir = vec_normalize(vec_create(lights[l].v.x,
                                       lights[l].v.y,
                                       lights[l].v.z));
        switch (t)
        {
        case M3D_LIGHT_DIR:
            from = vec_scale(dir, -16);
            at   = vec_create(0,0,0);
            up   = vec_create(0,1,dir.y);
            break;
        case M3D_LIGHT_EYE:
            from = vec_scale(dir, -18);
            at   = vec_create(0,0,0);
            up   = vec_create(0,1,dir.y);
            break;
        case M3D_LIGHT_AMB:
            from = vec_create(-22 + 3*n_amb, -21, -10);
            at   = vec_create(from.x,from.y,from.z+1);
            up   = vec_create(0,1,0);
            n_amb++;
            break;
        }
            
        world = m3du_create_RBT_matrix(from,at,up);
        renderer.set_world_matrix(world);
         
        switch (t)
        {
        case M3D_LIGHT_DIR:
            // Directional lights are placed in world coordinates
            renderer.set_view_matrix(mtx_mult(view,view2));
            litecone.addToRenderer(renderer);
            break;
        case M3D_LIGHT_EYE:
            // Eye lights are placed in viewing coordinates
            renderer.set_view_matrix(view);
            litearrow.addToRenderer(renderer);
            break;
        case M3D_LIGHT_AMB:
            // Ambient lights are placed in viewing coordinates
            renderer.set_view_matrix(view);
            litebulb.addToRenderer(renderer);
            break;
        }
    }

    renderer.set_world_matrix(m3du_create_identity_matrix());
    renderer.set_view_matrix(mtx_mult(view,view2));

    // Draw the reference cube
    cube.addToRenderer(renderer);

    // Draw the lit sphere 
    sphere.addToRenderer(renderer);

    renderer.end_scene();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*                  Mini3D Element Initialization Functions                  */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Function  : initializeLitearrow()
 *
 * Purpose   : Initalize the arrow used for Eye Lights.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Modifications:
 *   Brad Whitlock, Mon Mar 3 13:30:18 PST 2003
 *   I made it be a member of the lighting widget and I changed how the
 *   object is contained.
 *
 *---------------------------------------------------------------------------*/

void
QvisLightingWidget::initializeLitearrow()
{
    int i;
    vector3 arrow1_pts[ARROW_XDIM+3];
    vector3 arrow2_pts[ARROW_XDIM*2+3];
    color   arrow_clr[ARROW_XDIM+1];

    /*
     * Create vertex arrays
     */

    /* cone-point */
    for (i=0; i<=ARROW_XDIM; i++)
    {
        float p = (float)i/(float)ARROW_XDIM;
        float pi = 3.14159265f;

        float x = sin(p*2*pi)*arrow_tip_rad;
        float y = cos(p*2*pi)*arrow_tip_rad;
        float z = 0;

        arrow1_pts[i].x = x;
        arrow1_pts[i].y = y;
        arrow1_pts[i].z = z;
    }
    arrow1_pts[ARROW_XDIM+1].x = 0;
    arrow1_pts[ARROW_XDIM+1].y = 0;
    arrow1_pts[ARROW_XDIM+1].z = arrow_tip_len;
    arrow1_pts[ARROW_XDIM+2].x = 0;
    arrow1_pts[ARROW_XDIM+2].y = 0;
    arrow1_pts[ARROW_XDIM+2].z = 0;

    /* cylinder-stem */
    for (i=0; i<=ARROW_XDIM; i++)
    {
        float p = (float)i/(float)ARROW_XDIM;
        float pi = 3.14159265f;

        float x = sin(p*2*pi)*arrow_stem_rad;
        float y = cos(p*2*pi)*arrow_stem_rad;
        float z = 0;

        arrow2_pts[i].x = x;
        arrow2_pts[i].y = y;
        arrow2_pts[i].z = z;

        z = -arrow_stem_len;

        arrow2_pts[ARROW_XDIM+1+i].x = x;
        arrow2_pts[ARROW_XDIM+1+i].y = y;
        arrow2_pts[ARROW_XDIM+1+i].z = z;
    }
    arrow2_pts[ARROW_XDIM*2+2].x = 0;
    arrow2_pts[ARROW_XDIM*2+2].y = 0;
    arrow2_pts[ARROW_XDIM*2+2].z = -arrow_stem_len;

    /*
     * Create the colors
     */
    for (i=0; i<ARROW_XDIM; i++)
    {
        float p = (float)i/(float)ARROW_XDIM;
        float pi = 3.14159265f;

        float b = fabs(sin(p*2*pi));

        arrow_clr[i].r = b*arrow_color.r;
        arrow_clr[i].g = b*arrow_color.g;
        arrow_clr[i].b = b*arrow_color.b;
    }
    arrow_clr[ARROW_XDIM].r = .5*arrow_color.r;
    arrow_clr[ARROW_XDIM].g = .5*arrow_color.g;
    arrow_clr[ARROW_XDIM].b = .5*arrow_color.b;

    /*
     * Create the elements
     */
    for (i=0; i<ARROW_XDIM; i++)
    {
        /* cone */
        litearrow.add_tri_c(arrow1_pts[i],
                            arrow1_pts[i+1],
                            arrow1_pts[ARROW_XDIM+1],
                            arrow_clr[i]);

        /* cone cap */
        litearrow.add_tri_c(arrow1_pts[i],
                            arrow1_pts[i+1],
                            arrow1_pts[ARROW_XDIM+2],
                            arrow_clr[ARROW_XDIM]);

        /* cylinder cap */
        litearrow.add_tri_c(arrow2_pts[ARROW_XDIM+1+i],
                            arrow2_pts[ARROW_XDIM+1+i+1],
                            arrow2_pts[ARROW_XDIM*2+2],
                            arrow_clr[ARROW_XDIM]);

        /* cylinder stem */
        litearrow.add_tri_c(arrow2_pts[i],
                            arrow2_pts[ARROW_XDIM+1+i  ],
                            arrow2_pts[ARROW_XDIM+1+i+1],
                            arrow_clr[i]);

        litearrow.add_tri_c(arrow2_pts[i],
                            arrow2_pts[i+1],
                            arrow2_pts[ARROW_XDIM+1+i+1],
                            arrow_clr[i]);
    }
}

/*---------------------------------------------------------------------------*
 * Function  : initializeLitecone()
 *
 * Purpose   : Initalize the cone used for Directional Lights.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Modifications:
 *   Brad Whitlock, Mon Mar 3 13:30:18 PST 2003
 *   I made it be a member of the lighting widget and I changed how the object
 *   is contained.
 *
 *---------------------------------------------------------------------------*/

void
QvisLightingWidget::initializeLitecone()
{
    int i;
    vector3 cone_pts[CONE_XDIM+3];
    color   cone_clr[CONE_XDIM+1];

    /*
     * Create vertex arrays
     */
    for (i=0; i<=CONE_XDIM; i++)
    {
        float p = (float)i/(float)CONE_XDIM;
        float pi = 3.14159265f;

        float x = sin(p*2*pi)*cone_rad;
        float y = cos(p*2*pi)*cone_rad;
        float z = 0;

        cone_pts[i].x = x;
        cone_pts[i].y = y;
        cone_pts[i].z = z;
    }
    cone_pts[CONE_XDIM+1].x = 0;
    cone_pts[CONE_XDIM+1].y = 0;
    cone_pts[CONE_XDIM+1].z = -cone_len;
    cone_pts[CONE_XDIM+2].x = 0;
    cone_pts[CONE_XDIM+2].y = 0;
    cone_pts[CONE_XDIM+2].z = 0;

    /*
     * Create the colors
     */
    for (i=0; i<CONE_XDIM; i++)
    {
        float p = (float)i/(float)CONE_XDIM;
        float pi = 3.14159265f;

        float b = fabs(sin(p*2*pi));

        cone_clr[i].r = b*cone_color.r;
        cone_clr[i].g = b*cone_color.g;
        cone_clr[i].b = b*cone_color.b;
    }
    cone_clr[CONE_XDIM].r = .5*cone_color.r;
    cone_clr[CONE_XDIM].g = .5*cone_color.g;
    cone_clr[CONE_XDIM].b = .5*cone_color.b;


    /*
     * Create the elements
     */
    for (i=0; i<CONE_XDIM; i++)
    {
        /* cone */
        litecone.add_tri_c(cone_pts[i],
                           cone_pts[i+1],
                           cone_pts[CONE_XDIM+1],
                           cone_clr[i]);

        /* cone cap */
        litecone.add_tri_c(cone_pts[i],
                           cone_pts[i+1],
                           cone_pts[CONE_XDIM+2],
                           cone_clr[CONE_XDIM]);
    }
}

/*---------------------------------------------------------------------------*
 * Function  : initializeLitebulb()
 *
 * Purpose   : Initalize the bulb used for Ambient Lights.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Modifications:
 *   Brad Whitlock, Mon Mar 3 13:30:18 PST 2003
 *   I made it be a member of the lighting widget and I changed how the object
 *   is contained.
 *
 *---------------------------------------------------------------------------*/

void
QvisLightingWidget::initializeLitebulb()
{
    /* vert central */
    litebulb.set_default_color(bulb_color);
    litebulb.add_tri_c(-bulb_s3, bulb_s2, 0,
                       bulb_s3, bulb_s2, 0,
                       bulb_s3,-bulb_s1, 0);

    litebulb.add_tri_c(-bulb_s3, bulb_s2, 0,
                       -bulb_s3,-bulb_s1, 0,
                       bulb_s3,-bulb_s1, 0);

    /* horiz central */
    litebulb.add_tri_c(-bulb_s2,-bulb_s3, 0,
                       -bulb_s2, bulb_s3, 0,
                       bulb_s2, bulb_s3, 0);
    litebulb.add_tri_c(-bulb_s2,-bulb_s3, 0,
                       bulb_s2,-bulb_s3, 0,
                       bulb_s2, bulb_s3, 0);

    /* fillets */
    litebulb.add_tri_c(-bulb_s2, bulb_s3, 0,
                       -bulb_s3, bulb_s2, 0,
                       -bulb_s3, bulb_s3, 0);
    litebulb.add_tri_c(bulb_s2, bulb_s3, 0,
                       bulb_s3, bulb_s2, 0,
                       bulb_s3, bulb_s3, 0);
    litebulb.add_tri_c(-bulb_s2,-bulb_s3, 0,
                       -bulb_s3,-bulb_s2, 0,
                       -bulb_s3,-bulb_s3, 0);
    litebulb.add_tri_c(bulb_s2,-bulb_s3, 0,
                       bulb_s3,-bulb_s2, 0,
                       bulb_s3,-bulb_s3, 0);

    /* base */
    litebulb.set_default_color(bulb_color2);
    litebulb.add_tri_c(-bulb_s3,-bulb_s5, 0,
                       -bulb_s4,-bulb_s6, 0,
                       -bulb_s4,-bulb_s5, 0);
    litebulb.add_tri_c(bulb_s3,-bulb_s5, 0,
                       bulb_s4,-bulb_s6, 0,
                       bulb_s4,-bulb_s5, 0);
    litebulb.add_tri_c(-bulb_s4,-bulb_s5, 0,
                       -bulb_s4,-bulb_s6, 0,
                       bulb_s4,-bulb_s5, 0);
    litebulb.add_tri_c(bulb_s4,-bulb_s6, 0,
                       -bulb_s4,-bulb_s6, 0,
                       bulb_s4,-bulb_s5, 0);

    /* and the highlight */
    color white = {1.,1.,1.,1.};
    litebulb.set_default_color(white);
    litebulb.add_tri_c( bulb_s3,-bulb_s4, .01f,
                       0   , bulb_s4, .01f,
                       0   , bulb_s3, .01f);
}

/*---------------------------------------------------------------------------*
 * Function  : initializeSphere()
 *
 * Purpose   : Initalized the sphere used to show the active lighting.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Modifications:
 *   Brad Whitlock, Mon Mar 3 13:33:29 PST 2003
 *   I made it a member of the lighting widget class and I changed how the
 *   object is contained.
 *
 *---------------------------------------------------------------------------*/

void
QvisLightingWidget::initializeSphere()
{
    int i,j;
    vector3 sphere_pts[SPHERE_XDIM+1][SPHERE_YDIM+1];
    vector3 sphere_nrm[SPHERE_XDIM  ][SPHERE_YDIM  ];

    /*
     * Create vertex arrays
     */
    for (i=0; i<=SPHERE_XDIM; i++)
    {
        for (j=0; j<=SPHERE_YDIM; j++)
        {
            float p  = (float)i/(float)SPHERE_XDIM;
            float q  = (float)j/(float)SPHERE_YDIM;
            float pi = 3.14159265f;

            float f = sin(q * pi);
            float z = cos(q * pi)*sphere_rad;
            float x = cos(p*2*pi)*sphere_rad * f;
            float y = sin(p*2*pi)*sphere_rad * f;

            sphere_pts[i][j].x = x;
            sphere_pts[i][j].y = y;
            sphere_pts[i][j].z = z;
        }
    }

    /*
     * Create the normals
     */
    for (i=0; i<SPHERE_XDIM; i++)
    {
        for (j=0; j<SPHERE_YDIM; j++)
        {
            sphere_nrm[i][j].x = (sphere_pts[i  ][j  ].x +
                                  sphere_pts[i  ][j+1].x +
                                  sphere_pts[i+1][j  ].x +
                                  sphere_pts[i+1][j+1].x) / 4.;
            sphere_nrm[i][j].y = (sphere_pts[i  ][j  ].y +
                                  sphere_pts[i  ][j+1].y +
                                  sphere_pts[i+1][j  ].y +
                                  sphere_pts[i+1][j+1].y) / 4.;
            sphere_nrm[i][j].z = (sphere_pts[i  ][j  ].z +
                                  sphere_pts[i  ][j+1].z +
                                  sphere_pts[i+1][j  ].z +
                                  sphere_pts[i+1][j+1].z) / 4.;
        }
    }

    /*
     * Create the elements
     */
    color white = {1.,1.,1.,1.};
    sphere.set_default_color(white);
    for (i=0; i<SPHERE_XDIM; i++)
    {
        for (j=0; j<SPHERE_YDIM; j++)
        {
            sphere.add_tri_n(sphere_pts[i  ][j  ],
                             sphere_pts[i  ][j+1],
                             sphere_pts[i+1][j  ],
                             sphere_nrm[i][j]);
            sphere.add_tri_n(sphere_pts[i+1][j+1],
                             sphere_pts[i  ][j+1],
                             sphere_pts[i+1][j  ],
                             sphere_nrm[i][j]);
        }
    }
}

/*---------------------------------------------------------------------------*
 * Function  : initializeCube()
 *
 * Purpose   : Initalize the cube used as a frame of reference.
 *
 * Programmer: Jeremy Meredith
 * Date      : October 27, 1999
 *
 * Modifications:
 *   Brad Whitlock, Mon Mar 3 13:34:29 PST 2003
 *   I made it a member of the QvisLightingWidget class and I changed how
 *   the object is contained.
 *
 *---------------------------------------------------------------------------*/

void
QvisLightingWidget::initializeCube()
{
    float s = cube_size;

    color white = {1., 1., 1., 1.};

    cube.set_default_color(white);
    cube.add_line_c(-s,-s,-s,  -s,-s, s,  0,0,1); /* red x axis */
    cube.add_line_c(-s, s,-s,  -s, s, s);
    cube.add_line_c( s,-s,-s,   s,-s, s);
    cube.add_line_c( s, s,-s,   s, s, s);

    cube.add_line_c(-s,-s,-s,  -s, s,-s,  0,1,0); /* grn y axis */
    cube.add_line_c(-s,-s, s,  -s, s, s);
    cube.add_line_c( s,-s,-s,   s, s,-s);
    cube.add_line_c( s,-s, s,   s, s, s);

    cube.add_line_c(-s,-s,-s,   s,-s,-s,  1,0,0); /* blu z axis */
    cube.add_line_c(-s,-s, s,   s,-s, s);
    cube.add_line_c(-s, s,-s,   s, s,-s);
    cube.add_line_c(-s, s, s,   s, s, s);
}
