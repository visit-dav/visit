// ************************************************************************* //
//                            avtSimDBFileFormat.C                           //
// ************************************************************************* //

#include <avtSimDBFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <InvalidVariableException.h>

#include <avtDatabase.h>
#include <visitstream.h>

using     std::string;

extern int p_nx;
extern int p_ny;
extern int p_nz;
extern double *p_xcoords;
extern double *p_ycoords;
extern double *p_zcoords;
extern double *p_zvalues;
extern double *p_nvalues;

int *nx;
int *ny;
int *nz;
double **xcoords;
double **ycoords;
double **zcoords;
double **nvalues;
double **zvalues;


// ****************************************************************************
//  Method: avtSimDB constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   March 25, 2004
//
// ****************************************************************************

avtSimDBFileFormat::avtSimDBFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    nx = &p_nx;
    ny = &p_ny;
    nz = &p_nz;
    xcoords = &p_xcoords;
    ycoords = &p_ycoords;
    zcoords = &p_zcoords;
    zvalues = &p_zvalues;
    nvalues = &p_nvalues;
}

// ****************************************************************************
//  Method: avtSimDBFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 25, 2004
//
// ****************************************************************************

void
avtSimDBFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtSimDBFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 25, 2004
//
// ****************************************************************************

void
avtSimDBFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    md->SetIsSimulation(true);
    md->SetSimHost(host);
    md->SetSimPort(port);
    AddMeshToMetaData(md, "mesh", AVT_CURVILINEAR_MESH, NULL,
                      1, 0, 3, 3);
    AddScalarVarToMetaData(md, "speed", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "density", "mesh", AVT_ZONECENT);

    //
    // CODE TO ADD A MESH
    //
    // string meshname = ...
    //
    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH,
    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH
    // avtMeshType mt = AVT_RECTILINEAR_MESH;
    //
    // int nblocks = YOU_MUST_DECIDE;
    // int block_origin = 0;
    // int spatial_dimension = 2;
    // int topological_dimension = 2;
    // float *extents = NULL;
    //
    // Here's the call that tells the meta-data object that we have a mesh:
    //
    // AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
    //                   spatial_dimension, topological_dimension);
    //

    //
    // CODE TO ADD A SCALAR VARIABLE
    //
    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes
    // string varname = ...
    //
    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT
    // avtCentering cent = AVT_NODECENT;
    //
    //
    // Here's the call that tells the meta-data object that we have a var:
    //
    // AddScalarVarToMetaData(md, varname, mesh_for_this_var, cent);
    //

    //
    // CODE TO ADD A VECTOR VARIABLE
    //
    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes
    // string varname = ...
    // int vector_dim = 2;
    //
    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT
    // avtCentering cent = AVT_NODECENT;
    //
    //
    // Here's the call that tells the meta-data object that we have a var:
    //
    // AddVectorVarToMetaData(md, varname, mesh_for_this_var, cent,vector_dim);
    //

    //
    // CODE TO ADD A TENSOR VARIABLE
    //
    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes
    // string varname = ...
    // int tensor_dim = 9;
    //
    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT
    // avtCentering cent = AVT_NODECENT;
    //
    //
    // Here's the call that tells the meta-data object that we have a var:
    //
    // AddTensorVarToMetaData(md, varname, mesh_for_this_var, cent,tensor_dim);
    //

    //
    // CODE TO ADD A MATERIAL
    //
    // string mesh_for_mat = meshname; // ??? -- could be multiple meshes
    // string matname = ...
    // int nmats = ...;
    // vector<string> mnames;
    // for (int i = 0 ; i < nmats ; i++)
    // {
    //     char str[32];
    //     sprintf(str, "mat%d", i);
    //     -- or -- 
    //     strcpy(str, "Aluminum");
    //     mnames.push_back(str);
    // }
    // 
    // Here's the call that tells the meta-data object that we have a mat:
    //
    // AddMaterialToMetaData(md, matname, mesh_for_mat, nmats, mnames);
    //
}


// ****************************************************************************
//  Method: avtSimDBFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 25, 2004
//
// ****************************************************************************
vtkDataSet *
avtSimDBFileFormat::GetMesh(int domain, const char *meshname)
{
    //
    // Create the VTK objects and connect them up.
    //
    vtkStructuredGrid    *sgrid   = vtkStructuredGrid::New(); 
    vtkPoints            *points  = vtkPoints::New();
    sgrid->SetPoints(points);
    points->Delete();

    //
    // Tell the grid what its dimensions are and populate the points array.
    //
    int dims[3];
    dims[0] = *nx;
    dims[1] = *ny;
    dims[2] = *nz;
    sgrid->SetDimensions(dims);

    //
    // Populate the coordinates.
    //
    points->SetNumberOfPoints(*nx * *ny * *nz);
    float *pts = (float *) points->GetVoidPointer(0);

    int npts = 0;
    for (int i=0; i<*nx; i++)
    {
        for (int j=0; j<*ny; j++)
        {
            for (int k=0; k<*nz; k++)
            {
                pts[npts*3 + 0] = (*xcoords)[i];
                pts[npts*3 + 1] = (*ycoords)[j];
                pts[npts*3 + 2] = (*zcoords)[k];
                npts++;
            }
        }
    }

    return sgrid;
}


// ****************************************************************************
//  Method: avtSimDBFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 25, 2004
//
// ****************************************************************************

vtkDataArray *
avtSimDBFileFormat::GetVar(int domain, const char *varname)
{
    return NULL;
    //
    // If you have a file format where variables don't apply (for example a
    // strictly polygonal format like the STL (Stereo Lithography) format,
    // then uncomment the code below.
    //
    // EXCEPTION0(InvalidVariableException, varname);
    //

    //
    // If you do have a scalar variable, here is some code that may be helpful.
    //
    // int ntuples = XXX; // this is the number of entries in the variable.
    // vtkFloatArray *rv = vtkFloatArray::New();
    // rv->SetNumberOfTuples(ntuples);
    // for (int i = 0 ; i < ntuples ; i++)
    // {
    //      rv->SetTuple1(i, VAL);  // you must determine value for ith entry.
    // }
    //
    // return rv;
    //
}


// ****************************************************************************
//  Method: avtSimDBFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 25, 2004
//
// ****************************************************************************

vtkDataArray *
avtSimDBFileFormat::GetVectorVar(int domain, const char *varname)
{
    return NULL;
    //
    // If you have a file format where variables don't apply (for example a
    // strictly polygonal format like the STL (Stereo Lithography) format,
    // then uncomment the code below.
    //
    // EXCEPTION0(InvalidVariableException, varname);
    //

    //
    // If you do have a vector variable, here is some code that may be helpful.
    //
    // int ncomps = YYY;  // This is the rank of the vector - typically 2 or 3.
    // int ntuples = XXX; // this is the number of entries in the variable.
    // vtkFloatArray *rv = vtkFloatArray::New();
    // int ucomps = (ncomps == 2 ? 3 : ncomps);
    // rv->SetNumberOfComponents(ucomps);
    // rv->SetNumberOfTuples(ntuples);
    // float *one_entry = new float[ucomps];
    // for (int i = 0 ; i < ntuples ; i++)
    // {
    //      int j;
    //      for (j = 0 ; j < ncomps ; j++)
    //           one_entry[j] = ...
    //      for (j = ncomps ; j < ucomps ; j++)
    //           one_entry[j] = 0.;
    //      rv->SetTuple(i, one_entry); 
    // }
    //
    // delete [] one_entry;
    // return rv;
    //
}
