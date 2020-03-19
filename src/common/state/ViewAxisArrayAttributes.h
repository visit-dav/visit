// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWAXISARRAYATTRIBUTES_H
#define VIEWAXISARRAYATTRIBUTES_H
#include <state_exports.h>
#include <AttributeSubject.h>


// ****************************************************************************
// Class: ViewAxisArrayAttributes
//
// Purpose:
//    This class contains the axis array view attributes.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class STATE_API ViewAxisArrayAttributes : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    ViewAxisArrayAttributes();
    ViewAxisArrayAttributes(const ViewAxisArrayAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    ViewAxisArrayAttributes(private_tmfs_t tmfs);
    ViewAxisArrayAttributes(const ViewAxisArrayAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~ViewAxisArrayAttributes();

    virtual ViewAxisArrayAttributes& operator = (const ViewAxisArrayAttributes &obj);
    virtual bool operator == (const ViewAxisArrayAttributes &obj) const;
    virtual bool operator != (const ViewAxisArrayAttributes &obj) const;
private:
    void Init();
    void Copy(const ViewAxisArrayAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectDomainCoords();
    void SelectRangeCoords();
    void SelectViewportCoords();

    // Property setting methods
    void SetDomainCoords(const double *domainCoords_);
    void SetRangeCoords(const double *rangeCoords_);
    void SetViewportCoords(const double *viewportCoords_);

    // Property getting methods
    const double *GetDomainCoords() const;
          double *GetDomainCoords();
    const double *GetRangeCoords() const;
          double *GetRangeCoords();
    const double *GetViewportCoords() const;
          double *GetViewportCoords();

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
        ID_domainCoords = 0,
        ID_rangeCoords,
        ID_viewportCoords,
        ID__LAST
    };

private:
    double domainCoords[2];
    double rangeCoords[2];
    double viewportCoords[4];

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define VIEWAXISARRAYATTRIBUTES_TMFS "DDD"

#endif
