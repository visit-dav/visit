// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisMovieProgressDialog.h>
#include <QLayout>
#include <QPixmap>
#include <QPushButton>

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

QvisMovieProgressDialog::QvisMovieProgressDialog(QPixmap &pix, QWidget *parent) 
: QDialog(parent)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(10,10,10,10);
    topLayout->setSpacing(5);

    QHBoxLayout *labelLayout = new QHBoxLayout();
    topLayout->addLayout(labelLayout);
    labelLayout->setContentsMargins(5,5,5,5);
    picture = new QLabel(this);
    picture->setPixmap(pix);
    picture->setMinimumWidth(pix.width());
    picture->setMinimumHeight(pix.height());
    labelLayout->addWidget(picture);

    labelTextLabel = new QLabel(this);
    labelTextLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelTextLabel->setMinimumWidth(2 * pix.width());
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
