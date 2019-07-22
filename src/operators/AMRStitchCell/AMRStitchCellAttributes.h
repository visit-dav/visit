// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AMRSTITCHCELLATTRIBUTES_H
#define AMRSTITCHCELLATTRIBUTES_H
#include <string>
#include <AttributeSubject.h>


// ****************************************************************************
// Class: AMRStitchCellAttributes
//
// Purpose:
//    Attributes for Stitch Cell Operator
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class AMRStitchCellAttributes : public AttributeSubject
{
public:
    enum CreateType
    {
        DualGridAndStitchCells,
        DualGrid,
        StitchCells
    };

    // These constructors are for objects of this class
    AMRStitchCellAttributes();
    AMRStitchCellAttributes(const AMRStitchCellAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    AMRStitchCellAttributes(private_tmfs_t tmfs);
    AMRStitchCellAttributes(const AMRStitchCellAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~AMRStitchCellAttributes();

    virtual AMRStitchCellAttributes& operator = (const AMRStitchCellAttributes &obj);
    virtual bool operator == (const AMRStitchCellAttributes &obj) const;
    virtual bool operator != (const AMRStitchCellAttributes &obj) const;
private:
    void Init();
    void Copy(const AMRStitchCellAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetCreateCellsOfType(CreateType CreateCellsOfType_);

    // Property getting methods
    CreateType GetCreateCellsOfType() const;

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Enum conversion functions
    static std::string CreateType_ToString(CreateType);
    static bool CreateType_FromString(const std::string &, CreateType &);
protected:
    static std::string CreateType_ToString(int);
public:

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;


    // IDs that can be used to identify fields in case statements
    enum {
        ID_CreateCellsOfType = 0,
        ID__LAST
    };

private:
    int CreateCellsOfType;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define AMRSTITCHCELLATTRIBUTES_TMFS "i"

#endif
