// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CONTOURATTRIBUTES_H
#define CONTOURATTRIBUTES_H
#include <string>
#include <AttributeSubject.h>

#include <ColorControlPointList.h>
#include <ColorAttribute.h>
#include <ColorAttributeList.h>

// ****************************************************************************
// Class: ContourAttributes
//
// Purpose:
//    This class contains the plot attributes for the contour plot.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class ContourAttributes : public AttributeSubject
{
public:
    enum Select_by
    {
        Level,
        Value,
        Percent
    };
    enum Scaling
    {
        Linear,
        Log
    };
    enum ColoringMethod
    {
        ColorBySingleColor,
        ColorByMultipleColors,
        ColorByColorTable
    };
    static const int MAX_CONTOURS;

    // These constructors are for objects of this class
    ContourAttributes();
    ContourAttributes(const ContourAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    ContourAttributes(private_tmfs_t tmfs);
    ContourAttributes(const ContourAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~ContourAttributes();

    virtual ContourAttributes& operator = (const ContourAttributes &obj);
    virtual bool operator == (const ContourAttributes &obj) const;
    virtual bool operator != (const ContourAttributes &obj) const;
private:
    void Init();
    void Copy(const ContourAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectDefaultPalette();
    void SelectChangedColors();
    void SelectColorTableName();
    void SelectSingleColor();
    void SelectContourValue();
    void SelectContourPercent();
    void SelectMultiColor();

    // Property setting methods
    void SetDefaultPalette(const ColorControlPointList &defaultPalette_);
    void SetChangedColors(const unsignedCharVector &changedColors_);
    void SetColorType(ColoringMethod colorType_);
    void SetColorTableName(const std::string &colorTableName_);
    void SetInvertColorTable(bool invertColorTable_);
    void SetLegendFlag(bool legendFlag_);
    void SetLineWidth(int lineWidth_);
    void SetSingleColor(const ColorAttribute &singleColor_);
    void SetContourMethod(Select_by contourMethod_);
    void SetContourNLevels(int contourNLevels_);
    void SetContourValue(const doubleVector &contourValue_);
    void SetContourPercent(const doubleVector &contourPercent_);
    void SetMultiColor(const ColorAttributeList &multiColor_);
    void SetMinFlag(bool minFlag_);
    void SetMaxFlag(bool maxFlag_);
    void SetMin(double min_);
    void SetMax(double max_);
    void SetScaling(Scaling scaling_);
    void SetWireframe(bool wireframe_);

    // Property getting methods
    const ColorControlPointList &GetDefaultPalette() const;
          ColorControlPointList &GetDefaultPalette();
    const unsignedCharVector    &GetChangedColors() const;
          unsignedCharVector    &GetChangedColors();
    ColoringMethod              GetColorType() const;
    const std::string           &GetColorTableName() const;
          std::string           &GetColorTableName();
    bool                        GetInvertColorTable() const;
    bool                        GetLegendFlag() const;
    int                         GetLineWidth() const;
    const ColorAttribute        &GetSingleColor() const;
          ColorAttribute        &GetSingleColor();
    Select_by                   GetContourMethod() const;
    int                         GetContourNLevels() const;
    const doubleVector          &GetContourValue() const;
          doubleVector          &GetContourValue();
    const doubleVector          &GetContourPercent() const;
          doubleVector          &GetContourPercent();
    const ColorAttributeList    &GetMultiColor() const;
          ColorAttributeList    &GetMultiColor();
    bool                        GetMinFlag() const;
    bool                        GetMaxFlag() const;
    double                      GetMin() const;
    double                      GetMax() const;
    Scaling                     GetScaling() const;
    bool                        GetWireframe() const;

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Enum conversion functions
    static std::string Select_by_ToString(Select_by);
    static bool Select_by_FromString(const std::string &, Select_by &);
protected:
    static std::string Select_by_ToString(int);
public:
    static std::string Scaling_ToString(Scaling);
    static bool Scaling_FromString(const std::string &, Scaling &);
protected:
    static std::string Scaling_ToString(int);
public:
    static std::string ColoringMethod_ToString(ColoringMethod);
    static bool ColoringMethod_FromString(const std::string &, ColoringMethod &);
protected:
    static std::string ColoringMethod_ToString(int);
public:

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    bool ChangesRequireRecalculation(const ContourAttributes &obj);
    void SetContourValue(int i, double d);
    void SetContourPercent(int i, double d);
    void EnlargeMultiColor(int newSize);
    bool ColorIsChanged(int index) const;
    void MarkColorAsChanged(int index);
    virtual bool SetValue(const std::string &name, const int &value);
    virtual bool SetValue(const std::string &name, const doubleVector &value);

    // IDs that can be used to identify fields in case statements
    enum {
        ID_defaultPalette = 0,
        ID_changedColors,
        ID_colorType,
        ID_colorTableName,
        ID_invertColorTable,
        ID_legendFlag,
        ID_lineWidth,
        ID_singleColor,
        ID_contourMethod,
        ID_contourNLevels,
        ID_contourValue,
        ID_contourPercent,
        ID_multiColor,
        ID_minFlag,
        ID_maxFlag,
        ID_min,
        ID_max,
        ID_scaling,
        ID_wireframe,
        ID__LAST
    };

private:
    ColorControlPointList defaultPalette;
    unsignedCharVector    changedColors;
    int                   colorType;
    std::string           colorTableName;
    bool                  invertColorTable;
    bool                  legendFlag;
    int                   lineWidth;
    ColorAttribute        singleColor;
    int                   contourMethod;
    int                   contourNLevels;
    doubleVector          contourValue;
    doubleVector          contourPercent;
    ColorAttributeList    multiColor;
    bool                  minFlag;
    bool                  maxFlag;
    double                min;
    double                max;
    int                   scaling;
    bool                  wireframe;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define CONTOURATTRIBUTES_TMFS "au*isbbiaiid*d*abbddib"

#endif
