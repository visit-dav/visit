// ************************************************************************* //
//                            avtCMATFileFormat.C                            //
// ************************************************************************* //

#include <avtCMATFileFormat.h>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabaseMetaData.h>

#include <InvalidFilesException.h>
#include <DebugStream.h>

const int avtCMATFileFormat::n_zones_per_dom = 100000;

// ****************************************************************************
// Method: avtCMAT constructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 13:33:58 PST 2004
//
// Modifications:
//
// ****************************************************************************

avtCMATFileFormat::avtCMATFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1), title()
{
    xdims = 0, ydims = 0;
    xmin = xmax = 0.f;
    ymin = ymax = 0.f;
    nYPerDomain = 1;
    numDomains = 1;
    data = 0;
}

// ****************************************************************************
// Method: avtCMATFileFormat::~avtCMATFileFormat
//
// Purpose: 
//   Destructor for avtCMATFileFormat.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 13:44:27 PST 2004
//
// Modifications:
//   
// ****************************************************************************

avtCMATFileFormat::~avtCMATFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
// Method: avtCMATFileFormat::FreeUpResources
//
// Purpose:
//     When VisIt is done focusing on a particular timestep, it asks that
//     timestep to free up any resources (memory, file descriptors) that
//     it has associated with it.  This method is the mechanism for doing
//     that.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 13:33:58 PST 2004
//
// ****************************************************************************

void
avtCMATFileFormat::FreeUpResources(void)
{
    if(data)
    {
        delete [] data;
        data = 0;
    }
}

// ****************************************************************************
// Method: avtCMATFileFormat::PopulateDatabaseMetaData
//
// Purpose:
//     This database meta-data object is like a table of contents for the
//     file.  By populating it, you are telling the rest of VisIt what
//     information it can request from you.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 13:33:58 PST 2004
//
// Modifications:
//
// ****************************************************************************

void
avtCMATFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // Set the database comment by initializing the database.
    TRY
    {
        Initialize(false);
        md->SetDatabaseComment(title);
    }
    CATCH(InvalidFilesException)
    {
#ifndef MDSERVER
        // Only rethrow the exception on the engine.
        RETHROW;
#endif
    }
    ENDTRY

    // Add the mesh
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Mesh";
    mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->numBlocks = numDomains;
    mesh->blockOrigin = 1;
    mesh->spatialDimension = 2;
    mesh->topologicalDimension = 2;
    mesh->hasSpatialExtents = true;
    mesh->minSpatialExtents[0] = xmin;
    mesh->minSpatialExtents[1] = ymin;
    mesh->minSpatialExtents[2] = 0.f;
    mesh->maxSpatialExtents[0] = xmax;
    mesh->maxSpatialExtents[1] = ymax;
    mesh->maxSpatialExtents[2] = 0.f;
    md->Add(mesh);

    // Add the variable.
    AddScalarVarToMetaData(md, "M", mesh->name, AVT_ZONECENT, NULL);
}

// ****************************************************************************
// Method: avtCMATFileFormat::Initialize
//
// Purpose: 
//   Initializes certain members by reading the file.
//
// Note:       The problem size data is only read on the engine.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 14:14:46 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
avtCMATFileFormat::Initialize(bool readData)
{
    const char *mName = "avtCMATFileFormat::Initialize: ";

    if(data == 0)
    {
        // Open the file.
        ifstream ifile(filenames[0]);
        if (ifile.fail())
        {
            EXCEPTION1(InvalidFilesException, filenames[0]);
        }

        // Read in the title section.
        char line[1024];
        ifile.getline(line, 1024);
        title = std::string(line);

        // Read in xdims, xmin, xmax, ydims, ymin, ymax
        ifile.getline(line, 1024); // skip
        ifile.getline(line, 1024);
        float dx, dy;
        if(sscanf(line, "%d %f %f %f  %d %f %f %f",
            &xdims, &xmin, &xmax, &dx,
            &ydims, &ymin, &ymax, &dy) != 8)
        {
            xdims = ydims = 0;
            EXCEPTION1(InvalidFilesException, filenames[0]);
        }

        // Figure out the number of domains. If the product of xdims and ydims
        // is non-zero then we know how many zones there are and we can
        // split up the problem into some number of domains.
        int nzones;
        if((nzones = xdims * ydims) > 0)
        {
            nYPerDomain = 1;
            for( ; nYPerDomain * ydims < n_zones_per_dom; ++nYPerDomain);
    
            int ndoms = nzones / (nYPerDomain * ydims);
            if(ndoms * (nYPerDomain * ydims) < nzones)
                ++ndoms;
            numDomains = ndoms;
            debug4 << "Splitting dataset into " << numDomains << " domains" << endl;
        }

        debug4 << mName << endl;
        debug4 << "\txdims = " << xdims << endl;
        debug4 << "\txmin = " << xmin << endl;
        debug4 << "\txmax = " << xmax << endl;
        debug4 << "\tydims = " << ydims << endl;
        debug4 << "\tymin = " << ymin << endl;
        debug4 << "\tymax = " << ymax << endl;
        debug4 << "\tnumDomains = " << numDomains << endl;
        debug4 << "\tnYPerDomain = " << nYPerDomain << endl;

#ifndef MDSERVER
        if(readData && nzones > 0)
        {
            // If we're reading in the data, read it now.
            data = new float[nzones];
            float *fptr = data;
            debug4 << mName << "Allocated " << nzones << " floats" << endl;
            for(int i = 0; i < nzones; ++i)
            {
                if(ifile.eof())
                    *fptr++ = 0.f;
                else
                {
                    int val;
                    ifile >> val;
                    *fptr++ = float(val);
                }
            }
            debug4 << mName << "Done reading data." << endl;
        }
#endif
    }
}

// ****************************************************************************
// Method: avtCMATFileFormat::GetMesh
//
// Purpose:
//     Gets the mesh associated with this file.  The mesh is returned as a
//     derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//     vtkUnstructuredGrid, etc).
//
// Arguments:
//    domain   : The domain whose data we want.
//    meshname : The name of the mesh of interest.  This can be ignored if
//               there is only one mesh.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 13:33:58 PST 2004
//
// Modifications:
//
// ****************************************************************************

vtkDataSet *
avtCMATFileFormat::GetMesh(int domain, const char *)
{
#ifdef MDSERVER
    return 0;
#else
    Initialize(false);

    const char *mName = "avtCMATFileFormat::GetMesh: ";

    // We have to cut up y.
    int minYIndex = domain * nYPerDomain;
    int maxYIndex = minYIndex + nYPerDomain;
    if(maxYIndex >= ydims-1)
        maxYIndex = ydims;
    int size = maxYIndex - minYIndex;
    float min_t = float(minYIndex) / float(ydims-1);
    float max_t = float(maxYIndex) / float(ydims-1);
    float dY = ymax - ymin;
    float min_y = ymin + min_t * dY;
    float max_y = ymin + max_t * dY;

    debug4 << mName << "domain " << domain << " y = [" << min_y << ", "
           << max_y << "] size+1=" << size+1 << endl;

    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 
    int dims[3] = {xdims+1, size+1, 1};
    float minvals[3] = {xmin, min_y, 0.};
    float maxvals[3] = {xmax, max_y, 0.};
    int ndims = 2;
    vtkFloatArray *coords[3];

    for (int i = 0 ; i < 3 ; i++)
    {
        // Default number of components for an array is 1.
        coords[i] = vtkFloatArray::New();

        if (i < ndims)
        {
            coords[i]->SetNumberOfTuples(dims[i]);
            for (int j = 0; j < dims[i]; j++)
            {
                float t = float(j) / float(dims[i]-1);
                float val = (t * maxvals[i]) + ((1.f - t) * minvals[i]);
                coords[i]->SetComponent(j, 0, val);
            }
        }
        else
        {
            dims[i] = 1;
            coords[i]->SetNumberOfTuples(1);
            coords[i]->SetComponent(0, 0, 0.);
        }
    }

    rgrid->SetDimensions(dims);
    rgrid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    rgrid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    rgrid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    return rgrid;
#endif
}


// ****************************************************************************
// Method: avtCMATFileFormat::GetVar
//
// Purpose:
//     Gets a scalar variable associated with this file.  Although VTK has
//     support for many different types, the best bet is vtkFloatArray, since
//     that is supported everywhere through VisIt.
//
// Arguments:
//    domain  : The domain whose data we want.
//    varname : The name of the variable requested.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 13:33:58 PST 2004
//
// Modifications:
//
// ****************************************************************************

vtkDataArray *
avtCMATFileFormat::GetVar(int domain, const char *varname)
{
#ifdef MDSERVER
    return 0;
#else
    Initialize(true);

    // Figure out the offset into the data.
    int size = nYPerDomain * xdims;
    int offset = domain * size;
    int nzones = xdims * ydims;
    if(offset + size >= nzones)
        size = nzones % size;

    debug4 << "avtCMATFileFormat::GetVar: domain=" << domain
           << ", offset = " << offset << ", size=" << size << endl;

    // Copy a block of floats into a new VTK float array.
    vtkFloatArray *f = vtkFloatArray::New();
    f->SetNumberOfTuples(size);
    const float *src = data + offset;
    float *dest = (float *)f->GetVoidPointer(0);
    for(int i = 0; i < size; ++i)
        *dest++ = *src++;

    return f;
#endif
}

