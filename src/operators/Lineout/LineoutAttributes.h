// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef LINEOUTATTRIBUTES_H
#define LINEOUTATTRIBUTES_H
#include <string>
#include <AttributeSubject.h>

#include <visitstream.h>

// ****************************************************************************
// Class: LineoutAttributes
//
// Purpose:
//    Attributes for the Lineout operator 
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class LineoutAttributes : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    LineoutAttributes();
    LineoutAttributes(const LineoutAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    LineoutAttributes(private_tmfs_t tmfs);
    LineoutAttributes(const LineoutAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~LineoutAttributes();

    virtual LineoutAttributes& operator = (const LineoutAttributes &obj);
    virtual bool operator == (const LineoutAttributes &obj) const;
    virtual bool operator != (const LineoutAttributes &obj) const;
private:
    void Init();
    void Copy(const LineoutAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectPoint1();
    void SelectPoint2();
    void SelectDesignator();

    // Property setting methods
    void SetPoint1(const double *point1_);
    void SetPoint2(const double *point2_);
    void SetInteractive(bool interactive_);
    void SetIgnoreGlobal(bool ignoreGlobal_);
    void SetSamplingOn(bool samplingOn_);
    void SetNumberOfSamplePoints(int numberOfSamplePoints_);
    void SetReflineLabels(bool reflineLabels_);
    void SetDesignator(const std::string &designator_);

    // Property getting methods
    const double      *GetPoint1() const;
          double      *GetPoint1();
    const double      *GetPoint2() const;
          double      *GetPoint2();
    bool              GetInteractive() const;
    bool              GetIgnoreGlobal() const;
    bool              GetSamplingOn() const;
    int               GetNumberOfSamplePoints() const;
    bool              GetReflineLabels() const;
    const std::string &GetDesignator() const;
          std::string &GetDesignator();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);


    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    void Print(ostream &, bool) const;

    // IDs that can be used to identify fields in case statements
    enum {
        ID_point1 = 0,
        ID_point2,
        ID_interactive,
        ID_ignoreGlobal,
        ID_samplingOn,
        ID_numberOfSamplePoints,
        ID_reflineLabels,
        ID_designator,
        ID__LAST
    };

private:
    double      point1[3];
    double      point2[3];
    bool        interactive;
    bool        ignoreGlobal;
    bool        samplingOn;
    int         numberOfSamplePoints;
    bool        reflineLabels;
    std::string designator;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define LINEOUTATTRIBUTES_TMFS "DDbbbibs"

#endif
