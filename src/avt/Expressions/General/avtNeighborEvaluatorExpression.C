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
//                       avtNeighborEvaluatorExpression.C                        //
// ************************************************************************* //

#include <avtNeighborEvaluatorExpression.h>

#include <math.h>
#include <vector>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtCallback.h>

using std::vector;


// ****************************************************************************
//  Method: avtNeighborEvaluatorExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

avtNeighborEvaluatorExpression::avtNeighborEvaluatorExpression()
{
    evaluationType = BIGGEST_NEIGHBOR;
    buff = NULL;
    nEntries = NULL;
}


// ****************************************************************************
//  Method: avtNeighborEvaluatorExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

avtNeighborEvaluatorExpression::~avtNeighborEvaluatorExpression()
{
}


// ****************************************************************************
//  Method: avtNeighborEvaluatorExpression::PreExecute
//
//  Purpose:
//      Called before Execute, this will initialize haveIssuedWarning.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtNeighborEvaluatorExpression::PreExecute(void)
{
    avtSingleInputExpressionFilter::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtNeighborEvaluatorExpression::DeriveVariable
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
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

vtkDataArray *
avtNeighborEvaluatorExpression::DeriveVariable(vtkDataSet *in_ds)
{
    int  i, j, k;

    int nPoints = in_ds->GetNumberOfPoints();
    int nCells  = in_ds->GetNumberOfCells();
    bool nodal = true;
    vtkDataArray *input = in_ds->GetPointData()->GetArray(activeVariable); 
    if (input == NULL)
    {
        input = in_ds->GetCellData()->GetArray(activeVariable); 
        nodal = false;
    }

    vtkDataArray *results = vtkFloatArray::New();
    results->SetNumberOfComponents(1);
    int nvals = (nodal ? nPoints : nCells);
    results->SetNumberOfTuples(nvals);        
    float *r_ptr = (float *) results->GetVoidPointer(0);

    if (input == NULL || input->GetNumberOfComponents() != 1)
    {
        // Don't return uninitialized memory.
        for (i = 0 ; i < nvals ; i++)
            r_ptr[i] = 0.;
         
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The biggest neighbor/smallest neighbor/"
                                      "average neighbor expression only "
                                      "operates on scalar quantities.");
            haveIssuedWarning = true;
        }

        return results;
    }

    int dtype = in_ds->GetDataObjectType();
    bool isStructured = false;
    int dims[3];
    if (dtype == VTK_RECTILINEAR_GRID || dtype == VTK_STRUCTURED_GRID)
    {
        isStructured = true;
        if (dtype == VTK_RECTILINEAR_GRID)
            ((vtkRectilinearGrid *) in_ds)->GetDimensions(dims);
        else if (dtype == VTK_STRUCTURED_GRID)
            ((vtkStructuredGrid *) in_ds)->GetDimensions(dims);
    }
  
    InitializeEvaluation(nvals, r_ptr);

    float *ptr = (float *) input->GetVoidPointer(0);
    if (nodal)
    {
        if (isStructured)
        {
            int i_step = 1;
            int j_step = dims[0];
            int k_step = dims[1]*dims[0];
            for (i = 0 ; i < dims[0] ; i++)
                for (j = 0 ; j < dims[1] ; j++)
                    for (k = 0 ; k < dims[2] ; k++)
                    {
                        int index = k*k_step + j*j_step + i;
                        if ((i-1) > 0)
                            EvaluateNeighbor(index, ptr[index-i_step]);
                        if ((i+1) < dims[0])
                            EvaluateNeighbor(index, ptr[index+i_step]);
                        if ((j-1) > 0)
                            EvaluateNeighbor(index, ptr[index-j_step]);
                        if ((j+1) < dims[1])
                            EvaluateNeighbor(index, ptr[index+j_step]);
                        if ((k-1) > 0)
                            EvaluateNeighbor(index, ptr[index-k_step]);
                        if ((k+1) < dims[2])
                            EvaluateNeighbor(index, ptr[index+k_step]);
                    }
        }
        else // unstructured or polydata
        {
            //
            // Note: this technique will count some edges multiple times,
            // which can skew averages.
            //
            for (i = 0 ; i < nCells ; i++)
            {
                vtkCell *cell = in_ds->GetCell(i);
                int nEdges = cell->GetNumberOfEdges();
                for (j = 0 ; j < nEdges ; j++)
                {
                    vtkCell *edge = cell->GetEdge(j);
                    int id1 = edge->GetPointId(0);
                    int id2 = edge->GetPointId(1);
                    EvaluateNeighbor(id1, ptr[id2]);
                    EvaluateNeighbor(id2, ptr[id1]);
                }
            }
        }
    }
    else // zonal
    {
        if (isStructured)
        {
            int rangeX = (dims[0] <= 1 ? 1 : dims[0]-1);
            int rangeY = (dims[1] <= 1 ? 1 : dims[1]-1);
            int rangeZ = (dims[2] <= 1 ? 1 : dims[2]-1);
            int i_step = 1;
            int j_step = rangeX;
            int k_step = rangeX*rangeY;
            for (i = 0 ; i < rangeX ; i++)
                for (j = 0 ; j < rangeY; j++)
                    for (k = 0 ; k < rangeZ ; k++)
                    {
                        int index = k*k_step + j*j_step + i;
                        if ((i-1) > 0)
                            EvaluateNeighbor(index, ptr[index-i_step]);
                        if ((i+1) < dims[0]-1)
                            EvaluateNeighbor(index, ptr[index+i_step]);
                        if ((j-1) > 0)
                            EvaluateNeighbor(index, ptr[index-j_step]);
                        if ((j+1) < dims[1]-1)
                            EvaluateNeighbor(index, ptr[index+j_step]);
                        if ((k-1) > 0)
                            EvaluateNeighbor(index, ptr[index-k_step]);
                        if ((k+1) < dims[2]-1)
                            EvaluateNeighbor(index, ptr[index+k_step]);
                    }
        }
        else // unstructured or polydata
        {
            //
            // Note: this technique will count some zones multiple times,
            // which can skew averages.
            //
            vtkIdList *cell_ids = vtkIdList::New();
            for (i = 0 ; i < nCells ; i++)
            {
                vtkCell *cell = in_ds->GetCell(i);
                vtkIdList *pt_ids = cell->GetPointIds();
                int npts = pt_ids->GetNumberOfIds();
                for (j = 0 ; j < npts ; j++)
                {
                    in_ds->GetPointCells(pt_ids->GetId(j), cell_ids);
                    int ncells = cell_ids->GetNumberOfIds();
                    for (k = 0 ; k < ncells ; k++)
                    {
                        int cell = cell_ids->GetId(k);
                        if (cell != i)
                            EvaluateNeighbor(i, ptr[cell]);
                    }
                }
            }
            cell_ids->Delete();
        }
    }

    FinalizeEvaluation(nvals);

    return results;
}


// ****************************************************************************
//  Method: avtNeighborEvaluatorExpression::InitializeEvaluation
//
//  Purpose:
//     Initializes arrays needed for evaluation.
//     
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

void
avtNeighborEvaluatorExpression::InitializeEvaluation(int nvals, float *ptr)
{
    buff = ptr;
    nEntries = new int[nvals];
    for (int i = 0 ; i < nvals ; i++)
    {
        buff[i] = 0.;
        nEntries[i] = 0;
    }
}


// ****************************************************************************
//  Method: avtNeighborEvaluatorExpression::EvaluateNeighbor
//
//  Purpose:
//      Updates our buffer with a neighbor.
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

void
avtNeighborEvaluatorExpression::EvaluateNeighbor(int idx, float val)
{
    if (evaluationType == BIGGEST_NEIGHBOR)
    {
        if (nEntries[idx] == 0)
        {
            buff[idx] = val;
            nEntries[idx] = 1;
        }
        else
            buff[idx] = (buff[idx] < val ? val : buff[idx]);
    }
    else if (evaluationType == SMALLEST_NEIGHBOR)
    {
        if (nEntries[idx] == 0)
        {
            buff[idx] = val;
            nEntries[idx] = 1;
        }
        else
            buff[idx] = (buff[idx] > val ? val : buff[idx]);
    }
    else if (evaluationType == AVERAGE_NEIGHBOR)
    {
        buff[idx] += val;
        nEntries[idx]++;
    }
}


// ****************************************************************************
//  Method: avtNeighborEvaluatorExpression::FinalizeEvaluation
//
//  Purpose:
//     Finalizes arrays needed for evaluation.
//     
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

void
avtNeighborEvaluatorExpression::FinalizeEvaluation(int nvals)
{
    if (evaluationType == AVERAGE_NEIGHBOR)
    {
        for (int i = 0 ; i < nvals ; i++)
            if (nEntries[i] > 0)
                buff[i] /= nEntries[i];
    }

    delete [] nEntries;
}


// ****************************************************************************
//  Method: avtNeighborEvaluatorExpression::ModifyContract
//
//  Purpose:
//      Declares that we need ghost data.
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

avtContract_p
avtNeighborEvaluatorExpression::ModifyContract(avtContract_p s)
{
    avtContract_p spec = new avtContract(s);

    //
    // We will need the ghost zones so that we can interpolate along domain
    // boundaries and still get the biggest neighbor.
    //
    spec->GetDataRequest()->SetDesiredGhostDataType(GHOST_ZONE_DATA);

    return spec;
}


