#include <QvisMovieProgressDialog.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpushbutton.h>

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
//   
// ****************************************************************************

QvisMovieProgressDialog::QvisMovieProgressDialog(QWidget *parent,
    const char *name) : QDialog(parent, name)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setMargin(10);
    topLayout->setSpacing(5);

    QHBoxLayout *labelLayout = new QHBoxLayout(topLayout);
    labelLayout->setMargin(5);
    QPixmap moviereel(moviereel_xpm);
    picture = new QLabel(this, "picture");
    picture->setPixmap(moviereel);
    picture->setMinimumWidth(moviereel.width());
    picture->setMinimumHeight(moviereel.height());
    labelLayout->addWidget(picture);

    labelTextLabel = new QLabel(this, "labelText");
    labelTextLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    labelTextLabel->setMinimumWidth(2 * moviereel.width());
    labelLayout->addWidget(labelTextLabel);

    progressBar = new QProgressBar(100, this, "progress");
    progressBar->setProgress(0);
    topLayout->addWidget(progressBar);

    QHBoxLayout *btnLayout = new QHBoxLayout(topLayout);
    btnLayout->addStretch(10);
    cancelButton = new QPushButton("Cancel", this, "cancel");
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
//   
// ****************************************************************************

void
QvisMovieProgressDialog::setProgress(int val)
{
    progressBar->setProgress(val);

    // If we've reached the target, hide the dialog.
    if(val == progressBar->totalSteps())
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
