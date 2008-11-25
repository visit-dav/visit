/*=========================================================================
  vtkQtRenderWindowInteractor.cpp - copyright 2000 Matthias Koenig 
  koenig@isg.cs.uni-magdeburg.de
  http://wwwisg.cs.uni-magdeburg.de/~makoenig
  =========================================================================*/
/* Portions of this code marked with "LLNL" have been modified from the
 * original sources by Matthias Koenig.  Contact VisIt@llnl.gov.  */
/*=========================================================================
  This module is an extension of the "Visualization Toolkit 
  ( copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen )".
  and combines it with "Qt (copyright (C) 1992-2000 Troll Tech AS)".
  =========================================================================*/
/*=========================================================================

  Module:    $RCSfile: vtkQtRenderWindowInteractor.cpp,v $
  Date:    $Date: 2000/04/15 18:58:01 $
  Version:   $Revision: 1.5 $

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
#include <QtCore>
#include <QCursor>
#include <QMouseEvent>

#include "vtkQtRenderWindowInteractor.h"
#include <ctype.h>
#include <visitstream.h>

#include <vtkCommand.h>

// Modifications:
//  Brad Whitlock, Mon Mar 13 15:38:12 PST 2006
//  I added support for alt and alt+shift keys.
//
vtkQtRenderWindowInteractor::vtkQtRenderWindowInteractor() {
    qtRenWin = NULL;
    alt = false;
    altshift = false;
}

vtkQtRenderWindowInteractor::~vtkQtRenderWindowInteractor() {
}

void vtkQtRenderWindowInteractor::Start() {
    if (! this->qtRenWin) {
      vtkErrorMacro(<<"No vtkQtRenderWindow defined!");
      return;
    }
    if (! qApp) {
      vtkErrorMacro(<<"No QApplication defined!");
      return;
    }
    qApp->exec();
}

// GetSize calling sequence changed by LLNL to fit VTK interface change.
void vtkQtRenderWindowInteractor::SetRenderWindow(vtkQtRenderWindow* aren) {
    qtRenWin = aren;
    aren->SetInteractor(this);
    vtkRenderWindowInteractor::SetRenderWindow(aren);
    int *size = qtRenWin->GetSize();
    Size[0] = size[0];
    Size[1] = size[1];
}

void vtkQtRenderWindowInteractor::PrintSelf(ostream&os, vtkIndent indent) {
    vtkRenderWindowInteractor::PrintSelf(os, indent);
}

// Modifications:
//  GetSize calling sequence changed by LLNL to fit VTK interface change.
//
//  Brad Whitlock, Mon Mar 13 15:38:12 PST 2006
//  I added support for alt and alt+shift keys.
//
//  Brad Whitlock, Fri May  9 09:59:32 PDT 2008
//  Qt 4.
void vtkQtRenderWindowInteractor::mousePressEvent(QMouseEvent *me) {
    if (!Enabled)
      return;

    int *size = qtRenWin->GetSize();
    Size[0] = size[0];
    Size[1] = size[1];

    int ctrl = 0, shift = 0;
    if (me->modifiers() & Qt::ControlModifier)
      ctrl = 1;
    if (me->modifiers() & Qt::ShiftModifier)
      shift = 1;
    // Set the alt and altshift flags.
    alt = (me->modifiers() & Qt::AltModifier);
    if(me->modifiers() & Qt::AltModifier &&
       me->modifiers() & Qt::ShiftModifier)
    {
        altshift = true;
        shift = 0; 
        alt = false;
    }
    else
        altshift = false;

    int xp = me->x();
    int yp = Size[1]- me->y() -1;

    SetEventInformation(xp, yp, ctrl, shift);
    switch (me->button()) {
    case Qt::LeftButton:
      if(altshift)
          InvokeEvent(vtkCommand::MiddleButtonPressEvent, NULL);
      else if(alt)
          InvokeEvent(vtkCommand::RightButtonPressEvent, NULL);
      else
          InvokeEvent(vtkCommand::LeftButtonPressEvent, NULL);
      break;
    case Qt::MidButton:
      InvokeEvent(vtkCommand::MiddleButtonPressEvent, NULL); 
      break;
    case Qt::RightButton:
      InvokeEvent(vtkCommand::RightButtonPressEvent, NULL); 
      break;
    default:
      return;
    }
}

// Modifications:
//  GetSize calling sequence changed by LLNL to fit VTK interface change.
//
//  Brad Whitlock, Mon Mar 13 15:38:12 PST 2006
//  I added support for alt and alt+shift keys.
//
void vtkQtRenderWindowInteractor::mouseReleaseEvent(QMouseEvent *me) {
    if (!Enabled)
      return;

    int *size = qtRenWin->GetSize();
    Size[0] = size[0];
    Size[1] = size[1];

    int ctrl = 0, shift = 0;
    if (me->modifiers() & Qt::ControlModifier)
      ctrl = 1;
    if (me->modifiers() & Qt::ShiftModifier)
      shift = 1;
    // Set the alt and altshift flags.
    if(altshift)
        shift = 0; 

    int xp = me->x();
    int yp = Size[1]- me->y() -1;

    SetEventInformation(xp, yp, ctrl, shift);
    switch (me->button()) {
    case Qt::LeftButton:
      if(altshift)
          InvokeEvent(vtkCommand::MiddleButtonReleaseEvent);
      else if(alt)
          InvokeEvent(vtkCommand::RightButtonReleaseEvent);
      else
          InvokeEvent(vtkCommand::LeftButtonReleaseEvent);
      break;
    case Qt::MidButton:
      InvokeEvent(vtkCommand::MiddleButtonReleaseEvent);
      break;
    case Qt::RightButton:
      InvokeEvent(vtkCommand::RightButtonReleaseEvent);
      break;
    default:
      return;
    }

    alt = false;
    altshift = false;
}

// Modifications:
//   Gunther H. Weber, Fri Aug  3 17:56:01 PDT 2007
//   Added wheelEvent method
void vtkQtRenderWindowInteractor::wheelEvent(QWheelEvent*we)
{
    if (we->delta() > 0) 
	InvokeEvent(vtkCommand::MouseWheelForwardEvent, NULL);
    else if (we->delta() < 0)
	InvokeEvent(vtkCommand::MouseWheelBackwardEvent, NULL);
}

void vtkQtRenderWindowInteractor::timer() {
    if (!Enabled)
      return;
    QPoint cp = qtRenWin->mapFromGlobal(QCursor::pos());
    
    SetEventInformation(cp.x(), Size[1] - cp.y(), 0, 0);
    InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
    InvokeEvent(vtkCommand::TimerEvent, NULL);
}

int vtkQtRenderWindowInteractor::CreateTimer(int timertype) {
    if (timertype == VTKI_TIMER_FIRST) {
        QObject::connect(&qTimer, SIGNAL(timeout()), SLOT(timer()));
        qTimer.start(10);
    }
    return 1;
}

int vtkQtRenderWindowInteractor::DestroyTimer() {
    qTimer.stop();
    QObject::disconnect(&qTimer, SIGNAL(timeout()), this, 0);
    return 1;
}

// Modifications:
//  GetSize calling sequence changed by LLNL to fit VTK interface change.
//
//  Brad Whitlock, Mon Mar 13 15:38:12 PST 2006
//  I added support for alt and alt+shift keys.
//
void vtkQtRenderWindowInteractor::keyPressEvent(QKeyEvent *ke) {
    if (!Enabled)
      return;
    
    int *size = qtRenWin->GetSize();
    Size[0] = size[0];
    Size[1] = size[1];
    int ctrl = 0, shift = 0;
    if (ke->modifiers() & Qt::ControlModifier)
      ctrl = 1;
    if (ke->modifiers() & Qt::ShiftModifier)
      shift = 1;
    // Set the alt and altshift flags.
    alt = (ke->modifiers() & Qt::AltModifier);
    if(ke->modifiers() & Qt::AltModifier &&
       ke->modifiers() & Qt::ShiftModifier)
    {
        altshift = true;
        shift = 0;
        alt = false;
    }
    else
        altshift = false;

    QPoint cp = qtRenWin->mapFromGlobal(QCursor::pos());
    int xp = cp.x();
    int yp = Size[1]- cp.y() -1;
    
    SetEventInformation(xp, yp, 0, 0);
    InvokeEvent(vtkCommand::MouseMoveEvent, NULL);
   
    SetEventInformation(xp, yp, ctrl, shift, tolower(ke->key()), 1);
    InvokeEvent(vtkCommand::KeyPressEvent, NULL);
    InvokeEvent(vtkCommand::CharEvent, NULL);
}

