/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef QVIS_COLOR_BUTTON_H
#define QVIS_COLOR_BUTTON_H
#include <gui_exports.h>

#include <QAbstractButton>
#include <QColor>
#include <vector>

class QPainter;
class QMenu;
class QvisColorSelectionWidget;

// ****************************************************************************
// Class: QvisColorButton
//
// Purpose:
//   This class represents a color button widget that can be used to select
//   colors for materials, isocontours, and other items in the gui.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 4 09:49:14 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 15:28:12 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisColorButton : public QAbstractButton
{
    Q_OBJECT

    typedef std::vector<QvisColorButton *> ColorButtonVector;
public:
    QvisColorButton(QWidget *parent = 0, const void *userData = 0);
    virtual ~QvisColorButton();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy () const;

    void setButtonColor(const QColor &c);
    const QColor &buttonColor() const;

    void setUserData(const void *data);
    const void *getUserData() const;
signals:
    void selectedColor(const QColor &c);
    void selectedColor(const QColor &c, const void *data);
protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void drawButton(QPainter *);
    virtual void drawButtonLabel(QPainter *);
private slots:
    void popupPressed();
    void colorSelected(const QColor &c);
private:
    QColor                           color;
    const void                      *userData;

    static QvisColorSelectionWidget *popup;
    static ColorButtonVector         buttons;
};

#endif
