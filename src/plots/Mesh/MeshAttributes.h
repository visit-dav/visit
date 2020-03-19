// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MESHATTRIBUTES_H
#define MESHATTRIBUTES_H
#include <string>
#include <GlyphTypes.h>
#include <AttributeSubject.h>

#include <ColorAttribute.h>

// ****************************************************************************
// Class: MeshAttributes
//
// Purpose:
//    Attributes for the mesh plot
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class MeshAttributes : public AttributeSubject
{
public:
    enum SmoothingLevel
    {
        None,
        Fast,
        High
    };
    enum MeshColor
    {
        Foreground,
        MeshCustom,
        MeshRandom
    };
    enum OpaqueColor
    {
        Background,
        OpaqueCustom,
        OpaqueRandom
    };
    enum OpaqueMode
    {
        Auto,
        On,
        Off
    };

    // These constructors are for objects of this class
    MeshAttributes();
    MeshAttributes(const MeshAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    MeshAttributes(private_tmfs_t tmfs);
    MeshAttributes(const MeshAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~MeshAttributes();

    virtual MeshAttributes& operator = (const MeshAttributes &obj);
    virtual bool operator == (const MeshAttributes &obj) const;
    virtual bool operator != (const MeshAttributes &obj) const;
private:
    void Init();
    void Copy(const MeshAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectMeshColor();
    void SelectOpaqueColor();
    void SelectPointSizeVar();

    // Property setting methods
    void SetLegendFlag(bool legendFlag_);
    void SetLineWidth(int lineWidth_);
    void SetMeshColor(const ColorAttribute &meshColor_);
    void SetMeshColorSource(MeshColor meshColorSource_);
    void SetOpaqueColorSource(OpaqueColor opaqueColorSource_);
    void SetOpaqueMode(OpaqueMode opaqueMode_);
    void SetPointSize(double pointSize_);
    void SetOpaqueColor(const ColorAttribute &opaqueColor_);
    void SetSmoothingLevel(SmoothingLevel smoothingLevel_);
    void SetPointSizeVarEnabled(bool pointSizeVarEnabled_);
    void SetPointSizeVar(const std::string &pointSizeVar_);
    void SetPointType(GlyphType pointType_);
    void SetOpaqueMeshIsAppropriate(bool opaqueMeshIsAppropriate_);
    void SetShowInternal(bool showInternal_);
    void SetPointSizePixels(int pointSizePixels_);
    void SetOpacity(double opacity_);

    // Property getting methods
    bool                 GetLegendFlag() const;
    int                  GetLineWidth() const;
    const ColorAttribute &GetMeshColor() const;
          ColorAttribute &GetMeshColor();
    MeshColor            GetMeshColorSource() const;
    OpaqueColor          GetOpaqueColorSource() const;
    OpaqueMode           GetOpaqueMode() const;
    double               GetPointSize() const;
    const ColorAttribute &GetOpaqueColor() const;
          ColorAttribute &GetOpaqueColor();
    SmoothingLevel       GetSmoothingLevel() const;
    bool                 GetPointSizeVarEnabled() const;
    const std::string    &GetPointSizeVar() const;
          std::string    &GetPointSizeVar();
    GlyphType            GetPointType() const;
    bool                 GetOpaqueMeshIsAppropriate() const;
    bool                 GetShowInternal() const;
    int                  GetPointSizePixels() const;
    double               GetOpacity() const;

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Enum conversion functions
    static std::string SmoothingLevel_ToString(SmoothingLevel);
    static bool SmoothingLevel_FromString(const std::string &, SmoothingLevel &);
protected:
    static std::string SmoothingLevel_ToString(int);
public:
    static std::string MeshColor_ToString(MeshColor);
    static bool MeshColor_FromString(const std::string &, MeshColor &);
protected:
    static std::string MeshColor_ToString(int);
public:
    static std::string OpaqueColor_ToString(OpaqueColor);
    static bool OpaqueColor_FromString(const std::string &, OpaqueColor &);
protected:
    static std::string OpaqueColor_ToString(int);
public:
    static std::string OpaqueMode_ToString(OpaqueMode);
    static bool OpaqueMode_FromString(const std::string &, OpaqueMode &);
protected:
    static std::string OpaqueMode_ToString(int);
public:

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    bool ChangesRequireRecalculation(const MeshAttributes &, const int);
    virtual void ProcessOldVersions(DataNode *parentNode, const char *configVersion);

    // IDs that can be used to identify fields in case statements
    enum {
        ID_legendFlag = 0,
        ID_lineWidth,
        ID_meshColor,
        ID_meshColorSource,
        ID_opaqueColorSource,
        ID_opaqueMode,
        ID_pointSize,
        ID_opaqueColor,
        ID_smoothingLevel,
        ID_pointSizeVarEnabled,
        ID_pointSizeVar,
        ID_pointType,
        ID_opaqueMeshIsAppropriate,
        ID_showInternal,
        ID_pointSizePixels,
        ID_opacity,
        ID__LAST
    };

private:
    bool           legendFlag;
    int            lineWidth;
    ColorAttribute meshColor;
    int            meshColorSource;
    int            opaqueColorSource;
    int            opaqueMode;
    double         pointSize;
    ColorAttribute opaqueColor;
    int            smoothingLevel;
    bool           pointSizeVarEnabled;
    std::string    pointSizeVar;
    GlyphType      pointType;
    bool           opaqueMeshIsAppropriate;
    bool           showInternal;
    int            pointSizePixels;
    double         opacity;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define MESHATTRIBUTES_TMFS "biaiiidaibsibbid"

#endif
