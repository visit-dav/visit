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
#include <QvisAxisAttributesWidget.h>

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qtimer.h>

#include <QNarrowLineEdit.h>
#include <QvisFontAttributesWidget.h>

// Leave this macro defined until we have a chance to go back into the 
// VisWindow and the axes to support different fonts.
#define FONTS_ARE_DISABLED

// ****************************************************************************
// Method: QvisAxisAttributesWidget::QvisAxisAttributesWidget
//
// Purpose: 
//   Constructor.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:50:42 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisAxisAttributesWidget::QvisAxisAttributesWidget(QWidget *parent, 
    const char *name, bool tickEnabled, bool titleEnabled) :
    QVBox(parent, name), GUIBase(), atts()
{
    autoScaling = true;
    autoTickMarks = true;
    setMargin(5);
    setSpacing(5);

    //
    // Create the title group
    //
    titleGroup = new QGroupBox(this, "titleGroup");
    titleGroup->setTitle("Title");
    if(titleEnabled)
    {
        titleGroup->setCheckable(true);
        connect(titleGroup, SIGNAL(toggled(bool)),
                this, SLOT(titleToggled(bool)));
    }
    QVBoxLayout *tInnerLayout = new QVBoxLayout(titleGroup);
    tInnerLayout->setMargin(10);
    tInnerLayout->addSpacing(15);
    tInnerLayout->setSpacing(10);
    int row = 0;
    QGridLayout *tLayout = new QGridLayout(tInnerLayout, 4, 2);
    tLayout->setSpacing(5);
    customTitleToggle = new QCheckBox("Custom title", titleGroup,
        "customTitleToggle");
    connect(customTitleToggle, SIGNAL(toggled(bool)),
            this, SLOT(customTitleToggled(bool)));
    tLayout->addWidget(customTitleToggle, row, 0);
    customTitle = new QLineEdit(titleGroup, "customTitle");
    connect(customTitle, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tLayout->addWidget(customTitle, row, 1);
    ++row;

    customUnitsToggle = new QCheckBox("Custom Units", titleGroup,
        "customUnitsToggle");
    connect(customUnitsToggle, SIGNAL(toggled(bool)),
            this, SLOT(customUnitsToggled(bool)));
    tLayout->addWidget(customUnitsToggle, row, 0);
    customUnits = new QLineEdit(titleGroup, "customUnits");
    connect(customUnits, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tLayout->addWidget(customUnits, row, 1);
    ++row;

    QFrame *titleSep = new QFrame(titleGroup, "labelSep");
    titleSep->setFrameStyle(QFrame::HLine + QFrame::Sunken);
    tLayout->addMultiCellWidget(titleSep, row, row, 0, 1);
    ++row;

    titleFont = new QvisFontAttributesWidget(titleGroup, "titleFont");
#ifdef FONTS_ARE_DISABLED
    titleFont->setEnabled(false);
#endif
    connect(titleFont, SIGNAL(fontChanged(const FontAttributes &)),
            this, SLOT(titleFontChanged(const FontAttributes &)));
    tLayout->addMultiCellWidget(titleFont, row, row, 0, 1);

    //
    // Create the label group
    //
    row = 0;
    labelGroup = new QGroupBox(this, "labelGroup");
    labelGroup->setTitle("Labels");
    labelGroup->setCheckable(true);
    connect(labelGroup, SIGNAL(toggled(bool)),
            this, SLOT(labelToggled(bool)));
    QVBoxLayout *lInnerLayout = new QVBoxLayout(labelGroup);
    lInnerLayout->setMargin(10);
    lInnerLayout->addSpacing(15);
    lInnerLayout->setSpacing(10);
    QGridLayout *lLayout = new QGridLayout(lInnerLayout, 3, 2);
    lLayout->setSpacing(5);
    lLayout->setColStretch(1, 10);

    labelScaling = new QSpinBox(0, 300, 1, labelGroup, "labelScaling");
    connect(labelScaling, SIGNAL(valueChanged(int)),
            this, SLOT(labelScalingChanged(int)));
    labelScalingLabel = new QLabel(labelScaling, "Scaling (x10^?)", 
        labelGroup, "labelScalingLabel");
    lLayout->addWidget(labelScalingLabel, row, 0);
    lLayout->addWidget(labelScaling, row, 1);
    ++row;

    QFrame *labelSep = new QFrame(labelGroup, "labelSep");
    labelSep->setFrameStyle(QFrame::HLine + QFrame::Sunken);
    lLayout->addMultiCellWidget(labelSep, row, row, 0, 1);
    ++row;

    labelFont = new QvisFontAttributesWidget(labelGroup, "labelFont");
#ifdef FONTS_ARE_DISABLED
    labelFont->setEnabled(false);
#endif
    connect(labelFont, SIGNAL(fontChanged(const FontAttributes &)),
            this, SLOT(labelFontChanged(const FontAttributes &)));
    lLayout->addMultiCellWidget(labelFont, row, row, 0, 1);

    //
    // Create the tick group
    //
    row = 0;
    tickGroup = new QGroupBox(this, "tickGroup");
    tickGroup->setTitle("Tick marks");
    if(tickEnabled)
    {
        tickGroup->setCheckable(true);
        connect(tickGroup, SIGNAL(toggled(bool)),
                this, SLOT(tickToggled(bool)));
    }
    QVBoxLayout *tgInnerLayout = new QVBoxLayout(tickGroup);
    tgInnerLayout->setMargin(10);
    tgInnerLayout->addSpacing(15);
    tgInnerLayout->setSpacing(10);
    QGridLayout *tgLayout = new QGridLayout(tgInnerLayout, 4, 2);
    tgLayout->setSpacing(5);

    majorMinimum = new QNarrowLineEdit(tickGroup, "majorMinimum");
    connect(majorMinimum, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tgLayout->addWidget(majorMinimum, row, 1);
    majorMinimumLabel = new QLabel(majorMinimum, "Major minimum", 
        tickGroup, "majorMinimumLabel");
    tgLayout->addWidget(majorMinimumLabel, row, 0);
    ++row;

    majorMaximum = new QNarrowLineEdit(tickGroup, "majorMaximum");
    connect(majorMaximum, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tgLayout->addWidget(majorMaximum, row, 1);
    majorMaximumLabel = new QLabel(majorMaximum, "Major maximum", 
        tickGroup, "majorMaximumLabel");
    tgLayout->addWidget(majorMaximumLabel, row, 0);
    ++row;

    minorSpacing = new QNarrowLineEdit(tickGroup, "minorSpacing");
    connect(minorSpacing, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tgLayout->addWidget(minorSpacing, row, 1);
    minorSpacingLabel = new QLabel(minorSpacing, "Minor spacing", 
        tickGroup, "minorSpacingLabel");
    tgLayout->addWidget(minorSpacingLabel, row, 0);
    ++row;

    majorSpacing = new QNarrowLineEdit(tickGroup, "majorSpacing");
    connect(majorSpacing, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tgLayout->addWidget(majorSpacing, row, 1);
    majorSpacingLabel = new QLabel(majorSpacing, "Major spacing", 
        tickGroup, "majorSpacingLabel");
    tgLayout->addWidget(majorSpacingLabel, row, 0);
    ++row;

    // Add the grid check box.
    grid = new QCheckBox("Show grid", this, "grid");
    connect(grid, SIGNAL(toggled(bool)),
            this, SLOT(gridToggled(bool)));
}

// ****************************************************************************
// Method: QvisAxisAttributesWidget::~QvisAxisAttributesWidget
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:50:56 PST 2008
//
// Modifications:
//   
// ****************************************************************************

QvisAxisAttributesWidget::~QvisAxisAttributesWidget()
{
}

// ****************************************************************************
// Method: QvisAxisAttributesWidget::setAxisAttributes
//
// Purpose: 
//   Set the axis attributes and update the widget.
//
// Arguments:
//   aa : The new attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:51:09 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAxisAttributesWidget::setAxisAttributes(const AxisAttributes &aa)
{
    atts = aa;
    Update(-1, -1); 
}

// ****************************************************************************
// Method: QvisAxisAttributesWidget::getAxisAttributes
//
// Purpose: 
//   Gets the current axis attributes and returns them.
//
// Arguments:
//
// Returns:    The new axis attributes.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:51:53 PST 2008
//
// Modifications:
//   
// ****************************************************************************

const AxisAttributes &
QvisAxisAttributesWidget::getAxisAttributes()
{
    GetCurrentValues(atts, -1);
    return atts;
}

void
QvisAxisAttributesWidget::setAutoScaling(bool val)
{
    autoScaling = val;
    Update(AxisAttributes::ID_label, AxisLabels::ID_scaling);
}

void
QvisAxisAttributesWidget::setAutoTickMarks(bool val)
{
    autoTickMarks = val;
    Update(AxisAttributes::ID_tickMarks, -1);
}

// ****************************************************************************
// Method: QvisAxisAttributesWidget::Update
//
// Purpose: 
//   Called to update the widgets with the axis attributes.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:52:33 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAxisAttributesWidget::Update(int which_group, int which_widget)
{
    bool doAllGroups = which_group == -1;
    bool doAll = which_widget == -1;

    // Do the title group
    if(doAllGroups || which_group == AxisAttributes::ID_title)
    {
        const AxisTitles &title = atts.GetTitle();
        if(doAll || which_widget == AxisTitles::ID_visible)
        {
            customTitleToggle->setEnabled(title.GetVisible());
            customTitle->setEnabled(title.GetVisible());
            customUnitsToggle->setEnabled(title.GetVisible());
            customUnits->setEnabled(title.GetVisible());
#ifndef FONTS_ARE_DISABLED
            titleFont->setEnabled(title.GetVisible());
#endif
        }
        if(doAll || which_widget == AxisTitles::ID_font)
        {
            titleFont->setFontAttributes(title.GetFont());
        }
        if(doAll || which_widget == AxisTitles::ID_userTitle)
        {
            customTitleToggle->blockSignals(true);
            customTitleToggle->setChecked(title.GetUserTitle());
            customTitleToggle->blockSignals(false);

            customTitle->setEnabled(title.GetUserTitle());
        }
        if(doAll || which_widget == AxisTitles::ID_userUnits)
        {
            customUnitsToggle->blockSignals(true);
            customUnitsToggle->setChecked(title.GetUserUnits());
            customUnitsToggle->blockSignals(false);

            customUnits->setEnabled(title.GetUserUnits());
        }
        if(doAll || which_widget == AxisTitles::ID_title)
        {
            customTitle->setText(title.GetTitle().c_str());
        }
        if(doAll || which_widget == AxisTitles::ID_units)
        {
            customUnits->setText(title.GetUnits().c_str());
        }
    }

    // Do the label group
    if(doAllGroups || which_group == AxisAttributes::ID_label)
    {
        const AxisLabels &label = atts.GetLabel();
        if(doAll || which_widget == AxisLabels::ID_visible)
        {
            labelScalingLabel->setEnabled(label.GetVisible());
            labelScaling->setEnabled(label.GetVisible());
#ifndef FONTS_ARE_DISABLED
            labelFont->setEnabled(label.GetVisible());
#endif
        }
        if(doAll || which_widget == AxisLabels::ID_font)
        {
            labelFont->setFontAttributes(label.GetFont());
        }
        if(doAll || which_widget == AxisLabels::ID_scaling)
        {
            labelScaling->blockSignals(true);
            labelScaling->setValue(label.GetScaling());
            labelScaling->blockSignals(false);

            labelScaling->setEnabled(!autoScaling);
            labelScalingLabel->setEnabled(!autoScaling);
        }
    }

    // Do the tick mark group
    if(doAllGroups || which_group == AxisAttributes::ID_tickMarks)
    {
        const AxisTickMarks &ticks = atts.GetTickMarks();
        if(doAll || which_widget == AxisTickMarks::ID_visible)
        {
            bool e = !autoTickMarks && ticks.GetVisible();
            majorMinimumLabel->setEnabled(ticks.GetVisible());
            majorMinimum->setEnabled(e);
            majorMaximumLabel->setEnabled(ticks.GetVisible());
            majorMaximum->setEnabled(e);
            minorSpacingLabel->setEnabled(ticks.GetVisible());
            minorSpacing->setEnabled(e);
            majorSpacingLabel->setEnabled(ticks.GetVisible());
            majorSpacing->setEnabled(e);
        }
        if(doAll || which_widget == AxisTickMarks::ID_majorMinimum)
        {
            QString val; val.setNum(ticks.GetMajorMinimum());
            majorMinimum->setText(val);
            majorMinimum->setEnabled(!autoTickMarks);
        }
        if(doAll || which_widget == AxisTickMarks::ID_majorMaximum)
        {
            QString val; val.setNum(ticks.GetMajorMaximum());
            majorMaximum->setText(val);
            majorMaximum->setEnabled(!autoTickMarks);
        }
        if(doAll || which_widget == AxisTickMarks::ID_minorSpacing)
        {
            QString val; val.setNum(ticks.GetMinorSpacing());
            minorSpacing->setText(val);
            minorSpacing->setEnabled(!autoTickMarks);
        }
        if(doAll || which_widget == AxisTickMarks::ID_majorSpacing)
        {
            QString val; val.setNum(ticks.GetMajorSpacing());
            majorSpacing->setText(val);
            majorSpacing->setEnabled(!autoTickMarks);
        }
    }

    // Do the grid check box.
    if(doAllGroups || which_group == AxisAttributes::ID_grid)
    {
        grid->blockSignals(true);
        grid->setChecked(atts.GetGrid());
        grid->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisAxisAttributesWidget::GetCurrentValues
//
// Purpose: 
//   Called when we need to get the current values for widgets that may
//   not always generate signals.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:52:49 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAxisAttributesWidget::GetCurrentValues(AxisAttributes &aa, 
    int which_group, int which_widget)
{
    bool doAllGroups = which_group == -1;
    bool doAll = which_widget == -1;

    // Do the title group
    if(doAllGroups || which_group == AxisAttributes::ID_title)
    {
        if(doAll || which_widget == AxisTitles::ID_font)
            atts.GetTitle().SetFont(titleFont->getFontAttributes());
        if(doAll || which_widget == AxisTitles::ID_title)
            atts.GetTitle().SetTitle(customTitle->displayText().latin1());
        if(doAll || which_widget == AxisTitles::ID_units)
            atts.GetTitle().SetUnits(customUnits->displayText().latin1());
    }

    // Do the label group
    if(doAllGroups || which_group == AxisAttributes::ID_title)
    {
        if(doAll || which_widget == AxisLabels::ID_font)
            atts.GetLabel().SetFont(labelFont->getFontAttributes());
        if(doAll || which_widget == AxisLabels::ID_scaling)
        {
            ForceSpinBoxUpdate(labelScaling);
            atts.GetLabel().SetScaling(labelScaling->value());
        }
    }

    // Do the ticks group
    if(doAllGroups || which_group == AxisAttributes::ID_tickMarks)
    {
        if(doAll || which_widget == AxisTickMarks::ID_majorMinimum)
        {
            double val;
            if(GetDouble(val, majorMinimum, "majorMinimum"))
                atts.GetTickMarks().SetMajorMinimum(val);
        }
        if(doAll || which_widget == AxisTickMarks::ID_majorMaximum)
        {
            double val;
            if(GetDouble(val, majorMaximum, "majorMaximum"))
                atts.GetTickMarks().SetMajorMaximum(val);
        }
        if(doAll || which_widget == AxisTickMarks::ID_minorSpacing)
        {
            double val;
            if(GetDouble(val, minorSpacing, "minorSpacing"))
                atts.GetTickMarks().SetMinorSpacing(val);
        }
        if(doAll || which_widget == AxisTickMarks::ID_majorSpacing)
        {
            double val;
            if(GetDouble(val, majorSpacing, "majorSpacing"))
                atts.GetTickMarks().SetMajorSpacing(val);
        }
    }
}

// ****************************************************************************
// Method: QvisAxisAttributesWidget::GetDouble
//
// Purpose: 
//   Gets a double from a line edit.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:53:35 PST 2008
//
// Modifications:
//   
// ****************************************************************************

bool
QvisAxisAttributesWidget::GetDouble(double &val, QLineEdit *le, const QString &name)
{
    QString temp(le->displayText().simplifyWhiteSpace());
    bool okay = !temp.isEmpty();
    if(okay)
    {
        double d = temp.toDouble(&okay);
        if(okay)
            val = d;
    }
    if(!okay)
    {
        QString msg;
        msg.sprintf("An invalid value was provided for %s.", name.latin1());
        Message(msg);
    }
    return okay;
}

// ****************************************************************************
// Method: QvisAxisAttributesWidget::ForceSpinBoxUpdate
//
// Purpose: 
//   Forces a spin box to update so we can get its new value.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 17:53:47 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisAxisAttributesWidget::ForceSpinBoxUpdate(QSpinBox *sb)
{
    // Block signals.
    sb->blockSignals(true);

    // textChanged is protected and the compiler does not let us call
    // it directly so call it as a slot. We do this to make the spin box
    // think that it has new text input so it will parse it again when
    // we call value().
    QTimer::singleShot(0, sb, SLOT(textChanged()));

    // Call the value function to make the spin box parse the new value.
    sb->value();

    // Let the spinbox emit signals again.
    sb->blockSignals(false);
}

//
// Qt slot functions
//
void
QvisAxisAttributesWidget::Apply()
{
    GetCurrentValues(atts, -1, -1);
    emit axisChanged(atts);
}

void
QvisAxisAttributesWidget::titleToggled(bool val)
{
    atts.GetTitle().SetVisible(val);
    Update(AxisAttributes::ID_title, AxisTitles::ID_visible);
    Apply();
}

void
QvisAxisAttributesWidget::customTitleToggled(bool val)
{
    atts.GetTitle().SetUserTitle(val);
    Update(AxisAttributes::ID_title, AxisTitles::ID_userTitle);
    Apply();
}

void
QvisAxisAttributesWidget::customUnitsToggled(bool val)
{
    atts.GetTitle().SetUserUnits(val);
    Update(AxisAttributes::ID_title, AxisTitles::ID_userUnits);
    Apply();
}

void
QvisAxisAttributesWidget::titleFontChanged(const FontAttributes &f)
{
    atts.GetTitle().SetFont(f);
    Apply();
}

void
QvisAxisAttributesWidget::labelToggled(bool val)
{
    atts.GetLabel().SetVisible(val);
    Update(AxisAttributes::ID_label, AxisLabels::ID_visible);
    Apply();
}

void
QvisAxisAttributesWidget::labelScalingChanged(int scaling)
{
    atts.GetLabel().SetScaling(scaling);
    Apply();
}

void
QvisAxisAttributesWidget::labelFontChanged(const FontAttributes &f)
{
    atts.GetLabel().SetFont(f);
    Apply();
}

void
QvisAxisAttributesWidget::tickToggled(bool val)
{
    atts.GetTickMarks().SetVisible(val);
    Update(AxisAttributes::ID_tickMarks, AxisTickMarks::ID_visible);
    Apply();
}

void
QvisAxisAttributesWidget::gridToggled(bool val)
{
    atts.SetGrid(val);
    Apply();
}
