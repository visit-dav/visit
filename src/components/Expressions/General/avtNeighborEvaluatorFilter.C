// ************************************************************************* //
//                       avtNeighborEvaluatorFilter.C                        //
// ************************************************************************* //

#include <avtNeighborEvaluatorFilter.h>

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
//  Method: avtNeighborEvaluatorFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

avtNeighborEvaluatorFilter::avtNeighborEvaluatorFilter()
{
    evaluationType = BIGGEST_NEIGHBOR;
    buff = NULL;
    nEntries = NULL;
}


// ****************************************************************************
//  Method: avtNeighborEvaluatorFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

avtNeighborEvaluatorFilter::~avtNeighborEvaluatorFilter()
{
}


// ****************************************************************************
//  Method: avtNeighborEvaluatorFilter::PreExecute
//
//  Purpose:
//      Called before Execute, this will initialize haveIssuedWarning.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

void
avtNeighborEvaluatorFilter::PreExecute(void)
{
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtNeighborEvaluatorFilter::DeriveVariable
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
avtNeighborEvaluatorFilter::DeriveVariable(vtkDataSet *in_ds)
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
//  Method: avtNeighborEvaluatorFilter::InitializeEvaluation
//
//  Purpose:
//     Initializes arrays needed for evaluation.
//     
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

void
avtNeighborEvaluatorFilter::InitializeEvaluation(int nvals, float *ptr)
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
//  Method: avtNeighborEvaluatorFilter::EvaluateNeighbor
//
//  Purpose:
//      Updates our buffer with a neighbor.
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

void
avtNeighborEvaluatorFilter::EvaluateNeighbor(int idx, float val)
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
//  Method: avtNeighborEvaluatorFilter::FinalizeEvaluation
//
//  Purpose:
//     Finalizes arrays needed for evaluation.
//     
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

void
avtNeighborEvaluatorFilter::FinalizeEvaluation(int nvals)
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
//  Method: avtNeighborEvaluatorFilter::PerformRestriction
//
//  Purpose:
//      Declares that we need ghost data.
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2004
//
// ****************************************************************************

avtPipelineSpecification_p
avtNeighborEvaluatorFilter::PerformRestriction(avtPipelineSpecification_p s)
{
    avtPipelineSpecification_p spec = new avtPipelineSpecification(s);

    //
    // We will need the ghost zones so that we can interpolate along domain
    // boundaries and still get the biggest neighbor.
    //
    spec->GetDataSpecification()->SetDesiredGhostDataType(GHOST_ZONE_DATA);

    return spec;
}


