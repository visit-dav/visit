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
// Method: vtkQtGLWidget::wheelEvent
//
// Purpose: 
//   Passes wheel events to the interactor.
//
// Arguments:
//   we : A pointer to a wheel event.
//
// Programmer: Gunther H. Weber
// Creation:   Fri Aug  3 17:56:01 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtGLWidget::wheelEvent(QWheelEvent *we)
{
    if (interactor)
        interactor->wheelEvent(we);
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

