// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MERGEOPERATORATTRIBUTES_H
#define MERGEOPERATORATTRIBUTES_H
#include <AttributeSubject.h>


// ****************************************************************************
// Class: MergeOperatorAttributes
//
// Purpose:
//    Attributes for Merge operaetor
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class MergeOperatorAttributes : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    MergeOperatorAttributes();
    MergeOperatorAttributes(const MergeOperatorAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    MergeOperatorAttributes(private_tmfs_t tmfs);
    MergeOperatorAttributes(const MergeOperatorAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~MergeOperatorAttributes();

    virtual MergeOperatorAttributes& operator = (const MergeOperatorAttributes &obj);
    virtual bool operator == (const MergeOperatorAttributes &obj) const;
    virtual bool operator != (const MergeOperatorAttributes &obj) const;
private:
    void Init();
    void Copy(const MergeOperatorAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetParallelMerge(bool parallelMerge_);
    void SetTolerance(double tolerance_);

    // Property getting methods
    bool   GetParallelMerge() const;
    double GetTolerance() const;

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
        ID_parallelMerge = 0,
        ID_tolerance,
        ID__LAST
    };

private:
    bool   parallelMerge;
    double tolerance;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define MERGEOPERATORATTRIBUTES_TMFS "bd"

#endif
