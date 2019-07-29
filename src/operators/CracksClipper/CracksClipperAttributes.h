// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CRACKSCLIPPERATTRIBUTES_H
#define CRACKSCLIPPERATTRIBUTES_H
#include <string>
#include <AttributeSubject.h>


// ****************************************************************************
// Class: CracksClipperAttributes
//
// Purpose:
//    Attributes for the cracks clipper operator
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class CracksClipperAttributes : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    CracksClipperAttributes();
    CracksClipperAttributes(const CracksClipperAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    CracksClipperAttributes(private_tmfs_t tmfs);
    CracksClipperAttributes(const CracksClipperAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~CracksClipperAttributes();

    virtual CracksClipperAttributes& operator = (const CracksClipperAttributes &obj);
    virtual bool operator == (const CracksClipperAttributes &obj) const;
    virtual bool operator != (const CracksClipperAttributes &obj) const;
private:
    void Init();
    void Copy(const CracksClipperAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectCrack1Var();
    void SelectCrack2Var();
    void SelectCrack3Var();
    void SelectStrainVar();
    void SelectInMassVar();

    // Property setting methods
    void SetCrack1Var(const std::string &crack1Var_);
    void SetCrack2Var(const std::string &crack2Var_);
    void SetCrack3Var(const std::string &crack3Var_);
    void SetStrainVar(const std::string &strainVar_);
    void SetShowCrack1(bool showCrack1_);
    void SetShowCrack2(bool showCrack2_);
    void SetShowCrack3(bool showCrack3_);
    void SetInMassVar(const std::string &inMassVar_);

    // Property getting methods
    const std::string &GetCrack1Var() const;
          std::string &GetCrack1Var();
    const std::string &GetCrack2Var() const;
          std::string &GetCrack2Var();
    const std::string &GetCrack3Var() const;
          std::string &GetCrack3Var();
    const std::string &GetStrainVar() const;
          std::string &GetStrainVar();
    bool              GetShowCrack1() const;
    bool              GetShowCrack2() const;
    bool              GetShowCrack3() const;
    const std::string &GetInMassVar() const;
          std::string &GetInMassVar();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);


    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    bool ChangesRequireRecalculation(const CracksClipperAttributes &) const;

    // IDs that can be used to identify fields in case statements
    enum {
        ID_crack1Var = 0,
        ID_crack2Var,
        ID_crack3Var,
        ID_strainVar,
        ID_showCrack1,
        ID_showCrack2,
        ID_showCrack3,
        ID_inMassVar,
        ID__LAST
    };

private:
    std::string crack1Var;
    std::string crack2Var;
    std::string crack3Var;
    std::string strainVar;
    bool        showCrack1;
    bool        showCrack2;
    bool        showCrack3;
    std::string inMassVar;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define CRACKSCLIPPERATTRIBUTES_TMFS "ssssbbbs"

#endif
