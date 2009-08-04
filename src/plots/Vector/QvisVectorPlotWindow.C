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

#include <QvisVectorPlotWindow.h>
#include <QLayout> 
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>

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
    VectorAttributes *_vecAtts, const QString &caption, const QString &shortName,
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
//   Cyrus Harrison, Wed Aug 27 08:54:49 PDT 2008
//   Made sure a button groups have parents, so we don't need to explicitly
//   delete.
//
// ****************************************************************************

QvisVectorPlotWindow::~QvisVectorPlotWindow()
{
    vectorAtts = 0;
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
//   Jeremy Meredith, Tue Jul  8 15:15:18 EDT 2008
//   Added ability to limit vectors to come from original cell only
//   (useful for material-selected vector plots).
//
//   Jeremy Meredith, Tue Jul  8 16:56:25 EDT 2008
//   Changed the phrasing for the "limit to original" toggle.
//
//   Brad Whitlock, Tue Jul 29 10:38:34 PDT 2008
//   Qt 4.
//
//   Dave Pugmire, Thu Oct 30 08:40:26 EDT 2008
//   Switch the order of the min/max.
//
// ****************************************************************************

void
QvisVectorPlotWindow::CreateWindowContents()
{
    QRadioButton *rb;

    QTabWidget *topTab = new QTabWidget(central);
    topLayout->addWidget(topTab);

    QGridLayout *gLayout = new QGridLayout(0);
    topLayout->addLayout(gLayout);
    gLayout->setSpacing(10);

    //
    // Create the reduce-related widgets.
    //
    reduceGroup = new QWidget(central);
    topTab->addTab(reduceGroup, tr("Reduce by"));

    QVBoxLayout *rgTopLayout = new QVBoxLayout(reduceGroup);
    QGridLayout *rgLayout = new QGridLayout(0);
    rgTopLayout->addLayout(rgLayout);
    rgTopLayout->addStretch(10);
    rgLayout->setMargin(0);
    rgLayout->setSpacing(10);
    rgLayout->setColumnStretch(1, 10);

    // Create the reduce button group.
    reduceButtonGroup = new QButtonGroup(reduceGroup);
    connect(reduceButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(reduceMethodChanged(int)));
    rb= new QRadioButton(tr("N vectors"), reduceGroup);
    rb->setChecked(true);
    reduceButtonGroup->addButton(rb, 0);
    rgLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton(tr("Stride"), reduceGroup);
    reduceButtonGroup->addButton(rb, 1);
    rgLayout->addWidget(rb, 1, 0);

    // Add the N vectors line edit.
    nVectorsLineEdit = new QLineEdit(reduceGroup);
    connect(nVectorsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processNVectorsText()));
    rgLayout->addWidget(nVectorsLineEdit, 0, 1);

    // Add the stride line edit.
    strideLineEdit = new QLineEdit(reduceGroup);
    connect(strideLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processStrideText()));
    rgLayout->addWidget(strideLineEdit, 1, 1);

    // Add the toggle to limit to one vector per original cell/node
    limitToOrigToggle = new QCheckBox(tr("Only show vectors on original nodes/cells"),
                                      reduceGroup);
    connect(limitToOrigToggle, SIGNAL(toggled(bool)),
            this, SLOT(limitToOrigToggled(bool)));
    rgLayout->addWidget(limitToOrigToggle, 2, 0, 1, 2);

    //
    // Create the style-related widgets
    //
    styleGroup = new QWidget(central);
    topTab->addTab(styleGroup, tr("Style"));

    QVBoxLayout *styleTopLayout = new QVBoxLayout(styleGroup);
    QGridLayout *styleLayout = new QGridLayout(0);
    styleTopLayout->addLayout(styleLayout);
    styleTopLayout->addStretch(10);
    styleLayout->setMargin(0);
    styleLayout->setColumnStretch(1, 10);

    // Create the line stem method radio buttons
    lineStemButtonGroup = new QButtonGroup(styleGroup);
    connect(lineStemButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(lineStemMethodChanged(int)));
    rb = new QRadioButton(tr("Line"), styleGroup);
    lineStemButtonGroup->addButton(rb, 0);
    styleLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton(tr("Cylinder"), styleGroup);
    lineStemButtonGroup->addButton(rb, 1);
    styleLayout->addWidget(rb, 2, 0);

    // Create the lineStyle widget.
    lineStyle = new QvisLineStyleWidget(0, styleGroup);
    styleLayout->addWidget(lineStyle, 0, 2);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    lineStyleLabel = new QLabel(tr("Style"), styleGroup);
    lineStyleLabel->setBuddy(lineStyle);
    styleLayout->addWidget(lineStyleLabel, 0, 1);

    // Create the lineWidth widget.
    lineWidth = new QvisLineWidthWidget(0, styleGroup);
    styleLayout->addWidget(lineWidth, 1, 2);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    lineWidthLabel = new QLabel(tr("Width"), styleGroup);
    lineWidthLabel->setBuddy(lineWidth);
    styleLayout->addWidget(lineWidthLabel, 1, 1);

    // Add the stem width edit.
    stemWidthEdit = new QLineEdit(styleGroup);
    connect(stemWidthEdit, SIGNAL(returnPressed()),
            this, SLOT(processStemWidthText()));
    styleLayout->addWidget(stemWidthEdit, 2, 2);
    stemWidthLabel = new QLabel(tr("Width"), styleGroup);
    stemWidthLabel->setBuddy(stemWidthEdit);
    styleLayout->addWidget(stemWidthLabel, 2, 1);

    // Add the "draw head" toggle button.
    drawHeadToggle = new QCheckBox(tr("Draw head"), styleGroup);
    connect(drawHeadToggle, SIGNAL(clicked()),
            this, SLOT(drawHeadToggled()));
    styleLayout->addWidget(drawHeadToggle, 4, 0);

    // Add the head size edit.
    headSizeLineEdit = new QLineEdit(styleGroup);
    connect(headSizeLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processHeadSizeText()));
    styleLayout->addWidget(headSizeLineEdit, 4, 2);
    QLabel *headSizeLabel = new QLabel(tr("Size"), styleGroup);
    headSizeLabel->setBuddy(headSizeLineEdit);
    styleLayout->addWidget(headSizeLabel, 4, 1);

    // Create the high quality toggle
    highQualityToggle = new QCheckBox(tr("High quality geometry"), styleGroup);
    styleLayout->addWidget(highQualityToggle, 6, 0, 1, 3);
    connect(highQualityToggle, SIGNAL(toggled(bool)),
            this, SLOT(highQualityToggled(bool)));

    //
    // Create the radio buttons to choose the glyph origin
    //
    
    QWidget *originBox = new QWidget(styleGroup);
    originButtonGroup = new QButtonGroup(originBox);
    QHBoxLayout *originLayout = new QHBoxLayout(originBox);
    originLayout->setMargin(0);
    originLayout->setSpacing(10);
    QLabel *vectorOriginLabel = new QLabel(tr("Vector origin"), originBox);
    connect(originButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(originTypeChanged(int)));
    originLayout->addWidget(vectorOriginLabel);
    rb = new QRadioButton(tr("Head"), originBox);
    originButtonGroup->addButton(rb,0);
    originLayout->addWidget(rb);
    rb = new QRadioButton(tr("Middle"), originBox);
    originButtonGroup->addButton(rb,1);
    originLayout->addWidget(rb);
    rb = new QRadioButton(tr("Tail"), originBox);
    originButtonGroup->addButton(rb,2);
    originLayout->addWidget(rb);
    styleLayout->addWidget(originBox, 7, 0, 1, 3);


    //
    // Create the color-related widgets.
    //
    colorGroup = new QWidget(central);
    topTab->addTab(colorGroup, tr("Color"));

    QVBoxLayout *colorTopLayout = new QVBoxLayout(colorGroup);
    QGridLayout *cgLayout = new QGridLayout(0);
    colorTopLayout->addLayout(cgLayout);
    colorTopLayout->addStretch(10);
    cgLayout->setMargin(0);
    cgLayout->setSpacing(10);
    cgLayout->setColumnStretch(1, 10);

    // Add the vector color label.
    colorButtonGroup = new QButtonGroup(colorGroup);
    connect(colorButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(colorModeChanged(int)));
    rb = new QRadioButton(tr("Magnitude"), colorGroup);
    colorButtonGroup->addButton(rb, 0);
    cgLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton(tr("Constant"), colorGroup);
    rb->setChecked(true);
    colorButtonGroup->addButton(rb, 1);
    cgLayout->addWidget(rb, 2, 0);

    // Create the color-by-magnitude button.
    colorTableButton = new QvisColorTableButton(colorGroup);
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    cgLayout->addWidget(colorTableButton, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);

    // Create the vector color button.
    vectorColor = new QvisColorButton(colorGroup);
    vectorColor->setButtonColor(QColor(255, 0, 0));
    connect(vectorColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(vectorColorChanged(const QColor &)));
    cgLayout->addWidget(vectorColor, 2, 1, Qt::AlignLeft | Qt::AlignVCenter);

    //
    // Create the Limits stuff
    //
    limitsGroup = new QWidget(colorGroup);
    cgLayout->addWidget(limitsGroup, 1, 0, 1, 3);
    QGridLayout *limitsLayout = new QGridLayout(limitsGroup);
    limitsLayout->setMargin(0);
    limitsLayout->setSpacing(10);
    cgLayout->setColumnStretch(1, 10);

    limitsSelect = new QComboBox(limitsGroup);
    limitsSelect->addItem(tr("Use Original Data"));
    limitsSelect->addItem(tr("Use Current Plot"));
    connect(limitsSelect, SIGNAL(activated(int)),
            this, SLOT(limitsSelectChanged(int))); 
    QLabel *limitsLabel = new QLabel(tr("Limits"), limitsGroup);
    limitsLabel->setBuddy(limitsSelect);
    limitsLayout->addWidget(limitsLabel, 0, 0);
    limitsLayout->addWidget(limitsSelect, 0, 1, 1, 2);

    // Create the max toggle and line edit
    maxToggle = new QCheckBox(tr("Maximum"), limitsGroup);
    limitsLayout->addWidget(maxToggle, 1, 1);
    connect(maxToggle, SIGNAL(toggled(bool)),
            this, SLOT(maxToggled(bool)));
    maxLineEdit = new QLineEdit(limitsGroup);
    connect(maxLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMaxLimitText())); 
    limitsLayout->addWidget(maxLineEdit, 1, 2);

    // Create the min toggle and line edit
    minToggle = new QCheckBox(tr("Minimum"), limitsGroup);
    limitsLayout->addWidget(minToggle, 2, 1);
    connect(minToggle, SIGNAL(toggled(bool)),
            this, SLOT(minToggled(bool)));
    minLineEdit = new QLineEdit(limitsGroup);
    connect(minLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMinLimitText())); 
    limitsLayout->addWidget(minLineEdit, 2, 2);

    //
    // Create the scale-related widgets.
    //
    scaleGroup = new QWidget(central);
    topTab->addTab(scaleGroup, tr("Scale"));

    QVBoxLayout *sgTopLayout = new QVBoxLayout(scaleGroup);
    QGridLayout *sgLayout = new QGridLayout(0);
    sgTopLayout->addLayout(sgLayout);
    sgTopLayout->addStretch(10);
    sgLayout->setMargin(0);
    sgLayout->setSpacing(10);
    sgLayout->setColumnStretch(1, 10);

    // Add the scale line edit.
    scaleLineEdit = new QLineEdit(scaleGroup);
    connect(scaleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processScaleText()));
    sgLayout->addWidget(scaleLineEdit, 0, 1);
    QLabel *scaleLabel = new QLabel(tr("Scale"), scaleGroup);
    scaleLabel->setBuddy(scaleLineEdit);
    sgLayout->addWidget(scaleLabel, 0, 0, Qt::AlignRight | Qt::AlignVCenter);

    // Add the scale by magnitude toggle button.
    scaleByMagnitudeToggle = new QCheckBox(tr("Scale by magnitude"), scaleGroup);
    connect(scaleByMagnitudeToggle, SIGNAL(clicked()), this, SLOT(scaleByMagnitudeToggled()));
    sgLayout->addWidget(scaleByMagnitudeToggle, 1, 0, 1, 2);

    // Add the auto scale toggle button.
    autoScaleToggle = new QCheckBox(tr("Auto scale"), scaleGroup);
    connect(autoScaleToggle, SIGNAL(clicked()), this, SLOT(autoScaleToggled()));
    sgLayout->addWidget(autoScaleToggle, 2, 0, 1, 2);


    //
    // Add the toggle buttons
    //

    // Add the legend toggle button.
    legendToggle = new QCheckBox(tr("Legend"), central);
    connect(legendToggle, SIGNAL(clicked()), this, SLOT(legendToggled()));
    gLayout->addWidget(legendToggle, 4, 0, 1, 2);
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
//   Jeremy Meredith, Tue Jul  8 15:15:18 EDT 2008
//   Added ability to limit vectors to come from original cell only
//   (useful for material-selected vector plots).
//
//   Brad Whitlock, Tue Jul 29 11:07:34 PDT 2008
//   Qt 4.
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
          case VectorAttributes::ID_useStride:
            reduceButtonGroup->blockSignals(true);
            reduceButtonGroup->button(vectorAtts->GetUseStride()?1:0)->setChecked(true);
            reduceButtonGroup->blockSignals(false);

            nVectorsLineEdit->setEnabled(!vectorAtts->GetUseStride());
            strideLineEdit->setEnabled(vectorAtts->GetUseStride());
            break;
          case VectorAttributes::ID_stride:
            temp.sprintf("%d", vectorAtts->GetStride());
            strideLineEdit->setText(temp);
            break;
          case VectorAttributes::ID_nVectors:
            temp.sprintf("%d", vectorAtts->GetNVectors());
            nVectorsLineEdit->setText(temp);
            break;
          case VectorAttributes::ID_lineStyle:
            lineStyle->blockSignals(true);
            lineStyle->SetLineStyle(vectorAtts->GetLineStyle());
            lineStyle->blockSignals(false);
            break;
          case VectorAttributes::ID_lineWidth:
            lineWidth->blockSignals(true);
            lineWidth->SetLineWidth(vectorAtts->GetLineWidth());
            lineWidth->blockSignals(false);
            break;
          case VectorAttributes::ID_scale:
            temp.setNum(vectorAtts->GetScale());
            scaleLineEdit->setText(temp);
            break;
          case VectorAttributes::ID_scaleByMagnitude:
            scaleByMagnitudeToggle->blockSignals(true);
            scaleByMagnitudeToggle->setChecked(vectorAtts->GetScaleByMagnitude());
            scaleByMagnitudeToggle->blockSignals(false);
            break;
          case VectorAttributes::ID_autoScale:
            autoScaleToggle->blockSignals(true);
            autoScaleToggle->setChecked(vectorAtts->GetAutoScale());
            autoScaleToggle->blockSignals(false);
            break;
          case VectorAttributes::ID_headSize:
            temp.setNum(vectorAtts->GetHeadSize());
            headSizeLineEdit->setText(temp);
            break;
          case VectorAttributes::ID_headOn:
            drawHeadToggle->blockSignals(true);
            drawHeadToggle->setChecked(vectorAtts->GetHeadOn());
            drawHeadToggle->blockSignals(false);
            break;
          case VectorAttributes::ID_colorByMag:
            colorButtonGroup->blockSignals(true);
            colorButtonGroup->button(vectorAtts->GetColorByMag() ? 0 : 1)->setChecked(true);
            colorButtonGroup->blockSignals(false);
            limitsGroup->setEnabled(vectorAtts->GetColorByMag());
            break;
          case VectorAttributes::ID_useLegend:
            legendToggle->blockSignals(true);
            legendToggle->setChecked(vectorAtts->GetUseLegend());
            legendToggle->blockSignals(false);
            break;
          case VectorAttributes::ID_vectorColor:
            { // new scope
            QColor temp(vectorAtts->GetVectorColor().Red(),
                        vectorAtts->GetVectorColor().Green(),
                        vectorAtts->GetVectorColor().Blue());
            vectorColor->blockSignals(true);
            vectorColor->setButtonColor(temp);
            vectorColor->blockSignals(false);
            }
          case VectorAttributes::ID_colorTableName:
            colorTableButton->setColorTable(vectorAtts->GetColorTableName().c_str());
            break;
          case VectorAttributes::ID_vectorOrigin:
            originButtonGroup->blockSignals(true);
            switch (vectorAtts->GetVectorOrigin())
            {
              case VectorAttributes::Head:
                originButtonGroup->button(0)->setChecked(true);
                break;
              case VectorAttributes::Middle:
                originButtonGroup->button(1)->setChecked(true);
                break;
              case VectorAttributes::Tail:
                originButtonGroup->button(2)->setChecked(true);
                break;
            }
            originButtonGroup->blockSignals(false);
          break;
          case VectorAttributes::ID_minFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(minToggle, SIGNAL(toggled(bool)),
                       this, SLOT(minToggled(bool)));
            minToggle->setChecked(vectorAtts->GetMinFlag());
            minLineEdit->setEnabled(vectorAtts->GetMinFlag());
            connect(minToggle, SIGNAL(toggled(bool)),
                    this, SLOT(minToggled(bool)));
            break;
          case VectorAttributes::ID_maxFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(maxToggle, SIGNAL(toggled(bool)),
                       this, SLOT(maxToggled(bool)));
            maxToggle->setChecked(vectorAtts->GetMaxFlag());
            maxLineEdit->setEnabled(vectorAtts->GetMaxFlag());
            connect(maxToggle, SIGNAL(toggled(bool)),
                    this, SLOT(maxToggled(bool)));
           break;
          case VectorAttributes::ID_limitsMode:
            limitsSelect->blockSignals(true);
            limitsSelect->setCurrentIndex(vectorAtts->GetLimitsMode());
            limitsSelect->blockSignals(false);
            break;
          case VectorAttributes::ID_min:
            temp.setNum(vectorAtts->GetMin());
            minLineEdit->setText(temp);
            break;
          case VectorAttributes::ID_max:
            temp.setNum(vectorAtts->GetMax());
            maxLineEdit->setText(temp);
            break;
          case VectorAttributes::ID_lineStem:
            lineStemButtonGroup->blockSignals(true);
            lineStemButtonGroup->button(vectorAtts->GetLineStem()?0:1)->setChecked(true);
            lineStemButtonGroup->blockSignals(false);
            lineWidth->setEnabled(vectorAtts->GetLineStem());
            lineStyle->setEnabled(vectorAtts->GetLineStem());
            lineWidthLabel->setEnabled(vectorAtts->GetLineStem());
            lineStyleLabel->setEnabled(vectorAtts->GetLineStem());
            stemWidthEdit->setEnabled(!vectorAtts->GetLineStem());
            stemWidthLabel->setEnabled(!vectorAtts->GetLineStem());
            break;
          case VectorAttributes::ID_highQuality:
            highQualityToggle->blockSignals(true);
            highQualityToggle->setChecked(vectorAtts->GetHighQuality());
            highQualityToggle->blockSignals(false);
            break;
          case VectorAttributes::ID_stemWidth:
            temp.setNum(vectorAtts->GetStemWidth());
            stemWidthEdit->setText(temp);
            break;
          case VectorAttributes::ID_origOnly:
            limitToOrigToggle->blockSignals(true);
            limitToOrigToggle->setChecked(vectorAtts->GetOrigOnly());
            limitToOrigToggle->blockSignals(false);
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
        double val;
        if(LineEditGetDouble(scaleLineEdit, val))
            vectorAtts->SetScale(val);
        else
        {
            ResettingError(tr("scale value"),
                DoubleToQString(vectorAtts->GetScale()));
            vectorAtts->SetScale(vectorAtts->GetScale());
        }
    }

    // Do the head size value.
    if(which_widget == 1 || doAll)
    {
        double val;
        if(LineEditGetDouble(headSizeLineEdit, val))
            vectorAtts->SetHeadSize(val);
        else
        {
            ResettingError(tr("head size"),
                DoubleToQString(vectorAtts->GetHeadSize()));
            vectorAtts->SetHeadSize(vectorAtts->GetHeadSize());
        }
    }

    // Do the N vectors value.
    if(which_widget == 2 || doAll)
    {
        int val;
        if(LineEditGetInt(nVectorsLineEdit, val))
            vectorAtts->SetNVectors(val);
        else
        {
            ResettingError(tr("number of vectors"),
                IntToQString(vectorAtts->GetNVectors()));
            vectorAtts->SetNVectors(vectorAtts->GetNVectors());
        }
    }

    // Do the stride value.
    if(which_widget == 3 || doAll)
    {
        int val;
        if(LineEditGetInt(strideLineEdit, val))
            vectorAtts->SetStride(val);
        else
        {
            ResettingError(tr("stride"), 
                IntToQString(vectorAtts->GetStride()));
            vectorAtts->SetStride(vectorAtts->GetStride());
        }
    }
        // Do the minimum value.
    if(which_widget == 4 || doAll)
    {
        double val;
        if(LineEditGetDouble(minLineEdit, val))
            vectorAtts->SetMin(val);
        else
        {
            ResettingError(tr("minimum value"),
                DoubleToQString(vectorAtts->GetMin()));
            vectorAtts->SetMin(vectorAtts->GetMin());
        }
    }

    // Do the maximum value
    if(which_widget == 5 || doAll)
    {
        double val;
        if(LineEditGetDouble(maxLineEdit, val))
            vectorAtts->SetMax(val);
        else
        {
            ResettingError(tr("maximum value"),
                DoubleToQString(vectorAtts->GetMax()));
            vectorAtts->SetMax(vectorAtts->GetMax());
        }
    }

    // Do the stem width value.
    if(which_widget == 22 || doAll)
    {
        double val;
        if((okay = LineEditGetDouble(stemWidthEdit, val)) == true)
        {
            if (val >=0 && val <= 0.5)
                vectorAtts->SetStemWidth(val);
            else
                okay = false;
        }

        if(!okay)
        {
            ResettingError(tr("stem width"),
                DoubleToQString(vectorAtts->GetStemWidth()));
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
//   Set the enabled state for the limitsGroup based on ColorByMag.
//   
// ****************************************************************************

void
QvisVectorPlotWindow::colorModeChanged(int index)
{
    vectorAtts->SetColorByMag(index == 0);
    limitsGroup->setEnabled(vectorAtts->GetColorByMag());
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
    vectorAtts->SetColorTableName(ctName.toStdString());
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

// ****************************************************************************
// Method: QvisVectorPlotWindow::limitToOrigToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's limit to original node/cell toggle button.
//
// Programmer: Jeremy Meredith
// Creation:   July  8, 2008
//
// Modifications:
//   
// ****************************************************************************
void
QvisVectorPlotWindow::limitToOrigToggled(bool val)
{
    vectorAtts->SetOrigOnly(val);
    Apply();
}

