// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisColorTableWidget.h>
#include <QvisColorTableButton.h>
#include <QCheckBox>
#include <QGridLayout>



// ****************************************************************************
// Method: QvisColorTableWidget::QvisColorTableWidget
//
// Purpose: 
//   Constructor for the QvisColorTableWidget class.
//
// Arguments:
//   parent : The parent widget.
//   name   : The name to associate with this widget.
//
// Programmer: Kathleen Bonnell 
// Creation:   January 17, 2011
//
// Modifications:
//
// ****************************************************************************

QvisColorTableWidget::QvisColorTableWidget(QWidget *parent, 
     bool showInvertButton) : QWidget(parent)
{

    QGridLayout *topLayout = new QGridLayout(this);
    topLayout->setContentsMargins(0,0,0,0);
    topLayout->setSpacing(10);

    colorTableButton = new QvisColorTableButton(this);
    colorTableButton->move(0, 0);
    colorTableButton->resize(colorTableButton->sizeHint());
    connect(colorTableButton, 
            SIGNAL(selectedColorTable (bool, const QString &)),
            this, SLOT(handleSelectedColorTable(bool, const QString &)));

    topLayout->addWidget(colorTableButton, 0, 0);
    invertToggle = new QCheckBox(tr("Invert"), this);
    invertToggle->move(0, colorTableButton->sizeHint().height());
    invertToggle->resize(colorTableButton->sizeHint().width(),
                         invertToggle->sizeHint().height());
    connect(invertToggle, SIGNAL(toggled(bool)),
            this, SLOT(invertToggled(bool)));

    topLayout->addWidget(invertToggle, 0, 1);
    if (!showInvertButton)
    {
        invertToggle->hide();
        invertToggle->setEnabled(false);
    }
}

// ****************************************************************************
// Method: QvisColorTableWidget::~QvisColorTableWidget
//
// Purpose: 
//   This is the destructor for the QvisColorTableWidget class.
//
// Programmer: Kathleen Bonnell 
// Creation:   January 17, 2011
//
// Modifications:
//
// ****************************************************************************

QvisColorTableWidget::~QvisColorTableWidget()
{
    // nothing here
}


// ****************************************************************************
// Method: QvisColorTableWidget::sizeHint
//
// Purpose: 
//   Returns the widget's preferred size.
//
// Returns:    The widget's preferred size.
//
// Programmer: Kathleen Bonnell 
// Creation:   January 17, 2011
//
// Modifications:
//   
// ****************************************************************************

QSize
QvisColorTableWidget::sizeHint() const
{
    QSize s1 = colorTableButton->sizeHint();
    QSize s2 = invertToggle->sizeHint();

    // Find the largest width.
    int maxWidth = s1.width();
    if (s2.width() > maxWidth)
        maxWidth = s2.width();
   
    return QSize(maxWidth, s1.height() + s2.height());
}


// ****************************************************************************
// Method: QvisColorTableWidget::setColorTable
//
// Purpose: 
//   Tells the widget to use a specified color table.
//
// Arguments:
//   ctName : The name of the color table to use.
//
// Programmer: Kathleen Bonnell 
// Creation:   January 17, 2011
//
// Modifications:
//
// ****************************************************************************
void
QvisColorTableWidget::setColorTable(const QString &ctName)
{
    colorTableButton->setColorTable(ctName);
}

void
QvisColorTableWidget::setInvertColorTable(bool val)
{
    invertToggle->setChecked(val);
}


//
// Qt slot functions.
//

// ****************************************************************************
// Method: QvisColorTableWidget::handleSelectedColorTable
//
// Purpose: 
//   This is a Qt slot function that is called when a color table has been
//   selected from the popup menu. The widget then emits a selectedColorTable
//   signal.
//
// Arguments:
//   val
//   def
//
// Programmer: Kathleen Bonnell
// Creation:   January 17, 2011
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWidget::handleSelectedColorTable(bool useDefault, 
    const QString &ctName)
{
    emit  selectedColorTable(useDefault, ctName);
}

// ****************************************************************************
// Method: QvisColorTableWidget::invertToggled
//
// Purpose:
//   This is a Qt slot function that is called when the invert button is 
//   toggled.  The widget then emits an invertColorTableToggled signal.
//
// Arguments:
//   val : The new state of the button
//
// Programmer: Kathleen Bonnell 
// Creation:   January 17, 2011
//
// Modifications:
// 
// ****************************************************************************

void
QvisColorTableWidget::invertToggled(bool val)
{
    emit  invertColorTableToggled(val);
}

