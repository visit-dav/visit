// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef NAMESPACEATTRIBUTES_H
#define NAMESPACEATTRIBUTES_H
#include <state_exports.h>
#include <AttributeSubject.h>


// ****************************************************************************
// Class: NamespaceAttributes
//
// Purpose:
//    This class contain the information needed to represent a namespace.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class STATE_API NamespaceAttributes : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    NamespaceAttributes();
    NamespaceAttributes(const NamespaceAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    NamespaceAttributes(private_tmfs_t tmfs);
    NamespaceAttributes(const NamespaceAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~NamespaceAttributes();

    virtual NamespaceAttributes& operator = (const NamespaceAttributes &obj);
    virtual bool operator == (const NamespaceAttributes &obj) const;
    virtual bool operator != (const NamespaceAttributes &obj) const;
private:
    void Init();
    void Copy(const NamespaceAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectSubsets();

    // Property setting methods
    void SetType(int type_);
    void SetSubsets(const intVector &subsets_);
    void SetMin(int min_);
    void SetMax(int max_);

    // Property getting methods
    int             GetType() const;
    const intVector &GetSubsets() const;
          intVector &GetSubsets();
    int             GetMin() const;
    int             GetMax() const;

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
        ID_type = 0,
        ID_subsets,
        ID_min,
        ID_max,
        ID__LAST
    };

private:
    int       type;
    intVector subsets;
    int       min;
    int       max;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define NAMESPACEATTRIBUTES_TMFS "ii*ii"

#endif
