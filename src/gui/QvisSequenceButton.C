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
    
#include <QvisSequenceButton.h>
#include <qpixmap.h>
#include <qpopupmenu.h>

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
//   
// ****************************************************************************

QvisSequenceButton::QvisSequenceButton(QWidget *parent, 
    const char *name) : QPushButton(parent, name)
{
    setText("New sequence");

    // 
    // Create the menu based on the contents of the MovieSequenceFactory
    // class factory.
    //
    menu = new QPopupMenu(0, "menu");
    QPopupMenu *transitions = new QPopupMenu(menu, "transitions");
    QPopupMenu *compositing = new QPopupMenu(menu, "compositing");
    QPopupMenu *rotations = new QPopupMenu(menu, "rotations");
    menu->insertItem("Compositing", compositing);
    menu->insertItem("Transitions", transitions);
    menu->insertItem("Rotations", rotations);

    MovieSequenceFactory *f = MovieSequenceFactory::Instance();
    for(int index = 0; index < f->NumSequenceTypes(); ++index)
    {
        int id;
        if(f->SequenceIdForIndex(index, id) &&
           f->SequenceProvidesMenu(id))
        {
            QPopupMenu *m = menu;
            int mIndex = -1;
            f->SequenceSubMenuIndex(id, mIndex);
            if(mIndex == 0)
                m = transitions;
            else if(mIndex == 1)
                m = compositing;
            else if(mIndex == 2)
                m = rotations;

            std::string menuName;
            if(f->SequenceMenuName(id, menuName))
            {
                QString mName(menuName.c_str());
                QPixmap pix;
                f->SequencePixmap(id, pix);

                if(!pix.isNull())
                {
                    QIconSet icon;
                    icon.setPixmap(pix, QIconSet::Small);
                    m->insertItem(icon, mName, id);
                }
                else
                    m->insertItem(mName, id);
            }
        }
    }

    setPopup(menu);
    connect(menu, SIGNAL(activated(int)),
            this, SIGNAL(activated(int)));
    connect(transitions, SIGNAL(activated(int)),
            this, SIGNAL(activated(int)));
    connect(compositing, SIGNAL(activated(int)),
            this, SIGNAL(activated(int)));
    connect(rotations, SIGNAL(activated(int)),
            this, SIGNAL(activated(int)));
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
