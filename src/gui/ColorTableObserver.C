// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ColorTableObserver.h>
#include <ColorControlPointList.h>
#include <ColorTableAttributes.h>
#include <QvisColorTableButton.h>
#include <QvisNoDefaultColorTableButton.h>

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
//   Brad Whitlock, Mon Dec 17 11:06:50 PST 2007
//   Made it use ids.
//
//   Kathleen Biagas, Mon Aug  4 15:48:31 PDT 2014
//   Send category name when ading a color table, change in groupingFlag means
//   the buttons need a change.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed categories, added tags, and new `active` array that says if color
//   should be visible in the button.
// 
//   Justin Privitera, Mon Feb 13 14:32:02 PST 2023
//   Removed check for tagging flag, as it no longer exists.
// 
//   Justin Privitera, Mon Aug 21 15:54:50 PDT 2023
//   Changed ColorTableAttributes `names` to `colorTableNames` and `active` to
//   `colorTableActiveFlags`.
// 
//   Justin Privitera, Mon Aug 28 11:22:47 PDT 2023
//   The tag list being selected now also triggers the update.
//
//   Justin Privitera, Tue Sep  5 12:49:42 PDT 2023
//   Check if tags match any has been selected to trigger update.
//   Removed obsolete GetChangesMade check and setting.
//   Removed disaster recovery code for CTactive and CTnames not being the same
//   length as it it no longer necessary.
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

    // NOTE: The logic here is identical to that in ViewerSubject::HandleColorTable.
    // If you are making a change to either, change in both places.

    // If the names or the color table attributes are changing, then we
    // have to update the widget.
    if(colorAtts->IsSelected(ColorTableAttributes::ID_colorTableNames) ||
       colorAtts->IsSelected(ColorTableAttributes::ID_tagListNames) ||
       colorAtts->IsSelected(ColorTableAttributes::ID_colorTables) ||
       colorAtts->IsSelected(ColorTableAttributes::ID_tagsMatchAny))
    {
        // Clear all of the color tables.
        QvisColorTableButton::setColorTableAttributes(colorAtts);
        QvisColorTableButton::clearAllColorTables();
        QvisNoDefaultColorTableButton::setColorTableAttributes(colorAtts);
        QvisNoDefaultColorTableButton::clearAllColorTables();

        int nNames = colorAtts->GetNumColorTables();
        const stringVector &ctnames = colorAtts->GetColorTableNames();
        const intVector &ctactive = colorAtts->GetColorTableActiveFlags();
        for (int i = 0; i < nNames; ++i)
        {
            if (ctactive[i])
            {
                QvisColorTableButton::addColorTable(ctnames[i].c_str());
                QvisNoDefaultColorTableButton::addColorTable(ctnames[i].c_str());
            }
        }

        // Update all of the QvisColorTableButton widgets.
        QvisColorTableButton::updateColorTableButtons();
        QvisNoDefaultColorTableButton::updateColorTableButtons();
    }
}
