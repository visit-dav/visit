#include <vtkQtGLWidget.h>
#include <vtkQtRenderWindow.h>
#include <vtkQtRenderWindowInteractor.h>

// ****************************************************************************
// Method: vtkQtGLWidget::vtkQtGLWidget
//
// Purpose: 
//   Constructor for the vtkQtGLWidget class.
//
// Arguments:
//   parent : The parent vtkQtRenderWindow widget.
//   name   : The name of this widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 27 17:32:28 PST 2003
//
// Modifications:
//   
// ****************************************************************************

vtkQtGLWidget::vtkQtGLWidget(QWidget *parent, const char *name) :
    QGLWidget(parent, name)
{
    lineSmoothing = false;
    interactor = 0;
}

// ****************************************************************************
// Method: vtkQtGLWidget::initializeGL
//
// Purpose: 
//   Initializes the OpenGL context.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 27 17:32:28 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtGLWidget::initializeGL()
{   
    glMatrixMode( GL_MODELVIEW );
    
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_DEPTH_TEST );
    
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    
    // initialize blending for transparency
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable(GL_BLEND);
   
    if (lineSmoothing)
    {
        glEnable(GL_LINE_SMOOTH);
    }
    else 
    {
        glDisable(GL_LINE_SMOOTH);
    }
    
    glEnable( GL_NORMALIZE );
    glAlphaFunc(GL_GREATER,0);
}

// ****************************************************************************
// Method: vtkQtGLWidget::paintGL
//
// Purpose: 
//   This method is called when the OpenGL stuff in this widget needs to be
//   redrawn. Since the parent vtkQtRenderWindow contains all of that stuff,
//   we tell it to render its actors.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 27 17:32:28 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtGLWidget::paintGL()
{
    // Tell the parent to render.
    vtkQtRenderWindow *parent = (vtkQtRenderWindow *)parentWidget();
    if(parent)
        parent->TriggerRender();
}

// ****************************************************************************
// Method: vtkQtGLWidget::resizeGL
//
// Purpose: 
//   This method is called when the widget changes sizes. We need to tell the
//   parent the sice of the widget so it knows to update the viewport.
//
// Arguments:
//   w : The new width
//   h : The new height
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 10:16:27 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtGLWidget::resizeGL(int w, int h)
{
    // Tell the parent to render.
    vtkQtRenderWindow *parent = (vtkQtRenderWindow *)parentWidget();
    if(parent)
        parent->TellGLSize(w, h);
}

// ****************************************************************************
// Method: vtkQtGLWidget::mousePressEvent
//
// Purpose: 
//   Passes mouse press events to the interactor.
//
// Arguments:
//   me : A pointer to a mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 27 17:32:28 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtGLWidget::mousePressEvent(QMouseEvent *me)
{
    if (interactor)
        interactor->mousePressEvent(me);
}

// ****************************************************************************
// Method: vtkQtGLWidget::mouseReleaseEvent
//
// Purpose: 
//   Passes mouse release events to the interactor.
//
// Arguments:
//   me : A pointer to a mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 27 17:32:28 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtGLWidget::mouseReleaseEvent(QMouseEvent *me)
{
    if (interactor)
        interactor->mouseReleaseEvent(me);
}

// ****************************************************************************
// Method: vtkQtGLWidget::keyPressEvent
//
// Purpose: 
//   Passes key press events to the interactor.
//
// Arguments:
//   ke : A pointer to a key press event object.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 27 17:32:28 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtGLWidget::keyPressEvent(QKeyEvent *ke)
{
    if (interactor)
        interactor->keyPressEvent(ke);
}

QSizePolicy vtkQtGLWidget::sizePolicy() const { return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding); }
QSize vtkQtGLWidget::sizeHint() const { return QSize(200, 200); }

