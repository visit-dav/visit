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
#include <QvisSessionSourceChangerDialog.h>
#include <qapplication.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qvbox.h>

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
// ****************************************************************************

QvisSessionSourceChangerDialog::QvisSessionSourceChangerDialog(
    QWidget *parent, const char *name) : QDialog(parent, name)
{
    QString title = tr("Update sources");
    QString description = tr("Make sure that the sources used in "
        "this session are up to date. You can change the sources here to "
        "restore your session using different sources, making your session "
        "file a helpful visualization template.");
    setCaption(title);

    QVBoxLayout *pageLayout = new QVBoxLayout(this);
    pageLayout->setMargin(10);
    pageLayout->setSpacing(10);
    QLabel *prompt = new QLabel(SplitPrompt(description), this, "prompt");
    pageLayout->addWidget(prompt);
    pageLayout->addSpacing(10);

    body = new QvisSessionSourceChanger(this, "body");
    body->setMinimumHeight(300);
    body->setMinimumWidth(500);
    pageLayout->addWidget(body);

    QHBoxLayout *buttonLayout = new QHBoxLayout(pageLayout);
    buttonLayout->setSpacing(10);
    buttonLayout->addStretch(10);
    QPushButton *ok = new QPushButton( tr( "&OK" ), this, "ok" );
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    buttonLayout->addWidget(ok);
    QPushButton *cancel = new QPushButton( tr( "&Cancel" ), this, "cancel" );
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
//   
// ****************************************************************************

void
QvisSessionSourceChangerDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->state() == 0 || (e->state() & Keypad && e->key() == Key_Enter))
    {
        if(e->key() == Key_Enter || e->key() == Key_Return)
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
        stringVector words(SplitValues(std::string(s.latin1()), ' '));
        QString r;
        int len = 0;
        for(size_t i = 0; i < words.size(); ++i)
        {
            if(len > 0)
                r += " ";
            r += QString(words[i].c_str());
            len += words[i].size();
            if(len >= MAX_PROMPT_LENGTH)
            {
                r += "\n";
                len = 0;
            }
        }
        return r;
    }
}
