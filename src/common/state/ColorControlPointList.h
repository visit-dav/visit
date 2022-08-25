// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef COLORCONTROLPOINTLIST_H
#define COLORCONTROLPOINTLIST_H
#include <state_exports.h>
#include <string>
#include <AttributeSubject.h>

class ColorControlPoint;
#include <algorithm>

// ****************************************************************************
// Class: ColorControlPointList
//
// Purpose:
//    This class contains a list of ColorControlPoint objects.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class STATE_API ColorControlPointList : public AttributeSubject
{
public:
    enum SmoothingMethod
    {
        None,
        Linear,
        CubicSpline
    };

    // These constructors are for objects of this class
    ColorControlPointList();
    ColorControlPointList(const ColorControlPointList &obj);
protected:
    // These constructors are for objects derived from this class
    ColorControlPointList(private_tmfs_t tmfs);
    ColorControlPointList(const ColorControlPointList &obj, private_tmfs_t tmfs);
public:
    virtual ~ColorControlPointList();

    virtual ColorControlPointList& operator = (const ColorControlPointList &obj);
    virtual bool operator == (const ColorControlPointList &obj) const;
    virtual bool operator != (const ColorControlPointList &obj) const;
private:
    void Init();
    void Copy(const ColorControlPointList &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectControlPoints();
    void SelectTagNames();

    // Property setting methods
    void SetSmoothing(SmoothingMethod smoothing_);
    void SetEqualSpacingFlag(bool equalSpacingFlag_);
    void SetDiscreteFlag(bool discreteFlag_);
    void SetExternalFlag(bool externalFlag_);
    void SetTagNames(const stringVector &tagNames_);
    void SetTagChangesMade(bool tagChangesMade_);
    void SetBuiltIn(bool builtIn_);

    // Property getting methods
    const AttributeGroupVector &GetControlPoints() const;
          AttributeGroupVector &GetControlPoints();
    SmoothingMethod    GetSmoothing() const;
    bool               GetEqualSpacingFlag() const;
    bool               GetDiscreteFlag() const;
    bool               GetExternalFlag() const;
    const stringVector &GetTagNames() const;
          stringVector &GetTagNames();
    bool               GetTagChangesMade() const;
    bool               GetBuiltIn() const;

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);


    // Attributegroup convenience methods
    void AddControlPoints(const ColorControlPoint &);
    void ClearControlPoints();
    void RemoveControlPoints(int i);
    int  GetNumControlPoints() const;
    ColorControlPoint &GetControlPoints(int i);
    const ColorControlPoint &GetControlPoints(int i) const;

    ColorControlPoint &operator [] (int i);
    const ColorControlPoint &operator [] (int i) const;

    // Enum conversion functions
    static std::string SmoothingMethod_ToString(SmoothingMethod);
    static bool SmoothingMethod_FromString(const std::string &, SmoothingMethod &);
protected:
    static std::string SmoothingMethod_ToString(int);
public:

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    float EvalCubicSpline(float t, const float *allX, const float *allY, int n) const;
    void GetColorsCubicSpline(unsigned char *rgb, int ncolors, unsigned char *alpha=NULL) const;
    void GetColors(unsigned char *rgb, int ncolors, unsigned char *alpha=NULL) const;
    bool CompactCreateNode(DataNode *parentNode, bool completeSave, bool forceAdd);
    void AddTag(const std::string newtag);
    void RemoveTag(const std::string tag);
    void ClearTags();
    std::string GetTag(const int index) const;
    int GetNumTags() const;
    std::string GetTagsAsString() const;
    bool HasTag(const std::string tag) const;
    int GetTagIndex(const std::string tag) const;
    std::pair<bool, std::string> ValidateTag(const std::string tag) const;
    void SetNumControlPoints(const int n);
    virtual void ProcessOldVersions(DataNode *parentNode, const char *configVersion);

    // IDs that can be used to identify fields in case statements
    enum {
        ID_controlPoints = 0,
        ID_smoothing,
        ID_equalSpacingFlag,
        ID_discreteFlag,
        ID_externalFlag,
        ID_tagNames,
        ID_tagChangesMade,
        ID_builtIn,
        ID__LAST
    };

protected:
    AttributeGroup *CreateSubAttributeGroup(int index);
private:
    AttributeGroupVector controlPoints;
    int                  smoothing;
    bool                 equalSpacingFlag;
    bool                 discreteFlag;
    bool                 externalFlag;
    stringVector         tagNames;
    bool                 tagChangesMade;
    bool                 builtIn;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define COLORCONTROLPOINTLIST_TMFS "a*ibbbs*bb"

#endif
