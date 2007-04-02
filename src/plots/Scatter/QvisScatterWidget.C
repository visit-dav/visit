/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#include <QvisScatterWidget.h>
#include <math.h>
#include <qdrawutil.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>

#include <mini3D.h>

#define NUM_ARROWS         1
#define ARROW_NUMSIDES     20
#define ARROW_LENGTH       10.f
#define ARROW_LENGTH_DIVISIONS 5
#define ARROW_HEAD_LENGTH  (0.1f * ARROW_LENGTH)
#define ARROW_HEAD_RADIUS  (ARROW_HEAD_LENGTH * 0.5f)
#define ARROW_SHAFT_RADIUS (ARROW_HEAD_RADIUS * 0.4f)
#define CELLS_PER_ARROW    (4 * ARROW_NUMSIDES)

#define SPHERE_XDIM  9
#define SPHERE_YDIM  9
#define SPHERE_RAD   0.75f

#define ANIMATION_TIMEOUT 100
#define ANIMATION_NSTATES 10

inline color color_create(const float *c)
{
    color retval = {c[0], c[1], c[2]};
    return retval;
}

//
// Static members.
//
bool QvisScatterWidget::sharedElementsCreated = false;
m3d_complex_element QvisScatterWidget::sphere;
m3d_complex_element QvisScatterWidget::arrow;

// ****************************************************************************
// Method: QvisScatterWidget::QvisScatterWidget
//
// Purpose: 
//   Constructor for the QvisScatterWidget class.
//
// Arguments:
//   parent : The parent of this widget.
//   name   : This widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:17:50 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisScatterWidget::QvisScatterWidget(QWidget *parent, const char *name) : 
    QWidget(parent, name), renderer(250,250)
{
    pixmap = 0;
    pixmapDirty = true;
    rendererCreated = false;

    createSharedElements();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),
            this, SLOT(handleTimer()));
    animationProgress = 0;
    animationCountPositive = true;

    threeD = false;
    highlightedAxis = true;
    coloredPoints = false;
}

// ****************************************************************************
// Method: QvisScatterWidget::~QvisScatterWidget
//
// Purpose: 
//   Destructor for the QvisScatterWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:33:38 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisScatterWidget::~QvisScatterWidget()
{
    deleteBackingPixmap();
}

// ****************************************************************************
// Method: QvisScatterWidget::sizeHint
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
QvisScatterWidget::sizeHint() const
{
    return QSize(250,250);
}

// ****************************************************************************
// Method: QvisScatterWidget::sizePolicy
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
QvisScatterWidget::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

// ****************************************************************************
// Method: QvisScatterWidget::setThreeD
//
// Purpose: 
//   Sets whether the widget should draw a 3D scatter plot.
//
// Arguments:
//   val : True for 3D; false for 2D.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:34:09 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::setThreeD(bool val)
{
    if(threeD != val)
    {
        threeD = val;
        pixmapDirty = true;
        update();
    }
}

// ****************************************************************************
// Method: QvisScatterWidget::setHighlightAxis
//
// Purpose: 
//   Sets whether the axis should be highlighted (animated).
//
// Arguments:
//   val : The new highlight value.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:34:40 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::setHighlightAxis(bool val)
{
    if(highlightedAxis != val)
    {
        highlightedAxis = val;
        pixmapDirty = true;
        if(isVisible())
        {
            update();
            if(highlightedAxis)
            {
                animationProgress = 0;
                animationCountPositive = true;
                timer->start(ANIMATION_TIMEOUT);
            }
            else
                timer->stop();
        }
    }
}

// ****************************************************************************
// Method: QvisScatterWidget::setColoredPoints
//
// Purpose: 
//   Sets whether the points should be colored.
//
// Arguments:
//   val : Whether the scatter points should be colored.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:35:22 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::setColoredPoints(bool val)
{
    if(coloredPoints != val)
    {
        coloredPoints = val;
        pixmapDirty = true;
        update();
    }
}

// ****************************************************************************
// Method: QvisScatterWidget::createSharedElements
//
// Purpose: 
//   Called to initialize the objects that are shared for all instances.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:36:17 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::createSharedElements()
{
    if(!sharedElementsCreated)
    {
        initializeSphere(sphere, SPHERE_XDIM, SPHERE_YDIM,
                         SPHERE_RAD,
                         float(colorGroup().background().red()) / 255.,
                         float(colorGroup().background().green()) / 255.,
                         float(colorGroup().background().blue()) / 255.);
        initializeArrow();

        sharedElementsCreated = true;
    }
}

// ****************************************************************************
// Method: QvisScatterWidget::deleteBackingPixmap
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
QvisScatterWidget::deleteBackingPixmap()
{
    if(pixmap)
    {
        delete pixmap;
        pixmap = 0;
    }
}

// ****************************************************************************
// Method: QvisScatterWidget::redrawScene
//
// Purpose: 
//   Redraws the scene.
//
// Arguments:
//   painter : The painter to use to redraw the scene.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:37:26 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::redrawScene(QPainter *painter)
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

    // Fill in black and draw a beveled border.
    QBrush b(QColor(0,0,0));
    qDrawShadePanel(painter, 0, 0, width(), height(), colorGroup(), true,
                    2, &b);

    if(threeD)
        redrawScene3D(painter);
    else
        redrawScene2D(painter);
}

// ****************************************************************************
// Method: QvisScatterWidget::redrawScene2D
//
// Purpose: 
//   Redraw the scene in 2D.
//
// Arguments:
//   painter : The painter to use when redrawing the scene.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:37:59 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::redrawScene2D(QPainter *painter)
{
    //
    // Set up the camera.
    //
    vector3 camera = vec_create(5, 5, 14.);
    matrix4 view = m3du_create_view_matrix(
            camera,
            vec_create(5,5,0),
            vec_create(0,1,0));
    renderer.set_view_matrix(view);
    renderer.set_view_reference_point(camera);

    // Set up the lights.
    renderer.set_light(1, M3D_LIGHT_AMB, 0., 0., 0., 0.1f, 0.1f, 0.1f);
    renderer.set_light(2, M3D_LIGHT_EYE, -35.f, -35.f, -50.f, 0.7f, 0.7f, 0.7f);
    renderer.begin_scene(painter);
    renderer.set_world_matrix(m3du_create_identity_matrix());

    // Draw the X axis.
    matrix4 r = m3du_create_RBT_matrix(vec_create(0.,0.,0.),
        vec_create(1.,0.,0.), vec_create(0.,1.,0.));
    renderer.set_world_matrix(r);
    color c_red = {1.,0.,0.};
    arrow.addToRenderer(renderer, c_red);
    vector3 x_label_loc = renderer.transform_world_point(
        vec_create(ARROW_LENGTH, 0., 0.));
    renderer.set_world_matrix(m3du_create_identity_matrix());

    // Draw the Y axis.
    matrix4 m = m3du_create_RBT_matrix(vec_create(0.,0.,0.), 
        vec_create(0.,1.,0.), vec_create(1.,0.,0.));
    float lengthScale = 1.;
    if(highlightedAxis)
    {
        float scale = float(animationProgress) / float(ANIMATION_NSTATES-1);
        lengthScale = 0.75 + 0.25 * scale;
        matrix4 s = m3du_create_scaling_matrix(1.2, 1.2, lengthScale);
        m = mtx_mult(m, s);
    }
    renderer.set_world_matrix(m);
    color c_green = {0., 1., 0.};
    arrow.addToRenderer(renderer, c_green);
    vector3 y_label_loc = renderer.transform_world_point(
        vec_create(0., lengthScale * ARROW_LENGTH, 0.));

    renderer.set_world_matrix(m3du_create_identity_matrix());

    // Draw the points
    drawSpherePoints();

    // Render the scene
    renderer.end_scene();

    // Draw the labels.
    painter->setPen(QColor(255,255,255));
    painter->drawText(int(x_label_loc.x), int(x_label_loc.y), "X");
    int h = fontMetrics().height();
    painter->drawText(int(y_label_loc.x) + h,
        int(y_label_loc.y) + h, "Y");
}

// ****************************************************************************
// Method: QvisScatterWidget::redrawScene3D
//
// Purpose: 
//   Redraws the scene in 3D.
//
// Arguments:
//   painter : The painter to use to redraw the scene.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:38:27 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::redrawScene3D(QPainter *painter)
{
    //
    // Set up the camera.
    //
    vector3 camera = vec_create(10,4,21);
    matrix4 view = m3du_create_view_matrix(
            camera,
            vec_create(4,4,5),
            vec_create(0,1,0));
    renderer.set_view_matrix(view);
    renderer.set_view_reference_point(camera);

    // Set up the lights.
    renderer.set_light(1, M3D_LIGHT_AMB, 0., 0., 0., 0.1f, 0.1f, 0.1f);
    renderer.set_light(2, M3D_LIGHT_EYE, -35.f, -35.f, -50.f, 0.7f, 0.7f, 0.7f);
    renderer.begin_scene(painter);
    matrix4 identity = m3du_create_identity_matrix();
    renderer.set_world_matrix(identity);

    // Draw the X axis.
    matrix4 r = m3du_create_RBT_matrix(vec_create(0.,0.,0.),
        vec_create(1.,0.,0.), vec_create(0.,1.,0.));
    renderer.set_world_matrix(r);
    color c_red = {1.,0.,0.};
    arrow.addToRenderer(renderer, c_red);
    vector3 x_label_loc = renderer.transform_world_point(
        vec_create(ARROW_LENGTH, 0., 0.));
    renderer.set_world_matrix(identity);

    // Draw the Y axis.
    r = m3du_create_RBT_matrix(vec_create(0.,0.,0.),
        vec_create(0.,1.,0.), vec_create(1.,0.,0.));
    renderer.set_world_matrix(r);
    color c_green = {0., 1., 0.};
    arrow.addToRenderer(renderer, c_green);
    vector3 y_label_loc = renderer.transform_world_point(
        vec_create(0., ARROW_LENGTH, 0.));
    renderer.set_world_matrix(identity);

    // Draw the Z axis.
    matrix4 m = identity;
    float lengthScale = 1.f;
    if(highlightedAxis)
    {
        float scale = float(animationProgress) / float(ANIMATION_NSTATES-1);
        lengthScale = 0.75 + 0.25 * scale;
        m = m3du_create_scaling_matrix(1.2, 1.2, lengthScale);
    }
    renderer.set_world_matrix(m);
    color c_blue = {0., 0., 1.};
    arrow.addToRenderer(renderer, c_blue);
    vector3 z_label_loc = renderer.transform_world_point(
        vec_create(0, 0, lengthScale * ARROW_LENGTH));

    renderer.set_world_matrix(identity);

    // Draw the points
    drawSpherePoints();

    // Render the scene
    renderer.end_scene();

    // Draw the labels.
    painter->setPen(QColor(255,255,255));
    painter->drawText(int(x_label_loc.x), int(x_label_loc.y), "X");
    painter->drawText(int(y_label_loc.x), int(y_label_loc.y), "Y");
    painter->drawText(int(z_label_loc.x), int(z_label_loc.y), "Z");
}

// ****************************************************************************
// Method: QvisScatterWidget::drawSpherePoints
//
// Purpose: 
//   Draws the scatter points.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:38:53 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::drawSpherePoints()
{
    // Draw a small cloud of points.
    const float centers[9][3] = {
        {3,2,2},
        {2,5,6},
        {4,4,9},
        {4,7,6},
        {6,4,4},
        {6,6,1},
        {9,5,7},
        {8,7,3},
        {1.5, 8.,7.}
    };
    const float colors[9][3] = {
        {0,0,255},
        {0,128,128},
        {0,255,255},
        {0,255,128},
        {0,255,0},
        {128,255,0},
        {255,255,0},
        {255,128,0},
        {255,0,0}
    };
    int nPoints = 9;

    for(int i = 0; i < nPoints; ++i)
    {
        const float *pt = centers[i];
        float z = threeD ? pt[2] : 0.;
        matrix4 translate = m3du_create_translation_matrix(pt[0],pt[1],z);
        renderer.set_world_matrix(translate);

        float c[3];
        if(coloredPoints)
        {
            const float *c0 = colors[i];
            c[0] = c0[0] / 255.;
            c[1] = c0[1] / 255.;
            c[2] = c0[2] / 255.;
        }
        else
        {
            c[0] = c[1] = c[2] = 0.6;
        }

        // Add the sphere to the renderer in the specified color.
        sphere.addToRenderer(renderer, color_create(c));
    }
}

// ****************************************************************************
// Method: QvisScatterWidget::paintEvent
//
// Purpose: 
//   This method is called when the widget needs to be repainted.
//
// Arguments:
//   e : The paint event to process.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:39:10 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::paintEvent(QPaintEvent *e)
{
    bool clipByRegion = true;

    // Draw the scene into the backing pixmap.
    bool needsPaint = pixmapDirty;
    if(pixmap == 0)
    {
        needsPaint = true;
        pixmap = new QPixmap(width(), height());
    }
    if(needsPaint)
    {
        QPainter pixpaint(pixmap);
        redrawScene(&pixpaint);
        setBackgroundPixmap(*pixmap);
        clipByRegion = false;
        pixmapDirty = false;
    }

    // Blit the pixmap to the screen.
    QPainter paint(this);
    if(clipByRegion && !e->region().isEmpty() && !e->region().isNull())
        paint.setClipRegion(e->region());
    paint.drawPixmap(QPoint(0,0), *pixmap);
}

// ****************************************************************************
// Method: QvisScatterWidget::resizeEvent
//
// Purpose: 
//   This method is called when the widget needs to be resized.
//
// Arguments:
//   e : The resize event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:39:38 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::resizeEvent(QResizeEvent *e)
{
    deleteBackingPixmap();
    renderer.resize(e->size().width(), e->size().height());
}

//
// Qt slot functions
//

// ****************************************************************************
// Method: QvisScatterWidget::show
//
// Purpose: 
//   This is a Qt slot function that is called when the widget is shown.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:43:36 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::show()
{
    QWidget::show();
    if(highlightedAxis)
        timer->start(ANIMATION_TIMEOUT);
}

// ****************************************************************************
// Method: QvisScatterWidget::hide
//
// Purpose: 
//   This is a Qt slot function that is called when the widget is hidden.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:43:36 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::hide()
{
    QWidget::hide();
    if(highlightedAxis)
        timer->stop();
}

// ****************************************************************************
// Method: QvisScatterWidget::handleTimer
//
// Purpose: 
//   This is a Qt slot function that is called in response to the widget's
//   timer and is used to perform animation in the widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 13 14:43:36 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisScatterWidget::handleTimer()
{
    if(isVisible())
    {
        deleteBackingPixmap();
        if(animationCountPositive)
        {
            ++animationProgress;
            if(animationProgress >= ANIMATION_NSTATES)
                animationCountPositive = false;
        }
        else
        {
            --animationProgress;
            if(animationProgress <= 0)
                animationCountPositive = true;
        }
        update();
    }
}

//
// Methods to create sphere, arrow objects.
//

void
QvisScatterWidget::initializeSphere(m3d_complex_element &obj, int nx, int ny,
    float rad, float r, float g, float b)
{
    int i,j;
    vector3 sphere_pts[SPHERE_XDIM+1][SPHERE_YDIM+1];
    vector3 sphere_nrm[SPHERE_XDIM  ][SPHERE_YDIM  ];

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

void
QvisScatterWidget::initializeArrow()
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
