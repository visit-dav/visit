// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisTensorPlotWindow.h>
#include <QLayout> 
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QTabWidget>

#include <TensorAttributes.h>
#include <ViewerProxy.h>

#include <QvisColorButton.h>
#include <QvisColorTableWidget.h>

// ****************************************************************************
// Method: QvisTensorPlotWindow::QvisTensorPlotWindow
//
// Purpose: 
//   Constructor for the QvisTensorPlotWindow class.
//
// Arguments:
//   type      : An identifier used to identify the plot type in the viewer.
//   _vecAtts  : The tensor attributes that the window observes.
//   caption   : The caption displayed in the window decorations.
//   shortName : The name used in the notepad.
//   notepad   : The notepad area where the window posts itself.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

QvisTensorPlotWindow::QvisTensorPlotWindow(const int type,
    TensorAttributes *_vecAtts, const QString &caption, const QString &shortName,
    QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(_vecAtts, caption, shortName, notepad)
{
    plotType = type;
    tensorAtts = _vecAtts;

    // Initialize parentless widgets.
    reduceButtonGroup = 0;
    colorButtonGroup = 0;
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::~QvisTensorPlotWindow
//
// Purpose: 
//   Destructor for the QvisTensorPlotWindow class.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// Modifications:
//   Cyrus Harrison, Wed Aug 27 08:54:49 PDT 2008
//   Made sure a button groups have parents, so we don't need to explicitly
//   delete.
//
// ****************************************************************************

QvisTensorPlotWindow::~QvisTensorPlotWindow()
{
    tensorAtts = nullptr;
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets that are in the window and sets
//   up their signals/slots.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// Modifications:
//   Eric Brugger, Wed Nov 24 11:43:13 PST 2004
//   Added scaleByMagnitude and autoScale.
//
//   Brad Whitlock, Wed Apr 23 12:04:43 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Tue Aug 8 20:12:23 PST 2008
//   Qt 4.
//
//   Allen Sanderson, Sun Mar  7 12:49:56 PST 2010
//   Change layout of window for 2.0 interface changes.
//
//   Kathleen Bonnell, Mon Jan 17 18:17:26 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
// ****************************************************************************

void
QvisTensorPlotWindow::CreateWindowContents()
{
    QTabWidget *propertyTabs = new QTabWidget(central);
    topLayout->addWidget(propertyTabs);

    // ----------------------------------------------------------------------
    // Sampling tab
    // ----------------------------------------------------------------------
    QWidget *samplingTab = new QWidget(central);
    propertyTabs->addTab(samplingTab, tr("Sampling"));
    CreateSamplingTab(samplingTab);

    // ----------------------------------------------------------------------
    // Data tab
    // ----------------------------------------------------------------------
    QWidget *dataTab = new QWidget(central);
    propertyTabs->addTab(dataTab, tr("Data"));
    CreateDataTab(dataTab);

    // ----------------------------------------------------------------------
    // Geometry tab
    // ----------------------------------------------------------------------
    QWidget *geometryTab = new QWidget(central);
    propertyTabs->addTab(geometryTab, tr("Geometry"));
    CreateGeometryTab(geometryTab);
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::CreateSamplingTab
//
// Purpose: 
//   Populates the sampling tab.
//
// Programmer: Allen Sanderson
// Creation:   September 20 2013
//
// Modifications:
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisTensorPlotWindow::CreateSamplingTab(QWidget *pageTensor)
{
    QGridLayout *topLayout = new QGridLayout(pageTensor);
    topLayout->setContentsMargins(5,5,5,5);
    topLayout->setSpacing(10);

    //
    // Create the reduce-related widgets.
    //
    QGroupBox * reduceGroupBox = new QGroupBox(central);
    reduceGroupBox->setTitle(tr("Where to place the tensors and how many of them"));
    topLayout->addWidget(reduceGroupBox);
    QGridLayout *rgLayout = new QGridLayout(reduceGroupBox);
    rgLayout->setSpacing(10);
//    rgLayout->setColumnStretch(1, 10);

    // Create the data location button group.
    QLabel *locationLabel = new QLabel(tr("Placement"), reduceGroupBox);
    rgLayout->addWidget(locationLabel, 0, 0);
    locationButtonGroup = new QButtonGroup(reduceGroupBox);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(locationButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(locationMethodChanged(int)));
#else
    connect(locationButtonGroup, SIGNAL(idClicked(int)),
            this, SLOT(locationMethodChanged(int)));
#endif
    QRadioButton *rb = new QRadioButton(tr("Adapt to the mesh resolution"), reduceGroupBox);
    rb->setChecked(true);
    locationButtonGroup->addButton(rb, 0);
    rgLayout->addWidget(rb, 0, 1, 1, 3);
    rb = new QRadioButton(tr("Uniformly located throughout mesh"), reduceGroupBox);
    locationButtonGroup->addButton(rb, 1);
    rgLayout->addWidget(rb, 2, 1, 1, 3);

    QFrame *hline1 = new QFrame(reduceGroupBox);
    hline1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    rgLayout->addWidget(hline1, 3, 0, 1, 4);

    // Create the reduce button group.
    QLabel *reduceLabel = new QLabel(tr("Sampling"), reduceGroupBox);
    rgLayout->addWidget(reduceLabel, 4, 0);
    reduceButtonGroup = new QButtonGroup(reduceGroupBox);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(reduceButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(reduceMethodChanged(int)));
#else
    connect(reduceButtonGroup, SIGNAL(idClicked(int)),
            this, SLOT(reduceMethodChanged(int)));
#endif
    rb = new QRadioButton(tr("Fixed number"), reduceGroupBox);
    rb->setChecked(true);
    reduceButtonGroup->addButton(rb, 0);
    rgLayout->addWidget(rb, 4, 1);
    strideRB = new QRadioButton(tr("Stride"), reduceGroupBox);
    reduceButtonGroup->addButton(strideRB, 1);
    rgLayout->addWidget(strideRB, 5, 1);

    // Add the N tensors line edit.
    nTensorsLineEdit = new QLineEdit(reduceGroupBox);
    connect(nTensorsLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processNTensorsText()));
    rgLayout->addWidget(nTensorsLineEdit, 4, 2);

    // Add the stride line edit.
    strideLineEdit = new QLineEdit(reduceGroupBox);
    connect(strideLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processStrideText()));
    rgLayout->addWidget(strideLineEdit, 5, 2);

    QFrame *hline2 = new QFrame(reduceGroupBox);
    hline2->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    rgLayout->addWidget(hline2, 6, 0, 1, 4);

    // Add the toggle to limit to one vector per original cell/node
    limitToOrigToggle =
      new QCheckBox(tr("Only show tensors on original nodes/cells"),
                    reduceGroupBox);
    connect(limitToOrigToggle, SIGNAL(toggled(bool)),
            this, SLOT(limitToOrigToggled(bool)));
    rgLayout->addWidget(limitToOrigToggle, 7, 0, 1, 4);
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::CreateDataTab
//
// Purpose: 
//   Populates the data tab.
//
// Programmer: Allen Sanderson
// Creation:   September 20 2013
//
// Modifications:
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisTensorPlotWindow::CreateDataTab(QWidget *pageTensor)
{
    QGridLayout *topLayout = new QGridLayout(pageTensor);
    topLayout->setContentsMargins(5,5,5,5);
    topLayout->setSpacing(10);

    //
    // Create the Limits stuff
    //
    limitsGroup = new QGroupBox(central);
    limitsGroup->setTitle(tr("Limits"));
    topLayout->addWidget(limitsGroup);

    QGridLayout *limitsLayout = new QGridLayout(limitsGroup);
    limitsLayout->setContentsMargins(5,5,5,5);
    limitsLayout->setSpacing(10);

    limitsLayout->addWidget( new QLabel(tr("Limits"), central), 0, 0);

    limitsSelect = new QComboBox(central);
    limitsSelect->addItem(tr("Use Original Data"));
    limitsSelect->addItem(tr("Use Current Plot"));
    connect(limitsSelect, SIGNAL(activated(int)),
            this, SLOT(limitsSelectChanged(int))); 
    limitsLayout->addWidget(limitsSelect, 0, 1, 1, 2, Qt::AlignLeft);

    // Create the min toggle and line edit
    minToggle = new QCheckBox(tr("Minimum"), central);
    limitsLayout->addWidget(minToggle, 1, 0);
    connect(minToggle, SIGNAL(toggled(bool)),
            this, SLOT(minToggled(bool)));
    minLineEdit = new QLineEdit(central);
    connect(minLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMinLimitText())); 
    limitsLayout->addWidget(minLineEdit, 1, 1);

    // Create the max toggle and line edit
    maxToggle = new QCheckBox(tr("Maximum"), central);
    limitsLayout->addWidget(maxToggle, 2, 0);
    connect(maxToggle, SIGNAL(toggled(bool)),
            this, SLOT(maxToggled(bool)));
    maxLineEdit = new QLineEdit(central);
    connect(maxLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processMaxLimitText())); 
    limitsLayout->addWidget(maxLineEdit, 2, 1);

    //
    // Create the color-related widgets.
    //
    QGroupBox * colorGroupBox = new QGroupBox(central);
    colorGroupBox->setTitle(tr("Color"));
    topLayout->addWidget(colorGroupBox);

    QGridLayout *cgLayout = new QGridLayout(colorGroupBox);
    cgLayout->setContentsMargins(5,5,5,5);
    cgLayout->setSpacing(10);
    cgLayout->setColumnStretch(1, 10);

    // Add the color label.
    colorButtonGroup = new QButtonGroup(colorGroupBox);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(colorButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(colorModeChanged(int)));
#else
    connect(colorButtonGroup, SIGNAL(idClicked(int)),
            this, SLOT(colorModeChanged(int)));
#endif
    QRadioButton* rb = new QRadioButton(tr("Eigen values"), colorGroupBox);
    colorButtonGroup->addButton(rb, 0);
    cgLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton(tr("Constant"), colorGroupBox);
    rb->setChecked(true);
    colorButtonGroup->addButton(rb, 1);
    cgLayout->addWidget(rb, 1, 0);

    // Create the color-by-eigenvalues button.
    colorTableWidget = new QvisColorTableWidget(colorGroupBox, true);
    connect(colorTableWidget, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    connect(colorTableWidget,
            SIGNAL(invertColorTableToggled(bool)),
            this,
            SLOT(invertColorTableToggled(bool)));
    cgLayout->addWidget(colorTableWidget, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);

    // Create the tensor color button.
    tensorColor = new QvisColorButton(colorGroupBox);
    tensorColor->setButtonColor(QColor(255, 0, 0));
    connect(tensorColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(tensorColorChanged(const QColor &)));
    cgLayout->addWidget(tensorColor, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

    //
    // Create the misc stuff
    //
    QGroupBox * miscGroup = new QGroupBox(central);
    miscGroup->setTitle(tr("Misc"));
    topLayout->addWidget(miscGroup);

    QGridLayout *miscLayout = new QGridLayout(miscGroup);
    miscLayout->setContentsMargins(5,5,5,5);
    miscLayout->setSpacing(10);
 
    // Create the legend toggle
    legendToggle = new QCheckBox(tr("Legend"), central);
    connect(legendToggle, SIGNAL(toggled(bool)),
            this, SLOT(legendToggled(bool)));
    miscLayout->addWidget(legendToggle, 0, 0);
}


// ****************************************************************************
// Method: QvisTensorPlotWindow::CreateGeometryTab
//
// Purpose: 
//   Populates the geometry tab.
//
// Programmer: Allen Sanderson
// Creation:   September 20 2013
//
// Modifications:
//
// ****************************************************************************

void
QvisTensorPlotWindow::CreateGeometryTab(QWidget *pageGlyphs)
{
    QGridLayout *topLayout = new QGridLayout(pageGlyphs);
    topLayout->setContentsMargins(5,5,5,5);
    topLayout->setSpacing(10);

    //
    // Create the scale-related widgets.
    //
    QGroupBox * scaleGroupBox = new QGroupBox(central);
    scaleGroupBox->setTitle(tr("Scale"));
    topLayout->addWidget(scaleGroupBox);

    QGridLayout *sgLayout = new QGridLayout(scaleGroupBox);
    sgLayout->setContentsMargins(5,5,5,5);
    sgLayout->setSpacing(10);
    sgLayout->setColumnStretch(1, 10);

    // Add the scale line edit.
    scaleLineEdit = new QLineEdit(scaleGroupBox);
    connect(scaleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processScaleText()));
    sgLayout->addWidget(scaleLineEdit, 0, 1);
    QLabel *scaleLabel = new QLabel(tr("Scale"), scaleGroupBox);
    scaleLabel->setBuddy(scaleLineEdit);
    sgLayout->addWidget(scaleLabel, 0, 0, Qt::AlignRight | Qt::AlignVCenter);

    // Add the scale by magnitude toggle button.
    scaleByMagnitudeToggle = new QCheckBox(tr("Scale by magnitude"), scaleGroupBox);
    connect(scaleByMagnitudeToggle, SIGNAL(clicked(bool)), 
            this, SLOT(scaleByMagnitudeToggled(bool)));
    sgLayout->addWidget(scaleByMagnitudeToggle, 0, 2);

    // Add the auto scale toggle button.
    autoScaleToggle = new QCheckBox(tr("Auto scale"), scaleGroupBox);
    connect(autoScaleToggle, SIGNAL(clicked(bool)),
            this, SLOT(autoScaleToggled(bool)));
    sgLayout->addWidget(autoScaleToggle, 0, 3);
}


// ****************************************************************************
// Method: QvisTensorPlotWindow::UpdateWindow
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
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// Modifications:
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Eric Brugger, Wed Nov 24 11:43:13 PST 2004
//   Added scaleByMagnitude and autoScale.
//
//   Brad Whitlock, Tue Aug 8 20:12:23 PST 2008
//   Qt 4.
//
//   Kathleen Bonnell, Mon Jan 17 18:17:26 MST 2011
//   Change colorTableButton to colorTableWidget to gain invert toggle.
//
//   Kathleen Biagas, Thu Apr 9 07:19:54 MST 2015
//   Use helper function DoubleToQString for consistency in formatting across
//   all windows.
//
// ****************************************************************************

void
QvisTensorPlotWindow::UpdateWindow(bool doAll)
{
    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < tensorAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!tensorAtts->IsSelected(i))
                continue;
        }
        
        switch(i)
        {
          case TensorAttributes::ID_glyphLocation:
            locationButtonGroup->blockSignals(true);
            locationButtonGroup->button(tensorAtts->GetGlyphLocation() == TensorAttributes::AdaptsToMeshResolution ? 0 : 1);
            strideRB->setEnabled(tensorAtts->GetGlyphLocation() == TensorAttributes::AdaptsToMeshResolution);
            limitToOrigToggle->setEnabled(tensorAtts->GetGlyphLocation() == TensorAttributes::AdaptsToMeshResolution);
            locationButtonGroup->blockSignals(false);
          case TensorAttributes::ID_useStride:
            reduceButtonGroup->blockSignals(true);
            reduceButtonGroup->button(tensorAtts->GetUseStride()?1:0)->setChecked(true);
            reduceButtonGroup->blockSignals(false);

            nTensorsLineEdit->setEnabled(!tensorAtts->GetUseStride());
            strideLineEdit->setEnabled(tensorAtts->GetUseStride());
            break;
          case TensorAttributes::ID_stride:
            strideLineEdit->setText(IntToQString(tensorAtts->GetStride()));
            break;
          case TensorAttributes::ID_nTensors:
            nTensorsLineEdit->setText(IntToQString(tensorAtts->GetNTensors()));
            break;
          case TensorAttributes::ID_origOnly:
            limitToOrigToggle->blockSignals(true);
            limitToOrigToggle->setChecked(tensorAtts->GetOrigOnly());
            limitToOrigToggle->blockSignals(false);
            break;

          case TensorAttributes::ID_limitsMode:
            limitsSelect->blockSignals(true);
            limitsSelect->setCurrentIndex(tensorAtts->GetLimitsMode());
            limitsSelect->blockSignals(false);
            break;
          case TensorAttributes::ID_minFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(minToggle, SIGNAL(toggled(bool)),
                       this, SLOT(minToggled(bool)));
            minToggle->setChecked(tensorAtts->GetMinFlag());
            minLineEdit->setEnabled(tensorAtts->GetMinFlag());
            connect(minToggle, SIGNAL(toggled(bool)),
                    this, SLOT(minToggled(bool)));
            break;
          case TensorAttributes::ID_maxFlag:
            // Disconnect the slot before setting the toggle and
            // reconnect it after. This prevents multiple updates.
            disconnect(maxToggle, SIGNAL(toggled(bool)),
                       this, SLOT(maxToggled(bool)));
            maxToggle->setChecked(tensorAtts->GetMaxFlag());
            maxLineEdit->setEnabled(tensorAtts->GetMaxFlag());
            connect(maxToggle, SIGNAL(toggled(bool)),
                    this, SLOT(maxToggled(bool)));
           break;
          case TensorAttributes::ID_min:
            minLineEdit->setText(DoubleToQString(tensorAtts->GetMin()));
            break;
          case TensorAttributes::ID_max:
            maxLineEdit->setText(DoubleToQString(tensorAtts->GetMax()));
            break;

          case TensorAttributes::ID_colorByEigenValues:
            colorButtonGroup->blockSignals(true);
            colorButtonGroup->button(tensorAtts->GetColorByEigenValues() ? 0 : 1)->setChecked(true);
            colorButtonGroup->blockSignals(false);
//            limitsGroup->setEnabled(tensorAtts->GetColorByEigenValues());
            break;
          case TensorAttributes::ID_tensorColor:
            { // new scope
            QColor temp(tensorAtts->GetTensorColor().Red(),
                        tensorAtts->GetTensorColor().Green(),
                        tensorAtts->GetTensorColor().Blue());
            tensorColor->blockSignals(true);
            tensorColor->setButtonColor(temp);
            tensorColor->blockSignals(false);
            }
          case TensorAttributes::ID_colorTableName:
            colorTableWidget->setColorTable(tensorAtts->GetColorTableName().c_str());
            break;
          case TensorAttributes::ID_invertColorTable:
            colorTableWidget->setInvertColorTable(tensorAtts->GetInvertColorTable());
            break;
          case TensorAttributes::ID_useLegend:
            legendToggle->blockSignals(true);
            legendToggle->setChecked(tensorAtts->GetUseLegend());
            legendToggle->blockSignals(false);
            break;

          case TensorAttributes::ID_scale:
            scaleLineEdit->setText(DoubleToQString(tensorAtts->GetScale()));
            break;
          case TensorAttributes::ID_scaleByMagnitude:
            scaleByMagnitudeToggle->blockSignals(true);
            scaleByMagnitudeToggle->setChecked(tensorAtts->GetScaleByMagnitude());
            scaleByMagnitudeToggle->blockSignals(false);
            break;
          case TensorAttributes::ID_autoScale:
            autoScaleToggle->blockSignals(true);
            autoScaleToggle->setChecked(tensorAtts->GetAutoScale());
            autoScaleToggle->blockSignals(false);
            break;
        }
    } // end for
}


// ****************************************************************************
// Method: QvisTensorPlotWindow::GetCurrentValues
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
QvisTensorPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do the N tensors value.
    if(which_widget == TensorAttributes::ID_nTensors || doAll)
    {
        int val;
        if(LineEditGetInt(nTensorsLineEdit, val))
            tensorAtts->SetNTensors(val);
        else
        {
            ResettingError(tr("number of tensors"),
                IntToQString(tensorAtts->GetNTensors()));
            tensorAtts->SetNTensors(tensorAtts->GetNTensors());
        }
    }

    // Do the stride value.
    if(which_widget == TensorAttributes::ID_stride || doAll)
    {
        int val;
        if(LineEditGetInt(strideLineEdit, val))
            tensorAtts->SetStride(val);
        else
        {
            ResettingError(tr("stride"), 
                IntToQString(tensorAtts->GetStride()));
            tensorAtts->SetStride(tensorAtts->GetStride());
        }
    }

    // Do the minimum value.
    if(which_widget == TensorAttributes::ID_min || doAll)
    {
        double val;
        if(LineEditGetDouble(minLineEdit, val))
            tensorAtts->SetMin(val);
        else
        {
            ResettingError(tr("minimum value"),
                DoubleToQString(tensorAtts->GetMin()));
            tensorAtts->SetMin(tensorAtts->GetMin());
        }
    }

    // Do the maximum value
    if(which_widget == TensorAttributes::ID_max || doAll)
    {
        double val;
        if(LineEditGetDouble(maxLineEdit, val))
            tensorAtts->SetMax(val);
        else
        {
            ResettingError(tr("maximum value"),
                DoubleToQString(tensorAtts->GetMax()));
            tensorAtts->SetMax(tensorAtts->GetMax());
        }
    }

    // Do the scale value.
    if(which_widget == TensorAttributes::ID_scale || doAll)
    {
        double val;
        if(LineEditGetDouble(scaleLineEdit, val))
            tensorAtts->SetScale(val);
        else
        {
            ResettingError(tr("scale value"),
                DoubleToQString(tensorAtts->GetScale()));
            tensorAtts->SetScale(tensorAtts->GetScale());
        }
    }

}

// ****************************************************************************
// Method: QvisTensorPlotWindow::Apply
//
// Purpose: 
//   This method applies the tensor attributes and optionally tells the viewer
//   to apply them to the plot.
//
// Arguments:
//   ignore : This flag, when true, tells the code to ignore the
//            AutoUpdate function and tell the viewer to apply the
//            tensor attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 22 23:52:51 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisTensorPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Get the current aslice attributes and tell the other
        // observers about them.
        GetCurrentValues(-1);
        tensorAtts->Notify();

        // Tell the viewer to set the tensor attributes.
        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        tensorAtts->Notify();
}

//
// Qt Slot functions...
//

void
QvisTensorPlotWindow::apply()
{
    Apply(true);
}

void
QvisTensorPlotWindow::makeDefault()
{
    // Tell the viewer to set the default tensor attributes.
    GetCurrentValues(-1);
    tensorAtts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}

void
QvisTensorPlotWindow::reset()
{
    // Tell the viewer to reset the aslice attributes to the last
    // applied values.
    GetViewerMethods()->ResetPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisTensorPlotWindow::tensorColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   tensor color.
//
// Arguments:
//   color : The new tensor color.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:21:58 PDT 2001
//
// Modifications:
//   Brad Whitlock, Sat Jun 16 19:19:30 PST 2001
//   Added code to disable coloration by tensor magnitude.
//
// ****************************************************************************

void
QvisTensorPlotWindow::tensorColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    tensorAtts->SetTensorColor(temp);
    tensorAtts->SetColorByEigenValues(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::processScaleText
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
QvisTensorPlotWindow::processScaleText()
{
    GetCurrentValues(TensorAttributes::ID_scale);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::scaleByMagnitudeToggled
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
QvisTensorPlotWindow::scaleByMagnitudeToggled(bool)
{
    tensorAtts->SetScaleByMagnitude(!tensorAtts->GetScaleByMagnitude());
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::autoScaleToggled
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
QvisTensorPlotWindow::autoScaleToggled(bool)
{
    tensorAtts->SetAutoScale(!tensorAtts->GetAutoScale());
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::reduceMethodChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   method used to reduce the number of tensors.
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
QvisTensorPlotWindow::reduceMethodChanged(int index)
{
    tensorAtts->SetUseStride(index != 0);
    Apply();   
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::locationMethodChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   method used to place the tensors.
//
// Arguments:
//   index : The location method.
//
// Programmer: Hank Childs
// Creation:   August 24, 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisTensorPlotWindow::locationMethodChanged(int index)
{
    tensorAtts->SetGlyphLocation(index == 0 
                                   ? TensorAttributes::AdaptsToMeshResolution
                                   : TensorAttributes::UniformInSpace);
    if (tensorAtts->GetGlyphLocation() == TensorAttributes::UniformInSpace)
        tensorAtts->SetUseStride(false);
    Apply();   
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::processNTensorsText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   N tensors line edit.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 12:22:33 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisTensorPlotWindow::processNTensorsText()
{
    GetCurrentValues(TensorAttributes::ID_nTensors);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::processStrideText
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
QvisTensorPlotWindow::processStrideText()
{
    GetCurrentValues(TensorAttributes::ID_stride);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::limitToOrigToggled
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
QvisTensorPlotWindow::limitToOrigToggled(bool val)
{
    tensorAtts->SetOrigOnly(val);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::legendToggled
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
QvisTensorPlotWindow::legendToggled(bool)
{
    tensorAtts->SetUseLegend(!tensorAtts->GetUseLegend());
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::colorByMagnitudeToggled
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
QvisTensorPlotWindow::colorModeChanged(int index)
{
    tensorAtts->SetColorByEigenValues(index == 0);
//    limitsGroup->setEnabled(tensorAtts->GetColorByEigenValues());
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::colorTableClicked
//
// Purpose: 
//   This is a Qt slot function that sets the desired color table name into
//   the tensor plot attributes.
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
QvisTensorPlotWindow::colorTableClicked(bool useDefault,
    const QString &ctName)
{
    tensorAtts->SetColorByEigenValues(true);
    tensorAtts->SetColorTableName(ctName.toStdString());
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::invertColorTableToggled
//
// Purpose: 
//   This is a Qt slot function that sets the invert color table flag into the
//   tensor plot attributes.
//
// Arguments:
//   val    :  Whether or not to invert the color table.
//
// Programmer: Kathleen Bonnell
// Creation:   January  17, 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisTensorPlotWindow::invertColorTableToggled(bool val)
{
    tensorAtts->SetInvertColorTable(val);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::limitsSelectChanged
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
QvisTensorPlotWindow::limitsSelectChanged(int mode)
{
    // Only do it if it changed.
    if(mode != tensorAtts->GetLimitsMode())
    {
        tensorAtts->SetLimitsMode(TensorAttributes::LimitsMode(mode));
        Apply();
    }
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::processMinLimitText
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
QvisTensorPlotWindow::processMinLimitText()
{
    GetCurrentValues(TensorAttributes::ID_min);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::processMaxLimitText
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
QvisTensorPlotWindow::processMaxLimitText()
{
    GetCurrentValues(TensorAttributes::ID_max);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::minToggled
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
QvisTensorPlotWindow::minToggled(bool val)
{
    tensorAtts->SetMinFlag(val);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::maxToggled
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
QvisTensorPlotWindow::maxToggled(bool val)
{
    tensorAtts->SetMaxFlag(val);
    Apply();
}
