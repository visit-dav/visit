// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef DEFEREXPRESSIONATTRIBUTES_H
#define DEFEREXPRESSIONATTRIBUTES_H
#include <string>
#include <AttributeSubject.h>


// ****************************************************************************
// Class: DeferExpressionAttributes
//
// Purpose:
//    Attributes for the DeferExpression operator
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class DeferExpressionAttributes : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    DeferExpressionAttributes();
    DeferExpressionAttributes(const DeferExpressionAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    DeferExpressionAttributes(private_tmfs_t tmfs);
    DeferExpressionAttributes(const DeferExpressionAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~DeferExpressionAttributes();

    virtual DeferExpressionAttributes& operator = (const DeferExpressionAttributes &obj);
    virtual bool operator == (const DeferExpressionAttributes &obj) const;
    virtual bool operator != (const DeferExpressionAttributes &obj) const;
private:
    void Init();
    void Copy(const DeferExpressionAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectExprs();

    // Property setting methods
    void SetExprs(const stringVector &exprs_);

    // Property getting methods
    const stringVector &GetExprs() const;
          stringVector &GetExprs();

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
        ID_exprs = 0,
        ID__LAST
    };

private:
    stringVector exprs;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define DEFEREXPRESSIONATTRIBUTES_TMFS "s*"

#endif
