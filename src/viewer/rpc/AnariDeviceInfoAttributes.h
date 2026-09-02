// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ANARI_DEVICE_INFO_ATTRIBUTES_H
#define ANARI_DEVICE_INFO_ATTRIBUTES_H
#include <AttributeSubject.h>
#include <viewerrpc_exports.h>
#include <string>

// ****************************************************************************
//  Class:  AnariDeviceInfoAttributes
//
//  Purpose:
//    Carries the result of a GetAnariDeviceInfoRPC (a MapNode, serialized to
//    XML) from the viewer back to the client that requested it, so the
//    client's ANARI rendering settings dialog can be populated without the
//    client creating a local ANARI device. A dedicated state object is used
//    (rather than reusing QueryAttributes.XmlResult) so this doesn't collide
//    with actual query results.
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

class VIEWER_RPC_API AnariDeviceInfoAttributes : public AttributeSubject
{
public:
    AnariDeviceInfoAttributes();
    AnariDeviceInfoAttributes(const AnariDeviceInfoAttributes &obj);
    virtual ~AnariDeviceInfoAttributes();

    AnariDeviceInfoAttributes &operator=(const AnariDeviceInfoAttributes &obj);
    bool operator==(const AnariDeviceInfoAttributes &obj) const;
    bool operator!=(const AnariDeviceInfoAttributes &obj) const;

    virtual void SelectAll();
    virtual const std::string TypeName() const { return "AnariDeviceInfoAttributes"; }
    virtual AttributeSubject *NewInstance(bool) const;
    virtual bool CopyAttributes(const AttributeGroup *atts);
    virtual bool FieldsEqual(int index, const AttributeGroup *rhs) const;

    void SetXmlResult(const std::string &xmlResult_);
    const std::string &GetXmlResult() const { return xmlResult; }
          std::string &GetXmlResult() { return xmlResult; }

private:
    std::string xmlResult;
};

#endif
