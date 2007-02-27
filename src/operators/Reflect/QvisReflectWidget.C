/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisReflectWidget.h>
#include <math.h>
#include <qdrawutil.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>

#include <mini3D.h>

#include <visitstream.h>

//
// Constants and defines.
//

const float axes_size = 10.;

#define ARROW_ID 10
#define NUM_ARROWS         1
#define ARROW_NUMSIDES     20
#define ARROW_LENGTH       12.f
#define ARROW_LENGTH_DIVISIONS 5
#define ARROW_HEAD_LENGTH  (0.333f * ARROW_LENGTH)
#define ARROW_HEAD_RADIUS  (ARROW_HEAD_LENGTH * 0.5f)
#define ARROW_SHAFT_RADIUS (ARROW_HEAD_RADIUS * 0.4f)
#define CELLS_PER_ARROW    (4 * ARROW_NUMSIDES)


#define ANIMATION_NFRAMES 20
#define ANIMATION_DELTA   (1. / float(ANIMATION_NFRAMES))

#define SPHERE_ON_XDIM  30
#define SPHERE_ON_YDIM  15
#define SPHERE_ON_RAD   3.5f

#define SPHERE_OFF_XDIM 20
#define SPHERE_OFF_YDIM 10
#define SPHERE_OFF_RAD  2.5f

#define CUBE_ON_NX    7
#define CUBE_ON_NY    7
#define CUBE_ON_SIZE  6.f

#define CUBE_OFF_NX   6
#define CUBE_OFF_NY   6
#define CUBE_OFF_SIZE 4.f

//
// Static members.
//
bool QvisReflectWidget::sharedElementsCreated = false;
m3d_complex_element QvisReflectWidget::axes;
m3d_complex_element QvisReflectWidget::axes2D;
m3d_complex_element QvisReflectWidget::onCube;
m3d_complex_element QvisReflectWidget::offCube;
m3d_complex_element QvisReflectWidget::onSphere;
m3d_complex_element QvisReflectWidget::offSphere;
m3d_complex_element QvisReflectWidget::arrow;

// ****************************************************************************
// Method: QvisReflectWidget::QvisReflectWidget
//
// Purpose: 
//   Constructor for the QvisReflectWidget class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 09:47:38 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Jun 23 16:50:18 PST 2003
//   I initialized the mode2D member.
//
// ****************************************************************************

QvisReflectWidget::QvisReflectWidget(QWidget *parent, const char *name) : 
    QWidget(parent, name), renderer(250,250)
{
    pixmap = 0;
    rendererCreated = false;
    mode2D = true;

    for(int i = 0; i < 8; ++i)
        octantOn[i] = false;
    originOctant = 0;
    octantOn[originOctant] = true;

    createSharedElements();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),
            this, SLOT(handleTimer()));
    cameraInterpolant = 0.;
    switchingCameras = false;
    activeCamera = 0;
}

// ****************************************************************************
// Method: QvisReflectWidget::~QvisReflectWidget
//
// Purpose: 
//   Destructor for the QvisReflectWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 09:48:28 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisReflectWidget::~QvisReflectWidget()
{
    deleteBackingPixmap();
}

// ****************************************************************************
// Method: QvisReflectWidget::sizeHint
//
// Purpose: 
//   Returns the widget's preferred size.
//
// Returns:    The widget's preferred size.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 09:51:38 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisReflectWidget::sizeHint() const
{
    return QSize(250,250);
}

// ****************************************************************************
// Method: QvisReflectWidget::sizePolicy
//
// Purpose: 
//   Returns the widget's size policy.
//
// Returns:    The widget's size policy.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 09:52:11 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QSizePolicy
QvisReflectWidget::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

// ****************************************************************************
// Method: QvisReflectWidget::setMode2D
//
// Purpose: 
//   Sets the 2d mode.
//
// Arguments:
//   val : The new 2d mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 23 16:51:40 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::setMode2D(bool val)
{
    if(mode2D != val)
    {
        mode2D = val;
        deleteBackingPixmap();
        update();
    }
}

// ****************************************************************************
// Method: QvisReflectWidget::getMode2D
//
// Purpose: 
//   Returns the 2d mode.
//
// Returns:    The 2d mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 23 16:51:49 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
QvisReflectWidget::getMode2D() const
{
    return mode2D;
}

// ****************************************************************************
// Method: QvisReflectWidget::createSharedElements
//
// Purpose: 
//   Creates the static geometry that is rendered when we draw the widget.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 09:52:39 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 24 16:32:45 PST 2003
//   I added a call to initializeAxes2D.
//
// ****************************************************************************

void
QvisReflectWidget::createSharedElements()
{
    if(!sharedElementsCreated)
    {
        initializeAxes();
        initializeAxes2D();
        initializeSphere(onSphere, SPHERE_ON_XDIM, SPHERE_ON_YDIM,
                         SPHERE_ON_RAD, 1., 0., 1.);
        initializeSphere(offSphere, SPHERE_OFF_XDIM, SPHERE_OFF_YDIM,
                         SPHERE_OFF_RAD,
                         float(colorGroup().background().red()) / 255.,
                         float(colorGroup().background().green()) / 255.,
                         float(colorGroup().background().blue()) / 255.);

        initializeCube(onCube, CUBE_ON_NX, CUBE_ON_NY, CUBE_ON_SIZE,
                       0., 1., 0.);
        initializeCube(offCube, CUBE_OFF_NX, CUBE_OFF_NY, CUBE_OFF_SIZE,
                       float(colorGroup().background().red()) / 255.,
                       float(colorGroup().background().green()) / 255.,
                       float(colorGroup().background().blue()) / 255.);
        initializeArrow();

        sharedElementsCreated = true;
    }
}

// ****************************************************************************
// Method: QvisReflectWidget::deleteBackingPixmap
//
// Purpose: 
//   Deletes the backing pixmap.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 09:53:21 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::deleteBackingPixmap()
{
    if(pixmap)
    {
        delete pixmap;
        pixmap = 0;
    }
}

// ****************************************************************************
// Method: QvisReflectWidget::redrawScene
//
// Purpose: 
//   Redraws the scene using the specified painter.
//
// Arguments:
//   painter : The painter that we're using to redraw the scene.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 09:53:56 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Jun 23 16:56:48 PST 2003
//   I split out the code that draws 3d into its own method and added code
//   to draw the 2d scene too.
//
// ****************************************************************************

void
QvisReflectWidget::redrawScene(QPainter *painter)
{
    if(!rendererCreated)
    {
        renderer.resize(width(), height());
        renderer.set_specular_enabled(true);
        renderer.set_backface_culling(true);

        matrix4 proj = m3du_create_proj_matrix(1,1000,3.14159f/4.f);
        renderer.set_proj_matrix(proj);
        rendererCreated = true;
    }

    if(mode2D)
        redrawScene2D(painter);
    else
        redrawScene3D(painter);
}

// ****************************************************************************
// Method: QvisReflectWidget::redrawScene2D
//
// Purpose: 
//   Redraws the reflection widget in its 2d mode.
//
// Arguments:
//   painter : The painter to use to draw the widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 23 16:57:31 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Aug 22 09:02:18 PDT 2003
//   Changed how the background brush is selected so it works on MacOS X.
//
//   Hank Childs, Thu Jun  8 14:08:18 PDT 2006
//   Fix compiler warnings for casting.
//
// ****************************************************************************

void
QvisReflectWidget::redrawScene2D(QPainter *painter)
{
    // Fill in the background color.
    painter->fillRect(rect(), colorGroup().brush(QColorGroup::Background));

    //
    // Set up the camera.
    //
    vector3 camera = vec_create(0, 0, 35.);
    matrix4 view = m3du_create_view_matrix(
            camera,
            vec_create(0,0,0),
            vec_create(0,1,0));
    renderer.set_view_matrix(view);
    renderer.set_view_reference_point(camera);

    // Set up the lights.
    renderer.set_light(1, M3D_LIGHT_AMB, 0., 0., 0., 0.1f, 0.1f, 0.1f);
    renderer.set_light(2, M3D_LIGHT_EYE, -35.f, -35.f, -50.f, 0.7f, 0.7f, 0.7f);
    renderer.begin_scene(painter);
    renderer.set_world_matrix(m3du_create_identity_matrix());

    // Draw the reference axes
    axes2D.addToRenderer(renderer);

    // Draw the on/off actors.
    drawOnOffActors(4, 1.2f);

    // Render the scene
    renderer.end_scene();

    // Draw the Axis labels.
    int h = fontMetrics().height();
    vector3 x0 = renderer.transform_world_point(vec_create(-axes_size, 0, axes_size));
    vector3 x1 = renderer.transform_world_point(vec_create(axes_size, 0, axes_size));
    vector3 y0 = renderer.transform_world_point(vec_create(0, axes_size, axes_size));
    vector3 y1 = renderer.transform_world_point(vec_create(0, -axes_size, axes_size));
    painter->setPen(colorGroup().foreground());
    const char *x = "+X";
    painter->drawText((int) x0.x, (int) (x0.y + h), "-X");
    painter->drawText((int) (x1.x - fontMetrics().width(x)), (int) (x1.y + h), x);
    painter->drawText((int) (y0.x + 5), (int) (y0.y + h), "+Y");
    painter->drawText((int) (y1.x + 5), (int) (y1.y), "-Y");
}

// ****************************************************************************
// Method: QvisReflectWidget::drawOnOffActors
//
// Purpose: 
//   Draws the on/off actors (the cubes, sphere).
//
// Arguments:
//   n     : Number of things to draw.
//   scale : How big to draw everything.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 24 17:39:47 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::drawOnOffActors(int n, float scale)
{
    // Draw the cubes or spheres.
    for(int i = 0; i < n; ++i)
    {
        if(i == originOctant)
        {
            if(octantOn[i])
            {
                ScaleTranslateFromOriginToOctant(i, scale);
                onSphere.addToRenderer(renderer, i);
            }
            else
            {
                ScaleTranslateFromOriginToOctant(i, scale);
                offSphere.addToRenderer(renderer, i);
            }
        }
        else if(octantOn[i])
        {
            ScaleTranslateFromOriginToOctant(i, scale);
            onCube.addToRenderer(renderer, i);
        }
        else
        {
            ScaleTranslateFromOriginToOctant(i, scale);
            offCube.addToRenderer(renderer, i);
        }
    }
}

// ****************************************************************************
// Method: QvisReflectWidget::redrawScene3D
//
// Purpose: 
//   Redraws the scene in its 3d mode.
//
// Arguments:
//   painter : The painter used to redraw the scene.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 23 16:58:09 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::redrawScene3D(QPainter *painter)
{
    setupCamera();

    // This redraws the show!
    setupAndDraw(painter);
}

// ****************************************************************************
// Method: QvisReflectWidget::setupCamera
//
// Purpose: 
//   Sets up the camera based on the current camera and the camera interpolant.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 09:54:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::setupCamera()
{
    // Set the view.
    vector3 camera1 = vec_create(24.2, 16.5, 29.7);
    vector3 camera2 = vec_create(-24.2, 16.5, -29.7);

#ifdef DONT_KNOW_THE_VALUES
    // we're debugging
    float r = sqrt(camera1.x * camera1.x + camera1.z * camera1.z);
    float camera1Angle = -acos(camera1.x / r) + 3.14159 / 2.;
    float camera2Angle = camera1Angle + 3.14159;
#else
    // we know the values.
    const float r = 38.309f;
    const float camera1Angle = 0.683708f;
    const float camera2Angle = 3.8253f;
#endif

    float angle = (1. - cameraInterpolant) * camera1Angle + 
                  cameraInterpolant * camera2Angle;

    // figure out x and z using the angle.
    float x = r * cos(angle);
    float z = r * sin(angle);

    vector3 camera = vec_create(x, camera1.y, z);

    matrix4 view = m3du_create_view_matrix(
            camera,
            vec_create(0,-3,0),
            vec_create(0,1,0));
    renderer.set_view_matrix(view);
    renderer.set_view_reference_point(camera);
}

// ****************************************************************************
// Method: QvisReflectWidget::setupAndDraw
//
// Purpose: 
//   Sets up the scene and draws it using the specified painter.
//
// Arguments:
//   p : The painter that we use to draw the scene.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 09:57:09 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 24 14:34:02 PST 2003
//   I made it use drawOnOffActors.
//
//   Brad Whitlock, Wed Sep 10 08:44:35 PDT 2003
//   I added the axis labels for the back view.
//
//   Brad Whitlock, Fri Aug 22 09:03:06 PDT 2003
//   I changed how the brush is selected so it works on MacOS X.
//
//   Hank Childs, Thu Jun  8 14:08:18 PDT 2006
//   Fix compiler warnings for casting.
//
// ****************************************************************************

void
QvisReflectWidget::setupAndDraw(QPainter *p)
{
    // Fill in the background color.
    p->fillRect(rect(), colorGroup().brush(QColorGroup::Background));

    renderer.set_light(1, M3D_LIGHT_EYE, 0.f,0.f,-35.f, 1.f, 1.f, 1.f);
    renderer.set_light(2, M3D_LIGHT_OFF, 0.f,0.f,-1.f, 0.f, 0.f, 0.f);
    renderer.begin_scene(p);
    renderer.set_world_matrix(m3du_create_identity_matrix());

    // Draw the reference axes
    axes.addToRenderer(renderer);

    // Draw the cubes or spheres.
    drawOnOffActors(8, 1.f);

    // If we're not switching cameras, add the arrow to the scene.
    if(!switchingCameras)
    {
        if(activeCamera == 0)
        {
            matrix4 world = mtx_mult(
                m3du_create_scaling_matrix(1., 1., -1.),
                m3du_create_translation_matrix(axes_size + 3, -axes_size,
                    -ARROW_LENGTH * 0.5f));
            renderer.set_world_matrix(world);
            arrow.addToRenderer(renderer, ARROW_ID);
        }
        else
        {
            matrix4 world = mtx_mult(
                m3du_create_scaling_matrix(1., 1., -1.),
                m3du_create_translation_matrix(-axes_size - 3,
                    -axes_size, -ARROW_LENGTH * 0.5));
            renderer.set_world_matrix(world);
            arrow.addToRenderer(renderer, ARROW_ID);
        }
    }

    // Render the scene
    renderer.end_scene();

    // Add some annotation
    if(!switchingCameras)
    {
        int cx = width() / 2;
        int cy = height() / 2;
        int edge = QMIN(width(), height());
        QRect square(cx - edge / 2, cy - edge / 2, edge, edge);

        if(activeCamera == 0)
        {
            const char *txt = "Front view";
            p->drawText(square.x() + 5, square.y() + square.height() - 5, txt);

            vector3 p0 = renderer.transform_world_point(vec_create(axes_size, 0, 0));
            vector3 p1 = renderer.transform_world_point(vec_create(0, axes_size, 0));
            vector3 p2 = renderer.transform_world_point(vec_create(0, 0, axes_size));

            p->drawText((int) (p0.x + 5), (int) (p0.y + 5), "+X");
            p->drawText((int) (p1.x), (int) (p1.y - 5), "+Y");
            p->drawText((int) (p2.x - 20), (int) (p2.y + 5), "+Z");
        }
        else
        {
            const char *txt = "Back view";
            p->drawText(square.x() + 5, square.y() + square.height() - 5, txt);

            vector3 p0 = renderer.transform_world_point(vec_create(-axes_size, 0, 0));
            vector3 p1 = renderer.transform_world_point(vec_create(0, axes_size, 0));
            vector3 p2 = renderer.transform_world_point(vec_create(0, 0, -axes_size));

            p->drawText((int) (p0.x + 5), (int) (p0.y + 5), "-X");
            p->drawText((int) (p1.x), (int) (p1.y - 5), "+Y");
            p->drawText((int) (p2.x - 20), (int) (p2.y + 5), "-Z");
        }
    }
}

// ****************************************************************************
// Method: QvisReflectWidget::ScaleTranslateFromOriginToOctant
//
// Purpose: 
//   Sets the world matrix so that the origin is translated to the center
//   of the specified octant.
//
// Arguments:
//   octant : The octant to which we're translating.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 10:25:52 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 24 14:29:45 PST 2003
//   I renamed the method and let it also scale the matrix.
//
// ****************************************************************************

void
QvisReflectWidget::ScaleTranslateFromOriginToOctant(int octant, float s)
{
    matrix4 scale = m3du_create_scaling_matrix(s,s,s);
    matrix4 translate;

    // Translate from the origin to the center of the specified octant.
    switch(octant)
    {
    case 0:
        translate = m3du_create_translation_matrix(5,5,5);
        break;
    case 1:
        translate = m3du_create_translation_matrix(-5,5,5);
        break;
    case 2:
        translate = m3du_create_translation_matrix(-5,-5,5);
        break;
    case 3:
        translate = m3du_create_translation_matrix(5,-5,5);
        break;
    case 4:
        translate = m3du_create_translation_matrix(5,5,-5);
        break;
    case 5:
        translate = m3du_create_translation_matrix(-5,5,-5);
        break;
    case 6:
        translate = m3du_create_translation_matrix(-5,-5,-5);
        break;
    case 7:
        translate = m3du_create_translation_matrix(5,-5,-5);
        break;
    }

    // Set the transform
    renderer.set_world_matrix(mtx_mult(scale, translate));
}

// ****************************************************************************
// Method: QvisReflectWidget::setValues
//
// Purpose: 
//   Sets the on/off flags for all 8 octants.
//
// Arguments:
//   octants : The new on/off flags for all 8 octants.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 10:26:53 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::setValues(bool *octants)
{
    bool different = false;

    for(int i = 0; i < 8; ++i)
    {
        different |= (octantOn[i] != octants[i]);
        octantOn[i] = octants[i];
    }

    if(different)
    {
        deleteBackingPixmap();
        update();

        emit valueChanged(octantOn);
    }
}

// ****************************************************************************
// Method: QvisReflectWidget::setOriginalOctant
//
// Purpose: 
//   Sets the original octant (the one with the ball)
//
// Arguments:
//   octant : The new original octant.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 10:27:49 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::setOriginalOctant(int octant)
{
    if(octant != originOctant)
    {
        originOctant = octant;

        deleteBackingPixmap();
        update();

        emit octantChanged(originOctant);
    }
}

// ****************************************************************************
// Method: QvisReflectWidget::mouseReleaseEvent
//
// Purpose: 
//   This is an event handling function that is called when the mouse is
//   released in the widget.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 10:28:25 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Jun 23 17:00:31 PST 2003
//   I added code to handle mouse release events differently when the window
//   is in 2d mode.
//
// ****************************************************************************

void
QvisReflectWidget::mouseReleaseEvent(QMouseEvent *e)
{
    int id = renderer.probe(e->x(), e->y());

    if(id >= 0 && id < 8)
    {
        // id happens to be the octant that was clicked.
        octantOn[id] = !octantOn[id];

        deleteBackingPixmap();
        update();

        // Tell others about the new selections.
        emit valueChanged(octantOn);
    }
    else if (!mode2D && !switchingCameras && id == ARROW_ID)
    {
        // We clicked on the arrow. Start changing cameras.
        if(activeCamera == 1)
        {
            activeCamera = 0;
            switchingCameras = true;
            cameraInterpolant = 1. - ANIMATION_DELTA;
            timer->start(1000 / ANIMATION_NFRAMES);
        }
        else
        {
            activeCamera = 1;
            switchingCameras = true;
            cameraInterpolant = ANIMATION_DELTA;
            timer->start(1000 / ANIMATION_NFRAMES);
        }
    }
}

// ****************************************************************************
// Method: QvisReflectWidget::handleTimer
//
// Purpose: 
//   This is an internal slot function that is called when the animation timer
//   is activated. We use it to update the camera location.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 10:30:43 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::handleTimer()
{
    deleteBackingPixmap();
    update();

    if(activeCamera == 0) 
        cameraInterpolant -= ANIMATION_DELTA;
    else
        cameraInterpolant += ANIMATION_DELTA;

    if(cameraInterpolant >= 1.)
    {
        switchingCameras = false;
        timer->stop();
        cameraInterpolant = 1.;
    }
    else if(cameraInterpolant <= 0.)
    {
        switchingCameras = false;
        timer->stop();
        cameraInterpolant = 0.;
    }
}

// ****************************************************************************
// Method: QvisReflectWidget::paintEvent
//
// Purpose: 
//   This method is called when the widget needs to paint itself.
//
// Arguments:
//   e : The paint event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 10:31:51 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::paintEvent(QPaintEvent *e)
{
    bool clipByRegion = true;

    // Draw the scene into the backing pixmap.
    if(pixmap == 0)
    {
        pixmap = new QPixmap(width(), height());
        QPainter pixpaint(pixmap);
        redrawScene(&pixpaint);
        setBackgroundPixmap(*pixmap);
        clipByRegion = false;
    }

    // Blit the pixmap to the screen.
    QPainter paint(this);
    if(clipByRegion && !e->region().isEmpty() && !e->region().isNull())
        paint.setClipRegion(e->region());
    paint.drawPixmap(QPoint(0,0), *pixmap);
}

// ****************************************************************************
// Method: QvisReflectWidget::resizeEvent
//
// Purpose: 
//   This method is called when the widget is resized.
//
// Arguments:
//   e : The resize event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 10:32:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::resizeEvent(QResizeEvent *e)
{
    deleteBackingPixmap();
    renderer.resize(e->size().width(), e->size().height());
}

// ****************************************************************************
// Method: QvisReflectWidget::initializeAxes2D
//
// Purpose: 
//   Initializes the 2D axes elements.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 24 17:05:22 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::initializeAxes2D()
{
    float s = axes_size;

    color r = {1., 0., 0.};
    axes2D.set_default_color(r);
    axes2D.set_default_line_style(DOT_LINE);
    axes2D.add_line_c(-s, 0, s,  0, 0, s);
    axes2D.set_default_line_style(SOLID_LINE);
    axes2D.add_line_c(0, 0, s,  s, 0, s);

    color g = {0., 1., 0.};
    axes2D.set_default_color(g);
    axes2D.add_line_c(0, s, s,  0, 0, s);
    axes2D.set_default_line_style(DOT_LINE);
    axes2D.add_line_c( 0, 0, s, 0, -s, s);
}

// ****************************************************************************
// Method: initializeAxes
//
// Purpose: 
//   Initializes the axes elements.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 3 10:38:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::initializeAxes()
{
    float s = axes_size;

    axes.set_default_line_style(DOT_LINE);
    color c;
    c.r = float(colorGroup().foreground().red()) / 255.;
    c.r = float(colorGroup().foreground().green()) / 255.;
    c.b = float(colorGroup().foreground().blue()) / 255.;

    axes.set_default_color(c);
    axes.add_line_c(-s,-s,-s,  -s,-s, s);
    axes.add_line_c(-s, s,-s,  -s, s, s);
    axes.add_line_c( s,-s,-s,   s,-s, s);
    axes.add_line_c( s, s,-s,   s, s, s);

    axes.add_line_c(-s,-s,-s,  -s, s,-s);
    axes.add_line_c(-s,-s, s,  -s, s, s);
    axes.add_line_c( s,-s,-s,   s, s,-s);
    axes.add_line_c( s,-s, s,   s, s, s);

    axes.add_line_c(-s,-s,-s,   s,-s,-s);
    axes.add_line_c(-s,-s, s,   s,-s, s);
    axes.add_line_c(-s, s,-s,   s, s,-s);
    axes.add_line_c(-s, s, s,   s, s, s);

    axes.set_default_line_style(SOLID_LINE);
    axes.set_default_line_segments(10);
    axes.add_line_c(-s, 0, -s,  -s, 0,  s);
    axes.add_line_c(-s, 0,  s,   s, 0,  s);
    axes.add_line_c( s, 0,  s,   s, 0, -s);
    axes.add_line_c( s, 0, -s,  -s, 0, -s);

    axes.add_line_c(0, -s, -s,  0, -s,  s);
    axes.add_line_c(0, -s,  s,  0,  s,  s);
    axes.add_line_c(0,  s,  s,  0,  s, -s);
    axes.add_line_c(0,  s, -s,  0, -s, -s);

    axes.add_line_c(-s, -s, 0,  s,  -s, 0);
    axes.add_line_c(s,  -s, 0,  s,  s, 0);
    axes.add_line_c(s,  s, 0,  -s,  s, 0);
    axes.add_line_c(-s,  s, 0,  -s,  -s, 0);

    axes.set_default_line_style(SOLID_LINE);
    axes.add_line_c(0,0,0,  s,0,0,  1,0,0);
    axes.add_line_c(0,0,0,  0,s,0,  0,1,0);
    axes.add_line_c(0,0,0,  0,0,s,  0,0,1);

    axes.set_default_line_style(DOT_LINE);
    axes.add_line_c(0,0,0,  -s,0,0,  1,0,0);
    axes.add_line_c(0,0,0,  0,-s,0,  0,1,0);
    axes.add_line_c(0,0,0,  0,0,-s,  0,0,1);
}

// ****************************************************************************
// Method: QvisReflectWidget::initializeSphere
//
// Purpose: 
//   Initializes a sphere object.
//
// Arguments:
//   obj    : The object to initialize.
//   nx, ny : Number of divisions.
//   rad    : The radius
//   r,g,b  : The object color.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 10:33:19 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::initializeSphere(m3d_complex_element &obj, int nx, int ny,
    float rad, float r, float g, float b)
{
    int i,j;
    vector3 sphere_pts[SPHERE_ON_XDIM+1][SPHERE_ON_YDIM+1];
    vector3 sphere_nrm[SPHERE_ON_XDIM  ][SPHERE_ON_YDIM  ];

    /*
     * Create vertex arrays
     */
    for (i=0; i<=nx; i++)
    {
        for (j=0; j<=ny; j++)
        {
            float p  = (float)i/(float)nx;
            float q  = (float)j/(float)ny;
            float pi = 3.14159265f;

            float f = sin(q * pi);
            float z = cos(q * pi)*rad;
            float x = cos(p*2*pi)*rad * f;
            float y = sin(p*2*pi)*rad * f;

            sphere_pts[i][j].x = x;
            sphere_pts[i][j].y = y;
            sphere_pts[i][j].z = z;
        }
    }

    /*
     * Create the normals
     */
    for (i=0; i<nx; i++)
    {
        for (j=0; j<ny; j++)
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
    color c = {r, g, b, 1.};
    obj.set_default_color(c);
    for (i=0; i<nx; i++)
    {
        for (j=0; j<ny; j++)
        {
            obj.add_tri_n(sphere_pts[i  ][j  ],
                          sphere_pts[i  ][j+1],
                          sphere_pts[i+1][j  ],
                          sphere_nrm[i][j]);

            obj.add_tri_n(sphere_pts[i+1][j+1],
                          sphere_pts[i  ][j+1],
                          sphere_pts[i+1][j  ],
                          sphere_nrm[i][j]);
        }
    }
}

// ****************************************************************************
// Method: QvisReflectWidget::initializeCube
//
// Purpose: 
//   Creates a rounded cube.
//
// Arguments:
//   obj   : The object to initialize.
//   nx,ny : The number of divisions.
//   scale : The size of a side.
//   r,g,b : The object color.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 10:34:35 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::initializeCube(m3d_complex_element &obj, int nx, int ny,
    float scale, float r, float g, float b)
{
static float verts[8][3] = {
{0, 0, 0},
{0, 0, 1},
{0, 1, 0},
{0, 1, 1},
{1, 0, 0},
{1, 0, 1},
{1, 1, 0},
{1, 1, 1}};

#define ROOT3 1.7320508f

static int faces[6][4] = {
{1,5,7,3},
{5,4,6,7},
{4,0,2,6},
{0,1,3,2},
{0,4,5,1},
{3,7,6,2}};

    color cubeColor = {r, g, b, 1.};
    obj.set_default_color(cubeColor);

    for(int i = 0; i < 6; ++i)
    {
        float *v0 = verts[faces[i][0]];
        float *v1 = verts[faces[i][1]];
        float *v2 = verts[faces[i][2]];
        float *v3 = verts[faces[i][3]];

        for(int j = 0; j < ny - 1; ++j)
        {
            int c;
            float tj = float(j) / float(ny - 1);
            float tj1 = float(j+1) / float(ny - 1);

            float m0[3], m1[3], m2[3], m3[3]; // points
            for(c = 0; c < 3; ++c)
            {
                // points
                m0[c] = (1. - tj) * v0[c] + tj * v3[c];
                m1[c] = (1. - tj) * v1[c] + tj * v2[c];
                m2[c] = (1. - tj1) * v1[c] + tj1 * v2[c];
                m3[c] = (1. - tj1) * v0[c] + tj1 * v3[c];
            }

            for(int k = 0; k < nx - 1; ++k)
            {
                float tk = float(k) / float(nx - 1);
                float tk1 = float(k+1) / float(nx - 1);

                float p0[3], p1[3], p2[3], p3[3];
                for(c = 0; c < 3; ++c)
                {
                    // points
                    p0[c] = (1. - tk) * m0[c] + tk * m1[c] - 0.5;
                    p1[c] = (1. - tk1) * m0[c] + tk1 * m1[c] - 0.5;
                    p2[c] = (1. - tk1) * m3[c] + tk1 * m2[c] - 0.5;
                    p3[c] = (1. - tk) * m3[c] + tk * m2[c] - 0.5;
                }

                // Distance from the center to each point
                float p0d = sqrt(p0[0]*p0[0] + p0[1]*p0[1] + p0[2]*p0[2]);
                float p1d = sqrt(p1[0]*p1[0] + p1[1]*p1[1] + p1[2]*p1[2]);
                float p2d = sqrt(p2[0]*p2[0] + p2[1]*p2[1] + p2[2]*p2[2]);
                float p3d = sqrt(p3[0]*p3[0] + p3[1]*p3[1] + p3[2]*p3[2]);

                float deltas[4];
                deltas[0] = (ROOT3/2. - p0d) / 3.;
                deltas[1] = (ROOT3/2. - p1d) / 3.;
                deltas[2] = (ROOT3/2. - p2d) / 3.;
                deltas[3] = (ROOT3/2. - p3d) / 3.;

                // Deform the box.
                switch(i)
                {
                case 0:
                    if (k > 0 && j > 0) p0[2] += deltas[0];
                    if (j > 0 && k < nx - 2) p1[2] += deltas[1];
                    if (k < nx-2 && j < ny - 2) p2[2] += deltas[2];
                    if (k > 0 && j < ny - 2) p3[2] += deltas[3];
                    break;
                case 1:
                    if (k > 0 && j > 0) p0[0] += deltas[0];
                    if (j > 0 && k < nx - 2) p1[0] += deltas[1];
                    if (k < nx-2 && j < ny - 2) p2[0] += deltas[2];
                    if (k > 0 && j < ny - 2) p3[0] += deltas[3];
                    break;
                case 2:
                    if (k > 0 && j > 0) p0[2] -= deltas[0];
                    if (j > 0 && k < nx - 2) p1[2] -= deltas[1];
                    if (k < nx-2 && j < ny - 2) p2[2] -= deltas[2];
                    if (k > 0 && j < ny - 2) p3[2] -= deltas[3];
                    break;
                case 3:
                    if (k > 0 && j > 0) p0[0] -= deltas[0];
                    if (j > 0 && k < nx - 2) p1[0] -= deltas[1];
                    if (k < nx-2 && j < ny - 2) p2[0] -= deltas[2];
                    if (k > 0 && j < ny - 2) p3[0] -= deltas[3];
                    break;
                case 4:
                    if (k > 0 && j > 0) p0[1] -= deltas[0];
                    if (j > 0 && k < nx - 2) p1[1] -= deltas[1];
                    if (k < nx-2 && j < ny - 2) p2[1] -= deltas[2];
                    if (k > 0 && j < ny - 2) p3[1] -= deltas[3];
                    break;
                case 5:
                    if (k > 0 && j > 0) p0[1] += deltas[0];
                    if (j > 0 && k < nx - 2) p1[1] += deltas[1];
                    if (k < nx-2 && j < ny - 2) p2[1] += deltas[2];
                    if (k > 0 && j < ny - 2) p3[1] += deltas[3];
                    break;
                }

                for(c = 0; c < 3; ++c)
                {
                    // transform the points.
                    p0[c] *= scale;
                    p1[c] *= scale;
                    p2[c] *= scale;
                    p3[c] *= scale;
                }

                // Shrink the corners so they are not so pointy.
#define SHRINK 0.925f
                if(k == 0 && j == 0)
                {
                    p0[0] *= SHRINK; p0[1] *= SHRINK; p0[2] *= SHRINK;
                }
                else if(k == 0 && j == ny - 2)
                {
                    p3[0] *= SHRINK; p3[1] *= SHRINK; p3[2] *= SHRINK;
                }
                else if(k == nx - 2 && j == 0)
                {
                    p1[0] *= SHRINK; p1[1] *= SHRINK; p1[2] *= SHRINK;
                }
                else if(k == nx - 2 && j == ny - 2)
                {
                    p2[0] *= SHRINK; p2[1] *= SHRINK; p2[2] *= SHRINK;
                }

                int split = 0;
                int ny2 = (ny - 1) / 2;
                int nx2 = (nx - 1) / 2;
                if(j < ny2 && k < nx2)
                    split = 1;
                else if(j > ny2 && k > nx2)
                    split = 1;

#define ADD_TRI(A, B, C) \
    norm = vec_normalize(vec_cross( \
        vec_create(B[0] - A[0], B[1] - A[1], B[2] - A[2]), \
        vec_create(C[0] - A[0], C[1] - A[1], C[2] - A[2]))); \
    obj.add_tri_n(A[0], A[1], A[2], \
                  B[0], B[1], B[2], \
                  C[0], C[1], C[2], \
                  norm.x, norm.y, norm.z);

                vector3 norm;
                if(split == 0)
                {
                    // Do a 013 and 123 split.
                    ADD_TRI(p0, p1, p3);
                    ADD_TRI(p1, p2, p3);
                }
                else
                {
                    // Do a 012 and 023 split.
                    ADD_TRI(p0, p1, p2);
                    ADD_TRI(p0, p2, p3);
                }
            }
        }
    }
}

// ****************************************************************************
// Method: QvisReflectWidget::initializeArrow
//
// Purpose: 
//   Initializes the arrow object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 10:36:49 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisReflectWidget::initializeArrow()
{
    double dTheta = (2. * 3.14159) / double(ARROW_NUMSIDES);
    int numPtsPerVector = (ARROW_LENGTH_DIVISIONS + 1) * ARROW_NUMSIDES + 2;
    int numPts = (NUM_ARROWS * numPtsPerVector);
    vector3 *pts = new vector3[numPts];

    //
    // Create the points.
    //
    int    i, j, ptIndex = 0;
    double theta;

    float startZ = 0.;
    float endZ = (ARROW_LENGTH - ARROW_HEAD_LENGTH);

    for(j = 0; j < ARROW_LENGTH_DIVISIONS; ++j)
    {
        float t = float(j) / float(ARROW_LENGTH_DIVISIONS - 1);
        float z = (1. - t) * startZ + t * endZ;

        for(i = 0, theta = 0.; i < ARROW_NUMSIDES; ++i, theta += dTheta)
        {
            pts[ptIndex].x = cos(theta) * ARROW_SHAFT_RADIUS;
            pts[ptIndex].y = sin(theta) * ARROW_SHAFT_RADIUS;
            pts[ptIndex].z = z;
            ++ptIndex;
        }
    }
    int capStart = ptIndex;
    for(i = 0, theta = 0.; i < ARROW_NUMSIDES; ++i, theta += dTheta)
    {
        pts[ptIndex].x = cos(theta) * ARROW_HEAD_RADIUS;
        pts[ptIndex].y = sin(theta) * ARROW_HEAD_RADIUS;
        pts[ptIndex].z = (ARROW_LENGTH - ARROW_HEAD_LENGTH) - (0.02 * ARROW_LENGTH);
        ++ptIndex;
    }
    pts[ptIndex].x = 0.;
    pts[ptIndex].y = 0.;
    pts[ptIndex].z = 0.;
    int startPoint = ptIndex;
    ++ptIndex;

    pts[ptIndex].x = 0.;
    pts[ptIndex].y = 0.;
    pts[ptIndex].z = ARROW_LENGTH;
    int endPoint = ptIndex;
    ++ptIndex;

#define ADD_TRIANGLE_N() \
    vector3 *p0 = pts + ptIds[0]; \
    vector3 *p1 = pts + ptIds[1]; \
    vector3 *p2 = pts + ptIds[2]; \
    vector3 p1p0 = vec_sub(*p1, *p0); \
    vector3 p2p0 = vec_sub(*p2, *p0); \
    vector3 norm = vec_normalize(vec_cross(p2p0, p1p0)); \
    arrow.add_tri_n(*p0, *p1, *p2, norm);

#define ADD_QUAD_N() \
    vector3 *p0 = pts + ptIds[0]; \
    vector3 *p1 = pts + ptIds[1]; \
    vector3 *p2 = pts + ptIds[2]; \
    vector3 *p3 = pts + ptIds[3]; \
    vector3 p1p0 = vec_sub(*p1, *p0); \
    vector3 p2p0 = vec_sub(*p2, *p0); \
    vector3 norm = vec_normalize(vec_cross(p1p0, p2p0)); \
    arrow.add_tri_n(*p0, *p1, *p2, norm); \
    arrow.add_tri_n(*p2, *p1, *p3, norm);


    color c = {0., 1., 1., 0.};
    arrow.set_default_color(c);

    //
    // Create the endpoint cells.
    //
    int ptIds[4];
    for(i = 0; i < ARROW_NUMSIDES; ++i)
    {
        ptIds[0] = startPoint;
        ptIds[1] = i;
        ptIds[2] = ((i < (ARROW_NUMSIDES - 1)) ? (i + 1) : 0);
        ADD_TRIANGLE_N();
    }

    //
    // Create the shaft cells.
    //
    int offset = 0;
    for(j = 0; j < ARROW_LENGTH_DIVISIONS - 1; ++j)
    {
        for(i = 0; i < ARROW_NUMSIDES; ++i)
        {
            int index = offset + i;
            ptIds[0] = index;
            ptIds[1] = (i < (ARROW_NUMSIDES - 1)) ? (index + 1) : (j*ARROW_NUMSIDES);
            ptIds[2] = index + ARROW_NUMSIDES;
            ptIds[3] = (i < (ARROW_NUMSIDES - 1)) ? (index + ARROW_NUMSIDES + 1) : (j+1)*ARROW_NUMSIDES;

            ADD_QUAD_N();
        }

        offset += ARROW_NUMSIDES;
    }

    //
    // Create the head-back cells.
    //
    offset = ((ARROW_LENGTH_DIVISIONS - 1) * ARROW_NUMSIDES);
    for(i = 0; i < ARROW_NUMSIDES; ++i)
    {
        ptIds[0] = i + offset;
        ptIds[1] = ((i < (ARROW_NUMSIDES - 1)) ? (i + offset + 1) : offset);
        ptIds[2] = i + capStart;
        ptIds[3] = ((i < (ARROW_NUMSIDES - 1)) ? (i + capStart + 1) : capStart);

        ADD_QUAD_N();
    }

    //
    // Create the arrow front cells.
    //
    for(i = 0; i < ARROW_NUMSIDES; ++i)
    {
        ptIds[0] = i + capStart;
        ptIds[1] = endPoint;
        ptIds[2] = ((i < (ARROW_NUMSIDES - 1)) ? (i + capStart + 1) : capStart);
        ADD_TRIANGLE_N();
    }

    delete [] pts;
}
