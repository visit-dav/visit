/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                   avtNeighborEvaluatorExpression.C                        //
// ************************************************************************* //

#include <avtNeighborEvaluatorExpression.h>

#include <math.h>
#include <vector>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtAccessor.h>
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
//  Method: avtNeighborEvaluatorExpression::EvaluateNeighbor
//
//  Purpose:
//      Updates our buffer with a neighbor.
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

template <class Accessor>
void
avtNeighborEvaluatorExpression::EvaluateNeighbor(vtkIdType idx, int *nEntries, 
    double val, vtkDataArray *&resArray)
{
    Accessor buff(resArray);
    if (evaluationType == BIGGEST_NEIGHBOR)
    {
        if (nEntries[idx] == 0)
        {
            buff.SetTuple1(idx, val);
            nEntries[idx] = 1;
        }
        else
        {
            buff.SetTuple1(idx, 
                (buff.GetTuple1(idx) < val ? val : buff.GetTuple1(idx)));
        }
    }
    else if (evaluationType == SMALLEST_NEIGHBOR)
    {
        if (nEntries[idx] == 0)
        {
            buff.SetTuple1(idx, val);
            nEntries[idx] = 1;
        }
        else
            buff.SetTuple1(idx, 
                (buff.GetTuple1(idx) > val ? val : buff.GetTuple1(idx)));
    }
    else if (evaluationType == AVERAGE_NEIGHBOR)
    {
        buff.SetTuple1(idx, buff.GetTuple1(idx) + val);
        nEntries[idx]++;
    }
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


template <class Accessor>
void
avtNeighborEvaluatorExpression::DeriveVariableT(vtkDataSet *in_ds, 
    bool nodal, vtkDataArray *input, vtkDataArray *&results)
{
    vtkIdType nPoints = in_ds->GetNumberOfPoints(); 
    vtkIdType nCells  = in_ds->GetNumberOfCells(); 
    vtkIdType nvals   = (nodal ? nPoints : nCells);

    Accessor ptr(input);
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
  
    // Initialize Evaluation
    Accessor r_ptr(results);
    int *nEntries = new int[nvals];
    for (vtkIdType i = 0 ; i < nvals ; i++)
    {
        r_ptr.SetTuple1(i, 0.);
        nEntries[i] = 0;
    }

    if (nodal)
    {
        if (isStructured)
        {
            int i_step = 1;
            int j_step = dims[0];
            int k_step = dims[1]*dims[0];
            for (int i = 0 ; i < dims[0] ; i++)
                for (int j = 0 ; j < dims[1] ; j++)
                    for (int k = 0 ; k < dims[2] ; k++)
                    {
                        int index = k*k_step + j*j_step + i;
                        if ((i-1) > 0)
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index-i_step), results);
                        if ((i+1) < dims[0])
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index+i_step), results);
                        if ((j-1) > 0)
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index-j_step), results);
                        if ((j+1) < dims[1])
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index+j_step), results);
                        if ((k-1) > 0)
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index-k_step), results);
                        if ((k+1) < dims[2])
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index+k_step), results);
                    }
        }
        else // unstructured or polydata
        {
            //
            // Note: this technique will count some edges multiple times,
            // which can skew averages.
            //
            for (vtkIdType i = 0 ; i < nCells ; i++)
            {
                vtkCell *cell = in_ds->GetCell(i);
                vtkIdType nEdges = cell->GetNumberOfEdges();
                for (vtkIdType j = 0 ; j < nEdges ; j++)
                {
                    vtkCell *edge = cell->GetEdge(j);
                    vtkIdType id1 = edge->GetPointId(0);
                    vtkIdType id2 = edge->GetPointId(1);
                    EvaluateNeighbor<Accessor>(id1, nEntries, 
                        ptr.GetTuple1(id2), results);
                    EvaluateNeighbor<Accessor>(id2, nEntries, 
                        ptr.GetTuple1(id1), results);
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
            for (int i = 0 ; i < rangeX ; i++)
                for (int j = 0 ; j < rangeY; j++)
                    for (int k = 0 ; k < rangeZ ; k++)
                    {
                        int index = k*k_step + j*j_step + i;
                        if ((i-1) > 0)
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index-i_step), results);
                        if ((i+1) < dims[0]-1)
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index+i_step), results);
                        if ((j-1) > 0)
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index-j_step), results);
                        if ((j+1) < dims[1]-1)
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index+j_step), results);
                        if ((k-1) > 0)
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index-k_step), results);
                        if ((k+1) < dims[2]-1)
                            EvaluateNeighbor<Accessor>(index, nEntries, 
                                ptr.GetTuple1(index+k_step), results);
                    }
        }
        else // unstructured or polydata
        {
            //
            // Note: this technique will count some zones multiple times,
            // which can skew averages.
            //
            vtkIdList *cell_ids = vtkIdList::New();
            for (vtkIdType i = 0 ; i < nCells ; i++)
            {
                vtkCell *cell = in_ds->GetCell(i);
                vtkIdList *pt_ids = cell->GetPointIds();
                vtkIdType npts = pt_ids->GetNumberOfIds();
                for (vtkIdType j = 0 ; j < npts ; j++)
                {
                    in_ds->GetPointCells(pt_ids->GetId(j), cell_ids);
                    vtkIdType ncells = cell_ids->GetNumberOfIds();
                    for (vtkIdType k = 0 ; k < ncells ; k++)
                    {
                        vtkIdType cell = cell_ids->GetId(k);
                        if (cell != i)
                            EvaluateNeighbor<Accessor>(i, nEntries, 
                                ptr.GetTuple1(cell), results);
                    }
                }
            }
            cell_ids->Delete();
        }
    }

    // Finalize Evaluation
    if (evaluationType == AVERAGE_NEIGHBOR)
    {
        for (vtkIdType i = 0 ; i < nvals ; i++)
            if (nEntries[i] > 0)
                r_ptr.SetTuple1(i, r_ptr.GetTuple1(i) / nEntries[i]);
    }

    delete [] nEntries;
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
avtNeighborEvaluatorExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkIdType nPoints = in_ds->GetNumberOfPoints();
    vtkIdType nCells  = in_ds->GetNumberOfCells();
    bool nodal = true;
    vtkDataArray *input = in_ds->GetPointData()->GetArray(activeVariable); 
    if (input == NULL)
    {
        input = in_ds->GetCellData()->GetArray(activeVariable); 
        nodal = false;
    }

    vtkDataArray *results = input->NewInstance();
    results->SetNumberOfComponents(1);
    vtkIdType nvals = (nodal ? nPoints : nCells);
    results->SetNumberOfTuples(nvals);        
    if (input == NULL || input->GetNumberOfComponents() != 1)
    {
        // Don't return uninitialized memory.
        for (vtkIdType i = 0 ; i < nvals ; i++)
            results->SetTuple1(i, 0.);
         
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The biggest neighbor/smallest neighbor/"
                                      "average neighbor expression only "
                                      "operates on scalar quantities.");
            haveIssuedWarning = true;
        }

        return results;
    }

    if (input->GetDataType() == VTK_FLOAT)
        DeriveVariableT<avtDirectAccessor<float> >(in_ds, nodal, 
            input, results);
    else if (input->GetDataType() == VTK_DOUBLE)
        DeriveVariableT<avtDirectAccessor<double> >(in_ds, nodal, 
            input, results);
    else 
        DeriveVariableT<avtTupleAccessor>(in_ds, nodal, 
            input, results);

    return results;
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

#if 0

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

template <class Accessor, typename T>
void
avtNeighborEvaluatorExpression::EvaluateNeighbor(vtkIdType idx, int *nEntries, 
    T val, T *buff)
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


template <typename T>
void
avtNeighborEvaluatorExpression::DeriveVariableT(vtkDataSet *in_ds, 
    vtkIdType nvals, T *ptr, T *r_ptr)
{
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
  
    int nEntries = new int[nvals];
    for (vtkIdType i = 0 ; i < nvals ; i++)
    {
        r_ptr[i] = 0.;
        nEntries[i] = 0;
    }

    if (nodal)
    {
        if (isStructured)
        {
            int i_step = 1;
            int j_step = dims[0];
            int k_step = dims[1]*dims[0];
            for (int i = 0 ; i < dims[0] ; i++)
                for (int j = 0 ; j < dims[1] ; j++)
                    for (int k = 0 ; k < dims[2] ; k++)
                    {
                        int index = k*k_step + j*j_step + i;
                        if ((i-1) > 0)
                            EvaluateNeighbor(index, nEntries, ptr[index-i_step], r_ptr);
                        if ((i+1) < dims[0])
                            EvaluateNeighbor(index, nEntries, ptr[index+i_step], r_ptr);
                        if ((j-1) > 0)
                            EvaluateNeighbor(index, nEntries, ptr[index-j_step], r_ptr);
                        if ((j+1) < dims[1])
                            EvaluateNeighbor(index, nEntries, ptr[index+j_step], r_ptr);
                        if ((k-1) > 0)
                            EvaluateNeighbor(index, nEntries, ptr[index-k_step], r_ptr);
                        if ((k+1) < dims[2])
                            EvaluateNeighbor(index, nEntries, ptr[index+k_step], r_ptr);
                    }
        }
        else // unstructured or polydata
        {
            //
            // Note: this technique will count some edges multiple times,
            // which can skew averages.
            //
            for (vtkIdType i = 0 ; i < nCells ; i++)
            {
                vtkCell *cell = in_ds->GetCell(i);
                int nEdges = cell->GetNumberOfEdges();
                for (j = 0 ; j < nEdges ; j++)
                {
                    vtkCell *edge = cell->GetEdge(j);
                    vtkIdType id1 = edge->GetPointId(0);
                    vtkIdType id2 = edge->GetPointId(1);
                    EvaluateNeighbor(id1, nEntries, ptr[id2], r_ptr);
                    EvaluateNeighbor(id2, nEntries, ptr[id1], r_ptr);
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
            for (int i = 0 ; i < rangeX ; i++)
                for (int j = 0 ; j < rangeY; j++)
                    for (int k = 0 ; k < rangeZ ; k++)
                    {
                        int index = k*k_step + j*j_step + i;
                        if ((i-1) > 0)
                            EvaluateNeighbor(index, nEntries, ptr[index-i_step], r_ptr);
                        if ((i+1) < dims[0]-1)
                            EvaluateNeighbor(index, nEntries, ptr[index+i_step], r_ptr);
                        if ((j-1) > 0)
                            EvaluateNeighbor(index, nEntries, ptr[index-j_step], r_ptr);
                        if ((j+1) < dims[1]-1)
                            EvaluateNeighbor(index, nEntries, ptr[index+j_step], r_ptr);
                        if ((k-1) > 0)
                            EvaluateNeighbor(index, nEntries, ptr[index-k_step], r_ptr);
                        if ((k+1) < dims[2]-1)
                            EvaluateNeighbor(index, nEntries, ptr[index+k_step], r_ptr);
                    }
        }
        else // unstructured or polydata
        {
            //
            // Note: this technique will count some zones multiple times,
            // which can skew averages.
            //
            vtkIdList *cell_ids = vtkIdList::New();
            for (vtkIdType i = 0 ; i < nCells ; i++)
            {
                vtkCell *cell = in_ds->GetCell(i);
                vtkIdList *pt_ids = cell->GetPointIds();
                vtkIdType npts = pt_ids->GetNumberOfIds();
                for (vtkIdType j = 0 ; j < npts ; j++)
                {
                    in_ds->GetPointCells(pt_ids->GetId(j), cell_ids);
                    vtkIdType ncells = cell_ids->GetNumberOfIds();
                    for (vtkIdType k = 0 ; k < ncells ; k++)
                    {
                        vtkIdType cell = cell_ids->GetId(k);
                        if (cell != i)
                            EvaluateNeighbor(i, nEntries, ptr[cell], r_ptr);
                    }
                }
            }
            cell_ids->Delete();
        }
    }

    if (evaluationType == AVERAGE_NEIGHBOR)
    {
        for (vtkIdType i = 0 ; i < nvals ; i++)
            if (nEntries[i] > 0)
                buff[i] /= nEntries[i];
    }

    delete [] nEntries;
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
    vtkIdType nPoints = in_ds->GetNumberOfPoints();
    vtkIdType nCells  = in_ds->GetNumberOfCells();
    bool nodal = true;
    vtkDataArray *input = in_ds->GetPointData()->GetArray(activeVariable); 
    if (input == NULL)
    {
        input = in_ds->GetCellData()->GetArray(activeVariable); 
        nodal = false;
    }

    vtkDataArray *results = input->NewInstance();
    results->SetNumberOfComponents(1);
    vtkIdType nvals = (nodal ? nPoints : nCells);
    results->SetNumberOfTuples(nvals);        
    if (input == NULL || input->GetNumberOfComponents() != 1)
    {
        // Don't return uninitialized memory.
        for (vtkIdType i = 0 ; i < nvals ; i++)
            results->SetTuple1(i, 0.);
         
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The biggest neighbor/smallest neighbor/"
                                      "average neighbor expression only "
                                      "operates on scalar quantities.");
            haveIssuedWarning = true;
        }

        return results;
    }

    if (input->GetDataType() == VTK_FLOAT)
        DeriveVariableT<float>(in_ds, nvals, input->GetVoidPointer(), 
                               results->GetVoidPointer() );
    else if (input->GetDataType() == VTK_DOUBLE)
        DeriveVariableT<double>(in_ds, nvals, input->GetVoidPointer(),
                                results->GetVoidPointer() );

    return results;
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


#endif
