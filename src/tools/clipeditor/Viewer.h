// ----------------------------------------------------------------------------
// File:  Viewer.h
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
// ----------------------------------------------------------------------------

#ifndef VIEWER_H
#define VIEWER_H

#include <qgl.h>

#include "Vector.h"
#include "Matrix.h"
#include "DataSet.h"

class Viewer : public QGLWidget
{
    Q_OBJECT
  public:
                  Viewer(QWidget *parent=NULL, const char *name=NULL);
                 ~Viewer();

    void          initializeGL();
    void          paintGL();
    void          resizeGL(int w, int h);
    void          setDataSet(DataSet *ds);
  protected:

    virtual void  mouseMoveEvent(QMouseEvent*);
    virtual void  mousePressEvent(QMouseEvent*);
    virtual void  mouseReleaseEvent(QMouseEvent*);

    void          setupMatrices();

    int dl;

    bool       mousedown;
    int        lastx, lasty;

    // Matrices
    Matrix     P;  // Projection matrix
    Matrix     V;  // View matrix
    Matrix     M;  // Model->world matrix
    Matrix     R;  // Accumulated trackball rotations
    Matrix     A;  // Accumulated angle-change rotations

    // Dataset
    DataSet   *data;
    float      ds_size;

    Vector from;
    Vector at;
    Vector up;
    float  fov;
    float  aspect;
    float  nearplane;
    float  farplane;
};

#endif
