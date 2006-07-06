// NOTE - This code incomplete and requires underlined portions
// to be replaced with code to read values from your file format.

#include <avtMaterial.h>

// STMD version of GetAuxiliaryData.
void *
avtXXXXFileFormat::GetAuxiliaryData(const char *var,
    int domain, const char *type, void *,
    DestructorFunction &df)
{
    void *retval = 0;

    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        int dims[3] = {1,1,1}, ndims = 1;
        // Structured mesh case
        ndims = MESH DIMENSION, 2 OR 3;
        dims[0] = NUMBER OF ZONES IN X DIMENSION;
        dims[1] = NUMBER OF ZONES IN Y DIMENSION;
        dims[2] = NUMBER OF ZONES IN Z DIMENSION, OR 1 IF 2D;

        // Unstructured mesh case
        dims[0] = NUMBER OF ZONES IN THE MESH
        ndims = 1;        

        // Read the number of materials from the file. This
        // must have already been read from the file when
        // PopulateDatabaseMetaData was called.
        int nmats = NUMBER OF MATERIALS;

        // The matnos array contains the list of numbers that
        // are associated with particular materials. For example,
        // matnos[0] is the number that will be associated with
        // the first material and any time it is seen in the
        // matlist array, that number should be taken to mean
        // material 1. The numbers in the matnos array must
        // all be greater than or equal to 1.
        int *matnos = new int[nmats];
        READ nmats INTEGER VALUES INTO THE matnos ARRAY.

        // Read the material names from your file format or
        // make up names for the materials. Use the same
        // approach as when you created material names in
        // the PopulateDatabaseMetaData method.
        char **names = new char *[nmats];
        READ MATERIAL NAMES FROM YOUR FILE FORMAT UNTIL EACH
        ELEMENT OF THE names ARRAY POINTS TO ITS OWN STRING.

        // Read the matlist array, which tells what the material
        // is for each zone in the mesh.
        int nzones = dims[0] * dims[1] * dims[2];
        int *matlist = new int[nzones];
        READ nzones INTEGERS INTO THE matlist array.

        // Optionally create mix_mat, mix_next, mix_zone, mix_vf
        // arrays and read their contents from the file format.

        // Use the information to create an avtMaterial object.
        avtMaterial *mat = new avtMaterial(
            nmats,
            matnos,
            names,
            ndims,
            dims,
            0,
            matlist,
            0, // length of mix arrays
            0, // mix_mat array
            0, // mix_next array
            0, // mix_zone array
            0  // mix_vf array
            );

        // Clean up.
        delete [] matlist;
        delete [] matnos;
        for(int i = 0; i < nmats; ++i)
            delete [] names[i];
        delete [] names;

        // Set the return values.
        retval = (void *)mat;
        df = avtMaterial::Destruct;
    }

    return retval;
}
