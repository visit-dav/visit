// ----------------------------------------------------------------------------
// File:  Viewer.C
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
// ----------------------------------------------------------------------------

#include "Viewer.h"
#include "Matrix.h"
#include "Shape.h"

#include <qevent.h>

Viewer::Viewer(QWidget *parent, const char *name)
    : QGLWidget(parent, name)
{
    mousedown = false;
    M.CreateIdentity();

    data = NULL;

    dl = -1;
}

Viewer::~Viewer()
{
}

void
Viewer::setDataSet(DataSet *ds)
{
    data = ds;
    ds_size = sqrt(data->xsize*data->xsize +
                   data->ysize*data->ysize +
                   data->zsize*data->zsize);

    at   = Vector((data->xmax+data->xmin)/2.,
                  (data->ymax+data->ymin)/2.,
                  (data->zmax+data->zmin)/2.);
    from = at + Vector(0,0, -ds_size*2);
    up   = Vector(0,1,0);
}

void
Viewer::initializeGL()
{
    glClearColor(0.0,0.0,0.0,1.0); 

}


//#define DLIST

void
Viewer::paintGL()
{
    if (!data)
        return;

    setupMatrices();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    //glEnable(GL_LIGHTING);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
    glEnable(GL_DEPTH_TEST);

    float ambient[4]   = {0.01,0.01,0.01,1.0};
    float diffuse[4]   = {0.9,0.9,0.9,1.0};

    Vector dir1 = from.normalized() * -10;

    float direction1[4] = {dir1.x, dir1.y, dir1.z, 1};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, direction1);

    glEnable(GL_LIGHT0);



    if (dl == -1)
    {
#ifdef DLIST
        dl = glGenLists(1);
        glNewList(dl, GL_COMPILE);
#endif
        Vector right = ((from-at) % up).normalized();
        data->DrawPolyData(up, right);
        /*
        glColor3f(1.,.1,.1);
        glBegin(GL_TRIANGLES);
        for (int i=0; i<data->ntris; i++)
        {
            for (int j=0; j<3; j++)
            {
                glNormal3fv(&data->tnormals[i*3+0]);
                glVertex3fv(&data->pts[data->tris[3*i + j]*3 + 0]);
            }
        }
        glEnd();
        */
#ifdef DLIST
        glEndList();
#endif
    }

#ifdef DLIST
    glCallList(dl);
#endif
}

void
Viewer::setupMatrices()
{
    if (!data)
        return;

    aspect = float(width())/float(height());
    fov = 0.5;

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    P.CreatePerspectiveProjection(ds_size/16., ds_size*4, fov, aspect);
    glMultMatrixf(P.GetOpenGLMatrix());

    V.CreateView(from,at,up);
    glMultMatrixf(V.GetOpenGLMatrix());

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}

void
Viewer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void
Viewer::mousePressEvent(QMouseEvent *mev)
{
    lastx = mev->x();
    lasty = mev->y();
    mousedown = true;
    updateGL();
}

void
Viewer::mouseMoveEvent(QMouseEvent *mev)
{
    if (!data)
        return;

    int x = mev->x();
    int y = mev->y();

    if (mousedown)
    {
        float x1 =  ((float(lastx*2)/float(width())) - 1.0);
        float y1 = -((float(lasty*2)/float(height())) - 1.0);
        float x2 =  ((float(  x  *2)/float(width())) - 1.0);
        float y2 = -((float(  y  *2)/float(height())) - 1.0);

        if (mev->state() == LeftButton)
        {
            Matrix R1;
            R1.CreateTrackball(-x1,-y1, -x2,-y2);
            R = R1*R;

            Matrix T1;
            T1.CreateTranslate(-(data->xmax+data->xmin)/2.,
                               -(data->ymax+data->ymin)/2.,
                               -(data->zmax+data->zmin)/2.);
            Matrix T2(T1);
            T2.Inverse();
            Matrix V1(V);
            V1.m[0][3]=0;
            V1.m[1][3]=0;
            V1.m[2][3]=0;
            Matrix V2(V1);
            V2.Inverse();
            Matrix RI(R);
            RI.Inverse();
            Matrix MM = T2 * V2 * R1 * V1 * T1;

            from = MM*from;
            at   = MM*at;
            up   = MM^up;
            V.CreateView(from,at,up);
        }
        else if (mev->state() == MidButton)
        {
            Vector view_dir((at-from).normalized());
            at   += view_dir*(y2-y1)*ds_size;
            from += view_dir*(y2-y1)*ds_size;
            V.CreateView(from,at,up);
        }
        else if (mev->state() & RightButton)
        {
            Vector view_dir(at-from);
            Vector right(up % view_dir);
            right.normalize();
            at   += right*(x1-x2)*ds_size + up*(y1-y2)*ds_size;
            from += right*(x1-x2)*ds_size + up*(y1-y2)*ds_size;
            V.CreateView(from,at,up);
        }
    }
    lastx = x;
    lasty = y;

    updateGL();
}

void
Viewer::mouseReleaseEvent(QMouseEvent *mev)
{
    mousedown = false;
    updateGL();
}


