// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVTSPECIESMETADATA_H
#define AVTSPECIESMETADATA_H
#include <dbatts_exports.h>
#include <string>
#include <AttributeSubject.h>

class avtMatSpeciesMetaData;

// ****************************************************************************
// Class: avtSpeciesMetaData
//
// Purpose:
//    Contains species metadata attributes
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class DBATTS_API avtSpeciesMetaData : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    avtSpeciesMetaData();
    avtSpeciesMetaData(const avtSpeciesMetaData &obj);
protected:
    // These constructors are for objects derived from this class
    avtSpeciesMetaData(private_tmfs_t tmfs);
    avtSpeciesMetaData(const avtSpeciesMetaData &obj, private_tmfs_t tmfs);
public:
    virtual ~avtSpeciesMetaData();

    virtual avtSpeciesMetaData& operator = (const avtSpeciesMetaData &obj);
    virtual bool operator == (const avtSpeciesMetaData &obj) const;
    virtual bool operator != (const avtSpeciesMetaData &obj) const;
private:
    void Init();
    void Copy(const avtSpeciesMetaData &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectSpecies();

    // Property setting methods

    // Property getting methods
    const AttributeGroupVector &GetSpecies() const;
          AttributeGroupVector &GetSpecies();


    // Attributegroup convenience methods
    void AddSpecies(const avtMatSpeciesMetaData &);
    void ClearSpecies();
    void RemoveSpecies(int i);
    int  GetNumSpecies() const;
    avtMatSpeciesMetaData &GetSpecies(int i);
    const avtMatSpeciesMetaData &GetSpecies(int i) const;

    avtMatSpeciesMetaData &operator [] (int i);
    const avtMatSpeciesMetaData &operator [] (int i) const;


    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    avtSpeciesMetaData(const std::string &n, const std::string &meshn, const std::string &matn, int nummat, const intVector &ns, const std::vector<stringVector> &sn);
    void Print(ostream &, int = 0) const;

    // IDs that can be used to identify fields in case statements
    enum {
        ID_name = 0,
        ID_originalName,
        ID_validVariable,
        ID_meshName,
        ID_materialName,
        ID_numMaterials,
        ID_species,
        ID__LAST
    };

protected:
    AttributeGroup *CreateSubAttributeGroup(int index);
public:
    std::string          name;
    std::string          originalName;
    bool                 validVariable;
    std::string          meshName;
    std::string          materialName;
    int                  numMaterials;
private:
    AttributeGroupVector species;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define AVTSPECIESMETADATA_TMFS "ssbssia*"

#endif
