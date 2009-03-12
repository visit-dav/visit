/*****************************************************************************
*
* Copyright (c) 2000 - 2008, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "QvisParallelCoordinatesPlotWindow.h"

#include <ParallelCoordinatesAttributes.h>
#include <ViewerProxy.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QWidget>
#include <QNarrowLineEdit.h>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>

#include <math.h>
#include <stdio.h>
#include <string>

using std::string;

// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWindow::QvisParallelCoordinatesPlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: xml2window
// Creation:   Thu Mar 15 13:59:40 PST 2007
//
// Modifications:
//   
// ****************************************************************************

QvisParallelCoordinatesPlotWindow::QvisParallelCoordinatesPlotWindow(const int type,
                         ParallelCoordinatesAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWindow::~QvisParallelCoordinatesPlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: xml2window
// Creation:   Thu Mar 15 13:59:40 PST 2007
//
// Modifications:
//   
// ****************************************************************************

QvisParallelCoordinatesPlotWindow::~QvisParallelCoordinatesPlotWindow()
{
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Jeremy Meredith
// Creation:   March 16, 2007
//
// Modifications:
//    Jeremy Meredith, Wed Mar 21 18:20:47 EDT 2007
//    Added a checkbox to allow the lines to be hidden when the extents
//    tool has not limited the viewing range to a focus.
//   
//    Jeremy Meredith, Fri Feb  8 12:34:19 EST 2008
//    Added ability to unify extents across all axes.
//
//    Jeremy Meredith, Fri Feb  8 16:12:06 EST 2008
//    Changed axis list to QTreeView to support multiple columns.
//    Added min/max extents columns for each axis, and a button to reset them.
//
//    Jeremy Meredith, Mon Feb 18 16:18:06 EST 2008
//    Changed some wording slightly.
//
//    Brad Whitlock, Wed Apr 23 10:07:16 PDT 2008
//    Added tr()'s
//
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
//    Jeremy Meredith, Wed Feb 25 12:55:54 EST 2009
//    Added number of bins for line drawing.
//    Allow user to force into the mode using individual data point lines
//    for the focus instead of using a histogram.
//
//    Jeremy Meredith, Thu Mar 12 13:12:37 EDT 2009
//    Qt4 port of new additions.
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::CreateWindowContents()
{
    //
    // Axes to plot
    //
    axisGroup = new QGroupBox(tr("Axes"),central);
    topLayout->addWidget(axisGroup);
    QGridLayout *axisLayout = new QGridLayout(axisGroup);

    // axes list
    
    axisTree = new QTreeWidget(axisGroup);
    axisTree->setSortingEnabled(false);
    axisTree->setRootIsDecorated(false);
    
    QTreeWidgetItem *header = new QTreeWidgetItem();
    header->setText(0,tr("Axis"));
    header->setText(1,tr("Min"));
    header->setText(2,tr("Max"));
    axisTree->setHeaderItem(header);
        
    axisLayout->addWidget(axisTree, 0,0, 4,1);
    connect(axisTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(axisSelected(QTreeWidgetItem*)));

    // axes new/del/up/down buttons
    axisNewButton = new QvisVariableButton(false, true, true,
                                           QvisVariableButton::Scalars,
                                           axisGroup);
    axisNewButton->setText(tr("Add axis"));
    axisNewButton->setChangeTextOnVariableChange(false);
    axisLayout->addWidget(axisNewButton, 0, 1);
    connect(axisNewButton, SIGNAL(activated(const QString &)),
            this, SLOT(addAxis(const QString &)));

    axisDelButton = new QPushButton(tr("Delete"), axisGroup);
    axisLayout->addWidget(axisDelButton, 1, 1);
    connect(axisDelButton, SIGNAL(clicked()),
            this, SLOT(delAxis()));

    axisUpButton = new QPushButton(tr("Move up"), axisGroup);
    axisLayout->addWidget(axisUpButton, 2, 1);
    connect(axisUpButton, SIGNAL(clicked()),
            this, SLOT(moveAxisUp()));

    axisDownButton = new QPushButton(tr("Move down"), axisGroup);
    axisLayout->addWidget(axisDownButton, 3, 1);
    connect(axisDownButton, SIGNAL(clicked()),
            this, SLOT(moveAxisDown()));

    axisResetExtentsButton = new QPushButton(tr("Reset all axis restrictions"),
                                             axisGroup);
    axisLayout->addWidget(axisResetExtentsButton, 4, 0);
    connect(axisResetExtentsButton, SIGNAL(clicked()),
            this, SLOT(resetAxisExtents()));

    //
    // Draw lines, and the needed settings
    //
    drawLines = new QGroupBox(tr("Draw individual lines"),central);
    drawLines->setCheckable(true);
    connect(drawLines, SIGNAL(toggled(bool)),
            this, SLOT(drawLinesChanged(bool)));
    topLayout->addWidget(drawLines);

    QVBoxLayout *linesSpacingLayout = new QVBoxLayout(drawLines);

    QGridLayout *linesLayout = new QGridLayout();
    linesSpacingLayout->addLayout(linesLayout );

    // Lines color
    linesOnlyIfExtents = new QCheckBox(
                         tr("... but only when axis extents have been restricted"),
                         drawLines);
    connect(linesOnlyIfExtents, SIGNAL(toggled(bool)),
            this, SLOT(linesOnlyIfExtentsToggled(bool)));
    linesLayout->addWidget(linesOnlyIfExtents, 0,0, 1,2);

    // Lines color
    linesColorLabel = new QLabel(tr("Line color"), drawLines);
    linesLayout->addWidget(linesColorLabel,1,0);
    linesColor = new QvisColorButton(drawLines);
    connect(linesColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(linesColorChanged(const QColor&)));
    linesLayout->addWidget(linesColor, 1,1);

    // Number of partitions
    linesNumPartitionsLabel = new QLabel(tr("Vertical screen resolution"),
                                         drawLines);
    linesLayout->addWidget(linesNumPartitionsLabel,2,0);
    linesNumPartitions = new QNarrowLineEdit(drawLines);
    connect(linesNumPartitions, SIGNAL(returnPressed()),
            this, SLOT(linesNumPartitionsProcessText()));
    linesLayout->addWidget(linesNumPartitions, 2,1);
    linesNumPartitionsSlider = new QSlider(Qt::Horizontal,drawLines);
    linesNumPartitionsSlider->setRange(1,7);
    linesNumPartitionsSlider->setPageStep(1);
    linesNumPartitionsSlider->setValue(4);
    connect(linesNumPartitionsSlider, SIGNAL(valueChanged(int)),
            this, SLOT(linesNumPartitionsSliderChanged(int)));
    connect(linesNumPartitionsSlider, SIGNAL(sliderReleased()),
            this, SLOT(linesNumPartitionsSliderReleased()));
    linesLayout->addWidget(linesNumPartitionsSlider, 2,2);

    //
    // Draw context, and the needed settings
    //
    drawContext = new QGroupBox(tr("Draw context"),central);
    drawContext->setCheckable(true);
    connect(drawContext, SIGNAL(toggled(bool)),
            this, SLOT(drawContextChanged(bool)));
    topLayout->addWidget(drawContext);

    QVBoxLayout *contextSpacingLayout = new QVBoxLayout(drawContext);

    QGridLayout *contextLayout = new QGridLayout();
    contextSpacingLayout->addLayout(contextLayout);

    // Contex gamma correction
    contextGammaLabel = new QLabel(tr("Brightness (gamma)"),drawContext);
    contextLayout->addWidget(contextGammaLabel,0,0);
    contextGamma = new QNarrowLineEdit(drawContext);
    connect(contextGamma, SIGNAL(returnPressed()),
            this, SLOT(contextGammaProcessText()));
    contextLayout->addWidget(contextGamma, 0,1);
    contextGammaSlider = new QSlider(Qt::Horizontal,drawContext);
    contextGammaSlider->setRange(0,100);
    contextGammaSlider->setPageStep(5);
    contextGammaSlider->setValue(66);
    
                                     
    connect(contextGammaSlider, SIGNAL(valueChanged(int)),
            this, SLOT(contextGammaSliderChanged(int)));
    connect(contextGammaSlider, SIGNAL(sliderReleased()),
            this, SLOT(contextGammaSliderReleased()));
    contextLayout->addWidget(contextGammaSlider, 0,2);

    // Number of partitions
    contextNumPartitionsLabel = new QLabel(tr("Number of partitions"),
                                           drawContext);
    contextLayout->addWidget(contextNumPartitionsLabel,1,0);
    contextNumPartitions = new QNarrowLineEdit(drawContext);
    connect(contextNumPartitions, SIGNAL(returnPressed()),
            this, SLOT(contextNumPartitionsProcessText()));
    contextLayout->addWidget(contextNumPartitions, 1,1);
    contextNumPartitionsSlider = new QSlider(Qt::Horizontal,drawContext);
    contextNumPartitionsSlider->setRange(1,10);
    contextNumPartitionsSlider->setPageStep(1);
    contextNumPartitionsSlider->setValue(5);

    connect(contextNumPartitionsSlider, SIGNAL(valueChanged(int)),
            this, SLOT(contextNumPartitionsSliderChanged(int)));
    connect(contextNumPartitionsSlider, SIGNAL(sliderReleased()),
            this, SLOT(contextNumPartitionsSliderReleased()));
    contextLayout->addWidget(contextNumPartitionsSlider, 1,2);

    // Context color
    contextColorLabel = new QLabel(tr("Context color"),drawContext);
    contextLayout->addWidget(contextColorLabel,2,0);
    contextColor = new QvisColorButton(drawContext);
    connect(contextColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(contextColorChanged(const QColor&)));
    contextLayout->addWidget(contextColor, 2,1);

    // Unify axis extents
    unifyAxisExtents = new QCheckBox(tr("Unify the data extents across all axes"),
                                     central);
    connect(unifyAxisExtents, SIGNAL(toggled(bool)),
            this, SLOT(unifyAxisExtentsToggled(bool)));
    topLayout->addWidget(unifyAxisExtents);

    // Force individual data point lines for focus
    forceIndividualLineFocusToggle =
        new QCheckBox(tr("Force individual data point lines for focus"),
                      central);
    connect(forceIndividualLineFocusToggle, SIGNAL(toggled(bool)),
            this, SLOT(forceIndividualLineFocusToggled(bool)));
    topLayout->addWidget(forceIndividualLineFocusToggle);
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Jeremy Meredith
// Creation:   March 16, 2007
//
// Modifications:
//    Jeremy Meredith, Wed Mar 21 18:20:47 EDT 2007
//    Added a checkbox to allow the lines to be hidden when the extents
//    tool has not limited the viewing range to a focus.
//   
//    Jeremy Meredith, Thu Feb  7 17:42:48 EST 2008
//    For an empty list of axis names, disable critical widgets and
//    put in a useful message.
//
//    Jeremy Meredith, Fri Feb  8 12:34:19 EST 2008
//    Added ability to unify extents across all axes.  Also fixed typo.
//
//    Jeremy Meredith, Fri Feb  8 16:12:06 EST 2008
//    Changed axis list to QTreeView to support multiple columns.
//    Added min/max extents columns for each axis, and a button to reset them.
//
//    Jeremy Meredith, Fri Feb 15 13:16:46 EST 2008
//    Renamed orderedAxisNames to scalarAxisNames to distinguish these
//    as names of actual scalars instead of just display names.  Added
//    visualAxisNames.
//
//    Jeremy Meredith, Mon Feb 18 16:17:21 EST 2008
//    Don't enable de/up/down buttons if we were created from an array var.
//
//    Cyrus Harrison, Mon Feb 25 13:42:21 PST 2008
//    Resolved AIX QString init error. 
//
//    Kathleen Bonnell, Thu Mar  6 09:47:34 PST 2008
//    Cannot convert a std::string to a QString on windows, must use .c_str().
//
//    Brad Whitlock, Wed Apr 23 10:09:42 PDT 2008
//    Added tr()
//
//    Kathleen Bonnel, Wed Jun 4 07:58:48 PDT 2008
//    Removed unused variables.
//
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
//    Jeremy Meredith, Thu Mar 12 13:12:37 EDT 2009
//    Qt4 port of new additions.
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;

    QString oldAxis = axisTree->currentItem() ? 
        axisTree->currentItem()->text(0) : QString("");

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        int   sliderpos;
        QColor                tempcolor;
        switch(i)
        {
          case ParallelCoordinatesAttributes::ID_visualAxisNames:
          case ParallelCoordinatesAttributes::ID_extentMinima:
          case ParallelCoordinatesAttributes::ID_extentMaxima:
            axisTree->blockSignals(true);
            axisTree->clear();
            for (int ax=0; ax<atts->GetExtentMinima().size(); ax++)
            {
                QString name, emin("min"), emax("max");
                if (atts->GetExtentMinima()[ax] > -1e+37)
                    emin.sprintf("%f",atts->GetExtentMinima()[ax]);
                if (atts->GetExtentMaxima()[ax] < +1e+37)
                    emax.sprintf("%f",atts->GetExtentMaxima()[ax]);
                if (atts->GetVisualAxisNames().size() > ax)
                    name = (atts->GetVisualAxisNames()[ax]).c_str();
                else
                {
                    name.sprintf(" %02d",ax);
                    name = tr("Axis") + name;
                }
                QTreeWidgetItem *item =
                    new QTreeWidgetItem(axisTree);
                item->setText(0,name);
                item->setText(1,emin);
                item->setText(2,emax);
            }
            axisTree->resizeColumnToContents(0);
            axisTree->resizeColumnToContents(1);
            axisTree->resizeColumnToContents(2);
            axisTree->blockSignals(false);
            break;
          case ParallelCoordinatesAttributes::ID_drawLines:
            drawLines->blockSignals(true);
            drawLines->setChecked(atts->GetDrawLines());
            drawLines->blockSignals(false);
            break;
          case ParallelCoordinatesAttributes::ID_linesColor:
            tempcolor = QColor(atts->GetLinesColor().Red(),
                               atts->GetLinesColor().Green(),
                               atts->GetLinesColor().Blue());
            linesColor->blockSignals(true);
            linesColor->setButtonColor(tempcolor);
            linesColor->blockSignals(false);
            break;
          case ParallelCoordinatesAttributes::ID_linesNumPartitions:
            linesNumPartitions->blockSignals(true);
            linesNumPartitionsSlider->blockSignals(true);
            temp.sprintf("%d", atts->GetLinesNumPartitions());
            sliderpos = int(log(float(atts->GetLinesNumPartitions()))/log(2.f)+.5)-5;
            sliderpos = qMin(qMax(1, sliderpos), 10);
            linesNumPartitionsSlider->setValue(sliderpos);
            linesNumPartitions->setText(temp);
            linesNumPartitions->blockSignals(false);
            linesNumPartitionsSlider->blockSignals(false);
            break;
          case ParallelCoordinatesAttributes::ID_drawContext:
            drawContext->blockSignals(true);
            drawContext->setChecked(atts->GetDrawContext());
            drawContext->blockSignals(false);
            break;
          case ParallelCoordinatesAttributes::ID_contextGamma:
            contextGamma->blockSignals(true);
            contextGammaSlider->blockSignals(true);
            temp.sprintf("%.2f", atts->GetContextGamma());
            contextGamma->setText(temp);
            sliderpos = int(50 + 50*log10(atts->GetContextGamma()) + .5);
            sliderpos = qMin(qMax(0, sliderpos), 100);
            contextGammaSlider->setValue(sliderpos);
            contextGamma->blockSignals(false);
            contextGammaSlider->blockSignals(false);
            break;
          case ParallelCoordinatesAttributes::ID_contextNumPartitions:
            contextNumPartitions->blockSignals(true);
            contextNumPartitionsSlider->blockSignals(true);
            temp.sprintf("%d", atts->GetContextNumPartitions());
            sliderpos = int(log((float)atts->GetContextNumPartitions())/log(2.f)+.5);
            sliderpos = qMin(qMax(1, sliderpos), 10);
            contextNumPartitionsSlider->setValue(sliderpos);
            contextNumPartitions->setText(temp);
            contextNumPartitions->blockSignals(false);
            contextNumPartitionsSlider->blockSignals(false);
            break;
          case ParallelCoordinatesAttributes::ID_contextColor:
            tempcolor = QColor(atts->GetContextColor().Red(),
                               atts->GetContextColor().Green(),
                               atts->GetContextColor().Blue());
            contextColor->blockSignals(true);
            contextColor->setButtonColor(tempcolor);
            contextColor->blockSignals(false);
            break;
          case ParallelCoordinatesAttributes::ID_drawLinesOnlyIfExtentsOn:
            linesOnlyIfExtents->blockSignals(true);
            linesOnlyIfExtents->setChecked(atts->GetDrawLinesOnlyIfExtentsOn());
            linesOnlyIfExtents->blockSignals(false);
            break;
          case ParallelCoordinatesAttributes::ID_unifyAxisExtents:
            unifyAxisExtents->blockSignals(true);
            unifyAxisExtents->setChecked(atts->GetUnifyAxisExtents());
            unifyAxisExtents->blockSignals(false);
            break;
          case ParallelCoordinatesAttributes::ID_forceFullDataFocus:
            forceIndividualLineFocusToggle->blockSignals(true);
            forceIndividualLineFocusToggle->setChecked(atts->GetForceFullDataFocus());
            forceIndividualLineFocusToggle->blockSignals(false);
            break;
        }
    }

    // Re-select the previously selected item in case updating this window
    // regenerated the tree contents
    bool found = false;
    int nitems = axisTree->topLevelItemCount();
    for(int i=0;i< nitems && !found; i++)
    {
        QTreeWidgetItem *item = axisTree->topLevelItem(i);
        if(item->text(0) == oldAxis)
        {
            axisTree->setCurrentItem(item);
            found = true;
        }
    }
    
    // Set enabled states
    
    nitems = atts->GetScalarAxisNames().size();
    axisDelButton->setEnabled( nitems > 2 &&
                              axisTree->currentItem()!= NULL);
    axisUpButton->setEnabled(nitems  > 0 &&
                             axisTree->currentItem()!= axisTree->topLevelItem(0));
    axisDownButton->setEnabled(atts->GetScalarAxisNames().size() > 0 &&
                               axisTree->currentItem()!= axisTree->topLevelItem(nitems -1));
    axisNewButton->setEnabled(atts->GetScalarAxisNames().size() > 0);
    axisResetExtentsButton->setEnabled(atts->GetExtentMinima().size() > 0);
    axisTree->setEnabled(atts->GetExtentMinima().size() > 0);
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: Jeremy Meredith
// Creation:   March 16, 2007
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 10:10:28 PDT 2008
//   Added tr()
//
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
//    Jeremy Meredith, Wed Feb 25 13:00:02 EST 2009
//    Added number of bins for line drawing.
//    Added fields for iterating over time.
//    Switched to named indexes.
//
//    Jeremy Meredith, Thu Mar 12 13:12:37 EDT 2009
//    Qt4 port of new additions.
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do contextGamma
    if(which_widget == ParallelCoordinatesAttributes::ID_contextGamma || doAll)
    {
        temp = contextGamma->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            float val = temp.toFloat(&okay);
            if (val>0 && val<1000)
                atts->SetContextGamma(val);
            else
                okay = false;
        }

        if(!okay)
        {
            msg = tr("The value of contextGamma was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetContextGamma());
            Message(msg);
            atts->SetContextGamma(atts->GetContextGamma());
        }
    }

    // Do contextNumPartitions
    if(which_widget == ParallelCoordinatesAttributes::ID_contextNumPartitions || doAll)
    {
        temp = contextNumPartitions->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if (val>0 && val<10000)
                atts->SetContextNumPartitions(val);
            else
                okay = false;
        }

        if(!okay)
        {
            msg = tr("The value of contextNumPartitions was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetContextNumPartitions());
            Message(msg);
            atts->SetContextNumPartitions(atts->GetContextNumPartitions());
        }
    }

    // Do linesNumPartitionsSlider
    if(which_widget == ParallelCoordinatesAttributes::ID_linesNumPartitions || doAll)
    {
        temp = linesNumPartitions->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if (val>0 && val<10000)
                atts->SetLinesNumPartitions(val);
            else
                okay = false;
        }

        if(!okay)
        {
            msg.sprintf("The value of linesNumPartitions was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetLinesNumPartitions());
            Message(msg);
            atts->SetLinesNumPartitions(atts->GetLinesNumPartitions());
        }
    }
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: xml2window
// Creation:   Thu Mar 15 13:59:40 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        atts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: xml2window
// Creation:   Thu Mar 15 13:59:40 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: xml2window
// Creation:   Thu Mar 15 13:59:40 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisParallelCoordinatesPlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: xml2window
// Creation:   Thu Mar 15 13:59:40 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::reset()
{
    GetViewerMethods()->ResetPlotOptions(plotType);
}

// ****************************************************************************
//
//  Standard autogenerated Qt/Qvis slot functions.
//
// ****************************************************************************
void
QvisParallelCoordinatesPlotWindow::drawLinesChanged(bool val)
{
    atts->SetDrawLines(val);
    SetUpdate(false);
    Apply();
}


void
QvisParallelCoordinatesPlotWindow::linesColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    atts->SetLinesColor(temp);
    SetUpdate(false);
    Apply();
}


void
QvisParallelCoordinatesPlotWindow::linesNumPartitionsProcessText()
{
    GetCurrentValues(ParallelCoordinatesAttributes::ID_linesNumPartitions);
    Apply();
}


void
QvisParallelCoordinatesPlotWindow::drawContextChanged(bool val)
{
    atts->SetDrawContext(val);
    SetUpdate(false);
    Apply();
}


void
QvisParallelCoordinatesPlotWindow::contextGammaProcessText()
{
    GetCurrentValues(ParallelCoordinatesAttributes::ID_contextGamma);
    Apply();
}


void
QvisParallelCoordinatesPlotWindow::contextNumPartitionsProcessText()
{
    GetCurrentValues(ParallelCoordinatesAttributes::ID_contextNumPartitions);
    Apply();
}


void
QvisParallelCoordinatesPlotWindow::contextColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    atts->SetContextColor(temp);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::resetAxisExtents
//
//  Purpose:
//    resets all extents to the full min/max range
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  8, 2008
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::resetAxisExtents()
{
    for (int i=0; i<atts->GetExtentMinima().size(); i++)
    {
        atts->GetExtentMinima()[i] = -1e+37;
    }
    for (int i=0; i<atts->GetExtentMaxima().size(); i++)
    {
        atts->GetExtentMaxima()[i] = +1e+37;
    }
    atts->SelectExtentMinima();
    atts->SelectExtentMaxima();
    Apply();
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::axisSelected
//
//  Purpose:
//    Executed when they click on an axis in the list.
//    This only sets the enabled states of the axis modifier buttons.
//
//  Arguments:
//    axis       the index in the list box
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
//  Modifications:
//    Jeremy Meredith, Fri Feb  8 16:12:06 EST 2008
//    Changed axis list to QTreeView to support multiple columns.
//    Added min/max extents columns for each axis, and a button to reset them.
//
//    Jeremy Meredith, Mon Feb 18 16:17:21 EST 2008
//    Don't enable de/up/down buttons if we were created from an array var.
//
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::axisSelected(QTreeWidgetItem*)
{
    int nitems = atts->GetScalarAxisNames().size();
    axisDelButton->setEnabled( nitems > 2 &&
                              axisTree->currentItem()!= NULL);
    axisUpButton->setEnabled(nitems > 0 &&
                           axisTree->currentItem()!= axisTree->topLevelItem(0));

    
    axisDownButton->setEnabled(nitems > 0 &&
                   axisTree->currentItem() != axisTree->topLevelItem(nitems-1));
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::addAxis
//
//  Purpose:
//    add a variable (i.e. axis) to the plot
//
//  Arguments:
//    axisToAdd   the name of the new variable to add
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
//  Modifications:
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::addAxis(const QString &axisToAdd)
{
    atts->InsertAxis(axisToAdd.toStdString());
    //SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::delAxis
//
//  Purpose:
//    removes the currently selected variable (i.e. axis) from the plot
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
//  Modifications:
//    Jeremy Meredith, Fri Feb  8 16:12:06 EST 2008
//    Changed axis list to QTreeView to support multiple columns.
//
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::delAxis()
{
    if (axisTree->currentItem())
    { 
        QString axis = axisTree->currentItem()->text(0);
        atts->DeleteAxis(axis.toStdString(), 2);
        Apply();
    }
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::moveAxisUp
//
//  Purpose:
//    move the currently selected variable (i.e. axis) up in the list
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
//  Modifications:
//    Jeremy Meredith, Fri Feb  8 16:12:06 EST 2008
//    Changed axis list to QTreeView to support multiple columns.
//
//    Jeremy Meredith, Fri Feb 15 13:16:46 EST 2008
//    Renamed orderedAxisNames to scalarAxisNames to distinguish these
//    as names of actual scalars instead of just display names.
//
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::moveAxisUp()
{
    // Find the index of the current item
    int index = GetSelectedAxisIndex();
    // verify something is selected
    // and it wasn't the first itme (can't move first axis up in list)
    if (index <= 0)
        return;

    // must make a local copy
    stringVector axes = atts->GetScalarAxisNames();
    int naxes = axes.size();

    // InsertAxis() will reorder axes already in the list, so we
    // just insert all the changed ones in the new desired order
    atts->InsertAxis(axes[index]);
    atts->InsertAxis(axes[index-1]);
    for (int i=index+1; i<naxes; i++)
        atts->InsertAxis(axes[i]);

    // Done; apply changes
    Apply();
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::moveAxisDown
//
//  Purpose:
//    move the currently selected variable (i.e. axis) down in the list
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
//  Modifications:
//    Jeremy Meredith, Fri Feb  8 16:12:06 EST 2008
//    Changed axis list to QTreeView to support multiple columns.
//
//    Jeremy Meredith, Fri Feb 15 13:16:46 EST 2008
//    Renamed orderedAxisNames to scalarAxisNames to distinguish these
//    as names of actual scalars instead of just display names.
//
//    Cyrus Harrison, Mon Jul 21 08:33:47 PDT 2008
//    Initial Qt4 Port. 
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::moveAxisDown()
{

    int index = GetSelectedAxisIndex();
    if( index < 0)
        return;
    
    // must make a local copy
    stringVector axes = atts->GetScalarAxisNames();
    int naxes = axes.size();
 
    // can't move last axis down in list
    if (index >= axes.size()-1)
        return;

    // InsertAxis() will reorder axes already in the list, so we
    // just insert all the changed ones in the new desired order
    atts->InsertAxis(axes[index+1]);
    atts->InsertAxis(axes[index]);
    for (int i=index+2; i<naxes; i++)
        atts->InsertAxis(axes[i]);

    // Done; apply changes
    Apply();
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::contextGammaSliderChanged
//
//  Purpose:
//    Set the gamma based on the integral gamma slider position
//
//  Arguments:
//    val        the position of the slider (currently [0,100])
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::contextGammaSliderChanged(int val)
{
#if defined(__GNUC__) && ((__GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ < 2) || (__GNUC__ == 3 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ == 0))
    float gamma = pow(10.,double(val/50.)-1);
#else
    float gamma = powf(10.f,float(val/50.)-1);
#endif
    //old: gamma = 0.1 * float(val);

    // round:
    gamma = int(gamma*100+.5)/100.;

    // set the attributes
    atts->SetContextGamma(gamma);

    // Set the value in the line edit.
    QString tmp;
    tmp.sprintf("%.2f", gamma);
    contextGamma->setText(tmp);
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::contextGammaSliderReleased
//
//  Purpose:
//    When the slider is released, update the plot atts as necessary.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::contextGammaSliderReleased()
{
    Apply();
}


// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::contextNumPartitionsSliderChanged
//
//  Purpose:
//    Set the number of partitions based on the integral slider position
//
//  Arguments:
//    val        the position of the slider (currently [1,10])
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::contextNumPartitionsSliderChanged(int val)
{
    int nparts = 1<<val;
    atts->SetContextNumPartitions(nparts);

    // Set the value in the line edit.
    QString tmp;
    tmp.sprintf("%d", nparts);
    contextNumPartitions->setText(tmp);
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::contextNumPartitionsSliderReleased
//
//  Purpose:
//    When the slider is released, update the plot atts as necessary.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::contextNumPartitionsSliderReleased()
{
    Apply();
}


// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::linesOnlyIfExtentsToggled
//
//  Purpose:
//    Executed when the toggle button for restricting line drawing
//    to the case when extents have limited the focus is toggled.
//
//  Arguments:
//    val        the new state
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 21, 2007
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::linesOnlyIfExtentsToggled(bool val)
{
    atts->SetDrawLinesOnlyIfExtentsOn(val);
    Apply();
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::unifyAxisExtentsToggled
//
//  Purpose:
//    Executed when the toggle button for unifying data extents across
//    all processors is toggled.
//
//  Arguments:
//    val        the new state
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  8, 2008
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::unifyAxisExtentsToggled(bool val)
{
    atts->SetUnifyAxisExtents(val);
    Apply();
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::GetSelectedAxisIndex
//
//  Purpose:
//    Helper that obtains the index of the currently selected axis, or
//    -1 if no axis is selected. 
//
//
//  Programmer:  Cyrus Harrison
//  Creation:    Mon Jul 21 09:12:28 PDT 2008
//
// ****************************************************************************

int
QvisParallelCoordinatesPlotWindow::GetSelectedAxisIndex()
{
    int nitems = axisTree->topLevelItemCount();
    for(int i =0;i<nitems;i++)
    {
        if(axisTree->currentItem() == axisTree->topLevelItem(i))
            return i;
    }
    return -1;
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::forceIndividualLineFocusToggled
//
//  Purpose:
//    Executed when the toggle button for forcing focus to be draw
//    using individual data points is toggled.
//
//  Arguments:
//    val        the new state
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 27, 2008
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::forceIndividualLineFocusToggled(bool val)
{
    atts->SetForceFullDataFocus(val);
    Apply();
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::linesNumPartitionsSliderChanged
//
//  Purpose:
//    Set the number of partitions based on the integral slider position
//
//  Arguments:
//    val        the position of the slider (currently [1,7])
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2008
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::linesNumPartitionsSliderChanged(int val)
{
    // 1 maps to 2^6, i.e. 64, 7 maps to 2^12, i.e. 4096
    int nparts = 1<<(val+5);
    atts->SetLinesNumPartitions(nparts);

    // Set the value in the line edit.
    QString tmp;
    tmp.sprintf("%d", nparts);
    linesNumPartitions->setText(tmp);
}

// ****************************************************************************
//  Method:  QvisParallelCoordinatesPlotWindow::linesNumPartitionsSliderReleased
//
//  Purpose:
//    When the slider is released, update the plot atts as necessary.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2008
//
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::linesNumPartitionsSliderReleased()
{
    Apply();
}


