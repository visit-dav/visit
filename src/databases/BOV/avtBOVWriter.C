/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                               avtBOVWriter.C                              //
// ************************************************************************* //

#include <avtBOVWriter.h>

#include <float.h>
#include <vtk_zlib.h>
#include <vector>

#include <visit-config.h>
#include <visitstream.h>

#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtParallel.h>

#include <InvalidDBTypeException.h>
#include <ImproperUseException.h>

using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtBOVWriter::OpenFile
//
//  Purpose:
//      Does no actual work.  Just records the stem name for the files.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2004
//
//  Modifications:
//    Jeremy Meredith/Hank Childs, Tue Mar 27 17:03:47 EDT 2007
//    Added numblocks to the OpenFile interface.
//
// ****************************************************************************

void
avtBOVWriter::OpenFile(const string &stemname, int nb)
{
    stem = stemname;
    nblocks = nb;
}


// ****************************************************************************
//  Method: avtBOVWriter::WriteHeaders
//
//  Purpose:
//      Writes out the BOV header file.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2004
//
//  Modifications:
//
//    Hank Childs, Wed Dec 22 09:16:53 PST 2004
//    Throw a more informative exception.
//
//    Hank Childs, Wed Mar 28 10:04:53 PDT 2007
//    Use the real number of blocks, not what is in the meta-data.  (There
//    may be SIL selection or there may be a resample operator.
//
//    Dave Pugmire, Thu May  7 12:59:09 EDT 2009
//    Allow multi-block export in parallel.
//
// ****************************************************************************

void
avtBOVWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           vector<string> &scalars, vector<string> &vectors,
                           vector<string> &materials)
{
    if (shouldChangeTotalZones || shouldChangeChunks)
    {
        //We are running from visitconvert, multiblock is ok.
    }
    else if (nblocks != 1)
    {
        // We can only handle single block files from file export.
        EXCEPTION1(InvalidDBTypeException, 
                         "The BOV writer can only handle single block files.");
    }
   
    // 
    // Don't bother writing the header now.  We only support single block
    // datasets and we need to examine that dataset before we can write out
    // the header.  So just punt on writing the header and do it when we get
    // the single block.
    //
}


// ****************************************************************************
//  Function: ResampleGrid
//
//  Purpose:
//      Resamples a rectilinear grid onto another rectilinear grid.  Note that
//      this isn't a generally useful thing to do, but it is good for scaling
//      studies.
//
//  Arguments:
//      rgrid          The rectilinear grid we are sampling *from*.
//      ptr            The data defined on rgrid.
//      samples        The buffer to write the interpolated values into.
//      brick_bounds   The bounds of the brick we are supposed to interpolate
//                     onto.
//      brick_dims     The dimensions of the brick we are supposed to 
//                     interpolate onto.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2004
//
//  Modifications:
//  
//    Dave Pugmire, Mon Jun  2 09:33:26 EDT 2008
//    Handle data with more than one component.
//
//    Hank Childs, Tue May 12 02:21:19 PDT 2009
//    Fix problem with floating point precision inaccuracies leading to
//    access of uninitialized memory.
//
// ****************************************************************************

static void
ResampleGrid(vtkRectilinearGrid *rgrid, float *ptr, float *samples, int numComponents,
             float *brick_bounds, int *brick_dims)
{
    int  i, j, k;

    float x_step = (brick_bounds[1] - brick_bounds[0]) / (brick_dims[0]-1);
    float y_step = (brick_bounds[3] - brick_bounds[2]) / (brick_dims[1]-1);
    float z_step = (brick_bounds[5] - brick_bounds[4]) / (brick_dims[2]-1);

    int grid_dims[3];
    rgrid->GetDimensions(grid_dims);

    float *x_prop = new float[brick_dims[0]];
    int   *x_ind  = new int[brick_dims[0]];
    for (i = 0 ; i < brick_dims[0] ; i++)
    {
        float x = brick_bounds[0] + x_step*i;
        if (i == brick_dims[0]-1)
            x = brick_bounds[1];  // floating-point roundoff screws up <,>
        for (j = 0 ; j < grid_dims[0]-1 ; j++)
        {
            float x1 = rgrid->GetXCoordinates()->GetTuple1(j);
            float x2 = rgrid->GetXCoordinates()->GetTuple1(j+1);
            if ((x1 <= x && x <= x2) || (j == grid_dims[0]-2))
            {
                x_ind[i] = j;
                float dist = x2-x1;
                if (dist == 0)
                    x_prop[i] = 0.;
                else
                {
                    float offset = x-x1;
                    x_prop[i] = offset / dist;
                }
                break;
            }
        }
    }

    float *y_prop = new float[brick_dims[1]];
    int   *y_ind  = new int[brick_dims[1]];
    for (i = 0 ; i < brick_dims[1] ; i++)
    {
        float y = brick_bounds[2] + y_step*i;
        if (i == brick_dims[1]-1)
            y = brick_bounds[3];  // floating-point roundoff screws up <,>
        for (j = 0 ; j < grid_dims[1]-1 ; j++)
        {
            float y1 = rgrid->GetYCoordinates()->GetTuple1(j);
            float y2 = rgrid->GetYCoordinates()->GetTuple1(j+1);
            if ((y1 <= y && y <= y2) || (j == grid_dims[1]-2))
            {
                y_ind[i] = j;
                float dist = y2-y1;
                if (dist == 0)
                    y_prop[i] = 0.;
                else
                {
                    float offset = y-y1;
                    y_prop[i] = offset / dist;
                }
                break;
            }
        }
    }

    float *z_prop = new float[brick_dims[2]];
    int   *z_ind  = new int[brick_dims[2]];
    for (i = 0 ; i < brick_dims[2] ; i++)
    {
        float z = brick_bounds[4] + z_step*i;
        if (i == brick_dims[2]-1)
            z = brick_bounds[5];  // floating-point roundoff screws up <,>
        for (j = 0 ; j < grid_dims[2]-1 ; j++)
        {
            float z1 = rgrid->GetZCoordinates()->GetTuple1(j);
            float z2 = rgrid->GetZCoordinates()->GetTuple1(j+1);
            if ((z1 <= z && z <= z2) || (j == grid_dims[2]-2))
            {
                z_ind[i] = j;
                float dist = z2-z1;
                if (dist == 0)
                    z_prop[i] = 0.;
                else
                {
                    float offset = z-z1;
                    z_prop[i] = offset / dist;
                }
                break;
            }
        }
    }

    for (k = 0 ; k < brick_dims[2] ; k++)
    {
        for (j = 0 ; j < brick_dims[1] ; j++)
        {
            for (i = 0 ; i < brick_dims[0] ; i++)
            {
                for ( int c = 0; c < numComponents; c++)
                {
                    // Tri-linear interpolation.
                    float val = 0.;
                    for (int l = 0 ; l < 8 ; l++)
                    {
                        int i_ = x_ind[i] + (l & 1 ? 1 : 0);
                        int j_ = y_ind[j] + (l & 2 ? 1 : 0);
                        int k_ = z_ind[k] + (l & 4 ? 1 : 0);
                        float x_prop_ = (l & 1 ? x_prop[i] : 1.-x_prop[i]);
                        float y_prop_ = (l & 2 ? y_prop[j] : 1.-y_prop[j]);
                        float z_prop_ = (l & 4 ? z_prop[k] : 1.-z_prop[k]);
                        int pt = k_*grid_dims[1]*grid_dims[0]*numComponents +
                                 j_*grid_dims[0]*numComponents +
                                 i_*numComponents +
                                 c;
                        val += x_prop_*y_prop_*z_prop_*ptr[pt];
                    }
                    int pt = k*brick_dims[1]*brick_dims[0]*numComponents + j*brick_dims[0]*numComponents + i*numComponents +c;
                    samples[pt] = val;
                }
            }
        }
    }

    delete [] x_prop;
    delete [] x_ind;
    delete [] y_prop;
    delete [] y_ind;
    delete [] z_prop;
    delete [] z_ind;
}


// ****************************************************************************
//  Method: avtBOVWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2004
//
//  Modifications:
//
//    Hank Childs, Thu Oct 21 18:10:29 PDT 2004
//    Fix problem with writing out one block while resampling.
//
//    Brad Whitlock, Wed Nov 3 12:13:15 PDT 2004
//    Changed long long coding for Windows.
//
//    Hank Childs, Wed Dec 22 09:16:53 PST 2004
//    Throw a more informative exception.  Also support situations where the
//    number of chunks is specified, but not the number of zones ['5736].
//    Also quote the variable name so that we can maintain multiple word
//    variable names. ['5733]
//
//    Hank Childs, Fri Apr 21 16:53:27 PDT 2006
//    Fix cut-n-paste bug.
//
//    Dave Pugmire, Mon Jun  2 09:33:26 EDT 2008
//    Handle data with more than one component.
//
//    Jeremy Meredith, Thu Aug  7 15:55:00 EDT 2008
//    Use const char* for string literals.
//    Remove unnecessary format modifiers in sprintf that caused warnings.
//
//    Dave Pugmire (on behalf of Hank Childs), Thu May  7 12:59:09 EDT 2009
//    Allow parallel writing of BOV files. Also, change bov file formatting
//    to consider how many chunks are written.
//
//    Hank Childs, Mon May 11 23:21:35 CDT 2009
//    Only have processor 0 write the header file.
//
//    Hank Childs, Thu May 21 18:57:31 PDT 2009
//    Add a cast to make AIX happy.
//
// ****************************************************************************

void
avtBOVWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    int   i;
    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        EXCEPTION1(InvalidDBTypeException, 
                         "The BOV writer can only handle rectilinear grids.");
    }
    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) ds;

    //
    // We are only set up to operate on one nodal array.  Which array we 
    // operate on should be set by the commanding program (ie convert).
    //
    vtkDataArray *arr = rgrid->GetPointData()->GetScalars();
    if (arr == NULL)
    {
        int nPtVars = rgrid->GetPointData()->GetNumberOfArrays();
        for (i = 0 ; nPtVars ; i++)
        {
            vtkDataArray *arr2 = rgrid->GetPointData()->GetArray(i);
            if (arr2->GetNumberOfComponents() == 1)
            {
                arr = arr2;
                break;
            }
            else if (arr2->GetNumberOfComponents() == 3)
            {
                arr = arr2;
                break;
            }
        }

        if (arr == NULL)
            EXCEPTION1(InvalidDBTypeException, 
                       "The BOV writer could not find a variable to write.  "
                       "This may be because it only supports nodal variables "
                       "and you wanted to write a zonal variable.");
    }

    if (arr->GetDataType() != VTK_FLOAT)
        EXCEPTION1(InvalidDBTypeException, 
                       "The BOV writer can only handle floating point data.");
    float *ptr = (float *) arr->GetVoidPointer(0);

    char filename[1024];
    sprintf(filename, "%s.bov", stem.c_str());
    ofstream *ofile = NULL;
    if (PAR_Rank() == 0)
    {
        ofile = new ofstream(filename);
        *ofile << "#BOV version: 1.0" << endl;
        *ofile << "# file written by VisIt conversion program " << endl;
    }

    int dims[3];
    rgrid->GetDimensions(dims);

    int nBricklets = 1;
    int brickletsPerX = 1;
    int brickletsPerY = 1;
    int brickletsPerZ = 1;
    int brickletNI = dims[0];
    int brickletNJ = dims[1];
    int brickletNK = dims[2];
    if (shouldChangeChunks)
    {
        double cubeRoot = pow(double(nTargetChunks), 0.3333);
        int approxCubeRoot = ((int) cubeRoot);
        if (approxCubeRoot*approxCubeRoot*approxCubeRoot != nTargetChunks)
            approxCubeRoot += 1;
        brickletsPerX = approxCubeRoot;
        brickletsPerY = approxCubeRoot;
        brickletsPerZ = approxCubeRoot;
        nBricklets = brickletsPerX*brickletsPerY*brickletsPerZ;
    }
    if (shouldChangeTotalZones || shouldChangeChunks)
    {
        // If we are changing the chunks, but not the zones, we should set the
        // target number of zones to be the current number of zones, so we
        // can then split that number into chunks.
        if (!shouldChangeTotalZones)
            targetTotalZones = dims[0]*dims[1]*dims[2];
        VISIT_LONG_LONG zonesPerBricklet = targetTotalZones / (VISIT_LONG_LONG) nBricklets;
        zonesPerBricklet += 1;
        double cubeRoot = pow( (double) zonesPerBricklet, 0.3333);
        int approxCubeRoot = ((int) cubeRoot) + 1;

        brickletNI = approxCubeRoot;
        brickletNJ = approxCubeRoot;
        brickletNK = approxCubeRoot;
    }

    int numDecimals = 4;
    if (nBricklets > 1)
    {
        numDecimals = (int)log10((double)nBricklets)+1;
        if (numDecimals < 4)
            numDecimals = 4;

        char str[1024];
        sprintf(str, "%s_%%0.%dd.bof.gz", stem.c_str(), numDecimals);
        if (PAR_Rank() == 0)
            *ofile << "DATA_FILE: " << str << endl;
    }
    else
    {
        if (PAR_Rank() == 0)
            *ofile << "DATA_FILE: " << stem.c_str() << endl;
    }

    if (PAR_Rank() == 0)
        *ofile << "DATA SIZE: " << brickletsPerX*brickletNI << " "
               << brickletsPerY*brickletNJ << " " << brickletsPerZ*brickletNK
               << endl;

    if (nBricklets > 1)
        if (PAR_Rank() == 0)
            *ofile << "DATA_BRICKLETS: " << brickletNI << " " << brickletNJ
                   << " " << brickletNK << endl;

    if (PAR_Rank() == 0)
        *ofile << "DATA FORMAT: FLOATS" << endl;

    if (arr->GetNumberOfComponents() > 1)
        if (PAR_Rank() == 0)
            *ofile << "DATA_COMPONENTS: " << arr->GetNumberOfComponents() << endl;
    if (PAR_Rank() == 0)
        *ofile << "VARIABLE: \"" << arr->GetName()  << "\"" << endl;

    int nvals = dims[0]*dims[1]*dims[2]*arr->GetNumberOfComponents();

    if (arr->GetNumberOfComponents() == 1)
    {
        float max = -FLT_MAX;
        float min = +FLT_MAX;
        for (i = 0 ; i < nvals ; i++)
        {
            min = (min < ptr[i] ? min : ptr[i]);
            max = (max > ptr[i] ? max : ptr[i]);
        }
        if (PAR_Rank() == 0)
        {
            *ofile << "VARIABLE PALETTE MIN: " << min << endl;
            *ofile << "VARIABLE PALETTE MAX: " << max << endl;
        }
    }
     
    double bounds[6];
    rgrid->GetBounds(bounds);
 
    if (PAR_Rank() == 0)
    {
        *ofile << "BRICK ORIGIN: " << bounds[0] << " " << bounds[2] << " "
               << bounds[4] << endl;
        *ofile << "BRICK SIZE: " << bounds[1]-bounds[0] << " " 
               << bounds[3]-bounds[2] << " "
               << bounds[5]-bounds[4] << endl;
        *ofile << "BRICK X_AXIS: 1.000 0.000 0.000" << endl;
        *ofile << "BRICK Y_AXIS: 0.000 1.000 0.000" << endl;
        *ofile << "BRICK Z_AXIS: 0.000 0.000 1.000" << endl;
    }

    // 
    // The information below is good to have.  But if we write it out, then
    // we are breaking with the "standard" BOV format.  But if we are writing
    // out multiple bricks, then we are already breaking with the format, so
    // we may as well.
    //
#ifdef WORDS_BIGENDIAN
    const char *endian_str = "BIG";
#else
    const char *endian_str = "LITTLE";
#endif
    if (nBricklets > 1)
    {
        if (PAR_Rank() == 0)
        {
            *ofile << "DATA_ENDIAN: " << endian_str << endl;
            *ofile << "CENTERING: nodal" << endl;
        }
    }

    if (nBricklets == 1 && brickletNI == dims[0] && brickletNJ == dims[1]
        && brickletNK == dims[2])
    {
        //
        // No resampling necessary.  Also, don't gzip, so we can stay 
        // compatible with the "standard" BOV format.
        //
        FILE *file_handle = fopen(stem.c_str(), "w");
        fwrite(ptr, sizeof(float), nvals, file_handle);
        fclose(file_handle);
    }
    else
    {
        int vals_per_bricklet = brickletNI*brickletNJ*brickletNK*arr->GetNumberOfComponents();
        float *samples = new float[vals_per_bricklet];
        for (int i = 0 ; i < brickletsPerX ; i++)
        {
            for (int j = 0 ; j < brickletsPerY ; j++)
            {
                for (int k = 0 ; k < brickletsPerZ ; k++)
                {
                    int brick = k*brickletsPerY*brickletsPerX 
                              + j*brickletsPerX + i;
                    if ((brick % PAR_Size()) != PAR_Rank())
                        continue;
                    float brick_bounds[6];
                    float x_step = (bounds[1] - bounds[0]) / brickletsPerX;
                    brick_bounds[0] = bounds[0] + i*x_step;
                    brick_bounds[1] = bounds[0] + (i+1)*x_step;
                    float y_step = (bounds[3] - bounds[2]) / brickletsPerY;
                    brick_bounds[2] = bounds[2] + j*y_step;
                    brick_bounds[3] = bounds[2] + (j+1)*y_step;
                    float z_step = (bounds[5] - bounds[4]) / brickletsPerZ;
                    brick_bounds[4] = bounds[4] + k*z_step;
                    brick_bounds[5] = bounds[4] + (k+1)*z_step;
                    int brick_dims[3] = { brickletNI, brickletNJ, brickletNK };
                    ResampleGrid(rgrid, ptr, samples, arr->GetNumberOfComponents(),brick_bounds,brick_dims);
                    char str[1024];
                    if (nBricklets > 1)
                    {
                        char fmt[1024];
                        sprintf(fmt, "%s_%%0.%dd.bof.gz", stem.c_str(), numDecimals);
                        sprintf(str, fmt, brick);
                        void *gz_handle = gzopen(str, "w");
                        gzwrite(gz_handle, samples, 
                                vals_per_bricklet*sizeof(float));
                        gzclose(gz_handle);
                    }
                    else
                    {
                        FILE *file_handle = fopen(stem.c_str(), "w");
                        fwrite(samples, sizeof(float), vals_per_bricklet, 
                               file_handle);
                        fclose(file_handle);
                    }
                }
            }
        }
        delete [] samples;
    }
}


// ****************************************************************************
//  Method: avtBOVWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2004
//
// ****************************************************************************

void
avtBOVWriter::CloseFile(void)
{
    // Just needed to meet interface.
}


