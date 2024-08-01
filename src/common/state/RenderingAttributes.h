// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef RENDERINGATTRIBUTES_H
#define RENDERINGATTRIBUTES_H
#include <state_exports.h>
#include <string>
#include <AttributeSubject.h>

#include <ColorAttribute.h>
#include <float.h>

// ****************************************************************************
// Class: RenderingAttributes
//
// Purpose:
//    This class contains special rendering attributes like antialiasing and stero settings.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class STATE_API RenderingAttributes : public AttributeSubject
{
public:
    enum GeometryRepresentation
    {
        Surfaces,
        Wireframe,
        Points
    };
    enum StereoTypes
    {
        RedBlue,
        Interlaced,
        CrystalEyes,
        RedGreen
    };
    enum TriStateMode
    {
        Never,
        Always,
        Auto
    };
    static const int DEFAULT_SCALABLE_AUTO_THRESHOLD;
    static const int DEFAULT_SCALABLE_ACTIVATION_MODE;
    static const int DEFAULT_COMPACT_DOMAINS_ACTIVATION_MODE;
    static const int DEFAULT_COMPACT_DOMAINS_AUTO_THRESHOLD;

    // These constructors are for objects of this class
    RenderingAttributes();
    RenderingAttributes(const RenderingAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    RenderingAttributes(private_tmfs_t tmfs);
    RenderingAttributes(const RenderingAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~RenderingAttributes();

    virtual RenderingAttributes& operator = (const RenderingAttributes &obj);
    virtual bool operator == (const RenderingAttributes &obj) const;
    virtual bool operator != (const RenderingAttributes &obj) const;
private:
    void Init();
    void Copy(const RenderingAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectSpecularColor();
    void SelectStartCuePoint();
    void SelectEndCuePoint();
    void SelectAnariLibrary();
    void SelectAnariLibrarySubtype();
    void SelectAnariRendererSubtype();
    void SelectAnariDebugMethod();
    void SelectUsdDir();

    // Property setting methods
    void SetAntialiasing(bool antialiasing_);
    void SetOrderComposite(bool orderComposite_);
    void SetDepthCompositeThreads(int depthCompositeThreads_);
    void SetDepthCompositeBlocking(int depthCompositeBlocking_);
    void SetAlphaCompositeThreads(int alphaCompositeThreads_);
    void SetAlphaCompositeBlocking(int alphaCompositeBlocking_);
    void SetDepthPeeling(bool depthPeeling_);
    void SetOcclusionRatio(double occlusionRatio_);
    void SetNumberOfPeels(int numberOfPeels_);
    void SetMultiresolutionMode(bool multiresolutionMode_);
    void SetMultiresolutionCellSize(float multiresolutionCellSize_);
    void SetGeometryRepresentation(GeometryRepresentation geometryRepresentation_);
    void SetStereoRendering(bool stereoRendering_);
    void SetStereoType(StereoTypes stereoType_);
    void SetNotifyForEachRender(bool notifyForEachRender_);
    void SetScalableActivationMode(TriStateMode scalableActivationMode_);
    void SetScalableAutoThreshold(int scalableAutoThreshold_);
    void SetSpecularFlag(bool specularFlag_);
    void SetSpecularCoeff(float specularCoeff_);
    void SetSpecularPower(float specularPower_);
    void SetSpecularColor(const ColorAttribute &specularColor_);
    void SetDoShadowing(bool doShadowing_);
    void SetShadowStrength(double shadowStrength_);
    void SetDoDepthCueing(bool doDepthCueing_);
    void SetDepthCueingAutomatic(bool depthCueingAutomatic_);
    void SetStartCuePoint(const double *startCuePoint_);
    void SetEndCuePoint(const double *endCuePoint_);
    void SetCompressionActivationMode(TriStateMode compressionActivationMode_);
    void SetColorTexturingFlag(bool colorTexturingFlag_);
    void SetCompactDomainsActivationMode(TriStateMode compactDomainsActivationMode_);
    void SetCompactDomainsAutoThreshold(int compactDomainsAutoThreshold_);
    void SetOsprayRendering(bool osprayRendering_);
    void SetOspraySPP(int ospraySPP_);
    void SetOsprayAO(int osprayAO_);
    void SetOsprayShadows(bool osprayShadows_);
    void SetAnariRendering(bool anariRendering_);
    void SetAnariSPP(int anariSPP_);
    void SetAnariAO(int anariAO_);
    void SetAnariLibrary(const std::string &anariLibrary_);
    void SetAnariLibrarySubtype(const std::string &anariLibrarySubtype_);
    void SetAnariRendererSubtype(const std::string &anariRendererSubtype_);
    void SetUseAnariDenoiser(bool useAnariDenoiser_);
    void SetAnariLightFalloff(float anariLightFalloff_);
    void SetAnariAmbientIntensity(float anariAmbientIntensity_);
    void SetAnariMaxDepth(int anariMaxDepth_);
    void SetAnariRValue(float anariRValue_);
    void SetAnariDebugMethod(const std::string &anariDebugMethod_);
    void SetUsdDir(const std::string &usdDir_);
    void SetUsdAtCommit(bool usdAtCommit_);
    void SetUsdOutputBinary(bool usdOutputBinary_);
    void SetUsdOutputMaterial(bool usdOutputMaterial_);
    void SetUsdOutputPreviewSurface(bool usdOutputPreviewSurface_);
    void SetUsdOutputMDL(bool usdOutputMDL_);
    void SetUsdOutputMDLColors(bool usdOutputMDLColors_);
    void SetUsdOutputDisplayColors(bool usdOutputDisplayColors_);
    void SetUsingUsdDevice(bool usingUsdDevice_);

    // Property getting methods
    bool                 GetAntialiasing() const;
    bool                 GetOrderComposite() const;
    int                  GetDepthCompositeThreads() const;
    int                  GetDepthCompositeBlocking() const;
    int                  GetAlphaCompositeThreads() const;
    int                  GetAlphaCompositeBlocking() const;
    bool                 GetDepthPeeling() const;
    double               GetOcclusionRatio() const;
    int                  GetNumberOfPeels() const;
    bool                 GetMultiresolutionMode() const;
    float                GetMultiresolutionCellSize() const;
    GeometryRepresentation GetGeometryRepresentation() const;
    bool                 GetStereoRendering() const;
    StereoTypes          GetStereoType() const;
    bool                 GetNotifyForEachRender() const;
    TriStateMode         GetScalableActivationMode() const;
    int                  GetScalableAutoThreshold() const;
    bool                 GetSpecularFlag() const;
    float                GetSpecularCoeff() const;
    float                GetSpecularPower() const;
    const ColorAttribute &GetSpecularColor() const;
          ColorAttribute &GetSpecularColor();
    bool                 GetDoShadowing() const;
    double               GetShadowStrength() const;
    bool                 GetDoDepthCueing() const;
    bool                 GetDepthCueingAutomatic() const;
    const double         *GetStartCuePoint() const;
          double         *GetStartCuePoint();
    const double         *GetEndCuePoint() const;
          double         *GetEndCuePoint();
    TriStateMode         GetCompressionActivationMode() const;
    bool                 GetColorTexturingFlag() const;
    TriStateMode         GetCompactDomainsActivationMode() const;
    int                  GetCompactDomainsAutoThreshold() const;
    bool                 GetOsprayRendering() const;
    int                  GetOspraySPP() const;
    int                  GetOsprayAO() const;
    bool                 GetOsprayShadows() const;
    bool                 GetAnariRendering() const;
    int                  GetAnariSPP() const;
    int                  GetAnariAO() const;
    const std::string    &GetAnariLibrary() const;
          std::string    &GetAnariLibrary();
    const std::string    &GetAnariLibrarySubtype() const;
          std::string    &GetAnariLibrarySubtype();
    const std::string    &GetAnariRendererSubtype() const;
          std::string    &GetAnariRendererSubtype();
    bool                 GetUseAnariDenoiser() const;
    float                GetAnariLightFalloff() const;
    float                GetAnariAmbientIntensity() const;
    int                  GetAnariMaxDepth() const;
    float                GetAnariRValue() const;
    const std::string    &GetAnariDebugMethod() const;
          std::string    &GetAnariDebugMethod();
    const std::string    &GetUsdDir() const;
          std::string    &GetUsdDir();
    bool                 GetUsdAtCommit() const;
    bool                 GetUsdOutputBinary() const;
    bool                 GetUsdOutputMaterial() const;
    bool                 GetUsdOutputPreviewSurface() const;
    bool                 GetUsdOutputMDL() const;
    bool                 GetUsdOutputMDLColors() const;
    bool                 GetUsdOutputDisplayColors() const;
    bool                 GetUsingUsdDevice() const;

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Enum conversion functions
    static std::string GeometryRepresentation_ToString(GeometryRepresentation);
    static bool GeometryRepresentation_FromString(const std::string &, GeometryRepresentation &);
protected:
    static std::string GeometryRepresentation_ToString(int);
public:
    static std::string StereoTypes_ToString(StereoTypes);
    static bool StereoTypes_FromString(const std::string &, StereoTypes &);
protected:
    static std::string StereoTypes_ToString(int);
public:
    static std::string TriStateMode_ToString(TriStateMode);
    static bool TriStateMode_FromString(const std::string &, TriStateMode &);
protected:
    static std::string TriStateMode_ToString(int);
public:

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    static int GetEffectiveScalableThreshold(TriStateMode mode, int autoThreshold);
    static int GetEffectiveCompactDomainsThreshold(TriStateMode mode, int autoThreshold);

    // IDs that can be used to identify fields in case statements
    enum {
        ID_antialiasing = 0,
        ID_orderComposite,
        ID_depthCompositeThreads,
        ID_depthCompositeBlocking,
        ID_alphaCompositeThreads,
        ID_alphaCompositeBlocking,
        ID_depthPeeling,
        ID_occlusionRatio,
        ID_numberOfPeels,
        ID_multiresolutionMode,
        ID_multiresolutionCellSize,
        ID_geometryRepresentation,
        ID_stereoRendering,
        ID_stereoType,
        ID_notifyForEachRender,
        ID_scalableActivationMode,
        ID_scalableAutoThreshold,
        ID_specularFlag,
        ID_specularCoeff,
        ID_specularPower,
        ID_specularColor,
        ID_doShadowing,
        ID_shadowStrength,
        ID_doDepthCueing,
        ID_depthCueingAutomatic,
        ID_startCuePoint,
        ID_endCuePoint,
        ID_compressionActivationMode,
        ID_colorTexturingFlag,
        ID_compactDomainsActivationMode,
        ID_compactDomainsAutoThreshold,
        ID_osprayRendering,
        ID_ospraySPP,
        ID_osprayAO,
        ID_osprayShadows,
        ID_anariRendering,
        ID_anariSPP,
        ID_anariAO,
        ID_anariLibrary,
        ID_anariLibrarySubtype,
        ID_anariRendererSubtype,
        ID_useAnariDenoiser,
        ID_anariLightFalloff,
        ID_anariAmbientIntensity,
        ID_anariMaxDepth,
        ID_anariRValue,
        ID_anariDebugMethod,
        ID_usdDir,
        ID_usdAtCommit,
        ID_usdOutputBinary,
        ID_usdOutputMaterial,
        ID_usdOutputPreviewSurface,
        ID_usdOutputMDL,
        ID_usdOutputMDLColors,
        ID_usdOutputDisplayColors,
        ID_usingUsdDevice,
        ID__LAST
    };

private:
    bool           antialiasing;
    bool           orderComposite;
    int            depthCompositeThreads;
    int            depthCompositeBlocking;
    int            alphaCompositeThreads;
    int            alphaCompositeBlocking;
    bool           depthPeeling;
    double         occlusionRatio;
    int            numberOfPeels;
    bool           multiresolutionMode;
    float          multiresolutionCellSize;
    int            geometryRepresentation;
    bool           stereoRendering;
    int            stereoType;
    bool           notifyForEachRender;
    int            scalableActivationMode;
    int            scalableAutoThreshold;
    bool           specularFlag;
    float          specularCoeff;
    float          specularPower;
    ColorAttribute specularColor;
    bool           doShadowing;
    double         shadowStrength;
    bool           doDepthCueing;
    bool           depthCueingAutomatic;
    double         startCuePoint[3];
    double         endCuePoint[3];
    int            compressionActivationMode;
    bool           colorTexturingFlag;
    int            compactDomainsActivationMode;
    int            compactDomainsAutoThreshold;
    bool           osprayRendering;
    int            ospraySPP;
    int            osprayAO;
    bool           osprayShadows;
    bool           anariRendering;
    int            anariSPP;
    int            anariAO;
    std::string    anariLibrary;
    std::string    anariLibrarySubtype;
    std::string    anariRendererSubtype;
    bool           useAnariDenoiser;
    float          anariLightFalloff;
    float          anariAmbientIntensity;
    int            anariMaxDepth;
    float          anariRValue;
    std::string    anariDebugMethod;
    std::string    usdDir;
    bool           usdAtCommit;
    bool           usdOutputBinary;
    bool           usdOutputMaterial;
    bool           usdOutputPreviewSurface;
    bool           usdOutputMDL;
    bool           usdOutputMDLColors;
    bool           usdOutputDisplayColors;
    bool           usingUsdDevice;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define RENDERINGATTRIBUTES_TMFS "bbiiiibdibfibibiibffabdbbDDibiibiibbiisssbffifssbbbbbbbb"

#endif
