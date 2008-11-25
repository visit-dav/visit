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

#include <QvisMovieProgressDialog.h>
#include <QLayout>
#include <QPixmap>
#include <QPushButton>

#include <icons/moviereel.xpm>

// Window flags to make the dialog modal.
#define DIALOG_FLAGS (WType_Modal | WStyle_Customize | WStyle_NormalBorder | WStyle_Title)

// ****************************************************************************
// Method: QvisMovieProgressDialog::QvisMovieProgressDialog
//
// Purpose: 
//   Constructor for the QvisMovieProgressDialog class.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The name of the widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 20 16:27:19 PST 2005
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

QvisMovieProgressDialog::QvisMovieProgressDialog(QWidget *parent) 
: QDialog(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setMargin(10);
    topLayout->setSpacing(5);

    QHBoxLayout *labelLayout = new QHBoxLayout();
    topLayout->addLayout(labelLayout);
    labelLayout->setMargin(5);
    QPixmap moviereel(moviereel_xpm);
    picture = new QLabel(this);
    picture->setPixmap(moviereel);
    picture->setMinimumWidth(moviereel.width());
    picture->setMinimumHeight(moviereel.height());
    labelLayout->addWidget(picture);

    labelTextLabel = new QLabel(this);
    labelTextLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelTextLabel->setMinimumWidth(2 * moviereel.width());
    labelLayout->addWidget(labelTextLabel);

    progressBar = new QProgressBar(this);
    progressBar->setRange(0,100);
    progressBar->setValue(0);
    topLayout->addWidget(progressBar);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    topLayout->addLayout(btnLayout);
    btnLayout->addStretch(10);
    cancelButton = new QPushButton(tr("Cancel"), this);
    connect(cancelButton, SIGNAL(clicked()),
            this, SLOT(cancelClicked()));
    btnLayout->addWidget(cancelButton);
}

// ****************************************************************************
// Method: QvisMovieProgressDialog::~QvisMovieProgressDialog
//
// Purpose: 
//   Destructor for the QvisMovieProgressDialog class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 20 16:28:01 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisMovieProgressDialog::~QvisMovieProgressDialog()
{
}

// ****************************************************************************
// Method: QvisMovieProgressDialog::setProgress
//
// Purpose: 
//   Sets the progress for the dialog's progress bar. 
//
// Arguments:
//   val : The new value for the progress bar.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 20 16:28:18 PST 2005
//
// Modifications:
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisMovieProgressDialog::setProgress(int val)
{
    progressBar->setValue(val);

    // If we've reached the target, hide the dialog.
    if(val == progressBar->maximum())
        hide();
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisMovieProgressDialog::cancelClicked
//
// Purpose: 
//   This is a Qt slot function that is called when the dialog's Cancel button
//   is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 20 16:28:59 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisMovieProgressDialog::cancelClicked()
{
    reject();
    emit cancelled();
}
