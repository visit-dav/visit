#ifndef COLOR_TABLE_OBSERVER_H
#define COLOR_TABLE_OBSERVER_H
#include <gui_exports.h>
#include <Observer.h>

// ****************************************************************************
// Class: ColorTableObserver
//
// Purpose:
//   This observer watches the color table attributes from the viewer proxy
//   and uses them to update the list of color tables in the
//   QvisColorTableButton widget.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 20 13:27:40 PST 2001
//
// Modifications:
//   
// ****************************************************************************

class GUI_API ColorTableObserver : public Observer
{
public:
    ColorTableObserver(Subject *subj);
    virtual ~ColorTableObserver();
    virtual void Update(Subject *subj);
};

#endif
