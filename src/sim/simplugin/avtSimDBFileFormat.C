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

extern int cycle;

extern int p_nx;
extern int p_ny;
extern int p_nz;
extern double *p_xcoords;
extern double *p_ycoords;
extern double *p_zcoords;
extern double *p_zvalues;
extern double *p_nvalues;

extern int numdomains;

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
//  Creation:   August 24, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Nov  1 17:28:49 PST 2004
//    Made it two domains so it would work in parallel.
//
// ****************************************************************************

void
avtSimDBFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // Note -- it is not yet safe to remove this:
    // This is probably because it gets propagated to the viewer as-is.
    // Furthermore, there are now pieces of the engine that depend on it,
    // and obviously the engine cannot retrieve metadata from the MDServer to
    // determine if it is a simulation.
    md->SetIsSimulation(true);

    AddMeshToMetaData(md, "mesh", AVT_CURVILINEAR_MESH, NULL,
                      numdomains, 0, 3, 3);
    AddScalarVarToMetaData(md, "speed", "mesh", AVT_NODECENT);
    AddScalarVarToMetaData(md, "density", "mesh", AVT_ZONECENT);
    
}

// ****************************************************************************
//  Method:  avtSimDBFileFormat::GetCycle
//
//  Purpose:
//    return the current cycle
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 24, 2004
//
// ****************************************************************************

int
avtSimDBFileFormat::GetCycle()
{
    return cycle;
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
//  Creation:   August 24, 2004
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
//  Creation:   August 24, 2004
//
// ****************************************************************************

vtkDataArray *
avtSimDBFileFormat::GetVar(int domain, const char *varname)
{
    vtkFloatArray *array = vtkFloatArray::New();
    int nzones = ((*nx)-1) * ((*ny)-1) * ((*nz)-1);
    int nnodes = *nx * *ny * *nz;
    if (string(varname) == "density")
    {
        array->SetNumberOfTuples(nzones);
        for (int i=0; i<nzones; i++)
        {
            array->SetTuple1(i, (*zvalues)[i]);
        }
    }
    else if (string(varname) == "speed")
    {
        array->SetNumberOfTuples(nnodes);
        for (int i=0; i<nnodes; i++)
        {
            array->SetTuple1(i, (*nvalues)[i]);
        }
    }
    else
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    return array;
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
}
