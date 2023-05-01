// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "VisItChangeUsernameWindow.h"

#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

// ****************************************************************************
//  Constructor:  VisItChangeUsernameWindow::VisItChangeUsernameWindow
//
//  Programmer:  Hank Childs
//  Creation:    November 11, 2007
//
//  Modifications:
//    Brad Whitlock, Tue Apr 29 11:58:00 PDT 2008
//    Added tr()'s
//
//    Brad Whitlock, Fri May 23 10:47:52 PDT 2008
//    Qt 4.
//
// ****************************************************************************

VisItChangeUsernameWindow::VisItChangeUsernameWindow(QWidget *parent) : 
    QDialog(parent)
{
    setModal(true);

    setWindowTitle(tr("Choose new username"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10,10,10,10);

    QHBoxLayout *l2 = new QHBoxLayout;
    layout->addLayout(l2);
    l2->setSpacing(5);
    label = new QLabel(tr("Username for localhost: "), this);
    l2->addWidget(label);

    usernameedit = new QLineEdit(this);
    l2->addWidget(usernameedit);
    connect(usernameedit, SIGNAL(returnPressed()), this, SLOT(accept()));
    layout->addSpacing(20);

    QHBoxLayout *l3 = new QHBoxLayout;
    layout->addLayout(l3);
    QPushButton *okay = new QPushButton(tr("Confirm username"), this);
    connect(okay, SIGNAL(clicked()), this, SLOT(accept()));
    l3->addWidget(okay);
    l3->addStretch(10);

    QPushButton *cancel = new QPushButton(tr("Cancel"), this);
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    l3->addWidget(cancel);
}

// ****************************************************************************
//  Destructor:  VisItChangeUsernameWindow::~VisItChangeUsernameWindow
//
//  Programmer:  Hank Childs
//  Creation:    November 11, 2007
//
// ****************************************************************************

VisItChangeUsernameWindow::~VisItChangeUsernameWindow()
{
}

// ****************************************************************************
// Method: VisItChangeUsernameWindow::getUsername
//
// Purpose: 
//   Retrieve the username from the widget.
//
// Returns:    Returns the username name if this has been instantiated, 
//             NULL otherwise. 
//
// Programmer: Kathleen Bonnell 
// Creation:   February 13, 2008 
//
// ****************************************************************************

QString
VisItChangeUsernameWindow::getUsername(const QString &host, 
    VisItChangeUsernameWindow::ReturnCode &ret)
{
    // Set the username prompt.
    QString labelText(tr("New username for %1: ").arg(host));
    this->label->setText(labelText);

    // Make the username window be the active window.
    this->activateWindow();
    this->raise();

    // Clear the username.
    this->usernameedit->clear();
    this->usernameedit->setText("");

    // Give focus to the username window.
    QTimer::singleShot(300, this->usernameedit, SLOT(setFocus()));

    // Enter the local event loop for the dialog.
    int status = this->exec();

    QString username;
    if (status == Accepted)
    {
        ret = UW_Accepted;
        username = this->usernameedit->text();
    }
    else
        ret = UW_Rejected;

    return username;
}
