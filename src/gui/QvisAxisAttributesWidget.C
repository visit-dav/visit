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
#include <QvisAxisAttributesWidget.h>

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimer>

#include <QNarrowLineEdit.h>
#include <QvisFontAttributesWidget.h>

#define DISABLE_TEXT_OPACITY

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
//   Brad Whitlock, Thu Mar 27 09:59:40 PDT 2008
//   Temporarily disable text opacity.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Thu Jun 26 13:33:59 PDT 2008
//   Qt 4.
//
//   Kathleen Bonnell, Mon Sep 22 18:37:29 PDT 2008 
//   Allow labelScaling values to be negative. 
//
//   Jeremy Meredith, Fri Jan 16 11:12:48 EST 2009
//   Allow clients to not expose the "ShowGrid" and custom title/units settings
//
// ****************************************************************************

QvisAxisAttributesWidget::QvisAxisAttributesWidget(QWidget *parent, 
    bool tickEnabled, bool titleEnabled,
    bool showGridEnabled, bool customTitleAndUnitsEnabled) :
    QWidget(parent), GUIBase(), atts()
{
    autoScaling = true;
    autoTickMarks = true;
    QVBoxLayout *vbLayout = new QVBoxLayout(this);
    vbLayout->setMargin(5);
    vbLayout->setSpacing(10);

    //
    // Create the title group
    //
    titleGroup = new QGroupBox(this);
    titleGroup->setTitle(tr("Title"));
    vbLayout->addWidget(titleGroup);
    if(titleEnabled)
    {
        titleGroup->setCheckable(true);
        connect(titleGroup, SIGNAL(toggled(bool)),
                this, SLOT(titleToggled(bool)));
    }
    int row = 0;
    QGridLayout *tLayout = new QGridLayout(titleGroup);
    tLayout->setSpacing(5);
    customTitleToggle = new QCheckBox(tr("Custom title"), titleGroup);
    connect(customTitleToggle, SIGNAL(toggled(bool)),
            this, SLOT(customTitleToggled(bool)));
    tLayout->addWidget(customTitleToggle, row, 0);
    customTitle = new QLineEdit(titleGroup);
    connect(customTitle, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tLayout->addWidget(customTitle, row, 1);
    ++row;
    if (!customTitleAndUnitsEnabled)
    {
        customTitleToggle->hide();
        customTitle->hide();
    }

    customUnitsToggle = new QCheckBox(tr("Custom Units"), titleGroup);
    connect(customUnitsToggle, SIGNAL(toggled(bool)),
            this, SLOT(customUnitsToggled(bool)));
    tLayout->addWidget(customUnitsToggle, row, 0);
    customUnits = new QLineEdit(titleGroup);
    connect(customUnits, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tLayout->addWidget(customUnits, row, 1);
    ++row;
    if (!customTitleAndUnitsEnabled)
    {
        customUnitsToggle->hide();
        customUnits->hide();
    }

    QFrame *titleSep = new QFrame(titleGroup);
    titleSep->setFrameStyle(QFrame::HLine + QFrame::Sunken);
    tLayout->addWidget(titleSep, row, 0, 1, 2);
    ++row;

    titleFont = new QvisFontAttributesWidget(titleGroup);
#ifdef DISABLE_TEXT_OPACITY
    titleFont->disableOpacity();
#endif
    connect(titleFont, SIGNAL(fontChanged(const FontAttributes &)),
            this, SLOT(titleFontChanged(const FontAttributes &)));
    tLayout->addWidget(titleFont, row, 0, 1, 2);

    //
    // Create the label group
    //
    row = 0;
    labelGroup = new QGroupBox(this);
    vbLayout->addWidget(labelGroup);
    labelGroup->setTitle(tr("Labels"));
    labelGroup->setCheckable(true);
    connect(labelGroup, SIGNAL(toggled(bool)),
            this, SLOT(labelToggled(bool)));
    QGridLayout *lLayout = new QGridLayout(labelGroup);
    lLayout->setSpacing(5);
    lLayout->setColumnStretch(1, 10);

    labelScaling = new QSpinBox(labelGroup);
    labelScaling->setMinimum(-300);
    labelScaling->setMaximum(300);
    connect(labelScaling, SIGNAL(valueChanged(int)),
            this, SLOT(labelScalingChanged(int)));
    labelScalingLabel = new QLabel(tr("Scaling (x10^?)"), labelGroup);
    labelScalingLabel->setBuddy(labelScaling);
    lLayout->addWidget(labelScalingLabel, row, 0);
    lLayout->addWidget(labelScaling, row, 1);
    ++row;

    QFrame *labelSep = new QFrame(labelGroup);
    labelSep->setFrameStyle(QFrame::HLine + QFrame::Sunken);
    lLayout->addWidget(labelSep, row, 0, 1, 2);
    ++row;

    labelFont = new QvisFontAttributesWidget(labelGroup);
#ifdef DISABLE_TEXT_OPACITY
    labelFont->disableOpacity();
#endif
    connect(labelFont, SIGNAL(fontChanged(const FontAttributes &)),
            this, SLOT(labelFontChanged(const FontAttributes &)));
    lLayout->addWidget(labelFont, row, 0, 1, 2);

    //
    // Create the tick group
    //
    row = 0;
    tickGroup = new QGroupBox(this);
    vbLayout->addWidget(tickGroup);
    tickGroup->setTitle(tr("Tick marks"));
    if(tickEnabled)
    {
        tickGroup->setCheckable(true);
        connect(tickGroup, SIGNAL(toggled(bool)),
                this, SLOT(tickToggled(bool)));
    }
    QGridLayout *tgLayout = new QGridLayout(tickGroup);
    tgLayout->setSpacing(5);

    majorMinimum = new QNarrowLineEdit(tickGroup);
    connect(majorMinimum, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tgLayout->addWidget(majorMinimum, row, 1);
    majorMinimumLabel = new QLabel(tr("Major minimum"), tickGroup);
    majorMinimumLabel->setBuddy(majorMinimum);
    tgLayout->addWidget(majorMinimumLabel, row, 0);
    ++row;

    majorMaximum = new QNarrowLineEdit(tickGroup);
    connect(majorMaximum, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tgLayout->addWidget(majorMaximum, row, 1);
    majorMaximumLabel = new QLabel(tr("Major maximum"), tickGroup);
    majorMaximumLabel->setBuddy(majorMaximum);
    tgLayout->addWidget(majorMaximumLabel, row, 0);
    ++row;

    minorSpacing = new QNarrowLineEdit(tickGroup);
    connect(minorSpacing, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tgLayout->addWidget(minorSpacing, row, 1);
    minorSpacingLabel = new QLabel(tr("Minor spacing"), tickGroup);
    minorSpacingLabel->setBuddy(minorSpacing);
    tgLayout->addWidget(minorSpacingLabel, row, 0);
    ++row;

    majorSpacing = new QNarrowLineEdit(tickGroup);
    connect(majorSpacing, SIGNAL(returnPressed()),
            this, SLOT(Apply()));
    tgLayout->addWidget(majorSpacing, row, 1);
    majorSpacingLabel = new QLabel(tr("Major spacing"), tickGroup);
    majorSpacingLabel->setBuddy(majorSpacing);
    tgLayout->addWidget(majorSpacingLabel, row, 0);
    ++row;

    // Add the grid check box.
    grid = new QCheckBox(tr("Show grid"), this);
    connect(grid, SIGNAL(toggled(bool)),
            this, SLOT(gridToggled(bool)));
    vbLayout->addWidget(grid);
    vbLayout->addStretch(100);
    if (!showGridEnabled)
        grid->hide();
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
            atts.GetTitle().SetTitle(customTitle->displayText().toStdString());
        if(doAll || which_widget == AxisTitles::ID_units)
            atts.GetTitle().SetUnits(customUnits->displayText().toStdString());
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
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

bool
QvisAxisAttributesWidget::GetDouble(double &val, QLineEdit *le, const QString &name)
{
    QString temp(le->displayText().simplified());
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
        msg = tr("An invalid value was provided for ") + name + QString(".");
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
// No longer needed...
#if 0
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
#endif
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
