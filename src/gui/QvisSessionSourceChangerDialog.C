// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisSessionSourceChangerDialog.h>
#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>
#include <QKeyEvent>

#include <Utility.h>

// ****************************************************************************
// Method: QvisSessionSourceChangerDialog::QvisSessionSourceChangerDialog
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   parent : The widget's parent.
//   name   : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 15:24:49 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//   
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

QvisSessionSourceChangerDialog::QvisSessionSourceChangerDialog(QWidget *parent) 
: QDialog(parent)
{
    QString title = tr("Update sources");
    QString description = tr("Make sure that the sources used in "
        "this session are up to date. You can change the sources here to "
        "restore your session using different sources, making your session "
        "file a helpful visualization template.");
    setWindowTitle(title);

    QVBoxLayout *pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(10,10,10,10);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(description), this);
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    body = new QvisSessionSourceChanger(this);
    body->setMinimumHeight(300);
    body->setMinimumWidth(500);
    pageLayout->addWidget(body);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    pageLayout->addLayout(buttonLayout);
    buttonLayout->setSpacing(10);
    buttonLayout->addStretch(10);
    QPushButton *ok = new QPushButton( tr( "&OK" ), this);
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    buttonLayout->addWidget(ok);
    QPushButton *cancel = new QPushButton( tr( "&Cancel" ), this);
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    buttonLayout->addWidget(cancel);
}

// ****************************************************************************
// Method: QvisSessionSourceChangerDialog::~QvisSessionSourceChangerDialog
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 15:25:16 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisSessionSourceChangerDialog::~QvisSessionSourceChangerDialog()
{
}

// ****************************************************************************
// Method: QvisSessionSourceChangerDialog::keyPressEvent
//
// Purpose: 
//   This event handling method intercepts Enter and Return key presses for
//   the dialog and prevents QDialog::keyPressEvent from getting them in
//   certain cases because QDialog would make the active pushbutton fire, which
//   is probably not what we want.
//
// Arguments:
//   e : The key event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 14 16:35:28 PST 2006
//
// Modifications:
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisSessionSourceChangerDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->modifiers() == Qt::NoModifier || 
       (e->modifiers() & Qt::KeypadModifier && e->key() == Qt::Key_Enter))
    {
        if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
        {
            if(qApp->focusWidget() != 0 &&
               qApp->focusWidget()->inherits("QButton"))
            {
                QDialog::keyPressEvent(e);
            }
            else
            {
                e->ignore();
            }
        }
        else
            QDialog::keyPressEvent(e);
    }
    else
        QDialog::keyPressEvent(e);
}

//
// Set/Get methods.
//

void
QvisSessionSourceChangerDialog::setSources(const stringVector &keys,
    const stringVector &values, const std::map<std::string, stringVector> &uses)
{
    body->setSources(keys, values, uses);
}

const stringVector &
QvisSessionSourceChangerDialog::getSources() const
{
    return body->getSources();
}

// ****************************************************************************
// Method: QvisSessionSourceChangerDialog::SplitPrompt
//
// Purpose: 
//   Splits a prompt into words and reassembles it into a paragraph of text 
//   where each line has a max length.
//
// Arguments:
//   s : The string to split.
//
// Returns:    The processed string.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 23 11:26:59 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QString
QvisSessionSourceChangerDialog::SplitPrompt(const QString &s) const
{
    static const int MAX_PROMPT_LENGTH = 60;
    if(s.length() < MAX_PROMPT_LENGTH)
        return s;
    else
    {
        stringVector words(SplitValues(std::string(s.toStdString()), ' '));
        QString r;
        size_t len = 0;
        for(size_t i = 0; i < words.size(); ++i)
        {
            if(len > 0)
                r += " ";
            r += QString(words[i].c_str());
            len += words[i].size();
            if(len >= (size_t)MAX_PROMPT_LENGTH)
            {
                r += "\n";
                len = 0;
            }
        }
        return r;
    }
}


// ****************************************************************************
// Method: QvisSessionSourceChangerDialog::setFallbackPath
//
// Purpose: 
//   Sets a fallbackpath to in the FileOpen dialog if the current source
//   directory listed in the session file no longer exists.
//
// Arguments:
//   path : The path to use
//
// Programmer: Kathleen Bonnell 
// Creation:   Fri May 13 14:27:49 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisSessionSourceChangerDialog::setFallbackPath(const QString &path)
{
    if (body != 0)
        body->setFallbackPath(path);
}
