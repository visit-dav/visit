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
//  Modifications:
//    Kevin Griffin, Tue 22 Sep 2026
//    Added requestor field ("ss" codetype).
//
// ****************************************************************************

AnariDeviceInfoAttributes::AnariDeviceInfoAttributes() : AttributeSubject("ss")
{
    xmlResult = "";
    requestor = "";
}

// ****************************************************************************
//  Constructor:  AnariDeviceInfoAttributes::AnariDeviceInfoAttributes
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
//  Modifications:
//    Kevin Griffin, Tue 22 Sep 2026
//    Added requestor field ("ss" codetype).
//
// ****************************************************************************

AnariDeviceInfoAttributes::AnariDeviceInfoAttributes(const AnariDeviceInfoAttributes &obj)
    : AttributeSubject("ss")
{
    xmlResult = obj.xmlResult;
    requestor = obj.requestor;
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
//  Modifications:
//    Kevin Griffin, Tue 22 Sep 2026
//    Added requestor field.
//
// ****************************************************************************

AnariDeviceInfoAttributes &
AnariDeviceInfoAttributes::operator=(const AnariDeviceInfoAttributes &obj)
{
    if(this != &obj)
    {
        xmlResult = obj.xmlResult;
        requestor = obj.requestor;
    }
    return *this;
}

// ****************************************************************************
//  Method:  AnariDeviceInfoAttributes::operator==
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
//  Modifications:
//    Kevin Griffin, Tue 22 Sep 2026
//    Added requestor field.
//
// ****************************************************************************

bool
AnariDeviceInfoAttributes::operator==(const AnariDeviceInfoAttributes &obj) const
{
    return xmlResult == obj.xmlResult && requestor == obj.requestor;
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
//  Modifications:
//    Kevin Griffin, Tue 22 Sep 2026
//    Added requestor field (index 1).
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
    case 1:
        retval = (requestor == obj.requestor);
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
//  Modifications:
//    Kevin Griffin, Tue 22 Sep 2026
//    Added requestor field (index 1).
//
// ****************************************************************************

void
AnariDeviceInfoAttributes::SelectAll()
{
    Select(0, (void*)&xmlResult);
    Select(1, (void*)&requestor);
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

// ****************************************************************************
//  Method:  AnariDeviceInfoAttributes::SetRequestor
//
//  Purpose:
//    Identifies which ANARI settings panel this result is meant for, as a
//    "<rendertype>" (e.g. "surface", "volume").
//
//  Programmer:  Kevin Griffin
//  Creation:    Tue 22 Sep 2026
//
// ****************************************************************************

void
AnariDeviceInfoAttributes::SetRequestor(const std::string &requestor_)
{
    requestor = requestor_;
    Select(1, (void *)&requestor);
}
