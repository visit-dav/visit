// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ACTIONGROUPDESCRIPTION_H
#define ACTIONGROUPDESCRIPTION_H
#include <viewercore_exports.h>
#include <string>
#include <AttributeSubject.h>

#include <ViewerRPC.h>

// ****************************************************************************
// Class: ActionGroupDescription
//
// Purpose:
//    This class contains the attributes that describe an action group.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API ActionGroupDescription : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    ActionGroupDescription();
    ActionGroupDescription(const ActionGroupDescription &obj);
protected:
    // These constructors are for objects derived from this class
    ActionGroupDescription(private_tmfs_t tmfs);
    ActionGroupDescription(const ActionGroupDescription &obj, private_tmfs_t tmfs);
public:
    virtual ~ActionGroupDescription();

    virtual ActionGroupDescription& operator = (const ActionGroupDescription &obj);
    virtual bool operator == (const ActionGroupDescription &obj) const;
    virtual bool operator != (const ActionGroupDescription &obj) const;
private:
    void Init();
    void Copy(const ActionGroupDescription &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectName();
    void SelectActions();

    // Property setting methods
    void SetName(const std::string &name_);
    void SetActions(const stringVector &actions_);
    void SetVisible(bool visible_);

    // Property getting methods
    const std::string  &GetName() const;
          std::string  &GetName();
    const stringVector &GetActions() const;
          stringVector &GetActions();
    bool               GetVisible() const;

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);


    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    ActionGroupDescription(const std::string &n);
    void AddAction(ViewerRPC::ViewerRPCType t);
    ViewerRPC::ViewerRPCType GetAction(int index) const;

    // IDs that can be used to identify fields in case statements
    enum {
        ID_name = 0,
        ID_actions,
        ID_visible,
        ID__LAST
    };

private:
    std::string  name;
    stringVector actions;
    bool         visible;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define ACTIONGROUPDESCRIPTION_TMFS "ss*b"

#endif
