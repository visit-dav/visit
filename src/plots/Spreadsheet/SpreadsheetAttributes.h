// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SPREADSHEETATTRIBUTES_H
#define SPREADSHEETATTRIBUTES_H
#include <string>
#include <AttributeSubject.h>

#include <ColorAttribute.h>
#include <PlaneAttributes.h>
#include <PickAttributes.h>
#include <math.h>

// ****************************************************************************
// Class: SpreadsheetAttributes
//
// Purpose:
//    Contains the attributes for the visual spreadsheet.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class SpreadsheetAttributes : public AttributeSubject
{
public:
    enum NormalAxis
    {
        X,
        Y,
        Z
    };

    // These constructors are for objects of this class
    SpreadsheetAttributes();
    SpreadsheetAttributes(const SpreadsheetAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    SpreadsheetAttributes(private_tmfs_t tmfs);
    SpreadsheetAttributes(const SpreadsheetAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~SpreadsheetAttributes();

    virtual SpreadsheetAttributes& operator = (const SpreadsheetAttributes &obj);
    virtual bool operator == (const SpreadsheetAttributes &obj) const;
    virtual bool operator != (const SpreadsheetAttributes &obj) const;
private:
    void Init();
    void Copy(const SpreadsheetAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectSubsetName();
    void SelectFormatString();
    void SelectColorTableName();
    void SelectTracerColor();
    void SelectSpreadsheetFont();
    void SelectCurrentPickLetter();
    void SelectPastPicks();
    void SelectPastPickLetters();

    // Property setting methods
    void SetSubsetName(const std::string &subsetName_);
    void SetFormatString(const std::string &formatString_);
    void SetUseColorTable(bool useColorTable_);
    void SetColorTableName(const std::string &colorTableName_);
    void SetShowTracerPlane(bool showTracerPlane_);
    void SetTracerColor(const ColorAttribute &tracerColor_);
    void SetNormal(NormalAxis normal_);
    void SetSliceIndex(int sliceIndex_);
    void SetSpreadsheetFont(const std::string &spreadsheetFont_);
    void SetShowPatchOutline(bool showPatchOutline_);
    void SetShowCurrentCellOutline(bool showCurrentCellOutline_);
    void SetCurrentPick(int currentPick_);
    void SetCurrentPickType(int currentPickType_);
    void SetCurrentPickValid(bool currentPickValid_);
    void SetCurrentPickLetter(const std::string &currentPickLetter_);
    void SetPastPicks(const doubleVector &pastPicks_);
    void SetPastPickLetters(const stringVector &pastPickLetters_);

    // Property getting methods
    const std::string    &GetSubsetName() const;
          std::string    &GetSubsetName();
    const std::string    &GetFormatString() const;
          std::string    &GetFormatString();
    bool                 GetUseColorTable() const;
    const std::string    &GetColorTableName() const;
          std::string    &GetColorTableName();
    bool                 GetShowTracerPlane() const;
    const ColorAttribute &GetTracerColor() const;
          ColorAttribute &GetTracerColor();
    NormalAxis           GetNormal() const;
    int                  GetSliceIndex() const;
    const std::string    &GetSpreadsheetFont() const;
          std::string    &GetSpreadsheetFont();
    bool                 GetShowPatchOutline() const;
    bool                 GetShowCurrentCellOutline() const;
    int                  GetCurrentPick() const;
    int                  GetCurrentPickType() const;
    bool                 GetCurrentPickValid() const;
    const std::string    &GetCurrentPickLetter() const;
          std::string    &GetCurrentPickLetter();
    const doubleVector   &GetPastPicks() const;
          doubleVector   &GetPastPicks();
    const stringVector   &GetPastPickLetters() const;
          stringVector   &GetPastPickLetters();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Enum conversion functions
    static std::string NormalAxis_ToString(NormalAxis);
    static bool NormalAxis_FromString(const std::string &, NormalAxis &);
protected:
    static std::string NormalAxis_ToString(int);
public:

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    bool ChangesRequireRecalculation(const SpreadsheetAttributes &) const;

    // IDs that can be used to identify fields in case statements
    enum {
        ID_subsetName = 0,
        ID_formatString,
        ID_useColorTable,
        ID_colorTableName,
        ID_showTracerPlane,
        ID_tracerColor,
        ID_normal,
        ID_sliceIndex,
        ID_spreadsheetFont,
        ID_showPatchOutline,
        ID_showCurrentCellOutline,
        ID_currentPick,
        ID_currentPickType,
        ID_currentPickValid,
        ID_currentPickLetter,
        ID_pastPicks,
        ID_pastPickLetters,
        ID__LAST
    };

private:
    std::string    subsetName;
    std::string    formatString;
    bool           useColorTable;
    std::string    colorTableName;
    bool           showTracerPlane;
    ColorAttribute tracerColor;
    int            normal;
    int            sliceIndex;
    std::string    spreadsheetFont;
    bool           showPatchOutline;
    bool           showCurrentCellOutline;
    int            currentPick;
    int            currentPickType;
    bool           currentPickValid;
    std::string    currentPickLetter;
    doubleVector   pastPicks;
    stringVector   pastPickLetters;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define SPREADSHEETATTRIBUTES_TMFS "ssbsbaiisbbiibsd*s*"

#endif
