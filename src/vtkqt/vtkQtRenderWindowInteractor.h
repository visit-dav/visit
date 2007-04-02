/*=========================================================================
  vtkQtRenderWindowInteractor.h - copyright 2000 Matthias Koenig 
  koenig@isg.cs.uni-magdeburg.de
  http://wwwisg.cs.uni-magdeburg.de/~makoenig
  =========================================================================*/
/*=========================================================================
  This module is an extension of the "Visualization Toolkit 
  ( copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen )".
  and combines it with "Qt (copyright (C) 1992-2000 Troll Tech AS)".
  =========================================================================*/
/*=========================================================================

  Module:    $RCSfile: vtkQtRenderWindowInteractor.h,v $
  Date:      $Date: 2000/04/15 18:57:45 $
  Version:   $Revision: 1.3 $

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  
  * Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
   
  * Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
   
  * Modified source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
  =========================================================================*/

#ifndef _vtkQtRenderWindowInteractor_h
#define _vtkQtRenderWindowInteractor_h
#include <vtkqt_exports.h>

#include "vtkInteractorStyle.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkQtRenderWindow.h"
#include "vtkQtGLWidget.h"
#include <qobject.h>
#include <qtimer.h>

class VTKQT_API vtkQtRenderWindowInteractor : public QObject, public vtkRenderWindowInteractor 
{
    Q_OBJECT
    friend class vtkQtGLWidget;
public:
    vtkQtRenderWindowInteractor();
    ~vtkQtRenderWindowInteractor();
    static vtkQtRenderWindowInteractor *New(); // inline
    virtual const char *GetClassName() const; // inline
    void PrintSelf(ostream& os, vtkIndent indent);
    
    void Start();
    
    void SetRenderWindow(vtkQtRenderWindow*);
    
    // timer methods
    int CreateTimer(int timertype);
    int DestroyTimer(void);
    
    // own exit method 
    void ExitCallback(); // inline 
    
protected slots:
    void timer();
protected:
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
private:
    vtkQtRenderWindow *qtRenWin;
    QTimer qTimer;
    bool alt;
    bool altshift;
};

inline const char *vtkQtRenderWindowInteractor::GetClassName() const { return "vtkQtRenderWindowInteractor"; }
inline vtkQtRenderWindowInteractor *vtkQtRenderWindowInteractor::New() { return new vtkQtRenderWindowInteractor; }
inline void vtkQtRenderWindowInteractor::ExitCallback() { qApp->exit(); }

#endif

