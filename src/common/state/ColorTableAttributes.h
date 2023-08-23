// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef COLORTABLEATTRIBUTES_H
#define COLORTABLEATTRIBUTES_H
#include <state_exports.h>
#include <string>
#include <AttributeSubject.h>

class ColorControlPointList;

// ****************************************************************************
// Class: ColorTableAttributes
//
// Purpose:
//    This class contains the list of colortables.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class STATE_API ColorTableAttributes : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    ColorTableAttributes();
    ColorTableAttributes(const ColorTableAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    ColorTableAttributes(private_tmfs_t tmfs);
    ColorTableAttributes(const ColorTableAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~ColorTableAttributes();

    virtual ColorTableAttributes& operator = (const ColorTableAttributes &obj);
    virtual bool operator == (const ColorTableAttributes &obj) const;
    virtual bool operator != (const ColorTableAttributes &obj) const;
private:
    void Init();
    void Copy(const ColorTableAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectColorTableNames();
    void SelectColorTableActiveFlags();
    void SelectColorTables();
    void SelectDefaultContinuous();
    void SelectDefaultDiscrete();
    void SelectTagListNames();
    void SelectTagListActive();
    void SelectTagListNumRefs();
    void SelectTagListTableItemFlag();
    void SelectTagChangesTag();
    void SelectTagChangesType();
    void SelectTagChangesCTName();

    // Property setting methods
    void SetColorTableNames(const stringVector &colorTableNames_);
    void SetColorTableActiveFlags(const intVector &colorTableActiveFlags_);
    void SetDefaultContinuous(const std::string &defaultContinuous_);
    void SetDefaultDiscrete(const std::string &defaultDiscrete_);
    void SetChangesMade(bool changesMade_);
    void SetTagsMatchAny(bool tagsMatchAny_);
    void SetTagListNames(const stringVector &tagListNames_);
    void SetTagListActive(const intVector &tagListActive_);
    void SetTagListNumRefs(const intVector &tagListNumRefs_);
    void SetTagListTableItemFlag(const intVector &tagListTableItemFlag_);
    void SetTagChangesTag(const stringVector &tagChangesTag_);
    void SetTagChangesType(const intVector &tagChangesType_);
    void SetTagChangesCTName(const stringVector &tagChangesCTName_);

    // Property getting methods
    const stringVector &GetColorTableNames() const;
          stringVector &GetColorTableNames();
    const intVector    &GetColorTableActiveFlags() const;
          intVector    &GetColorTableActiveFlags();
    const AttributeGroupVector &GetColorTables() const;
          AttributeGroupVector &GetColorTables();
    const std::string  &GetDefaultContinuous() const;
          std::string  &GetDefaultContinuous();
    const std::string  &GetDefaultDiscrete() const;
          std::string  &GetDefaultDiscrete();
    bool               GetChangesMade() const;
    bool               GetTagsMatchAny() const;
    const stringVector &GetTagListNames() const;
          stringVector &GetTagListNames();
    const intVector    &GetTagListActive() const;
          intVector    &GetTagListActive();
    const intVector    &GetTagListNumRefs() const;
          intVector    &GetTagListNumRefs();
    const intVector    &GetTagListTableItemFlag() const;
          intVector    &GetTagListTableItemFlag();
    const stringVector &GetTagChangesTag() const;
          stringVector &GetTagChangesTag();
    const intVector    &GetTagChangesType() const;
          intVector    &GetTagChangesType();
    const stringVector &GetTagChangesCTName() const;
          stringVector &GetTagChangesCTName();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);


    // Attributegroup convenience methods
    void AddColorTables(const ColorControlPointList &);
    void ClearColorTables();
    void RemoveColorTables(int i);
    int  GetNumColorTables() const;
    ColorControlPointList &GetColorTables(int i);
    const ColorControlPointList &GetColorTables(int i) const;

    ColorControlPointList &operator [] (int i);
    const ColorControlPointList &operator [] (int i) const;


    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    int GetColorTableIndex(const std::string &name) const;
    const ColorControlPointList *GetColorControlPoints(int index) const;
    const ColorControlPointList *GetColorControlPoints(const std::string &name) const;
    void AddColorTable(const std::string &name, const ColorControlPointList &cpts);
    void RemoveColorTable(const std::string &name);
    void RemoveColorTable(int index);
    void SetColorTableActiveFlag(int index, bool val);
    bool GetColorTableActiveFlag(int index);
    void MergeTagChanges(const stringVector tagChangesTagFromNode, const intVector tagChangesTypeFromNode, const stringVector tagChangesCTNameFromNode);
    void addTagToColorTable(const std::string ctName, const std::string tagName, ColorControlPointList* ccpl);
    std::pair<bool, std::string> removeTagFromColorTable(const std::string ctName, const std::string tagName, ColorControlPointList* ccpl);
    bool AllTagsSelected();
    void EnableDisableAllTags(bool enable);
    int GetIndexOfTag(const std::string tagname);
    void CreateTagListEntry(const std::string tagname, const bool active, const int numrefs, const bool tagTableItemExists);
    void RemoveTagListEntry(const int index);
    void SelectTagList();
    void CreateTagChangesEntry(const std::string tagname, const int addOrRemove, const std::string ctName);
    void RemoveTagChangesEntry(const int index);
    void SelectTagChanges();
    void SetTagActive(const std::string tagname, const bool active);
    void SetTagActive(const int index, const bool active);
    bool GetTagActive(const std::string tagname);
    void IncrementTagNumRefs(const std::string tagname);
    void IncrementTagNumRefs(const int index);
    void DecrementTagNumRefs(const std::string tagname);
    void DecrementTagNumRefs(const int index);
    int GetTagNumRefs(const std::string tagname);
    void SetTagTableItemFlag(const std::string tagname, const bool tagTableItemExists);
    void SetTagTableItemFlag(const int index, const bool tagTableItemExists);
    bool GetTagTableItemFlag(const std::string tagname);
    bool CheckTagInTagList(const std::string tagname);
    void RemoveUnusedTagsFromTagTable(std::vector<std::string> &removedTags);
    void GetNewTagNames(std::vector<std::string> &tagsToAdd);
    bool FilterTableByTag(const ColorControlPointList &ccpl);
    void FilterTablesByTag();
    void PrintTagList();
    virtual void ProcessOldVersions(DataNode *parentNode, const char *configVersion);

    // IDs that can be used to identify fields in case statements
    enum {
        ID_colorTableNames = 0,
        ID_colorTableActiveFlags,
        ID_colorTables,
        ID_defaultContinuous,
        ID_defaultDiscrete,
        ID_changesMade,
        ID_tagsMatchAny,
        ID_tagListNames,
        ID_tagListActive,
        ID_tagListNumRefs,
        ID_tagListTableItemFlag,
        ID_tagChangesTag,
        ID_tagChangesType,
        ID_tagChangesCTName,
        ID__LAST
    };

protected:
    AttributeGroup *CreateSubAttributeGroup(int index);
private:
    stringVector         colorTableNames;
    intVector            colorTableActiveFlags;
    AttributeGroupVector colorTables;
    std::string          defaultContinuous;
    std::string          defaultDiscrete;
    bool                 changesMade;
    bool                 tagsMatchAny;
    stringVector         tagListNames;
    intVector            tagListActive;
    intVector            tagListNumRefs;
    intVector            tagListTableItemFlag;
    stringVector         tagChangesTag;
    intVector            tagChangesType;
    stringVector         tagChangesCTName;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define COLORTABLEATTRIBUTES_TMFS "s*i*a*ssbbs*i*i*i*s*i*s*"

#endif
