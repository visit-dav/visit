#include <QvisVectorPlotWindow.h>
#include <qlayout.h> 
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qhbox.h>

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
// ****************************************************************************

void
QvisVectorPlotWindow::CreateWindowContents()
{
    QGridLayout *gLayout = new QGridLayout(topLayout, 6, 4);
    gLayout->setSpacing(10);

    // Create the lineStyle widget.
    lineStyle = new QvisLineStyleWidget(0, central, "lineStyle");
    gLayout->addWidget(lineStyle, 0, 1);
    connect(lineStyle, SIGNAL(lineStyleChanged(int)),
            this, SLOT(lineStyleChanged(int)));
    QLabel *lineStyleLabel = new QLabel(lineStyle, "Line style",
                                        central, "lineStyleLabel");
    gLayout->addWidget(lineStyleLabel, 0, 0);

    // Create the lineWidth widget.
    lineWidth = new QvisLineWidthWidget(0, central, "lineWidth");
    gLayout->addWidget(lineWidth, 0, 3);
    connect(lineWidth, SIGNAL(lineWidthChanged(int)),
            this, SLOT(lineWidthChanged(int)));
    QLabel *lineWidthLabel = new QLabel(lineWidth, "Line width",
                                        central, "lineWidthLabel");
    gLayout->addWidget(lineWidthLabel, 0, 2);

    //
    // Create the color-related widgets.
    //
    colorGroupBox = new QGroupBox(central, "colorGroupBox");
    colorGroupBox->setTitle("Vector color");
    gLayout->addMultiCellWidget(colorGroupBox, 1, 1, 0, 3);
    QVBoxLayout *cgTopLayout = new QVBoxLayout(colorGroupBox);
    cgTopLayout->setMargin(10);
    cgTopLayout->addSpacing(15);
    QGridLayout *cgLayout = new QGridLayout(cgTopLayout, 2, 2);
    cgLayout->setSpacing(10);
    cgLayout->setColStretch(1, 10);

    // Add the vector color label.
    colorButtonGroup = new QButtonGroup(0, "colorModeButtons");
    connect(colorButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(colorModeChanged(int)));
    QRadioButton *rb = new QRadioButton("Magnitude", colorGroupBox, "Magnitude");
    colorButtonGroup->insert(rb, 0);
    cgLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton("Constant", colorGroupBox, "constant");
    rb->setChecked(true);
    colorButtonGroup->insert(rb, 1);
    cgLayout->addWidget(rb, 1, 0);

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
    cgLayout->addWidget(vectorColor, 1, 1, AlignLeft | AlignVCenter);

    //
    // Create the scale-related widgets.
    //
    scaleGroupBox = new QGroupBox(central, "scaleGroupBox");
    scaleGroupBox->setTitle("Vector scale");
    gLayout->addMultiCellWidget(scaleGroupBox, 2, 2, 0, 3);
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

    // Add the head size edit.
    headSizeLineEdit = new QLineEdit(scaleGroupBox, "headSizeLineEdit");
    connect(headSizeLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processHeadSizeText()));
    sgLayout->addWidget(headSizeLineEdit, 1, 1);
    QLabel *headSizeLabel = new QLabel(headSizeLineEdit, "Head size",
                                       scaleGroupBox, "headSizeLabel");
    sgLayout->addWidget(headSizeLabel, 1, 0, AlignRight | AlignVCenter);

    //
    // Create the reduce-related widgets.
    //
    reduceGroupBox = new QGroupBox(central, "reduceGroupBox");
    reduceGroupBox->setTitle("Reduce by");
    gLayout->addMultiCellWidget(reduceGroupBox, 3, 3, 0, 3);
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
    rb= new QRadioButton("N vectors", reduceGroupBox);
    rb->setChecked(true);
    reduceButtonGroup->insert(rb, 0);
    rgLayout->addWidget(rb, 0, 0);
    rb = new QRadioButton("Stride", reduceGroupBox);
    reduceButtonGroup->insert(rb, 1);
    rgLayout->addWidget(rb, 1, 0);

    // Add the N vectors line edit.
    nVectorsLineEdit = new QLineEdit(reduceGroupBox, "nVectorsLineEdit");
    connect(scaleLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processNVectorsText()));
    rgLayout->addWidget(nVectorsLineEdit, 0, 1);

    // Add the stride line edit.
    strideLineEdit = new QLineEdit(reduceGroupBox, "strideLineEdit");
    connect(strideLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processStrideText()));
    rgLayout->addWidget(strideLineEdit, 1, 1);

    //
    // Create the radio buttons to choose the glyph origin
    //
    originButtonGroup = new QButtonGroup(0, "originButtonGroup");
    QHBox *originBox = new QHBox(central, "originBox");
    originBox->setSpacing(10);
    new QLabel("Vector origin", originBox, "originLabel");
    connect(originButtonGroup, SIGNAL(clicked(int)),
            this, SLOT(originTypeChanged(int)));
    rb = new QRadioButton("Head", originBox);
    originButtonGroup->insert(rb,0);
    rb = new QRadioButton("Middle", originBox);
    originButtonGroup->insert(rb,1);
    rb = new QRadioButton("Tail", originBox);
    originButtonGroup->insert(rb,2);
    gLayout->addMultiCellWidget(originBox, 4, 4, 0, 3);

    //
    // Add the toggle buttons
    //

    // Add the legend toggle button.
    legendToggle = new QCheckBox("Legend", central, "legendToggle");
    connect(legendToggle, SIGNAL(clicked()), this, SLOT(legendToggled()));
    gLayout->addMultiCellWidget(legendToggle, 5, 5, 0, 1);

    // Add the "draw head" toggle button.
    drawHeadToggle = new QCheckBox("Draw head", central, "drawHeadToggle");
    connect(drawHeadToggle, SIGNAL(clicked()), this, SLOT(drawHeadToggled()));
    gLayout->addMultiCellWidget(drawHeadToggle, 5, 5, 2, 3);
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
            temp.sprintf("%g", vectorAtts->GetScale());
            scaleLineEdit->setText(temp);
            break;
        case 6: // headSize
            temp.sprintf("%g", vectorAtts->GetHeadSize());
            headSizeLineEdit->setText(temp);
            break;
        case 7: // headOn
            drawHeadToggle->blockSignals(true);
            drawHeadToggle->setChecked(vectorAtts->GetHeadOn());
            drawHeadToggle->blockSignals(false);
            break;
        case 8: // colorByMag
            colorButtonGroup->blockSignals(true);
            colorButtonGroup->setButton(vectorAtts->GetColorByMag() ? 0 : 1);
            colorButtonGroup->blockSignals(false);
            break;
        case 9: // useLegend
            legendToggle->blockSignals(true);
            legendToggle->setChecked(vectorAtts->GetUseLegend());
            legendToggle->blockSignals(false);
            break;
        case 10: // vectorColor
            { // new scope
            QColor temp(vectorAtts->GetVectorColor().Red(),
                        vectorAtts->GetVectorColor().Green(),
                        vectorAtts->GetVectorColor().Blue());
            vectorColor->blockSignals(true);
            vectorColor->setButtonColor(temp);
            vectorColor->blockSignals(false);
            }
        case 11: // colorTableName
            colorTableButton->setColorTable(vectorAtts->GetColorTableName().c_str());
            break;
        case 12: // vectorOrigin
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
            vectorAtts->SetScale(val);
        }

        if(!okay)
        {
            msg.sprintf("The scale value was invalid. "
                "Resetting to the last good value of %g.",
                vectorAtts->GetScale());
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
            vectorAtts->SetHeadSize(val);
        }

        if(!okay)
        {
            msg.sprintf("The head size was invalid. "
                "Resetting to the last good value of %g.",
                vectorAtts->GetHeadSize());
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
            vectorAtts->SetNVectors(val);
        }

        if(!okay)
        {
            msg.sprintf("The head size was invalid. "
                "Resetting to the last good value of %d.",
                vectorAtts->GetNVectors());
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
            vectorAtts->SetStride(val);
        }

        if(!okay)
        {
            msg.sprintf("The stride was invalid. "
                "Resetting to the last good value of %d.",
                vectorAtts->GetStride());
            Message(msg);
            vectorAtts->SetStride(vectorAtts->GetStride());
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
        viewer->SetPlotOptions(plotType);
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
    viewer->SetDefaultPlotOptions(plotType);
}

void
QvisVectorPlotWindow::reset()
{
    // Tell the viewer to reset the aslice attributes to the last
    // applied values.
    viewer->ResetPlotOptions(plotType);
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
//   
// ****************************************************************************

void
QvisVectorPlotWindow::colorModeChanged(int index)
{
    vectorAtts->SetColorByMag(index == 0);
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
