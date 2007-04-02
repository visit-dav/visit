// NOTE - This code incomplete and is for example purposes only.
//        Do not try to compile.

void
avtXXXXFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // Add a mesh called "mesh" to the metadata object.

    // Add a scalar to the metadata. Note that this plugin will
    // always expose a scalar called "temperature" to VisIt. A real
    // plugin may want to read a list of scalars from the data
    // file.
    avtScalarMetaData *smd = new avtScalarMetaData;
    smd->name = "temperature";
    smd->meshName = "mesh";
    smd->centering = AVT_ZONECENT;
    smd->hasUnits = true;
    smd->units = "Celsius";
    md->Add(smd);

    // Add other objects to the metadata object.
}
