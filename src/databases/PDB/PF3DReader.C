#include <PF3DReader.h>
#include <math.h>

#include <avtDatabaseMetaData.h>
#include <avtTypes.h>

#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>

#include <DebugStream.h>

#include <InvalidVariableException.h>

// ****************************************************************************
// Method: PF3DReader::PF3DReader
//
// Purpose: 
//   Constructor for the PF3DReader class.
//
// Arguments:
//   p : The PDBFile object pointer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:28:54 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

PF3DReader::PF3DReader(PDBfile *p) : PDBReader(p)
{
    dx = dy = dz = 0.;
    lx = ly = lz = 0.;
    nx = ny = nz = 0;
    nxg0 = nyg0 = nzg0 = 0;
    nxg1 = nyg1 = nzg1 = 0;
    ipol = 0;
    cycle = 0;
    dtime = 0.;
    domain = 0;
    nDomains = 1;
}

// ****************************************************************************
// Method: PF3DReader::~PF3DReader
//
// Purpose: 
//   Destructor for the PF3DReader class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:29:21 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

PF3DReader::~PF3DReader()
{
}

// ****************************************************************************
// Method: PF3DReader::Identify
//
// Purpose: 
//   Tests the file to see if it is really PF3D.
//
// Returns:    true if the file is PF3D; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:29:40 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
PF3DReader::Identify()
{
    //
    // Look for a few items that tell us that the file came from pF3d.
    //
    bool validFile = true;
    validFile &= SymbolExists("PF3D_VERSION_MAJOR");
    validFile &= SymbolExists("PF3D_VERSION_MINOR");

    //
    // Try and read in some necessary values.
    //
    if(validFile)
    {
        validFile &= GetDouble("lx", &lx);
        validFile &= GetDouble("ly", &ly);
        validFile &= GetDouble("lz", &lz);
        validFile &= GetDouble("dx", &dx);
        validFile &= GetDouble("dy", &dy);
        validFile &= GetDouble("dz", &dz);
        validFile &= GetInteger("nx", &nx);
        validFile &= GetInteger("ny", &ny);
        validFile &= GetInteger("nz", &nz);
        validFile &= GetInteger("nxl", &nxl);
        validFile &= GetInteger("nyl", &nyl);
        validFile &= GetInteger("nzl", &nzl);

        validFile &= GetInteger("nxg0", &nxg0);
        validFile &= GetInteger("nyg0", &nyg0);
        validFile &= GetInteger("nzg0", &nzg0);
        validFile &= GetInteger("nxg1", &nxg1);
        validFile &= GetInteger("nyg1", &nyg1);
        validFile &= GetInteger("nzg1", &nzg1);

        debug4 << "nx=" << nx << ", ny=" << ny << ", nz=" << nz << endl;
        debug4 << "nxg0=" << nxg0 << ", nyg0=" << nyg0 << ", nzg0=" << nzg0 << endl;
        debug4 << "nxg1=" << nxg1 << ", nyg1=" << nyg1 << ", nzg1=" << nzg1 << endl;
        debug4 << "nxl=" << nxl << ", nyl=" << nyl << ", nzl=" << nzl << endl;
        debug4 << "lx=" << lx << ", ly=" << ly << ", lz=" << lz << endl;
        debug4 << "dx=" << dx << ", dy=" << dy << ", dz=" << dz << endl;

        //
        // Read in other values that are nice but we can live without.
        //
        //GetInteger("cycle", &cycle);
        //GetDouble("dtime", &dtime);
        GetInteger("ipol", &ipol);
        GetInteger("mp_rank", &domain);
        GetInteger("mp_size", &nDomains);

        //
        // This is a temporary hack for determining the offset to the
        // rho array.
        //
        ComputeKludgeOffset();
    }

    return validFile;
}

//
// This method should go away when we can successfully PD_read their files.
//

void
PF3DReader::ComputeKludgeOffset()
{
//    kludgeOffset = 27; // Looks okay? for s2 database.
//    kludgeOffset = 10; // Looks perfect for 5901 database.
 
    kludgeOffset = 0;
#if 1
    defstr *dp = PD_inquire_host_type(pdb, (char *)"mesh");
    if(dp != NULL)
    {
         memdes *member = dp->members;
         while(member != NULL)
         {
             ++kludgeOffset;
             member = member->next;
         }
    }
#endif
    debug4 << "KLUDGEOFFSET = " << kludgeOffset << endl;
}

// ****************************************************************************
// Method: PF3DReader::GetTimeVaryingInformation
//
// Purpose: 
//   Reads time varying information when the file is opened.
//
// Arguments:
//   timestep : The timestep.
//   md       : The metadata that we're going to populate.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:49:27 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
PF3DReader::GetTimeVaryingInformation(avtDatabaseMetaData *md)
{
    md->SetCycle(0, cycle);
    md->SetTime(0, dtime);
}

// ****************************************************************************
// Method: PF3DReader::PopulateDatabaseMetaData
//
// Purpose: 
//   Populates the metadata with the list of variables from the file.
//
// Arguments:
//   md : The metadata object that we're going to populate.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:43:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
PF3DReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    avtMeshType mt = AVT_RECTILINEAR_MESH;
    syment *ep = NULL;
    int   ndims = 3;
    int   cellOrigin = 1;
    int   dims[3], first[3], last[3];
    float extents[6];
    bool  ghostPresent;

    //
    // Check for the hydro mesh (fluid).
    //
    if((ep = PD_inquire_entry(pdb, "fluid", 0, NULL)) != NULL)
    {
        debug4 << "PF3DFile::PopulateDatabaseMetaData: var=fluid"
               << ", type=" << PD_entry_type(ep) << endl;

        if(strcmp(PD_entry_type(ep), "mesh") == 0)
        {
            // Add the hydro mesh to the metadata.
            GetMeshInfo("fluid", extents, dims, ghostPresent, first, last);

            avtMeshMetaData *mmd = new avtMeshMetaData(extents,
                "fluid", 1, 0, cellOrigin, ndims, ndims, mt);
            md->Add(mmd);

            // Add the hydro mesh's variables.
            avtScalarMetaData *rho, *rhoe, *dte, *p;
            rho = new avtScalarMetaData("rho", "fluid", AVT_ZONECENT);
            md->Add(rho);
#ifdef KNOW_HOW_TO_READ
            rhoe = new avtScalarMetaData("rhoe", "fluid", AVT_ZONECENT);
            md->Add(rhoe);
            dte = new avtScalarMetaData("dte", "fluid", AVT_ZONECENT);
            md->Add(dte);
            p = new avtScalarMetaData("p", "fluid", AVT_ZONECENT);
            md->Add(p);
            avtVectorMetaData *s, *v;
            s = new avtVectorMetaData("s", "fluid", AVT_NODECENT, 3);
            md->Add(s);
            v = new avtVectorMetaData("v", "fluid", AVT_NODECENT, 3);
            md->Add(v);
#endif
        }
    }
    else
        debug4 << "PF3DFile::PopulateDatabaseMetaData: Cannot query fluid!" << endl;

    //
    // Check for the light mesh (light).
    //
    if((ep = PD_inquire_entry(pdb, "light", 0, NULL)) != NULL)
    {
        int i;
        char tn[10];
        debug4 << "PF3DFile::PopulateDatabaseMetaData: var=light"
               << ", type=" << PD_entry_type(ep) << endl;

        if(strcmp(PD_entry_type(ep), "light_mesh") == 0)
        {
            // Add the light mesh to the metadata.
            GetMeshInfo("light", extents, dims, ghostPresent, first, last);
            avtMeshMetaData *mmd = new avtMeshMetaData(extents,
                "light", 1, 0, cellOrigin, ndims, ndims, mt);
            md->Add(mmd);

#ifdef KNOW_HOW_TO_READ
            // Add some variables over the light mesh.
            for(i = 0; i < 4; ++i)
            {
                sprintf(tn, "t%d", i);
                avtScalarMetaData *ti = new avtScalarMetaData(tn, "light",
                    AVT_NODECENT);
                md->Add(ti);
            }

            // Only define these vectors if the file's ipol value if nonzero.
            int ipol;
            if(GetInteger("ipol", &ipol))
            {
                if(ipol > 0)
                {
                    for(i = 0; i < 4; ++i)
                    {
                        sprintf(tn, "t%dps", i);
                        avtScalarMetaData *ti = new avtScalarMetaData(tn,
                            "light", AVT_NODECENT);
                        md->Add(ti);
                    }
                }
            }
#endif
        }
    }
    else
        debug4 << "PF3DFile::PopulateDatabaseMetaData: Cannot query light!" << endl;
}

// ****************************************************************************
// Method: PF3DReader::GetMesh
//
// Purpose: 
//   Gets the named mesh and returns it as a vtkDataSet.
//
// Arguments:
//   meshName : The name of the mesh.
//
// Returns:    a vtkDataSet containing the mesh.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:43:51 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
PF3DReader::GetMesh(const char *meshName)
{
    int   dims[3], first[3], last[3];
    float extents[6];
    bool  ghostPresent;

    //
    // Get the mesh information.
    //
    GetMeshInfo(meshName, extents, dims, ghostPresent, first, last);

    //
    // Populate the coordinates.
    //
    vtkFloatArray *coords[3];
    for (int i = 0 ; i < 3 ; i++)
    {
        // Default number of components for an array is 1.
        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(dims[i]);
        int i2 = i * 2;
        int i21 = i2 + 1;
        for (int j = 0 ; j < dims[i] ; j++)
        {
            float t = float(j) / float(dims[i] - 1);
            float c = (1.-t)*extents[i2] + t*extents[i21];
            coords[i]->SetComponent(j, 0, c);
        }
    }
    vtkRectilinearGrid *grid = vtkRectilinearGrid::New(); 
    grid->SetDimensions(dims);
    grid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    grid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    grid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    //
    // Populate the ghost zones array if necessary.
    //
    if(ghostPresent)
    {
        long nNodes = dims[0] * dims[1] * dims[2];
        bool *ghostPoints = new bool[nNodes];

        //
        // Initialize as all ghost levels
        //
        for (int ii = 0; ii < nNodes; ii++)
            ghostPoints[ii] = true; 

        //
        // Set real values
        //
        for (int k = first[2]; k <= last[2]; k++)
            for (int j = first[1]; j <= last[1]; j++)
                for (int i = first[0]; i <= last[0]; i++)
                {
                    int index = k*dims[1]*dims[0] + j*dims[0] + i;
                    ghostPoints[index] = false; 
                }
        //
        //  okay, now we have ghost points, but what we really want
        //  are ghost cells ... convert:  if all points associated with
        //  cell are 'real' then so is the cell.
        //
        unsigned char realVal = 0, ghostVal = 1;
        int ncells = grid->GetNumberOfCells();
        vtkIdList *ptIds = vtkIdList::New();
        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("vtkGhostLevels");
        ghostCells->Allocate(ncells);
 
        for (int i = 0; i < ncells; i++)
        {
            grid->GetCellPoints(i, ptIds);
            bool ghost = false;
            for (int idx = 0; idx < ptIds->GetNumberOfIds(); idx++)
                ghost |= ghostPoints[ptIds->GetId(idx)];

            if (ghost)
                ghostCells->InsertNextValue(ghostVal);
            else
                ghostCells->InsertNextValue(realVal);
 
        } 
        grid->GetCellData()->AddArray(ghostCells);
        delete [] ghostPoints;
        ghostCells->Delete();
        ptIds->Delete();

        vtkIntArray *realDims = vtkIntArray::New();
        realDims->SetName("avtRealDims");
        realDims->Allocate(6);
        realDims->SetValue(0, first[0]);
        realDims->SetValue(1, last[0]);
        realDims->SetValue(2, first[1]);
        realDims->SetValue(3, last[1]);
        realDims->SetValue(4, first[2]);
        realDims->SetValue(5, last[2]);
        grid->GetFieldData()->AddArray(realDims);
        grid->GetFieldData()->CopyFieldOn("avtRealDims");
        realDims->Delete();

        grid->SetUpdateGhostLevel(0);
    }

    //
    // Determine the indices of the mesh within its group.  Add that to the
    // VTK dataset as field data.
    //
    int base_index[] = {0,0,0};
    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(3);
    arr->SetValue(0, base_index[0]);
    arr->SetValue(1, base_index[1]);
    arr->SetValue(2, base_index[2]);
    arr->SetName("base_index");
    grid->GetFieldData()->AddArray(arr);
    arr->Delete();

    return grid;
}

// ****************************************************************************
// Method: PF3DReader::GetMeshInfo
//
// Purpose: 
//   Gets information about the mesh when given a mesh name.
//
// Arguments:
//   meshName : The name of the mesh.
//   extents  : Return array for the mesh extents.
//   nodeDims : Return array for the number of nodes in each dimension.
//   ghostPresent : Whether or not the mesh has ghost zones.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 10:50:59 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
PF3DReader::GetMeshInfo(const char *meshName, float extents[6],
    int nodeDims[3], bool &ghostPresent, int first[3], int last[3])
{
    bool edge[6];

    if(strcmp(meshName, "fluid") == 0)
    {
        // These are the node dimensions for the whole problem.
        //   nodeDims[0] = 2 * nx + 2 + 1;
        //   nodeDims[1] = 2 * nx + 2 + 1;
        //   nodeDims[2] = nz + 2 + 1;

        //
        // Determine if the edges of the domain match the edges of the problem.
        //
        edge[0] = (nxg0 == 1);
        edge[1] = (nxg1 == 2*nx);
        edge[2] = (nyg0 == 1);
        edge[3] = (nyg1 == 2*ny);
        edge[4] = (nzg0 == 1);
        edge[5] = (nzg1 == nz+1);

        //
        // Figure out the node dimensions.
        //
        nodeDims[0] = nxl + 1 + (edge[0]?0:1) + (edge[1]?0:1);
        nodeDims[1] = nyl + 1 + (edge[2]?0:1) + (edge[3]?0:1);
        nodeDims[2] = nzl + 1 + (edge[4]?0:1) + (edge[5]?0:1);

        //
        // Figure out the first/last values for ghost zones.
        //
        first[0] = edge[0]?0:1;
        first[1] = edge[2]?0:1;
        first[2] = edge[4]?0:1;
        last[0] = first[0] + nodeDims[0]-1 - (edge[0]?0:1) - (edge[1]?0:1);
        last[1] = first[1] + nodeDims[1]-1 - (edge[2]?0:1) - (edge[3]?0:1);
        last[2] = first[2] + nodeDims[2]-1 - (edge[4]?0:1) - (edge[5]?0:1);

        ghostPresent = true;
    }
    else // light mesh - no ghost cells.
    {
        // These are the node dimensions for the whole problem.
        //   nodeDims[0] = 2 * nx + 1;
        //   nodeDims[1] = 2 * ny + 1;
        //   nodeDims[2] = nz + 1;

        // These are the node dimensions for this domain.
        nodeDims[0] = nxl+1;
        nodeDims[1] = nyl+1;
        nodeDims[2] = nzl+1;
        first[0] = 0;
        first[1] = 0;
        first[2] = 0;
        last[0] = nodeDims[0] - 2;
        last[1] = nodeDims[1] - 2;
        last[2] = nodeDims[2] - 2;

        ghostPresent = false;
    }

    //
    // Figure out the location of the mesh.
    //
    if(nDomains > 1)
    {
        float xMin = (float(nxg0) - 0.5) * dx;
        float yMin = (float(nyg0) - 0.5) * dy;
        float zMin = (float(nzg0) - 0.5) * dz;
        float xMax = (float(nxg1) + 0.5) * dx;
        float yMax = (float(nyg1) + 0.5) * dy;
        float zMax = (float(nzg1) + 0.5) * dz;

        extents[0] = -lx + xMin;
        extents[1] = -lx + xMax;
        extents[2] = -ly + yMin;
        extents[3] = -ly + yMax;
        extents[4] = 0. + zMin;
        extents[5] = zMax;
    }
    else
    {
        extents[0] = -lx;
        extents[1] = lx;
        extents[2] = -ly;
        extents[3] = ly;
        extents[4] = 0.;
        extents[5] = lz;
    }

    //
    // Take the ghost zones into account when determining the extents
    //
    if(ghostPresent)
    {
        extents[0] -= (edge[0]?0.:dx);
        extents[1] += (edge[1]?0.:dx);
        extents[2] -= (edge[2]?0.:dy);
        extents[3] += (edge[3]?0.:dy);
        extents[4] -= (edge[4]?0.:dz);
        extents[5] += (edge[5]?0.:dz);
    }
}

// ****************************************************************************
// Method: PF3DReader::GetVar
//
// Purpose: 
//   Returns the named scalar variable.
//
// Arguments:
//   varName : The variable name.
//
// Returns:    A vtkDataArray containing the variable's data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:53:09 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
PF3DReader::GetVar(const char *varName)
{
    char *fileVar = NULL;
    char *meshName = "fluid";
    vtkFloatArray *retval = NULL;

    //
    // Determine the file variable and the mesh.
    //
    if(strcmp(varName, "rho") == 0)
        fileVar = "fluid.rho";
    else if(strcmp(varName, "rhoe") == 0)
        fileVar = "fluid.rhoe";
    else if(strcmp(varName, "dte") == 0)
        fileVar = "fluid.dte";
    else if(strcmp(varName, "p") == 0)
        fileVar = "fluid.p";
    else
    {
        meshName = "light";
        if(strcmp(varName, "t0") == 0)
            fileVar = "light.t0";
        else if(strcmp(varName, "t1") == 0)
            fileVar = "light.t1";
        else if(strcmp(varName, "t2") == 0)
            fileVar = "light.t2";
        else if(strcmp(varName, "t3") == 0)
            fileVar = "light.t3";
        else if(ipol > 0)
        {
            if(strcmp(varName, "t0ps") == 0)
                fileVar = "light.t0ps";
            else if(strcmp(varName, "t1ps") == 0)
                fileVar = "light.t1ps";
            else if(strcmp(varName, "t2ps") == 0)
                fileVar = "light.t2ps";
            else if(strcmp(varName, "t3ps") == 0)
                fileVar = "light.t3ps";
        }
    }

    //
    // If we were able to figure out a file variable, read it.
    //
    if(fileVar == NULL)
    {
        EXCEPTION1(InvalidVariableException, varName);
    }
    else
    {
        float extents[6];
        int   nodeDims[3], first[3], last[3];
        bool  ghostPresent;
        GetMeshInfo(meshName, extents, nodeDims, ghostPresent, first, last);

        if(strcmp(meshName, "fluid") == 0)
        {
            // The variables are zonal so figure out the number of cells and
            // the number of bytes that we'll need.
            int originalDims[] = {nxl+2, nyl+2, nzl+2};
            int nExpectedCells = originalDims[0] * originalDims[1] * originalDims[2];
            int dims[] = {nodeDims[0]-1,nodeDims[1]-1,nodeDims[2]-1};
            int nCells = dims[0] * dims[1] * dims[2];
            vtkFloatArray   *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples(nCells);
            float *var = (float*)scalars->GetVoidPointer(0);
            float *data = new float[nExpectedCells + kludgeOffset];

            if(data != NULL && var != NULL)
            {
                int j, k;

                //
                // Create a definition for pointer that allows the PDB
                // library to convert the data from the file.
                //
                char pointer_def[100];
                sprintf(pointer_def, "float var[%d]", nExpectedCells);
                PD_defstr(pdb, "pointer", pointer_def, LAST);

                //
                // Try to read the data from the PDB file.
                //
                if(PD_read(pdb, fileVar, data) == TRUE)
                {
                    //
                    // Copy the parts of the data that we need. We don't just
                    // read in the whole thing because we've removed the ghost
                    // zones on the edge of the problem and that changes the
                    // size of the mesh.
                    //
                    register float *fptr = var;
                    int firstX = (first[0]==0)?1:0;
                    int firstY = (first[1]==0)?1:0;
                    int firstZ = (first[2]==0)?1:0;
                    int lastX = firstX + dims[0];
                    int lastY = firstY + dims[1];
                    int lastZ = firstZ + dims[2];
                    int dX = lastX - firstX;
                    int knxy = kludgeOffset + firstX;
                    int originalXY = originalDims[1]*originalDims[0];

                    for (k = lastZ; k > firstZ; --k)
                    {
                        int jnx = knxy;
                        for (j = lastY; j > firstY; --j)
                        {
                            register int i = dX;
                            register float *data2 = data + jnx;
                            for (; i > 8; i -= 8)
                            {
                                *fptr++ = *data2++;
                                *fptr++ = *data2++;
                                *fptr++ = *data2++;
                                *fptr++ = *data2++;
                                *fptr++ = *data2++;
                                *fptr++ = *data2++;
                                *fptr++ = *data2++;
                                *fptr++ = *data2++;
                            }
                            for (; i > 0; --i)
                            {
                                *fptr++ = *data2++;
                            }

                            jnx += originalDims[0];
                        }

                        knxy += originalXY;
                    }

                    // Delete the data array.
                    delete [] data;

                    // Set the return value.
                    retval = scalars;
                }
                else
                {
                    debug4 << "PF3DReader could NOT read " << fileVar
                           << ". " << PD_err << endl;
                    scalars->Delete();
                    scalars = NULL;
                    delete [] data;

                    EXCEPTION1(InvalidVariableException, varName);
                }
            }
            else
            {
                debug4 << "PF3DReader could not allocate " << nCells
                       << " floats!" << endl;
                scalars->Delete();
                scalars = NULL;
            }
        }
        else // light mesh
        {
            // All variables are complex. Create a variable that is the 
            // magnitude of the complex vector.

            // The variables are nodal so figure out the number of nodes and
            // the number of bytes that we'll need.
            int nNodes = nodeDims[0]*nodeDims[1]*nodeDims[2];
            vtkFloatArray   *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples(nNodes);
            float *var = (float*)scalars->GetVoidPointer(0);
            float *data = new float[nNodes*2];

            if(data != NULL && var != NULL)
            {
                // Create a definition for pointer that allows the PDB
                // library to convert the data from the file.
                char pointer_def[100];
                sprintf(pointer_def, "float var[%d]", nNodes*2);
                PD_defstr(pdb, "pointer", pointer_def, LAST);

                // Try to read the data from the PDB file.
                if(PD_read(pdb, fileVar, data) == TRUE)
                {
                    // Process the data array.
                    float *var2 = var;
                    float *data2 = data;
                    for(int kk = nNodes; kk > 0; --kk)
                    {
                        float a = *data2++;
                        float b = *data2++;
                        *var2++ = (a*a + b*b);
                    }

                    // Delete the data array.
                    delete [] data;

                    // Set the return value.
                    retval = scalars;
                }
                else
                {
                    debug4 << "PF3DReader could NOT read " << fileVar
                           << ". " << PD_err << endl;
                    scalars->Delete();
                    scalars = NULL;
                    delete [] data;

                    EXCEPTION1(InvalidVariableException, varName);
                }
            }
            else
            {
                debug4 << "PF3DReader could not allocate " << nNodes*2
                       << " floats!" << endl;
                scalars->Delete();
                scalars = NULL;
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: PF3DReader::GetVectorVar
//
// Purpose: 
//   Returns the named vector variable.
//
// Arguments:
//   varName : The variable name.
//
// Returns:    A vtkDataArray containing the variable's data.
//
// Note:       THIS HAS NEVER BEEN USED!
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 10 08:53:09 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
PF3DReader::GetVectorVar(const char *varName)
{
    char *fileVar = NULL;
    char *meshName = "fluid";
    vtkFloatArray *retval = NULL;

    //
    // Determine the file variable and the mesh.
    //
    if(strcmp(varName, "s") == 0)
        fileVar = "fluid.s";
    else if(strcmp(varName, "v") == 0)
        fileVar = "fluid.v";

    if(fileVar == NULL)
    {
        EXCEPTION1(InvalidVariableException, varName);
    }
    else
    {
        float extents[6];
        int   nodeDims[3], first[3], last[3];
        bool  ghostPresent;
        GetMeshInfo(meshName, extents, nodeDims, ghostPresent, first, last);

        debug4 << "PF3DReader asked for vector variable: " << fileVar << endl;
        debug4 << "PF3DReader::GetVectorVar: 0: nodeDims={"
               << nodeDims[0] << ", "
               << nodeDims[1] << ", "
               << nodeDims[2] << "}" << endl;

        // The variables are nodal so figure out the number of nodes and
        // the number of bytes that we'll need.
        int nNodes = nodeDims[0]*nodeDims[1]*nodeDims[2];
        vtkFloatArray *vectors = vtkFloatArray::New();

        vectors->SetNumberOfTuples(nNodes);
        vectors->SetNumberOfComponents(3);
        float *data = new float[nNodes*3];
        if(data != NULL)
        {
            // Create a definition for pointer that allows the PDB
            // library to convert the data from the file.
            char pointer_def[100];
            sprintf(pointer_def, "float var[%d]", nNodes*3);
            PD_defstr(pdb, "pointer", pointer_def, LAST);

            // Try to read the data from the PDB file.
            if(PD_read(pdb, fileVar, data) == TRUE)
            {
                float *data2 = data;
                for(int i = 0; i < nNodes; ++i)
                {
                    float a = *data2++;
                    float b = *data2++;
                    float c = *data2++;
                    vectors->SetTuple3(i, a, b, c);
                }
                // Delete the data array.
                delete [] data;
                // Set the return value.
                retval = vectors;
            }
            else
            {
                debug4 << "PF3DReader could NOT read " << fileVar
                       << ". " << PD_err << endl;
                vectors->Delete();
                vectors = NULL;
                delete [] data;

                EXCEPTION1(InvalidVariableException, varName);
            }
        }
        else
        {
            debug4 << "PF3DReader could not allocate " << nNodes*3
                   << " floats!" << endl;
            vectors->Delete();
            vectors = NULL;
        }
    }

    return retval;
}
