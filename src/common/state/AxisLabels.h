// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AXISLABELS_H
#define AXISLABELS_H
#include <state_exports.h>
#include <AttributeSubject.h>

#include <FontAttributes.h>

// ****************************************************************************
// Class: AxisLabels
//
// Purpose:
//    Contains the label properties for one axis.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class STATE_API AxisLabels : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    AxisLabels();
    AxisLabels(const AxisLabels &obj);
protected:
    // These constructors are for objects derived from this class
    AxisLabels(private_tmfs_t tmfs);
    AxisLabels(const AxisLabels &obj, private_tmfs_t tmfs);
public:
    virtual ~AxisLabels();

    virtual AxisLabels& operator = (const AxisLabels &obj);
    virtual bool operator == (const AxisLabels &obj) const;
    virtual bool operator != (const AxisLabels &obj) const;
private:
    void Init();
    void Copy(const AxisLabels &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectFont();

    // Property setting methods
    void SetVisible(bool visible_);
    void SetFont(const FontAttributes &font_);
    void SetScaling(int scaling_);

    // Property getting methods
    bool                 GetVisible() const;
    const FontAttributes &GetFont() const;
          FontAttributes &GetFont();
    int                  GetScaling() const;

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
        ID_visible = 0,
        ID_font,
        ID_scaling,
        ID__LAST
    };

private:
    bool           visible;
    FontAttributes font;
    int            scaling;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define AXISLABELS_TMFS "bai"

#endif
