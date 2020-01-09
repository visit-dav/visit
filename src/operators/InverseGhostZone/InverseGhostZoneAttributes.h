// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef INVERSEGHOSTZONEATTRIBUTES_H
#define INVERSEGHOSTZONEATTRIBUTES_H
#include <AttributeSubject.h>


// ****************************************************************************
// Class: InverseGhostZoneAttributes
//
// Purpose:
//    This class contains attributes for the inverse ghost zone operator.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class InverseGhostZoneAttributes : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    InverseGhostZoneAttributes();
    InverseGhostZoneAttributes(const InverseGhostZoneAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    InverseGhostZoneAttributes(private_tmfs_t tmfs);
    InverseGhostZoneAttributes(const InverseGhostZoneAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~InverseGhostZoneAttributes();

    virtual InverseGhostZoneAttributes& operator = (const InverseGhostZoneAttributes &obj);
    virtual bool operator == (const InverseGhostZoneAttributes &obj) const;
    virtual bool operator != (const InverseGhostZoneAttributes &obj) const;
private:
    void Init();
    void Copy(const InverseGhostZoneAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetRequestGhostZones(bool requestGhostZones_);
    void SetShowDuplicated(bool showDuplicated_);
    void SetShowEnhancedConnectivity(bool showEnhancedConnectivity_);
    void SetShowReducedConnectivity(bool showReducedConnectivity_);
    void SetShowAMRRefined(bool showAMRRefined_);
    void SetShowExterior(bool showExterior_);
    void SetShowNotApplicable(bool showNotApplicable_);

    // Property getting methods
    bool GetRequestGhostZones() const;
    bool GetShowDuplicated() const;
    bool GetShowEnhancedConnectivity() const;
    bool GetShowReducedConnectivity() const;
    bool GetShowAMRRefined() const;
    bool GetShowExterior() const;
    bool GetShowNotApplicable() const;

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
        ID_requestGhostZones = 0,
        ID_showDuplicated,
        ID_showEnhancedConnectivity,
        ID_showReducedConnectivity,
        ID_showAMRRefined,
        ID_showExterior,
        ID_showNotApplicable,
        ID__LAST
    };

private:
    bool requestGhostZones;
    bool showDuplicated;
    bool showEnhancedConnectivity;
    bool showReducedConnectivity;
    bool showAMRRefined;
    bool showExterior;
    bool showNotApplicable;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define INVERSEGHOSTZONEATTRIBUTES_TMFS "bbbbbbb"

#endif
