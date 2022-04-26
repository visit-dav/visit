// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

// ****************************************************************************
// Method: AttributeSubject::TypeName
//
// Purpose:
//   Returns the name of the type.
//
// Returns:    The name of the type.
//
// Programmer: Kathleen Bonnell
// Creation:   June 20, 2006
//
// Modifications:
//
// ****************************************************************************

const std::string
AttributeSubject::TypeName() const
{
    return "AttributeSubject";
}


// ****************************************************************************
// Method: AttributeSubject::DeprecationMessage
//
// Purpose:
//   Constructs a deprecation message from the field and version.
//
// Arguments:
//   oldField   The field being deprecated.
//   version    The version the field will no longer be supported.
//
// Returns:    The deprecation message.
//
// Programmer: Kathleen Biagas
// Creation:   April 26, 2022
//
// Modifications:
//
// ****************************************************************************

std::string
AttributeSubject::DeprecationMessage(const std::string &oldField,
                                     const std::string &version) const
{
    return TypeName() + std::string("::") + oldField +
           std::string(" is deprecated and will be removed in version ") +
           version +
           std::string(". Please update your session and/or config files.");
}


// ****************************************************************************
// Method: AttributeSubject::DeprecationMessage
//
// Purpose:
//   Constructs a deprecation message from the field and version.
//
// Arguments:
//   oldField   The field being deprecated.
//   newField   The new field that replaces the old.
//   version    The version the field will no longer be supported.
//
// Returns:    The deprection message.
//
// Programmer: Kathleen Biagas
// Creation:   April 26, 2022
//
// Modifications:
//
// ****************************************************************************

std::string
AttributeSubject::DeprecationMessage(const std::string &oldField,
                                     const std::string &newField,
                                     const std::string &version) const
{
    return TypeName() + std::string("::") + oldField +
           std::string(" is deprecated and will be removed in version ") +
           version +
           std::string(". Please use ")  + newField +
           std::string(" instead.") +
           std::string(" Please update your session and/or config files.");
}

