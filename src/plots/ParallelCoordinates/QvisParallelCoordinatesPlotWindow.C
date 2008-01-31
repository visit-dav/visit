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

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qradiobutton.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qvbox.h>
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
                         const char *caption,
                         const char *shortName,
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
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::CreateWindowContents()
{
    //
    // Axes to plot
    //
    axisGroup = new QGroupBox("Axes",
                              central, "axisGroup");
    topLayout->addWidget(axisGroup);
    QVBoxLayout *axisSpacingLayout = new QVBoxLayout(axisGroup);
    axisSpacingLayout->setMargin(10);
    axisSpacingLayout->addSpacing(20);

    QGridLayout *axisLayout = new QGridLayout(axisSpacingLayout, 4, 2, 5);

    // axes list
    axisList = new QListBox(axisGroup, "axisList");
    axisLayout->addMultiCellWidget(axisList, 0,3, 0,0);
    connect(axisList, SIGNAL(highlighted(int)),
            this, SLOT(axisSelected(int)));

    // axes new/del/up/down buttons
    axisNewButton = new QvisVariableButton(false, true, true,
                                           QvisVariableButton::Scalars,
                                           axisGroup, "axisNewButton");
    axisNewButton->setText("Add axis");
    axisNewButton->setChangeTextOnVariableChange(false);
    axisLayout->addWidget(axisNewButton, 0, 1);
    connect(axisNewButton, SIGNAL(activated(const QString &)),
            this, SLOT(addAxis(const QString &)));

    axisDelButton = new QPushButton("Delete", axisGroup, "axisDelButton");
    axisLayout->addWidget(axisDelButton, 1, 1);
    connect(axisDelButton, SIGNAL(clicked()),
            this, SLOT(delAxis()));

    axisUpButton = new QPushButton("Move up", axisGroup, "axisUpButton");
    axisLayout->addWidget(axisUpButton, 2, 1);
    connect(axisUpButton, SIGNAL(clicked()),
            this, SLOT(moveAxisUp()));

    axisDownButton = new QPushButton("Move down", axisGroup, "axisDownButton");
    axisLayout->addWidget(axisDownButton, 3, 1);
    connect(axisDownButton, SIGNAL(clicked()),
            this, SLOT(moveAxisDown()));

    //
    // Draw lines, and the needed settings
    //
    drawLines = new QGroupBox("Draw individual lines",
                              central, "drawLines");
    drawLines->setCheckable(true);
    connect(drawLines, SIGNAL(toggled(bool)),
            this, SLOT(drawLinesChanged(bool)));
    topLayout->addWidget(drawLines);

    QVBoxLayout *linesSpacingLayout = new QVBoxLayout(drawLines);
    linesSpacingLayout->setMargin(10);
    linesSpacingLayout->addSpacing(20);

    QGridLayout *linesLayout = new QGridLayout(linesSpacingLayout, 2, 2, 5);

    // Lines color
    linesOnlyIfExtents = new QCheckBox(
                                 "... but only when extents have been limited",
                                       drawLines, "linesOnlyIfExtents");
    connect(linesOnlyIfExtents, SIGNAL(toggled(bool)),
            this, SLOT(linesOnlyIfExtentsToggled(bool)));
    linesLayout->addMultiCellWidget(linesOnlyIfExtents, 0,0, 0,1);

    // Lines color
    linesColorLabel = new QLabel("Line color", drawLines, "linesColorLabel");
    linesLayout->addWidget(linesColorLabel,1,0);
    linesColor = new QvisColorButton(drawLines, "linesColor");
    connect(linesColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(linesColorChanged(const QColor&)));
    linesLayout->addWidget(linesColor, 1,1);

    //
    // Draw context, and the needed settings
    //
    drawContext = new QGroupBox("Draw context",
                              central, "drawContext");
    drawContext->setCheckable(true);
    connect(drawContext, SIGNAL(toggled(bool)),
            this, SLOT(drawContextChanged(bool)));
    topLayout->addWidget(drawContext);

    QVBoxLayout *contextSpacingLayout = new QVBoxLayout(drawContext);
    contextSpacingLayout->setMargin(10);
    contextSpacingLayout->addSpacing(20);

    QGridLayout *contextLayout = new QGridLayout(contextSpacingLayout, 3, 3, 5);

    // Contex gamma correction
    contextGammaLabel = new QLabel("Brightness (gamma)",
                                   drawContext, "contextGammaLabel");
    contextLayout->addWidget(contextGammaLabel,0,0);
    contextGamma = new QNarrowLineEdit(drawContext, "contextGamma");
    connect(contextGamma, SIGNAL(returnPressed()),
            this, SLOT(contextGammaProcessText()));
    contextLayout->addWidget(contextGamma, 0,1);
    contextGammaSlider = new QSlider(0,100,5,66,Qt::Horizontal,drawContext,
                                     "contextGammaSlider");
    connect(contextGammaSlider, SIGNAL(valueChanged(int)),
            this, SLOT(contextGammaSliderChanged(int)));
    connect(contextGammaSlider, SIGNAL(sliderReleased()),
            this, SLOT(contextGammaSliderReleased()));
    contextLayout->addWidget(contextGammaSlider, 0,2);

    // Number of partitions
    contextNumPartitionsLabel = new QLabel("Number of partitions", drawContext,
                                           "contextNumPartitionsLabel");
    contextLayout->addWidget(contextNumPartitionsLabel,1,0);
    contextNumPartitions = new QNarrowLineEdit(drawContext,
                                               "contextNumPartitions");
    connect(contextNumPartitions, SIGNAL(returnPressed()),
            this, SLOT(contextNumPartitionsProcessText()));
    contextLayout->addWidget(contextNumPartitions, 1,1);
    contextNumPartitionsSlider = new QSlider(1,10,1,5,Qt::Horizontal,
                                      drawContext,"contextNumPartitionsSlider");
    connect(contextNumPartitionsSlider, SIGNAL(valueChanged(int)),
            this, SLOT(contextNumPartitionsSliderChanged(int)));
    connect(contextNumPartitionsSlider, SIGNAL(sliderReleased()),
            this, SLOT(contextNumPartitionsSliderReleased()));
    contextLayout->addWidget(contextNumPartitionsSlider, 1,2);

    // Context color
    contextColorLabel = new QLabel("Context color",
                                   drawContext, "contextColorLabel");
    contextLayout->addWidget(contextColorLabel,2,0);
    contextColor = new QvisColorButton(drawContext, "contextColor");
    connect(contextColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(contextColorChanged(const QColor&)));
    contextLayout->addWidget(contextColor, 2,1);

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
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;
    double r;

    QString oldAxis = axisList->currentText();

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
        const double         *dptr;
        const float          *fptr;
        const int            *iptr;
        const char           *cptr;
        const unsigned char  *uptr;
        const string         *sptr;
        QColor                tempcolor;
        switch(i)
        {
          case ParallelCoordinatesAttributes::ID_orderedAxisNames:
            axisList->blockSignals(true);
            axisList->clear();
            for (int ax=0; ax<atts->GetOrderedAxisNames().size(); ax++)
            {
                axisList->insertItem(atts->GetOrderedAxisNames()[ax].c_str());
            }
            axisList->blockSignals(false);
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
            sliderpos = QMIN(QMAX(0, sliderpos), 100);
            contextGammaSlider->setValue(sliderpos);
            contextGamma->blockSignals(false);
            contextGammaSlider->blockSignals(false);
            break;
          case ParallelCoordinatesAttributes::ID_contextNumPartitions:
            contextNumPartitions->blockSignals(true);
            contextNumPartitionsSlider->blockSignals(true);
            temp.sprintf("%d", atts->GetContextNumPartitions());
            sliderpos = int(log((float)atts->GetContextNumPartitions())/log(2.f)+.5);
            sliderpos = QMIN(QMAX(1, sliderpos), 10);
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
        }
    }

    // Re-select the previously selected item in case updating this window
    // regenerated the list box contents
    axisList->setCurrentItem(-1);
    for (int i=0; i<axisList->count(); i++)
    {
        if (axisList->text(i) == oldAxis)
        {
            axisList->setCurrentItem(i);
        }
    }

    // Set enabled states
    axisDelButton->setEnabled(axisList->currentItem() >= 0);
    axisUpButton->setEnabled(axisList->currentItem() > 0);
    axisDownButton->setEnabled(axisList->currentItem() < axisList->count()-1);
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
//   
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do contextGamma
    if(which_widget == 18 || doAll)
    {
        temp = contextGamma->displayText().simplifyWhiteSpace();
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
            msg.sprintf("The value of contextGamma was invalid. "
                "Resetting to the last good value of %g.",
                atts->GetContextGamma());
            Message(msg);
            atts->SetContextGamma(atts->GetContextGamma());
        }
    }

    // Do contextNumPartitions
    if(which_widget == 19 || doAll)
    {
        temp = contextNumPartitions->displayText().simplifyWhiteSpace();
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
            msg.sprintf("The value of contextNumPartitions was invalid. "
                "Resetting to the last good value of %d.",
                atts->GetContextNumPartitions());
            Message(msg);
            atts->SetContextNumPartitions(atts->GetContextNumPartitions());
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
QvisParallelCoordinatesPlotWindow::drawContextChanged(bool val)
{
    atts->SetDrawContext(val);
    SetUpdate(false);
    Apply();
}


void
QvisParallelCoordinatesPlotWindow::contextGammaProcessText()
{
    GetCurrentValues(18);
    Apply();
}


void
QvisParallelCoordinatesPlotWindow::contextNumPartitionsProcessText()
{
    GetCurrentValues(19);
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
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::axisSelected(int axis)
{
    axisDelButton->setEnabled(axisList->currentItem() >= 0);
    axisUpButton->setEnabled(axisList->currentItem() > 0);
    axisDownButton->setEnabled(axisList->currentItem() < axisList->count()-1);
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
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::addAxis(const QString &axisToAdd)
{
    atts->InsertAxis(axisToAdd.latin1());
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
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::delAxis()
{
    QString axis = axisList->currentText();
    atts->DeleteAxis(axis.latin1(), 2);
    Apply();
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
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::moveAxisUp()
{
    int index = axisList->currentItem();

    // verify something is selected
    if (index < 0)
        return;

    // must make a local copy
    stringVector axes = atts->GetOrderedAxisNames();
    int naxes = axes.size();

    // can't move first axis up in list
    if (index == 0)
        return;

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
// ****************************************************************************

void
QvisParallelCoordinatesPlotWindow::moveAxisDown()
{
    int index = axisList->currentItem();

    // verify something is selected
    if (index < 0)
        return;

    // must make a local copy
    stringVector axes = atts->GetOrderedAxisNames();
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
