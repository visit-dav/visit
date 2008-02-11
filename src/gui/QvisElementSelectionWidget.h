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

#ifndef QVIS_ELEMENT_SELECTION_WIDGET_H
#define QVIS_ELEMENT_SELECTION_WIDGET_H
#include <gui_exports.h>
#include <qwidget.h>

// Forward declarations.
class QPushButton;
class QTimer;
class QVBoxLayout;
class QvisPeriodicTableWidget;

// ****************************************************************************
// Class: QvisElementSelectionWidget
//
// Purpose:
//   This contains the periodic table widget plus a new button that
//   allows a wildcard ("match any element").
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Note: borrowed heavily from Brad's color selection widget
//
// Modifications:
//
// ****************************************************************************

class GUI_API QvisElementSelectionWidget : public QWidget
{
    Q_OBJECT
  public:
    QvisElementSelectionWidget(QWidget *parent = 0, const char *name = 0,
                               WFlags f = 0);
    virtual ~QvisElementSelectionWidget();
    virtual QSize sizeHint() const;

    void setSelectedElement(int);
    signals:
    void selectedElement(int);
            public slots:
            virtual void show();
    virtual void hide();
  protected:
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
             private slots:
             void matchAnyElementClicked();
    void handleSelectedElement(int);
  private:
    QvisPeriodicTableWidget *periodicTable;
    QPushButton             *matchAnyElementButton;
    QTimer                  *timer;
};

#endif
