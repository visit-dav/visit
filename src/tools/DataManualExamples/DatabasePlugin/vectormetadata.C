// NOTE - This code incomplete and is for example purposes only.
//        Do not try to compile.

void
avtXXXXFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // Add a mesh called "mesh" to the metadata object.

    // Add a vector to the metadata. Note that this plugin will
    // always expose a vector called "velocity" to VisIt. A real
    // plugin may want to read a list of vectors from the data
    // file.
    avtVectorMetaData *vmd = new avtVectorMetaData;
    vmd->name = "velocity";
    vmd->meshName = "mesh";
    vmd->centering = AVT_ZONECENT;
    vmd->hasUnits = true;
    vmd->units = "m/s";
    vmd->varDim = 3;
    md->Add(vmd);

    // Add other objects to the metadata object.
}
