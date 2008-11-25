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

#ifndef QVIS_COLOR_SELECTION_WIDGET_H
#define QVIS_COLOR_SELECTION_WIDGET_H
#include <gui_exports.h>
#include <QWidget>

// Forward declarations.
class QPushButton;
class QTimer;
class QVBoxLayout;
class QvisColorGridWidget;

// ****************************************************************************
// Class: QvisColorSelectionWidget
//
// Purpose:
//   This widget contains an array of colors from which the user may choose.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 14:30:50 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Oct 26 14:07:58 PST 2001
//   Added a timer to hide the widget after a little while.
//
//   Brad Whitlock, Tue Jun  3 15:30:09 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisColorSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    QvisColorSelectionWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~QvisColorSelectionWidget();
    virtual QSize sizeHint() const;

    void setSelectedColor(const QColor &color);
signals:
    void selectedColor(const QColor &color);
public slots:
    virtual void show();
    virtual void hide();
protected:
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
private slots:
    void getCustomColor();
    void handleSelectedColor(const QColor &color);
private:
    int                 numCustomColors;
    QvisColorGridWidget *standardColorGrid;
    QvisColorGridWidget *customColorGrid;
    QPushButton         *moreColorsButton;
    QTimer              *timer;

    static const unsigned char colorComponents[];
    static const int MAX_CUSTOM_COLORS;
};

#endif
