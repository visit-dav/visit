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
#ifndef QVIS_ANNOTATION_WIDGET_H
#define QVIS_ANNOTATION_WIDGET_H
#include <QWidget>
#include <SimpleObserver.h>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;

class QNarrowLineEdit;

class VisItViewer;

class AnnotationAttributes;
class AnnotationObjectList;
class ViewAxisArrayAttributes;

// ****************************************************************************
// Class: QvisAnnotaionWidget
//
// Purpose:
//   Widget that makes it easier to show/update AxisAttributes objects.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 09:20:22 PST 2009
//
// Modifications:
//
// ****************************************************************************

class QvisAnnotationWidget : public QWidget, public SimpleObserver
{
    Q_OBJECT
public:
    QvisAnnotationWidget(QWidget *parent, VisItViewer *v);
    virtual ~QvisAnnotationWidget();

    virtual void Update(Subject *subject);
    virtual void SubjectRemoved(Subject *subject);

    void ConnectAnnotationAttributes(AnnotationAttributes *a);
    void ConnectViewAxisArrayAttributes(ViewAxisArrayAttributes *v);
    void ConnectAnnotationObjectList(AnnotationObjectList *a);

    void CreateWindow();
    void UpdateAnnotationWidgets();
    void UpdateViewAxisArrayWidgets();
    void UpdateAnnotationObjectListWidgets();

    bool QStringToDoubles(const QString &, double *, int);
    QString DoublesToQString(const double *, int);
    bool LineEditGetDouble(QLineEdit *lineEdit, double &val);

signals:
    void annotationChanged(const AnnotationAttributes *);
    void viewAxisArrayChanged(const ViewAxisArrayAttributes *);
    void annotationObjectListChanged(const AnnotationObjectList *);
private slots:
    void titleToggled(bool val);
    void titleFontFamilyChanged(int value);
    void titleBoldToggled(bool val);
    void titleItalicToggled(bool val);
    void labelToggled(bool val);
    void labelFontFamilyChanged(int value);
    void labelBoldToggled(bool val);
    void labelItalicToggled(bool val);
    void legendFontFamilyChanged(int value);
    void legendBoldToggled(bool val);
    void legendItalicToggled(bool val);
    void tickToggled(bool val);
    void processTitleFontScaleText();
    void processLabelFontScaleText();
    void processLegendFontHeightText();
    void processMajorMinimumText();
    void processMajorMaximumText();
    void processMinorSpacingText();
    void processMajorSpacingText();
    void processViewportText();
    void processDomainText();
    void processRangeText();
private:
    VisItViewer             *viewer;

    AnnotationAttributes    *annotationAtts;
    ViewAxisArrayAttributes *axisArrayAtts;
    AnnotationObjectList    *annotationObjectList;

    QGroupBox               *titleGroup;
    QComboBox               *titleFontFamilyComboBox;
    QNarrowLineEdit         *titleFontScale;
    QCheckBox               *titleBoldCheckBox;
    QCheckBox               *titleItalicCheckBox;
    QGroupBox               *labelGroup;
    QComboBox               *labelFontFamilyComboBox;
    QNarrowLineEdit         *labelFontScale;
    QCheckBox               *labelBoldCheckBox;
    QCheckBox               *labelItalicCheckBox;
    QGroupBox               *legendGroup;
    QComboBox               *legendFontFamilyComboBox;
    QNarrowLineEdit         *legendFontHeight;
    QCheckBox               *legendBoldCheckBox;
    QCheckBox               *legendItalicCheckBox;
    QGroupBox               *tickGroup;
    QLabel                  *majorMinimumLabel;
    QNarrowLineEdit         *majorMinimum;
    QLabel                  *majorMaximumLabel;
    QNarrowLineEdit         *majorMaximum;
    QLabel                  *minorSpacingLabel;
    QNarrowLineEdit         *minorSpacing;
    QLabel                  *majorSpacingLabel;
    QNarrowLineEdit         *majorSpacing;
    QGroupBox               *viewGroup;
    QLineEdit               *viewportLineEdit;
    QLabel                  *viewportLabel;
    QLineEdit               *domainLineEdit;
    QLabel                  *domainLabel;
    QLineEdit               *rangeLineEdit;
    QLabel                  *rangeLabel;
};

#endif
