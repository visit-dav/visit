#ifndef MATERIAL_PROPERTIES_H
#define MATERIAL_PROPERTIES_H
#include <string>
#include <vector>

struct MaterialProperties
{
    MaterialProperties();

    bool        enabled;
    int         materialNumber;
    std::string materialName;
    double      density;
    double      strength;
    double      equivalentPlasticStrain;
};

typedef std::vector<MaterialProperties> MaterialPropertiesVector;

MaterialPropertiesVector ReadMaterialPropertiesFile(const std::string &filename);

void OverrideMaterialProperties(MaterialPropertiesVector &mats,
                                const MaterialPropertiesVector &overrides);

void SetEnabledMaterials(MaterialPropertiesVector &props, const std::string &matlist);

void PrintMaterialData(FILE *f, const MaterialPropertiesVector &props);

#endif
