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

#include <QvisPointControl.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QvisVariableButton.h>

#define POINT_TYPE_POINTS 3
#define POINT_TYPE_SPHERE 4

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
//    Brad Whitlock, Wed Jul 20 13:48:43 PST 2005
//    Initialize lastGoodSizePixels.
//
//    Brad Whitlock, Thu Aug 25 09:31:41 PDT 2005
//    I replaced the buttongroup with a combobox.
//
//    Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//    Support for internationalization.
//
//   Cyrus Harrison, Tue Jul  8 09:58:45 PDT 2008
//   Initial Qt4 Port
//
// ****************************************************************************

QvisPointControl::QvisPointControl(QWidget *parent) :
    QWidget(parent)
{
    // Set some default values.

    lastGoodPointType = 0;
    lastGoodSize = 0.05;
    lastGoodSizePixels = 1;
    lastGoodVar = "default";

    // Create the top layout.
    QGridLayout *topLayout = new QGridLayout(this);
    topLayout->setMargin(0);
    topLayout->setSpacing(10);

    QString temp;
    // Create the size label and line edit.
    sizeLineEdit = new QLineEdit(this);
    connect(sizeLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processSizeText()));
    topLayout->addWidget(sizeLineEdit, 0, 1, 1, 2);
    sizeLabel = new QLabel(tr("Point size"), this);
    sizeLabel->setBuddy(sizeLabel);
    topLayout->addWidget(sizeLabel, 0, 0);

    // Create the size variable check box and variable button.
    sizeVarToggle = new QCheckBox(tr("Scale point size by variable"), this);
    connect(sizeVarToggle, SIGNAL(toggled(bool)),
            this, SLOT(sizeVarToggled(bool)));
    topLayout->addWidget(sizeVarToggle, 1, 0, 1, 2);
    sizeVarButton = new QvisVariableButton(true, true, true,
        QvisVariableButton::Scalars, this);
    sizeVarButton->setEnabled(false);
    connect(sizeVarButton, SIGNAL(activated(const QString &)),
            this, SLOT(sizeVarChanged(const QString &)));
    topLayout->addWidget(sizeVarButton, 1, 2);

    // Create the type combo box
    typeComboBox = new QComboBox(this);
    typeComboBox->addItem(tr("Box"));
    typeComboBox->addItem(tr("Axis"));
    typeComboBox->addItem(tr("Icosahedron"));
    typeComboBox->addItem(tr("Point"));
    typeComboBox->addItem(tr("Sphere"));
    connect(typeComboBox, SIGNAL(activated(int)),
            this, SLOT(typeComboBoxChanged(int)));
    topLayout->addWidget(new QLabel(tr("Point Type"), this), 2, 0);
    topLayout->addWidget(typeComboBox, 2, 1, 1, 2);

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
//   Brad Whitlock, Wed Jul 20 13:53:17 PST 2005
//   Made it use ProcessSizeText.
//
//   Brad Whitlock, Thu Aug 25 09:52:13 PDT 2005
//   Added support for sphere points.
//
// ****************************************************************************

void
QvisPointControl::processSizeText()
{
    if(ProcessSizeText(lastGoodPointType))
    {
        if(!signalsBlocked())
        {
            if(lastGoodPointType == POINT_TYPE_POINTS ||
               lastGoodPointType == POINT_TYPE_SPHERE)
            {
                emit pointSizePixelsChanged(lastGoodSizePixels);
            }
            else
                emit pointSizeChanged(lastGoodSize);
        }
    }
}

// ****************************************************************************
// Method: QvisPointControl::ProcessSizeText
//
// Purpose: 
//   Processes the text in the size text field based on the point type.
//
// Arguments:
//   pointType : The point type to use when interpreting the size text.
//
// Returns:    True if the text was processed okay.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 20 14:49:19 PST 2005
//
// Modifications:
//   Brad Whitlock, Thu Aug 25 09:51:31 PDT 2005
//   Added support for sphere points.
//
// ****************************************************************************

bool
QvisPointControl::ProcessSizeText(int pointType)
{
    QString temp = sizeLineEdit->displayText().trimmed();
    bool okay = !temp.isEmpty();

    if(pointType == POINT_TYPE_POINTS ||
       pointType == POINT_TYPE_SPHERE)
    {
        int val;
        if (okay)
            val = temp.toInt(&okay);
        if (okay)
            lastGoodSizePixels = val;
        else
            SetPointSizePixels(lastGoodSizePixels);
    }
    else
    {
        double val;
        if (okay)
            val = temp.toDouble(&okay);
        if (okay)
            lastGoodSize = val;
        else
            SetPointSize(lastGoodSize);
    }

    return okay;
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
    if(lastGoodPointType < POINT_TYPE_POINTS)
    {
        blockSignals(true);
        ProcessSizeText(lastGoodPointType);
        blockSignals(false);
    }
    return lastGoodSize;
}

// ****************************************************************************
// Method: QvisPointControl::GetPointSizePixels
//
// Purpose: 
//   Gets the point size in terms of pixels.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 20 14:49:00 PST 2005
//
// Modifications:
//   Brad Whitlock, Thu Aug 25 09:49:50 PDT 2005
//   Added support for sphere points.
//
// ****************************************************************************

int
QvisPointControl::GetPointSizePixels() 
{
    if(lastGoodPointType == POINT_TYPE_POINTS ||
       lastGoodPointType == POINT_TYPE_SPHERE)
    {
        blockSignals(true);
        ProcessSizeText(lastGoodPointType);
        blockSignals(false);
    }
    return lastGoodSizePixels;
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
//  Method:  QvisPointControl::typeComboBoxChanged
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
//    Brad Whitlock, Wed Jul 20 14:09:29 PST 2005
//    Added code to get the point size.
//
// ****************************************************************************

void
QvisPointControl::typeComboBoxChanged(int type)
{
    // Get the values that are in the text field.
    ProcessSizeText(lastGoodPointType);

    lastGoodPointType = type;

    UpdateSizeText();
    UpdatePointType();

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
    lastGoodSize = val;
    UpdateSizeText();
}

// ****************************************************************************
// Method: QvisPointControl::SetPointSizePixels
//
// Purpose: 
//   Sets the point size in terms of pixels.
//
// Arguments:
//   val : The new pixel size.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 20 14:47:26 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisPointControl::SetPointSizePixels(int val)
{
    lastGoodSizePixels = val;
    UpdateSizeText();
}

// ****************************************************************************
// Method: QvisPointControl::UpdateSizeText
//
// Purpose: 
//   Updates the size text.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 20 14:48:01 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisPointControl::UpdateSizeText()
{
    if(lastGoodPointType < POINT_TYPE_POINTS)
    {
        QString temp;
        temp.sprintf("%g", lastGoodSize);
        sizeLineEdit->setText(temp);
    }
    else
    {
        QString temp;
        temp.sprintf("%d", lastGoodSizePixels);
        sizeLineEdit->setText(temp);
    }
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
//   Brad Whitlock, Wed Jul 20 14:13:43 PST 2005
//   I made it set the lastGoodPointType and update the text.
//
//   Brad Whitlock, Thu Aug 25 09:57:49 PDT 2005
//   Changed to a combobox.
//
//   Cyrus Harrison, Tue Jul  8 09:58:45 PDT 2008
//   Initial Qt4 Port
//
//   Hank Childs, Tue Dec 23 17:34:19 PST 2008
//   Change limit from >3 to >4, since 4 now corresponds to Sphere.
//
// ****************************************************************************

void QvisPointControl::SetPointType(int type)
{
    if (type < 0 || type > 4)
        return;

    typeComboBox->blockSignals(true);
    typeComboBox->setCurrentIndex(type);
    typeComboBox->blockSignals(false);

    lastGoodPointType = type;
    UpdateSizeText();
    UpdatePointType();

    if (!signalsBlocked())
        emit pointTypeChanged(type);
}

// ****************************************************************************
// Method: QvisPointControl::UpdatePointType
//
// Purpose: 
//   Updates some point widgets based on the point type.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 20 14:48:35 PST 2005
//
// Modifications:
//   Kathleen Bonnell, Mon Jul 25 17:27:52 PDT 2005
//   Make enabled state of sizeVarButton depend on checked state of
//   sizeVarToggle.
//
//   Brad Whitlock, Thu Aug 25 09:58:49 PDT 2005
//   Added support for sphere points.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
QvisPointControl::UpdatePointType()
{
    bool e = false;
    if(lastGoodPointType != POINT_TYPE_POINTS &&
       lastGoodPointType != POINT_TYPE_SPHERE)
    {
        sizeLabel->setText(tr("Point size"));
        e = true;
    }
    else
        sizeLabel->setText(tr("Point size (pixels)"));

    sizeVarToggle->setEnabled(e);
    sizeVarButton->setEnabled(sizeVarToggle->isChecked());
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
//   Brad Whitlock, Thu Aug 25 10:03:41 PDT 2005
//   Changed to a combobox.
//
//   Cyrus Harrison, Tue Jul  8 09:58:45 PDT 2008
//   Initial Qt4 Port
//
// ****************************************************************************

int
QvisPointControl::GetPointType() const
{
    return typeComboBox->currentIndex();
}

