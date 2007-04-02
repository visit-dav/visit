// NOTE - This code incomplete and is for example purposes only.
//        Do not try to compile.

void
avtXXXXFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // Add a mesh called "mesh" to the metadata object.

    // Add a material to the metadata. Note that this plugin will
    // always expose a material called "mat1" to VisIt. A real
    // plugin may want to use from the data file to construct
    // a material.
    avtMaterialMetaData *matmd = new avtMaterialMetaData;
    matmd->name = "mat1";
    matmd->meshName = "mesh";
    matmd->numMaterials = 4;
    matmd->materialNames.push_back("Steel");
    matmd->materialNames.push_back("Wood");
    matmd->materialNames.push_back("Glue");
    matmd->materialNames.push_back("Air");
    md->Add(matmd);

    // Add other objects to the metadata object.
}
