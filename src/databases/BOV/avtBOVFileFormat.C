/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
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

#include <vtk_zlib.h>

#include <visitstream.h>
#include <visit-config.h>
#include <snprintf.h>

#include <vtkUnsignedCharArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkShortArray.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtIntervalTree.h>
#include <avtParallel.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtIsenburgSGG.h>
#include <avtTypes.h>
#include <avtVariableCache.h>

#include <Expression.h>

#include <DebugStream.h>
#include <BadDomainException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>


static int FormatLine(char *line);


//  This Boolean should only be set to false when we are using the
//  Isenburg streaming module that fills these gaps itself.
static bool fillSpace = true;


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
//    Hank Childs, Wed Feb 21 11:07:25 PST 2007
//    byteToFloatTransform was initialized twice ... removed one.
//
//    Hank Childs, Thu Apr 24 13:24:51 PDT 2008
//    Change initializations of char *'s that have been converted to strings.
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
    file_pattern = "";
    cycle = 0;
    full_size[0] = 0;
    full_size[1] = 0;
    full_size[2] = 0;
    bricklet_size[0] = 0;
    bricklet_size[1] = 0;
    bricklet_size[2] = 0;
    varname = "var";
    byteToFloatTransform = false;
    min = 0.;
    max = 1.;
    origin[0] = 0.;
    origin[1] = 0.;
    origin[2] = 0.;
    dimensions[0] = 1.;
    dimensions[1] = 1.;
    dimensions[2] = 1.;
    declaredEndianess = false;
    littleEndian = false;
    hasBoundaries = false;
    nodalCentering = true;
    dataFormat = ByteData;
    dataNumComponents = 1;
    byteOffset = 0;
    divideBrick = false;

    haveReadTOC = false;
}


// ****************************************************************************
//  Method: avtBOVFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Apr 24 13:25:15 PDT 2008
//    Remove destruction of members that are now handled through STL.
//
// ****************************************************************************

avtBOVFileFormat::~avtBOVFileFormat()
{
    if (path != NULL)
    {
        delete [] path;
        path = NULL;
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
//    Jeremy Meredith, Thu Jul 24 14:55:41 EDT 2008
//    Change most int's and long's to long longs to support >4GB files.
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
    long long nx = full_size[0] / bricklet_size[0];
    long long ny = full_size[1] / bricklet_size[1];
    long long nz = full_size[2] / bricklet_size[2];

    //
    // Sanity check.
    //
    long long nbricks = nx*ny*nz;
    if (dom < 0 || dom >= nbricks)
    {
        EXCEPTION2(BadDomainException, dom, nbricks);
    }

    long long z_off = dom / (nx*ny);
    long long y_off = (dom % (nx*ny)) / nx;
    long long x_off = dom % nx;

    //
    // Establish what the range is of this dataset.
    //
    float x_step = dimensions[0] / (nx);
    float y_step = dimensions[1] / (ny);
    float z_step = dimensions[2] / (nz);
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
    long long dx = bricklet_size[0];
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

    // Don't fill in gaps
    if (!fillSpace)
        x_stop -= (x_step / dx);

    for (i = 0 ; i < dx ; i++)
        x->SetTuple1(i, x_start + i * (x_stop-x_start) / (dx-1));

    vtkFloatArray *y = vtkFloatArray::New();
    long long dy = bricklet_size[1];
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

    // Don't fill in gaps
    if (!fillSpace)
        y_stop -= (y_step / dy);

    y->SetNumberOfTuples(dy);
    for (i = 0 ; i < dy ; i++)
        y->SetTuple1(i, y_start + i * (y_stop-y_start) / (dy-1));

    vtkFloatArray *z = vtkFloatArray::New();
    long long dz = bricklet_size[2];
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
 
        // Don't fill in gaps
        if (!fillSpace)
            z_stop -= (z_step / dz);

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
//   Jeremy Meredith, Thu Jul 24 14:55:41 EDT 2008
//   Change most int's and long's to long longs to support >4GB files.
//
// ****************************************************************************

template<class T>
void
ReadBricklet(FILE *fp, T *dest, const long long *full_size,
             const long long *start, const long long *end, long long offset,
             long long ncomp)
{
    T *ptr = dest;

    // Seek to the right Z page
    long long zPage = start[2]*full_size[0]*full_size[1];
    long long seekOffset = zPage * sizeof(T) + offset;
    if(seekOffset > 0)
        fseek(fp, seekOffset * ncomp, SEEK_SET);

    // Now start reading the data
    long long dx = end[0] - start[0];
    long long nxelem = dx * ncomp;
    long long extraseek = 0;
    for(long long z = start[2]; z < end[2]; ++z)
    {
         // Get to the starting data location in the zPage.
         long long corner = start[1]*full_size[0] + start[0];
         seekOffset = (corner + extraseek) * sizeof(T);
         if(seekOffset > 0)
             fseek(fp, seekOffset * ncomp, SEEK_CUR);
         extraseek = 0;

         for(long long y = start[1]; y < end[1]; ++y)
         {
             // Read in a line of data in x.
             fread((void *)ptr, sizeof(T), nxelem, fp);
             ptr += nxelem;

             // Seek to the next line
             if(y < end[1]-1)
             {
                 long long right = full_size[0] - end[0];
                 long long left = start[0];
                 seekOffset = (right + left) * sizeof(T);
                 if(seekOffset > 0)
                     fseek(fp, seekOffset * ncomp, SEEK_CUR);
             }
         }

         // Seek to the next page
         if(z < end[2]-1)
         {
             long long right = full_size[0] - end[0];
             long long bottom = (full_size[1] - end[1]) * full_size[0];
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
int16_Reverse_Endian(short val, unsigned char *output)
{
    unsigned char *input  = ((unsigned char *)&val);

    output[0] = input[1];
    output[1] = input[0];

    return 2;
}

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
//    Jeremy Meredith, Thu Jul 24 14:55:41 EDT 2008
//    Change most int's and long's to long longs to support >4GB files.
//   
// ****************************************************************************

template <class T>
inline void
ExtractBrick(T *dest, const T *src, 
    unsigned long long x_start, unsigned long long x_stop, 
    unsigned long long y_start, unsigned long long y_stop, 
    unsigned long long z_start, unsigned long long z_stop, 
    unsigned long long dx, unsigned long long dy, unsigned long long ncomp)
{
    T *buff = dest;

    if(ncomp == 1)
    {
        for (unsigned long long i = z_start ; i < z_stop ; i++)
        {
            unsigned long long Z = i*dx*dy;
            for (unsigned long long j = y_start ; j < y_stop ; j++)
            {
                unsigned long long Y = j * dx;
                for (unsigned long long k = x_start ; k < x_stop ; k++)
                {
                    unsigned long long index = Z + Y + k;
                    *buff++ = src[index];
                }
            }
        }
    }
    else
    {
        unsigned long long dX = x_stop - x_start;
        unsigned long long nvals = dX * ncomp;

        // Extract N components
        for (unsigned long long i = z_start ; i < z_stop ; i++)
        {
            unsigned long long Z = i*dx*dy;
            for (unsigned long long j = y_start ; j < y_stop ; j++)
            {
                unsigned long long Y = j * dx;
                unsigned long long index = (Z + Y + x_start) * ncomp;
                const T *xdata = src + index;
                for (unsigned long long k = 0 ; k < nvals ; k++)
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
//    Hank Childs, Fri Feb 15 16:37:42 PST 2008
//    Throw an exception if assumptions are violated.
//
//    Jeremy Meredith, Thu Jul 24 14:55:41 EDT 2008
//    Change most int's and long's to long longs to support >4GB files.
//
//    Brad Whitlock, Wed Apr  8 09:40:42 PDT 2009
//    Added short int support.
//
// ****************************************************************************

void
avtBOVFileFormat::ReadWholeAndExtractBrick(void *dest, bool gzipped,
    void *file_handle, void *gz_handle,
    unsigned long long unit_size,
    unsigned long long x_start, unsigned long long x_stop, 
    unsigned long long y_start, unsigned long long y_stop, 
    unsigned long long z_start, unsigned long long z_stop, 
    unsigned long long dx, unsigned long long dy,
    unsigned long long whole_size)
{
    const char *mName = "avtBOVFileFormat::ReadWholeAndExtractBrick: ";

    // Allocate enough memory to read the whole brick.
    void *whole_buff = 0;
    unsigned long long whole_nelem = whole_size * dataNumComponents;
    if(dataFormat == ByteData)
        whole_buff = (void *)(new unsigned char[whole_nelem]);
    else if(dataFormat == ShortData)
        whole_buff = (void *)(new short[whole_nelem]);
    else if(dataFormat == IntegerData)
        whole_buff = (void *)(new int[whole_nelem]);
    else if(dataFormat == FloatData)
        whole_buff = (void *)(new float[whole_nelem]);
    else if(dataFormat == DoubleData)
        whole_buff = (void *)(new double[whole_nelem]);
    else
    {
        EXCEPTION0(ImproperUseException);
    }

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
    else if(dataFormat == ShortData)
    {
        short *s_buff = (short *)whole_buff;
        ExtractBrick((short *)dest, s_buff,
                     x_start, x_stop, y_start, y_stop, z_start, z_stop,
                     dx, dy, dataNumComponents);
        // Delete the array containing the whole BOV
        delete [] s_buff;
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
//    Hank Childs, Wed Feb 21 11:07:25 PST 2007
//    Only do a byte-to-float transform if the data we get from the file is 
//    really byte data.
//
//    Hank Childs, Thu Apr 24 13:26:21 PDT 2008
//    Change references from char *'s to strings.
//
//    Jeremy Meredith, Thu Jul 24 14:55:41 EDT 2008
//    Change most int's and long's to long longs to support >4GB files.
//
//    Brad Whitlock, Wed Apr  8 09:42:24 PDT 2009
//    I added short int support.
//
// ****************************************************************************

vtkDataArray *
avtBOVFileFormat::GetVar(int dom, const char *var)
{
    const char *mName = "avtBOVFileFormat::GetVar: ";
    if (varname != var)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    long long nx = full_size[0] / bricklet_size[0];
    long long ny = full_size[1] / bricklet_size[1];
    long long nz = full_size[2] / bricklet_size[2];

    long long x_off = dom % nx;
    long long y_off = (dom % (nx*ny)) / nx;
    long long z_off = dom / (nx*ny);

    long long nbricks = nx*ny*nz;

    if (dom < 0 || dom >= nbricks)
    {
        EXCEPTION2(BadDomainException, dom, nbricks);
    }

    char filename[1024];
    sprintf(filename, file_pattern.c_str(), dom);
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
    long long unit_size;
    vtkDataArray *rv = 0;
    if(dataFormat == ByteData)
    {
        unit_size = sizeof(unsigned char);
        vtkUnsignedCharArray *ca = vtkUnsignedCharArray::New();
        rv = ca;
    }
    else if(dataFormat == ShortData)
    {
        unit_size = sizeof(short);
        vtkShortArray *sa = vtkShortArray::New();
        rv = sa;
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
    long long ncomp = (dataNumComponents==2)?3:dataNumComponents;
    rv->SetNumberOfComponents(ncomp);

    //
    // Read the data using the various methods.
    //
    if(hasBoundaries)
    { 
        long long dx = bricklet_size[0] + 2;
        long long x_start = (x_off == 0 ? 1 : 0);
        long long x_stop  = (x_off >= nx-1 ? dx-1 : dx);
        long long dy = bricklet_size[1] + 2;
        long long y_start = (y_off == 0 ? 1 : 0);
        long long y_stop  = (y_off >= ny-1 ? dy-1 : dy);
        long long dz = bricklet_size[2] + 2;
        long long z_start = (z_off == 0 ? 1 : 0);
        long long z_stop  = (z_off >= nz-1 ? dz-1 : dz);

        long long n_real_vals = (x_stop-x_start)*(y_stop-y_start)*(z_stop-z_start);
        rv->SetNumberOfTuples(n_real_vals);
        long long total_vals = (dx*dy*dz);

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
        unsigned long long nvals = bricklet_size[0] * 
                             bricklet_size[1] *
                             bricklet_size[2];
        rv->SetNumberOfTuples(nvals);

        long long x_start = x_off * bricklet_size[0];
        long long y_start = y_off * bricklet_size[1];
        long long z_start = z_off * bricklet_size[2];
        long long x_stop = x_start + bricklet_size[0];
        long long y_stop = y_start + bricklet_size[1];
        long long z_stop = z_start + bricklet_size[2];

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
            long long start[3] = {x_start, y_start, z_start};
            long long stop[3] = {x_stop, y_stop, z_stop};

            if(dataFormat == ByteData)
            {
                debug4 << mName << "Reading char bricklet" << endl;
                // Read the unsigned char data.
                unsigned char *buff = (unsigned char *) rv->GetVoidPointer(0);
                ReadBricklet(file_handle, buff, full_size, start, stop,
                             byteOffset, dataNumComponents);
            }
            else if(dataFormat == ShortData)
            {
                debug4 << mName << "Reading short bricklet" << endl;
                // Read the short data.
                short *buff = (short *) rv->GetVoidPointer(0);
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
            unsigned long long whole_size = full_size[0] * full_size[1] *full_size[2];
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
        long long nvals = bricklet_size[0] * bricklet_size[1] * bricklet_size[2];
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
    if (byteToFloatTransform && dataFormat == ByteData)
    {
        debug4 << mName << "Transforming byte data to float data" << endl;
        vtkFloatArray *fa = vtkFloatArray::New();
        fa->SetNumberOfComponents(rv->GetNumberOfComponents());
        fa->SetNumberOfTuples(rv->GetNumberOfTuples());
        float *fptr = (float *)fa->GetVoidPointer(0);
        unsigned char *ucptr = (unsigned char *)rv->GetVoidPointer(0);
        unsigned long long nvals = rv->GetNumberOfTuples() * dataNumComponents;
        float d_range = (max - min);
        for(unsigned long long i = 0; i < nvals; ++i, ++ucptr)
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
            long long nvals = rv->GetNumberOfTuples();
            unsigned long long ntotal = nvals * dataNumComponents;

            if (dataFormat == ShortData)
            {
                debug4 << mName << "Reversing endian for shorts" << endl;
                short *buff = (short *) rv->GetVoidPointer(0);
                for (long long i = 0 ; i < ntotal ; i++)
                {
                    int tmp;
                    int16_Reverse_Endian(buff[i], (unsigned char *) &tmp);
                    buff[i] = tmp;
                }
            }
            else if (dataFormat == IntegerData)
            {
                debug4 << mName << "Reversing endian for ints" << endl;
                int *buff = (int *) rv->GetVoidPointer(0);
                for (long long i = 0 ; i < ntotal ; i++)
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
                for (long long i = 0 ; i < ntotal ; i++)
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
                for (long long i = 0 ; i < ntotal ; i++)
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
        else if(dataFormat == ShortData)
        {
            ReArrangeTuple2ToTuple3((short *)rv->GetVoidPointer(0),
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
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
//    Hank Childs, Thu Apr 24 13:26:44 PDT 2008
//    Change references from pointers to STL objects.
//
//    Jeremy Meredith, Thu Jul 24 14:55:41 EDT 2008
//    Change most int's and long's to long longs to support >4GB files.
//
// ****************************************************************************

void *
avtBOVFileFormat::GetAuxiliaryData(const char *var, int domain,
                                   const char *type, void *,
                                   DestructorFunction &df)
{
    void *rv = NULL;

    long long nx = full_size[0] / bricklet_size[0];
    long long ny = full_size[1] / bricklet_size[1];
    long long nz = full_size[2] / bricklet_size[2];
    long long nbricks = nx*ny*nz;

    if (strcmp(type, AUXILIARY_DATA_DATA_EXTENTS) == 0)
    {
        if (strcmp(var, varname.c_str()) != 0)
        {
            return NULL;
        }
        if (var_brick_min.size() > 0 && var_brick_max.size() > 0)
        {
            avtIntervalTree *itree = new avtIntervalTree(nbricks, 1);
            for (long long i = 0 ; i < nbricks ; i++)
            {
                double range[2] = { var_brick_min[i], var_brick_max[i] };
                itree->AddElement(i, range);
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
            long long z_off = i / (nx*ny);
            long long y_off = (i % (nx*ny)) / nx;
            long long x_off = i % nx;

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
            itree->AddElement(i, bounds);
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
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Fixed leaks of Expression objects 
//
//    Hank Childs, Thu Apr 24 13:27:39 PDT 2008
//    Change references from char *'s to STL.  Also add support for
//    the Isenburg streaming ghost module, although it is commented out.
//
//    Jeremy Meredith, Thu Jul 24 14:55:41 EDT 2008
//    Change most int's and long's to long longs to support >4GB files.
//
// ****************************************************************************

void
avtBOVFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ReadTOC();
    long long nx = full_size[0] / bricklet_size[0];
    long long ny = full_size[1] / bricklet_size[1];
    long long nz = full_size[2] / bricklet_size[2];

    long long dim = (full_size[2] == 1 ? 2 : 3);

    long long nbricks = nx*ny*nz;
 
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
        Expression mag;
        mag.SetName("real");
        mag.SetDefinition(varname + "[0]");
        mag.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&mag);
        Expression phase;
        phase.SetName("imaginary");
        phase.SetDefinition(varname + "[1]");
        phase.SetType(Expression::ScalarMeshVar);
        md->AddExpression(&phase);
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
            SNPRINTF(n, sizeof(n), "comp%02d", i);
            amd->compNames.push_back(n);

            SNPRINTF(def, sizeof(def), "array_decompose(%s, %d)", varname.c_str(), i);
            Expression e;
            e.SetName(n);
            e.SetDefinition(def);
            e.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&e);
        }
        md->Add(amd);
    }

    if (!avtDatabase::OnlyServeUpMetaData() && nbricks > 1)
    {
        avtRectilinearDomainBoundaries *rdb = 
                                      new avtRectilinearDomainBoundaries(true);
        rdb->SetNumDomains(nbricks);
        for (long long i = 0 ; i < nbricks ; i++)
        {
            long long nx = full_size[0] / bricklet_size[0];
            long long ny = full_size[1] / bricklet_size[1];
            long long nz = full_size[2] / bricklet_size[2];
            long long z_off = i / (nx*ny);
            long long y_off = (i % (nx*ny)) / nx;
            long long x_off = i % nx;
            int extents[6];
            long long correction = (nodalCentering ? 1 : 0);
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
/*
        avtIsenburgSGG *rdb = new avtIsenburgSGG;
        rdb->SetNumberOfDomains(nbricks);
        for (int i = 0 ; i < nbricks ; i++)
        {
            int sz[3] = { bricklet_size[0], bricklet_size[1], bricklet_size[2] };
            int ori[3];
            int nei[6];
            int nx = full_size[0] / bricklet_size[0];
            int ny = full_size[1] / bricklet_size[1];
            int nz = full_size[2] / bricklet_size[2];
            int z_off = i / (nx*ny);
            int y_off = (i % (nx*ny)) / nx;
            int x_off = i % nx;

            ori[0] = x_off * (bricklet_size[0]);
            ori[1] = y_off * (bricklet_size[1]);
            ori[2] = z_off * (bricklet_size[2]);
            nei[0] = (x_off == 0 ? -1 : i-1);
            nei[1] = (x_off == (nx-1) ? -1 : i+1);
            nei[2] = (y_off == 0 ? -1 : i-nx);
            nei[3] = (y_off == (ny-1) ? -1 : i+nx);
            nei[4] = (z_off == 0 ? -1 : i-nx*ny);
            nei[5] = (z_off == (nz-1) ? -1 : i+nx*ny);
            rdb->SetInfoForDomain(i, ori, sz, nei);
        }
        rdb->FinalizeDomainInformation();

        void_ref_ptr vr = void_ref_ptr(rdb,
                                   avtStreamingGhostGenerator::Destruct);
        cache->CacheVoidRef("any_mesh",
                       AUXILIARY_DATA_STREAMING_GHOST_GENERATION, -1, -1, vr);
 */
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
//    Hank Childs, Thu Apr 24 11:40:13 PDT 2008
//    Only have processor 0 read the .bov file.
//
//    Brad Whitlock, Wed Apr  8 09:45:38 PDT 2009
//    I added short int support.
//
// ****************************************************************************

void
avtBOVFileFormat::ReadTOC(void)
{
    if (haveReadTOC)
        return;
    haveReadTOC = true;

    const char *fname = filenames[0];

    if (PAR_Rank() == 0)
    {
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
                file_pattern = line;
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
                else if (strncmp(line, "SHORT", strlen("SHORT")) == 0)
                    dataFormat = ShortData;
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
                varname = line;
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
                var_brick_min.clear();
                var_brick_min.resize(nbricks);
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
                var_brick_max.clear();
                var_brick_max.resize(nbricks);
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
    }

    if (PAR_Size() > 1)
    {
        // Note that the "Broadcast<Type>" calls serve to both
        // send from processor 0 and receive on other processors
        // simultaneously.
        vector<int> vals(16);
        vals[0]  = cycle;
        vals[1]  = full_size[0];
        vals[2]  = full_size[1];
        vals[3]  = full_size[2];
        vals[4]  = (int) dataFormat;
        vals[5]  = dataNumComponents;
        vals[6]  = bricklet_size[0];
        vals[7]  = bricklet_size[1];
        vals[8]  = bricklet_size[2];
        vals[9]  = (int) hasBoundaries;
        vals[10] = (int) littleEndian;
        vals[11] = (int) declaredEndianess;
        vals[12] = (int) byteToFloatTransform;
        vals[13] = (int) nodalCentering;
        vals[14] = byteOffset;
        vals[15] = (int) divideBrick;
        BroadcastIntVector(vals, PAR_Rank());
        cycle             = vals[0];
        full_size[0]      = vals[1];
        full_size[1]      = vals[2];
        full_size[2]      = vals[3];
        dataFormat        = (DataFormatEnum) vals[4];
        dataNumComponents = vals[5];
        bricklet_size[0]  = vals[6];
        bricklet_size[1]  = vals[7];
        bricklet_size[2]  = vals[8];
        hasBoundaries     = (bool) vals[9];
        littleEndian      = (bool) vals[10];
        declaredEndianess = (bool) vals[11];
        byteToFloatTransform = (bool) vals[12];
        nodalCentering    = (bool) vals[13];
        byteOffset        = vals[14];
        divideBrick       = (bool) vals[15];

        bool hasExtents = false;
        if (var_brick_min.size() > 0)
            hasExtents = true;
        BroadcastBool(hasExtents);
        if (hasExtents)
        {
            BroadcastDoubleVector(var_brick_min, PAR_Rank());
        }

        hasExtents = false;
        if (var_brick_max.size() > 0)
            hasExtents = true;
        BroadcastBool(hasExtents);
        if (hasExtents)
        {
            BroadcastDoubleVector(var_brick_max, PAR_Rank());
        }

        vector<double> valsd(8);
        valsd[0] = min;
        valsd[1] = max;
        valsd[2] = origin[0];
        valsd[3] = origin[1];
        valsd[4] = origin[2];
        valsd[5] = dimensions[0];
        valsd[6] = dimensions[1];
        valsd[7] = dimensions[2];
        BroadcastDoubleVector(valsd, PAR_Rank());
        min           = valsd[0];
        max           = valsd[1];
        origin[0]     = valsd[2];
        origin[1]     = valsd[3];
        origin[2]     = valsd[4];
        dimensions[0] = valsd[5];
        dimensions[1] = valsd[6];
        dimensions[2] = valsd[7];

        BroadcastString(file_pattern, PAR_Rank());
debug1 << "Pattern is " << file_pattern << endl;
        BroadcastString(varname, PAR_Rank());
    }

    if (file_pattern == "")
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


