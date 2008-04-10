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
#include <QvisSessionSourceChanger.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>

#include <QvisDialogLineEdit.h>

// ****************************************************************************
// Method: QvisSessionSourceChanger::QvisSessionSourceChanger
//
// Purpose: 
//   Constructor
//
// Arguments:
//   parent : The parent widget.
//   name   : The name of the widget.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:41:28 PST 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

QvisSessionSourceChanger::QvisSessionSourceChanger(QWidget *parent,
    const char *name) : QFrame(parent, name),
    sourceIds(), sources(), sourceUses()
{
    setFrameStyle(QFrame::NoFrame);
    QVBoxLayout *frameinnerLayout = new QVBoxLayout(this);
    frameinnerLayout->setMargin(10);
    frameinnerLayout->addSpacing(10);

    QVBoxLayout *pageLayout = new QVBoxLayout(frameinnerLayout);
    pageLayout->setSpacing(10);
    QGridLayout *gridLayout = new QGridLayout(pageLayout, 2, 2);
    gridLayout->setSpacing(5);

    // Create the source list.
    sourceList = new QListBox(this, "sourceList");
    connect(sourceList, SIGNAL(selectionChanged()),
            this, SLOT(selectedSourceChanged()));
    gridLayout->addWidget(new QLabel(tr("Source identifiers"), this, "sourceIdLabel"), 0, 0);
    gridLayout->addWidget(sourceList, 1, 0);

    QGroupBox *sourceProperties = new QGroupBox(tr("Source"), this,
        "sourceProperties");
    gridLayout->addMultiCellWidget(sourceProperties, 0, 1, 1, 1);
    gridLayout->setColStretch(1, 10);
    QVBoxLayout *srcPropLayout = new QVBoxLayout(sourceProperties);
    srcPropLayout->setMargin(10);
    srcPropLayout->setSpacing(5);
    srcPropLayout->addSpacing(10);

    // Create the source property controls.
    fileLineEdit = new QvisDialogLineEdit(sourceProperties, "fileLineEdit");
    fileLineEdit->setDialogMode(QvisDialogLineEdit::ChooseFile);
    connect(fileLineEdit, SIGNAL(returnPressed()),
            this, SLOT(sourceChanged()));
    connect(fileLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(sourceChanged(const QString &)));
    srcPropLayout->addWidget(fileLineEdit);

    useList = new QListBox(sourceProperties, "useList");
    srcPropLayout->addWidget(new QLabel(tr("Source used in"), sourceProperties, "srcUseLabel"));
    srcPropLayout->addWidget(useList);
}

// ****************************************************************************
// Method: QvisSessionSourceChanger::~QvisSessionSourceChanger
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:41:57 PST 2006
//
// Modifications:
//   
// ****************************************************************************

QvisSessionSourceChanger::~QvisSessionSourceChanger()
{
}

// ****************************************************************************
// Method: QvisSessionSourceChanger::setSources
//
// Purpose: 
//   Set the sources that will be displayed in this widget.
//
// Arguments:
//   keys   : The source ids.
//   values : The source names.
//   uses   : Where the sources are used.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:42:13 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSessionSourceChanger::setSources(const stringVector &keys, 
    const stringVector &values,
    const std::map<std::string, stringVector> &uses)
{
    sourceIds = keys;
    sources = values;
    sourceUses = uses;

    // Update the UI.
    sourceList->blockSignals(true);
    sourceList->clear();
    for(int i = 0; i < keys.size(); ++i)
        sourceList->insertItem(keys[i].c_str());
    sourceList->setSelected(0, true);
    sourceList->blockSignals(false);

    updateControls(0);
}

// ****************************************************************************
// Method: QvisSessionSourceChanger::updateControls
//
// Purpose: 
//   Updates the controls when we get a new list of sources.
//
// Arguments:
//   ci : The index of the new active source.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:42:58 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisSessionSourceChanger::updateControls(int ci)
{
    useList->blockSignals(true);
    fileLineEdit->blockSignals(true);

    useList->clear();

    if(ci >= 0 && ci < sources.size())
    {
        if(sources.size() > 0)
            fileLineEdit->setText(sources[ci].c_str());
        else
            fileLineEdit->setText("");

        if(sourceIds.size() > 0)
        {
            std::map<std::string, stringVector>::const_iterator pos = 
                sourceUses.find(sourceIds[ci]);
            if(pos != sourceUses.end())
            {
                for(int i = 0; i < pos->second.size(); ++i)
                    useList->insertItem(pos->second[i].c_str());        
                useList->setSelected(ci, true);
            }
        }
    }
    else
    {
        fileLineEdit->setText("");
    }

    fileLineEdit->blockSignals(false);
    useList->blockSignals(false);
}

// ****************************************************************************
// Method: QvisSessionSourceChanger::getSources
//
// Purpose: 
//   Returns a reference to the sources.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 13 15:43:30 PST 2006
//
// Modifications:
//   
// ****************************************************************************

const stringVector &
QvisSessionSourceChanger::getSources() const
{
    return sources;
}

//
// Qt slot functions
//

void
QvisSessionSourceChanger::selectedSourceChanged()
{
    updateControls(sourceList->currentItem());
}

void
QvisSessionSourceChanger::sourceChanged()
{
    // Get the active source index.
    int ci = sourceList->currentItem();
    if(ci >= 0)
        sources[ci] = std::string(fileLineEdit->text().latin1());
}

void
QvisSessionSourceChanger::sourceChanged(const QString &s)
{
    // Get the active source index.
    int ci = sourceList->currentItem();
    if(ci >= 0)
        sources[ci] = std::string(s.latin1());
}

