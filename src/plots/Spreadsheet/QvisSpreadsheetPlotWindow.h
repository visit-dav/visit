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

#ifndef QVISSPREADSHEETPLOTWINDOW_H
#define QVISSPREADSHEETPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class SpreadsheetAttributes;
class QLabel;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QButtonGroup;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisDialogLineEdit;

// ****************************************************************************
// Class: QvisSpreadsheetPlotWindow
//
// Purpose: 
//   Defines QvisSpreadsheetPlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 12:02:25 PDT 2007
//
// Modifications:
//   Gunther H. Weber, Thu Sep 27 12:05:14 PDT 2007
//   Added font selection for spreadsheet
//   
//   Gunther H. Weber, Wed Oct 17 14:48:16 PDT 2007
//   Support toggling patch outline and tracer plane separately
//
//   Gunther H. Weber, Wed Nov 28 15:37:17 PST 2007
//   Support toggeling the current cell outline
//
// ****************************************************************************

class QvisSpreadsheetPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisSpreadsheetPlotWindow(const int type,
                         SpreadsheetAttributes *subj,
                         const QString &caption = QString::null,
                         const QString &shortName = QString::null,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisSpreadsheetPlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
    void UpdateSubsetNames();
private slots:
    void formatStringProcessText();
    void useColorTableChanged(bool val);
    void colorTableNameChanged(bool useDefault, const QString &ctName);
    void showPatchOutlineChanged(bool val);
    void showCurrentCellOutlineChanged(bool val);
    void showTracerPlaneChanged(bool val);
    void tracerColorChanged(const QColor &color);
    void tracerOpacityChanged(int);
    void normalChanged(int val);
    void subsetNameChanged(const QString &);
    void fontNameChanged(const QString &newFont);
private:
    int plotType;
    QComboBox *subsetName;
    QLineEdit *formatString;
    QvisDialogLineEdit *fontName;
    QCheckBox *useColorTable;
    QvisColorTableButton *colorTableName;
    QCheckBox *showPatchOutline;
    QCheckBox *showCurrentCellOutline;
    QCheckBox *showTracerPlane;
    QvisColorButton   *tracerColor;
    QvisOpacitySlider *tracerOpacity;

    QButtonGroup *normal;

    QLabel *subsetNameLabel;
    QLabel *formatStringLabel;
    QLabel *tracerColorLabel;
    QLabel *normalLabel;

    SpreadsheetAttributes *atts;
    int silTopSet;
    int silNumSets;
    int silNumCollections;

    static const char *defaultItem;
};

#endif
