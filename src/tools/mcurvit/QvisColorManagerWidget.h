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

#ifndef QVIS_COLOR_MANAGER_WIDGET_H
#define QVIS_COLOR_MANAGER_WIDGET_H
#include <gui_exports.h>
#include <vector>
#include <QColor>
#include <QLayout>
#include <QScrollArea>
#include <QString>

// Forward declarations.
class QGrid;
class QLabel;
class QvisColorButton;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisColorManagerWidget
//
// Purpose:
//   This class contains a list of names items and can select colors and
//   opacities for them.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:28:39 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Feb 16 17:46:57 PST 2001
//   Made the column header labels class members and added methods to set
//   the text that is displayed in them.
//
//   Brad Whitlock, Thu Sep 6 15:25:19 PST 2001
//   Added a method that is called when the palette changes.
//
//   Brad Whitlock, Thu Aug 22 12:12:25 PDT 2002
//   I added an override for the setEnabled method.
//
//   Brad Whitlock, Tue Jul 15 15:01:03 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisColorManagerWidget : public QScrollArea
{
    Q_OBJECT

    typedef struct
    {
        QLabel            *nameLabel;
        QvisColorButton   *colorBox;
        QvisOpacitySlider *opacitySlider;
    } ColorEntry;

    typedef std::vector<ColorEntry *> ColorEntryVector;
public:
    QvisColorManagerWidget(QWidget *parent = 0);
    virtual ~QvisColorManagerWidget();
    virtual QSize sizeHint() const;
    virtual void setEnabled(bool val);

    void addEntry(const QString &name, const QColor &color, int opacity);
    void removeLastEntry();
    void setEntry(int index, const QString &name, const QColor &color,
                  int opacity);

    void setAttributeName(int index, const QString &name);
    void setColor(int index, const QColor &color);
    void setOpacity(int index, int opacity);
    void setNameLabelText(const QString &s);
    void setColorLabelText(const QString &s);
    void setOpacityLabelText(const QString &s);

    int numEntries() const;
    QString name(int index) const;
    QColor  color(int index) const;
    int opacity(int index) const;
signals:
    void colorChanged(const QColor &color, int index);
    void opacityChanged(int opacity, int index);
private slots:
    void selectedColor(const QColor &color, const void *userData);
    void changedOpacity(int opacity, const void *userData);
private:
    QWidget          *top;
    QGridLayout      *grid;
    QLabel           *nameLabel;
    QLabel           *colorLabel;
    QLabel           *opacityLabel;

    ColorEntryVector colorEntries;
};

#endif
