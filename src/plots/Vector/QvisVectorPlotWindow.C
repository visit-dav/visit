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

#include <QvisVectorPlotWindow.h>
#include <qlayout.h> 
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtabwidget.h>

#include <VectorAttributes.h>
#include <ViewerProxy.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisColorButton.h>
#include <QvisColorTableButton.h>

// ****************************************************************************
// Method: QvisVectorPlotWindow::QvisVectorPlotWindow
//
// Purpose: 
//   Constructor for the QvisVectorPlotWindow class.
//
// Arguments:
//   type      : An identifier used to identify the plot type in the viewer.
//   _vecAtts  : The vector attributes that the window observes.
//   caption   : The caption displayed in the window decorations.
//   shortName : The name used in the notepad.
//   notepad   : The notepad area where the window posts itself.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 23:45:22 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 15:04:45 PST 2002
//   Initialized widgets with no parents.
//
// ****************************************************************************

QvisVectorPlotWindow::QvisVectorPlotWindow(const int type,
    VectorAttributes *_vecAtts, const char *caption, const char *shortName,
    QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(_vecAtts, caption, shortName, notepad)
{
    plotType = type;
    vectorAtts = _vecAtts;

    // Initialize parentless widgets.
    reduceButtonGroup = 0;
    colorButtonGroup = 0;
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::~QvisVectorPlotWindow
//
// Purpose: 
//   Destructor for the QvisVectorPlotWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 23:50:47 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 15:04:58 PST 2002
//   Deleted widgets with no parents.
//
// ****************************************************************************

QvisVectorPlotWindow::~QvisVectorPlotWindow()
{
    vectorAtts = 0;

    // Delete widgets with no parents.
    delete reduceButtonGroup;
    delete colorButtonGroup;
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets that are in the window and sets
//   up their signals/slots.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 23:50:55 PST 2001
//
// Modifications:
//   Disabled the lineStyleLabel so that users will know that lineStyle
//   is disabled, until such a time as we get the vtk version that has
//   line stippling available.
//
//   Brad Whitlock, Sat Jun 16 19:08:44 PST 2001
//   Added color table controls.
//
//   Kathleen Bonnell, Thu Jun 21 16:33:54 PDT 2001
//   Enabled lineStyleLabel, lineStyle.
//
//   Brad Whitlock, Fri Aug 29 11:37:35 PDT 2003
//   Grouped like items into group boxes.
//
//   Jeremy Meredith, Fri Nov 21 12:29:29 PST 2003
//   Added vector origin type radio buttons.
//
//   Eric Brugger, Tue Nov 23 10:18:29 PST 2004
//   Added scaleByMagnitude and autoScale.
//
//   Kathleen Bonnell, Wed Dec 22 16:42:35 PST 2004
//   Added widgets for min/max and limits selection. 
//
//   Jeremy Meredith, Mon Mar 19 16:24:08 EDT 2007
//   Added controls for lineStem, stemWidth, and highQuality.
//   Reorganized the window a bit.  Allowed disabling use of tabs
//   by simply removing the TABS #define below -- this puts it back
//   to the tall version of the window with groupboxes for sections.
//
//   Brad Whitlock, Wed Apr 23 12:09:56 PDT 2008
//   Added tr()'s
//
// ****************************************************************************
#define TABS
void
QvisVectorPlotWindow::CreateWindowContents()
{
    QRadioButton *rb;

#ifdef TABS
    QTabWidget *topTab = new QTabWidget(central, "topTab");
    topLayout->addWidget(topTab);
#endif
    QGridLayout *gLayout = new QGridLayout(topLayout, 6, 2);
    gLayout->setSpacing(10);

    //
    // Create the reduce-related widgets.
    //
    reduceGroupBox = new QGroupBox(central, "reduceGroupBox");
#ifdef TABS
    topTab->addTab(reduceGroupBox, tr("Reduce by"));
    reduceGroupBox->setFrameStyle(QFrame::NoFrame);
#else
    gLayout->addMultiCellWidget(reduceGroupBox, 0,0, 0,1);
    reduceGroupBox->setTitle(tr("Reduce by"));
#endif
    QVBoxLayout *rgTopLayout = new QVBoxLayout(reduceGroupBox);
    rgTopLayout->setMargin(10);
    rgTopLayout->addSpacing(15);
    QGridLayout *rgLayout = new QGridLayout(rgTopLayout, 3, 2);
    rgLayout->setSpacing(10);
    rgLayout->setColStretch(1, 10);

    // Create the reduce button group.
    reduceButtonGroup = new QButtonGroup(0, "reduceButtonGroup");
    connect(reduceButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(reduceMethodChanged(int)));
    rb= new QRadioButton(tr("N vectors"), reduceGroupBox);
    rb->setChecked(true);
    reduceButtonGroup->insert(rb, 0);
    rgLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton(tr("Stride"), reduceGroupBox);
    reduceButtonGroup->insert(rb, 1);
    rgLayout->addWidget(rb, 1, 0);

    // Add the N vectors line edit.
    nVectorsLineEdit = new QLineEdit(reduceGroupBox, "nVectorsLineEdit");
    connect(nVectorsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processNVectorsText()));
    rgLayout->addWidget(nVectorsLineEdit, 0, 1);

    // Add the stride line edit.
    strideLineEdit = new QLineEdit(reduceGroupBox, "strideLineEdit");
    connect(strideLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processStrideText()));
    rgLayout->addWidget(strideLineEdit, 1, 1);


    //
    // Create the style-related widgets
    //
    styleGroupBox = new QGroupBox(central, "styleGroupBox");
#ifdef TABS
    topTab->addTab(styleGroupBox, tr("Style"));
    styleGroupBox->setFrameStyle(QFrame::NoFrame);
#else
    gLayout->addMultiCellWidget(styleGroupBox, 1,1, 0,1);
    styleGroupBox->setTitle(tr("Vector style"));
#endif
    QVBoxLayout *styleTopLayout = new QVBoxLayout(styleGroupBox);
    styleTopLayout->setMargin(10);
    styleTopLayout->addSpacing(15);
    QGridLayout *styleLayout = new QGridLayout(styleTopLayout, 8, 4);
    styleLayout->setSpacing(10);
    styleLayout->setColSpacing(0, 20);
    styleLayout->setColStretch(1, 10);

    // Create the line stem method radio buttons
    lineStemButtonGroup = new QButtonGroup(0, "lineStemButtonGroup");
    connect(lineStemButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(lineStemMethodChanged(int)));
    rb = new QRadioButton(tr("Line:"), styleGroupBox);
    lineStemButtonGroup->insert(rb, 0);
    styleLayout->addMultiCellWidget(rb, 0,0, 0,0);
    rb = new QRadioButton(tr("Cylinder:"), styleGroupBox);
    lineStemButtonGroup->insert(rb, 1);
    styleLayout->addMultiCellWidget(rb, 2,2, 0,0);

    // Create the lineStyle widget.
    lineStyle = new QvisLineStyleWidget(0, styleGroupBox, "lineStyle");
    styleLayout->addWidget(lineStyle, 0, 2);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    lineStyleLabel = new QLabel(lineStyle, tr("Style"),
                                        styleGroupBox, "lineStyleLabel");
    styleLayout->addWidget(lineStyleLabel, 0, 1);

    // Create the lineWidth widget.
    lineWidth = new QvisLineWidthWidget(0, styleGroupBox, "lineWidth");
    styleLayout->addWidget(lineWidth, 1, 2);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(lineWidth, tr("Width"),
                                        styleGroupBox, "lineWidthLabel");
    styleLayout->addWidget(lineWidthLabel, 1, 1);

    // Add the stem width edit.
    stemWidthEdit = new QLineEdit(styleGroupBox, "stemWidthEdit");
    connect(stemWidthEdit, SIGNAL(returnPressed()),
            this, SLOT(processStemWidthText()));
    styleLayout->addMultiCellWidget(stemWidthEdit, 2,2, 2,2);
    stemWidthLabel = new QLabel(stemWidthEdit, tr("Width"),
                                styleGroupBox, "stemWidthLabel");
    styleLayout->addWidget(stemWidthLabel, 2, 1);

    // Add the "draw head" toggle button.
    drawHeadToggle = new QCheckBox(tr("Draw head:"), styleGroupBox,
                                   "drawHeadToggle");
    connect(drawHeadToggle, SIGNAL(clicked()),
            this, SLOT(drawHeadToggled()));
    styleLayout->addMultiCellWidget(drawHeadToggle, 4, 4, 0, 0);

    // Add the head size edit.
    headSizeLineEdit = new QLineEdit(styleGroupBox, "headSizeLineEdit");
    connect(headSizeLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processHeadSizeText()));
    styleLayout->addMultiCellWidget(headSizeLineEdit, 4,4, 2,2);
    QLabel *headSizeLabel = new QLabel(headSizeLineEdit, tr("Size"),
                                       styleGroupBox, "headSizeLabel");
    styleLayout->addMultiCellWidget(headSizeLabel, 4,4, 1,1);

    // Create the high quality toggle
    highQualityToggle = new QCheckBox(tr("High quality geometry"), styleGroupBox,
                                   "highQualityToggle");
    styleLayout->addMultiCellWidget(highQualityToggle, 6,6, 0,2);
    connect(highQualityToggle, SIGNAL(toggled(bool)),
            this, SLOT(highQualityToggled(bool)));

    //
    // Create the radio buttons to choose the glyph origin
    //
    originButtonGroup = new QButtonGroup(0, "originButtonGroup");
    QHBox *originBox = new QHBox(styleGroupBox, "originBox");
    originBox->setSpacing(10);
    new QLabel(tr("Vector origin"), originBox, "originLabel");
    connect(originButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(originTypeChanged(int)));
    rb = new QRadioButton(tr("Head"), originBox);
    originButtonGroup->insert(rb,0);
    rb = new QRadioButton(tr("Middle"), originBox);
    originButtonGroup->insert(rb,1);
    rb = new QRadioButton(tr("Tail"), originBox);
    originButtonGroup->insert(rb,2);
    styleLayout->addMultiCellWidget(originBox, 7, 7, 0, 2);


    //
    // Create the color-related widgets.
    //
    colorGroupBox = new QGroupBox(central, "colorGroupBox");
#ifdef TABS
    topTab->addTab(colorGroupBox, tr("Color"));
    colorGroupBox->setFrameStyle(QFrame::NoFrame);
#else
    gLayout->addMultiCellWidget(colorGroupBox, 2, 2, 0, 1);
    colorGroupBox->setTitle(tr("Vector color"));
#endif
    QVBoxLayout *cgTopLayout = new QVBoxLayout(colorGroupBox);
    cgTopLayout->setMargin(10);
    cgTopLayout->addSpacing(15);
    QGridLayout *cgLayout = new QGridLayout(cgTopLayout, 4, 3);
    cgLayout->setSpacing(10);
    cgLayout->setColStretch(1, 10);

    // Add the vector color label.
    colorButtonGroup = new QButtonGroup(0, "colorModeButtons");
    connect(colorButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(colorModeChanged(int)));
    rb = new QRadioButton(tr("Magnitude"), colorGroupBox, "Magnitude");
    colorButtonGroup->insert(rb, 0);
    cgLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton(tr("Constant"), colorGroupBox, "constant");
    rb->setChecked(true);
    colorButtonGroup->insert(rb, 1);
    cgLayout->addWidget(rb, 2, 0);

    // Create the color-by-magnitude button.
    colorTableButton = new QvisColorTableButton(colorGroupBox, "colorTableButton");
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    cgLayout->addWidget(colorTableButton, 0, 1, AlignLeft | AlignVCenter);

    // Create the vector color button.
    vectorColor = new QvisColorButton(colorGroupBox, "vectorColorButton");
    vectorColor->setButtonColor(QColor(255, 0, 0));
    connect(vectorColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(vectorColorChanged(const QColor &)));
    cgLayout->addWidget(vectorColor, 2, 1, AlignLeft | AlignVCenter);

    //
    // Create the Limits stuff
    //
    limitsGroupBox = new QGroupBox(colorGroupBox, "limitsGroupBox");
    limitsGroupBox->setFrameStyle(QFrame::NoFrame);
    cgLayout->addMultiCellWidget(limitsGroupBox, 1, 1, 0, 2);
    QGridLayout *limitsLayout = new QGridLayout(limitsGroupBox, 3, 3);
    limitsLayout->setSpacing(10);
    cgLayout->setColStretch(1, 10);

    limitsSelect = new QComboBox(false, limitsGroupBox, "limitsSelect");
    limitsSelect->insertItem(tr("Use Original Data"));
    limitsSelect->insertItem(tr("Use Current Plot"));
    connect(limitsSelect, SIGNAL(activated(int)),
            this, SLOT(limitsSelectChanged(int))); 
    QLabel *limitsLabel = new QLabel(limitsSelect, tr("Limits"), 
                                     limitsGroupBox, "limitsLabel");
    limitsLayout->addWidget(limitsLabel, 0, 0);
    limitsLayout->addMultiCellWidget(limitsSelect, 0, 0, 1, 2, AlignLeft);

    // Create the min toggle and line edit
    minToggle = new QCheckBox(tr("Min"), limitsGroupBox, "minToggle");
    limitsLayout->addWidget(minToggle, 1, 1);
    connect(minToggle, SIGNAL(toggled(bool)),
            this, SLOT(minToggled(bool)));
    minLineEdit = new QLineEdit(limitsGroupBox, "minLineEdit");
    connect(minLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMinLimitText())); 
    limitsLayout->addWidget(minLineEdit, 1, 2);

    // Create the max toggle and line edit
    maxToggle = new QCheckBox(tr("Max"), limitsGroupBox, "maxToggle");
    limitsLayout->addWidget(maxToggle, 2, 1);
    connect(maxToggle, SIGNAL(toggled(bool)),
            this, SLOT(maxToggled(bool)));
    maxLineEdit = new QLineEdit(limitsGroupBox, "maxLineEdit");
    connect(maxLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMaxLimitText())); 
    limitsLayout->addWidget(maxLineEdit, 2, 2);

    //
    // Create the scale-related widgets.
    //
    scaleGroupBox = new QGroupBox(central, "scaleGroupBox");
#ifdef TABS
    topTab->addTab(scaleGroupBox, tr("Scale"));
    scaleGroupBox->setFrameStyle(QFrame::NoFrame);
#else
    gLayout->addMultiCellWidget(scaleGroupBox, 3, 3, 0, 1);
    scaleGroupBox->setTitle(tr("Vector scale"));
#endif
    QVBoxLayout *sgTopLayout = new QVBoxLayout(scaleGroupBox);
    sgTopLayout->setMargin(10);
    sgTopLayout->addSpacing(15);
    QGridLayout *sgLayout = new QGridLayout(sgTopLayout, 4, 2);
    sgLayout->setSpacing(10);
    sgLayout->setColStretch(1, 10);

    // Add the scale line edit.
    scaleLineEdit = new QLineEdit(scaleGroupBox, "scaleLineEdit");
    connect(scaleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processScaleText()));
    sgLayout->addWidget(scaleLineEdit, 0, 1);
    QLabel *scaleLabel = new QLabel(scaleLineEdit, tr("Scale"), scaleGroupBox, "scaleLabel");
    sgLayout->addWidget(scaleLabel, 0, 0, AlignRight | AlignVCenter);

    // Add the scale by magnitude toggle button.
    scaleByMagnitudeToggle = new QCheckBox(tr("Scale by magnitude"), scaleGroupBox, "scaleByMagnitudeToggle");
    connect(scaleByMagnitudeToggle, SIGNAL(clicked()), this, SLOT(scaleByMagnitudeToggled()));
    sgLayout->addMultiCellWidget(scaleByMagnitudeToggle, 1, 1, 0, 1);

    // Add the auto scale toggle button.
    autoScaleToggle = new QCheckBox(tr("Auto scale"), scaleGroupBox, "autoScaleToggle");
    connect(autoScaleToggle, SIGNAL(clicked()), this, SLOT(autoScaleToggled()));
    sgLayout->addMultiCellWidget(autoScaleToggle, 2, 2, 0, 1);


    //
    // Add the toggle buttons
    //

    // Add the legend toggle button.
    legendToggle = new QCheckBox(tr("Legend"), central, "legendToggle");
    connect(legendToggle, SIGNAL(clicked()), this, SLOT(legendToggled()));
    gLayout->addMultiCellWidget(legendToggle, 4, 4, 0, 1);
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the window's subject is changed. The
//   subject tells this window what attributes changed and we put the
//   new values into those widgets.
//
// Arguments:
//   doAll : If this flag is true, update all the widgets regardless
//           of whether or not they are selected.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 23:51:26 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Mon Mar 26 18:17:53 PST 2001
//   Disabled lineStyle until we have vtk version
//   in which line stippling is available.
//
//   Brad Whitlock, Sat Jun 16 19:21:03 PST 2001
//   I added code to handle color table widgets.
//
//   Kathleen Bonnell, Thu Jun 21 16:33:54 PDT 2001
//   Enabled lineStyle.
//
//   Brad Whitlock, Fri Feb 15 11:49:23 PDT 2002
//   Fixed format strings.
//
//   Jeremy Meredith, Fri Nov 21 12:29:16 PST 2003
//   Added vector origin type radio buttons.
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Eric Brugger, Tue Nov 23 10:18:29 PST 2004
//   Added scaleByMagnitude and autoScale.
//
//   Kathleen Bonnell, Wed Dec 22 16:42:35 PST 2004
//   Update widgets for min/max and limits selection. 
//
//   Jeremy Meredith, Mon Mar 19 16:24:08 EDT 2007
//   Added controls for lineStem, stemWidth, and highQuality.
//
// ****************************************************************************

void
QvisVectorPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;

    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < vectorAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!vectorAtts->IsSelected(i))
                continue;
        }

        switch(i)
        {
        case 0: // useStride
            reduceButtonGroup->blockSignals(true);
            reduceButtonGroup->setButton(vectorAtts->GetUseStride()?1:0);
            reduceButtonGroup->blockSignals(false);

            nVectorsLineEdit->setEnabled(!vectorAtts->GetUseStride());
            strideLineEdit->setEnabled(vectorAtts->GetUseStride());
            break;
        case 1: // stride
            temp.sprintf("%d", vectorAtts->GetStride());
            strideLineEdit->setText(temp);
            break;
        case 2: // nVectors
            temp.sprintf("%d", vectorAtts->GetNVectors());
            nVectorsLineEdit->setText(temp);
            break;
        case 3: // lineStyle
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(vectorAtts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
        case 4: // lineWidth
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(vectorAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
        case 5: // scale
            temp.setNum(vectorAtts->GetScale());
            scaleLineEdit->setText(temp);
            break;
        case 6: // scaleByMagnitude
            scaleByMagnitudeToggle->blockSignals(true);
            scaleByMagnitudeToggle->setChecked(vectorAtts->GetScaleByMagnitude());
            scaleByMagnitudeToggle->blockSignals(false);
            break;
        case 7: // autoScale
            autoScaleToggle->blockSignals(true);
            autoScaleToggle->setChecked(vectorAtts->GetAutoScale());
            autoScaleToggle->blockSignals(false);
            break;
        case 8: // headSize
            temp.setNum(vectorAtts->GetHeadSize());
            headSizeLineEdit->setText(temp);
            break;
        case 9: // headOn
            drawHeadToggle->blockSignals(true);
            drawHeadToggle->setChecked(vectorAtts->GetHeadOn());
            drawHeadToggle->blockSignals(false);
            break;
        case 10: // colorByMag
            colorButtonGroup->blockSignals(true);
            colorButtonGroup->setButton(vectorAtts->GetColorByMag() ? 0 : 1);
            colorButtonGroup->blockSignals(false);
            limitsGroupBox->setEnabled(vectorAtts->GetColorByMag());
            break;
        case 11: // useLegend
            legendToggle->blockSignals(true);
            legendToggle->setChecked(vectorAtts->GetUseLegend());
            legendToggle->blockSignals(false);
            break;
        case 12: // vectorColor
            { // new scope
            QColor temp(vectorAtts->GetVectorColor().Red(),
                        vectorAtts->GetVectorColor().Green(),
                        vectorAtts->GetVectorColor().Blue());
            vectorColor->blockSignals(true);
            vectorColor->setButtonColor(temp);
            vectorColor->blockSignals(false);
            }
        case 13: // colorTableName
            colorTableButton->setColorTable(vectorAtts->GetColorTableName().c_str());
            break;
        case 14: // vectorOrigin
            originButtonGroup->blockSignals(true);
            switch (vectorAtts->GetVectorOrigin())
            {
              case VectorAttributes::Head:
                originButtonGroup->setButton(0);
                break;
              case VectorAttributes::Middle:
                originButtonGroup->setButton(1);
                break;
              case VectorAttributes::Tail:
                originButtonGroup->setButton(2);
                break;
            }
            originButtonGroup->blockSignals(false);
          break;
        case 15: // minFlag
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(minToggle, SIGNAL(toggled(bool)),
                       this, SLOT(minToggled(bool)));
            minToggle->setChecked(vectorAtts->GetMinFlag());
            minLineEdit->setEnabled(vectorAtts->GetMinFlag());
            connect(minToggle, SIGNAL(toggled(bool)),
                    this, SLOT(minToggled(bool)));
            break;
        case 16: // maxFlag
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(maxToggle, SIGNAL(toggled(bool)),
                       this, SLOT(maxToggled(bool)));
            maxToggle->setChecked(vectorAtts->GetMaxFlag());
            maxLineEdit->setEnabled(vectorAtts->GetMaxFlag());
            connect(maxToggle, SIGNAL(toggled(bool)),
                    this, SLOT(maxToggled(bool)));
           break;
        case 17: // limitsMode
            limitsSelect->blockSignals(true);
            limitsSelect->setCurrentItem(vectorAtts->GetLimitsMode());
            limitsSelect->blockSignals(false);
            break;
        case 18: // min
            temp.setNum(vectorAtts->GetMin());
            minLineEdit->setText(temp);
            break;
        case 19: // max
            temp.setNum(vectorAtts->GetMax());
            maxLineEdit->setText(temp);
            break;
        case 20: // lineStem
            lineStemButtonGroup->blockSignals(true);
            lineStemButtonGroup->setButton(vectorAtts->GetLineStem()?0:1);
            lineStemButtonGroup->blockSignals(false);
            lineWidth->setEnabled(vectorAtts->GetLineStem());
            lineStyle->setEnabled(vectorAtts->GetLineStem());
            lineWidthLabel->setEnabled(vectorAtts->GetLineStem());
            lineStyleLabel->setEnabled(vectorAtts->GetLineStem());
            stemWidthEdit->setEnabled(!vectorAtts->GetLineStem());
            stemWidthLabel->setEnabled(!vectorAtts->GetLineStem());
            break;
        case 21: // highQuality
            highQualityToggle->blockSignals(true);
            highQualityToggle->setChecked(vectorAtts->GetHighQuality());
            highQualityToggle->blockSignals(false);
            break;
        case 22: // stemWidth
            temp.setNum(vectorAtts->GetStemWidth());
            stemWidthEdit->setText(temp);
            break;
        }
    } // end for
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values for one or all of the lineEdit widgets.
//
// Arguments:
//   which_widget : The number of the widget to update. If -1 is passed,
//                  the routine gets the current values for all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 23:51:58 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 11:49:34 PDT 2002
//   Fixed format strings.
//
//   Kathleen Bonnell, Wed Dec 22 16:42:35 PST 2004
//   Get values for min and max.
//
//   Jeremy Meredith, Mon Mar 19 16:24:08 EDT 2007
//   Added stemWidth.
//
//   Brad Whitlock, Wed Apr 23 12:11:47 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisVectorPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the scale value.
    if(which_widget == 0 || doAll)
    {
        temp = scaleLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                vectorAtts->SetScale(val);
        }

        if(!okay)
        {
            msg = tr("The scale value was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(vectorAtts->GetScale());
            Message(msg);
            vectorAtts->SetScale(vectorAtts->GetScale());
        }
    }

    // Do the head size value.
    if(which_widget == 1 || doAll)
    {
        temp = headSizeLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                vectorAtts->SetHeadSize(val);
        }

        if(!okay)
        {
            msg = tr("The head size was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(vectorAtts->GetHeadSize());
            Message(msg);
            vectorAtts->SetHeadSize(vectorAtts->GetHeadSize());
        }
    }

    // Do the N vectors value.
    if(which_widget == 2 || doAll)
    {
        temp = nVectorsLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if(okay)
                vectorAtts->SetNVectors(val);
        }

        if(!okay)
        {
            msg = tr("The head size was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(vectorAtts->GetNVectors());
            Message(msg);
            vectorAtts->SetNVectors(vectorAtts->GetNVectors());
        }
    }

    // Do the stride value.
    if(which_widget == 3 || doAll)
    {
        temp = strideLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            if(okay)
                vectorAtts->SetStride(val);
        }

        if(!okay)
        {
            msg = tr("The stride was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(vectorAtts->GetStride());
            Message(msg);
            vectorAtts->SetStride(vectorAtts->GetStride());
        }
    }
        // Do the minimum value.
    if(which_widget == 4 || doAll)
    {
        temp = minLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                vectorAtts->SetMin(val);
        }

        if(!okay)
        {
            msg = tr("The minimum value was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(vectorAtts->GetMin());
            Message(msg);
            vectorAtts->SetMin(vectorAtts->GetMin());
        }
    }

    // Do the maximum value
    if(which_widget == 5 || doAll)
    {
        temp = maxLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if(okay)
                vectorAtts->SetMax(val);
        }

        if(!okay)
        {
            msg = tr("The maximum value was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(vectorAtts->GetMax());
            Message(msg);
            vectorAtts->SetMax(vectorAtts->GetMax());
        }
    }

    // Do the stem width value.
    if(which_widget == 22 || doAll)
    {
        temp = stemWidthEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            double val = temp.toDouble(&okay);
            if (val >=0 && val <= 0.5)
                vectorAtts->SetStemWidth(val);
            else
                okay = false;
        }

        if(!okay)
        {
            msg = tr("The stem width was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(vectorAtts->GetStemWidth());
            Message(msg);
            vectorAtts->SetStemWidth(vectorAtts->GetStemWidth());
        }
    }


}

// ****************************************************************************
// Method: QvisVectorPlotWindow::Apply
//
// Purpose: 
//   This method applies the vector attributes and optionally tells the viewer
//   to apply them to the plot.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            vector attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 23:52:51 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current aslice attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        vectorAtts->Notify();

        // Tell the viewer to set the vector attributes.
        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        vectorAtts->Notify();
}

//
// Qt Slot functions...
//

void
QvisVectorPlotWindow::apply()
{
    Apply(true);
}

void
QvisVectorPlotWindow::makeDefault()
{
    // Tell the viewer to set the default vector attributes.
    GetCurrentValues(-1);
    vectorAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}

void
QvisVectorPlotWindow::reset()
{
    // Tell the viewer to reset the aslice attributes to the last
    // applied values.
    GetViewerMethods()->ResetPlotOptions(plotType);
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::lineStyleChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   lineStyle widget.
//
// Arguments:
//   newStyle : The new line style.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:20:44 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::lineStyleChanged(int newStyle)
{
    vectorAtts->SetLineStyle(newStyle);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::lineWidthChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   lineWidth widget.
//
// Arguments:
//   newWidth : The new line width.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:20:44 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::lineWidthChanged(int newWidth)
{
    vectorAtts->SetLineWidth(newWidth);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::vectorColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   vector color.
//
// Arguments:
//   color : The new vector color.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:21:58 PDT 2001
//
// Modifications:
//   Brad Whitlock, Sat Jun 16 19:19:30 PST 2001
//   Added code to disable coloration by vector magnitude.
//
// ****************************************************************************

void
QvisVectorPlotWindow::vectorColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    vectorAtts->SetVectorColor(temp);
    vectorAtts->SetColorByMag(false);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::processScaleText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   scale line edit.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:22:33 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::processScaleText()
{
    GetCurrentValues(0);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::scaleByMagnitudeToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's scale by magnitude toggle button.
//
// Programmer: Eric Brugger
// Creation:   Tue Nov 23 10:18:29 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::scaleByMagnitudeToggled()
{
    vectorAtts->SetScaleByMagnitude(!vectorAtts->GetScaleByMagnitude());
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::autoScaleToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's auto scale toggle button.
//
// Programmer: Eric Brugger
// Creation:   Tue Nov 23 10:18:29 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::autoScaleToggled()
{
    vectorAtts->SetAutoScale(!vectorAtts->GetAutoScale());
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::processHeadSizeText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   head size line edit.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:22:33 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::processHeadSizeText()
{
    GetCurrentValues(1);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::reduceMethodChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   method used to reduce the number of vectors.
//
// Arguments:
//   index : The reduction method.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:24:08 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::reduceMethodChanged(int index)
{
    vectorAtts->SetUseStride(index != 0);
    Apply();   
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::processNVectorsText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   N vectors line edit.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:22:33 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::processNVectorsText()
{
    GetCurrentValues(2);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::processStrideText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   stride line edit.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:22:33 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::processStrideText()
{
    GetCurrentValues(3);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::legendToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's legend toggle button.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:24:55 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::legendToggled()
{
    vectorAtts->SetUseLegend(!vectorAtts->GetUseLegend());
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::drawHeadToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's "drawhead" toggle button.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:25:29 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::drawHeadToggled()
{
    vectorAtts->SetHeadOn(!vectorAtts->GetHeadOn());
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::colorByMagnitudeToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's "color by magnitude" toggle button.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:26:11 PDT 2001
//
// Modifications:
//   Kathleen Bonnell, Wed Dec 22 16:42:35 PST 2004
//   Set the enabled state for the limitsGroupBox based on ColorByMag.
//   
// ****************************************************************************

void
QvisVectorPlotWindow::colorModeChanged(int index)
{
    vectorAtts->SetColorByMag(index == 0);
    limitsGroupBox->setEnabled(vectorAtts->GetColorByMag());
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::colorTableClicked
//
// Purpose: 
//   This is a Qt slot function that sets the desired color table name into
//   the vector plot attributes.
//
// Arguments:
//   useDefault : If this is true, we want to use the default color table.
//   ctName     : The name of the color table to use if we're not going to
//                use the default.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 18:30:51 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisVectorPlotWindow::colorTableClicked(bool useDefault,
    const QString &ctName)
{
    vectorAtts->SetColorByMag(true);
    vectorAtts->SetColorTableName(ctName.latin1());
    Apply();
}

// ****************************************************************************
//  Method:  
//
//  Purpose:
//    Qt slot function to change the state of the vector origin type on
//    response to a radio button click.
//
//  Arguments:
//    index      the index of the radio button
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2003
//
// ****************************************************************************
void
QvisVectorPlotWindow::originTypeChanged(int index)
{
    if (index==0)
    {
        vectorAtts->SetVectorOrigin(VectorAttributes::Head);
    }
    else if (index==1)
    {
        vectorAtts->SetVectorOrigin(VectorAttributes::Middle);
    }
    else
    {
        vectorAtts->SetVectorOrigin(VectorAttributes::Tail);
    }
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::limitsSelectChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   window's limits selection combo box. 
//
// Programmer: Kathleen Bonnell 
// Creation:   December 22, 2004 
//
// Modifications:
//   
// ****************************************************************************
void
QvisVectorPlotWindow::limitsSelectChanged(int mode)
{
    // Only do it if it changed.
    if(mode != vectorAtts->GetLimitsMode())
    {
        vectorAtts->SetLimitsMode(VectorAttributes::LimitsMode(mode));
        Apply();
    }
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::processMinLimitText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   window's min line edit text. 
//
// Programmer: Kathleen Bonnell 
// Creation:   December 22, 2004 
//
// Modifications:
//   
// ****************************************************************************
void
QvisVectorPlotWindow::processMinLimitText()
{
    GetCurrentValues(4);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::processMaxLimitText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   window's max line edit text. 
//
// Programmer: Kathleen Bonnell 
// Creation:   December 22, 2004 
//
// Modifications:
//   
// ****************************************************************************
void
QvisVectorPlotWindow::processMaxLimitText()
{
    GetCurrentValues(5);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::minToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's min toggle button.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 22, 2004 
//
// Modifications:
//   
// ****************************************************************************
void
QvisVectorPlotWindow::minToggled(bool val)
{
    vectorAtts->SetMinFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::maxToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's max toggle button.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 22, 2004 
//
// Modifications:
//   
// ****************************************************************************
void
QvisVectorPlotWindow::maxToggled(bool val)
{
    vectorAtts->SetMaxFlag(val);
    Apply();
}


// ****************************************************************************
// Method: QvisVectorPlotWindow::lineStemMethodChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   window's line stem method.
//
// Programmer: Jeremy Meredith
// Creation:   March 19, 2007
//
// Modifications:
//   
// ****************************************************************************
void
QvisVectorPlotWindow::lineStemMethodChanged(int val)
{
    vectorAtts->SetLineStem(val==0 ? true : false);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::highQualityToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's high quality button.
//
// Programmer: Jeremy Meredith
// Creation:   March 19, 2007
//
// Modifications:
//   
// ****************************************************************************
void
QvisVectorPlotWindow::highQualityToggled(bool val)
{
    vectorAtts->SetHighQuality(val);
    Apply();
}

// ****************************************************************************
// Method: QvisVectorPlotWindow::processStemWidthText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   window's stem width edit text. 
//
// Programmer: Jeremy Meredith
// Creation:   March 19, 2007
//
// Modifications:
//   
// ****************************************************************************
void
QvisVectorPlotWindow::processStemWidthText()
{
    GetCurrentValues(22);
    Apply();
}

