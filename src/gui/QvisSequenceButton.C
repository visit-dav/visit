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
    
#include <QvisSequenceButton.h>
#include <QAction>
#include <QMenu>
#include <QPixmap>

#include <MovieSequenceFactory.h>

// ****************************************************************************
// Method: QvisSequenceButton::QvisSequenceButton
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 2 18:09:33 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Tue Oct  7 09:06:17 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisSequenceButton::QvisSequenceButton(QWidget *parent) : QPushButton(parent)
{
    setText(tr("New sequence"));

    // 
    // Create the menu based on the contents of the MovieSequenceFactory
    // class factory.
    //
    menu = new QMenu(0);
    QMenu *transitions = new QMenu(tr("Compositing"), menu);
    QMenu *compositing = new QMenu(tr("Transitions"), menu);
    QMenu *rotations = new QMenu(tr("Rotations"), menu);
    menu->addMenu(compositing);
    menu->addMenu(transitions);
    menu->addMenu(rotations);

    MovieSequenceFactory *f = MovieSequenceFactory::Instance();
    for(int index = 0; index < f->NumSequenceTypes(); ++index)
    {
        int id;
        if(f->SequenceIdForIndex(index, id) &&
           f->SequenceProvidesMenu(id))
        {
            QMenu *m = menu;
            int mIndex = -1;
            f->SequenceSubMenuIndex(id, mIndex);
            if(mIndex == 0)
                m = transitions;
            else if(mIndex == 1)
                m = compositing;
            else if(mIndex == 2)
                m = rotations;

            QString menuName;
            if(f->SequenceMenuName(id, menuName))
            {
                QPixmap pix;
                f->SequencePixmap(id, pix);
                QAction *action = 0;

                if(!pix.isNull())
                    action = m->addAction(QIcon(pix), menuName);
                else
                    action = m->addAction(menuName);

                action->setData(QVariant(id));
            }
        }
    }

    setMenu(menu);
    connect(menu, SIGNAL(triggered(QAction*)),
            this, SLOT(emitActivated(QAction*)));
}

// ****************************************************************************
// Method: QvisSequenceButton::~QvisSequenceButton
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 2 18:09:49 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisSequenceButton::~QvisSequenceButton()
{
    delete menu;
}

// ****************************************************************************
// Method: QvisSequenceButton::emitActivated
//
// Purpose: 
//   Translates a QAction activation to an activated signal that passes
//   along the action's actual sequence Id.
//
// Arguments:
//   action : The action that was triggered.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct  7 09:19:41 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisSequenceButton::emitActivated(QAction *action)
{
    int id = action->data().toInt();
    emit activated(id);
}
