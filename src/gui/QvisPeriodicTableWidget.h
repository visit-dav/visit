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

#ifndef QVIS_PERIODIC_TABLE_WIDGET_H
#define QVIS_PERIODIC_TABLE_WIDGET_H
#include <gui_exports.h>
#include <QWidget>
#include <QvisGridWidget.h>
#include <vector>

class QPixmap;
class QPainter;

// ****************************************************************************
// Class: QvisPeriodicTableWidget
//
// Purpose:
//   This widget contains an array of colors from which the user may choose.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   July  6, 2006
//
// Modifications:
//    Jeremy Meredith, Tue Feb 12 14:01:52 EST 2008
//    Added support for hinting some elements to the user, e.g. to highlight
//    the elements that are actually in the database.
//
//    Brad Whitlock, Tue Jun  3 14:21:26 PDT 2008
//    QT 4.
//
// ****************************************************************************

class GUI_API QvisPeriodicTableWidget : public QvisGridWidget
{
    Q_OBJECT
public:
    QvisPeriodicTableWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~QvisPeriodicTableWidget();

    void setSelectedElement(int e);
    void setHintedElements(const std::vector<int>&);

signals:
    void   selectedElement(int element);
protected:
    virtual void keyPressEvent(QKeyEvent *e);

    virtual void drawItem(QPainter &paint, int index);

private:
    int  indexToElement(int) const;
    virtual bool isValidIndex(int) const;
    virtual void emitSelection();

    bool *hintedElements;
};

#endif
