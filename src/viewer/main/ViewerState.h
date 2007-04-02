#ifndef VIEWER_STATE_H
#define VIEWER_STATE_H
#include <AttributeSubject.h>

// ****************************************************************************
// Class: ViewerState
//
// Purpose:
//   This class contains pointers to the state objects that the viewer uses
//   in their xfer order. It's a convenience class that lets us create all
//   of the state objects using the copy constructor.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu May 5 16:40:01 PST 2005
//
// Modifications:
//   
// ****************************************************************************

class ViewerState
{
public:
    ViewerState();
    ViewerState(const ViewerState &);
    virtual ~ViewerState();

    void Add(AttributeSubject *, bool partialSendOkay = true);
    void SetOwnsObjects(bool);

    AttributeSubject *GetStateObject(int i);
    const AttributeSubject *GetStateObject(int i) const;
    bool  GetPartialSendFlag(int i) const;
    int   GetNObjects() const;
private:
    AttributeGroupVector objVector;
    unsignedCharVector   partialSendVector;
    bool                 ownsObjects;
};

#endif
