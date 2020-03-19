// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
    
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
