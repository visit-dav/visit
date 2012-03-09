#ifndef MATERIAL_PROPERTIES_H
#define MATERIAL_PROPERTIES_H
#include <string>
#include <vector>

#define MATERIALPROPERTIES_NUM_ITEMS 80

// ****************************************************************************
// Class: MaterialProperties
//
// Purpose:
//   This class contains material properties for the material cards read from
//   a Dyna3D file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 26 12:23:33 PST 2012
//
// Modifications:
//   
// ****************************************************************************

struct MaterialProperties
{
    MaterialProperties();

    void ReadCard(void (*readline)(char *, int, void *), void *cbdata);
    void WriteCard(void (*writeline)(const char *, void *), void *cbdata) const;

    bool        enabled;
    int         materialNumber;
    std::string materialName;
    std::string eosName;
    double      density;
    double      strength;
    double      equivalentPlasticStrain;
    double      materialInformation[MATERIALPROPERTIES_NUM_ITEMS];
    std::vector<std::string> materialLines;
private:
    bool RequiresEOS() const;
};

typedef std::vector<MaterialProperties> MaterialPropertiesVector;

MaterialPropertiesVector ReadMaterialPropertiesFile(const std::string &filename);

void OverrideMaterialProperties(MaterialPropertiesVector &mats,
                                const MaterialPropertiesVector &overrides);

void SetEnabledMaterials(MaterialPropertiesVector &props, const std::string &matlist);

void PrintMaterialData(FILE *f, const MaterialPropertiesVector &props);

#endif
