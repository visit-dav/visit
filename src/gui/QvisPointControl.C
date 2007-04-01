#include <QvisPointControl.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <QvisVariableButton.h>

// ****************************************************************************
// Method: QvisPointControl::QvisPointControl
//
// Purpose: This is the constructor for the QvisPointControl class. 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//    Brad Whitlock, Thu Dec 9 17:05:12 PST 2004
//    I replaced one of the line edits with a QvisVariableButton.
//
// ****************************************************************************

QvisPointControl::QvisPointControl(QWidget *parent, const char *name) :
    QWidget(parent, name)
{
    // Set some default values.

    lastGoodSize = 0.05;
    lastGoodVar = "default";

    // Create the top layout.
    QGridLayout *topLayout = new QGridLayout(this, 3, 4);
    topLayout->setSpacing(10);

    QString temp;
    // Create the size label and line edit.
    sizeLineEdit = new QLineEdit(this, "sizeLineEdit");
    connect(sizeLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processSizeText()));
    topLayout->addMultiCellWidget(sizeLineEdit, 0, 0, 1, 3);
    sizeLabel = new QLabel(sizeLineEdit, "Point size", this, 
                                "sizeLabel");
    topLayout->addWidget(sizeLabel, 0, 0);

    // Create the size variable check box and variable button.
    sizeVarButton = new QvisVariableButton(true, true, true,
        QvisVariableButton::Scalars, this, "sizeVarButton");
    sizeVarButton->setEnabled(false);
    connect(sizeVarButton, SIGNAL(activated(const QString &)),
            this, SLOT(sizeVarChanged(const QString &)));
    topLayout->addMultiCellWidget(sizeVarButton, 1, 1, 2, 3);
    sizeVarToggle = new QCheckBox("Scale point size by variable", this, 
                                       "sizeVarToggle");
    connect(sizeVarToggle, SIGNAL(toggled(bool)),
            this, SLOT(sizeVarToggled(bool)));
    topLayout->addMultiCellWidget(sizeVarToggle, 1,1, 0,1);

    // Create the type buttons
    typeButtons = new QButtonGroup(0, "typeButtons");
    connect(typeButtons, SIGNAL(clicked(int)),
            this, SLOT(typeButtonChanged(int)));
    QGridLayout *typeLayout = new QGridLayout(1, 5);
    typeLayout->setSpacing(10);
    typeLayout->setColStretch(4, 1000);
    typeLayout->addWidget(new QLabel("Point Type", this), 0,0);
    QRadioButton *rb = new QRadioButton("Box", this, "Box");
    typeButtons->insert(rb);
    typeLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton("Axis", this, "Axis");
    typeButtons->insert(rb);
    typeLayout->addWidget(rb, 0, 2);
    rb = new QRadioButton("Icosahedron", this, "Icosahedron");
    typeButtons->insert(rb);
    typeLayout->addWidget(rb, 0, 3);
    rb = new QRadioButton("Point", this, "Point");
    typeButtons->insert(rb);
    typeLayout->addWidget(rb, 0, 4);
    topLayout->addMultiCellLayout(typeLayout, 2,2 , 0,3);

    SetPointSize(lastGoodSize);
    SetPointSizeVar(lastGoodVar);
}


// ****************************************************************************
// Method: QvisPointControl
//
// Purpose: This is the destructor for the QvisPointControl class.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   
// ****************************************************************************

QvisPointControl::~QvisPointControl()
{
    // nothing here.
}


// ****************************************************************************
// Method: QvisPointControl::processSizeText
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   size text and presses the Enter key.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
QvisPointControl::processSizeText()
{
    QString temp = sizeLineEdit->displayText().stripWhiteSpace();
    bool okay = !temp.isEmpty();
    double val;
    if (okay)
    {
        val = temp.toDouble(&okay);
    }
    if (okay)
    {
        lastGoodSize = val;
        if (!signalsBlocked())
            emit pointSizeChanged(val);
    }
    else 
    {
        SetPointSize(lastGoodSize);
    }
}


// ****************************************************************************
// Method: QvisPointControl::GetPointSize
//
// Purpose: 
//   This is a method to retrieve the value contained in the pointSizeLineEdit.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   
// ****************************************************************************

double
QvisPointControl::GetPointSize() 
{
    blockSignals(true);
    processSizeText();
    blockSignals(false);
    return lastGoodSize;
}


// ****************************************************************************
// Method: QvisPointControl::sizeVarChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the
//   size var text and pressed the Enter key.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:09:50 PST 2004
//   I renamed the method and adapted it to work with a variable button.
//
// ****************************************************************************

void
QvisPointControl::sizeVarChanged(const QString &var)
{
    if (var != lastGoodVar)
    {
        lastGoodVar = var;
        if (!signalsBlocked())
            emit pointSizeVarChanged(var);
    }
}


// ****************************************************************************
// Method: QvisPointControl::GetPointSizeVar
//
// Purpose: 
//   This is a method to retrieve the value contained in the 
//   pointSizeVarLineEdit.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:12:12 PST 2004
//   I changed the method to work with a variable button.
//
// ****************************************************************************

QString  &
QvisPointControl::GetPointSizeVar() 
{
    return lastGoodVar;
}

// ****************************************************************************
//  Method:  QvisPointControl::pointTypeChanged
//
//  Purpose:
//    Qt slot function that is called when one of the point type buttons
//    is clicked.
//
//  Arguments:
//    type   :   The new type
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 4, 2004 
//
//  Modifications:
//    Brad Whitlock, Thu Dec 9 17:12:39 PST 2004
//    I changed the name of one of the widgets.
//
// ****************************************************************************

void
QvisPointControl::typeButtonChanged(int type)
{
    sizeLabel->setEnabled(type != 3);
    sizeLineEdit->setEnabled(type != 3);
    sizeVarToggle->setEnabled(type != 3);
    sizeVarButton->setEnabled(type != 3);
    if (!signalsBlocked())
        emit pointTypeChanged(type);
}


// ****************************************************************************
// Method: QvisPointControl::sizeVarToggled
//
// Purpose: 
//   This is a Qt slot function that emits the pointSizeVarToggled signal.
//
// Arguments:
//   val : The new state of the sizeVar toggle.
//
// Programmer: Kathleen Bonnell
// Creation:   November 4, 2004 
// 
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:13:06 PST 2004
//   I changed the name of one of the widgets.
//
// ****************************************************************************

void
QvisPointControl::sizeVarToggled(bool val)
{
    sizeVarButton->setEnabled(val);
    if (!signalsBlocked())
        emit pointSizeVarToggled(val);
}


// ****************************************************************************
// Method: QvisPointControl::SetSize
//
// Purpose: 
//   This method sets the value in the sizeLineEdit.
//
// Arguments:
//   val : The size.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   
// ****************************************************************************

void QvisPointControl::SetPointSize(double val)
{
    QString temp;
    temp.sprintf("%g", val);
    sizeLineEdit->setText(temp);
    lastGoodSize = val;
}


// ****************************************************************************
// Method: QvisPointControl::SetSizeVarChecked
//
// Purpose: 
//   This method sets the sizeVar toggle. 
//
// Arguments:
//   checked : The value for the toggle 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:13:40 PST 2004
//   I changed the name of one of the widgets.
//
// ****************************************************************************

void QvisPointControl::SetPointSizeVarChecked(bool checked)
{
    sizeVarToggle->blockSignals(true);
    sizeVarToggle->setChecked(checked);
    sizeVarToggle->blockSignals(false);
    sizeVarButton->setEnabled(checked);
}


// ****************************************************************************
// Method: QvisPointControl::GetSizeVarChecked
//
// Purpose: 
//   Returns the state of the sizeVarToggle. 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   
// ****************************************************************************

bool
QvisPointControl::GetPointSizeVarChecked() const
{
    return sizeVarToggle->isChecked();
}


// ****************************************************************************
// Method: QvisPointControl::SetPointSizeVar
//
// Purpose: 
//   This method sets the value of the sizeVarLineEdit.
//
// Arguments:
//   var : The value for the sizeVarLineEdit 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:14:17 PST 2004
//   I changed the code so it works with a variable button.
//
// ****************************************************************************

void QvisPointControl::SetPointSizeVar(QString &var)
{
    sizeVarButton->setText(var);
    lastGoodVar = var;
}


// ****************************************************************************
// Method: QvisPointControl::SetPointType
//
// Purpose: 
//   This method sets the point type radio buttons. 
//
// Arguments:
//   type : Which button should be active.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   Brad Whitlock, Thu Dec 9 17:14:45 PST 2004
//   I changed the name of one of the widgets.
//
// ****************************************************************************

void QvisPointControl::SetPointType(int type)
{
    if (type < 0 || type > 3)
        return;

    typeButtons->blockSignals(true);
    typeButtons->setButton(type);
    typeButtons->blockSignals(false);
    sizeLabel->setEnabled(type != 3);
    sizeLineEdit->setEnabled(type != 3);
    sizeVarToggle->setEnabled(type != 3);
    sizeVarButton->setEnabled(type != 3);
    if (!signalsBlocked())
        emit pointTypeChanged(type);
}

// ****************************************************************************
// Method: QvisPointControl::GetPointType
//
// Purpose: 
//   Returns the state of the Point type radio button (which one is checked). 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 4, 2004 
//
// Modifications:
//   Brad Whitlock, Wed Dec 15 11:12:52 PDT 2004
//   Made it work with older versions of Qt.
//
// ****************************************************************************

int
QvisPointControl::GetPointType() const
{
#if QT_VERSION >= 0x030200
    return typeButtons->selectedId();
#else
    return typeButtons->id(typeButtons->selected());
#endif
}

