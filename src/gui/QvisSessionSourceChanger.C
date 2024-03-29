// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisSessionSourceChanger.h>

#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QListWidget>

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
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

QvisSessionSourceChanger::QvisSessionSourceChanger(QWidget *parent) 
: QWidget(parent),
    sourceIds(), sources(), sourceUses()
{
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(5);

    // Create the source list.
    sourceList = new QListWidget(this);
    connect(sourceList, SIGNAL(itemSelectionChanged()),
            this, SLOT(selectedSourceChanged()));
    gridLayout->addWidget(new QLabel(tr("Source identifiers"), this), 0, 0);
    gridLayout->addWidget(sourceList, 1, 0);

    QGroupBox *sourceProperties = new QGroupBox(tr("Source"), this);
    gridLayout->addWidget(sourceProperties, 0, 1, 2, 1);
    gridLayout->setColumnStretch(1, 10);
    
    QVBoxLayout *srcPropLayout = new QVBoxLayout(sourceProperties);
    srcPropLayout->setContentsMargins(10,10,10,10);
    srcPropLayout->setSpacing(5);

    // Create the source property controls.
    fileLineEdit = new QvisDialogLineEdit(sourceProperties);
    fileLineEdit->setDialogMode(QvisDialogLineEdit::ChooseFile);
    connect(fileLineEdit, SIGNAL(returnPressed()),
            this, SLOT(sourceChanged()));
    connect(fileLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(sourceChanged(const QString &)));
    srcPropLayout->addWidget(fileLineEdit);

    useList = new QListWidget(sourceProperties);
    srcPropLayout->addWidget(new QLabel(tr("Source used in"), sourceProperties));
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
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
//   Brad Whitlock, Thu Nov 11 12:14:31 PST 2010
//   Account for zero items.
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
    for(size_t i = 0; i < keys.size(); ++i)
        sourceList->addItem(keys[i].c_str());
    if(sourceList->count() > 0)
        sourceList->item(0)->setSelected(true);
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
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//
//   Kathleen Bonnell, Fri Jun 18 15:13:22 MST 2010 
//   Set useLists' zeroth item to selected, instead of ci'th. Prevents
//   a segv in some instances.
//
// ****************************************************************************

void
QvisSessionSourceChanger::updateControls(int ci)
{
    useList->blockSignals(true);
    fileLineEdit->blockSignals(true);

    useList->clear();

    if(ci >= 0 && (size_t)ci < sources.size())
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
                for(size_t i = 0; i < pos->second.size(); ++i)
                    useList->addItem(pos->second[i].c_str());        
                useList->item(0)->setSelected(true);
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
// Modifications:
//   Cyrus Harrison, Tue Jul  1 09:14:16 PDT 2008
//   Initial Qt4 Port.
//

void
QvisSessionSourceChanger::selectedSourceChanged()
{
    updateControls(sourceList->currentRow());
}

void
QvisSessionSourceChanger::sourceChanged()
{
    // Get the active source index.
    int ci = sourceList->currentRow();
    if(ci >= 0)
        sources[ci] = std::string(fileLineEdit->text().toStdString());
}

void
QvisSessionSourceChanger::sourceChanged(const QString &s)
{
    // Get the active source index.
    int ci = sourceList->currentRow();
    if(ci >= 0)
        sources[ci] = std::string(s.toStdString());
}

void
QvisSessionSourceChanger::setFallbackPath(const QString &path)
{
   if (fileLineEdit != 0)
      fileLineEdit->setFallbackPath(path);
}
