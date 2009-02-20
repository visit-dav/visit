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

#include <QvisColorTableWindow.h>
#include <ColorTableAttributes.h>
#include <QApplication>
#include <QButtonGroup>
#include <QLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QDesktopWidget>

#include <QvisSpectrumBar.h>
#include <QvisColorSelectionWidget.h>
#include <QvisColorGridWidget.h>
#include <ColorControlPoint.h>
#include <ColorControlPointList.h>
#include <DataNode.h>
#include <ViewerProxy.h>


// Defines. Make these part of ColorTableAttributes sometime.
#define DEFAULT_DISCRETE_COLS    6
#define DEFAULT_DISCRETE_ROWS    5
#define DEFAULT_DISCRETE_NCOLORS (DEFAULT_DISCRETE_ROWS * DEFAULT_DISCRETE_COLS)

#define SELECT_FOR_CONTINUOUS 0
#define SELECT_FOR_DISCRETE   1

// ****************************************************************************
// Method: QvisColorTableWindow::QvisColorTableWindow
//
// Purpose: 
//   This is the constructor for the QvisColorTableWindow class.
//
// Arguments:
//   colorAtts : The ColorTableAttributes that the window observes.
//   caption   : The caption that appears in the window's title.
//   shortName : The name that appears in the notepad area when posted.
//   notepad   : The notepad area to which the window posts.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 13:58:08 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Jun 20 13:41:16 PST 2001
//   Added initialization of an observer.
//
//   Brad Whitlock, Wed Nov 20 16:04:01 PST 2002
//   Added initialization of activeCT.
//
//   Brad Whitlock, Wed Feb 26 10:59:45 PDT 2003
//   Initialized colorTableTypeGroup.
//
//   Brad Whitlock, Wed Apr  9 11:59:35 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisColorTableWindow::QvisColorTableWindow(
    ColorTableAttributes *colorAtts_, const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(colorAtts_, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton, false),
    currentColorTable("none"), ctObserver(colorAtts_)
{
    colorAtts = colorAtts_;
    colorCycle = 0;
    sliding = false;
    colorSelect = 0;
    colorTableTypeGroup = 0;
}

// ****************************************************************************
// Method: QvisColorTableWindow::~QvisColorTableWindow
//
// Purpose: 
//   Destructor for the QvisColorTableWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 13:59:14 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 26 10:59:56 PDT 2003
//   Deleted colorTableTypeGroup.
//
// ****************************************************************************

QvisColorTableWindow::~QvisColorTableWindow()
{
    colorAtts = 0;
    delete colorSelect;
}

// ****************************************************************************
// Method: QvisColorTableWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 12:04:27 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Mar 13 16:52:58 PST 2002
//   Added a little spacing.
//
//   Brad Whitlock, Wed Nov 20 15:29:46 PST 2002
//   I added support for discrete color tables.
//
//   Brad Whitlock, Wed Feb 26 10:58:14 PDT 2003
//   I changed the window so that discrete color tables can have an arbitrary
//   number of colors.
//
//   Brad Whitlock, Tue Jul 1 16:38:50 PST 2003
//   I added an export button.
//
//   Jeremy Meredith, Fri Aug 11 17:14:32 EDT 2006
//   Refactoring of color grid widget caused index to lose "color" in names.
//   
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
//   Jeremy Meredith, Wed Dec 31 16:11:50 EST 2008
//   Added show index hints checkbox for discrete color tables.
//
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//
// ****************************************************************************

void
QvisColorTableWindow::CreateWindowContents()
{
    // Create the widgets needed to set the active color tables.
    topLayout->setMargin(2);
    activeGroup = new QGroupBox(central);
    activeGroup->setTitle(tr("Active color table"));
    topLayout->addWidget(activeGroup, 5);

    QVBoxLayout *innerActiveTopLayout = new QVBoxLayout(activeGroup);
    QGridLayout *innerActiveLayout = new QGridLayout();
    innerActiveTopLayout->addLayout(innerActiveLayout);
    innerActiveLayout->setColumnMinimumWidth(1, 10);

    activeContinuous = new QComboBox(activeGroup);
    connect(activeContinuous, SIGNAL(activated(const QString &)),
            this, SLOT(setActiveContinuous(const QString &)));
    innerActiveLayout->addWidget(activeContinuous, 0, 1);
    activeContinuousLabel = new QLabel(tr("Continuous"),activeGroup);
    innerActiveLayout->addWidget(activeContinuousLabel, 0, 0);

    activeDiscrete = new QComboBox(activeGroup);
    connect(activeDiscrete, SIGNAL(activated(const QString &)),
            this, SLOT(setActiveDiscrete(const QString &)));
    innerActiveLayout->addWidget(activeDiscrete, 1, 1);
    activeDiscreteLabel = new QLabel(tr("Discrete"),activeGroup);
    innerActiveLayout->addWidget(activeDiscreteLabel, 1, 0);


    // Create the widget group that contains all of the color table
    // management stuff.
    colorTableWidgetGroup = new QGroupBox(central);
    colorTableWidgetGroup->setTitle(tr("Manager"));
    topLayout->addWidget(colorTableWidgetGroup,5);
    QVBoxLayout *innerColorTableLayout = new QVBoxLayout(colorTableWidgetGroup);
    
    // Create the color management widgets.
    QGridLayout *mgLayout = new QGridLayout();
    innerColorTableLayout->addLayout(mgLayout);

    newButton = new QPushButton(tr("New"), colorTableWidgetGroup);
    connect(newButton, SIGNAL(clicked()), this, SLOT(addColorTable()));
    mgLayout->addWidget(newButton, 0, 0);

    deleteButton = new QPushButton(tr("Delete"), colorTableWidgetGroup);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteColorTable()));
    mgLayout->addWidget(deleteButton, 1, 0);

    exportButton = new QPushButton(tr("Export"), colorTableWidgetGroup);
    connect(exportButton, SIGNAL(clicked()), this, SLOT(exportColorTable()));
    mgLayout->addWidget(exportButton, 2, 0);

    nameListBox = new QListWidget(colorTableWidgetGroup);
    nameListBox->setMinimumHeight(100);
    connect(nameListBox, SIGNAL(currentRowChanged(int)),
            this, SLOT(highlightColorTable(int)));
    mgLayout->addWidget(nameListBox, 0, 1, 3, 1);

    QLabel *colorTableName = new QLabel(tr("Name"), colorTableWidgetGroup);
    mgLayout->addWidget(colorTableName, 3, 0, Qt::AlignRight);
    nameLineEdit = new QLineEdit(colorTableWidgetGroup);
    mgLayout->addWidget(nameLineEdit, 3, 1);

    // Add the group box that will contain the color-related widgets.
    colorWidgetGroup = new QGroupBox(central);
    colorWidgetGroup->setTitle(tr("Editor"));
    topLayout->addWidget(colorWidgetGroup, 100);
    QVBoxLayout *innerColorLayout = new QVBoxLayout(colorWidgetGroup);
    
    // Create controls to set the number of colors in the color table.
    QGridLayout *colorInfoLayout = new QGridLayout();
    innerColorLayout->addLayout(colorInfoLayout);
    colorNumColors = new QSpinBox(colorWidgetGroup);
    colorNumColors->setRange(2,200);
    colorNumColors->setSingleStep(1);
    connect(colorNumColors, SIGNAL(valueChanged(int)),
            this, SLOT(resizeColorTable(int)));
    colorInfoLayout->addWidget(colorNumColors, 0, 1, 1, 2);
    colorInfoLayout->addWidget(new QLabel(tr("Number of colors"),
                                          colorWidgetGroup), 0, 0);

    // Create radio buttons to convert the color table between color table types.
    colorInfoLayout->addWidget(new QLabel(tr("Color table type")), 1, 0);
    colorTableTypeGroup = new QButtonGroup(colorWidgetGroup);
    QRadioButton *rb = new QRadioButton(tr("Continuous"),colorWidgetGroup);
    colorTableTypeGroup->addButton(rb,0);
    colorInfoLayout->addWidget(rb, 1, 1);
    rb = new QRadioButton(tr("Discrete"),colorWidgetGroup);
    colorTableTypeGroup->addButton(rb,1);
    colorInfoLayout->addWidget(rb, 1, 2);
    connect(colorTableTypeGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(setColorTableType(int)));
    

    // Create the buttons that help manipulate the spectrum bar.
    QHBoxLayout *seLayout = new QHBoxLayout();
    innerColorLayout->addLayout(seLayout);
    
    alignPointButton = new QPushButton(tr("Align"), colorWidgetGroup);
    connect(alignPointButton, SIGNAL(clicked()),
            this, SLOT(alignControlPoints()));
    seLayout->addWidget(alignPointButton);
    seLayout->addStretch(10);

    smoothCheckBox = new QCheckBox(tr("Smooth"), colorWidgetGroup);
    smoothCheckBox->setChecked(true);
    connect(smoothCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(smoothToggled(bool)));
    seLayout->addWidget(smoothCheckBox);

    equalCheckBox = new QCheckBox(tr("Equal"), colorWidgetGroup);
    connect(equalCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(equalSpacingToggled(bool)));
    seLayout->addWidget(equalCheckBox);

    // Create the spectrum bar.
    spectrumBar = new QvisSpectrumBar(colorWidgetGroup);
    spectrumBar->setMinimumHeight(100);
    spectrumBar->addControlPoint(QColor(255,0,0),   0.);
    spectrumBar->addControlPoint(QColor(255,255,0), 0.25);
    spectrumBar->addControlPoint(QColor(0,255,0),   0.5);
    spectrumBar->addControlPoint(QColor(0,255,255), 0.75);
    spectrumBar->addControlPoint(QColor(0,0,255),   1.);
    connect(spectrumBar, SIGNAL(controlPointMoved(int,float)),
            this, SLOT(controlPointMoved(int,float)));
    connect(spectrumBar, SIGNAL(selectColor(int, const QPoint &)),
            this, SLOT(chooseContinuousColor(int, const QPoint &)));
    connect(spectrumBar, SIGNAL(activeControlPointChanged(int)),
            this, SLOT(activateContinuousColor(int)));

    innerColorLayout->addWidget(spectrumBar, 100);

    // Create the discrete color table widgets.
    discreteColors = new QvisColorGridWidget(colorWidgetGroup);
    QColor *tmpColors = new QColor[DEFAULT_DISCRETE_NCOLORS];
    for(int i = 0; i < DEFAULT_DISCRETE_NCOLORS; ++i)
    {
        int c = int((float(i)/float(DEFAULT_DISCRETE_NCOLORS - 1)) * 255);
        tmpColors[i] = QColor(c,c,c);
    }
    discreteColors->setPaletteColors(tmpColors, DEFAULT_DISCRETE_NCOLORS, DEFAULT_DISCRETE_COLS);
    discreteColors->setFrame(false);
    discreteColors->setShowIndexHints(false);
    discreteColors->setBoxSize(16);
    discreteColors->setBoxPadding(8);
    discreteColors->setSelectedIndex(0);
    connect(discreteColors, SIGNAL(selectedColor(const QColor &, int)),
            this, SLOT(activateDiscreteColor(const QColor &, int)));
    connect(discreteColors, SIGNAL(activateMenu(const QColor &, int, int, const QPoint &)),
            this, SLOT(chooseDiscreteColor(const QColor &, int, int, const QPoint &)));
    delete [] tmpColors;
    innerColorLayout->addWidget(discreteColors, 100);

    // Add a check box for index hinting
    showIndexHintsCheckBox = new QCheckBox(tr("Show index hints"), colorWidgetGroup);
    showIndexHintsCheckBox->setChecked(false);
    connect(showIndexHintsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showIndexHintsToggled(bool)));
    innerColorLayout->addWidget(showIndexHintsCheckBox);


    // Create the discrete color table sliders, text fields.
    QGridLayout *discreteLayout = new QGridLayout();
    innerColorLayout->addLayout(discreteLayout);
    QString cnames[4];
    cnames[0] = tr("Red");
    cnames[1] = tr("Green");
    cnames[2] = tr("Blue");
    cnames[3] = tr("Alpha");
    for(int j = 0; j < 4; ++j)
    {
        QString n;
        n.sprintf("componentSliders[%d]", j);
        componentSliders[j] = new QSlider(Qt::Horizontal,colorWidgetGroup);
        componentSliders[j]->setRange(0, 255);
        componentSliders[j]->setPageStep(10);
        componentSliders[j]->setValue(0);
        
        discreteLayout->addWidget(componentSliders[j], j, 1);

        n.sprintf("componentLabels[%d]", j);
        componentLabels[j] = new QLabel(cnames[j], colorWidgetGroup);
        discreteLayout->addWidget(componentLabels[j], j, 0);

        n.sprintf("discreteLineEdits[%d]", j);
        componentSpinBoxes[j] = new QSpinBox(colorWidgetGroup);
        componentSpinBoxes[j]->setRange(0,255);
        componentSpinBoxes[j]->setSingleStep(1);


        // Hook up some signals and slots
        if(j == 0)
        {
            connect(componentSliders[j], SIGNAL(valueChanged(int)),
                    this, SLOT(redValueChanged(int)));
            connect(componentSpinBoxes[j], SIGNAL(valueChanged(int)),
                    this, SLOT(redValueChanged(int)));
        }
        else if(j == 1)
        {
            connect(componentSliders[j], SIGNAL(valueChanged(int)),
                    this, SLOT(greenValueChanged(int)));
            connect(componentSpinBoxes[j], SIGNAL(valueChanged(int)),
                    this, SLOT(greenValueChanged(int)));
        }
        else if(j == 2)
        {
            connect(componentSliders[j], SIGNAL(valueChanged(int)),
                    this, SLOT(blueValueChanged(int)));
            connect(componentSpinBoxes[j], SIGNAL(valueChanged(int)),
                    this, SLOT(blueValueChanged(int)));
        }
        else
        {
            connect(componentSliders[j], SIGNAL(valueChanged(int)),
                    this, SLOT(alphaValueChanged(int)));
            connect(componentSpinBoxes[j], SIGNAL(valueChanged(int)),
                    this, SLOT(alphaValueChanged(int)));
        }
        connect(componentSliders[j], SIGNAL(sliderPressed()),
                this, SLOT(sliderPressed()));
        connect(componentSliders[j], SIGNAL(sliderReleased()),
                this, SLOT(sliderReleased()));

        discreteLayout->addWidget(componentSpinBoxes[j], j, 2);
    }
    innerColorLayout->addStretch(5);

    // Create the color selection widget.
    colorSelect = new QvisColorSelectionWidget(NULL,Qt::Popup);
    connect(colorSelect, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(selectedColor(const QColor &)));
}

// ****************************************************************************
// Method: QvisColorTableWindow::CreateNode
//
// Purpose: 
//   Saves the windows settings.
//
// Arguments:
//   parentNode : The node that will contain the settings.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 6 09:18:09 PDT 2006
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisColorTableWindow::CreateNode(DataNode *parentNode)
{
    // Call the base class's method to save the generic window attributes.
    QvisPostableWindowSimpleObserver::CreateNode(parentNode);

    if(saveWindowDefaults && 
       !currentColorTable.isEmpty() &&
       currentColorTable != "none")
    {
        DataNode *node = parentNode->GetNode(windowTitle().toStdString());

        // Save the current color table.
        std::string ct(currentColorTable.toStdString());
        node->AddNode(new DataNode("currentColorTable", ct));
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::SetFromNode
//
// Purpose: 
//   Gets the window's settings.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 6 09:18:41 PDT 2006
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisColorTableWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    DataNode *winNode = parentNode->GetNode(windowTitle().toStdString());
    if(winNode == 0)
        return;

    // Get the active tab and show it.
    DataNode *node;
    if((node = winNode->GetNode("currentColorTable")) != 0)
    {
        currentColorTable = QString(node->AsString().c_str());
    }

    // Call the base class's function.
    QvisPostableWindowSimpleObserver::SetFromNode(parentNode, borders);
}

// ****************************************************************************
// Method: QvisColorTableWindow::UpdateWindow
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
// Creation:   Mon Jun 11 12:07:55 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 16:11:25 PST 2002
//   I added support for discrete color tables.
//
//   Brad Whitlock, Mon Mar 6 10:18:18 PDT 2006
//   I changed the code so it only uses the first colortable name as a last
//   resort if the active color table is set to something invalid.
//
//   Brad Whitlock, Fri Dec 14 16:59:58 PST 2007
//   Made it use ids.
//
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
//   Jeremy Meredith, Wed Dec 31 16:01:40 EST 2008
//   Avoid use of temporaries that was referencing freed memory.
//
// ****************************************************************************

void
QvisColorTableWindow::UpdateWindow(bool doAll)
{
    bool updateNames = false;
    bool updateColorPoints = false;

    //
    // If our active color table, for some reason, does not appear in the
    // list of color tables then we should choose a new colortable. Note that
    // if we've not set the color table yet, it will be "none" and it will get
    // set here, if possible.
    //
    int nct = 3;
    bool invalidCt = true;
    QString ctNames[4];
    ctNames[0] = currentColorTable;
    ctNames[1] = colorAtts->GetActiveContinuous().c_str();
    ctNames[2] = colorAtts->GetActiveDiscrete().c_str();
    if(colorAtts->GetNames().size() > 0)
    {
        ctNames[3] = colorAtts->GetNames()[0].c_str();
        ++nct;
    }
    for(int c = 0; c < nct && invalidCt; ++c)
    {
        if(colorAtts->GetColorTableIndex(ctNames[c].toStdString()) != -1)
        {
            currentColorTable = ctNames[c];
            invalidCt = false;
        }
        else
            updateColorPoints = true;
    }
    if(invalidCt)
    {
        currentColorTable = "none";
        updateColorPoints = true;
    }

    //
    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    //
    for(int i = 0; i < colorAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!colorAtts->IsSelected(i))
                continue;
        }

        switch(i)
        {
        case ColorTableAttributes::ID_names:
            updateNames = true;
            break;
        case ColorTableAttributes::ID_colorTables:
            updateColorPoints = true;
            break;
        case ColorTableAttributes::ID_activeContinuous:
            updateNames = true;
            break;
        case ColorTableAttributes::ID_activeDiscrete:
            updateNames = true;
            break;
        }
    }

    //
    // Update the editor area.
    //
    if(updateColorPoints)
        UpdateEditor();

    //
    // Update the names.
    //
    if(updateNames)
        UpdateNames();

    // If there are no color tables, disable the color editor widgets.
    colorWidgetGroup->setEnabled(colorAtts->GetNumColorTables() > 0);
}

// ****************************************************************************
// Method: QvisColorTableWindow::UpdateEditor
//
// Purpose: 
//   Updates the editor area of the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 20 16:33:02 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Feb 26 11:04:52 PDT 2003
//   I changed the widgets for discrete color tables.
//
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
//   Jeremy Meredith, Wed Dec 31 16:11:50 EST 2008
//   Added show index hints checkbox for discrete color tables.
//
// ****************************************************************************

void
QvisColorTableWindow::UpdateEditor()
{
    const ColorControlPointList *ccpl = GetActiveColorControlPoints();
    if(ccpl)
    {
        if(ccpl->GetDiscreteFlag())
        {
            UpdateDiscreteSettings();

            smoothCheckBox->hide();
            equalCheckBox->hide();
            spectrumBar->hide();
            alignPointButton->hide();

            discreteColors->show();
            showIndexHintsCheckBox->show();
        }
        else
        {
            UpdateColorControlPoints();

            smoothCheckBox->show();
            equalCheckBox->show();
            spectrumBar->show();
            alignPointButton->show();

            discreteColors->hide();
            showIndexHintsCheckBox->hide();
        }

        colorTableTypeGroup->blockSignals(true);
        colorTableTypeGroup->button(ccpl->GetDiscreteFlag()?1:0)->setChecked(true);
        colorTableTypeGroup->blockSignals(false);

        colorNumColors->blockSignals(true);
        colorNumColors->setValue(ccpl->GetNumControlPoints());
        colorNumColors->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::UpdateNames
//
// Purpose: 
//   This method takes the list of names in the color table attributes and
//   puts it into the color table name list box.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 19:58:19 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 16:41:22 PST 2002
//   I changed the code so it uses currentColorTable.
//
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisColorTableWindow::UpdateNames()
{
    nameListBox->blockSignals(true);
    activeContinuous->blockSignals(true);
    activeDiscrete->blockSignals(true);

    // Clear out the existing names.
    nameListBox->clear();
    activeContinuous->clear();
    activeDiscrete->clear();

    // Put all of the color table names into the list box.
    int i;
    QString activeContinuousName(colorAtts->GetActiveContinuous().c_str());
    QString activeDiscreteName(colorAtts->GetActiveDiscrete().c_str());
    for(i = 0; i < colorAtts->GetNumColorTables(); ++i)
    {
        QString item(colorAtts->GetNames()[i].c_str());
        nameListBox->addItem(item);
        activeContinuous->addItem(item);
        activeDiscrete->addItem(item);
        if(item == activeContinuousName)
            activeContinuous->setCurrentIndex(i);
        if(item == activeDiscreteName)
            activeDiscrete->setCurrentIndex(i);
    }

    // Select the active color table.
    int index = colorAtts->GetColorTableIndex(currentColorTable.toStdString());
    if(index >= 0)
    {
        nameListBox->setCurrentRow(index);
        // Set the text of the active color table into the name line edit.
        nameLineEdit->setText(QString(colorAtts->GetNames()[index].c_str()));
    }

    nameListBox->blockSignals(false);
    activeContinuous->blockSignals(false);
    activeDiscrete->blockSignals(false);

    // Set the enabled state of the delete button.
    deleteButton->setEnabled(colorAtts->GetNumColorTables() > 1);
}

// ****************************************************************************
// Method: QvisColorTableWindow::GetActiveColorControlPoints
//
// Purpose: 
//   Returns a const pointer to the color control points of our active
//   color table.
//
// Returns:    A const pointer to the color control points of our active
//             color table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:19:43 PST 2002
//
// Modifications:
//   
// ****************************************************************************

const ColorControlPointList *
QvisColorTableWindow::GetActiveColorControlPoints() const
{
    return colorAtts->GetColorControlPoints(currentColorTable.toStdString());
}

// ****************************************************************************
// Method: QvisColorTableWindow::GetActiveColorControlPoints
//
// Purpose: 
//   Returns a pointer to the color control points of our active
//   color table.
//
// Returns:    A pointer to the color control points of our active
//             color table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:19:43 PST 2002
//
// Modifications:
//   
// ****************************************************************************

ColorControlPointList *
QvisColorTableWindow::GetActiveColorControlPoints()
{
    return (ColorControlPointList *)colorAtts->GetColorControlPoints(currentColorTable.toStdString());
}

// ****************************************************************************
// Method: QvisColorTableWindow::UpdateColorControlPoints
//
// Purpose: 
//   This method is called when the color control points must be updated.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 23:35:25 PST 2001
//
// Modifications:
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//   
// ****************************************************************************

void
QvisColorTableWindow::UpdateColorControlPoints()
{
    // Get a pointer to the active color table's control points.
    const ColorControlPointList *ccpl = GetActiveColorControlPoints();

    if(ccpl)
    {
        const ColorControlPointList &cpts = *ccpl;
        int   i;

        spectrumBar->setSuppressUpdates(true);
        spectrumBar->blockSignals(true);

        if(spectrumBar->numControlPoints() == cpts.GetNumControlPoints())
        {
            // Set the control points' colors and positions
            for(i = 0; i < spectrumBar->numControlPoints(); ++i)
            {
                QColor ctmp((int)cpts[i].GetColors()[0],
                            (int)cpts[i].GetColors()[1],
                            (int)cpts[i].GetColors()[2],
                            (int)cpts[i].GetColors()[3]);
                spectrumBar->setControlPointColor(i, ctmp);
                spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
            }
        }
        else if(spectrumBar->numControlPoints() < cpts.GetNumControlPoints())
        {
            // Set the control points' colors and positions
            for(i = 0; i < spectrumBar->numControlPoints(); ++i)
            {
                QColor ctmp((int)cpts[i].GetColors()[0],
                            (int)cpts[i].GetColors()[1],
                            (int)cpts[i].GetColors()[2],
                            (int)cpts[i].GetColors()[3]);
                spectrumBar->setControlPointColor(i, ctmp);
                spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
            }
    
            // We need to add control points.
            for(i = spectrumBar->numControlPoints(); i < cpts.GetNumControlPoints(); ++i)
            {
                QColor ctmp((int)cpts[i].GetColors()[0],
                            (int)cpts[i].GetColors()[1],
                            (int)cpts[i].GetColors()[2],
                            (int)cpts[i].GetColors()[3]);
                spectrumBar->addControlPoint(ctmp, cpts[i].GetPosition());
            }
        }
        else
        {
            // We need to remove control points.
            for(i = spectrumBar->numControlPoints()-1; i >= cpts.GetNumControlPoints(); --i)
                spectrumBar->removeControlPoint();

            // Set the control points' colors and positions
            for(i = 0; i < spectrumBar->numControlPoints(); ++i)
            {
                QColor ctmp((int)cpts[i].GetColors()[0],
                            (int)cpts[i].GetColors()[1],
                            (int)cpts[i].GetColors()[2],
                            (int)cpts[i].GetColors()[3]);

                spectrumBar->setControlPointColor(i, ctmp);
                spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
            }
        }

        // Set the smoothing and equal spacing flags in the spectrumbar.
        spectrumBar->setEqualSpacing(cpts.GetEqualSpacingFlag());
        spectrumBar->setSmoothing(cpts.GetSmoothingFlag());

        spectrumBar->blockSignals(false);
        spectrumBar->setSuppressUpdates(false);
        spectrumBar->update();

        // Update the equalSpacing and smoothing toggle buttons.
        smoothCheckBox->blockSignals(true);
        smoothCheckBox->setChecked(cpts.GetSmoothingFlag());
        smoothCheckBox->blockSignals(false);

        equalCheckBox->blockSignals(true);
        equalCheckBox->setChecked(cpts.GetEqualSpacingFlag());
        equalCheckBox->blockSignals(false);

        // Update the color sliders, etc.
        ShowSelectedColor(spectrumBar->controlPointColor(spectrumBar->activeControlPoint()));
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::UpdateDiscreteSettings
//
// Purpose: 
//   Updates the widgets associated with discrete color tables.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 20 17:14:54 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Feb 6 14:17:51 PST 2003
//   Added code to extend the color table.
//
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//
// ****************************************************************************

void
QvisColorTableWindow::UpdateDiscreteSettings()
{
    // Get a pointer to the active color table's control points.
    ColorControlPointList *ccpl = GetActiveColorControlPoints();
    if(ccpl && ccpl->GetDiscreteFlag())
    {
        bool doNotify = false;

        // Create an array of colors to set into the color grid widget.
        QColor *c = new QColor[ccpl->GetNumControlPoints()];
        int i;
        for(i = 0; i < ccpl->GetNumControlPoints(); ++i)
        {
            const ColorControlPoint &ccp = ccpl->operator[](i);
            int r = int(ccp.GetColors()[0]);
            int g = int(ccp.GetColors()[1]);
            int b = int(ccp.GetColors()[2]);
            c[i] = QColor(r, g, b);
        }

        // Set the color palette into the color grid widget.
        int cols = (ccpl->GetNumControlPoints() > 30) ?
            (2 * DEFAULT_DISCRETE_COLS) : DEFAULT_DISCRETE_COLS;
        discreteColors->setPaletteColors(c, ccpl->GetNumControlPoints(), cols);
        delete [] c;

        // Now update the sliders and the spin buttons.
        QColor sc(discreteColors->selectedColor());
        for(i = 0; i < 4; ++i)
        {
            componentSliders[i]->blockSignals(true);
            componentSpinBoxes[i]->blockSignals(true);
        }
        componentSliders[0]->setValue(sc.red());
        componentSpinBoxes[0]->setValue(sc.red());
        componentSliders[1]->setValue(sc.green());
        componentSpinBoxes[1]->setValue(sc.green());
        componentSliders[2]->setValue(sc.blue());
        componentSpinBoxes[2]->setValue(sc.blue());
        componentSliders[3]->setValue(sc.alpha());
        componentSpinBoxes[3]->setValue(sc.alpha());
        for(i = 0; i < 4; ++i)
        {
            componentSliders[i]->blockSignals(false);
            componentSpinBoxes[i]->blockSignals(false);
        }

        // If we've modified the color table, tell the viewer.
        if(doNotify)
        {
            SetUpdate(false);
            colorAtts->Notify();
            GetViewerMethods()->UpdateColorTable(currentColorTable.toStdString());
        }
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::PopupColorSelect
//
// Purpose: 
//   This is method pops up the color selection widget.
//
// Arguments:
//   index : The index of the color control point that wants changed.
//   p     : The point where the menu should appear.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 14:06:35 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Sep 19 15:42:53 PST 2001
//   I removed an unused variable.
//
//   Brad Whitlock, Wed Mar 13 18:00:14 PST 2002
//   I added the point argument.
//
//   Brad Whitlock, Thu Nov 21 13:06:29 PST 2002
//   I made it more general.
//
// ****************************************************************************

void
QvisColorTableWindow::PopupColorSelect(const QColor &c, const QPoint &p)
{
    // Set the popup's initial color.
    colorSelect->blockSignals(true);
    colorSelect->setSelectedColor(c);
    colorSelect->blockSignals(false);

    // Figure out a good place to popup the menu.
    int menuW = colorSelect->sizeHint().width();
    int menuH = colorSelect->sizeHint().height();
    int menuX = p.x();
    int menuY = p.y() - (menuH >> 1);

    // Fix the X dimension.
    if(menuX < 0)
        menuX = 0;
    else if(menuX + menuW > QApplication::desktop()->width())
        menuX -= (menuW + 5);

    // Fix the Y dimension.
    if(menuY < 0)
        menuY = 0;
    else if(menuY + menuH > QApplication::desktop()->height())
        menuY -= ((menuY + menuH) - QApplication::desktop()->height());

    // Show the popup menu.         
    colorSelect->move(menuX, menuY);
    colorSelect->show();
}

// ****************************************************************************
// Method: QvisColorTableWindow::ShowSelectedColor
//
// Purpose: 
//   Makes the discrete color widgets display the specified color.
//
// Arguments:
//   c : The color to display.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 13:38:52 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Aug 11 17:14:32 EDT 2006
//    Refactoring of color grid widget caused index to lose "color" in names.
//   
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//
// ****************************************************************************

void
QvisColorTableWindow::ShowSelectedColor(const QColor &c)
{
    int i;

    const ColorControlPointList *ccpl = GetActiveColorControlPoints();
    bool updateDiscrete = (ccpl && ccpl->GetDiscreteFlag());

    // Update the color
    if(updateDiscrete)
    {
        discreteColors->blockSignals(true);
        discreteColors->setPaletteColor(c, discreteColors->selectedIndex());
        discreteColors->blockSignals(false);
    }
    else
    {
        spectrumBar->blockSignals(true);
        spectrumBar->setControlPointColor(spectrumBar->activeControlPoint(), c);
        spectrumBar->blockSignals(false);
    }

    // Disable signals in the sliders.
    for(i = 0; i < 4; ++i)
    {
        componentSliders[i]->blockSignals(true);
        componentSpinBoxes[i]->blockSignals(true);
    }

    // Update the sliders and the text fields.
    componentSliders[0]->setValue(c.red());
    componentSpinBoxes[0]->setValue(c.red());
    componentSliders[1]->setValue(c.green());
    componentSpinBoxes[1]->setValue(c.green());
    componentSliders[2]->setValue(c.blue());
    componentSpinBoxes[2]->setValue(c.blue());
    componentSliders[3]->setValue(c.alpha());
    componentSpinBoxes[3]->setValue(c.alpha());

    // Enable signals in the sliders.
    for(i = 0; i < 4; ++i)
    {
        componentSliders[i]->blockSignals(false);
        componentSpinBoxes[i]->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::ChangeSelectedColor
//
// Purpose: 
//   Change the active discrete color to the specified color.
//
// Arguments:
//   c : the new color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 13:38:22 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Aug 11 17:14:32 EDT 2006
//    Refactoring of color grid widget caused index to lose "color" in names.
//   
//    Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//    Added alpha channel support.
//
// ****************************************************************************

void
QvisColorTableWindow::ChangeSelectedColor(const QColor &c)
{
    // Change the color in the colorAtts.
    ColorControlPointList *ccpl = GetActiveColorControlPoints();
    if(ccpl)
    {
        int index;

        if(ccpl->GetDiscreteFlag())
        { 
            // Use the selected color in the discreteColors widget to determine
            // which color needs to be replaced in the color control point list.
            index = discreteColors->selectedIndex();
        }
        else
            index = spectrumBar->activeControlPoint();

        // If a valid index was calculated, poke the new color value into
        // the specified color control point and update.
        if(index >= 0 && index < ccpl->GetNumControlPoints())
        {
            ColorControlPoint &ccp = ccpl->GetControlPoints(index);
            ccp.GetColors()[0] = (unsigned char)c.red();
            ccp.GetColors()[1] = (unsigned char)c.green();
            ccp.GetColors()[2] = (unsigned char)c.blue();
            ccp.GetColors()[3] = (unsigned char)c.alpha();
            ccp.SelectColors();
            ccpl->SelectControlPoints();

            // If we're sliding, don't tell the viewer about it until we're done.
            if(!sliding)
            {
                // Prevent updates so we can redraw just the widgets that we want.
                SetUpdate(false);
                Apply();
            }
        }
    }

    // Redraw only the discrete color table widgets so it is faster.
    ShowSelectedColor(c);
}

// ****************************************************************************
// Method: QvisColorTableWindow::GetNextColor
//
// Purpose: 
//   Returns the next color in the sequence.
//
// Returns:    A color.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 15:11:49 PST 2003
//
// Modifications:
//   
// ****************************************************************************

QColor
QvisColorTableWindow::GetNextColor()
{
    // Table of default control point colors.
    static int colors[] = {
        255, 0, 0,
        255, 255, 0,
        0, 255, 0,
        0, 255, 255,
        0, 0, 255,
        255, 0, 255,
        0, 0, 0,
        255, 255, 255};

    QColor retval(colors[colorCycle*3],
                  colors[colorCycle*3+1],
                  colors[colorCycle*3+2]);

    // Move on to the next color.
    colorCycle = (colorCycle < 7) ? (colorCycle + 1) : 0;

    return retval;
}

// ****************************************************************************
// Method: QvisColorTableWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values from certain widgets and stores the values in the
//   state object.
//
// Arguments:
//   which_widget : A number indicating which widget that we should get the
//                  values from.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 18:42:52 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 16:46:46 PST 2002
//   I changed the code so it supports discrete color tables.
//
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//
// ****************************************************************************

void
QvisColorTableWindow::GetCurrentValues(int which_widget)
{
    int i;

    // Get the rgb colors from the spectrumbar and put them into the state
    // object's rgbaColors array.
    if(which_widget == 0 || which_widget == -1)
    {
        // Store the widget's control points.
        ColorControlPointList cpts;
        cpts.ClearControlPoints();
        cpts.SetSmoothingFlag(spectrumBar->smoothing());
        cpts.SetEqualSpacingFlag(spectrumBar->equalSpacing());
        for(i = 0; i < spectrumBar->numControlPoints(); ++i)
        {
            QColor c(spectrumBar->controlPointColor(i));
            float  pos = spectrumBar->controlPointPosition(i);

            ColorControlPoint pt;
            unsigned char ptColors[4];
            ptColors[0] = (unsigned char)c.red();
            ptColors[1] = (unsigned char)c.green();
            ptColors[2] = (unsigned char)c.blue();
            ptColors[3] = (unsigned char)c.alpha();
            pt.SetColors(ptColors);
            pt.SetPosition(pos);
            cpts.AddControlPoints(pt);
        }

        // Get a pointer to the active color table's control points.
        ColorControlPointList *ccpl = GetActiveColorControlPoints();
        if(ccpl)
        {
            ColorControlPointList &activeControlPoints = *ccpl;
            activeControlPoints = cpts;
        }
    }

    // Get the name of the active color table. If it differs from the active
    // color table in the state object, set it into the state object.
    if(which_widget == 1 || which_widget == -1)
    {
        QString temp = nameLineEdit->displayText().simplified();
        bool okay = !temp.isEmpty();
        if(okay)
        {
            currentColorTable = temp;
        }
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::Apply
//
// Purpose: 
//   This is a Qt slot function that is called when the Apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 14:00:18 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Send the color table definitions to the viewer.
        GetCurrentValues(1);
        colorAtts->Notify();

        // Make the viewer update the plots that use the specified colortable.
        GetViewerMethods()->UpdateColorTable(currentColorTable.toStdString());
    }
    else
        colorAtts->Notify();
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisColorTableWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the window's Apply
//   button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 13:42:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisColorTableWindow::alignControlPoints
//
// Purpose: 
//   This is a Qt slot function that tells the spectrum bar to align its
//   control points.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 10:04:17 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Jul 14 15:04:07 PST 2003
//   Added code to block signals.
//
// ****************************************************************************

void
QvisColorTableWindow::alignControlPoints()
{
    // Align the control points.
    spectrumBar->blockSignals(true);
    spectrumBar->alignControlPoints();
    spectrumBar->blockSignals(false);

    // Get the current attributes.
    GetCurrentValues(0);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisColorTableWindow::controlPointMoved
//
// Purpose: 
//   This is a Qt slot function that is called when the spectrum bar's control
//   points are moved.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 11:39:32 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::controlPointMoved(int, float)
{
    // Get the current attributes.
    GetCurrentValues(0);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisColorTableWindow::chooseContinuousColor
//
// Purpose: 
//   This is a Qt slot function that is called when a color control point
//   in the spectrum bar is clicked such that the color selection widget
//   needs to be activated in order to select a new color.
//
// Arguments:
//   index : The index of the active color control point.
//   p     : The point that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 13:06:47 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::chooseContinuousColor(int index, const QPoint &p)
{
    popupMode = SELECT_FOR_CONTINUOUS;
    PopupColorSelect(spectrumBar->controlPointColor(index), p);
}

// ****************************************************************************
// Method: QvisColorTableWindow::chooseDiscreteColor
//
// Purpose: 
//   This is a Qt slot function that is called when a discrete color is clicked
//   such that the color selection widget needs to be activated in order to
//   select a new color.
//
// Arguments:
//   c : The discrete color.
//   p : The point that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 13:06:47 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::chooseDiscreteColor(const QColor &c, int, int,
    const QPoint &p)
{
    popupMode = SELECT_FOR_DISCRETE;
    PopupColorSelect(c, p);
}

// ****************************************************************************
// Method: QvisColorTableWindow::selectedColor
//
// Purpose: 
//   This is a Qt slot function that is called by the color popup menu when
//   a new color has been selected.
//
// Arguments:
//   color : The new color that was selected.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 14:07:31 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Nov 21 12:54:01 PDT 2002
//   I modified the routine so it behaves differently if the menu was
//   activated by choosing a discrete color table.
//
// ****************************************************************************

void
QvisColorTableWindow::selectedColor(const QColor &color)
{
    // Hide the popup menu.
    colorSelect->hide();

    if(color.isValid())
    {
        if(popupMode == SELECT_FOR_CONTINUOUS)
        {
            // Set the color of the active color control point.
            spectrumBar->setControlPointColor(spectrumBar->activeControlPoint(),
                                              color);

            // Get the current values for the spectrum bar.
            GetCurrentValues(0);
            Apply();
            ShowSelectedColor(color);
        }
        else
        {
            // Change the active discrete color to the new color.
            ChangeSelectedColor(color);
        }
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::smoothToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's smooth
//   toggle is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 15:38:06 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::smoothToggled(bool)
{
    // Get a pointer to the active color table's control points.
    ColorControlPointList *ccpl = GetActiveColorControlPoints();

    if(ccpl)
    {
        ccpl->SetSmoothingFlag(!ccpl->GetSmoothingFlag());
        colorAtts->SelectColorTables();
        Apply();
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::showIndexHintsToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's 
//   show index hints toggle is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   December 31, 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::showIndexHintsToggled(bool val)
{
    discreteColors->setShowIndexHints(val);
}

// ****************************************************************************
// Method: QvisColorTableWindow::equalSpacingToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the window's equal spacing
//   toggle is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 15:38:06 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::equalSpacingToggled(bool)
{
    // Get a pointer to the active color table's control points.
    ColorControlPointList *ccpl = GetActiveColorControlPoints();

    if(ccpl)
    {
        ccpl->SetEqualSpacingFlag(!ccpl->GetEqualSpacingFlag());
        colorAtts->SelectColorTables();
        Apply();
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::addColorTable
//
// Purpose: 
//   This is a Qt slot function that adds a new color table that, if possible,
//   is based on the current colortable.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:02:15 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 17:11:36 PST 2002
//   I changed the code so discrete color tables are supported. I also
//   removed the code to make the new color table the active color table.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisColorTableWindow::addColorTable()
{
    // Get a pointer to the active color table's control points.
    ColorControlPointList *ccpl = GetActiveColorControlPoints();

    // Get the name of the new colortable. This stores a new name into
    // the currentColorTable variable.
    GetCurrentValues(1);
    if(colorAtts->GetColorTableIndex(currentColorTable.toStdString()) < 0)
    {
        // Add the new colortable to colorAtts.
        if(ccpl)
        {
            // Copy the active color table into the new color table.
            ColorControlPointList cpts(*ccpl);
            colorAtts->AddColorTable(currentColorTable.toStdString(), cpts);
        }
        else
        {
            // There is no active color table so create a default color table
            // and add it with the specified name.
            ColorControlPointList cpts;
            cpts.AddControlPoints(ColorControlPoint(0., 255,0,0,255));
            cpts.AddControlPoints(ColorControlPoint(0.25, 255,255,0,255));
            cpts.AddControlPoints(ColorControlPoint(0.5, 0,255,0,255));
            cpts.AddControlPoints(ColorControlPoint(0.75, 0,255,255,255));
            cpts.AddControlPoints(ColorControlPoint(1., 0,0,255,255));
            cpts.SetSmoothingFlag(true);
            cpts.SetEqualSpacingFlag(false);
            cpts.SetDiscreteFlag(false);
            colorAtts->AddColorTable(currentColorTable.toStdString(), cpts);
        }

        // Tell all of the observers to update.
        Apply();
    }
    else
    {
        QString tmp;
        tmp = tr("The color table ") + 
              QString("\"") + currentColorTable + QString("\"") + 
              tr(" is already in the color table list. You must provide a "
                 "unique name for the new color table before it can be added.");
        Error(tmp);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::deleteColorTable
//
// Purpose: 
//   This is a Qt slot function that is called to delete a color table.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:03:26 PST 2001
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisColorTableWindow::deleteColorTable()
{
    // Get the index of the currently selected color table and tell the viewer
    // to remove it from the list of color tables.
    int index = nameListBox->currentRow();
    if(index >= 0 && index < colorAtts->GetNumColorTables())
    {
        std::string ctName(colorAtts->GetNames()[index]);
        GetViewerMethods()->DeleteColorTable(ctName.c_str());
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::highlightColorTable
//
// Purpose: 
//   This is a Qt slot function that is called when a color table is
//   highligted. It serves to make the highlighted plot the new active color
//   table.
//
// Arguments:
//   index : The index of the color table that was highlighted.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:03:56 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 4 15:32:49 PST 2002
//   Added a little code to set the name line edit's contents. We normally
//   do not set widget state outside of the UpdateWindow method but this
//   is a special case. This makes auto update work.
//
//   Brad Whitlock, Thu Nov 21 14:25:25 PST 2002
//   I rewrote the method.
//
// ****************************************************************************

void
QvisColorTableWindow::highlightColorTable(int index)
{
    // Set the active color table to the one that is currently highlighted.
    if(index < colorAtts->GetNumColorTables())
    {
        currentColorTable = QString(colorAtts->GetNames()[index].c_str());
        nameLineEdit->setText(currentColorTable);
        UpdateEditor();
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::setColorTableType
//
// Purpose: 
//   This is a Qt slot function that sets the type of the color table.
//
// Arguments:
//   index : The new color table type. 0=continuous, 1=discrete.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 11:10:52 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::setColorTableType(int index)
{
    ColorControlPointList *ccpl = GetActiveColorControlPoints();
    if(ccpl)
    {
        ccpl->SetDiscreteFlag(index == 1);
        colorAtts->SelectColorTables();
        Apply();
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::activateContinuousColor
//
// Purpose: 
//   This is a Qt slot function that is called when we click on a new
//   color control point in the spectrum bar. This function updates the
//   color sliders to the right values when we select a new control point.
//
// Arguments:
//   index : The index of the new color control point.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 12:49:41 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::activateContinuousColor(int index)
{
    ShowSelectedColor(spectrumBar->controlPointColor(index));
}

// ****************************************************************************
// Method: QvisColorTableWindow::activateDiscreteColor
//
// Purpose: 
//   This is a Qt slot function that is called when we click on a discrete
//   color thus making it active.
//
// Arguments:
//   c      : The color value.
//   row    : The color's row in the color grid.
//   column : The color's column in the color grid.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:43:38 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::activateDiscreteColor(const QColor &c, int)
{
    ShowSelectedColor(c);
}

// ****************************************************************************
// Method: QvisColorTableWindow::redValueChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the active discrete color
//   changes due to the red slider or spin box.
//
// Arguments:
//   r : The new red value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:44:44 PST 2002
//
// Modifications:
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//   
// ****************************************************************************

void
QvisColorTableWindow::redValueChanged(int r)
{
    const ColorControlPointList *ccpl = GetActiveColorControlPoints();
    if(ccpl)
    {
        QColor c;

        if(ccpl->GetDiscreteFlag())
            c = discreteColors->selectedColor();
        else
            c = spectrumBar->controlPointColor(spectrumBar->activeControlPoint());

        c.setRgb(r, c.green(), c.blue(), c.alpha());
        ChangeSelectedColor(c);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::greenValueChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the active discrete color
//   changes due to the green slider or spin box.
//
// Arguments:
//   g : The new green value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:44:44 PST 2002
//
// Modifications:
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//   
// ****************************************************************************

void
QvisColorTableWindow::greenValueChanged(int g)
{
    const ColorControlPointList *ccpl = GetActiveColorControlPoints();
    if(ccpl)
    {
        QColor c;

        if(ccpl->GetDiscreteFlag())
            c = discreteColors->selectedColor();
        else
            c = spectrumBar->controlPointColor(spectrumBar->activeControlPoint());

        c.setRgb(c.red(), g, c.blue(), c.alpha());
        ChangeSelectedColor(c);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::blueValueChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the active discrete color
//   changes due to the blue slider or spin box.
//
// Arguments:
//   b : The new blue value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:44:44 PST 2002
//
// Modifications:
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//   
// ****************************************************************************

void
QvisColorTableWindow::blueValueChanged(int b)
{
    const ColorControlPointList *ccpl = GetActiveColorControlPoints();
    if(ccpl)
    {
        QColor c;

        if(ccpl->GetDiscreteFlag())
            c = discreteColors->selectedColor();
        else
            c = spectrumBar->controlPointColor(spectrumBar->activeControlPoint());

        c.setRgb(c.red(), c.green(), b, c.alpha());
        ChangeSelectedColor(c);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::alphaValueChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the active discrete color
//   changes due to the alpha slider or spin box.
//
// Arguments:
//   a : The new alpha value.
//
// Programmer: Jeremy Meredith
// Creation:   February 20, 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::alphaValueChanged(int a)
{
    const ColorControlPointList *ccpl = GetActiveColorControlPoints();
    if(ccpl)
    {
        QColor c;

        if(ccpl->GetDiscreteFlag())
            c = discreteColors->selectedColor();
        else
            c = spectrumBar->controlPointColor(spectrumBar->activeControlPoint());

        c.setRgb(c.red(), c.green(), c.blue(), a);
        ChangeSelectedColor(c);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::sliderPressed
//
// Purpose: 
//   This is a Qt slot function that tells the window that the slider is
//   being dragged so we can prevent the window from sending Updates to the
//   viewer for each grad event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 10:53:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::sliderPressed()
{
    sliding = true;
}

// ****************************************************************************
// Method: QvisColorTableWindow::sliderReleased
//
// Purpose: 
//   Tells the window that the slider was released and that changes were made
//   so we should update.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 10:55:18 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::sliderReleased()
{
    sliding = false;
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisColorTableWindow::setActiveContinuous
//
// Purpose: 
//   This is Qt slot function that sets the active continuous color table.
//
// Arguments:
//   ct : The name of the new active color table.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 10:55:53 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::setActiveContinuous(const QString &ct)
{
    colorAtts->SetActiveContinuous(ct.toStdString());
    Apply();
}

// ****************************************************************************
// Method: QvisColorTableWindow::setActiveDiscrete
//
// Purpose: 
//   This is Qt slot function that sets the active discrete color table.
//
// Arguments:
//   ct : The name of the new active color table.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 10:55:53 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::setActiveDiscrete(const QString &ct)
{
    colorAtts->SetActiveDiscrete(ct.toStdString());
    Apply();
}

// ****************************************************************************
// Method: QvisColorTableWindow::resizeColorTable
//
// Purpose: 
//   This routine resizes the color table.
//
// Arguments:
//   size : The new color table size.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 15:12:20 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Jul 14 14:35:27 PST 2003
//   I added code to prevent the spectrum bar from emitting signals while
//   it is having color control points added or removed.
//
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//
// ****************************************************************************

void
QvisColorTableWindow::resizeColorTable(int size)
{
    ColorControlPointList *ccpl = GetActiveColorControlPoints();
    if(ccpl)
    {
        int i;

        if(ccpl->GetDiscreteFlag())
        {
            if(size < ccpl->GetNumControlPoints())
            {
                // Remove colors from the end of the list.
                int rmPoints = ccpl->GetNumControlPoints() - size;
                for(i = 0; i < rmPoints; ++i)
                    ccpl->RemoveControlPoints(ccpl->GetNumControlPoints()-1);

                // Reposition the points evenly.
                for(i = 0; i < ccpl->GetNumControlPoints(); ++i)
                {
                    ColorControlPoint &c = ccpl->GetControlPoints(i);
                    c.SetPosition(float(i) / float(ccpl->GetNumControlPoints() - 1));
                }

                colorAtts->SelectColorTables();
                Apply();   
            }
            else if(size > ccpl->GetNumControlPoints())
            {
                // Add colors to the end of the list.
                int addPoints = size - ccpl->GetNumControlPoints();
                for(i = 0; i < addPoints; ++i)
                {
                    QColor c(GetNextColor());
                    ColorControlPoint newPt(1., c.red(), c.green(), c.blue(), c.alpha());
                    ccpl->AddControlPoints(newPt);
                }

                // Reposition the points evenly.
                for(i = 0; i < ccpl->GetNumControlPoints(); ++i)
                {
                    ColorControlPoint &c = ccpl->GetControlPoints(i);
                    c.SetPosition(float(i) / float(ccpl->GetNumControlPoints() - 1));
                }

                colorAtts->SelectColorTables();
                Apply();
            }
        }
        else
        {
            //
            // If the color table is continuous, add points through the spectrum
            // bar so that they are added and removed in the right places. I know
            // this is a little weird but we need to do it like this.
            //
            if(size < ccpl->GetNumControlPoints())
            {
                int rmPoints = ccpl->GetNumControlPoints() - size;
                spectrumBar->blockSignals(true);
                spectrumBar->setSuppressUpdates(true);
                for(int i = 0; i < rmPoints; ++i)
                    spectrumBar->removeControlPoint();
                spectrumBar->blockSignals(false);
                spectrumBar->setSuppressUpdates(false);
                spectrumBar->update();

                GetCurrentValues(0);
                SetUpdate(false);
                Apply();   
            }
            else if(size > spectrumBar->numControlPoints())
            {
                int addPoints = size - spectrumBar->numControlPoints();
                spectrumBar->blockSignals(true);
                spectrumBar->setSuppressUpdates(true);
                for(int i = 0; i < addPoints; ++i)
                    spectrumBar->addControlPoint(GetNextColor());
                spectrumBar->blockSignals(false);
                spectrumBar->setSuppressUpdates(false);
                spectrumBar->update();

                GetCurrentValues(0);
                SetUpdate(false);
                Apply();
            }
        }
    }    
}

// ****************************************************************************
// Method: QvisColorTableWindow::exportColorTable
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to export the currently
//   selected color table.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 16:40:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisColorTableWindow::exportColorTable()
{
    GetViewerMethods()->ExportColorTable(currentColorTable.toStdString());
}
