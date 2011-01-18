/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
    topLayout->setMargin(0);
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

