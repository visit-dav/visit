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
#include <QvisAnnotationWidget.h>

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

#include <QNarrowLineEdit.h>

#include <AnnotationAttributes.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <ViewAxisArrayAttributes.h>
#include <VisItViewer.h>

// ****************************************************************************
// Method: QvisAnnotationWidget::QvisAnnotationWidget
//
// Purpose: 
//   Constructor.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 13:31:33 PST 2009
//
// Modifications:
//
// ****************************************************************************

QvisAnnotationWidget::QvisAnnotationWidget(QWidget *parent, VisItViewer *v) :
    QWidget(parent)
{
    viewer = v;

    CreateWindow();
}

// ****************************************************************************
// Method: QvisAnnotationWidget::~QvisAnnotationWidget
//
// Purpose: 
//   Destructor.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 13:31:33 PST 2009
//
// Modifications:
//   
// ****************************************************************************

QvisAnnotationWidget::~QvisAnnotationWidget()
{
}

// ****************************************************************************
// Method: QvisAnnotationWidget::Update
//
// Purpose: 
//   Update the widgets associated with the specified subject.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 13:31:33 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWidget::Update(Subject *subject)
{
    if (subject == annotationAtts)
        UpdateAnnotationWidgets();
    else if (subject == axisArrayAtts)
        UpdateViewAxisArrayWidgets();
    else if (subject == annotationObjectList)
        UpdateAnnotationObjectListWidgets();
}

// ****************************************************************************
// Method: QvisAnnotationWidget::SubjectRemoved
//
// Purpose: 
//   Tell the window that the subject being observed is no longer valid.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 13:31:33 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationWidget::SubjectRemoved(Subject *subject)
{
    if (subject == annotationAtts)
       annotationAtts = 0;
    else if (subject == axisArrayAtts)
       axisArrayAtts = 0;
}

// ****************************************************************************
// Method: QvisAnnotationWidget::ConnectAnnotationAttributes
//
// Purpose:
//   Makes the window observe the annotation attributes.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 13:31:33 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisAnnotationWidget::ConnectAnnotationAttributes(AnnotationAttributes *a)
{
    annotationAtts = a;
    a->Attach(this);
}

// ****************************************************************************
// Method: QvisAnnotationWidget::ConnectAxisArrayAttributes
//
// Purpose:
//   Makes the window observe the view axis array attributes.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 13:31:33 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisAnnotationWidget::ConnectViewAxisArrayAttributes(
    ViewAxisArrayAttributes *v)
{
    axisArrayAtts = v;
    v->Attach(this);
}

// ****************************************************************************
// Method: QvisAnnotationWidget::ConnectAnnotationObjectList
//
// Purpose:
//   Makes the window observe the annotation object list.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 13:31:33 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisAnnotationWidget::ConnectAnnotationObjectList(
    AnnotationObjectList *a)
{
    annotationObjectList = a;
    a->Attach(this);
}

// ****************************************************************************
// Method: QvisAnnotationWidget::CreateWindow
//
// Purpose:
//   Create the contents of the window.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 13:31:33 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisAnnotationWidget::CreateWindow()
{
    QVBoxLayout *pageAnnotLayout = new QVBoxLayout(this);
    pageAnnotLayout->setMargin(5);
    pageAnnotLayout->setSpacing(10);

    // Create the title group
    titleGroup = new QGroupBox(this);
    titleGroup->setTitle(tr("Title"));
    pageAnnotLayout->addWidget(titleGroup);
    titleGroup->setCheckable(true);
    connect(titleGroup, SIGNAL(toggled(bool)),
            this, SLOT(titleToggled(bool)));
    QGridLayout *titleLayout = new QGridLayout(titleGroup);
    titleLayout->setSpacing(5);

    // Add controls to set the font family.
    titleFontFamilyComboBox = new QComboBox(titleGroup);
    titleFontFamilyComboBox->addItem("Arial");
    titleFontFamilyComboBox->addItem("Courier");
    titleFontFamilyComboBox->addItem("Times");
    titleFontFamilyComboBox->setEditable(false);
    connect(titleFontFamilyComboBox, SIGNAL(activated(int)),
            this, SLOT(titleFontFamilyChanged(int)));
    QLabel *tffLabel = new QLabel(tr("Font name"), titleGroup);
    tffLabel->setBuddy(titleFontFamilyComboBox);
    titleLayout->addWidget(tffLabel, 0, 0);
    titleLayout->addWidget(titleFontFamilyComboBox, 0, 1);

    // Add control for text font height
    titleFontScale = new QNarrowLineEdit(titleGroup);
    connect(titleFontScale, SIGNAL(returnPressed()),
            this, SLOT(processTitleFontScaleText()));
    titleLayout->addWidget(titleFontScale, 0, 3);
    QLabel *tfsLabel = new QLabel(tr("Font scale"), titleGroup);
    tfsLabel->setBuddy(titleFontScale);
    titleLayout->addWidget(tfsLabel, 0, 2);

    // Add control for making the text bold
    titleBoldCheckBox = new QCheckBox(tr("Bold"), titleGroup);
    connect(titleBoldCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(titleBoldToggled(bool)));
    titleLayout->addWidget(titleBoldCheckBox, 1, 0);

    // Add control for making the text italic
    titleItalicCheckBox = new QCheckBox(tr("Italic"), titleGroup);
    connect(titleItalicCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(titleItalicToggled(bool)));
    titleLayout->addWidget(titleItalicCheckBox, 1, 1);

    // Create the label group.
    labelGroup = new QGroupBox(this);
    labelGroup->setTitle(tr("Labels"));
    pageAnnotLayout->addWidget(labelGroup);
    labelGroup->setCheckable(true);
    connect(labelGroup, SIGNAL(toggled(bool)),
            this, SLOT(labelToggled(bool)));
    QGridLayout *labelLayout = new QGridLayout(labelGroup);
    labelLayout->setSpacing(5);

    // Add controls to set the font family.
    labelFontFamilyComboBox = new QComboBox(labelGroup);
    labelFontFamilyComboBox->addItem("Arial");
    labelFontFamilyComboBox->addItem("Courier");
    labelFontFamilyComboBox->addItem("Times");
    labelFontFamilyComboBox->setEditable(false);
    connect(labelFontFamilyComboBox, SIGNAL(activated(int)),
            this, SLOT(labelFontFamilyChanged(int)));
    QLabel *labelFFLabel = new QLabel(tr("Font name"), labelGroup);
    labelFFLabel->setBuddy(labelFontFamilyComboBox);
    labelLayout->addWidget(labelFFLabel, 0, 0);
    labelLayout->addWidget(labelFontFamilyComboBox, 0, 1);

    // Add control for text font height
    labelFontScale = new QNarrowLineEdit(labelGroup);
    connect(labelFontScale, SIGNAL(returnPressed()),
            this, SLOT(processLabelFontScaleText()));
    labelLayout->addWidget(labelFontScale, 0, 3);
    QLabel *labelFSLabel = new QLabel(tr("Font scale"), labelGroup);
    labelFSLabel->setBuddy(labelFontScale);
    labelLayout->addWidget(labelFSLabel, 0, 2);

    // Add control for making the text bold
    labelBoldCheckBox = new QCheckBox(tr("Bold"), labelGroup);
    connect(labelBoldCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(labelBoldToggled(bool)));
    labelLayout->addWidget(labelBoldCheckBox, 1, 0);

    // Add control for making the text italic
    labelItalicCheckBox = new QCheckBox(tr("Italic"), labelGroup);
    connect(labelItalicCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(labelItalicToggled(bool)));
    labelLayout->addWidget(labelItalicCheckBox, 1, 1);

    // Create the legend group.
    legendGroup = new QGroupBox(this);
    legendGroup->setTitle(tr("Legend"));
    pageAnnotLayout->addWidget(legendGroup);
    QGridLayout *legendLayout = new QGridLayout(legendGroup);
    legendLayout->setSpacing(5);

    // Add controls to set the font family.
    legendFontFamilyComboBox = new QComboBox(legendGroup);
    legendFontFamilyComboBox->addItem("Arial");
    legendFontFamilyComboBox->addItem("Courier");
    legendFontFamilyComboBox->addItem("Times");
    legendFontFamilyComboBox->setEditable(false);
    connect(legendFontFamilyComboBox, SIGNAL(activated(int)),
            this, SLOT(legendFontFamilyChanged(int)));
    QLabel *legendFFLegend = new QLabel(tr("Font name"), legendGroup);
    legendFFLegend->setBuddy(legendFontFamilyComboBox);
    legendLayout->addWidget(legendFFLegend, 0, 0);
    legendLayout->addWidget(legendFontFamilyComboBox, 0, 1);

    // Add control for text font height
    legendFontHeight = new QNarrowLineEdit(legendGroup);
    connect(legendFontHeight, SIGNAL(returnPressed()),
            this, SLOT(processLegendFontHeightText()));
    legendLayout->addWidget(legendFontHeight, 0, 3);
    QLabel *legendFHLegend = new QLabel(tr("Font height"), legendGroup);
    legendFHLegend->setBuddy(legendFontHeight);
    legendLayout->addWidget(legendFHLegend, 0, 2);

    // Add control for making the text bold
    legendBoldCheckBox = new QCheckBox(tr("Bold"), legendGroup);
    connect(legendBoldCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(legendBoldToggled(bool)));
    legendLayout->addWidget(legendBoldCheckBox, 1, 0);

    // Add control for making the text italic
    legendItalicCheckBox = new QCheckBox(tr("Italic"), legendGroup);
    connect(legendItalicCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(legendItalicToggled(bool)));
    legendLayout->addWidget(legendItalicCheckBox, 1, 1);

    // Create the tick group
    tickGroup = new QGroupBox(this);
    tickGroup->setTitle(tr("Tick marks"));
    pageAnnotLayout->addWidget(tickGroup);
    tickGroup->setCheckable(true);
    connect(tickGroup, SIGNAL(toggled(bool)),
            this, SLOT(tickToggled(bool)));
    QGridLayout *tgLayout = new QGridLayout(tickGroup);
    tgLayout->setSpacing(5);

    majorMinimum = new QNarrowLineEdit(tickGroup);
    tgLayout->addWidget(majorMinimum, 0, 1);
    connect(majorMinimum, SIGNAL(returnPressed()),
            this, SLOT(processMajorMinimumText()));
    majorMinimumLabel = new QLabel(tr("Major minimum"), tickGroup);
    majorMinimumLabel->setBuddy(majorMinimum);
    tgLayout->addWidget(majorMinimumLabel, 0, 0);

    majorMaximum = new QNarrowLineEdit(tickGroup);
    tgLayout->addWidget(majorMaximum, 1, 1);
    connect(majorMaximum, SIGNAL(returnPressed()),
            this, SLOT(processMajorMaximumText()));
    majorMaximumLabel = new QLabel(tr("Major maximum"), tickGroup);
    majorMaximumLabel->setBuddy(majorMaximum);
    tgLayout->addWidget(majorMaximumLabel, 1, 0);

    minorSpacing = new QNarrowLineEdit(tickGroup);
    tgLayout->addWidget(minorSpacing, 2, 1);
    connect(minorSpacing, SIGNAL(returnPressed()),
            this, SLOT(processMinorSpacingText()));
    minorSpacingLabel = new QLabel(tr("Minor spacing"), tickGroup);
    minorSpacingLabel->setBuddy(minorSpacing);
    tgLayout->addWidget(minorSpacingLabel, 2, 0);

    majorSpacing = new QNarrowLineEdit(tickGroup);
    tgLayout->addWidget(majorSpacing, 3, 1);
    connect(majorSpacing, SIGNAL(returnPressed()),
            this, SLOT(processMajorSpacingText()));
    majorSpacingLabel = new QLabel(tr("Major spacing"), tickGroup);
    majorSpacingLabel->setBuddy(majorSpacing);
    tgLayout->addWidget(majorSpacingLabel, 3, 0);

    // Create the view group box.
    viewGroup = new QGroupBox(this);
    viewGroup->setTitle(tr("View"));
    pageAnnotLayout->addWidget(viewGroup);

    QGridLayout *viewLayout = new QGridLayout(viewGroup);
    viewLayout->setSpacing(5);
    viewLayout->setColumnStretch(1, 10);

    // Create the viewport widgets.
    viewportLineEdit = new QLineEdit(viewGroup);
    connect(viewportLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processViewportText()));
    viewLayout->addWidget(viewportLineEdit, 0, 1);
    QLabel *viewportLabel = new QLabel(tr("Viewport"), viewGroup);
    viewportLabel->setBuddy(viewportLineEdit);
    viewLayout->addWidget(viewportLabel, 0, 0);

    // Create the domain widgets.
    domainLineEdit = new QLineEdit(viewGroup);
    connect(domainLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processDomainText()));
    viewLayout->addWidget(domainLineEdit, 1, 1);
    QLabel *domainLabel = new QLabel(tr("Domain"), viewGroup);
    domainLabel->setBuddy(domainLineEdit);
    viewLayout->addWidget(domainLabel, 1, 0);

    // Create the range widgets.
    rangeLineEdit = new QLineEdit(viewGroup);
    connect(rangeLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processRangeText()));
    viewLayout->addWidget(rangeLineEdit, 2, 1);
    QLabel *rangeLabel = new QLabel(tr("Range"), viewGroup);
    rangeLabel->setBuddy(rangeLineEdit);
    viewLayout->addWidget(rangeLabel, 2, 0);
}

// ****************************************************************************
// Method: QvisAnnotationWidget::UpdateAnnotationWidgets
//
// Purpose:
//   Update the widgets associated with the annotation attributes.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 13:31:33 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisAnnotationWidget::UpdateAnnotationWidgets()
{
    titleGroup->blockSignals(true);
    titleGroup->setChecked(annotationAtts->GetAxesArray().GetAxes().GetTitle().GetVisible());
    titleGroup->blockSignals(false);

    titleFontFamilyComboBox->blockSignals(true);
    titleFontFamilyComboBox->setCurrentIndex(annotationAtts->GetAxesArray().GetAxes().GetTitle().GetFont().GetFont());
    titleFontFamilyComboBox->blockSignals(false);

    titleFontScale->setText(QString().setNum(annotationAtts->GetAxesArray().GetAxes().GetTitle().GetFont().GetScale()));

    titleBoldCheckBox->blockSignals(true);
    titleBoldCheckBox->setChecked(annotationAtts->GetAxesArray().GetAxes().GetTitle().GetFont().GetBold());
    titleBoldCheckBox->blockSignals(false);

    titleItalicCheckBox->blockSignals(true);
    titleItalicCheckBox->setChecked(annotationAtts->GetAxesArray().GetAxes().GetTitle().GetFont().GetItalic());
    titleItalicCheckBox->blockSignals(false);

    labelGroup->blockSignals(true);
    labelGroup->setChecked(annotationAtts->GetAxesArray().GetAxes().GetLabel().GetVisible());
    labelGroup->blockSignals(false);

    labelFontFamilyComboBox->blockSignals(true);
    labelFontFamilyComboBox->setCurrentIndex(annotationAtts->GetAxesArray().GetAxes().GetLabel().GetFont().GetFont());
    labelFontFamilyComboBox->blockSignals(false);

    labelFontScale->setText(QString().setNum(annotationAtts->GetAxesArray().GetAxes().GetLabel().GetFont().GetScale()));

    labelBoldCheckBox->blockSignals(true);
    labelBoldCheckBox->setChecked(annotationAtts->GetAxesArray().GetAxes().GetLabel().GetFont().GetBold());
    labelBoldCheckBox->blockSignals(false);

    labelItalicCheckBox->blockSignals(true);
    labelItalicCheckBox->setChecked(annotationAtts->GetAxesArray().GetAxes().GetLabel().GetFont().GetItalic());
    labelItalicCheckBox->blockSignals(false);

    tickGroup->blockSignals(true);
    tickGroup->setChecked(!annotationAtts->GetAxesArray().GetAutoSetTicks());
    tickGroup->blockSignals(false);

    QString val;
    val.setNum(annotationAtts->GetAxesArray().GetAxes().GetTickMarks().GetMajorMinimum());
    majorMinimum->setText(val);
    val.setNum(annotationAtts->GetAxesArray().GetAxes().GetTickMarks().GetMajorMaximum());
    majorMaximum->setText(val);
    val.setNum(annotationAtts->GetAxesArray().GetAxes().GetTickMarks().GetMinorSpacing());
    minorSpacing->setText(val);
    val.setNum(annotationAtts->GetAxesArray().GetAxes().GetTickMarks().GetMajorSpacing());
    majorSpacing->setText(val);
}

// ****************************************************************************
// Method: QvisAnnotationWidget::UpdateViewAxisArrayWidgets
//
// Purpose:
//   Update the widgets associated with the view axis array attributes.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 13:31:33 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisAnnotationWidget::UpdateViewAxisArrayWidgets()
{
    const double *viewport = axisArrayAtts->GetViewportCoords();
    viewportLineEdit->setText(DoublesToQString(viewport, 4));

    const double *domain = axisArrayAtts->GetDomainCoords();
    domainLineEdit->setText(DoublesToQString(domain, 2));

    const double *range = axisArrayAtts->GetRangeCoords();
    rangeLineEdit->setText(DoublesToQString(range, 2));
}

// ****************************************************************************
// Method: QvisAnnotationWidget::UpdateAnnotationObjectListWidgets
//
// Purpose:
//   Update the widgets associated with the annotation object list.
//
// Programmer: Eric Brugger
// Creation:   Mon Feb  2 13:31:33 PST 2009
//
// Modifications:
//
// ****************************************************************************

void
QvisAnnotationWidget::UpdateAnnotationObjectListWidgets()
{
    for (int i = 0; i < annotationObjectList->GetNumAnnotations(); i++)
    {
        if (annotationObjectList->GetAnnotation(i).GetObjectName() == "Plot0000")
        {
            legendFontFamilyComboBox->blockSignals(true);
            legendFontFamilyComboBox->setCurrentIndex(annotationObjectList->GetAnnotation(i).GetFontFamily());
            legendFontFamilyComboBox->blockSignals(false);

            legendFontHeight->setText(QString().setNum(annotationObjectList->GetAnnotation(i).GetDoubleAttribute1()));

            legendBoldCheckBox->blockSignals(true);
            legendBoldCheckBox->setChecked(annotationObjectList->GetAnnotation(i).GetFontBold());
            legendBoldCheckBox->blockSignals(false);

            legendItalicCheckBox->blockSignals(true);
            legendItalicCheckBox->setChecked(annotationObjectList->GetAnnotation(i).GetFontItalic());
            legendItalicCheckBox->blockSignals(false);
        }
    }
}

bool
QvisAnnotationWidget::QStringToDoubles(const QString &str, double *vals, int maxVals)
{
    bool retval = false;

    if(!str.isEmpty())
    {
        bool okay = true;
        QStringList s = str.split(" ", QString::SkipEmptyParts);
        for(int i = 0; i < maxVals && okay; ++i)
        {
            if(i < s.size())
                vals[i] = s[i].toDouble(&okay);
            else
                vals[i] = 0.;
        }
        retval = okay;
    }

    return retval;
}

QString
QvisAnnotationWidget::DoublesToQString(const double *vals, int nvals)
{
    QString s;
    for(int i = 0; i < nvals; ++i)
    {
        s += QString().setNum(vals[i]);
        if(i < nvals-1)
            s += " ";
    }
    return s;
}

bool
QvisAnnotationWidget::LineEditGetDouble(QLineEdit *lineEdit, double &val)
{
    bool okay = false;
    val = lineEdit->displayText().trimmed().toDouble(&okay);
    return okay;
}

//
// Qt slot functions
//
void
QvisAnnotationWidget::titleToggled(bool val)
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        atts->GetAxesArray().GetAxes().GetTitle().SetVisible(val);
        atts->Notify();
        emit annotationChanged(atts);
    }
}

void
QvisAnnotationWidget::titleFontFamilyChanged(int value)
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        atts->GetAxesArray().GetAxes().GetTitle().GetFont().SetFont((FontAttributes::FontName)value);
        atts->Notify();
        emit annotationChanged(atts);
    }
}

void
QvisAnnotationWidget::titleBoldToggled(bool val)
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        atts->GetAxesArray().GetAxes().GetTitle().GetFont().SetBold(val);
        atts->Notify();
        emit annotationChanged(atts);
    }
}

void
QvisAnnotationWidget::titleItalicToggled(bool val)
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        atts->GetAxesArray().GetAxes().GetTitle().GetFont().SetItalic(val)
;
        atts->Notify();
        emit annotationChanged(atts);
    }
}

void
QvisAnnotationWidget::labelToggled(bool val)
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        atts->GetAxesArray().GetAxes().GetLabel().SetVisible(val);
        atts->Notify();
        emit annotationChanged(atts);
    }
}

void
QvisAnnotationWidget::labelFontFamilyChanged(int value)
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        atts->GetAxesArray().GetAxes().GetLabel().GetFont().SetFont((FontAttributes::FontName)value);
        atts->Notify();
        emit annotationChanged(atts);
    }
}

void
QvisAnnotationWidget::labelBoldToggled(bool val)
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        atts->GetAxesArray().GetAxes().GetLabel().GetFont().SetBold(val);
        atts->Notify();
        emit annotationChanged(atts);
    }
}

void
QvisAnnotationWidget::labelItalicToggled(bool val)
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        atts->GetAxesArray().GetAxes().GetLabel().GetFont().SetItalic(val);
        atts->Notify();
        emit annotationChanged(atts);
    }
}

void
QvisAnnotationWidget::legendFontFamilyChanged(int value)
{
    AnnotationObjectList *atts = viewer->DelayedState()->GetAnnotationObjectList();
    for (int i = 0; i < atts->GetNumAnnotations(); i++)
    {
        if (atts->GetAnnotation(i).GetObjectName() == "Plot0000")
        {
            atts->GetAnnotation(i).SetFontFamily((AnnotationObject::FontFamily)value);
            atts->Notify();
            emit annotationObjectListChanged(atts);
        }
    }
}

void
QvisAnnotationWidget::legendBoldToggled(bool val)
{
    AnnotationObjectList *atts = viewer->DelayedState()->GetAnnotationObjectList();
    for (int i = 0; i < atts->GetNumAnnotations(); i++)
    {
        if (atts->GetAnnotation(i).GetObjectName() == "Plot0000")
        {
            atts->GetAnnotation(i).SetFontBold(val);
            atts->Notify();
            emit annotationObjectListChanged(atts);
        }
    }
}

void
QvisAnnotationWidget::legendItalicToggled(bool val)
{
    AnnotationObjectList *atts = viewer->DelayedState()->GetAnnotationObjectList();
    for (int i = 0; i < atts->GetNumAnnotations(); i++)
    {
        if (atts->GetAnnotation(i).GetObjectName() == "Plot0000")
        {
            atts->GetAnnotation(i).SetFontItalic(val);
            atts->Notify();
            emit annotationObjectListChanged(atts);
        }
    }
}

void
QvisAnnotationWidget::tickToggled(bool val)
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        atts->GetAxesArray().SetAutoSetTicks(!val);
        atts->Notify();
        emit annotationChanged(atts);
    }
}

void
QvisAnnotationWidget::processTitleFontScaleText()
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        double val;
        bool okay = LineEditGetDouble(titleFontScale, val);
        if (okay)
        {
            atts->GetAxesArray().GetAxes().GetTitle().GetFont().SetScale(val);
            atts->Notify();
            emit annotationChanged(atts);
        }
    }
}

void
QvisAnnotationWidget::processLabelFontScaleText()
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        double val;
        bool okay = LineEditGetDouble(labelFontScale, val);
        if (okay)
        {
            atts->GetAxesArray().GetAxes().GetLabel().GetFont().SetScale(val);
            atts->Notify();
            emit annotationChanged(atts);
        }
    }
}

void
QvisAnnotationWidget::processLegendFontHeightText()
{
    AnnotationObjectList *atts = viewer->DelayedState()->GetAnnotationObjectList();
    for (int i = 0; i < atts->GetNumAnnotations(); i++)
    {
        if (atts->GetAnnotation(i).GetObjectName() == "Plot0000")
        {
            double val;
            bool okay = LineEditGetDouble(legendFontHeight, val);
            if (okay)
            {
                atts->GetAnnotation(i).SetDoubleAttribute1(val);
                atts->Notify();
                emit annotationObjectListChanged(atts);
            }
        }
    }
}

void
QvisAnnotationWidget::processMajorMinimumText()
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        double val;
        bool okay = LineEditGetDouble(majorMinimum, val);
        if (okay)
        {
            atts->GetAxesArray().GetAxes().GetTickMarks().SetMajorMinimum(val);
            atts->Notify();
            emit annotationChanged(atts);
        }
    }
}

void
QvisAnnotationWidget::processMajorMaximumText()
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        double val;
        bool okay = LineEditGetDouble(majorMaximum, val);
        if (okay)
        {
            atts->GetAxesArray().GetAxes().GetTickMarks().SetMajorMaximum(val);
            atts->Notify();
            emit annotationChanged(atts);
        }
    }
}

void
QvisAnnotationWidget::processMinorSpacingText()
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        double val;
        bool okay = LineEditGetDouble(minorSpacing, val);
        if (okay)
        {
            atts->GetAxesArray().GetAxes().GetTickMarks().SetMinorSpacing(val);
            atts->Notify();
            emit annotationChanged(atts);
        }
    }
}

void
QvisAnnotationWidget::processMajorSpacingText()
{
    AnnotationAttributes *atts = viewer->DelayedState()->GetAnnotationAttributes();
    if(atts != 0)
    {
        double val;
        bool okay = LineEditGetDouble(majorSpacing, val);
        if (okay)
        {
            atts->GetAxesArray().GetAxes().GetTickMarks().SetMajorSpacing(val);
            atts->Notify();
            emit annotationChanged(atts);
        }
    }
}

void
QvisAnnotationWidget::processViewportText()
{
    ViewAxisArrayAttributes *atts = viewer->DelayedState()->GetViewAxisArrayAttributes();
    if(atts != 0)
    {
        double viewport[4];
        bool okay = QStringToDoubles(viewportLineEdit->displayText().trimmed(),
                                     viewport, 4);
        if (okay)
        {
            atts->SetViewportCoords(viewport);
            atts->Notify();
            emit viewAxisArrayChanged(atts);
        }
    }
}

void
QvisAnnotationWidget::processDomainText()
{
    ViewAxisArrayAttributes *atts = viewer->DelayedState()->GetViewAxisArrayAttributes();
    if(atts != 0)
    {
        double domain[2];
        bool okay = QStringToDoubles(domainLineEdit->displayText().trimmed(),
                                     domain, 2);
        if (okay)
        {
            atts->SetDomainCoords(domain);
            atts->Notify();
            emit viewAxisArrayChanged(atts);
        }
    }
}

void
QvisAnnotationWidget::processRangeText()
{
    ViewAxisArrayAttributes *atts = viewer->DelayedState()->GetViewAxisArrayAttributes();
    if(atts != 0)
    {
        double range[2];
        bool okay = QStringToDoubles(rangeLineEdit->displayText().trimmed(),
                                     range, 2);
        if (okay)
        {
            atts->SetRangeCoords(range);
            atts->Notify();
            emit viewAxisArrayChanged(atts);
        }
    }
}
