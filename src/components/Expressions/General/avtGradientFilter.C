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
//                             avtGradientFilter.C                           //
// ************************************************************************* //

#include <avtGradientFilter.h>

#include <math.h>
#include <vector>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkCellDerivatives.h> 
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPointDataToCellData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtCallback.h>

#include <DebugStream.h>
#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtGradientFilter constructor
//
//  Programmer: Akira Haddox <Transferred by Hank Childs>
//  Creation:   August 18, 2002
//
//  Modifications:
//
//    Hank Childs, Sat Dec 13 10:57:57 PST 2003
//    Added pd2cd.
//
//    Hank Childs, Fri Mar  4 08:21:04 PST 2005
//    Removed centering conversion modules.
//
//    Hank Childs, Mon Feb 13 15:08:41 PST 2006
//    Add support for logical gradients.
//
// ****************************************************************************

avtGradientFilter::avtGradientFilter()
{
    doLogicalGradients = false;
}


// ****************************************************************************
//  Method: avtGradientFilter destructor
//
//  Programmer: Akira Haddox <Transferred by Hank Childs>
//  Creation:   August 18, 2002
//
//  Modifications:
//
//    Hank Childs, Sat Dec 13 10:57:57 PST 2003
//    Added pd2cd.
//
//    Hank Childs, Fri Mar  4 08:21:04 PST 2005
//    Removed centering conversion modules.
//
// ****************************************************************************

avtGradientFilter::~avtGradientFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtGradientFilter::PreExecute
//
//  Purpose:
//      Initializes a flag saying whether or not we've issued a warning.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

void
avtGradientFilter::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtGradientFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Akira Haddox 
//  Creation:     July 30, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Nov 13 13:55:52 PST 2002
//    Fixed memory mismanagement issue that caused a crash on successive calls
//    and fixed memory leak.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    Use NewInstance instead of MakeObject, new vtkapi. 
// 
//    Akira Haddox, Wed Jun 18 13:03:23 PDT 2003
//    Added proper error check for scalar data, and check for 2D data.
// 
//    Hank Childs, Tue Nov 25 17:12:17 PST 2003
//    Do a better job of updating progress.
//
//    Hank Childs, Sat Dec 13 10:46:55 PST 2003
//    Optimize for rectilinear data.  Also allow the cell data to stay as
//    cell data in the output.
//
//    Hank Childs, Fri Mar  4 08:21:04 PST 2005
//    Create centering conversion modules if needed.
//
//    Hank Childs, Fri Mar 11 16:01:21 PST 2005
//    Fix memory leak.
//
//    Hank Childs, Mon Feb 13 15:08:41 PST 2006
//    Add support for logical gradients ['4385].
//
// ****************************************************************************

vtkDataArray *
avtGradientFilter::DeriveVariable(vtkDataSet *in_ds)
{
    if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        return RectilinearGradient((vtkRectilinearGrid *) in_ds);
    }
    if (doLogicalGradients)
    {
        if (in_ds->GetDataObjectType() != VTK_STRUCTURED_GRID)
        {
            if (!haveIssuedWarning)
                avtCallback::IssueWarning("You can only do logical gradients "
                                          "on structured grids.");
            haveIssuedWarning = true;
            int nvals = 0;
            if (in_ds->GetPointData()->GetScalars() != NULL)
                nvals = in_ds->GetNumberOfPoints();
            else
                nvals = in_ds->GetNumberOfCells();
            vtkFloatArray *rv = vtkFloatArray::New();
            rv->SetNumberOfComponents(3);
            rv->SetNumberOfTuples(nvals);
            float vals[3] = { 0., 0., 0. };
            for (int i = 0 ; i < nvals ; i++)
                rv->SetTuple(i, vals);
            return rv;
        }

        return LogicalGradient((vtkStructuredGrid *) in_ds);
    }

    vtkDataArray *scalarValues = in_ds->GetPointData()->GetScalars(); 
    bool recentered = false;
    
    if (!scalarValues)
    {
        if (!(in_ds->GetCellData()->GetScalars()))
        {
            EXCEPTION1(ExpressionException, "the scalar variable could not"
                                            " be found.");
        }

        vtkCellDataToPointData *cd2pd = vtkCellDataToPointData::New();
        cd2pd->SetInput(in_ds);
        cd2pd->Update();

        scalarValues = cd2pd->GetOutput()->GetPointData()->GetScalars();
        scalarValues->Register(NULL); // so we don't lose it
        recentered = true;
        cd2pd->Delete();
    }
    
    int nPoints = in_ds->GetNumberOfPoints();

    vtkDataArray *results = vtkFloatArray::New();
    results->SetNumberOfComponents(3);
    results->SetNumberOfTuples(nPoints);        
    
    for (int nodeId = 0 ; nodeId < nPoints; nodeId++)
    {
        if (nodeId % 10000 == 0)
        {
            int nsteps = (nPoints / 10000) + 1;
            UpdateProgress(currentNode*nsteps + nodeId/10000, 
                           totalNodes*nsteps);
        }

        float xDELTA=1e6, yDELTA=1e6, zDELTA=1e6;
       
        double nodeCoords[3]; 
        in_ds->GetPoint(nodeId, nodeCoords);
        float  nodeValue  = scalarValues->GetComponent(nodeId, 0);
        
        vtkIdList *neighborCellIds = vtkIdList::New();
        vtkIdList *myNodeId = vtkIdList::New();

        myNodeId->SetNumberOfIds(1);
        myNodeId->SetId(0, nodeId);
        
        in_ds->GetCellNeighbors(-1, myNodeId, neighborCellIds);

        myNodeId->Delete();

        int nCells=neighborCellIds->GetNumberOfIds();

        // Find appropriate deltas
        for (int ci = 0 ; ci < nCells ; ci++)
        {
            double *bounds = in_ds->GetCell(neighborCellIds->GetId(ci))
                                                                 ->GetBounds();
            if (bounds[1]-bounds[0] < xDELTA*5)
            {
                xDELTA = (bounds[1]-bounds[0]) / 5.0;
            }
            if (bounds[3]-bounds[2] < yDELTA*5)
            {
                yDELTA = (bounds[3]-bounds[2]) / 5.0;
            }
            if (bounds[5]-bounds[4] < zDELTA*5)
            {
                zDELTA = (bounds[5]-bounds[4]) / 5.0;
            }
        }
        
        float xComponent, yComponent, zComponent;
        
        xComponent=EvaluateComponent(nodeCoords[0],nodeCoords[1],nodeCoords[2],
                xDELTA, 0, 0, nodeValue, in_ds, scalarValues, neighborCellIds);
        yComponent=EvaluateComponent(nodeCoords[0],nodeCoords[1],nodeCoords[2],
                0, yDELTA, 0, nodeValue, in_ds, scalarValues, neighborCellIds);
        if (zDELTA == 0)
            zComponent = 0;
        else
            zComponent=EvaluateComponent(nodeCoords[0],nodeCoords[1],
                                         nodeCoords[2], 0, 0, zDELTA, 
                                         nodeValue, in_ds, scalarValues,
                                         neighborCellIds);

        neighborCellIds->Delete();
        results->SetTuple3(nodeId, xComponent, yComponent, zComponent);
    }
    
    if (recentered)
    {
        vtkDataSet *new_ds = (vtkDataSet*)in_ds->NewInstance();
        new_ds->CopyStructure(in_ds);
        new_ds->GetPointData()->SetVectors(results);

        vtkPointDataToCellData *pd2cd = vtkPointDataToCellData::New();
        pd2cd->SetInput(new_ds);
        pd2cd->Update();
        
        vtkDataArray *new_results = pd2cd->GetOutput()->GetCellData()
                                                                ->GetVectors();
        new_results->Register(NULL); // so we don't lose it

        results->Delete();
        results = new_results;

        new_ds->Delete();
        pd2cd->Delete();
        scalarValues->Delete();
    }

    return results;
}


// ****************************************************************************
//  Method: avtGradientFilter::EvaluateComponent
//
//  Purpose:
//      Calculate the directional derivative from the given point in the given
//      deltas.
//
//  Arguments:
//    x,y,z           The point.
//    dx, dy, dz      The direction, in delta scale: It is indended for (2/3)
//                    of them to be 0.
//    value           The value at the point.
//    in_ds           The dataset.
//    scalarValues    The values for the dataset.
//    neighborCells   The cells to look in.
//
//  Returns:      The value at that point.
//
//  Programmer:   Akira Haddox 
//  Creation:     July 30, 2002
//
// ****************************************************************************


float avtGradientFilter::EvaluateComponent(float x, float y, float z,
            float dx, float dy, float dz, float value, vtkDataSet *in_ds,
            vtkDataArray *scalarValues, vtkIdList *neighborCells)
{
    int deltaMultiplier = 2;
    float upper;
    bool  success = true;

    upper = EvaluateValue(x+dx, y+dy, z+dz, in_ds, scalarValues, neighborCells,
                        success);
    if (!success)
    {
        upper=value;
        --deltaMultiplier;
    }

    float lower;
    success = true;
    lower = EvaluateValue(x-dx, y-dy, z-dz, in_ds, scalarValues, neighborCells,
                          success);

    if (!success)
    {
        lower=value;
        --deltaMultiplier;
    }

    if (!deltaMultiplier)
        return 0;

    return (upper-lower)/ (double(deltaMultiplier)*(dx+dy+dz));
}


// ****************************************************************************
//  Method: avtGradientFilter::EvaluateValue
//
//  Purpose:
//      Calculate the value at a point.
//
//  Arguments:
//    x,y,z           The point.
//    in_ds           The dataset.
//    scalarValues    The values for the dataset.
//    neighborCells   The cells to look in.
//    success         Set to false if there is a problem.
//
//  Returns:      The value at that point.
//
//  Programmer:   Akira Haddox 
//  Creation:     July 30, 2002
//
//  Modifications:
//      Akira Haddox, Mon Jun 23 10:43:12 PDT 2003
//      Changed an accumulator from float to double for precision.
//
// ****************************************************************************


float avtGradientFilter::EvaluateValue(float x, float y, float z, 
                               vtkDataSet *in_ds, vtkDataArray *scalarValues,
                               vtkIdList *neighborCells, bool &success)
{
    // Find which cell contains this point
    double coords[3] = {x,y,z};
    int   junk2;
    double junk3[3];
    double junk4;
    double weights[8];  // This needs to be the max number of points a cell has

    double *abnormalWeights = NULL; // In case of more than 8 points
    
    int cellId;
    vtkCell *c;
    for (cellId = 0 ; cellId < neighborCells->GetNumberOfIds() ; cellId++)
    {
        c = in_ds->GetCell(neighborCells->GetId(cellId));
    
        if (c->GetNumberOfPoints() > 8)
        {
            abnormalWeights = new double[c->GetNumberOfPoints()];
            if (c->EvaluatePosition(coords, NULL, junk2, junk3, junk4, 
                                    abnormalWeights) == 1)
            {
                break;
            }
            
            delete[] abnormalWeights;
            abnormalWeights=NULL;
        }
        
        else if (c->EvaluatePosition(coords, NULL, junk2, junk3, junk4, 
                                     weights) == 1)
        {
            break;
        }
    }

    if (cellId == neighborCells->GetNumberOfIds())
    {
        success = false;
        return 0.;
    }
    
    double value = 0.;
    if (abnormalWeights)
    {
        for (int k=0 ; k < c->GetNumberOfPoints() ; k++)
        {
            int pt = c->GetPointId(k);
            value += abnormalWeights[k] * scalarValues->GetComponent(pt,0);
        }
        delete [] abnormalWeights;
    }
    else
    {
        for (int k = 0 ; k < c->GetNumberOfPoints() ; k++)
        {
            int pt = c->GetPointId(k);
            value += weights[k] * scalarValues->GetComponent(pt, 0);
        }
    }
    return value;    
} 


// ****************************************************************************
//  Method: avtGradientFilter::RectilinearGradient
//
//  Purpose:
//      Determines the gradient of a rectilinear dataset.
//
//  Programmer: Hank Childs
//  Creation:   December 13, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jul  2 15:58:01 PDT 2004
//    Added a check to make sure the scalars existed before proceeding.
//
//    Hank Childs, Mon Feb 13 16:12:23 PST 2006
//    Removed misleading comment.
//
// ****************************************************************************

vtkDataArray *
avtGradientFilter::RectilinearGradient(vtkRectilinearGrid *rg)
{
    int i, j, k;

    vtkDataArray *xc = rg->GetXCoordinates();
    vtkDataArray *yc = rg->GetYCoordinates();
    vtkDataArray *zc = rg->GetZCoordinates();
    float *x = (float *) xc->GetVoidPointer(0);
    float *y = (float *) yc->GetVoidPointer(0);
    float *z = (float *) zc->GetVoidPointer(0);
    bool deleteX = false;
    bool deleteY = false;
    bool deleteZ = false;

    int dims[3];
    rg->GetDimensions(dims);
    bool isNodal = true;
    vtkDataArray *s = rg->GetPointData()->GetScalars();
    if (s == NULL)
    {
         s = rg->GetCellData()->GetScalars();
         if (s == NULL)
         {
             EXCEPTION1(ExpressionException, "the scalar variable could not"
                                             " be found.");
         }

         isNodal = false;
         dims[0] -= 1;
         dims[1] -= 1;
         dims[2] -= 1;

         float *x_new = new float[dims[0]];
         for (i = 0 ; i < dims[0] ; i++)
             x_new[i] = (x[i] + x[i+1]) / 2.;
         x = x_new;

         deleteX = true;
         float *y_new = new float[dims[1]];
         for (i = 0 ; i < dims[1] ; i++)
             y_new[i] = (y[i] + y[i+1]) / 2.;
         y = y_new;
         deleteY = true;

         if (dims[2] > 1)
         {
             float *z_new = new float[dims[2]];
             for (i = 0 ; i < dims[2] ; i++)
                 z_new[i] = (z[i] + z[i+1]) / 2.;
             z = z_new;
             deleteZ = true;
         }
    }

    //
    // Now come up with the divisors.  A gradient is the change in variable
    // value divided by the span (in space).  Division is a killer when it
    // comes to performance.  Since we will be dividing by the same things 
    // repeatedly, calculate them once and then re-use them.
    //
    float *x_div = new float[dims[0]];
    x_div[0] = 1. / (x[1] - x[0]);
    x_div[dims[0]-1] = 1. / (x[dims[0]-1] - x[dims[0]-2]);
    for (i = 1 ; i < dims[0]-1 ; i++)
        x_div[i] = 1. / (x[i+1] - x[i-1]);

    float *y_div = new float[dims[1]];
    y_div[0] = 1. / (y[1] - y[0]);
    y_div[dims[1]-1] = 1. / (y[dims[1]-1] - y[dims[1]-2]);
    for (i = 1 ; i < dims[1]-1 ; i++)
        y_div[i] = 1. / (y[i+1] - y[i-1]);

    float *z_div = NULL;
    if (dims[2] > 1)
    {
        z_div = new float[dims[2]];
        z_div[0] = 1. / (z[1] - z[0]);
        z_div[dims[2]-1] = 1. / (z[dims[2]-1] - z[dims[2]-2]);
        for (i = 1 ; i < dims[2]-1 ; i++)
            z_div[i] = 1. / (z[i+1] - z[i-1]);
    }

    vtkDataArray *out_array = s->NewInstance();
    out_array->SetNumberOfComponents(3);
    out_array->SetNumberOfTuples(s->GetNumberOfTuples());

    float *in  = (float *) s->GetVoidPointer(0);
    float *out = (float *) out_array->GetVoidPointer(0);

    const int dims0 = dims[0];
    const int dims1 = dims[1];
    const int dims2 = dims[2];
    const int iskip = 1;
    const int jskip = dims0;
    const int kskip = dims0*dims1;
    if (dims2 <= 1)
    {
        for (j = 0 ; j < dims1 ; j++)
        {
            for (i = 0 ; i < dims0 ; i++)
            {
                int index     = j*jskip + i*iskip;
                int vec_index = 3*index;

                if ((i > 0) && (i < (dims0-1)))
                    out[vec_index] = in[index+iskip] - in[index-iskip];
                else if (i == 0)
                    out[vec_index] = in[index+iskip] - in[index];
                else // i == dims0-1
                    out[vec_index] = in[index] - in[index-iskip];
                out[vec_index++] *= x_div[i];

                if ((j > 0) && (j < (dims1-1)))
                    out[vec_index] = in[index+jskip] - in[index-jskip];
                else if (j == 0)
                    out[vec_index] = in[index+jskip] - in[index];
                else // j == dims1-1
                    out[vec_index] = in[index] - in[index-jskip];
                out[vec_index++] *= y_div[j];
                out[vec_index++] = 0.;
            }
        }
    }
    else
    {
        for (k = 0 ; k < dims2 ; k++)
        {
            for (j = 0 ; j < dims1 ; j++)
            {
                for (i = 0 ; i < dims0 ; i++)
                {
                    int index     = k*kskip + j*jskip + i*iskip;
                    int vec_index = 3*index;

                    if ((i > 0) && (i < (dims0-1)))
                        out[vec_index] = in[index+iskip]-in[index-iskip];
                    else if (i == 0)
                        out[vec_index] = in[index+iskip] - in[index];
                    else // i == dims0-1
                        out[vec_index] = in[index] - in[index-iskip];
                    out[vec_index++] *= x_div[i];

                    if ((j > 0) && (j < (dims1-1)))
                        out[vec_index] = in[index+jskip] - in[index-jskip];
                    else if (j == 0)
                        out[vec_index] = in[index+jskip] - in[index];
                    else // j == dims1-1
                        out[vec_index] = in[index] - in[index-jskip];
                    out[vec_index++] *= y_div[j];

                    if ((k > 0) && (k < (dims2-1)))
                        out[vec_index] = in[index+kskip] - in[index-kskip];
                    else if (k == 0)
                        out[vec_index] = in[index+kskip] - in[index];
                    else // k == dims2-1
                        out[vec_index] = in[index] - in[index-kskip];
                    out[vec_index++] *= z_div[k];
                }
            }
        }
    }

    if (deleteX)
        delete [] x;
    if (deleteY)
        delete [] y;
    if (deleteZ)
        delete [] z;
    delete [] x_div;
    delete [] y_div;
    if (z_div != NULL)
        delete [] z_div;

    return out_array;
}


// ****************************************************************************
//  Method: avtGradientFilter::LogicalGradient
//
//  Purpose:
//      Determines the logical gradient of a curvilinear dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
//  Modifications:
//
//    Hank Childs / Cyrus Harrison, Wed May 16 11:37:53 PDT 2007
//    Fix memory leak.
//
// ****************************************************************************

vtkDataArray *
avtGradientFilter::LogicalGradient(vtkStructuredGrid *sg)
{
    int i, j, k;

    vtkPoints *vtkpts = sg->GetPoints();
    float *pts = (float *) vtkpts->GetVoidPointer(0);
    bool deletePoints = false;

    int dims[3];
    sg->GetDimensions(dims);
    bool isNodal = true;
    vtkDataArray *s = sg->GetPointData()->GetScalars();
    if (s == NULL)
    {
         s = sg->GetCellData()->GetScalars();
         if (s == NULL)
         {
             EXCEPTION1(ExpressionException, "the scalar variable could not"
                                             " be found.");
         }

         isNodal = false;
         dims[0] -= 1;
         dims[1] -= 1;
         dims[2] -= 1;

         float *pts2 = NULL;
         if (dims[2] > 1)
         {
             pts2 = new float[3*dims[0]*dims[1]*dims[2]];
             for (k = 0 ; k < dims[2] ; k++)
             {
                 for (j = 0 ; j < dims[1] ; j++)
                 {
                     for (i = 0 ; i < dims[0] ; i++)
                     {
                         int c_idx = k*(dims[0])*(dims[1]) + j*dims[0] + i;
                         float *p = pts2 + 3*c_idx;
                         p[0] = 0.;
                         p[1] = 0.;
                         p[2] = 0.;
                         for (int l = 0 ; l < 8 ; l++)
                         {
                             int p_idx = (l&1 ? i+1 : i);
                             p_idx += (l&2 ? j+1 : j)*(dims[0]+1);
                             p_idx += (l&4 ? k+1 : k)*(dims[0]+1)*(dims[1]+1);
                             p[0] += pts[3*p_idx] / 8.;
                             p[1] += pts[3*p_idx+1] / 8.;
                             p[2] += pts[3*p_idx+2] / 8.;
                         }
                     }
                 }
             }
         }
         else
         {
             pts2 = new float[3*dims[0]*dims[1]];
             for (j = 0 ; j < dims[1] ; j++)
             {
                 for (i = 0 ; i < dims[0] ; i++)
                 {
                     int c_idx = j*dims[0] + i;
                     float *p = pts2 + 3*c_idx;
                     p[0] = 0.;
                     p[1] = 0.;
                     p[2] = 0.;
                     for (int l = 0 ; l < 4 ; l++)
                     {
                         int p_idx = (l&1 ? i+1 : i);
                         p_idx += (l&2 ? j+1 : j)*(dims[0]+1);
                         p[0] += pts[3*p_idx] / 4.;
                         p[1] += pts[3*p_idx+1] / 4.;
                     }
                 }
             }
         }

         pts = pts2;
         deletePoints = true;
    }

    vtkDataArray *out_array = s->NewInstance();
    out_array->SetNumberOfComponents(3);
    out_array->SetNumberOfTuples(s->GetNumberOfTuples());

    float *in  = (float *) s->GetVoidPointer(0);
    float *out = (float *) out_array->GetVoidPointer(0);

    const int dims0 = dims[0];
    const int dims1 = dims[1];
    const int dims2 = dims[2];
    const int iskip = 1;
    const int jskip = dims0;
    const int kskip = dims0*dims1;
    if (dims2 <= 1)
    {
        for (j = 0 ; j < dims1 ; j++)
        {
            for (i = 0 ; i < dims0 ; i++)
            {
                int index     = j*jskip + i*iskip;
                int vec_index = 3*index;

                float *pt1 = pts + 3*(index+iskip);
                float *pt2 = pts + 3*(index-iskip);
                if ((i > 0) && (i < (dims0-1)))
                    out[vec_index] = in[index+iskip] - in[index-iskip];
                else if (i == 0)
                {
                    pt2 = pts + 3*index;
                    out[vec_index] = in[index+iskip] - in[index];
                }
                else // i == dims0-1
                {
                    pt1 = pts + 3*index;
                    out[vec_index] = in[index] - in[index-iskip];
                }
                float diff[3];
                diff[0] = pt1[0] - pt2[0];
                diff[1] = pt1[1] - pt2[1];
                diff[2] = pt1[2] - pt2[2];
                float dist = sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
                if (dist == 0.)
                    out[vec_index++] = 0.;
                else
                    out[vec_index++] /= dist;

                pt1 = pts + 3*(index+jskip);
                pt2 = pts + 3*(index-jskip);
                if ((j > 0) && (j < (dims1-1)))
                    out[vec_index] = in[index+jskip] - in[index-jskip];
                else if (j == 0)
                {
                    pt2 = pts + 3*index;
                    out[vec_index] = in[index+jskip] - in[index];
                }
                else // j == dims1-1
                {
                    pt1 = pts + 3*index;
                    out[vec_index] = in[index] - in[index-jskip];
                }
                diff[0] = pt1[0] - pt2[0];
                diff[1] = pt1[1] - pt2[1];
                diff[2] = pt1[2] - pt2[2];
                dist = sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
                if (dist == 0.)
                    out[vec_index++] = 0.;
                else
                    out[vec_index++] /= dist;
                out[vec_index++] = 0.;
            }
        }
    }
    else
    {
        for (k = 0 ; k < dims2 ; k++)
        {
            for (j = 0 ; j < dims1 ; j++)
            {
                for (i = 0 ; i < dims0 ; i++)
                {
                    int index     = k*kskip + j*jskip + i*iskip;
                    int vec_index = 3*index;

                    float *pt1 = pts + 3*(index+iskip);
                    float *pt2 = pts + 3*(index-iskip);
                    if ((i > 0) && (i < (dims0-1)))
                        out[vec_index] = in[index+iskip]-in[index-iskip];
                    else if (i == 0)
                    {
                        pt2 = pts + 3*index;
                        out[vec_index] = in[index+iskip] - in[index];
                    }
                    else // i == dims0-1
                    {
                        pt1 = pts + 3*index;
                        out[vec_index] = in[index] - in[index-iskip];
                    }
                    float diff[3];
                    diff[0] = pt1[0] - pt2[0];
                    diff[1] = pt1[1] - pt2[1];
                    diff[2] = pt1[2] - pt2[2];
                    float dist = sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
                    if (dist == 0.)
                        out[vec_index++] = 0.;
                    else
                        out[vec_index++] /= dist;

                    pt1 = pts + 3*(index+jskip);
                    pt2 = pts + 3*(index-jskip);
                    if ((j > 0) && (j < (dims1-1)))
                        out[vec_index] = in[index+jskip] - in[index-jskip];
                    else if (j == 0)
                    {
                        pt2 = pts + 3*index;
                        out[vec_index] = in[index+jskip] - in[index];
                    }
                    else // j == dims1-1
                    {
                        pt1 = pts + 3*index;
                        out[vec_index] = in[index] - in[index-jskip];
                    }
                    diff[0] = pt1[0] - pt2[0];
                    diff[1] = pt1[1] - pt2[1];
                    diff[2] = pt1[2] - pt2[2];
                    dist = sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
                    if (dist == 0.)
                        out[vec_index++] = 0.;
                    else
                        out[vec_index++] /= dist;

                    pt1 = pts + 3*(index+kskip);
                    pt2 = pts + 3*(index-kskip);
                    if ((k > 0) && (k < (dims2-1)))
                        out[vec_index] = in[index+kskip] - in[index-kskip];
                    else if (k == 0)
                    {
                        pt2 = pts + 3*index;
                        out[vec_index] = in[index+kskip] - in[index];
                    }
                    else // k == dims2-1
                    {
                        pt1 = pts + 3*index;
                        out[vec_index] = in[index] - in[index-kskip];
                    }
                    diff[0] = pt1[0] - pt2[0];
                    diff[1] = pt1[1] - pt2[1];
                    diff[2] = pt1[2] - pt2[2];
                    dist = sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
                    if (dist == 0.)
                        out[vec_index++] = 0.;
                    else
                        out[vec_index++] /= dist;
                }
            }
        }
    }

    if (deletePoints)
        delete [] pts;
    return out_array;
}


// ****************************************************************************
//  Method: avtGradientFilter::PerformRestriction
//
//  Purpose:
//      Request ghost zones.
//
//  Programmer: Hank Childs
//  Creation:   February 13, 2006
//
// ****************************************************************************

avtPipelineSpecification_p
avtGradientFilter::PerformRestriction(avtPipelineSpecification_p in_spec)
{
    avtPipelineSpecification_p spec2 = 
                   avtSingleInputExpressionFilter::PerformRestriction(in_spec);
    spec2->GetDataSpecification()->SetDesiredGhostDataType(GHOST_ZONE_DATA);
    return spec2;
}
 

