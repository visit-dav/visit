#ifndef VTKQT_GL_WIDGET_H
#define VTKQT_GL_WIDGET_H
#include <vtkqt_exports.h>
#include <qgl.h>

class vtkQtRenderWindowInteractor;

// ****************************************************************************
// Class: vtkQtGLWidget
//
// Purpose:
//   This class extends the Qt OpenGL widget to include custom methods that
//   allow it to work with VTK while also allowing the widget to be contained
//   within a QMainWindow so we can have builtin toolbar support.
//
// Notes:      Mostly I had to subclass QGLWidget so that the proper
//             initializeGL method would be called.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 27 14:40:19 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Apr 7 14:43:12 PST 2003
//   I removed the minimumSizeHint method.
//
// ****************************************************************************

class VTKQT_API vtkQtGLWidget : public QGLWidget
{
    Q_OBJECT
public:
    vtkQtGLWidget(QWidget *parent, const char *name);
    virtual ~vtkQtGLWidget()        { }
    void setLineSmoothing(bool val) { lineSmoothing = val; }
    bool getLineSmoothing() const   { return lineSmoothing; }

    void SetInteractor(vtkQtRenderWindowInteractor *i) { interactor = i; }

    virtual void initializeGL();
    virtual QSizePolicy sizePolicy() const;
    virtual QSize sizeHint() const;
protected:
    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    virtual void focusInEvent(QFocusEvent*) { }
    virtual void focusOutEvent(QFocusEvent*) { }
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void keyPressEvent(QKeyEvent*);
private:
    bool                         lineSmoothing;
    vtkQtRenderWindowInteractor *interactor;
};

#endif
