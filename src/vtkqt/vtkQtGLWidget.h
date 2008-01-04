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
//   Gunther Weber, Fri Aug  3 17:56:01 PDT 2007
//   Added wheelEvent method.
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
    virtual void wheelEvent(QWheelEvent*);
    virtual void keyPressEvent(QKeyEvent*);
private:
    bool                         lineSmoothing;
    vtkQtRenderWindowInteractor *interactor;
};

#endif
