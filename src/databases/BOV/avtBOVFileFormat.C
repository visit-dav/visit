// ************************************************************************* //
//                            avtBOVFileFormat.C                             //
// ************************************************************************* //

#include <avtBOVFileFormat.h>

#include <zlib.h>

#include <visitstream.h>
#include <visit-config.h>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtIntervalTree.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtTypes.h>
#include <avtVariableCache.h>

#include <DebugStream.h>
#include <BadDomainException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>


static int FormatLine(char *line);


// ****************************************************************************
//  Method: avtBOVFileFormat constructor
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Sep 15 09:21:29 PDT 2003
//    Initialized nodalCentering.
//
//    Hank Childs, Sun May  9 07:56:42 PDT 2004
//    Allow for the format not to be broken up into bricklets.  Also add
//    support for byte data.
//
// ****************************************************************************

avtBOVFileFormat::avtBOVFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    //
    // Determine what path we should prepend to every file name.
    //
    int last_slash = -1;
    int len = strlen(fname);
    for (int i = len-1 ; i >= 0 ; i--)
        if (fname[i] == '/' || fname[i] == '\\')
        {
            last_slash = i;
            break;
        }

    if (last_slash >= 0)
    {
        path = new char[last_slash+2];
        strncpy(path, fname, last_slash+1);
        path[last_slash+1] = '\0';
    }
    else
    {
        path = new char[1];
        path[0] = '\0';
    }

    //
    // Put in some dummy values.  Some of these must be overwritten, some of
    // them are fine as is (for example, we *must* know the file pattern.  The
    // cycle is not necessary.)
    //
    file_pattern = NULL;
    cycle = 0;
    full_size[0] = 0;
    full_size[1] = 0;
    full_size[2] = 0;
    bricklet_size[0] = 0;
    bricklet_size[1] = 0;
    bricklet_size[2] = 0;
    varname = new char[strlen("var") + 1];
    strcpy(varname, "var");
    min = 0.;
    max = 1.;
    origin[0] = 0.;
    origin[1] = 0.;
    origin[2] = 0.;
    dimensions[0] = 1.;
    dimensions[1] = 1.;
    dimensions[2] = 1.;
    var_brick_min = NULL;
    var_brick_max = NULL;
    declaredEndianess = false;
    littleEndian = false;
    hasBoundaries = false;
    nodalCentering = true;
    byteData = true;

    ReadTOC();

    if (file_pattern == NULL)
    {
        debug1 << "Did not parse file pattern (\"DATA_FILE\")." << endl;
        EXCEPTION1(InvalidFilesException, fname);
    }
    if (full_size[0] <= 0 || full_size[1] <= 0 || full_size[2] <= 0)
    {
        debug1 << "Was not able to determine \"DATA_SIZE\"" << endl;
        EXCEPTION1(InvalidFilesException, fname);
    }
    if (bricklet_size[0] <= 0 || bricklet_size[1] <= 0 || bricklet_size[2] <= 0)
    {
        bricklet_size[0] = full_size[0];
        bricklet_size[1] = full_size[1];
        bricklet_size[2] = full_size[2];
    }
    if (dimensions[0] <= 0. || dimensions[1] <= 0. || dimensions[2] <= 0.)
    {
        debug1 << "Invalid dimensions \"BRICK_SIZE\"" << endl;
        EXCEPTION1(InvalidFilesException, fname);
    }

    if ((full_size[0] % bricklet_size[0]) != 0)
    {
        debug1 << "Full size must be a multiple of bricklet size" << endl;
        EXCEPTION1(InvalidFilesException, fname);
    }
    if ((full_size[1] % bricklet_size[1]) != 0)
    {
        debug1 << "Full size must be a multiple of bricklet size" << endl;
        EXCEPTION1(InvalidFilesException, fname);
    }
    if ((full_size[2] % bricklet_size[2]) != 0)
    {
        debug1 << "Full size must be a multiple of bricklet size" << endl;
        EXCEPTION1(InvalidFilesException, fname);
    }
}


// ****************************************************************************
//  Method: avtBOVFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
// ****************************************************************************

avtBOVFileFormat::~avtBOVFileFormat()
{
    if (path != NULL)
    {
        delete [] path;
        path = NULL;
    }
    if (file_pattern != NULL)
    {
        delete [] file_pattern;
        file_pattern = NULL;
    }
    if (varname != NULL)
    {
        delete [] varname;
        varname = NULL;
    }
    if (var_brick_min != NULL)
    {
        delete [] var_brick_min;
        var_brick_min = NULL;
    }
    if (var_brick_max != NULL)
    {
        delete [] var_brick_max;
        var_brick_max = NULL;
    }
}


// ****************************************************************************
//  Method: avtBOVFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with a specific domain.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Sep 15 09:49:06 PDT 2003
//    If we have zonal centering, then construct the mesh differently.
//
// ****************************************************************************

vtkDataSet *
avtBOVFileFormat::GetMesh(int dom, const char *meshname)
{
    int   i;

    if (strcmp(meshname, "mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    //
    // Establish which domain we are at.
    //
    int nx = full_size[0] / bricklet_size[0];
    int ny = full_size[1] / bricklet_size[1];
    int nz = full_size[2] / bricklet_size[2];

    //
    // Sanity check.
    //
    int nbricks = nx*ny*nz;
    if (dom < 0 || dom >= nbricks)
    {
        EXCEPTION2(BadDomainException, dom, nbricks);
    }

    int z_off = dom / (nx*ny);
    int y_off = (dom % (nx*ny)) / nx;
    int x_off = dom % nx;

    //
    // Establish what the range is of this dataset.
    //
    float x_step = dimensions[0] / nx;
    float y_step = dimensions[1] / ny;
    float z_step = dimensions[2] / nz;
    float x_start = origin[0] + x_step*x_off;
    float x_stop  = origin[0] + x_step*(x_off+1);
    float y_start = origin[1] + y_step*y_off;
    float y_stop  = origin[1] + y_step*(y_off+1);
    float z_start = origin[2] + z_step*z_off;
    float z_stop  = origin[2] + z_step*(z_off+1);

    //
    // Create the VTK construct.  Note that the mesh is being created to fit
    // the variable we are reading in.  If we are told that that variable is
    // nodal, then we need one node for each entry in the variable array.
    // If it is zonal, then we need one zone for each entry in the variable
    // array.  In the zonal case, if the variable array is NX x NY x NZ, then
    // the mesh should have (NX+1) x (NY+1) x (NZ+1) nodes.
    //
    vtkRectilinearGrid *rv = vtkRectilinearGrid::New();

    vtkFloatArray *x = vtkFloatArray::New();
    int dx = bricklet_size[0];
    if (hasBoundaries)
    {
        dx = bricklet_size[0]+2;
        if (x_off == 0)
            dx -= 1;
        if (x_off >= nx-1)
            dx -= 1;
    }
    if (! nodalCentering) 
        dx += 1;
    x->SetNumberOfTuples(dx);
    for (i = 0 ; i < dx ; i++)
        x->SetTuple1(i, x_start + i * (x_stop-x_start) / (dx-1));

    vtkFloatArray *y = vtkFloatArray::New();
    int dy = bricklet_size[1];
    if (hasBoundaries)
    {
        dy = bricklet_size[1]+2;
        if (y_off == 0)
            dy -= 1;
        if (y_off >= ny-1)
            dy -= 1;
    }
    if (! nodalCentering) 
        dy += 1;
    y->SetNumberOfTuples(dy);
    for (i = 0 ; i < dy ; i++)
        y->SetTuple1(i, y_start + i * (y_stop-y_start) / (dy-1));

    vtkFloatArray *z = vtkFloatArray::New();
    int dz = bricklet_size[2];
    if (hasBoundaries)
    {
        dz = bricklet_size[2]+2;
        if (z_off == 0)
            dz -= 1;
        if (z_off >= nz-1)
            dz -= 1;
    }
    if (! nodalCentering) 
        dz += 1;
    z->SetNumberOfTuples(dz);
    for (i = 0 ; i < dz ; i++)
        z->SetTuple1(i, z_start + i * (z_stop-z_start) / (dz-1));

    int dims[3] = { dx, dy, dz };
    rv->SetDimensions(dims);
    rv->SetXCoordinates(x);
    rv->SetYCoordinates(y);
    rv->SetZCoordinates(z);
    x->Delete();
    y->Delete();
    z->Delete();

    return rv;
}


// ****************************************************************************
//  Method: avtBOVFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with a domain.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
//  Modifications:
//
//    Hank Childs, Sun May  9 07:56:42 PDT 2004
//    Better support for non-bricklet format.  Also better support for fully
//    qualified pathnames.  Also add support for bytes vs floats.  Also
//    add support for gzipped vs non-gzipped.  Also do a better job of closing
//    file descriptors and freeing memory.
//
// ****************************************************************************

// FROM FConvert.C
static int
float32_Reverse_Endian(float val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 3;
    unsigned char *out = outbuf;
 
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;
 
    return 4;
}

vtkDataArray *
avtBOVFileFormat::GetVar(int dom, const char *var)
{
    if (strcmp(var, varname) != 0)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    int nx = full_size[0] / bricklet_size[0];
    int ny = full_size[1] / bricklet_size[1];
    int nz = full_size[2] / bricklet_size[2];

    int x_off = dom % nx;
    int y_off = (dom % (nx*ny)) / nx;
    int z_off = dom / (nx*ny);

    int nbricks = nx*ny*nz;

    if (dom < 0 || dom >= nbricks)
    {
        EXCEPTION2(BadDomainException, dom, nbricks);
    }

    char filename[1024];
    sprintf(filename, file_pattern, dom);
    char qual_filename[1024];
    if (filename[0] != '/')
        sprintf(qual_filename, "%s%s", path, filename);
    else
        strcpy(qual_filename, filename);

    void *gz_handle = NULL;
    FILE *file_handle = NULL;
    bool gzipped = false;
    if (strstr(qual_filename, ".gz") != NULL)
    {
        gz_handle = gzopen(qual_filename, "r");
        gzipped = true;
    }
    else
    {
        file_handle = fopen(qual_filename, "r");
        gzipped = false;
    }

    int unit_size = (byteData == true ? sizeof(unsigned char) : sizeof(float));

    if (file_handle == NULL && gz_handle == NULL)
    {
        EXCEPTION1(InvalidFilesException, qual_filename);
    }

    vtkFloatArray *rv = vtkFloatArray::New();
    if (hasBoundaries)
    {
        int dx = bricklet_size[0] + 2;
        int x_start = (x_off == 0 ? 1 : 0);
        int x_stop  = (x_off >= nx-1 ? dx-1 : dx);
        int dy = bricklet_size[1] + 2;
        int y_start = (y_off == 0 ? 1 : 0);
        int y_stop  = (y_off >= ny-1 ? dy-1 : dy);
        int dz = bricklet_size[2] + 2;
        int z_start = (z_off == 0 ? 1 : 0);
        int z_stop  = (z_off >= nz-1 ? dz-1 : dz);

        int n_real_vals = (x_stop-x_start)*(y_stop-y_start)*(z_stop-z_start);
        rv->SetNumberOfTuples(n_real_vals);
        float *buff = (float *) rv->GetVoidPointer(0);

        //
        // How much we should allocate depends on whether we have floats or 
        // unsigned chars.
        //
        int total_vals = (dx*dy*dz);
        unsigned char *uc_buff = NULL;
        float *f_buff = NULL;
        if (byteData)
            uc_buff = new unsigned char[total_vals];
        else
            f_buff = new float[total_vals];
        void *buff2 = (byteData ? (void *) uc_buff : (void *) f_buff);

        //
        // Read in based on whether or not we have a gzipped file.
        //
        if (gzipped)
            gzread(gz_handle, buff2, total_vals*unit_size);
        else
            fread(buff2, unit_size, total_vals, file_handle);

        int ptId = 0;
        for (int i = z_start ; i < z_stop ; i++)
            for (int j = y_start ; j < y_stop ; j++)
                for (int k = x_start ; k < x_stop ; k++)
                {
                    int index = i*dx*dy + j*dx + k;
                    if (byteData)
                        buff[ptId++] = min + (max-min)*(uc_buff[index]/255.);
                    else
                        buff[ptId++] = f_buff[index];
                }

        if (uc_buff != NULL)
            delete [] uc_buff;
        if (f_buff != NULL)
            delete [] f_buff;
    }
    else
    {
        int nvals = bricklet_size[0] * bricklet_size[1] * bricklet_size[2];
        rv->SetNumberOfTuples(nvals);

        //
        // If we have unsigned chars, then we have to create temporary buffer to
        // read into.  If we have floats, we can just use the buffer directly.
        //
        unsigned char *uc_buff = NULL;
        float *f_buff = NULL;
        if (byteData)
            uc_buff = new unsigned char[nvals];
        else
            f_buff = (float *) rv->GetVoidPointer(0);
        void *buff = (byteData ? (void *) uc_buff : (void *) f_buff);

        //
        // Read in based on whether or not we have a gzipped file.
        //
        if (gzipped)
            gzread(gz_handle, buff, nvals*unit_size);
        else
            fread(buff, unit_size, nvals, file_handle);

        if (byteData)
        {
            float *tmp = (float *) rv->GetVoidPointer(0);
            for (int i = 0 ; i < nvals ; i++)
                tmp[i] = min + (max-min)*(uc_buff[i]/255.);
            delete [] uc_buff;
        }
    }

    //
    // If the endian is opposite of this platform, then reverse it...
    //
    if (declaredEndianess)
    {
        bool machineEndianIsLittle = true;
#ifdef WORDS_BIGENDIAN
        machineEndianIsLittle = false;
#else
        machineEndianIsLittle = true;
#endif
   
        if (littleEndian != machineEndianIsLittle)
        {
            if (!byteData)
            {
                int nvals = rv->GetNumberOfTuples();
                float *buff = (float *) rv->GetVoidPointer(0);
                for (int i = 0 ; i < nvals ; i++)
                {
                    float tmp;
                    float32_Reverse_Endian(buff[i], (unsigned char *) &tmp);
                    buff[i] = tmp;
                }
            }
        }
    }

    //
    // Close the file descriptors.
    //
    if (gzipped)
        gzclose(gz_handle);
    else
        fclose(file_handle);

    return rv;
}


// ****************************************************************************
//  Method: avtBOVFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets auxiliary data about the file format.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
//  Modifications:
//
//    Hank Childs, Sun May  9 08:43:33 PDT 2004
//    Do not assume that we have valid data extents.
//
// ****************************************************************************

void *
avtBOVFileFormat::GetAuxiliaryData(const char *var, int domain,
                                   const char *type, void *,
                                   DestructorFunction &df)
{
    void *rv = NULL;

    int nx = full_size[0] / bricklet_size[0];
    int ny = full_size[1] / bricklet_size[1];
    int nz = full_size[2] / bricklet_size[2];
    int nbricks = nx*ny*nz;

    if (strcmp(type, AUXILIARY_DATA_DATA_EXTENTS) == 0)
    {
        if (strcmp(var, varname) != 0)
        {
            EXCEPTION1(InvalidVariableException, var);
        }
        if (var_brick_min != NULL && var_brick_max != NULL)
        {
            avtIntervalTree *itree = new avtIntervalTree(nbricks, 1);
            for (int i = 0 ; i < nbricks ; i++)
            {
                float range[2] = { var_brick_min[i], var_brick_max[i] };
                itree->AddDomain(i, range);
            }
            itree->Calculate(true);

            rv = (void *) itree;
            df = avtIntervalTree::Destruct;
        }
    }
    else if (strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0)
    {
        if (strcmp(var, "mesh") != 0)
        {
            EXCEPTION1(InvalidVariableException, var);
        }
        avtIntervalTree *itree = new avtIntervalTree(nbricks, 3);
        for (int i = 0 ; i < nbricks ; i++)
        {
            int z_off = i / (nx*ny);
            int y_off = (i % (nx*ny)) / nx;
            int x_off = i % nx;

            //
            // Establish what the range is of this dataset.
            //
            float x_step = dimensions[0] / nx;
            float y_step = dimensions[1] / ny;
            float z_step = dimensions[2] / nz;
            float bounds[6];
            bounds[0] = origin[0] + x_step*x_off;
            bounds[1] = origin[0] + x_step*(x_off+1);
            bounds[2] = origin[1] + y_step*y_off;
            bounds[3] = origin[1] + y_step*(y_off+1);
            bounds[4] = origin[2] + z_step*z_off;
            bounds[5] = origin[2] + z_step*(z_off+1);
            itree->AddDomain(i, bounds);
        }
        itree->Calculate(true);

        rv = (void *) itree;
        df = avtIntervalTree::Destruct;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtBOVFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Populates a structure that describes which variables are in this
//      file format.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Sep 15 09:51:16 PDT 2003
//    Account for the possibility of zonal variables.
//
//    Hank Childs, Sat Sep 11 16:15:20 PDT 2004
//    Create domain boundary information.
//
// ****************************************************************************

void
avtBOVFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int nx = full_size[0] / bricklet_size[0];
    int ny = full_size[1] / bricklet_size[1];
    int nz = full_size[2] / bricklet_size[2];

    int nbricks = nx*ny*nz;
 
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->numBlocks = nbricks;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->blockTitle = "bricks";
    mesh->blockPieceName = "brick";
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    AddScalarVarToMetaData(md, varname, "mesh",
                               (nodalCentering ? AVT_NODECENT : AVT_ZONECENT));

    if (!avtDatabase::OnlyServeUpMetaData() && nbricks > 1)
    {
        avtRectilinearDomainBoundaries *rdb = 
                                      new avtRectilinearDomainBoundaries(true);
        rdb->SetNumDomains(nbricks);
        for (int i = 0 ; i < nbricks ; i++)
        {
            int nx = full_size[0] / bricklet_size[0];
            int ny = full_size[1] / bricklet_size[1];
            int nz = full_size[2] / bricklet_size[2];
            int z_off = i / (nx*ny);
            int y_off = (i % (nx*ny)) / nx;
            int x_off = i % nx;
            int extents[6];
            extents[0] = x_off * (bricklet_size[0]-1);
            extents[1]  = (x_off+1) * (bricklet_size[0]-1);
            extents[2] = y_off * (bricklet_size[1]-1);
            extents[3]  = (y_off+1) * (bricklet_size[1]-1);
            extents[4] = z_off * (bricklet_size[2]-1);
            extents[5]  = (z_off+1) * (bricklet_size[2]-1);
            rdb->SetIndicesForRectGrid(i, extents);
        }
        rdb->CalculateBoundaries();

        void_ref_ptr vr = void_ref_ptr(rdb,
                                   avtStructuredDomainBoundaries::Destruct);
        cache->CacheVoidRef("any_mesh",
                       AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1, vr);
    }
}


// ****************************************************************************
//  Method: avtBOVFileFormat::ReadTOC
//
//  Purpose:
//      Reads in the table of content and initializes the file format object.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Sep 15 11:12:32 PDT 2003
//    Allow for centering to be specified.
//
//    Hank Childs, Sun May  9 08:51:38 PDT 2004
//    Add support for byte data.  Also interpret palette min/max as variable
//    min/max as well.
//
// ****************************************************************************

void
avtBOVFileFormat::ReadTOC(void)
{
    const char *fname = filenames[0];

    ifstream ifile(fname);
    if (ifile.fail())
    {
        EXCEPTION1(InvalidFilesException, fname);
    }

    char buff[32768]; // Is this big enough?
    while (!ifile.eof())
    {
        buff[0] = '\0';
        char *line = buff;
        ifile.getline(line, 32768);
        int nparts = FormatLine(line);
        if (nparts <= 0)
            continue;
        if (line[0] == '#')
            continue;
        else if (strcmp(line, "TIME:") == 0)
        {
            line += strlen("TIME:") + 1;
            cycle = atoi(line);
        }
        else if (strcmp(line, "DATA_FILE:") == 0)
        {
            line += strlen("DATA_FILE:") + 1;
            int len = strlen(line);
            if (file_pattern != NULL)
                delete [] file_pattern;
            file_pattern = new char[len+1];
            strcpy(file_pattern, line);
        }
        else if (strcmp(line, "DATA_SIZE:") == 0)
        {
            line += strlen("DATA_SIZE:") + 1;
            full_size[0] = atoi(line);
            line += strlen(line)+1;
            full_size[1] = atoi(line);
            line += strlen(line)+1;
            full_size[2] = atoi(line);
        }
        else if (strcmp(line, "DATA_FORMAT:") == 0)
        {
            line += strlen("DATA_FORMAT:") + 1;
            if (strncmp(line, "FLOAT", strlen("FLOAT")) == 0)
                byteData = false;
            else if (strncmp(line, "BYTE", strlen("BYTE")) == 0)
                byteData = true;
            else
                debug1 << "Unknown keyword for BOV byte data: " 
                       << line << endl;
        }
        else if (strcmp(line, "DATA_BRICKLETS:") == 0)
        {
            line += strlen("DATA_BRICKLETS:") + 1;
            bricklet_size[0] = atoi(line);
            line += strlen(line)+1;
            bricklet_size[1] = atoi(line);
            line += strlen(line)+1;
            bricklet_size[2] = atoi(line);
        }
        else if (strcmp(line, "VARIABLE:") == 0)
        {
            line += strlen("VARIABLE:") + 1;
            int len = strlen(line);
            if (varname != NULL)
                delete [] varname;
            varname = new char[len+1];
            strcpy(varname, line);
        }
        else if (strcmp(line, "HAS_BOUNDARY:") == 0)
        {
            line += strlen("HAS_BOUNDARY:") + 1;
            if (strcmp(line, "true") == 0)
            {
                hasBoundaries = true;
            }
        }
        else if (strcmp(line, "DATA_ENDIAN:") == 0)
        {
            line += strlen("DATA_ENDIAN:") + 1;
            if (strcmp(line, "LITTLE") == 0)
                littleEndian = true;
            else
                littleEndian = false;
            declaredEndianess = true;
        }
        else if (strcmp(line, "VARIABLE_PALETTE_MIN:") == 0)
        {
            line += strlen("VARIABLE_PALETTE_MIN:") + 1;
            min = atof(line);
        }
        else if (strcmp(line, "VARIABLE_PALETTE_MAX:") == 0)
        {
            line += strlen("VARIABLE_PALETTE_MAX:") + 1;
            max = atof(line);
        }
        else if (strcmp(line, "VARIABLE_MIN:") == 0)
        {
            line += strlen("VARIABLE_MIN:") + 1;
            min = atof(line);
        }
        else if (strcmp(line, "VARIABLE_MAX:") == 0)
        {
            line += strlen("VARIABLE_MAX:") + 1;
            max = atof(line);
        }
        else if (strcmp(line, "BRICK_ORIGIN:") == 0)
        {
            line += strlen("BRICK_ORIGIN:") + 1;
            origin[0] = atof(line);
            line += strlen(line)+1;
            origin[1] = atof(line);
            line += strlen(line)+1;
            origin[2] = atof(line);
        }
        else if (strcmp(line, "BRICK_SIZE:") == 0)
        {
            line += strlen("BRICK_SIZE:") + 1;
            dimensions[0] = atof(line);
            line += strlen(line)+1;
            dimensions[1] = atof(line);
            line += strlen(line)+1;
            dimensions[2] = atof(line);
        }
        else if (strcmp(line, "VARIABLE_BRICK_MIN:") == 0)
        {
            line += strlen("VARIABLE_BRICK_MIN:") + 1;
            int nbricks = nparts-1;
            if (var_brick_min != NULL)
                delete [] var_brick_min;
            var_brick_min = new float[nbricks];
            for (int i = 0 ; i < nbricks ; i++)
            {
                var_brick_min[i] = atof(line);
                if (i != nbricks-1)
                    line += strlen(line) + 1;
            }
        }
        else if (strcmp(line, "VARIABLE_BRICK_MAX:") == 0)
        {
            line += strlen("VARIABLE_BRICK_MAX:") + 1;
            int nbricks = nparts-1;
            if (var_brick_max != NULL)
                delete [] var_brick_max;
            var_brick_max = new float[nbricks];
            for (int i = 0 ; i < nbricks ; i++)
            {
                var_brick_max[i] = atof(line);
                if (i != nbricks-1)
                    line += strlen(line) + 1;
            }
        }
        else if (strcmp(line, "CENTERING:") == 0)
        {
            line += strlen("CENTERING:") + 1;
            if (strstr(line, "zon") != NULL)
                nodalCentering = false;
        }
    }
}


// ****************************************************************************
//  Function: FormatLine
//
//  Purpose:
//      Formats a line.  This means removing whitespace and putting null
//      characters between words.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
//  Modifications:
//
//    Hank Childs, Sun May  9 07:56:42 PDT 2004
//    Combine words before the first colon using underscores.
//
// ****************************************************************************

static int
FormatLine(char *line)
{
    char buff[32768];
    int nwords = 0;
 
    bool inWord = false;
    bool inQuotes = false;
    int buffOffset = 0;
    int nchar = strlen(line);
    bool hasColon = (strstr(line, ":") != NULL);
    bool foundColon = false;
    int i;
    for (i = 0 ; i < nchar ; i++)
    {
        if (line[i] == '\"')
            inQuotes = (inQuotes ? false : true);
        if (line[i] == ':')
            foundColon = true;
 
        bool is_space = isspace(line[i]);
        if (inQuotes)
            is_space = false;
 
        if (inWord)
        {
            if (is_space)
            {
                if (hasColon && !foundColon)
                    buff[buffOffset++] = '_';
                else
                {
                    buff[buffOffset++] = '\0';
                    inWord = false;
                }
            }
            else
            {
                if (line[i] != '\"')
                    buff[buffOffset++] = line[i];
            }
        }
        else
        {
            if (!is_space)
            {
                inWord = true;
                if (line[i] != '\"')
                    buff[buffOffset++] = line[i];
                nwords++;
            }
        }
    }
 
    // Make sure we have a trailing '\0'
    buff[buffOffset++] = '\0';
 
    for (i = 0 ; i < buffOffset ; i++)
        line[i] = buff[i];

    return nwords;
}


