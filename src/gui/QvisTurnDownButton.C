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

#include <QvisTurnDownButton.h>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionHeader>

// ****************************************************************************
// Method: QvisTurnDownButton::QvisTurnDownButton
//
// Purpose: 
//   Constructor for the QvisTurnDownButton class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:57:16 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun  3 16:13:20 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisTurnDownButton::QvisTurnDownButton(QWidget *parent) : 
    QPushButton(parent)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum));
}

// ****************************************************************************
// Method: QvisTurnDownButton::~QvisTurnDownButton
//
// Purpose: 
//   Destructor for the QvisTurnDownButton class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:57:59 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QvisTurnDownButton::~QvisTurnDownButton()
{
}

// ****************************************************************************
// Method: QvisTurnDownButton::drawButtonLabel
//
// Purpose: 
//   Draws the arrow instead of a button label.
//
// Arguments:
//   painter : The painter to use to draw the arrow.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 13:58:19 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Mar 6 14:52:07 PST 2006
//   Added Qt 3 implementation.
//
//   Brad Whitlock, Tue Jun  3 16:24:44 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisTurnDownButton::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);

    QPainter paint(this);
    QPolygon tri(3);
    tri.setPoint(0, width()/2, height()*2/3);
    tri.setPoint(1, width()-4, height()*1/3);
    tri.setPoint(2, 4, height()*1/3);
    paint.setBrush(palette().text());
    paint.drawConvexPolygon(tri);
}
