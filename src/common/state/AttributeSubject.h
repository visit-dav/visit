#ifndef ATTRIBUTESUBJECT_H
#define ATTRIBUTESUBJECT_H
#include <state_exports.h>
#include <AttributeGroup.h>
#include <Subject.h>

// ****************************************************************************
// Class: AttributeSubject
//
// Purpose:
//   This is a base class for state objects that can be transmitted
//   across a connection. It inherits an interface from Subject so it
//   can tell its observers when it changes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 7 12:52:00 PDT 2000
//
// Modifications:
//   Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001
//   Added virtual method VarChangeRequiresReset.
//
//   Brad Whitlock, Mon Feb 11 15:26:34 PST 2002
//   Added a new method to create compatible types.
//
// ****************************************************************************

class STATE_API AttributeSubject : public AttributeGroup, public Subject
{
public:
    AttributeSubject(const char *);
    virtual ~AttributeSubject();
    virtual void SelectAll() = 0;
    virtual void Notify();
    virtual AttributeSubject *CreateCompatible(const std::string &) const;

    // what the ?
    virtual bool VarChangeRequiresReset(void) { return false; };
};

#endif
