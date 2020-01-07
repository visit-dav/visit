// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SAVEWINDOWATTRIBUTES_H
#define SAVEWINDOWATTRIBUTES_H
#include <state_exports.h>
#include <string>
#include <AttributeSubject.h>

#include <SaveSubWindowsAttributes.h>
#include <DBOptionsAttributes.h>

// ****************************************************************************
// Class: SaveWindowAttributes
//
// Purpose:
//    This class contains the attributes used for saving windows.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class STATE_API SaveWindowAttributes : public AttributeSubject
{
public:
    enum FileFormat
    {
        BMP,
        CURVE,
        JPEG,
        OBJ,
        PNG,
        POSTSCRIPT,
        POVRAY,
        PPM,
        RGB,
        STL,
        TIFF,
        ULTRA,
        VTK,
        PLY,
        EXR
    };
    enum CompressionType
    {
        None,
        PackBits,
        Jpeg,
        Deflate,
        LZW
    };
    enum ResConstraint
    {
        NoConstraint,
        EqualWidthHeight,
        ScreenProportions
    };
    enum PixelData
    {
        ColorRGB = 1,
        ColorRGBA = 2,
        Luminance = 4,
        Value = 8,
        Depth = 16
    };

    // These constructors are for objects of this class
    SaveWindowAttributes();
    SaveWindowAttributes(const SaveWindowAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    SaveWindowAttributes(private_tmfs_t tmfs);
    SaveWindowAttributes(const SaveWindowAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~SaveWindowAttributes();

    virtual SaveWindowAttributes& operator = (const SaveWindowAttributes &obj);
    virtual bool operator == (const SaveWindowAttributes &obj) const;
    virtual bool operator != (const SaveWindowAttributes &obj) const;
private:
    void Init();
    void Copy(const SaveWindowAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectOutputDirectory();
    void SelectFileName();
    void SelectLastRealFilename();
    void SelectSubWindowAtts();
    void SelectOpts();

    // Property setting methods
    void SetOutputToCurrentDirectory(bool outputToCurrentDirectory_);
    void SetOutputDirectory(const std::string &outputDirectory_);
    void SetFileName(const std::string &fileName_);
    void SetFamily(bool family_);
    void SetFormat(FileFormat format_);
    void SetWidth(int width_);
    void SetHeight(int height_);
    void SetScreenCapture(bool screenCapture_);
    void SetSaveTiled(bool saveTiled_);
    void SetQuality(int quality_);
    void SetProgressive(bool progressive_);
    void SetBinary(bool binary_);
    void SetLastRealFilename(const std::string &lastRealFilename_);
    void SetStereo(bool stereo_);
    void SetCompression(CompressionType compression_);
    void SetForceMerge(bool forceMerge_);
    void SetResConstraint(ResConstraint resConstraint_);
    void SetPixelData(int pixelData_);
    void SetAdvancedMultiWindowSave(bool advancedMultiWindowSave_);
    void SetSubWindowAtts(const SaveSubWindowsAttributes &subWindowAtts_);
    void SetOpts(const DBOptionsAttributes &opts_);

    // Property getting methods
    bool                           GetOutputToCurrentDirectory() const;
    const std::string              &GetOutputDirectory() const;
          std::string              &GetOutputDirectory();
    const std::string              &GetFileName() const;
          std::string              &GetFileName();
    bool                           GetFamily() const;
    FileFormat                     GetFormat() const;
    int                            GetWidth() const;
    int                            GetHeight() const;
    bool                           GetScreenCapture() const;
    bool                           GetSaveTiled() const;
    int                            GetQuality() const;
    bool                           GetProgressive() const;
    bool                           GetBinary() const;
    const std::string              &GetLastRealFilename() const;
          std::string              &GetLastRealFilename();
    bool                           GetStereo() const;
    CompressionType                GetCompression() const;
    bool                           GetForceMerge() const;
    ResConstraint                  GetResConstraint() const;
    int                            GetPixelData() const;
    bool                           GetAdvancedMultiWindowSave() const;
    const SaveSubWindowsAttributes &GetSubWindowAtts() const;
          SaveSubWindowsAttributes &GetSubWindowAtts();
    const DBOptionsAttributes      &GetOpts() const;
          DBOptionsAttributes      &GetOpts();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Enum conversion functions
    static std::string FileFormat_ToString(FileFormat);
    static bool FileFormat_FromString(const std::string &, FileFormat &);
protected:
    static std::string FileFormat_ToString(int);
public:
    static std::string CompressionType_ToString(CompressionType);
    static bool CompressionType_FromString(const std::string &, CompressionType &);
protected:
    static std::string CompressionType_ToString(int);
public:
    static std::string ResConstraint_ToString(ResConstraint);
    static bool ResConstraint_FromString(const std::string &, ResConstraint &);
protected:
    static std::string ResConstraint_ToString(int);
public:
    static std::string PixelData_ToString(PixelData);
    static bool PixelData_FromString(const std::string &, PixelData &);
protected:
    static std::string PixelData_ToString(int);
public:

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    bool CurrentFormatIsImageFormat(void);
    bool CurrentFormatSupportsAlpha();

    // IDs that can be used to identify fields in case statements
    enum {
        ID_outputToCurrentDirectory = 0,
        ID_outputDirectory,
        ID_fileName,
        ID_family,
        ID_format,
        ID_width,
        ID_height,
        ID_screenCapture,
        ID_saveTiled,
        ID_quality,
        ID_progressive,
        ID_binary,
        ID_lastRealFilename,
        ID_stereo,
        ID_compression,
        ID_forceMerge,
        ID_resConstraint,
        ID_pixelData,
        ID_advancedMultiWindowSave,
        ID_subWindowAtts,
        ID_opts,
        ID__LAST
    };

private:
    bool                     outputToCurrentDirectory;
    std::string              outputDirectory;
    std::string              fileName;
    bool                     family;
    int                      format;
    int                      width;
    int                      height;
    bool                     screenCapture;
    bool                     saveTiled;
    int                      quality;
    bool                     progressive;
    bool                     binary;
    std::string              lastRealFilename;
    bool                     stereo;
    int                      compression;
    bool                     forceMerge;
    int                      resConstraint;
    int                      pixelData;
    bool                     advancedMultiWindowSave;
    SaveSubWindowsAttributes subWindowAtts;
    DBOptionsAttributes      opts;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define SAVEWINDOWATTRIBUTES_TMFS "bssbiiibbibbsbibiibaa"

#endif
