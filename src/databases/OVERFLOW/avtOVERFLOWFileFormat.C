// ************************************************************************* //
//                            avtOVERFLOWFileFormat.C                           //
// ************************************************************************* //

#include <avtOVERFLOWFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>

#include <avtDatabaseMetaData.h>
#include <avtDatabase.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <visit-config.h>
#include <snprintf.h>

using std::string;
using std::map;

//#define SWAP_4_assumeinteger(x) ( ((x) << 24) | (((x) << 8) & 0x00ff0000) | (((x) >> 8) & 0x0000ff00) | ((x) >> 24) )
#define SWAP_4(x) {unsigned char *_v = (unsigned char*)&x; unsigned char t; t=_v[0];_v[0]=_v[3];_v[3]=t; t=_v[1];_v[1]=_v[2];_v[2]=t; }


// ****************************************************************************
//  Method: avtOVERFLOW constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   July 21, 2004
//
// ****************************************************************************

avtOVERFLOWFileFormat::avtOVERFLOWFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1)
{
    origfilename = filename;
    ndomains = 0;
}

// ****************************************************************************
//  Method:  avtOVERFLOWFileFormat::InitializeFile
//
//  Purpose:
//    Create grid/solution file names, read the headers from the
//    respective files, and determine endianness
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 21, 2004
//
//  Modifications:
//    Jeremy Meredith, Wed Aug 11 13:56:47 PDT 2004
//    Allow naming convention to have a single grid file for all cycles.
//
// ****************************************************************************
void
avtOVERFLOWFileFormat::InitializeFile()
{
    // This is a hack to create a grid and solution file
    // name, no matter which one was given.  We just force
    // the first character of the filename to an x or a q
    // and see if that gives us good files.

    //
    // Find the last slash -- indicates the first character of the filename
    //
    int lastslash;
    for (lastslash=origfilename.size()-2; lastslash>0; lastslash--)
    {
        if (origfilename[lastslash]=='/')
            break;
    }

    //
    // Now change the first character after it to an x/q
    //
    string gridfilename(origfilename);
    string solfilename(origfilename);

    gridfilename[lastslash+1] = 'x';
    solfilename[lastslash+1] = 'q';

    //
    // Try to open the files
    //
    gridin.open(gridfilename.c_str());

    if (!gridin)
    {
        // Okay, that didn't work.  But it might simply be the case that
        // there was only one grid file for all cycles; remove the cycle
        // number from the grid file name and try again.  This is assumed
        // to be all digits in the file name itself.
        string gridfilename_withnumbers = gridfilename;
        gridfilename = "";
        for (int i=0; i<gridfilename_withnumbers.length(); i++)
        {
            if (i <= lastslash ||
                gridfilename_withnumbers[i] < '0' ||
                gridfilename_withnumbers[i] > '9')
            {
                gridfilename += gridfilename_withnumbers[i];
            }
        }

        gridin.open(gridfilename.c_str());

        if (!gridin)
        {
            gridin.close();
            EXCEPTION1(InvalidFilesException, gridfilename.c_str());
        }
    }

    solin.open(solfilename.c_str());
    if (!solin)
    {
        gridin.close();
        solin.close();
        EXCEPTION1(InvalidFilesException, solfilename.c_str());
    }

    //
    // Determine endianness of the file:
    //
    // We are assuming a FORTRAN unformatted file, which means that
    // the first integer in the file is a record length.  we are also
    // assuming multi-grid, which means that the first record will be
    // an integer containing the number of blocks.  This means that we
    // should get a 4from the first four byte integer no matter what.
    //
    int testvar;
    gridin.read((char*)&testvar, 4);
    if (testvar == 4)
    {
        swap_endian = false;
    }
    else
    {
        SWAP_4(testvar);
        if (testvar == 4)
        {
            swap_endian = true;
        }
        else
        {
            gridin.close();
            solin.close();
            EXCEPTION1(InvalidFilesException, gridfilename.c_str());
        }
    }
    // return to the beginning of the file
    gridin.seekg(0, ios::beg);


    //
    // Read the grid and solution headers
    //
    ReadGridHeader();
    ReadSolHeader();
}

// ****************************************************************************
//  Method:  avtOVERFLOWFileFormat::ReadGridHeader
//
//  Purpose:
//    Read the header from the grid file.  It contains
//    sizes of each domain.  Keep track of where the header
//    ended in the file.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 21, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Aug 11 17:49:12 PST 2004
//    Made it compile on Windows.
//
// ****************************************************************************

void
avtOVERFLOWFileFormat::ReadGridHeader()
{
    // The first record is the number of domains
    char *ndomsbuffer = read_fortran_record(gridin);
    char *ptr = ndomsbuffer;

    ndomains = parse_int(ptr);
    delete[] ndomsbuffer;

    // The next one contains ndomains triplets of nx,ny,nz sizes
    char *dimsbuffer = read_fortran_record(gridin);
    ptr = dimsbuffer;

    nx = new int[ndomains];
    ny = new int[ndomains];
    nz = new int[ndomains];

    for (int i=0; i<ndomains; i++)
    {
        nx[i] = parse_int(ptr);
        ny[i] = parse_int(ptr);
        nz[i] = parse_int(ptr);
    }

    delete[] dimsbuffer;

    //
    // Keep track of the start position of the real data so we can
    // seke directly to it later.
    //
    start_of_coords = gridin.tellg();
}

// ****************************************************************************
//  Method:  avtOVERFLOWFileFormat::ReadSolHeader
//
//  Purpose:
//    Read the header from the solution file.  It contains
//    some information redundant with the grid file (like domain
//    sizes), but other information like number of mesh variables
//    and number of species.  Also, read the global variables
//    from the first domain's record.  Keep track of where the
//    header ended in the file.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 21, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Aug 11 17:49:12 PST 2004
//    Made it compile on Windows.
//
// ****************************************************************************

void
avtOVERFLOWFileFormat::ReadSolHeader()
{
    // The first record is the number of domains, just
    // like the grid file.
    char *ndomsbuffer = read_fortran_record(solin);
    delete[] ndomsbuffer;

    // Then we've got all the dims, just like the grid file, 
    // except that after all the dims come NQ and NQC, which we need.
    char *dimsbuffer = read_fortran_record(solin);
    char *ptr = dimsbuffer;

    int i;
    for (i=0; i<ndomains; i++)
    {
        ptr += 12;
    }

    nq = parse_int(ptr);
    nqc = parse_int(ptr);
    // When we retrieve the global variables, it expects at least
    // two RGAS values, even if NQC is less than two.  Don't know why.
    nspec = (2 > nqc) ? 2 : nqc;

    delete[] dimsbuffer;

    //
    // Keep track of the start position of the real data so we can
    // seek directly to it later.
    //
    start_of_data = solin.tellg();

    //
    // Read global variables
    //
    char *varbuff = read_fortran_record(solin);
    char *varbuffptr = varbuff;
    varmap["FSMACH"] = parse_float(varbuffptr);
    varmap["ALPHA"] = parse_float(varbuffptr);
    varmap["REY"] = parse_float(varbuffptr);
    varmap["TIME"] = parse_float(varbuffptr);
    varmap["GAMINF"] = parse_float(varbuffptr);
    varmap["BETA"] = parse_float(varbuffptr);
    varmap["TINF"] = parse_float(varbuffptr);
    varmap["IGAM"] = parse_float(varbuffptr);
    varmap["HTINF"] = parse_float(varbuffptr);
    varmap["HT1"] = parse_float(varbuffptr);
    varmap["HT2"] = parse_float(varbuffptr);
    for (i=0; i<nspec; i++)
    {
        char rgas[1024];
        SNPRINTF(rgas, 1024, "RGAS%d", i+1);
        varmap[rgas] = parse_float(varbuffptr);
    }
    varmap["FSMACH"] = parse_float(varbuffptr);
    varmap["TVREF"] = parse_float(varbuffptr);
    varmap["DTVREF"] = parse_float(varbuffptr);
    delete[] varbuff;
}

// ****************************************************************************
//  Method:  avtOVERFLOWFileFormat::ReadCoords
//
//  Purpose:
//    Read the coordinate (x,y,z) and IBLANK data for a single domain.
//
//  Arguments:
//    domain     the domain number (zero-origin)  (i)
//    x,y,z,ib   the output arrays                (o)
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 21, 2004
//
// ****************************************************************************
void
avtOVERFLOWFileFormat::ReadCoords(int domain,
                                  float *&x, float *&y, float *&z, int *&ib)
{
    //
    // Go to the beginning of the coordinate data, and skip over
    // domains one at a time until you reach the right one.  This
    // could have also been done with a direct numerical calculation
    // but this was just as easy, and the seeking seemed fast enough.
    //
    int record_size;
    gridin.seekg(start_of_coords, ios::beg);
    for (int i=0; i<domain; i++)
    {
        record_size = read_int(gridin);
        gridin.seekg(record_size + 4, ios::cur);
    }

    //
    // Allocate array space
    //
    int npts = nx[domain]*ny[domain]*nz[domain];
    x  = new float[npts];
    y  = new float[npts];
    z  = new float[npts];
    ib = new int[npts];

    //
    // Read the raw buffers
    //
    record_size = read_int(gridin);
    gridin.read((char*)x, 4*npts);
    gridin.read((char*)y, 4*npts);
    gridin.read((char*)z, 4*npts);
    gridin.read((char*)ib, 4*npts);

    //
    // Swap endianness if necessary
    //
    if (swap_endian)
    {
        for (int i=0; i<npts; i++)
        {
            SWAP_4(x[i]);
            SWAP_4(y[i]);
            SWAP_4(z[i]);
            SWAP_4(ib[i]);
        }
    }
}

// ****************************************************************************
//  Method:  avtOVERFLOWFileFormat::ReadCoords
//
//  Purpose:
//    Read the coordinate (x,y,z) and IBLANK data for a single domain.
//
//  Arguments:
//    domain     the domain number (zero-origin)          (i)
//    var        the index of the variable (zero-origin)  (i)
//    vals       the output array                         (o)
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 21, 2004
//
// ****************************************************************************
void
avtOVERFLOWFileFormat::ReadVariable(int domain, int var, float *&vals)
{
    //
    // Go to the beginning of the coordinate data, and skip over
    // domains one at a time until you reach the right one.  This
    // could have also been done with a direct numerical calculation
    // but this was just as easy, and the seeking seemed fast enough.
    //
    // This is a little different from ReadCoords because the
    // solution file has two records per domain -- the global variables
    // and the mesh variables.
    int record_size;
    solin.seekg(start_of_data, ios::beg);
    for (int i=0; i<domain; i++)
    {
        record_size = read_int(solin);
        solin.seekg(record_size + 4, ios::cur);
        record_size = read_int(solin);
        solin.seekg(record_size + 4, ios::cur);
    }

    //
    // And skip over the globals for this domain
    //
    record_size = read_int(solin);
    solin.seekg(record_size + 4, ios::cur);

    //
    // Allocate array space
    //
    int npts = nx[domain]*ny[domain]*nz[domain];
    vals = new float[npts];

    //
    // Skip over the other variables, since var is an index
    //
    solin.seekg(var*npts*4, ios::cur);

    //
    // Read the raw data
    //
    record_size = read_int(solin);
    solin.read((char*)vals, 4*npts);

    //
    // Swap endianness if necessary
    //
    if (swap_endian)
    {
        for (int i=0; i<npts; i++)
        {
            SWAP_4(vals[i]);
        }
    }
}

// ****************************************************************************
//  Method: avtOVERFLOWFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: meredith -- generated by xml2avt
//  Creation:   Tue Jul 20 13:00:41 PST 2004
//
// ****************************************************************************

void
avtOVERFLOWFileFormat::FreeUpResources(void)
{
    gridin.close();
    solin.close();

    delete[] nx;
    delete[] ny;
    delete[] nz;

    //
    // ndomains is our key to know when we closed the files
    //
    ndomains = 0;
    nx = ny = nz = NULL;
}


// ****************************************************************************
//  Method: avtOVERFLOWFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   July 21, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Aug 11 17:56:42 PST 2004
//    Made it build on Windows.
//
// ****************************************************************************

void
avtOVERFLOWFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    //
    // We are using ndomains as a flag to indicate if we have read
    // the file.
    //
    if (!ndomains)
    {
        InitializeFile();
    }

    //
    // Create the mesh metadata
    //
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_CURVILINEAR_MESH;
    mesh->numBlocks = ndomains;
    mesh->blockOrigin = 1;
    mesh->cellOrigin = 1;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->blockTitle = "Zones";
    mesh->blockPieceName = "Zone";
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    //
    // Add our variable names
    //
    for (int i=0; i<nq; i++)
    {
        char name[1024];
        SNPRINTF(name, 1024, "Q%d", i+1);
        AddScalarVarToMetaData(md, name, "mesh", AVT_NODECENT);
    }
    AddScalarVarToMetaData(md, "gn", "mesh", AVT_NODECENT);

    //
    // The "global" variables work well as expressions with no type
    //
    for (std::map<string,float>::iterator it=varmap.begin(); it!=varmap.end(); it++)
    {
        Expression exp;
        exp.SetName(it->first);
        char def[1024];
        sprintf(def, "%f", it->second);
        exp.SetDefinition(def);
        exp.SetType(Expression::Unknown);
        md->AddExpression(&exp);
    }

    //
    // And don't forget we got TIME for this file!
    //
    md->SetTime(timestep, varmap["TIME"]);
}


// ****************************************************************************
//  Method: avtOVERFLOWFileFormat::GetMesh
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
//  Creation:   July 21, 2004
//
// ****************************************************************************

vtkDataSet *
avtOVERFLOWFileFormat::GetMesh(int domain, const char *meshname)
{
    float *x, *y, *z;
    int *ib;
    ReadCoords(domain, x,y,z,ib);

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
    dims[0] = nx[domain];
    dims[1] = ny[domain];
    dims[2] = nz[domain];
    sgrid->SetDimensions(dims);

    //
    // Populate the coordinates and ghost node levels.
    //
    int npts = nx[domain]*ny[domain]*nz[domain];
    points->SetNumberOfPoints(npts);
    float *pts = (float *) points->GetVoidPointer(0);

    vtkUnsignedCharArray *gn = vtkUnsignedCharArray::New();
    gn->SetName("vtkGhostNodes");
    gn->SetNumberOfTuples(npts);
    sgrid->GetPointData()->AddArray(gn);
    gn->Delete();

    unsigned char *gn_raw = gn->GetPointer(0);

    for (int i=0; i<npts; i++)
    {
        *pts++ = x[i];
        *pts++ = y[i];
        *pts++ = z[i];

        // From the document at:
        // http://www.nas.nasa.gov/FAST/RND-93-010.walatka-clucas/htmldocs/chp5.file_io.html
        // concerning IBLANKing:

        //     This indicates that the grid file has integer
        //     IBLANKing. IBLANKing is used to blank out regions of the
        //     grid geometry that should not be plotted and to indicate
        //     the zone interfaces for use in calculating particle
        //     traces. A single integer IBLANK value for each grid point
        //     describes the treatment of the grid and solution at that
        //     point. The file format for IBLANKed grids is included in
        //     the File Format section starting on page 112XREF. The
        //     solution and function files which match IBLANKed grids are
        //     unchanged. Acceptable IBLANK values and their descriptions
        //     are listed below.

        //     IBLANK = 1: Grid points requiring no special treatment
        //     (that is, those not at a boundary where IBLANKing is being
        //     used) are assigned a positive IBLANK value. The convention
        //     is to use the value 1.

        //     IBLANK = 0: The point is turned off. When multiple grids
        //     overlap (for example, a wing and a fuselage) the
        //     meaningless data points of one grid which lie inside a
        //     solid body of another grid should be turned off; the grid
        //     point is not drawn and the solution data is never used. If
        //     IBLANK is anything but zero, the data at that point are
        //     assumed valid.

        //     IBLANK = -n: Continue into grid n. In simulations involving
        //     multiple grid zones or a periodic grid, IBLANKing may be
        //     used at grid interfaces to indicate a continuation of
        //     physical space. Without IBLANKing, particle traces will
        //     stop when they reach the edge of a grid. Assigning IBLANK =
        //     -n (negative n) at the boundary allows a trace calculation
        //     to continue from the current grid, m, into grid
        //     n. Likewise, the grid points in grid n along the interface
        //     with grid m should be assigned IBLANK values of -m. In the
        //     case of a single periodic mesh or a c-mesh, the trace will
        //     continue back into the current grid m, if IBLANK = -m is
        //     assigned at all common boundaries.

        // Now, you might think that this means that anything with an
        // IBLANK value of "0" would get a nonzero ghost level, and
        // anything with an IBLANK value of "1" would get a zero ghost
        // level, but you might not be sure what to do with the negative
        // IBLANK values, though the first choice would be to have them
        // take a nonzero ghost level.

        // For some reason the first assumption is wrong -- turning a
        // zero IBLANK value seemed to remove more geometry than I wanted.

        // Given that, I am only having those nodes with negative IBLANK
        // values take a nonzero ghost node value, but leaving the zero
        // IBLANKs as zero ghost node values.  I am also making use of the
        // new "2" ghost node value, as these should be treated slightly
        // differently from our other usage of ghost nodes.

        *gn_raw++ = (ib[i] < 0) ? 2 : 0;
    }


    delete[] x;
    delete[] y;
    delete[] z;
    delete[] ib;

    return sgrid;
}


// ****************************************************************************
//  Method: avtOVERFLOWFileFormat::GetVar
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
//  Creation:   July 21, 2004
//
// ****************************************************************************

vtkDataArray *
avtOVERFLOWFileFormat::GetVar(int domain, const char *varname)
{
    // We assume variables are named Q# or Q## (etc.)
    if (strlen(varname) < 2)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    int varindex = atoi(&varname[1]) - 1;
    if (varindex < 0 || varindex >= nq)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Actually read it from the solution file, using this index
    float *vals;
    ReadVariable(domain, varindex, vals);

    int npts = nx[domain]*ny[domain]*nz[domain];

    // Put it in a VTK array
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(npts);
    for (int i = 0 ; i < npts ; i++)
    {
         rv->SetTuple1(i, vals[i]);
    }

    delete[] vals;

    return rv;
}


// ****************************************************************************
//  Method: avtOVERFLOWFileFormat::GetVectorVar
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
//  Creation:   July 21, 2004
//
// ****************************************************************************

vtkDataArray *
avtOVERFLOWFileFormat::GetVectorVar(int domain, const char *varname)
{
    // No vector variables yet
    return NULL;
}



// ****************************************************************************
//  Method:  avtOVERFLOWFileFormat::read_int
//
//  Purpose:
//    Read an integer from the file, swapping byte order if necessary.
//
//  Arguments:
//    in         the input stream
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 21, 2004
//
// ****************************************************************************
int avtOVERFLOWFileFormat::read_int(ifstream &in)
{
    int v;
    in.read((char*)&v, 4);

    if (swap_endian)
        SWAP_4(v);

    return v;
}

// ****************************************************************************
//  Method:  avtOVERFLOWFileFormat::read_fortran_record
//
//  Purpose:
//    Read the header, body, and footer from a file record.
//    Allocate the buffer space for the body.
//    Confirm that the footer matched the header (sanity check).
//
//  Arguments:
//    in         the input stream
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 21, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Aug 11 17:52:07 PST 2004
//    I removed the const from the return pointer since the output is
//    usually deleted with the delete[] operator.
//
// ****************************************************************************

char *avtOVERFLOWFileFormat::read_fortran_record(ifstream &in)
{
    int len = read_int(in);

    char *buffer = new char[len];
    in.read(buffer, len);

    int testlen;
    testlen = read_int(in);

    if (testlen != len)
    {
        EXCEPTION1(VisItException, "Internal error reading "
                   "FORTRAN unformatted record");
    }
    return buffer;
}

// ****************************************************************************
//  Method:  avtOVERFLOWFileFormat::parse_int
//
//  Purpose:
//    Read a single 32-bit integer from a buffer,
//    incrementing the buffer pointer approriately.
//    Swaps byte order if necessary.
//
//  Arguments:
//    buff       the current buffer position
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 21, 2004
//
// ****************************************************************************
int avtOVERFLOWFileFormat::parse_int(char *&buff)
{
    int v = *((int*)(buff));
    buff += 4;

    if (swap_endian)
        SWAP_4(v);

    return v;
}

// ****************************************************************************
//  Method:  avtOVERFLOWFileFormat::parse_float
//
//  Purpose:
//    Read a single precision floating point number from a buffer,
//    incrementing the buffer pointer approriately.
//    Swaps byte order if necessary.
//
//  Arguments:
//    buff       the current buffer position
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 21, 2004
//
// ****************************************************************************
float avtOVERFLOWFileFormat::parse_float(char *&buff)
{
    float v = *((float*)(buff));
    buff += 4;

    if (swap_endian)
        SWAP_4(v);

    return v;
}

