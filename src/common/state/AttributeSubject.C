#include <AttributeSubject.h>

// *******************************************************************
// Method: AttributeSubject::AttributeSubject
//
// Purpose: 
//   Constructor for the AttributeSubject class.
//
// Arguments:
//   formatString : This string describes the kinds of attributes that
//                  are contained in the object.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 7 12:53:39 PDT 2000
//
// Modifications:
//   
// *******************************************************************

AttributeSubject::AttributeSubject(const char *formatString) : 
    AttributeGroup(formatString), Subject()
{
    // nothing special here.
}

// *******************************************************************
// Method: AttributeSubject::~AttributeSubject
//
// Purpose: 
//   Destructor for the AttributeSubject class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 7 12:54:49 PDT 2000
//
// Modifications:
//   
// *******************************************************************

AttributeSubject::~AttributeSubject()
{
    // nothing special here either.
}

// *******************************************************************
// Method: AttributeSubject::Notify()
//
// Purpose: 
//   Tells all Observers to update, then unselects all the attributes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 15:55:06 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
AttributeSubject::Notify()
{
    // Call the base class's Notify method.
    Subject::Notify();

    // Now that all the Obsevrers have been called, unselect all the
    // attributes.
    UnSelectAll();
}

// ****************************************************************************
// Method: AttributeSubject::CreateCompatible
//
// Purpose: 
//   Creates a compatible object of the specified type.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 14:11:37 PST 2002
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
AttributeSubject::CreateCompatible(const std::string &) const
{
    return 0;
}

