// NOTE - This code incomplete and is for example purposes only.
//        Do not try to compile.

void
avtXXXXFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // Add a point mesh to the metadata. Note that this plugin will
    // always expose a mesh called "particles" to VisIt. A real
    // plugin may want to read a list of meshes from the data
    // file.
    avtMeshMetaData *mmd = new avtMeshMetaData;
    mmd->name = "particles";
    mmd->spatialDimension = 3;
    mmd->topologicalDimension = 0;
    mmd->meshType = AVT_POINT_MESH;
    mmd->numBlocks = 1;
    md->Add(mmd);

    // Add other objects to the metadata object.
}
