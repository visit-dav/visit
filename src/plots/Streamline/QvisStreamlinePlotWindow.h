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

#ifndef QVIS_STREAMLINE_PLOT_WINDOW_H
#define QVIS_STREAMLINE_PLOT_WINDOW_H
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations
class QLabel;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class QvisColorTableButton;
class QvisColorButton;
class QvisLineWidthWidget;
class StreamlineAttributes;

// ****************************************************************************
// Class: QvisStreamlinePlotWindow
//
// Purpose: 
//   Defines QvisStreamlinePlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 21 14:12:49 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 22 13:03:49 PST 2004
//   Added support for coloring by vorticity and ribbons.
//
//   Brad Whitlock, Mon Jan 3 15:32:24 PST 2005
//   Added override of ProcessOldVersions.
//
//   Hank Childs, Sat Mar  3 09:11:44 PST 2007
//   Added useWholeBox.
//
//   Dave Pugmire, Thu Nov 15 12:09:08 EST 2007
//   Add streamline direction option.
//
// ****************************************************************************

class QvisStreamlinePlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisStreamlinePlotWindow(const int type,
                         StreamlineAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisStreamlinePlotWindow();
    virtual void CreateWindowContents();

    virtual void ProcessOldVersions(DataNode *node, const char *ver);
  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
    void UpdateSourceAttributes();
  private slots:
    void sourceTypeChanged(int val);
    void directionTypeChanged(int val);  
    void stepLengthProcessText();
    void maxTimeProcessText();
    void pointSourceProcessText();
    void lineStartProcessText();
    void lineEndProcessText();
    void planeOriginProcessText();
    void planeNormalProcessText();
    void planeUpAxisProcessText();
    void planeRadiusProcessText();
    void sphereOriginProcessText();
    void sphereRadiusProcessText();
    void pointDensityChanged(int val);
    void displayMethodChanged(int val);
    void showStartChanged(bool val);
    void radiusProcessText();
    void lineWidthChanged(int style);
    void coloringMethodChanged(int val);
    void colorTableNameChanged(bool useDefault, const QString &ctName);
    void singleColorChanged(const QColor &color);
    void legendFlagChanged(bool val);
    void lightingFlagChanged(bool val);
    void useWholeBoxChanged(bool val);
    void boxExtentsProcessText();
  private:
    int plotType;
    QComboBox *sourceType;
    QGroupBox *sourceAtts;
    QComboBox *directionType;
    QLineEdit *stepLength;
    QLineEdit *maxTime;
    QLineEdit *pointSource;
    QLabel    *pointSourceLabel;
    QLineEdit *lineStart;
    QLabel    *lineStartLabel;
    QLineEdit *lineEnd;
    QLabel    *lineEndLabel;
    QLineEdit *planeOrigin;
    QLabel    *planeOriginLabel;
    QLineEdit *planeNormal;
    QLabel    *planeNormalLabel;
    QLineEdit *planeUpAxis;
    QLabel    *planeUpAxisLabel;
    QLineEdit *planeRadius;
    QLabel    *planeRadiusLabel;
    QLineEdit *sphereOrigin;
    QLabel    *sphereOriginLabel;
    QLineEdit *sphereRadius;
    QLabel    *sphereRadiusLabel;
    QCheckBox *useWholeBox;
    QLineEdit *boxExtents[3];
    QLabel    *boxExtentsLabel[3];
    QSpinBox  *pointDensity;
    QComboBox *displayMethod;
    QCheckBox *showStart;
    QLineEdit *radius;
    QLabel    *radiusLabel;
    QvisLineWidthWidget *lineWidth;
    QLabel    *lineWidthLabel;
    QComboBox *coloringMethod;
    QvisColorTableButton *colorTableName;
    QLabel    *colorTableNameLabel;
    QvisColorButton *singleColor;
    QLabel    *singleColorLabel;
    QCheckBox *legendFlag;
    QCheckBox *lightingFlag;

    StreamlineAttributes *streamAtts;
};



#endif
