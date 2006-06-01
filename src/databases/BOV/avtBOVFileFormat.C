/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtBOVFileFormat.C                             //
// ************************************************************************* //

#include <avtBOVFileFormat.h>

#include <zlib.h>

#include <visitstream.h>
#include <visit-config.h>
#include <snprintf.h>

#include <vtkUnsignedCharArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtIntervalTree.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtTypes.h>
#include <avtVariableCache.h>

#include <Expression.h>

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
//    Hank Childs, Fri Mar  4 16:02:03 PST 2005
//    Do not read in the entire TOC in this file any more.
//
//    Brad Whitlock, Thu Mar 16 14:56:37 PST 2006
//    Initialized byteOffset to 0 and set divideBrick to false.
//
//    Brad Whitlock, Thu May 4 14:38:24 PST 2006
//    Changed how dataFormat was initialized and added init for
//    dataNumComponents.
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
    byteToFloatTransform = false;
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
    dataFormat = ByteData;
    dataNumComponents = 1;
    byteOffset = 0;
    divideBrick = false;
    byteToFloatTransform = false;

    haveReadTOC = false;
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
//  Method: avtBOVFileFormat::ActivateTimestep
//
//  Purpose:
//      Activates the current timestep, prompting the header to be read.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2005
//
// ****************************************************************************

void
avtBOVFileFormat::ActivateTimestep(void)
{
    ReadTOC();
    if (metadata != NULL)
        metadata->SetCycle(timestep, cycle);
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
//    Hank Childs, Mon Apr 24 09:33:21 PDT 2006
//    Add support for 2D BOV files.
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
    if (dz == 1)
    {
        z->SetNumberOfTuples(1);
        z->SetTuple1(0, 0.);
    }
    else
    {
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
    }

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

#define BRICKLET_READ
#ifdef BRICKLET_READ
// ****************************************************************************
// Method: ReadBricklet
//
// Purpose: 
//   This is a template function that knows how to use fseek to access
//   a smaller brick of data within a BOV file.
//
// Arguments:
//   fp        : The file pointer to use.
//   dest      : The destination buffer for the data that gets read.
//   full_size : The whole size (x,y,z) of the brick in the file.
//   start     : The starting x,y,z that we want to read.
//   end       : The ending x,y,z that we want to read.
//   offset    : An additional amount (in bytes) to offset from the start
//               of the file.
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 17 15:17:19 PST 2006
//
// Modifications:
//   Brad Whitlock, Fri May 5 13:20:41 PST 2006
//   Modified it so it can read more than 1 component.
//
// ****************************************************************************

template<class T>
void
ReadBricklet(FILE *fp, T *dest, const int *full_size, const int *start,
    const int *end, int offset, int ncomp)
{
    T *ptr = dest;

    // Seek to the right Z page
    long zPage = start[2]*full_size[0]*full_size[1];
    long seekOffset = zPage * sizeof(T) + offset;
    if(seekOffset > 0)
        fseek(fp, seekOffset * ncomp, SEEK_SET);

    // Now start reading the data
    int dx = end[0] - start[0];
    int nxelem = dx * ncomp;
    long extraseek = 0;
    for(int z = start[2]; z < end[2]; ++z)
    {
         // Get to the starting data location in the zPage.
         long corner = start[1]*full_size[0] + start[0];
         seekOffset = (corner + extraseek) * sizeof(T);
         if(seekOffset > 0)
             fseek(fp, seekOffset * ncomp, SEEK_CUR);
         extraseek = 0;

         for(int y = start[1]; y < end[1]; ++y)
         {
             // Read in a line of data in x.
             fread((void *)ptr, sizeof(T), nxelem, fp);
             ptr += nxelem;

             // Seek to the next line
             if(y < end[1]-1)
             {
                 long right = full_size[0] - end[0];
                 long left = start[0];
                 seekOffset = (right + left) * sizeof(T);
                 if(seekOffset > 0)
                     fseek(fp, seekOffset * ncomp, SEEK_CUR);
             }
         }

         // Seek to the next page
         if(z < end[2]-1)
         {
             long right = full_size[0] - end[0];
             long bottom = (full_size[1] - end[1]) * full_size[0];
             extraseek = right + bottom;
         }
    }
}
#endif

//
// Endian conversion routines.
//

// FROM FConvert.C
static int
int32_Reverse_Endian(int val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 3;
    unsigned char *out = outbuf;

    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;

    return 4;
}

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

static int
double64_Reverse_Endian(double val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 7;
    unsigned char *out = outbuf;

    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;

    return 8;
}

// ****************************************************************************
// Method: ExtractBrick
//
// Purpose: 
//   Extracts a brick of values from 1 array and copies that brick to a new
//   array.
//
// Arguments:
//   dest    : The destination array.
//   src     : The source array.
//   x_start : The starting X index
//   x_stop  : The stopping X index
//   y_start : The starting Y index
//   y_stop  : The stopping Y index
//   z_start : The starting Z index
//   z_stop  : The stopping Z index
//   dx      : The total size of the large array in the X dimension.
//   dy      : The total size of the large array in the Y dimension.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 5 09:57:18 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

template <class T>
inline void
ExtractBrick(T *dest, const T *src, 
    unsigned int x_start, unsigned int x_stop, 
    unsigned int y_start, unsigned int y_stop, 
    unsigned int z_start, unsigned int z_stop, 
    unsigned int dx, unsigned int dy, unsigned int ncomp)
{
    T *buff = dest;

    if(ncomp == 1)
    {
        for (unsigned int i = z_start ; i < z_stop ; i++)
        {
            unsigned int Z = i*dx*dy;
            for (unsigned int j = y_start ; j < y_stop ; j++)
            {
                unsigned int Y = j * dx;
                for (unsigned int k = x_start ; k < x_stop ; k++)
                {
                    unsigned int index = Z + Y + k;
                    *buff++ = src[index];
                }
            }
        }
    }
    else
    {
        unsigned int dX = x_stop - x_start;
        unsigned int nvals = dX * ncomp;

        // Extract N components
        for (unsigned int i = z_start ; i < z_stop ; i++)
        {
            unsigned int Z = i*dx*dy;
            for (unsigned int j = y_start ; j < y_stop ; j++)
            {
                unsigned int Y = j * dx;
                unsigned int index = (Z + Y + x_start) * ncomp;
                const T *xdata = src + index;
                for (unsigned int k = 0 ; k < nvals ; k++)
                    *buff++ = *xdata++;
            }
        }
    }
}

// ****************************************************************************
// Method: ReArrangeTuple2ToTuple3
//
// Purpose: 
//   Template function for moving data from 2-tuple form to 3-tuple form.
//
// Arguments:
//   start   : The pointer to the start of the memory to rearrange.
//   nTuples : The number of tuples to rearrange.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 5 09:37:18 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

template <class T>
inline void
ReArrangeTuple2ToTuple3(T *start, vtkIdType nTuples)
{
    T *src = start + (2 * (nTuples - 1));
    T *dest = start + (3 * (nTuples - 1));
    while(src >= start)
    {
        dest[2] = 0;
        dest[1] = src[1];
        dest[0] = src[0];

        src -= 2;
        dest -= 3;
    }
}

// ****************************************************************************
// Method: avtBOVFileFormat::ReadWholeAndExtractBrick
//
// Purpose: 
//   Reads the whole brick and then copies some smaller brick of it into the
//   destination array.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May 5 11:34:26 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtBOVFileFormat::ReadWholeAndExtractBrick(void *dest, bool gzipped,
    void *file_handle, void *gz_handle,
    unsigned int unit_size,
    unsigned int x_start, unsigned int x_stop, 
    unsigned int y_start, unsigned int y_stop, 
    unsigned int z_start, unsigned int z_stop, 
    unsigned int dx, unsigned int dy,
    unsigned int whole_size)
{
    const char *mName = "avtBOVFileFormat::ReadWholeAndExtractBrick: ";

    // Allocate enough memory to read the whole brick.
    void *whole_buff = 0;
    unsigned int whole_nelem = whole_size * dataNumComponents;
    if(dataFormat == ByteData)
        whole_buff = (void *)(new unsigned char[whole_nelem]);
    else if(dataFormat == IntegerData)
        whole_buff = (void *)(new int[whole_nelem]);
    else if(dataFormat == FloatData)
        whole_buff = (void *)(new float[whole_nelem]);
    else if(dataFormat == DoubleData)
        whole_buff = (void *)(new double[whole_nelem]);

    if(gzipped)
    {
        // Read past the specified offset.
        if(byteOffset > 0)
            gzseek(gz_handle, byteOffset, SEEK_SET);

        // Read the whole dataset
        gzread(gz_handle, whole_buff, whole_nelem * unit_size);
    }
    else
    {
        FILE *fp = (FILE*)file_handle;

        // Read past the specified offset.
        if(byteOffset > 0)
            fseek(fp, byteOffset, SEEK_SET);

        size_t nread = fread(whole_buff, unit_size, whole_nelem, fp);

        debug4 << mName << "Reading " << whole_size << "*" << dataNumComponents
               << " elements of " << unit_size << " bytes. fread returned "
               << nread << endl;
    }

    //
    // Copy the piece that we want into the buffer.
    //
    if (dataFormat == ByteData)
    {
        unsigned char *uc_buff = (unsigned char *)whole_buff;
        ExtractBrick((unsigned char *)dest, uc_buff,
                     x_start, x_stop, y_start, y_stop, z_start, z_stop,
                     dx, dy, dataNumComponents);
        // Delete the array containing the whole BOV
        delete [] uc_buff;
    }
    else if(dataFormat == IntegerData)
    {
        int *i_buff = (int *)whole_buff;
        ExtractBrick((int *)dest, i_buff,
                     x_start, x_stop, y_start, y_stop, z_start, z_stop,
                     dx, dy, dataNumComponents);
        // Delete the array containing the whole BOV
        delete [] i_buff;
    }
    else if(dataFormat == FloatData)
    {
        float *f_buff = (float *)whole_buff;
        ExtractBrick((float *)dest, f_buff,
                     x_start, x_stop, y_start, y_stop, z_start, z_stop,
                     dx, dy, dataNumComponents);
        // Delete the array containing the whole BOV
        delete [] f_buff;
    }
    else if(dataFormat == DoubleData)
    {
        double *d_buff = (double *)whole_buff;
        ExtractBrick((double *)dest, d_buff,
                     x_start, x_stop, y_start, y_stop, z_start, z_stop,
                     dx, dy, dataNumComponents);
        // Delete the array containing the whole BOV
        delete [] d_buff;
    }
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
//    Brad Whitlock, Wed Mar 15 17:30:52 PST 2006
//    Added more support for reading in a piece of a brick (divideBrick).
//
//    Brad Whitlock, Wed May 3 16:24:50 PST 2006
//    Added some debug statements and changed the file access mode in fopen
//    so it is "rb" so fread works as expected on Windows. I also added support
//    for ints and doubles. Finally, I added support for reading in tuples
//    that have multiple components.
//
// ****************************************************************************

vtkDataArray *
avtBOVFileFormat::GetVar(int dom, const char *var)
{
    const char *mName = "avtBOVFileFormat::GetVar: ";
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
        file_handle = fopen(qual_filename, "rb");
        gzipped = false;
    }

    if (file_handle == NULL && gz_handle == NULL)
    {
        EXCEPTION1(InvalidFilesException, qual_filename);
    }

    //
    // Determine the unit_size, which is the size of the data in the file,
    // and allocate the return VTK object.
    //
    int unit_size;
    vtkDataArray *rv = 0;
    if(dataFormat == ByteData)
    {
        unit_size = sizeof(unsigned char);
        vtkUnsignedCharArray *ca = vtkUnsignedCharArray::New();
        rv = ca;
    }
    else if(dataFormat == IntegerData)
    {
        unit_size = sizeof(int);
        vtkIntArray *ia = vtkIntArray::New();
        rv = ia;
    }
    else if(dataFormat == FloatData)
    {
        unit_size = sizeof(float);
        vtkFloatArray *fa = vtkFloatArray::New();
        rv = fa;
    }
    else if(dataFormat == DoubleData)
    {
        unit_size = sizeof(double);
        vtkDoubleArray *da = vtkDoubleArray::New();
        rv = da;
    }
    int ncomp = (dataNumComponents==2)?3:dataNumComponents;
    rv->SetNumberOfComponents(ncomp);

    //
    // Read the data using the various methods.
    //
    if(hasBoundaries)
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
        int total_vals = (dx*dy*dz);

        // Read the parts that we need of the brick and store it in the
        // return VTK array.
        ReadWholeAndExtractBrick(rv->GetVoidPointer(0), gzipped,
            (void*)file_handle, gz_handle,
            unit_size,
            x_start, x_stop, 
            y_start, y_stop, 
            z_start, z_stop, 
            dx, dy,
            total_vals);
    }
    else if(divideBrick)
    {
        debug4 << mName << "Dividing whole brick" << endl;

        // Allocate enough memory for 1 bricklet.
        unsigned int nvals = bricklet_size[0] * 
                             bricklet_size[1] *
                             bricklet_size[2];
        rv->SetNumberOfTuples(nvals);

        int x_start = x_off * bricklet_size[0];
        int y_start = y_off * bricklet_size[1];
        int z_start = z_off * bricklet_size[2];
        int x_stop = x_start + bricklet_size[0];
        int y_stop = y_start + bricklet_size[1];
        int z_stop = z_start + bricklet_size[2];

        debug4 << mName << "byteOffset: " << byteOffset << endl;
        debug4 << mName << "Full size: " << full_size[0] << ", "
               << full_size[1] << ", " << full_size[2] << endl;
        debug4 << mName << "Bricklet start: " << x_start << ", "
               << y_start << ", " << z_start << endl;
        debug4 << mName << "Bricklet stop: " << x_stop << ", "
               << y_stop << ", " << z_stop << endl;
        debug4 << mName << "Number of tuples in bricklet: " << nvals << endl;

#ifdef BRICKLET_READ
        if(!gzipped)
        {
            debug4 << mName << "Reading bricklet directly from file" << endl;
            int start[3] = {x_start, y_start, z_start};
            int stop[3] = {x_stop, y_stop, z_stop};

            if(dataFormat == ByteData)
            {
                debug4 << mName << "Reading char bricklet" << endl;
                // Read the unsigned char data.
                unsigned char *buff = (unsigned char *) rv->GetVoidPointer(0);
                ReadBricklet(file_handle, buff, full_size, start, stop,
                             byteOffset, dataNumComponents);
            }
            else if(dataFormat == IntegerData)
            {
                debug4 << mName << "Reading int bricklet" << endl;
                // Read the int data.
                int *buff = (int *) rv->GetVoidPointer(0);
                ReadBricklet(file_handle, buff, full_size, start, stop,
                             byteOffset, dataNumComponents);
            }
            else if(dataFormat == FloatData)
            {
                debug4 << mName << "Reading float bricklet" << endl;
                // Read the float data.
                float *buff = (float *) rv->GetVoidPointer(0);
                ReadBricklet(file_handle, buff, full_size, start, stop,
                             byteOffset, dataNumComponents);
            }
            else if(dataFormat == DoubleData)
            {
                debug4 << mName << "Reading double bricklet" << endl;
                // Read the double data.
                double *buff = (double *) rv->GetVoidPointer(0);
                ReadBricklet(file_handle, buff, full_size, start, stop,
                             byteOffset, dataNumComponents);
            }
        }
        else
#endif
        {
            debug4 << mName << "Reading whole brick to obtain bricklet" << endl;
            //
            // In this case, we have 1 big brick and we're going to chunk it
            // up into bricklets so we can do some parallel processing upstream.
            //
            unsigned int whole_size = full_size[0] * full_size[1] *full_size[2];
            ReadWholeAndExtractBrick(rv->GetVoidPointer(0), gzipped,
                (void*)file_handle, gz_handle,
                unit_size,
                x_start, x_stop, 
                y_start, y_stop, 
                z_start, z_stop, 
                full_size[0], full_size[1],
                whole_size);
        }
    }
    else // read the whole BOV
    {
        int nvals = bricklet_size[0] * bricklet_size[1] * bricklet_size[2];
        rv->SetNumberOfTuples(nvals);

        debug4 << mName << "Read whole brick" << endl;
        debug4 << mName << "byteOffset: " << byteOffset << endl;
        debug4 << mName << "Full size: " << full_size[0] << ", "
               << full_size[1] << ", " << full_size[2] << endl;
        debug4 << mName << "Created data array of " << nvals << " "
               << ncomp << "-tuples" << endl;
          
        //
        // Read in based on whether or not we have a gzipped file.
        //
        if (gzipped)
        {
            // Read past the specified offset.
            if(byteOffset > 0)
                gzseek(gz_handle, byteOffset, SEEK_SET);

            gzread(gz_handle, rv->GetVoidPointer(0),
                   nvals * dataNumComponents * unit_size);
        }
        else
        {
            // Read past the specified offset.
            if(byteOffset > 0)
            {
                debug4 << mName << "Seeking past " << byteOffset << " bytes" << endl;
                fseek(file_handle, byteOffset, SEEK_SET);
            }
            
            size_t nread = fread(rv->GetVoidPointer(0), unit_size,
                                 nvals * dataNumComponents, file_handle);

            debug4 << mName << "Reading " << nvals << "*" << dataNumComponents
                   << " elements of " << unit_size << " bytes. fread returned "
                   << nread << endl;
        }
    }

    //
    // If we need to apply the byte to float transform, do it now.
    //
    if(byteToFloatTransform)
    {
        debug4 << mName << "Transforming byte data to float data" << endl;
        vtkFloatArray *fa = vtkFloatArray::New();
        fa->SetNumberOfComponents(rv->GetNumberOfComponents());
        fa->SetNumberOfTuples(rv->GetNumberOfTuples());
        float *fptr = (float *)fa->GetVoidPointer(0);
        unsigned char *ucptr = (unsigned char *)rv->GetVoidPointer(0);
        unsigned int nvals = rv->GetNumberOfTuples() * dataNumComponents;
        float d_range = (max - min);
        for(unsigned int i = 0; i < nvals; ++i, ++ucptr)
        {
            *fptr++ = min + d_range * (*ucptr / 255.f);
        }

        // Make sure that we return the new float version instead.
        rv->Delete();
        rv = fa;
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
            int nvals = rv->GetNumberOfTuples();
            unsigned int ntotal = nvals * dataNumComponents;

            if (dataFormat == IntegerData)
            {
                debug4 << mName << "Reversing endian for ints" << endl;
                int *buff = (int *) rv->GetVoidPointer(0);
                for (int i = 0 ; i < ntotal ; i++)
                {
                    int tmp;
                    int32_Reverse_Endian(buff[i], (unsigned char *) &tmp);
                    buff[i] = tmp;
                }
            }
            else if(dataFormat == FloatData)
            {
                debug4 << mName << "Reversing endian for floats" << endl;
                float *buff = (float *) rv->GetVoidPointer(0);
                for (int i = 0 ; i < ntotal ; i++)
                {
                    float tmp;
                    float32_Reverse_Endian(buff[i], (unsigned char *) &tmp);
                    buff[i] = tmp;
                }
            }
            else if(dataFormat == DoubleData)
            {
                debug4 << mName << "Reversing endian for doubles" << endl;
                double *buff = (double *) rv->GetVoidPointer(0);
                for (int i = 0 ; i < ntotal ; i++)
                {
                    double tmp;
                    double64_Reverse_Endian(buff[i], (unsigned char *) &tmp);
                    buff[i] = tmp;
                }
            }
        }
    }

    //
    // Our data arrays contain 2-tuples but to be vectors as far as VisIt is
    // concerned, they need to have 3-tuples. Fortunately, we allocated the
    // data arrays to have room for 3-tuples in this case. We just have to
    // shuffle memory a little.
    //
    if(dataNumComponents == 2)
    {
        debug4 << mName << "Rearranging 2-tuple into 3-tuple" << endl;
        if(dataFormat == ByteData ||
           dataFormat == FloatData)
        {
            ReArrangeTuple2ToTuple3((float *)rv->GetVoidPointer(0),
                                    rv->GetNumberOfTuples());
        }
        else if(dataFormat == IntegerData)
        {
            ReArrangeTuple2ToTuple3((int *)rv->GetVoidPointer(0),
                                    rv->GetNumberOfTuples());
        }
        else if(dataFormat == DoubleData)
        {
            ReArrangeTuple2ToTuple3((double *)rv->GetVoidPointer(0),
                                    rv->GetNumberOfTuples());
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
// Method: avtBOVFileFormat::GetVectorVar
//
// Purpose: 
//   Returns a vector var.
//
// Arguments:
//   dom : The domain to get.
//   var : The variable to get.
//
// Returns:    A data array containing the variable.
//
// Note:       Calls GetVar to do the actual work.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 5 10:34:13 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
avtBOVFileFormat::GetVectorVar(int dom, const char *var)
{
    return GetVar(dom, var);
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
//    Hank Childs, Fri Oct 22 11:52:37 PDT 2004
//    Do not throw an exception if the variable name is foreign to us -- it
//    is probably an expression.
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
            return NULL;
        }
        if (var_brick_min != NULL && var_brick_max != NULL)
        {
            avtIntervalTree *itree = new avtIntervalTree(nbricks, 1);
            for (int i = 0 ; i < nbricks ; i++)
            {
                double range[2] = { var_brick_min[i], var_brick_max[i] };
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
            double bounds[6];
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
//    Hank Childs, Thu Oct 21 11:51:28 PDT 2004
//    Have domain boundary information reflect the differing mesh sizes that
//    come with different centerings.
//
//    Hank Childs, Fri Mar  4 16:02:03 PST 2005
//    Make sure we read the table of contents first.
//
//    Hank Childs, Fri Apr 21 16:59:48 PDT 2006
//    Add support for 2D.
//
//    Brad Whitlock, Thu May 4 14:39:35 PST 2006
//    Added support for multi-component data.
//
// ****************************************************************************

void
avtBOVFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ReadTOC();
    int nx = full_size[0] / bricklet_size[0];
    int ny = full_size[1] / bricklet_size[1];
    int nz = full_size[2] / bricklet_size[2];

    int dim = (full_size[2] == 1 ? 2 : 3);

    int nbricks = nx*ny*nz;
 
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_RECTILINEAR_MESH;
    mesh->numBlocks = nbricks;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = dim;
    mesh->topologicalDimension = dim;
    mesh->blockTitle = "bricks";
    mesh->blockPieceName = "brick";
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    if(dataNumComponents == 1)
    {
        AddScalarVarToMetaData(md, varname, "mesh",
                               (nodalCentering ? AVT_NODECENT : AVT_ZONECENT));
    }
    else if(dataNumComponents == 2)
    {
        // Complex data
        avtVectorMetaData *vmd = new avtVectorMetaData;
        vmd->name = varname;
        vmd->meshName = "mesh";
        vmd->centering = (nodalCentering ? AVT_NODECENT : AVT_ZONECENT);
        vmd->varDim = 2;
        md->Add(vmd);

        // Add expressions to get the real and imaginary components
        Expression *mag = new Expression;
        mag->SetName("real");
        mag->SetDefinition(std::string(varname) + "[0]");
        mag->SetType(Expression::ScalarMeshVar);
        md->AddExpression(mag);
        Expression *phase = new Expression;
        phase->SetName("imaginary");
        phase->SetDefinition(std::string(varname) + "[1]");
        phase->SetType(Expression::ScalarMeshVar);
        md->AddExpression(phase);
    }
    else if(dataNumComponents == 3)
    {
        avtVectorMetaData *vmd = new avtVectorMetaData;
        vmd->name = varname;
        vmd->meshName = "mesh";
        vmd->centering = (nodalCentering ? AVT_NODECENT : AVT_ZONECENT);
        vmd->varDim = 3;
        md->Add(vmd);
    }
    else
    {
        avtArrayMetaData *amd = new avtArrayMetaData;
        amd->name = varname;
        amd->meshName = "mesh";
        amd->centering = (nodalCentering ? AVT_NODECENT : AVT_ZONECENT);
        amd->nVars = dataNumComponents;
        char n[30], def[200];
        for(int i = 0; i < dataNumComponents; ++i)
        {
            SNPRINTF(n, 30, "comp%02d", i);
            amd->compNames.push_back(n);

            SNPRINTF(def, 300, "array_decompose(%s, %d)", varname, i);
            Expression *e = new Expression;
            e->SetName(n);
            e->SetDefinition(def);
            e->SetType(Expression::ScalarMeshVar);
            md->AddExpression(e);
        }
        md->Add(amd);
    }

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
            int correction = (nodalCentering ? 1 : 0);
            extents[0] = x_off * (bricklet_size[0]-correction);
            extents[1]  = (x_off+1) * (bricklet_size[0]-correction);
            extents[2] = y_off * (bricklet_size[1]-correction);
            extents[3]  = (y_off+1) * (bricklet_size[1]-correction);
            if (dim > 2)
            {
                extents[4] = z_off * (bricklet_size[2]-correction);
                extents[5]  = (z_off+1) * (bricklet_size[2]-correction);
            }
            else 
                extents[4] = extents[5] = 0;
            rdb->SetIndicesForRectGrid(i, extents);
        }
        rdb->CalculateBoundaries();

        void_ref_ptr vr = void_ref_ptr(rdb,
                                   avtStructuredDomainBoundaries::Destruct);
        cache->CacheVoidRef("any_mesh",
                       AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1, vr);
    }
    md->SetCycle(timestep, cycle);
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
//    Hank Childs, Fri Mar  4 16:02:03 PST 2005
//    Do not re-read the TOC for each call to this method.  Also added error
//    checking previously in the constructor.
//
//    Brad Whitlock, Thu Mar 16 14:55:52 PST 2006
//    I added support for byteOffset, which lets us skip a header. I also
//    added support for divideBrick.
//
//    Brad Whitlock, Thu May 4 11:59:29 PDT 2006
//    I added support for ints, doubles and multi-component data.
//
// ****************************************************************************

void
avtBOVFileFormat::ReadTOC(void)
{
    if (haveReadTOC)
        return;
    haveReadTOC = true;
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
            if (strncmp(line, "FLOAT", strlen("FLOAT")) == 0 ||
                strncmp(line, "REAL", strlen("REAL")) == 0)
                dataFormat = FloatData;
            else if (strncmp(line, "BYTE", strlen("BYTE")) == 0 ||
                     strncmp(line, "CHAR", strlen("CHAR")) == 0)
                dataFormat = ByteData;
            else if (strncmp(line, "DOUBLE", strlen("DOUBLE")) == 0)
                dataFormat = DoubleData;
            else if (strncmp(line, "INT", strlen("INT")) == 0)
                dataFormat = IntegerData;
            else
                debug1 << "Unknown keyword for BOV byte data: " 
                       << line << endl;
        }
        else if (strcmp(line, "DATA_COMPONENTS:") == 0)
        {
            line += strlen("DATA_COMPONENTS:") + 1;
            if (strncmp(line, "COMPLEX", strlen("COMPLEX")) == 0)
                dataNumComponents = 2;
            else
                dataNumComponents = atoi(line);
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
            byteToFloatTransform = true;
        }
        else if (strcmp(line, "VARIABLE_PALETTE_MAX:") == 0)
        {
            line += strlen("VARIABLE_PALETTE_MAX:") + 1;
            max = atof(line);
            byteToFloatTransform = true;
        }
        else if (strcmp(line, "VARIABLE_MIN:") == 0)
        {
            line += strlen("VARIABLE_MIN:") + 1;
            min = atof(line);
            byteToFloatTransform = true;
        }
        else if (strcmp(line, "VARIABLE_MAX:") == 0)
        {
            line += strlen("VARIABLE_MAX:") + 1;
            max = atof(line);
            byteToFloatTransform = true;
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
        else if (strcmp(line, "BYTE_OFFSET:") == 0)
        {
            line += strlen("BYTE_OFFSET:") + 1;
            byteOffset = atoi(line);
            byteOffset = byteOffset < 0 ? 0 : byteOffset;
        }
        else if (strcmp(line, "DIVIDE_BRICK:") == 0)
        {
            line += strlen("DIVIDE_BRICK:") + 1;
            divideBrick = (strcmp(line, "true") == 0);
        }
    }

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
    if (dimensions[0] <= 0. || dimensions[1] <= 0. || dimensions[2] < 0.)
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
    if(divideBrick &&
       (full_size[0] == bricklet_size[0] &&
        full_size[1] == bricklet_size[1] &&
        full_size[2] == bricklet_size[2]))
    {
        debug1 << "Turning off DIVIDE_BRICK because DATA_SIZE and "
                  "DATA_BRICKLETS are the same size" << endl;
        divideBrick = false;
    }
    if(dataNumComponents < 1)
    {
        debug1 << "An invalid number of components was specified." << endl;
        EXCEPTION1(InvalidFilesException, fname);
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


