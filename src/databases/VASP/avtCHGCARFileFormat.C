/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                            avtCHGCARFileFormat.C                           //
// ************************************************************************* //

#include <avtCHGCARFileFormat.h>

#include <float.h>
#include <string>

#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMTSDFileFormatInterface.h>

#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <sstream>

using     std::string;

#define  ALLOW_TRANSFORMED_RECTILINEAR_GRIDS
#define  VALUES_PER_LINE 10

// ****************************************************************************
//  Method: avtCHGCAR constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file; now MTSD.
//
// ****************************************************************************

avtCHGCARFileFormat::avtCHGCARFileFormat(const char *fn)
    : avtMTSDFileFormat(&fn, 1)
{
    filename = fn;
    OpenFileAtBeginning();

    metadata_read = false;
    values_read = -1;
    values = NULL;

    ntimesteps = 0;
    natoms = 0;

    origdims[0] = 0;
    origdims[1] = 0;
    origdims[2] = 0;

    unitCell[0][0] = 1;    unitCell[0][1] = 0;    unitCell[0][2] = 0;
    unitCell[1][0] = 0;    unitCell[1][1] = 1;    unitCell[1][2] = 0;
    unitCell[2][0] = 0;    unitCell[2][1] = 0;    unitCell[2][2] = 1;
}


// ****************************************************************************
//  Method: avtCHGCARFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Tue Feb 27 11:10:46 EST 2007
//    Don't delete "values" if it's a NULL pointer.
//
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file.
//    values_read is now an int refering to the timestep whose values
//    have been read (or -1 if none).
//
// ****************************************************************************

void
avtCHGCARFileFormat::FreeUpResources(void)
{
    if (values)
        values->Delete();
    values = NULL;
    values_read = -1;
}


// ****************************************************************************
//  Method: avtCHGCARFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:35:04 EST 2007
//    I just added a bunch of infrastructure to visit to support
//    rectilinear grids in transformed space.  Make use of it
//    here (though this can be disabled by removing the #define at the
//    top of the file for now).
//
//    Jeremy Meredith, Fri Apr 20 14:59:53 EDT 2007
//    Added a special case where axis-aligned unit cell vectors
//    construct a *true* rectilinear grid, not a transformed one.
//
// ****************************************************************************

void
avtCHGCARFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int i, j, k;
    ReadAllMetaData();

    avtMeshMetaData *mmd = new avtMeshMetaData("mesh", 1, 0,0,0,
                                               3, 3,
                                               AVT_RECTILINEAR_MESH);

    for (i=0; i<9; i++)
    {
       mmd->unitCellVectors[i] = unitCell[i/3][i%3];
    }

#ifdef ALLOW_TRANSFORMED_RECTILINEAR_GRIDS
    if (!is_rectilinear)
    {
        for (i=0; i<4; i++)
        {
            for (j=0; j<4; j++)
            {
                if (i<3 && j<3)
                    mmd->rectilinearGridTransform[i*4+j] = unitCell[j][i];
                else if (i==j)
                    mmd->rectilinearGridTransform[i*4+j] = 1.0;
                else
                    mmd->rectilinearGridTransform[i*4+j] = 0.0;
            }
        }
        mmd->rectilinearGridHasTransform = true;
    }

    double extents[6] = { DBL_MAX,-DBL_MAX,DBL_MAX,-DBL_MAX,DBL_MAX,-DBL_MAX };
    for (i=0; i<=1; i++)
    {
        for (j=0; j<=1; j++)
        {
            for (k=0; k<=1; k++)
            {
                for (int axis=0; axis<3; axis++)
                {
                    double v = (mmd->unitCellVectors[3*0 + axis] * float(i) +
                                mmd->unitCellVectors[3*1 + axis] * float(j) +
                                mmd->unitCellVectors[3*2 + axis] * float(k));
                    if (extents[2*axis] > v)
                        extents[2*axis] = v;
                    if (extents[2*axis+1] < v)
                        extents[2*axis+1] = v;
                }
            }
        }
    }
    mmd->SetExtents(extents);
#endif

    md->Add(mmd);

    AddScalarVarToMetaData(md, "charge", "mesh", AVT_NODECENT);
}


// ****************************************************************************
//  Method: avtCHGCARFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:35:04 EST 2007
//    I just added a bunch of infrastructure to visit to support
//    rectilinear grids in transformed space.  Make use of it
//    here (when it's not disabled) by returning a unit-size rectilinear
//    grid (i.e. fractional lattice coordinates) instead of cartesian
//    coordinates in a curvilinear grid.
//
//    Jeremy Meredith, Fri Apr 20 14:59:53 EDT 2007
//    Added a special case where axis-aligned unit cell vectors
//    construct a *true* rectilinear grid, not a transformed one.
//
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file; now MTSD.
//
// ****************************************************************************

vtkDataSet *
avtCHGCARFileFormat::GetMesh(int ts, const char *meshname)
{
#ifdef ALLOW_TRANSFORMED_RECTILINEAR_GRIDS
    if (strcmp(meshname, "mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    vtkRectilinearGrid   *rgrid   = vtkRectilinearGrid::New(); 

    vtkFloatArray   *coords[3];
    for (int i = 0 ; i < 3 ; i++)
    {
        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(meshdims[i]);

        for (int j = 0 ; j < meshdims[i] ; j++)
        {
            if (is_rectilinear)
            {
                coords[i]->SetComponent(j, 0,
                       unitCell[i][i] * float(j) / float(origdims[i]) );
            }
            else
            {
                coords[i]->SetComponent(j, 0, float(j) / float(origdims[i]) );
            }
        }
    }
    rgrid->SetDimensions(meshdims);
    rgrid->SetXCoordinates(coords[0]);
    rgrid->SetYCoordinates(coords[1]);
    rgrid->SetZCoordinates(coords[2]);
    coords[0]->Delete();
    coords[1]->Delete();
    coords[2]->Delete();

    return rgrid;
#else    
    vtkStructuredGrid    *sgrid   = vtkStructuredGrid::New(); 
    vtkPoints            *points  = vtkPoints::New();
    sgrid->SetPoints(points);
    points->Delete();

    //
    // Tell the grid what its dimensions are and populate the points array.
    //
    sgrid->SetDimensions(meshdims);

    //
    // Populate the coordinates.
    //
    int nvals = meshdims[0]*meshdims[1]*meshdims[2];
    points->SetNumberOfPoints(nvals);

    double dx[3], dy[3], dz[3];
    for (int d=0; d<3; d++)
    {
        dx[d] = unitCell[d][0] / origdims[d];
        dy[d] = unitCell[d][1] / origdims[d];
        dz[d] = unitCell[d][2] / origdims[d];
    }

    int ctr = 0;
    for (int k=0; k<meshdims[2]; k++)
    {
        for (int j=0; j<meshdims[1]; j++)
        {
            for (int i=0; i<meshdims[0]; i++)
            {
                double x,y,z;
                x = i*dx[0] + j*dx[1] + k*dx[2];
                y = i*dy[0] + j*dy[1] + k*dy[2];
                z = i*dz[0] + j*dz[1] + k*dz[2];
                points->SetPoint(ctr, x,y,z);
                ctr++;
            }
        }
    }
    return sgrid;
#endif
}


// ****************************************************************************
//  Method: avtCHGCARFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:37:37 EST 2007
//    Fixed a crash occurring because we returned a vtkDataArray without
//    adding a new reference to it -- it was deleted elsewhere in visit
//    when we didn't know about it.
//
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file; now MTSD.
//
// ****************************************************************************

vtkDataArray *
avtCHGCARFileFormat::GetVar(int ts, const char *varname)
{
    ReadValues(ts);

    if (string(varname) == "charge")
    {
        values->Register(NULL);
        return values;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtCHGCARFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file; now MTSD.
//
// ****************************************************************************

vtkDataArray *
avtCHGCARFileFormat::GetVectorVar(int ts, const char *varname)
{
    return NULL;
}


// ****************************************************************************
//  Method:  avtCHGCARFileFormat::ReadValues
//
//  Purpose:
//    Actually read the values from the file
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file.
//    values_read is now an int refering to the timestep whose values
//    have been read (or -1 if none).  Use saved file positions.
//
// ****************************************************************************
void
avtCHGCARFileFormat::ReadValues(int timestep)
{
    if (values_read == timestep)
        return;

    if (values)
    {
        values->Delete();
        values = NULL;
    }

    OpenFileAtBeginning();
    in.seekg(file_positions[timestep]);

    values_read = timestep;

    int nvalues = meshdims[0]*meshdims[1]*meshdims[2];

    values = vtkDoubleArray::New();
    values->SetNumberOfTuples(nvalues);
    double *ptr = (double *) values->GetVoidPointer(0);

    int ctr = 0;
    for (int k=0; k<meshdims[2]; k++)
    {
        for (int j=0; j<meshdims[1]; j++)
        {
            for (int i=0; i<meshdims[0]; i++)
            {
                if (i<origdims[0] &&
                    j<origdims[1] &&
                    k<origdims[2])
                {
                    in >> ptr[ctr++];
                }
                else
                {
                    int ii = (i==origdims[0]) ? 0 : i;
                    int jj = (j==origdims[1]) ? 0 : j;
                    int kk = (k==origdims[2]) ? 0 : k;
                    int index = ii + meshdims[0]*(jj + meshdims[1]*kk);
                    ptr[ctr++] = ptr[index];
                }
            }
        }
    }
}

// ****************************************************************************
//  Method:  avtCHGCARFileFormat::ReadAllMetaData
//
//  Purpose:
//    Scan the file, quickly looking for metadata.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Fri Apr 20 14:59:53 EDT 2007
//    Added a special case where axis-aligned unit cell vectors
//    construct a *true* rectilinear grid, not a transformed one.
//
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file.
//    Save file positions for time steps, assuming 10 values per line
//    and an overall file structure matching the examples I've seen so far.
//
// ****************************************************************************
void
avtCHGCARFileFormat::ReadAllMetaData()
{
    if (metadata_read)
        return;

    metadata_read = true;

    char line[132];
    in.getline(line, 132);

    double scale;
    double lat[3][3];
    in >> scale;
    in >> lat[0][0] >> lat[0][1] >> lat[0][2];
    in >> lat[1][0] >> lat[1][1] >> lat[1][2];
    in >> lat[2][0] >> lat[2][1] >> lat[2][2];

    if (lat[0][1]==0 &&
        lat[0][2]==0 &&
        lat[1][0]==0 &&
        lat[1][2]==0 &&
        lat[2][0]==0 &&
        lat[2][1]==0)
    {
        is_rectilinear = true;
    }
    else
    {
        is_rectilinear = false;
    }

    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            unitCell[i][j] = scale*lat[i][j];

    in.getline(line, 132); // skip rest of last lattice line
    in.getline(line, 132); // get atom counts
    string atomcountline(line);

    natoms = 0;
    int tmp;
    std::istringstream count_in(atomcountline);
    while (count_in >> tmp)
    {
        natoms += tmp;
    }

    in.getline(line,132); // skip next line

    for (int i=0; i<natoms; i++)
    {
        in.getline(line,132); // skip atom lines
    }
    in.getline(line,132); // skip blank line

    in >> origdims[0] >> origdims[1] >> origdims[2];
    meshdims[0] = origdims[0]+1;
    meshdims[1] = origdims[1]+1;
    meshdims[2] = origdims[2]+1;

    in.getline(line,132); // skip rest of dims line

    // Mark the start of the volumetric grid data
    int values_per_vol = origdims[0]*origdims[1]*origdims[2];
    int lines_per_vol = (values_per_vol+VALUES_PER_LINE-1)/VALUES_PER_LINE;
    int lines_per_step = 7 + natoms + 2 + lines_per_vol;
    while (in)
    {
        file_positions.push_back(in.tellg());
        ntimesteps++;
        for (int i=0; i<lines_per_step; i++)
            in.getline(line,132);
    }
}

// ****************************************************************************
//  Method:  avtCHGCARFileFormat::Identify
//
//  Purpose:
//    Return true if the file given is a CHGCAR VASP file.
//    So far, only check based on the filename.
//
//  Arguments:
//    filename   the filename
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
// ****************************************************************************
bool
avtCHGCARFileFormat::Identify(const std::string &filename)
{
    // strip off all leading paths
    int pos = filename.length()-1;
    while (pos>=0 && filename[pos]!='/' && filename[pos]!='\\')
        pos--;

    std::string fn;
    if (pos >= 0)
        fn = filename.substr(pos+1);
    else
        fn = filename;

    // uppercase it
    for (int i=0; i<fn.size(); i++)
    {
        if (fn[i]>='a' && fn[i]<='z')
            fn[i] = fn[i] + ('A'-'a');
    }

    for (int i=0; i<=fn.length()-3; i++)
    {
        if (fn.substr(i,3) == "CHG")
            return true;
    }

    return false;
}

// ****************************************************************************
//  Method:  avtCHGCARFileFormat::CreateInterface
//
//  Purpose:
//    Create a file format interface from this reader.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Jan  2 14:11:38 EST 2008
//    Now MTSD.
//
// ****************************************************************************
avtFileFormatInterface *
avtCHGCARFileFormat::CreateInterface(const char *const *list,
                                     int nList, int nBlock)
{
    avtMTSDFileFormat **ffl = new avtMTSDFileFormat*[nList];
    for (int i = 0 ; i < nList ; i++)
    {
        ffl[i] = new avtCHGCARFileFormat(list[i]);
    }
    return new avtMTSDFileFormatInterface(ffl, nList);
}


// ****************************************************************************
//  Method:  avtCHGCARFileFormat::OpenFileAtBeginning
//
//  Purpose:
//    Opens the file, or else seeks to the beginning.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  2, 2008
//
// ****************************************************************************
void
avtCHGCARFileFormat::OpenFileAtBeginning()
{
    if (!in.is_open())
    {
        in.open(filename.c_str());
        if (!in)
        {
            EXCEPTION1(InvalidFilesException, filename.c_str());
        }
    }
    else
    {
        in.clear();
        in.seekg(0, ios::beg);
    }
}


// ****************************************************************************
//  Method:  avtCHGCARFileFormat::GetNTimesteps
//
//  Purpose:
//    return the number of timesteps
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  2, 2008
//
//  Modifications:
//
// ****************************************************************************
int
avtCHGCARFileFormat::GetNTimesteps(void)
{
    ReadAllMetaData();
    return ntimesteps;
}
