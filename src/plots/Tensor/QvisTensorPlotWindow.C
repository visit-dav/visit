#include <QvisTensorPlotWindow.h>
#include <qlayout.h> 
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>

#include <TensorAttributes.h>
#include <ViewerProxy.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisColorButton.h>
#include <QvisColorTableButton.h>

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
    TensorAttributes *_vecAtts, const char *caption, const char *shortName,
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
// ****************************************************************************

QvisTensorPlotWindow::~QvisTensorPlotWindow()
{
    tensorAtts = 0;

    // Delete widgets with no parents.
    delete reduceButtonGroup;
    delete colorButtonGroup;
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
// ****************************************************************************

void
QvisTensorPlotWindow::CreateWindowContents()
{
    QGridLayout *gLayout = new QGridLayout(topLayout, 4, 4);
    gLayout->setSpacing(10);

    //
    // Create the color-related widgets.
    //
    colorGroupBox = new QGroupBox(central, "colorGroupBox");
    colorGroupBox->setTitle("Tensor color");
    gLayout->addMultiCellWidget(colorGroupBox, 0, 0, 0, 3);
    QVBoxLayout *cgTopLayout = new QVBoxLayout(colorGroupBox);
    cgTopLayout->setMargin(10);
    cgTopLayout->addSpacing(15);
    QGridLayout *cgLayout = new QGridLayout(cgTopLayout, 1, 2);
    cgLayout->setSpacing(10);
    cgLayout->setColStretch(1, 10);

    // Add the tensor color label.
    colorButtonGroup = new QButtonGroup(0, "colorModeButtons");
    connect(colorButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(colorModeChanged(int)));
    QRadioButton *rb = new QRadioButton("Eigenvalues", colorGroupBox, "Eigenvalues");
    colorButtonGroup->insert(rb, 0);
    cgLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton("Constant", colorGroupBox, "constant");
    rb->setChecked(true);
    colorButtonGroup->insert(rb, 1);
    cgLayout->addWidget(rb, 1, 0);

    // Create the color-by-eigenvalues button.
    colorTableButton = new QvisColorTableButton(colorGroupBox, "colorTableButton");
    connect(colorTableButton, SIGNAL(selectedColorTable(bool, const QString &)),
            this, SLOT(colorTableClicked(bool, const QString &)));
    cgLayout->addWidget(colorTableButton, 0, 1, AlignLeft | AlignVCenter);

    // Create the tensor color button.
    tensorColor = new QvisColorButton(colorGroupBox, "tensorColorButton");
    tensorColor->setButtonColor(QColor(255, 0, 0));
    connect(tensorColor, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(tensorColorChanged(const QColor &)));
    cgLayout->addWidget(tensorColor, 1, 1, AlignLeft | AlignVCenter);

    //
    // Create the scale-related widgets.
    //
    scaleGroupBox = new QGroupBox(central, "scaleGroupBox");
    scaleGroupBox->setTitle("Tensor scale");
    gLayout->addMultiCellWidget(scaleGroupBox, 1, 1, 0, 3);
    QVBoxLayout *sgTopLayout = new QVBoxLayout(scaleGroupBox);
    sgTopLayout->setMargin(10);
    sgTopLayout->addSpacing(15);
    QGridLayout *sgLayout = new QGridLayout(sgTopLayout, 2, 2);
    sgLayout->setSpacing(10);
    sgLayout->setColStretch(1, 10);

    // Add the scale line edit.
    scaleLineEdit = new QLineEdit(scaleGroupBox, "scaleLineEdit");
    connect(scaleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processScaleText()));
    sgLayout->addWidget(scaleLineEdit, 0, 1);
    QLabel *scaleLabel = new QLabel(scaleLineEdit, "Scale", scaleGroupBox, "scaleLabel");
    sgLayout->addWidget(scaleLabel, 0, 0, AlignRight | AlignVCenter);

    //
    // Create the reduce-related widgets.
    //
    reduceGroupBox = new QGroupBox(central, "reduceGroupBox");
    reduceGroupBox->setTitle("Reduce by");
    gLayout->addMultiCellWidget(reduceGroupBox, 2, 2, 0, 3);
    QVBoxLayout *rgTopLayout = new QVBoxLayout(reduceGroupBox);
    rgTopLayout->setMargin(10);
    rgTopLayout->addSpacing(15);
    QGridLayout *rgLayout = new QGridLayout(rgTopLayout, 2, 2);
    rgLayout->setSpacing(10);
    rgLayout->setColStretch(1, 10);

    // Create the reduce button group.
    reduceButtonGroup = new QButtonGroup(0, "reduceButtonGroup");
    connect(reduceButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(reduceMethodChanged(int)));
    rb= new QRadioButton("N tensors", reduceGroupBox);
    rb->setChecked(true);
    reduceButtonGroup->insert(rb, 0);
    rgLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton("Stride", reduceGroupBox);
    reduceButtonGroup->insert(rb, 1);
    rgLayout->addWidget(rb, 1, 0);

    // Add the N tensors line edit.
    nTensorsLineEdit = new QLineEdit(reduceGroupBox, "nTensorsLineEdit");
    connect(scaleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processNTensorsText()));
    rgLayout->addWidget(nTensorsLineEdit, 0, 1);

    // Add the stride line edit.
    strideLineEdit = new QLineEdit(reduceGroupBox, "strideLineEdit");
    connect(strideLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processStrideText()));
    rgLayout->addWidget(strideLineEdit, 1, 1);

    //
    // Add the toggle buttons
    //

    // Add the legend toggle button.
    legendToggle = new QCheckBox("Legend", central, "legendToggle");
    connect(legendToggle, SIGNAL(clicked()), this, SLOT(legendToggled()));
    gLayout->addMultiCellWidget(legendToggle, 3, 3, 0, 1);
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
// ****************************************************************************

void
QvisTensorPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;

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
        case 0: // useStride
            reduceButtonGroup->blockSignals(true);
            reduceButtonGroup->setButton(tensorAtts->GetUseStride()?1:0);
            reduceButtonGroup->blockSignals(false);

            nTensorsLineEdit->setEnabled(!tensorAtts->GetUseStride());
            strideLineEdit->setEnabled(tensorAtts->GetUseStride());
            break;
        case 1: // stride
            temp.sprintf("%d", tensorAtts->GetStride());
            strideLineEdit->setText(temp);
            break;
        case 2: // nTensors
            temp.sprintf("%d", tensorAtts->GetNTensors());
            nTensorsLineEdit->setText(temp);
            break;
        case 3: // scale
            temp.setNum(tensorAtts->GetScale());
            scaleLineEdit->setText(temp);
            break;
        case 4: // colorByEigenvalues
            colorButtonGroup->blockSignals(true);
            colorButtonGroup->setButton(tensorAtts->GetColorByEigenvalues() ? 0 : 1);
            colorButtonGroup->blockSignals(false);
            break;
        case 5: // useLegend
            legendToggle->blockSignals(true);
            legendToggle->setChecked(tensorAtts->GetUseLegend());
            legendToggle->blockSignals(false);
            break;
        case 6: // tensorColor
            { // new scope
            QColor temp(tensorAtts->GetTensorColor().Red(),
                        tensorAtts->GetTensorColor().Green(),
                        tensorAtts->GetTensorColor().Blue());
            tensorColor->blockSignals(true);
            tensorColor->setButtonColor(temp);
            tensorColor->blockSignals(false);
            }
        case 7: // colorTableName
            colorTableButton->setColorTable(tensorAtts->GetColorTableName().c_str());
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
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::GetCurrentValues(int which_widget)
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
            tensorAtts->SetScale(val);
        }

        if(!okay)
        {
            msg.sprintf("The scale value was invalid. "
                "Resetting to the last good value of %g.",
                tensorAtts->GetScale());
            Message(msg);
            tensorAtts->SetScale(tensorAtts->GetScale());
        }
    }

    // Do the N tensors value.
    if(which_widget == 1 || doAll)
    {
        temp = nTensorsLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            tensorAtts->SetNTensors(val);
        }

        if(!okay)
        {
            msg.sprintf("The head size was invalid. "
                "Resetting to the last good value of %d.",
                tensorAtts->GetNTensors());
            Message(msg);
            tensorAtts->SetNTensors(tensorAtts->GetNTensors());
        }
    }

    // Do the stride value.
    if(which_widget == 2 || doAll)
    {
        temp = strideLineEdit->displayText().stripWhiteSpace();
        okay = !temp.isEmpty();
        if(okay)
        {
            int val = temp.toInt(&okay);
            tensorAtts->SetStride(val);
        }

        if(!okay)
        {
            msg.sprintf("The stride was invalid. "
                "Resetting to the last good value of %d.",
                tensorAtts->GetStride());
            Message(msg);
            tensorAtts->SetStride(tensorAtts->GetStride());
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
// Programmer: Hank Childs
// Creation:   September 23, 2003
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
        viewer->SetPlotOptions(plotType);
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
    viewer->SetDefaultPlotOptions(plotType);
}

void
QvisTensorPlotWindow::reset()
{
    // Tell the viewer to reset the aslice attributes to the last
    // applied values.
    viewer->ResetPlotOptions(plotType);
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
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::tensorColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    tensorAtts->SetTensorColor(temp);
    tensorAtts->SetColorByEigenvalues(false);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::processScaleText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   scale line edit.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::processScaleText()
{
    GetCurrentValues(0);
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
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::reduceMethodChanged(int index)
{
    tensorAtts->SetUseStride(index != 0);
    Apply();   
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::processNTensorsText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   N tensors line edit.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::processNTensorsText()
{
    GetCurrentValues(1);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::processStrideText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   stride line edit.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::processStrideText()
{
    GetCurrentValues(2);
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::legendToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's legend toggle button.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::legendToggled()
{
    tensorAtts->SetUseLegend(!tensorAtts->GetUseLegend());
    Apply();
}

// ****************************************************************************
// Method: QvisTensorPlotWindow::colorByEigenvaluesToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the user toggles the
//   window's "color by eigenvalues" toggle button.
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::colorModeChanged(int index)
{
    tensorAtts->SetColorByEigenvalues(index == 0);
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
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// ****************************************************************************

void
QvisTensorPlotWindow::colorTableClicked(bool useDefault,
    const QString &ctName)
{
    tensorAtts->SetColorByEigenvalues(true);
    tensorAtts->SetColorTableName(ctName.latin1());
    Apply();
}
