// NOTE - This code incomplete and is for example purposes only.
//        Do not try to compile.

#include <InvalidVariableException.h>

vtkDataSet *
avtXXXXFileFormat::GetMesh(const char *meshname)
{
    // Determine which mesh to return.
    if (strcmp(meshname, "mesh") == 0)
    {
        // Create a VTK object for "mesh"
        return mesh;
    }
    else if (strcmp(meshname, "mesh2") == 0)
    {
        // Create a VTK object for "mesh2"
        return mesh2;
    }
    else
    {
        // No mesh name that we recognize.
        EXCEPTION1(InvalidVariableException, meshname);
    }

    return 0;
}
