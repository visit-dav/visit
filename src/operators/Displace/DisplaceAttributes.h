// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DISPLACEATTRIBUTES_H
#define DISPLACEATTRIBUTES_H
#include <string>
#include <AttributeSubject.h>


// ****************************************************************************
// Class: DisplaceAttributes
//
// Purpose:
//    This class contains attributes for the displace operator.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class DisplaceAttributes : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    DisplaceAttributes();
    DisplaceAttributes(const DisplaceAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    DisplaceAttributes(private_tmfs_t tmfs);
    DisplaceAttributes(const DisplaceAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~DisplaceAttributes();

    virtual DisplaceAttributes& operator = (const DisplaceAttributes &obj);
    virtual bool operator == (const DisplaceAttributes &obj) const;
    virtual bool operator != (const DisplaceAttributes &obj) const;
private:
    void Init();
    void Copy(const DisplaceAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectVariable();

    // Property setting methods
    void SetFactor(double factor_);
    void SetVariable(const std::string &variable_);

    // Property getting methods
    double            GetFactor() const;
    const std::string &GetVariable() const;
          std::string &GetVariable();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);


    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;


    // IDs that can be used to identify fields in case statements
    enum {
        ID_factor = 0,
        ID_variable,
        ID__LAST
    };

private:
    double      factor;
    std::string variable;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define DISPLACEATTRIBUTES_TMFS "ds"

#endif
