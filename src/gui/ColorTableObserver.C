#include <ColorTableObserver.h>
#include <ColorTableAttributes.h>
#include <QvisColorTableButton.h>

// ****************************************************************************
// Method: ColorTableObserver::ColorTableObserver
//
// Purpose: 
//   Constructor for the ColorTableObserver class.
//
// Arguments:
//   subj : A pointer to the ColorTableAttributes subject that this object
//          will observe.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 20 13:36:35 PST 2001
//
// Modifications:
//   
// ****************************************************************************

ColorTableObserver::ColorTableObserver(Subject *subj) : Observer(subj)
{
    // nothing here
}

// ****************************************************************************
// Method: ColorTableObserver::~ColorTableObserver
//
// Purpose: 
//   This is the destructor for the ColorTableObserver class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 20 13:37:24 PST 2001
//
// Modifications:
//   
// ****************************************************************************

ColorTableObserver::~ColorTableObserver()
{
    // nothing here
}

// ****************************************************************************
// Method: ColorTableObserver::Update
//
// Purpose: 
//   This method is called when the ColorTableAttributes subject that this
//   object observes is updated.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 20 13:37:56 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ColorTableObserver::Update(Subject *)
{
    ColorTableAttributes *colorAtts = (ColorTableAttributes *)subject;

    // For when we have pixmaps later, here are the rules...
    // 1. if just #0 is ever selected, we're adding/deleting colortables.
    // 2. If just #1 is selected , we're modifying a color table.
    // 3. If just #2 is selected, we're changing the default colormap. This is
    //    only of concern if I decide to show what the default is in a widget.

    // If the names or the color table attributes are changing, then we
    // have to update the widget.
    if(colorAtts->IsSelected(0) || colorAtts->IsSelected(1))
    {
        // Clear all of the color tables.
        QvisColorTableButton::clearAllColorTables();

        int nNames = colorAtts->GetNames().size();
        const stringVector &names = colorAtts->GetNames();
        for(int i = 0; i < nNames; ++i)
        {
            QvisColorTableButton::addColorTable(names[i].c_str());
        }

        // Update all of the QvisColorTableButton widgets.
        QvisColorTableButton::updateColorTableButtons();
    }
}
