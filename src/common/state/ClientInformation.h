// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CLIENTINFORMATION_H
#define CLIENTINFORMATION_H
#include <state_exports.h>
#include <string>
#include <AttributeSubject.h>


// ****************************************************************************
// Class: ClientInformation
//
// Purpose:
//    This class contains the attributes that describe the client
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class STATE_API ClientInformation : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    ClientInformation();
    ClientInformation(const ClientInformation &obj);
protected:
    // These constructors are for objects derived from this class
    ClientInformation(private_tmfs_t tmfs);
    ClientInformation(const ClientInformation &obj, private_tmfs_t tmfs);
public:
    virtual ~ClientInformation();

    virtual ClientInformation& operator = (const ClientInformation &obj);
    virtual bool operator == (const ClientInformation &obj) const;
    virtual bool operator != (const ClientInformation &obj) const;
private:
    void Init();
    void Copy(const ClientInformation &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectClientName();
    void SelectMethodNames();
    void SelectMethodPrototypes();

    // Property setting methods
    void SetClientName(const std::string &clientName_);
    void SetMethodNames(const stringVector &methodNames_);
    void SetMethodPrototypes(const stringVector &methodPrototypes_);

    // Property getting methods
    const std::string  &GetClientName() const;
          std::string  &GetClientName();
    const stringVector &GetMethodNames() const;
          stringVector &GetMethodNames();
    const stringVector &GetMethodPrototypes() const;
          stringVector &GetMethodPrototypes();


    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    void DeclareMethod(const std::string &name, const std::string &proto);
    int GetMethodIndex(const std::string &name) const;
    std::string GetMethod(int i) const;
    std::string GetMethodPrototype(int i) const;
    void ClearMethods();

    // IDs that can be used to identify fields in case statements
    enum {
        ID_clientName = 0,
        ID_methodNames,
        ID_methodPrototypes,
        ID__LAST
    };

private:
    std::string  clientName;
    stringVector methodNames;
    stringVector methodPrototypes;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define CLIENTINFORMATION_TMFS "ss*s*"

#endif
