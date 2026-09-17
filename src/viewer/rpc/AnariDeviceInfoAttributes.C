// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <AnariDeviceInfoAttributes.h>

// ****************************************************************************
//  Constructor:  AnariDeviceInfoAttributes::AnariDeviceInfoAttributes
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

AnariDeviceInfoAttributes::AnariDeviceInfoAttributes() : AttributeSubject("s")
{
    xmlResult = "";
}

// ****************************************************************************
//  Constructor:  AnariDeviceInfoAttributes::AnariDeviceInfoAttributes
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

AnariDeviceInfoAttributes::AnariDeviceInfoAttributes(const AnariDeviceInfoAttributes &obj)
    : AttributeSubject("s")
{
    xmlResult = obj.xmlResult;
}

// ****************************************************************************
//  Destructor:  AnariDeviceInfoAttributes::~AnariDeviceInfoAttributes
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

AnariDeviceInfoAttributes::~AnariDeviceInfoAttributes()
{
}

// ****************************************************************************
//  Method:  AnariDeviceInfoAttributes::operator=
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

AnariDeviceInfoAttributes &
AnariDeviceInfoAttributes::operator=(const AnariDeviceInfoAttributes &obj)
{
    if(this != &obj)
        xmlResult = obj.xmlResult;
    return *this;
}

// ****************************************************************************
//  Method:  AnariDeviceInfoAttributes::operator==
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

bool
AnariDeviceInfoAttributes::operator==(const AnariDeviceInfoAttributes &obj) const
{
    return xmlResult == obj.xmlResult;
}

bool
AnariDeviceInfoAttributes::operator!=(const AnariDeviceInfoAttributes &obj) const
{
    return !(*this == obj);
}

// ****************************************************************************
//  Method:  AnariDeviceInfoAttributes::NewInstance
//
//  Purpose:
//    Creates a new instance of this class, used when ViewerState is
//    copy-constructed (e.g. once per client connection in
//    ViewerClientConnection). The AttributeSubject base class default
//    returns NULL, which crashes the caller if not overridden here.
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

AttributeSubject *
AnariDeviceInfoAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new AnariDeviceInfoAttributes(*this);
    else
        retval = new AnariDeviceInfoAttributes;

    return retval;
}

// ****************************************************************************
//  Method:  AnariDeviceInfoAttributes::CopyAttributes
//
//  Purpose:
//    Copies the fields of atts into this object. Required so that per-client
//    ViewerState copies (see ViewerClientConnection::BroadcastToClient) pick
//    up the real xmlResult from the master ViewerState object instead of
//    silently keeping their own (default-constructed, empty) copy -- the
//    AttributeGroup base class default is a no-op that returns false.
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

bool
AnariDeviceInfoAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    const AnariDeviceInfoAttributes *tmp = (const AnariDeviceInfoAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
//  Method:  AnariDeviceInfoAttributes::FieldsEqual
//
//  Purpose:
//    Compares one field between this object and rhs. Required for partial
//    (delta) sends to clients -- the AttributeGroup base class default
//    always returns false, which is functionally safe (it just disables the
//    "already equal, skip resending" optimization) but is provided here for
//    consistency with the CopyAttributes fix.
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

bool
AnariDeviceInfoAttributes::FieldsEqual(int index, const AttributeGroup *rhs) const
{
    const AnariDeviceInfoAttributes &obj = *((const AnariDeviceInfoAttributes*)rhs);
    bool retval = false;
    switch(index)
    {
    case 0:
        retval = (xmlResult == obj.xmlResult);
        break;
    default:
        retval = false;
    }
    return retval;
}

// ****************************************************************************
//  Method:  AnariDeviceInfoAttributes::SelectAll
//
//  Purpose:
//    Select all attributes.
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

void
AnariDeviceInfoAttributes::SelectAll()
{
    Select(0, (void*)&xmlResult);
}

// ****************************************************************************
//  Method:  AnariDeviceInfoAttributes::SetXmlResult
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

void
AnariDeviceInfoAttributes::SetXmlResult(const std::string &xmlResult_)
{
    xmlResult = xmlResult_;
    Select(0, (void *)&xmlResult);
}
