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
//                                Resample.C                                 //
// ************************************************************************* //

#include <float.h>
#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#ifdef PARALLEL
    #include <mpi.h>
#endif

#include "Resample.h"


// 
// Declaration of static consts.  They are not allowed as initializers in
// class definitions.
//

const int          Resample::VARS_N = 100;


// ****************************************************************************
//  Method: Resample constructor 
//
//  Programmer: Hank Childs
//  Creation:   December 21, 1999
//
// ****************************************************************************

Resample::Resample()
{
    int   i;

    optList = NULL;
    outMesh = NULL;
    outVars = new DBquadvar *[VARS_N];
    for (i = 0 ; i < VARS_N ; i++)
    {
        outVars[i] = NULL;
    }
    
    um           = NULL;
    qm           = NULL;
    gotExtents   = false;
    siloLocation = NULL;
    for (i = 0 ; i < 3 ; i++)
    {
        resolution[i] = 0;
    }

    badInput = false;
}


// ****************************************************************************
//  Method: Resample destructor
//
//  Programmer: Hank Childs
//  Creation:   December 21, 1999
//
// ****************************************************************************

Resample::~Resample()
{
    if (outMesh != NULL)
    {
        DBFreeQuadmesh(outMesh);
    }
    if (outVars != NULL)
    {
        for (int i = 0 ; i < VARS_N ; i++)
        {
            if (outVars[i] != NULL)
            {
                DBFreeQuadvar(outVars[i]);
            }
        }
        delete [] outVars;
    }

    if (optList != NULL)
    {
        DBFreeOptlist(optList);
    }

    //
    // These are referenced by other objects and should not be freed here.
    //
    //if (0)
    //{
    //  DBFreeQuadmesh(qm);
    //  DBFreeUcdmesh(um);
    //  delete [] siloLocation;
    //}
}


// ****************************************************************************
//  Method: Resample::BadInput
//
//  Purpose:
//      Allows for more graceful handling of bad input.  Mark this object as
//      invalid, issue an error message, but only if this object hasn't already
//      issued an error message.
//
//  Arguments:
//      err     The error message.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2000
//
// ****************************************************************************

void
Resample::BadInput(char *err)
{
    if (! badInput)
    {
        //
        // This is the first bad input that this object has received, so print
        // out an error.
        //
        cerr << err << endl;
    }

    badInput = true;
}


// ****************************************************************************
//  Method: Resample::AddExtents
//
//  Purpose:
//      Goes through an extents argument to find the true extents of the
//      (probably multimesh) mesh.
//
//  Arguments:
//      extents    Three extents (x,y,z).  This could be either the maximum
//                 or the minimum.  
//
//  Programmer: Hank Childs
//  Creation:   December 21, 1999
//
// ****************************************************************************

void
Resample::AddExtents(float *extents)
{
    if (gotExtents == false)
    {
        for (int i = 0 ; i < 3 ; i++)
        {
            minExtents[i] = extents[i];
            maxExtents[i] = extents[i];
        }
        gotExtents = true;
        return;
    }

    for (int i = 0 ; i < 3 ; i++)
    {
        if (extents[i] < minExtents[i])
        {
            minExtents[i] = extents[i];
        }
        if (extents[i] > maxExtents[i])
        {
            maxExtents[i] = extents[i];
        }
    }
}


// ****************************************************************************
//  Method: Resample::ConsolidateExtents
//
//  Purpose:
//      Different processors have read in different domains from the same mesh.
//      They all need to know the true extents of the mesh, so they get that
//      here through MPI calls.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
// ****************************************************************************

void
Resample::ConsolidateExtents(void)
{
    if (badInput)
    {
        return;
    }
#ifdef PARALLEL
    float   extents[3];
    int     i;

    //
    // Reduce the minimum extents.
    //
    MPI_Allreduce(minExtents, extents, 3, MPI_FLOAT, MPI_MIN, 
                  MPI_COMM_WORLD);
    for (i = 0 ; i < 3 ; i++)
    { 
        minExtents[i] = extents[i];
    }
 
    //
    // Reduce the maximum extents.
    //
    MPI_Allreduce(maxExtents, extents, 3, MPI_FLOAT, MPI_MAX, 
                  MPI_COMM_WORLD);
    for (i = 0 ; i < 3 ; i++)
    { 
        maxExtents[i] = extents[i];
    }
#endif
}


// ****************************************************************************
//  Method: Resample::AddMesh (DBucdmesh *)
//
//  Purpose:
//      Gets a mesh for a specific domain so that it can process vars later.
//
//  Arguments:
//      u    One block of the input ucdmesh.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
//  Modifications:
//
//    Hank Childs, Mon Jun 26 08:53:48 PDT 2000
//    Added check to make sure cells were hexahedral.
//
// ****************************************************************************

void
Resample::AddMesh(DBucdmesh *u)
{
    if (badInput)
    {
        return;
    }

    um = u;
    qm = NULL;
   
    if (u->ndims != 3)
    {
        BadInput("The ucdmehs must be three dimensional to be resampled.");
        return;
    }

    for (int i = 0 ; i < u->zones->nshapes ; i++)
    {
        if (u->zones->shapesize[i] != 8)
        {
            BadInput("Only hexahedral zones are supported.");
            return;
        }
    }

    if (outMesh == NULL)
    {
        ConstructQuadMesh(um);
    }
}


// ****************************************************************************
//  Method: Resample::AddMesh (DBquadmesh *)
//
//  Purpose:
//      Gets a mesh for a specific domain so that it can process vars later.
//
//  Arguments:
//      q    One block of the input (curvilinear) quadmesh.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
// ****************************************************************************

void
Resample::AddMesh(DBquadmesh *q)
{
    if (badInput)
    {
        return;
    }

    qm = q;
    um = NULL;

    if (q->ndims != 3)
    {
        BadInput("The quadmesh must be three dimensional to be resmapled.");
        return;
    }
    if (q->coordtype != DB_NONCOLLINEAR)
    {
        BadInput("The quadmesh must be non-collinear to be resampled.");
        return;
    }

    if (outMesh == NULL)
    {
        ConstructQuadMesh(qm);
    }
}


// ****************************************************************************
//  Method: Resample::AddVar (DBquadvar)
//
//  Purpose:
//      Resamples this var.
//
//  Arguments:
//      qv      A quadvar from the input.
//      index   The index of that quadvar for the resampling module.
//
//  Programmer: Hank Childs
//  Creation:   January 3, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Jun 21 09:58:41 PDT 2000
//    Changed routine to interpolating to the nodes in the output mesh in the
//    cell instead of rasterizing it.
//
// ****************************************************************************

void
Resample::AddVar(DBquadvar *qv, int index)
{
    if (badInput)
    {
        return;
    }

    if (qm == NULL)
    {
        cerr << "Cannot add a variable with no mesh." << endl;
        return;
    }

    if (outVars[index] == NULL)
    {
        ConstructQuadVar(qv, index);
    }

    //
    // Go through each zone and rasterize.
    //
    Cell     cell;
    for (int i = 0 ; i < qm->dims[0] - 1 ; i++)
    {
        for (int j = 0 ; j < qm->dims[1] - 1 ; j++)
        {
            for (int k = 0 ; k < qm->dims[2] - 1 ; k++)
            {
                //
                // Create a cell by traversing the node list.
                // 
                ConstructCell(cell, i, j, k);

                //
                // Set the cells value whether the input is node-centered or 
                // zone-centered.
                //
                SetCellValue(cell, qv, i, j, k);

                InterpolateCell(cell, index);

            }   // end for over dims[z]
        }       // end for over dims[y]
    }           // end for over dims[x]
}


// ****************************************************************************
//  Method: Resample::AddVar (DBucdvar)
//
//  Purpose:
//      Resamples this var.
// 
//  Arguments:
//      uv      A ucdvar from the input.
//      index   The index of that ucdvar for the resampling module.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
//  Modifications:
//
//    Hank Childs, Wed Jun 21 09:58:41 PDT 2000
//    Changed routine to interpolating to the nodes in the output mesh in the
//    cell instead of rasterizing it.
//
// ****************************************************************************

void
Resample::AddVar(DBucdvar *uv, int index)
{
    if (badInput)
    {
        return;
    }

    if (um == NULL)
    {
        cerr << "Cannot add a variable with no mesh." << endl;
        return;
    }
 
    if (outVars[index] == NULL)
    {
        ConstructQuadVar(uv, index);
    }

    //
    // Go through each zone and rasterize.
    //
    Cell   cell;
    for (int i = 0 ; i < um->zones->nzones ; i++)
    {
        //
        // Create a cell by traversing the node list.
        // 
        ConstructCell(cell, i);

        //
        // Set the cells value whether the input is node-centered or 
        // zone-centered.
        //
        SetCellValue(cell, uv, i);

        InterpolateCell(cell, index);
    }
}


// ****************************************************************************
//  Method: Resample::InterpolateCell
//
//  Purpose:
//      Determines which nodes in the output mesh are in this cell and 
//      linearly interpolates the variable to them.
//
//  Arguments:
//      cell    A hexahedral cell from the input mesh.
//      index   The index of the variable in the outVars.
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2000
//
// ****************************************************************************

void
Resample::InterpolateCell(Cell &cell, int index)
{
    float  bounds[6];
    cell.GetBounds(bounds);

    int    minmax[3][2];
    for (int i = 0 ; i < 3 ; i++)
    {
        DetermineMinMaxIndex(minmax[i], minExtents[i], maxExtents[i], 
                             bounds[2*i], bounds[2*i + 1], resolution[i]);
    }

    for (int xI = minmax[0][0] ; xI <= minmax[0][1] ; xI++)
    {
        for (int yI = minmax[1][0] ; yI <= minmax[1][1] ; yI++)
        {
            for (int zI = minmax[2][0] ; zI <= minmax[2][1] ; zI++)
            {
                float node[3];
                GetNode(node, xI, yI, zI);
                if (cell.IsInCell(node))
                {
                    float  val[VAR_DIM_MAX];
                    cell.LinearInterpolate(node, val);
                    for (int m = 0 ; m < cell.GetNDims() ; m++)
                    {
                        int vI = zI*resolution[1]*resolution[0] 
                                 + yI*resolution[0] + xI;
                        outVars[index]->vals[m][vI] = val[m];
                    }
                }
            }
        }
    }
}


// ****************************************************************************
//  Method: Resample::SetCellValue (DBucdvar)
//
//  Purpose:
//      Puts in the proper value for each node of the cell, whether its
//      zone-centered or node-centered.
//
//  Arguments:
//      cell   A cell object that should be populated with the variable.
//      uv     The ucdvar from the input.
//      node   The node number in the ucdvar that corresponds to cell.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
//  Modifications:
//
//    Hank Childs, Mon Jun 26 09:03:28 PDT 2000
//    Adapted for new Cell class.
//
// ****************************************************************************

void
Resample::SetCellValue(Cell &cell, DBucdvar *uv, int node)
{
    cell.SetNDims(uv->nvals);
    if (uv->centering == DB_NODECENT)
    {
        for (int k = 0 ; k < 8 ; k++)
        {
            int n = um->zones->nodelist[8*node + k];
            float  vector[VAR_DIM_MAX];
            for (int m = 0 ; m < uv->nvals ; m++)
            {
                vector[m] = uv->vals[m][n];
            }
            cell.SetNodeValue(k, vector);
        }
    }
    else if (uv->centering == DB_ZONECENT)
    {
        float  vector[VAR_DIM_MAX];
        for (int m = 0 ; m < uv->nvals ; m++)
        {
            vector[m] = uv->vals[m][node];
        }
        cell.SetZoneValue(vector);
    }
    else
    {
        BadInput( "Unrecognized centering value.");
        return;
    }
}


// ****************************************************************************
//  Method: Resample::SetCellValue (DBquadvar)
//
//  Purpose:
//      Puts in the proper value for each node of the cell, whether its
//      zone-centered or node-centered.
//
//  Arguments:
//      cell   A cell object that should be populated with the variable.
//      qv     The quadvar from the input.
//      x      The index of the x-coordinate in the quadvar for cell.
//      y      The index of the y-coordinate in the quadvar for cell.
//      z      The index of the z-coordinate in the quadvar for cell.
//
//  Programmer: Hank Childs
//  Creation:   January 3, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jun 26 09:03:28 PDT 2000
//    Adapted for new Cell class.
//
// ****************************************************************************

void
Resample::SetCellValue(Cell &cell, DBquadvar *qv, int x, int y, int z)
{
    cell.SetNDims(qv->nvals);
    if (qv->align[0] == 0.)
    {
        //
        // Easiest just to hard code the gray codes used for populating a cell.
        //
        int   grayCode[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };  
        for (int ii = 0 ; ii < 8 ; ii++)
        {
            int i      = grayCode[ii];
            int localX = x + (i % 2);
            int localY = y + ((i/2) % 2);
            int localZ = z + ((i/4) % 2);
            int node = localX + localY*qv->dims[0] 
                              + localZ*qv->dims[0]*qv->dims[1] ;
            float  vector[VAR_DIM_MAX];
            for (int m = 0 ; m < qv->nvals ; m++)
            {
                vector[m] = qv->vals[m][node];
            }
            cell.SetNodeValue(ii, vector);
        }
    }
    else 
    {
        int node = x + y*(qv->dims[0]-1) + z*(qv->dims[0]-1)*(qv->dims[1]-1);
        float vector[VAR_DIM_MAX];
        for (int m = 0 ; m < qv->nvals ; m++)
        {
            vector[m] = qv->vals[m][node];
        }
        cell.SetZoneValue(vector);
    }
}


// ****************************************************************************
//  Method: Resample::ConstructQuadVar (DBucdvar)
//
//  Purpose:
//      Creates a DBquadvar from the DBucdvar.
//
//  Arguments:
//      uv      The ucdvar from the input whose information will be used to
//              construct the output quadvar.
//      index   The index of uv.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
// ****************************************************************************

void
Resample::ConstructQuadVar(DBucdvar *uv, int index)
{ 
    ConstructQuadVar(uv->nvals, uv->name, index);
}


// ****************************************************************************
//  Method: Resample::ConstructQuadVar (DBquadvar)
//
//  Purpose:
//      Creates a DBquadvar from the DBquadvar to be sampled.
//
//  Arguments:
//      qv      The quadvar from the input whose information will be used to
//              construct the output quadvar.
//      index   The index of uv.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
// ****************************************************************************

void
Resample::ConstructQuadVar(DBquadvar *qv, int index)
{
    ConstructQuadVar(qv->nvals, qv->name, index);
}


// ****************************************************************************
//  Method: ConstructQuadVar
//
//  Purpose:
//      Creates a DBquadvar.
//
//  Arguments:
//      var_dims     The number of dimensions of the variable. 
//      name         The name of the variable.
//      index        The index into the outVars data member that this variable
//                   corresponds to.
//
//  Note:       All memory allocation for the quad vars must be done using
//              C constructs (malloc or calloc) since the DBFreeQuadVar uses a 
//              free on them and purify complains about a free memory mismatch.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
//  Modifications:
//
//    Hank Childs, Mon Jun 26 09:03:28 PDT 2000
//    Allowed resolution to vary in each dimension.
//
// ****************************************************************************

void
Resample::ConstructQuadVar(int var_dims, char *name, int index)
{
    int   i;

    //
    // Create the quadvar.
    //
    outVars[index] = DBAllocQuadvar();
    outVars[index]->name = (char *) calloc( strlen(name)+1, sizeof(char) );
    strcpy(outVars[index]->name, name);

    //
    // Determine the total number of values.  This should be resolution^3.
    //
    int   totalVals = 1;
    for (i = 0 ; i < 3 ; i++)
    {
        outVars[index]->dims[i] = resolution[i];
        totalVals *= resolution[i];
    }
    outVars[index]->nels  = totalVals;
    outVars[index]->ndims = 3;

    //
    // Allocate memory for each value and initialize to 0.
    //
    outVars[index]->vals = (float **) calloc( var_dims+1, sizeof(float *) );
    for (i = 0 ; i < var_dims ; i++)
    {
        outVars[index]->vals[i] = (float *) calloc( totalVals, sizeof(float) );
        for (int j = 0 ; j < totalVals ; j++)
        {
            outVars[index]->vals[i][j] = 0.;
        }
    }
    outVars[index]->vals[var_dims] = NULL;
    outVars[index]->nvals = var_dims;
}


// ****************************************************************************
//  Method: Resample::ConstructQuadMesh (DBucdmesh)
//
//  Purpose:
//      Creates the quadmesh we are going to resample on.
//
//  Arguments:
//      um      The ucdmesh from the input whose information will be used to
//              construct the output quadmesh.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
// ****************************************************************************

void
Resample::ConstructQuadMesh(DBucdmesh *um)
{
    ConstructQuadMesh(um->name, um->ndims, um->cycle, um->time, um->labels, 
                      um->units, um->coord_sys, um->origin);
}


// ****************************************************************************
//  Method: Resample::ConstructQuadMesh (DBquadmesh)
//
//  Purpose:
//      Creates the quadmesh we are going to resample on.
//
//  Arguments:
//      qm      The quadmesh from the input whose information will be used to
//              construct the output quadmesh.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
// ****************************************************************************

void
Resample::ConstructQuadMesh(DBquadmesh *qm)
{
    ConstructQuadMesh(qm->name, qm->ndims, qm->cycle, qm->time, qm->labels, 
                      qm->units, qm->coord_sys, qm->origin);
}


// ****************************************************************************
//  Method: Resample::ConstructQuadMesh
//
//  Purpose:
//      Creates the quadmesh we are going to resample on.
//
//  Arguments:
//      name       The name of the mesh.
//      ndims      The dimensionality of the mesh (should be three).
//      cycle      The cycle of the mesh.
//      time       The time of the mesh.
//      labels     The labels in each dimension for the mesh.
//      units      The units in each dimension for the mesh.
//      coord_sys  The coord_sys for the mesh.
//      origin     The origin of the mesh.
//
//  Note:       All memory that is allocated for the quad mesh should be 
//              allocated with the malloc command (not "new") because free is
//              used on it in the DBFreeQuadmesh command and purify gives a
//              FMM (free memory mismatch) error.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
//  Modifications:
//
//    Hank Childs, Mon Jun 26 09:03:28 PDT 2000
//    Allowed resolution to vary in each dimension.
//
// ****************************************************************************

void
Resample::ConstructQuadMesh(char *name, int ndims, int cycle, float time,
                            char *labels[3], char *units[3], int coord_sys,
                            int origin)
{
    //
    // Allocate memory for the mesh.
    //
    outMesh = DBAllocQuadmesh();

    //
    // Populate the fields of the quadmesh.  Copies must be made so that they 
    // can be fed into an optlist.  Control of when the original input mesh 
    // is deleted is not part of this object, so we cannot reference its
    // data members.
    //
    outMesh->cycle     = cycle;
    outMesh->time      = time;
    outMesh->coord_sys = coord_sys;
    outMesh->origin    = origin;
    if (labels != NULL)
    {
        for (int i = 0 ; i < 3 ; i++)
        {
            if (labels[i] == NULL)
            {
                outMesh->labels[i] = NULL;
            }
            else
            {
                outMesh->labels[i] = 
                        (char *) calloc( strlen(labels[i])+1, sizeof(char) );
                strcpy(outMesh->labels[i], labels[i]);
            }
        }
    }
    else
    {
        outMesh->labels[0] = outMesh->labels[1] = outMesh->labels[2] = NULL;
    }
    if (units != NULL)
    {
        for (int i = 0 ; i < 3 ; i++)
        {
            if (units[i] == NULL)
            {
                outMesh->units[i] = NULL;
            }
            else
            {
                outMesh->units[i] = 
                        (char *) calloc( strlen(units[i])+1, sizeof(char) );
                strcpy(outMesh->units[i], units[i]);
            }
        }
    }
    else
    {
        outMesh->units[0] = outMesh->units[1] = outMesh->units[2] = NULL;
    }

    if (ndims != 3)
    {
        BadInput("Cannot resample a mesh that is not three dimensional.");
        return;
    }
    outMesh->ndims = ndims;

    //
    // Create the new name.
    //
    if (name != NULL)
    {
        outMesh->name = (char *) calloc( strlen(name)+strlen("_resampled")+1,
                                         sizeof(char) );
        sprintf(outMesh->name, "%s_resampled", name);
    }

    // 
    // Create the coordinates.
    //
    for (int i = 0 ; i < 3 ; i++)
    {
        outMesh->dims[i]   = resolution[i];
        outMesh->coords[i] = (float *) calloc( resolution[i], sizeof(float) );
        float step = (maxExtents[i] - minExtents[i]) / (resolution[i]-1);
        outMesh->coords[i][0] = minExtents[i];
        for (int j = 1 ; j < resolution[i] ; j++)
        {
            outMesh->coords[i][j] = outMesh->coords[i][j-1] + step;
        }
    }

    //
    // Construct an opt list.
    //
    if (optList == NULL)
    {
        optList = DBMakeOptlist(15);
        DBAddOption(optList, DBOPT_CYCLE,    &(outMesh->cycle));
        DBAddOption(optList, DBOPT_TIME,     &(outMesh->time));
        DBAddOption(optList, DBOPT_COORDSYS, &(outMesh->coord_sys));
        DBAddOption(optList, DBOPT_ORIGIN,   &(outMesh->origin));
        if (outMesh->labels[0] != NULL)
        {
            DBAddOption(optList, DBOPT_XLABEL, outMesh->labels[0]);
        }
        if (outMesh->labels[1] != NULL)
        {
            DBAddOption(optList, DBOPT_YLABEL, outMesh->labels[1]);
        }
        if (outMesh->labels[2] != NULL)
        {
            DBAddOption(optList, DBOPT_ZLABEL, outMesh->labels[2]);
        }
        if (outMesh->units[0] != NULL)
        {
            DBAddOption(optList, DBOPT_XUNITS, outMesh->units[0]);
        }
        if (outMesh->units[1] != NULL)
        {
            DBAddOption(optList, DBOPT_YUNITS, outMesh->units[1]);
        }
        if (outMesh->units[2] != NULL)
        {
            DBAddOption(optList, DBOPT_ZUNITS, outMesh->units[2]);
        }
    }
}


// ****************************************************************************
//  Method: ConstructCell
//
//  Purpose:
//      Constructs a cell from the unstructured mesh.
//
//  Arguments:
//      cell           A cell object to put the coords in.
//      x              The x-index.
//      y              The y-index.
//      z              The z-index.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
//  Modifications:
//
//    Hank Childs, Mon Jun 26 09:03:28 PDT 2000
//    Removed logic to determine the minimum and maximum Z-extents of the cell
//    since the cell is no longer rasterized.
//
// ****************************************************************************

void
Resample::ConstructCell(Cell &cell, int x, int y, int z)
{
    //
    // Easiest just to hard code the gray codes used for populating a cell.
    //
    int   grayCode[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };  
    for (int ii = 0 ; ii < 8 ; ii++)
    {
        int i      = grayCode[ii];
        int localX = x + (i % 2);
        int localY = y + ((i/2) % 2);
        int localZ = z + ((i/4) % 2);
        int node = localX + localY*qm->dims[0] 
                          + localZ*qm->dims[0]*qm->dims[1] ;
        cell.SetNode(ii, qm->coords[0][node], qm->coords[1][node], 
                     qm->coords[2][node]); 
    }
}


// ****************************************************************************
//  Method: ConstructCell
//
//  Purpose:
//      Constructs a cell from the unstructured mesh.
//
//  Arguments:
//      cell           A cell object to put the coords in.
//      i              The cell index to work on.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
//  Modifications:
//
//    Hank Childs, Mon Jun 26 09:03:28 PDT 2000
//    Removed logic to determine the minimum and maximum Z-extents of the cell
//    since the cell is no longer rasterized.
//
// ****************************************************************************

void
Resample::ConstructCell(Cell &cell, int i)
{
    for (int k = 0 ; k < 8 ; k++)
    {
        int  node = um->zones->nodelist[8*i + k];
        cell.SetNode(k, um->coords[0][node], um->coords[1][node], 
                     um->coords[2][node]);
    }
}


// ****************************************************************************
//  Method: Resample::WrapUp
//
//  Purpose:
//      Wraps up a resample object.  If the object is distributed over several
//      processors, use MPI calls to get all of the information on proc. 0.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
// ****************************************************************************

void
Resample::WrapUp(void)
{
    if (badInput)
    {
        return;
    }

#ifdef PARALLEL
    //
    // A huge amount to be sending over the MPI channels.  
    // Also, assume that only processor 0 needs this info to (hopefully) 
    // reduce some of the contact.
    //
    for (int i = 0 ; i < VARS_N ; i++)
    {
        if (outVars[i] != NULL)
        {
            float   *reduceVals = new float[outVars[i]->nels];
            for (int di = 0 ; di < outVars[i]->nvals ; di++)
            {
                MPI_Reduce(outVars[i]->vals[di], reduceVals, outVars[i]->nels,
                           MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

                //
                // Copy it all into the vals array.  This will invalidate all
                // the arrays but processor 0.
                //
                for (int j = 0 ; j < outVars[i]->nels ; j++)
                {
                    outVars[i]->vals[di][j] = reduceVals[j];
                }
            }
            delete [] reduceVals;
        }
    }
#endif
}


// ****************************************************************************
//  Method: Resample::Write
//
//  Purpose:
//      Writes the quadmesh and quadvar to a SILO file.
//
//  Arguments:
//      dbfile   The file to write to.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
// ****************************************************************************

void
Resample::Write(DBfile *dbfile)
{
    if (badInput)
    {
        return;
    }

#ifdef PARALLEL
    //
    // Only processor 0 has valid data after the MPI_Reduce in WrapUp.
    //
    extern int my_rank;
    if (my_rank != 0)
    {
        return;
    }
#endif

    if (DBSetDir(dbfile, siloLocation) < 0)
    {
        cerr << "Unable to change directories in the silo file to "
             << siloLocation << "." << endl;
        exit(EXIT_FAILURE);
    }

    DBPutQuadmesh(dbfile, outMesh->name, NULL, outMesh->coords, outMesh->dims,
                  outMesh->ndims, DB_FLOAT, DB_COLLINEAR, optList);

    for (int i = 0 ; i < VARS_N ; i++)
    {
        if (outVars[i] != NULL)
        {
            if (outVars[i]->nvals == 1)
            {
                DBPutQuadvar1(dbfile, outVars[i]->name, outMesh->name, 
                              outVars[i]->vals[0], outVars[i]->dims, 
                              outVars[i]->ndims, NULL, 0, DB_FLOAT, 
                              DB_NODECENT, optList);
            }
            else
            {
                //
                // Must have var names for each var, so just put in dummies.
                //
                char *names[VAR_DIM_MAX];
                for (int j = 0 ; j < VAR_DIM_MAX ; j++)
                {
                    names[j] = outVars[i]->name;
                }
                DBPutQuadvar(dbfile, outVars[i]->name, outMesh->name,
                             outVars[i]->nvals, names, outVars[i]->vals, 
                             outVars[i]->dims, outVars[i]->ndims, NULL, 0,
                             DB_FLOAT, DB_NODECENT, optList);
            }
        }
    }
}


// ****************************************************************************
//  Method: Resample::DetermineMinMaxIndex
//
//  Purpose:
//      Determines what the minimum and maximum indices are in the resampled
//      quad mesh for the dimension of the arguments.
//
//  Arguments:
//      minmax     The minimum and maximum indices for this dimension.  The
//                 minimum will be the first index inside the cell and the
//                 maximum will be the last index inside the cell.
//      minMesh    The minimum extent for the mesh in this dimension.
//      maxMesh    The maximum extent for the mesh in this dimension.
//      minCell    The minimum extent for the cell in this dimension.
//      maxCell    The maximum extent for the cell in this dimension.
//      res        The resolution of the mesh in this dimension.
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2000
//      
// ****************************************************************************

void
Resample::DetermineMinMaxIndex(int minmax[2], float minMesh, float maxMesh, 
                               float minCell, float maxCell, int res)
{
    float  epsilon = 0.0001;
    float  step = (maxMesh - minMesh) / (res-1);

    float  minNumSteps = (minCell - minMesh) / step;

    //
    // Unfortunately, the inevitable floating point errors are cropping up 
    // when the cell is at the boundary.
    //
    if (fabs(minNumSteps) < epsilon)
    {
        minNumSteps = 0.;
    }

    minmax[0] = ceil(minNumSteps);

    float  maxNumSteps = (maxMesh - maxCell) / step;

    if (fabs(maxNumSteps) < epsilon)
    {
        maxNumSteps = 0.;
    }

    minmax[1] = (res-1) - ceil(maxNumSteps);
}


// ****************************************************************************
//  Method: Resample::GetNode
//
//  Purpose:
//      Puts the node from the output mesh into the node argument.
//
//  Arguments:
//      node     The node corresponding to (xI, yI, zI).
//      xI       The x-index of the desired node.
//      yI       The y-index of the desired node.
//      zI       The z-index of the desired node.
// 
//  Programmer:  Hank Childs
//  Creation:    June 21, 2000
//
// ****************************************************************************

void
Resample::GetNode(float node[3], int xI, int yI, int zI)
{
    node[0] = outMesh->coords[0][xI];
    node[1] = outMesh->coords[1][yI];
    node[2] = outMesh->coords[2][zI];
}


// ****************************************************************************
//  Method: Resample::SetResolution
//
//  Purpose:
//      Sets the resolution and the output directory.
//
//  Arguments:
//      r    The desired resolution of the output mesh.  This will be used in
//           all three dimensions.
//      l    The silo location where the resampled mesh and variables should
//           be written.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 1999
//
//  Modifications:
//
//    Hank Childs, Mon Jun 26 09:03:28 PDT 2000
//    Changed function to call set resolution function that takes resolution
//    in all three directions.
//
// ****************************************************************************

void
Resample::SetResolution(int r, char *l)
{
    int res[3];
    res[0] = res[1] = res[2] = r;
    SetResolution(res, l);
}


// ****************************************************************************
//  Method: Resample::SetResolution
//
//  Purpose:
//      Sets the resolution and the output directory.
//
//  Arguments:
//      res  The desired resolution of the output mesh in each dimension.
//      l    The silo location where the resampled mesh and variables should
//           be written.
//
//  Programmer: Hank Childs
//  Creation:   June 21, 2000
//
// ****************************************************************************

void
Resample::SetResolution(int res[3], char *l)
{
    for (int i = 0 ; i < 3 ; i++)
    {
        resolution[i] = res[i];
    }
    siloLocation = l;
}


