// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VECTORATTRIBUTES_H
#define VECTORATTRIBUTES_H
#include <string>
#include <AttributeSubject.h>

#include <ColorAttribute.h>

// ****************************************************************************
// Class: VectorAttributes
//
// Purpose:
//    Attributes for the vector plot
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class VectorAttributes : public AttributeSubject
{
public:
    enum Quality
    {
        Fast,
        High
    };
    enum OriginType
    {
        Head,
        Middle,
        Tail
    };
    enum GlyphType
    {
        Arrow,
        Ellipsoid
    };
    enum LineStem
    {
        Cylinder,
        Line
    };
    enum LimitsMode
    {
        OriginalData,
        CurrentPlot
    };
    enum GlyphLocation
    {
        AdaptsToMeshResolution,
        UniformInSpace
    };

    // These constructors are for objects of this class
    VectorAttributes();
    VectorAttributes(const VectorAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    VectorAttributes(private_tmfs_t tmfs);
    VectorAttributes(const VectorAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~VectorAttributes();

    virtual VectorAttributes& operator = (const VectorAttributes &obj);
    virtual bool operator == (const VectorAttributes &obj) const;
    virtual bool operator != (const VectorAttributes &obj) const;
private:
    void Init();
    void Copy(const VectorAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectColorTableName();
    void SelectVectorColor();

    // Property setting methods
    void SetGlyphLocation(GlyphLocation glyphLocation_);
    void SetUseStride(bool useStride_);
    void SetNVectors(int nVectors_);
    void SetStride(int stride_);
    void SetOrigOnly(bool origOnly_);
    void SetLimitsMode(LimitsMode limitsMode_);
    void SetMinFlag(bool minFlag_);
    void SetMin(double min_);
    void SetMaxFlag(bool maxFlag_);
    void SetMax(double max_);
    void SetColorByMagnitude(bool colorByMagnitude_);
    void SetColorTableName(const std::string &colorTableName_);
    void SetInvertColorTable(bool invertColorTable_);
    void SetVectorColor(const ColorAttribute &vectorColor_);
    void SetUseLegend(bool useLegend_);
    void SetScale(double scale_);
    void SetScaleByMagnitude(bool scaleByMagnitude_);
    void SetAutoScale(bool autoScale_);
    void SetGlyphType(GlyphType glyphType_);
    void SetHeadOn(bool headOn_);
    void SetHeadSize(double headSize_);
    void SetLineStem(LineStem lineStem_);
    void SetLineWidth(int lineWidth_);
    void SetStemWidth(double stemWidth_);
    void SetVectorOrigin(OriginType vectorOrigin_);
    void SetGeometryQuality(Quality geometryQuality_);
    void SetAnimationStep(int animationStep_);

    // Property getting methods
    GlyphLocation        GetGlyphLocation() const;
    bool                 GetUseStride() const;
    int                  GetNVectors() const;
    int                  GetStride() const;
    bool                 GetOrigOnly() const;
    LimitsMode           GetLimitsMode() const;
    bool                 GetMinFlag() const;
    double               GetMin() const;
    bool                 GetMaxFlag() const;
    double               GetMax() const;
    bool                 GetColorByMagnitude() const;
    const std::string    &GetColorTableName() const;
          std::string    &GetColorTableName();
    bool                 GetInvertColorTable() const;
    const ColorAttribute &GetVectorColor() const;
          ColorAttribute &GetVectorColor();
    bool                 GetUseLegend() const;
    double               GetScale() const;
    bool                 GetScaleByMagnitude() const;
    bool                 GetAutoScale() const;
    GlyphType            GetGlyphType() const;
    bool                 GetHeadOn() const;
    double               GetHeadSize() const;
    LineStem             GetLineStem() const;
    int                  GetLineWidth() const;
    double               GetStemWidth() const;
    OriginType           GetVectorOrigin() const;
    Quality              GetGeometryQuality() const;
    int                  GetAnimationStep() const;

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Enum conversion functions
    static std::string Quality_ToString(Quality);
    static bool Quality_FromString(const std::string &, Quality &);
protected:
    static std::string Quality_ToString(int);
public:
    static std::string OriginType_ToString(OriginType);
    static bool OriginType_FromString(const std::string &, OriginType &);
protected:
    static std::string OriginType_ToString(int);
public:
    static std::string GlyphType_ToString(GlyphType);
    static bool GlyphType_FromString(const std::string &, GlyphType &);
protected:
    static std::string GlyphType_ToString(int);
public:
    static std::string LineStem_ToString(LineStem);
    static bool LineStem_FromString(const std::string &, LineStem &);
protected:
    static std::string LineStem_ToString(int);
public:
    static std::string LimitsMode_ToString(LimitsMode);
    static bool LimitsMode_FromString(const std::string &, LimitsMode &);
protected:
    static std::string LimitsMode_ToString(int);
public:
    static std::string GlyphLocation_ToString(GlyphLocation);
    static bool GlyphLocation_FromString(const std::string &, GlyphLocation &);
protected:
    static std::string GlyphLocation_ToString(int);
public:

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    virtual void ProcessOldVersions(DataNode *parentNode, const char *configVersion);
    bool ChangesRequireRecalculation(const VectorAttributes &obj);
    double GetAnimationScale() const;

    // IDs that can be used to identify fields in case statements
    enum {
        ID_glyphLocation = 0,
        ID_useStride,
        ID_nVectors,
        ID_stride,
        ID_origOnly,
        ID_limitsMode,
        ID_minFlag,
        ID_min,
        ID_maxFlag,
        ID_max,
        ID_colorByMagnitude,
        ID_colorTableName,
        ID_invertColorTable,
        ID_vectorColor,
        ID_useLegend,
        ID_scale,
        ID_scaleByMagnitude,
        ID_autoScale,
        ID_glyphType,
        ID_headOn,
        ID_headSize,
        ID_lineStem,
        ID_lineWidth,
        ID_stemWidth,
        ID_vectorOrigin,
        ID_geometryQuality,
        ID_animationStep,
        ID__LAST
    };

private:
    int            glyphLocation;
    bool           useStride;
    int            nVectors;
    int            stride;
    bool           origOnly;
    int            limitsMode;
    bool           minFlag;
    double         min;
    bool           maxFlag;
    double         max;
    bool           colorByMagnitude;
    std::string    colorTableName;
    bool           invertColorTable;
    ColorAttribute vectorColor;
    bool           useLegend;
    double         scale;
    bool           scaleByMagnitude;
    bool           autoScale;
    int            glyphType;
    bool           headOn;
    double         headSize;
    int            lineStem;
    int            lineWidth;
    double         stemWidth;
    int            vectorOrigin;
    int            geometryQuality;
    int            animationStep;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define VECTORATTRIBUTES_TMFS "ibiibibdbdbsbabdbbibdiidiii"

#endif
