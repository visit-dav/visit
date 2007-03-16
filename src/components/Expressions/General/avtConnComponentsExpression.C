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
//                      avtConnComponentsExpression.C                        //
// ************************************************************************* //
#include <avtConnComponentsExpression.h>

#include <math.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtExpressionEvaluatorFilter.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtParallel.h>
#include <avtTerminatingSource.h>

#include <vtkAppendFilter.h>
#include <vtkCharArray.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridRelevantPointsFilter.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkVisItUtility.h>


#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <TimingsManager.h>
#include <Utility.h>


#ifdef PARALLEL
  #include <mpi.h>
#endif

// ****************************************************************************
//  Method: avtConnComponentsExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 22, 2007
//
// ****************************************************************************

avtConnComponentsExpression::avtConnComponentsExpression()
{
    nFinalComps = 0;
}


// ****************************************************************************
//  Method: avtConnComponentsExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 22, 2007
//
// ****************************************************************************

avtConnComponentsExpression::~avtConnComponentsExpression()
{
    ;
}

// ****************************************************************************
//  Method: avtConnComponentsExpression::GetNumberOfComponents
//
//  Purpose:
//      After expression execution returns the final number of components.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 2, 2007
//
// ****************************************************************************
int 
avtConnComponentsExpression::GetNumberOfComponents()
{
    return nFinalComps;
}


// ****************************************************************************
//  Method: avtConnComponentsExpression::Execute
//
//  Purpose:
//      Labels the connected components of an unstructured mesh.
//
//  Programmer: Hank Childs & Cyrus Harrison
//  Creation:   January 22, 2007
//
//  Modifications:
//    Cyrus Harrison, Fri Mar 16 10:46:28 PDT 2007
//    Added progress update. 
//
// ****************************************************************************
void
avtConnComponentsExpression::Execute()
{
    // loop index
    int i, j, k;
    // initialize number of components to zero 
    nFinalComps = 0;

    // get input data tree to obtain datasets
    avtDataTree_p tree = GetInputDataTree();

    
    // holds number of datasets
    int nsets;

    // get datasets
    vtkDataSet **data_sets = tree->GetAllLeaves(nsets);

    // get dataset domain ids
    vector<int> domain_ids;
    tree->GetAllDomainIds(domain_ids);

    // filter out any ghost cells
    vtkDataSetRemoveGhostCells **ghost_filters = 
                          new vtkDataSetRemoveGhostCells*[nsets];
 
    for( i = 0 ; i < nsets ; i++)
    {
        ghost_filters[i] = vtkDataSetRemoveGhostCells::New();
        ghost_filters[i]->SetInput(data_sets[i]);
        ghost_filters[i]->Update();
        vtkDataSet *ds_filtered = ghost_filters[i]->GetOutput();
        ds_filtered->Update();
        data_sets[i] = ds_filtered;
    }

    // array to hold output sets
    avtDataTree_p *leaves = new avtDataTree_p[nsets];

    // vectors to hold result sets and their component labels
    vector<vtkDataSet *>  result_sets;
    vector<vtkIntArray *> result_arrays;
    // vector to hold the number of components per set
    vector<int> results_num_comps;

    result_sets.resize(nsets);
    result_arrays.resize(nsets);
    results_num_comps.resize(nsets);

    int num_local_comps=0;
    int num_comps = 0;

    // set progress related vars
#ifdef PARALLEL
    totalSteps = nsets *4;
#else
    totalSteps = nsets *2;
#endif
    currentProgress = 0;

    // process all local sets
    for(i = 0; i < nsets ; i++)
    {
        // get current set
        vtkDataSet *curr_set = data_sets[i];

        // Make sure the input dataset is an unstructured grid
        if(curr_set->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
        {
            EXCEPTION1(ExpressionException,
          "Connected Components Expression requires unstructured mesh data");
        }


        // perform connected components labeling on current set
        // (this only resolves components within the set)
        vtkIntArray *res_array = SingleSetLabel(curr_set,results_num_comps[i]);

        // if there are multiple sets, we need to shift the component labels of
        // the current set to make sure they are unique
        ShiftLabels(res_array,num_local_comps);

        // update the total number of components found
        num_local_comps+= results_num_comps[i];

        // create a shallow copy of the current data set to add to output
        vtkDataSet *res_set = (vtkDataSet *) curr_set->NewInstance();
        res_set->ShallowCopy(curr_set);


        // add array to dataset
        res_set->GetCellData()->AddArray(res_array);

        // keep pointers to the result set and labels
        result_arrays[i] = res_array;
        result_sets[i]   = res_set;

        // add result as new leaf
        leaves[i] = new avtDataTree(res_set,domain_ids[i]);

        // update progress
        UpdateProgress(currentProgress++,totalSteps);
    }

    // create a boundary set 
    // this is used to for fast boundary queries to resolve components across
    // multiple datasets

    BoundarySet bset;
    for(i=0;i<nsets;i++)
    {
        // add each local mesh to the boundary set
        bset.AddMesh(result_sets[i]);
    }
    // prepare the boundary set for queries
    bset.Finalize();

    // resolve labels across multiple local sets
    num_local_comps = MultiSetResolve(num_local_comps,
                                     bset,
                                     result_sets,
                                     result_arrays);

    // update the total number of found components
    num_comps = num_local_comps;

#ifdef PARALLEL

    //
    // At this point each processor has resolved the labels across all local datasets.
    // Components on each processor are labeled 0 <-> (number of local comps - 1). 
    // In the parallel case we need to ensure that every component has a unique label
    // so we first perform a global label shift.
    //

    // Globally shift labels to make sure they are unique
    int num_global_comps = GlobalLabelShift(num_local_comps,result_arrays);

    // We can now globally resolve the labels

    // Resolve components across processors
    num_global_comps = GlobalResolve(num_global_comps,
                                     bset,
                                     result_sets,
                                     result_arrays);

    // update the total number of found components
    num_comps = num_global_comps;

#endif

    // create output data trees
    if(nsets > 0 )
    {
        // create output data tree
        avtDataTree_p result_tree = new avtDataTree(nsets,leaves);
        // set output data tree
        SetOutputDataTree(result_tree);
    }

    // cleanup leaves array
    delete [] leaves;
    // cleanup data_sets array
    delete [] data_sets;

    // cleanup result sets
    for(i = 0; i< nsets; i++)
    {
       // dec ref pointer for each set
       result_sets[i]->Delete();
       // dec ref pointer for each set's label array
       result_arrays[i]->Delete();
       // cleanup ghost filters
       ghost_filters[i]->Delete();
    }
    // cleanup ghost filters array
    delete [] ghost_filters;

    // Set progress to complete
    UpdateProgress(totalSteps,totalSteps);

    // set the final number of components
    nFinalComps  = num_comps;
}

// ****************************************************************************
//  Method: avtConnComponentsExpression::SingleSetLabel
//
//  Purpose:
//      Performs connected components labeling on a single data set.
//
//  Arguments:
//    data_set     Input mesh
//    num_comps    Resulting number of components (o)
//
//  Returns:       A new vtk integer array containing the label result
//
//  Programmer: Cyrus Harrison
//  Creation:   January 23, 2007
//
// ****************************************************************************
vtkIntArray *
avtConnComponentsExpression::SingleSetLabel(vtkDataSet *data_set,
                                            int &num_comps)
{
    int t0 = visitTimer->StartTimer();

    // loop indices
    int i,j,k;

    // initialize the number of found components to zero 
    num_comps = 0;

    // get number of points
    int npoints = data_set->GetNumberOfPoints();
    // get number of cells
    int ncells = data_set->GetNumberOfCells();

    // component label is stored as cell data in a vtkIntArray
    vtkIntArray *res_array = vtkIntArray::New();
    // set name of new variable
    res_array->SetName(outputVariableName);
    // result only has one component - the label value
    res_array->SetNumberOfComponents(1);
    // cell data has a tuple for each cell
    res_array->SetNumberOfTuples(ncells);

    //
    // Connected Components Labeling on an Unstructured Grid.
    // Our goal is to label connected cells.
    //
    // For the single set we start by considering each point as a separate 
    // component. Components that have points that are in the same cell are 
    // merged. The resolution of the disjoint set problem is handled by the 
    // union find data structure. 
    //
    // In this case points may not be a part of a cell, so their union find 
    // entries may be invalid. If a point is used in an Union operation it 
    // is marked valid. This weeds out points that are not part of any cell.
    // 



    // create a UnionFind object with # of entries  = # of points
    UnionFind union_find(npoints,false);

    // loop over all cells
    for( i = 0; i < ncells; i++)
    {
        // get current cell
        vtkCell *cell = data_set->GetCell(i);

        // get ids of the cell points
        vtkIdList *ids = cell->GetPointIds();
        int nids = ids->GetNumberOfIds();

        // union all point ids from the current cell
        for( j = 0; j < nids; j++)
        {
            for( k=0; k < j; k++)
            {
                // get the point ids
                int j_id = ids->GetId(j);
                int k_id = ids->GetId(k);

                // make sure these ids are not already in the same component
                if( union_find.Find(j_id) != union_find.Find(k_id) )
                {
                    // join the components connected to these two labels
                    union_find.Union(j_id,k_id);
                }
            }
        }
    }
    
    // flatten to resolve final labels
    num_comps = union_find.FinalizeLabels();
    // if we only found a single component, make sure to increment
    // the number of components (CHECK)
    if(ncells > 0 && num_comps == 0 ) num_comps++;
    
    // use union find label to set final output value
    for( i = 0; i < ncells; i++)
    {
        // get current cell
        vtkCell *cell = data_set->GetCell(i);
        // get label by finding the label of the first point in the cell
        // (any point would suffice)
        int label = union_find.GetFinalLabel(cell->GetPointId(0));
        // set value
        res_array->SetValue(i,label);
    }

    visitTimer->StopTimer(t0,"Single Set Connected Components Labeling");

    return res_array;
}


// ****************************************************************************
//  Method: avtConnComponentsExpression::MultiSetResolve
//
//  Purpose:
//      Performs connected components labeling across multiple domains.
//
//  Arguments:
//    num_comps    Total number of components in all input meshes
//    bset         Boundary set used for boundary queries
//    sets         Input meshes
//    labels       Component label arrays for the input meshes
//
//  Returns:       The number of resolved components
//
//  Programmer: Cyrus Harrison
//  Creation:   January 25, 2007
//
//  Modifications:
//    Cyrus Harrison, Fri Mar 16 15:49:42 PDT 2007
//    Added progress update. 
//
// ****************************************************************************
int
avtConnComponentsExpression::MultiSetResolve(int num_comps,
                                            const BoundarySet &bset,
                                            const vector<vtkDataSet*> &sets,
                                            const vector<vtkIntArray*> &labels)
{
    // loop indices
    int i,j,k;

    // find out the number of sets
    int nsets = sets.size();

    // if we have 0 or 1 set(s) multi-set resolve is not necessary
    if(nsets < 2) return num_comps; // not multi-set!

    int t0 = visitTimer->StartTimer();

    // create union find for the multi-set resolve
    // in this case, all union find representatives are valid
    UnionFind union_find(num_comps,true);

    // vectors to hold boundary query results
    vector<int> src_cells;
    vector<int> can_cells;
    int src_label, can_label;

    // loop over all sets
    for( i = 0; i < nsets; i++)
    {
        vtkIntArray  *src_labels   = labels[i];
        // get intersection between set i and j 
        for ( j = 0; j < nsets; j++)
        {
            // self intersection test not necessary
            if ( j == i )
                continue;

            vtkIntArray  *can_labels   = labels[j]; 
            bset.GetIntersectionSet(i,j,src_cells,can_cells);
            
            int nisect = src_cells.size();
            // check if these cell labels need to be merged. 
            for(k = 0; k < nisect; k++)
            {
                // get the source and candidate component labels
                src_label = src_labels->GetValue(src_cells[k]);
                can_label = can_labels->GetValue(can_cells[k]);
                // merge them if the are not already in the same component
                if( union_find.Find(src_label) !=  union_find.Find(can_label))
                {
                    union_find.Union(src_label,can_label);
                }
            }
        }

        // update progress
        UpdateProgress(currentProgress++,totalSteps);
    }

    // flatten the union find object to obtain final labels
    num_comps = union_find.FinalizeLabels();

    // resolve final labels
    for( i = 0; i< nsets;i++)
    {
        // get the current label array
        vtkIntArray *curr_array = labels[i];
        int          ncells     = curr_array->GetNumberOfTuples();

        for(j = 0; j < ncells; j ++)
        {
            // get the current label value
            int lbl = curr_array->GetValue(j);
            // find the final label value
            lbl = union_find.GetFinalLabel(lbl);
            // set the final label 
            curr_array->SetValue(j,lbl);
        }
    }

    visitTimer->StopTimer(t0,"Multi-Set Component Label Resolve");

    // return the final number of components
    return num_comps;
}


// ****************************************************************************
//  Method: avtConnComponentsExpression::MultiSetList
//
//  Purpose:
//      Produces a list of union operations required to resolve the component 
//      labels of the input sets. 
// 
//      This is used by the GlobalResolve function to generate per processor
//      union lists.
//
//  Arguments:
//    num_comps    Current number of components in all input meshes
//    bset         Boundary set used for boundary queries
//    sets         Input meshes
//    labels       Component label arrays for the input meshes
//    u_src        Holds the first component each resulting union entry  (o)
//    u_des        Holds the second component each resulting union entry (o)
//
//  Programmer: Cyrus Harrison
//  Creation:   January 25, 2007
//
// ****************************************************************************
void
avtConnComponentsExpression::MultiSetList(int num_comps,
                                         const BoundarySet &bset,
                                         const vector<vtkDataSet*> &sets,
                                         const vector<vtkIntArray*> &labels,
                                         vector<int> &u_src,
                                         vector<int> &u_des)
{
    // loop indices
    int i,j,k;

    // find out the number of sets
    int nsets = sets.size();
    // clear the result vectors
    u_src.clear();
    u_des.clear();

    // if we have 0 or 1 set(s) multi-set resolve is not necessary
    if(nsets < 2) return; // not multi-set!

    int t0 = visitTimer->StartTimer();

    //
    // The final label values are not resolved here, but a union find instance 
    // provides an efficient way to make sure redundant union operations are 
    // not listed. 
    // 

    // create union find for the multi-set resolve
    // in this case, all union find representatives are valid
    UnionFind union_find(num_comps,true);

    // vectors to hold boundary query results
    vector<int> src_cells;
    vector<int> can_cells;
    int src_label, can_label;

    // loop over all sets
    for( i = 0; i < nsets; i++)
    {
        vtkIntArray  *src_labels   = labels[i];
        // get intersection set
        for ( j = 0; j < nsets; j++)
        {
            if ( j == i )
                continue;

            vtkIntArray  *can_labels   = labels[j]; 
            bset.GetIntersectionSet(i,j,src_cells,can_cells);

            int nisect = src_cells.size();
            // check if these cell labels need to be merged. 
            for(k = 0; k < nisect; k++)
            {
                src_label = src_labels->GetValue(src_cells[k]);
                can_label = can_labels->GetValue(can_cells[k]);
                if( union_find.Find(src_label) !=  union_find.Find(can_label))
                {
                    // merge them if the are not already in the same component
                    union_find.Union(src_label,can_label);
                    // add them to the output list
                    u_src.push_back(src_label);
                    u_des.push_back(can_label);
                }
            }
        }
    }

    visitTimer->StopTimer(t0,"Multi-Set Component Label List");
}




// ****************************************************************************
//  Method: avtConnComponentExpression::GlobalLabelShift
//
//  Purpose:
//      Globally shifts component labels to ensure they are unique across all
//      processors.
//
//  Arguments:
//    num_local_comps  The number of local components
//    labels           Label arrays for local meshes
//
//  Returns:       The global number of components after the shift
//
//  Programmer: Cyrus Harrison
//  Creation:   January 30, 2007
//
//  Modifications:
//    Cyrus Harrison, Fri Mar 16 15:50:10 PDT 2007
//    Added progress update. 
//
// ****************************************************************************
int
avtConnComponentsExpression::GlobalLabelShift
(int num_local_comps, const vector<vtkIntArray*> &labels)
{
    // in the serial case the is no shift and the number of labels 
    // is unchanged
    int num_comps = num_local_comps;

#ifdef PARALLEL
    // loop indices
    int i,j;

    // get the processor id and # of processors
    int procid = PAR_Rank();
    int nprocs = PAR_Size();

    int t0 = visitTimer->StartTimer();

    // get number of labels from all other processors
    int *rcv_buffer = new int[nprocs]; 
    // send number of components per processor, gather to all processors
    MPI_Allgather(&num_local_comps,1,MPI_INT,
                  rcv_buffer,1,MPI_INT,
                  VISIT_MPI_COMM);

    // using the received component counts, calculate the shift for this 
    // processor

    int num_global_comps = 0;
    int shift = 0;
    for(i=0;i<nprocs;i++)
    {
        // shift using component count from processors with a lower rank
        if(i < procid)
        {
            shift+= rcv_buffer[i];
        }
        // keep a count of the total number of components
        num_global_comps  += rcv_buffer[i];
    }

    // cleanup the receive buffer 
    delete [] rcv_buffer;    

    // shift the labels
    int nsets = labels.size();
    for(i = 0; i < nsets ;i++)
    {
        ShiftLabels(labels[i],shift);
        // update progress
        UpdateProgress(currentProgress++,totalSteps);
    }

    visitTimer->StopTimer(t0,"Global Label Shift");

    // set the number of components after the global shift
    num_comps = num_global_comps;
#endif

    // return the number of components
    return num_comps;

}

// ****************************************************************************
//  Method: avtConnComponentsExpression::GlobalResolve
//
//  Purpose:
//     Resolves component labels across all processors. 
//
//  Arguments:
//    num_comps       The current number of global components
//    bset            Boundary set for boundary queries
//    local_sets      Local meshes
//    local_labels    Label arrays for local meshes
// 
//  Returns:          The final number of components
//
//  Programmer: Cyrus Harrison
//  Creation:   January 25, 2007
//
// ****************************************************************************
int
avtConnComponentsExpression::GlobalResolve(int num_comps,
                                      BoundarySet &bset,
                                      const vector<vtkDataSet*> &local_sets,
                                      const vector<vtkIntArray*> &local_labels)
{
#ifdef PARALLEL
    int t0 = visitTimer->StartTimer();

    // loop index
    int i;

    // The spatial partition is used to evenly distributed mesh data
    // across available processors 
    SpatialPartition     spart;

    // vectors used to access the relocated datasets and their label arrays
    vector<vtkDataSet*>  sets;
    vector<vtkIntArray*> labels;

    // get the id of the local processor
    int procid = PAR_Rank();
    // get the number of local datasets
    int n_local_sets = local_sets.size();

    // To create the spatial partition, we first need to know the bounds of 
    // the entire dataset.

    double bounds[6];
    // get the bounds of the local sets
    bset.GetBounds(bounds);

    // unify to get the bounds of the entire dataset
    UnifyMinMax(bounds,6);

    // create the spatial partition
    spart.CreatePartition(bset,bounds);

    // Relocate proper cells using boundary set
    bset.RelocateUsingPartition(spart);

    // get the relocated datasets
    sets = bset.GetMeshes();
    int n_reloc_sets = sets.size();

    // get the component label arrays
    labels.resize(n_reloc_sets);
    for( i=0; i < n_reloc_sets; i++)
    {
        labels[i] = (vtkIntArray*)sets[i]->GetCellData()->
                                       GetArray(outputVariableName);
    }

    // list the union operations required to resolve the relocated data
    vector<int> union_src, union_des;
    MultiSetList(num_comps,bset, sets, labels,  union_src,union_des);

    // perform global union using the listed union operations
    num_comps = GlobalUnion(num_comps,union_src,union_des,local_labels);

    visitTimer->StopTimer(t0,"Global Label Resolve");
#endif
    // return the final number of components
    return num_comps;
}


// ****************************************************************************
//  Method: avtConnComponentsExpression::GlobalUnion
//
//  Purpose:
//      Globally resolves component labels. Each processor provides a list of 
//      local union operations. These are sent to all processors where they are
//      executed to produce final component labels.
//
//  Arguments:
//    num_comps      The current number of global components
//    u_src          Holds the first component for each union operation
//    u_des          Holds the second component for each union operation 
//    local_labels   Label arrays for local meshes
// 
//  Returns:        The final number of components
//
//  Programmer: Cyrus Harrison
//  Creation:   January 25, 2007
//
//  Modifications:
//    Cyrus Harrison, Fri Mar 16 15:51:20 PDT 2007
//    Added progress update. 
//
// ****************************************************************************
int
avtConnComponentsExpression::GlobalUnion(int num_comps,
                                      const vector<int> &u_src,
                                      const vector<int> &u_des,
                                      const vector<vtkIntArray*> &local_labels)
{
#ifdef PARALLEL
    // loop indices
    int i,j;

    int t0 = visitTimer->StartTimer();

    // get the number of processors and the current processor id
    int nprocs = PAR_Size();
    int procid = PAR_Rank();

    // create a union find data structure for resolving the labels
    // (in this case all union representatives are valid)
    UnionFind union_find(num_comps,true);

    // get the number of local union operations
    int n_local_unions = u_src.size();

    // pack the union operation label ids into an array
    int msg_size = n_local_unions*2;

    int *snd_msg = new int[msg_size];
    int *ptr = snd_msg;

    // TODO
    // perform local unions & prepare union list for other processors
    for( i = 0; i < n_local_unions; i++)
    {
        union_find.Union(u_src[i],u_des[i]);

        ptr[0] = u_src[i];
        ptr[1] = u_des[i];
        ptr+=2;
    }

    // communicate to find out how much to allocate for incoming union lists
    int *rcv_count = new int[nprocs];
    int *rcv_disp  = new int[nprocs];

    MPI_Allgather(&msg_size,1,MPI_INT,
                  rcv_count,1,MPI_INT,
                  VISIT_MPI_COMM);

    // calculate the size of the receive message, and the displacements
    // for each processor
    rcv_disp[0] = 0;
    int rcv_msg_size = rcv_count[0];
    for( i = 1; i < nprocs; i++)
    {
        rcv_disp[i] = rcv_count[i-1]  +  rcv_disp[i-1];
        rcv_msg_size+= rcv_count[i];
    }

    // allocate memory for the receive message
    int  *rcv_msg =  new int[rcv_msg_size];


    MPI_Allgatherv(snd_msg, msg_size, MPI_INT,
                   rcv_msg, rcv_count, rcv_disp, MPI_INT,
                   VISIT_MPI_COMM);

    // cleanup the send message
    delete [] snd_msg;

    // find the total number of unions
    int n_rcv_unions  = 0;
    for( i = 0; i < nprocs; i++)
    {
        n_rcv_unions+= rcv_count[i];
    }

    // each union has 2 entries, so divide by 2
    n_rcv_unions/=2;

    // perform the union operations
    ptr = rcv_msg;
    for( i = 0; i < n_rcv_unions; i++)
    {
        union_find.Union(ptr[0],ptr[1]);
        ptr+=2;
    }

    // cleanup com related arrays
    delete [] rcv_msg;
    delete [] rcv_count;
    delete [] rcv_disp;

    // flatten union find to obtain the final labels
    num_comps = union_find.FinalizeLabels();

    int nsets = local_labels.size();
    // use the union find to resolve all local component ids
    for( i = 0; i< nsets;i++)
    {
        // get the current label array and # of cells
        vtkIntArray *curr_array = local_labels[i];
        int          ncells     = curr_array->GetNumberOfTuples();

        for(j = 0; j < ncells; j ++)
        {
            // get the current label value
            int lbl =  curr_array->GetValue(j);
            // lookup the final label value
            lbl = union_find.GetFinalLabel(lbl);
            // set the final label value
            curr_array->SetValue(j,lbl);
        }

        // update progress
        UpdateProgress(currentProgress++,totalSteps);
    }
    
    visitTimer->StopTimer(t0,"Global Label Union");

#endif
    // return the final number of components
    return num_comps;
}

// ****************************************************************************
//  Method: avtConnComponentsExpression::ShiftLabels
//
//  Purpose:
//      Helper to shift label values by a specified amount.
//
//  Arguments:
//    labels     A component label array
//    shift      Amount of shift each label by
//
//  Programmer: Cyrus Harrison
//  Creation:   January 25, 2007
//
// ****************************************************************************
void
avtConnComponentsExpression::ShiftLabels(vtkIntArray *labels, int shift)
{
    // loop over all tuples
    int nlabels = labels->GetNumberOfTuples();
    for(int i = 0; i < nlabels ; i++)
    {
        // get the current label value
        int curr = labels->GetValue(i);
        // add the shift amount
        curr += shift;
        // set the new label value
        labels->SetValue(i,curr);
    }
}


// ****************************************************************************
//  Method: UnionFind constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   January  23, 2007
//
// ****************************************************************************

avtConnComponentsExpression::UnionFind::UnionFind(int num_items,
                                                  bool all_valid)
{
    // prepare union find vectors
    parents.resize(num_items);
    ranks.resize(num_items);
    valid.resize(num_items);
    finalLabels.resize(num_items);

    // initialize union find vectors
    for(int i = 0; i < num_items ;i++)
    {
        parents[i] = -1;
        ranks[i] = 0;
        valid[i] = all_valid;
        finalLabels[i] = -1;
    }
}


// ****************************************************************************
//  Method: UnionFind destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   January  23, 2007
//
// ****************************************************************************

avtConnComponentsExpression::UnionFind::~UnionFind()
{
    ;
}


// ****************************************************************************
//  Method: UnionFind::Find
//
//  Purpose:
//      Finds the current representative of a given label.
//  
//  Arguments:
//    label     Label to find 
//
//  Return: The current component representative of the input label.
//
//  Programmer: Cyrus Harrison
//  Creation:   January  23, 2007
//
// ****************************************************************************
int
avtConnComponentsExpression::UnionFind::Find(int label)
{
    // if the current label does not have a parent, return its value
    if( parents[label] == -1)
    {
        return label;
    }

    // path compression
    parents[label] = Find(parents[label]);

    // return the path compressed parent
    return parents[label];
}

// ****************************************************************************
//  Method: UnionFind::Union
//
//  Purpose:
//      Unions the two components represented by the given labels.
// 
//  Arguments:
//    label_x     Input label
//    label_y     Input label
//
//  Programmer: Cyrus Harrison
//  Creation:   January  23, 2007
//
// ****************************************************************************
void
avtConnComponentsExpression::UnionFind::Union(int label_x, int label_y)
{
    // if labels are used in an union operation, they are 
    // considered valid
    valid[label_x] = true;
    valid[label_y] = true;

    // find the component values for the input labels
    int find_x = Find(label_x);
    int find_y = Find(label_y);

    // set parent based on which has higher rank
    if(ranks[find_x] > ranks[find_x])
    {
        parents[find_y] = find_x;
    }
    else if(ranks[find_x] < ranks[find_y])
    {
        parents[find_x] = find_y;
    }
    else if( find_x != find_y)
    {
        // if their ranks are equal and they are in different components
        // break the tie, and increase the rank
        parents[find_y] =  find_x;
        ranks[find_x]++;
    }
}

// ****************************************************************************
//  Method: UnionFind::IsValid
//
//  Purpose:
//      Checks if the given label belongs to a valid set. 
//      A set label is considered valid if it was used in an union operation
//      or explicitly set to valid (either when the union find object was 
//      constructed or using SetValid )
//
//  Arguments:
//    label     Label to check 
//
//  Returns: If the given label is valid
//
//  Programmer: Cyrus Harrison
//  Creation:   January  23, 2007
//
// ****************************************************************************
bool
avtConnComponentsExpression::UnionFind::IsValid(int label)
{
    return valid[label];
}

// ****************************************************************************
//  Method: UnionFind::SetValid
//
//  Purpose:
//      Allows a user to explicitly set if a label is valid.
//
//  Arguments:
//    label     Label to set 
//    valid     New label validity value
//
//  Programmer: Cyrus Harrison
//  Creation:   January  26, 2007
//
// ****************************************************************************
void
avtConnComponentsExpression::UnionFind::SetValid(int label, bool value)
{
    valid[label] =value;
}


// ****************************************************************************
//  Method: UnionFind::FinalizeLabels
//
//  Purpose:
//      Flattens the merged components to find the final number of unique
//      components. Assigns final labels ( 0 - (# of unique components -1)
//      to all valid component representative. These labels may be accessed
//      using GetFinalLabel()
//      
//  Returns: The final number of labels.
//
//  Programmer: Cyrus Harrison
//  Creation:   January  23, 2007
//
// ****************************************************************************
int
avtConnComponentsExpression::UnionFind::FinalizeLabels()
{
    // loop index
    int i;
    // get the total number of representatives
    int nitems = parents.size();

    // use a map to resolve labels
    std::map<int,int> labels;
    int current_label = 0;

    // loop over representatives
    for(i = 0; i < nitems; i++)
    {
        // if the label is valid, proceed
        if(valid[i])
        {
            // lookup label value
            int label = Find(i);

            // check if label is unique
            std::map<int,int>::iterator itr = labels.find(label);
            if(itr == labels.end())
            {
                // if the label does not exist in the map,
                // create entry with a new label
                labels[label] = current_label;
                current_label++;
            }

        }
    }

    // prepare the final labels lookup
    for(i = 0; i < nitems; i++)
    {
        // set final label if representative is valid
        if(valid[i])
        {
            finalLabels[i] = labels[Find(i)];
        }
    }
    // set and return the final number of of labels
    nFinalLabels = current_label;
    return nFinalLabels;
}

// ****************************************************************************
//  Method: UnionFind::GetFinalLabel
//
//  Purpose:
//      After FinalizeLabels() has been called, this method provides access
//      to the final label value of an input label.
//
//  Arguments:
//      label    Input label
//
//  Returns: The final label of the given representative, -1 if the input
//           is invalid.
//
//  Programmer: Cyrus Harrison
//  Creation:   January  23, 2007
//
// ****************************************************************************
int
avtConnComponentsExpression::UnionFind::GetFinalLabel(int label)
{
    return finalLabels[label];
}

// ****************************************************************************
//  Method: UnionFind::GetNumberOfFinalLabels
//
//  Purpose:
//      After FinalizeLabels() has been called this method reports the number
//      of final labels.
//
//  Returns: The final number of labels.
//
//  Programmer: Cyrus Harrison
//  Creation:   January  23, 2007
//
// ****************************************************************************
int
avtConnComponentsExpression::UnionFind::GetNumberOfFinalLabels()
{
    return nFinalLabels;
}


// ****************************************************************************
//  Method: BoundarySet constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   January  25, 2007
//
// ****************************************************************************

avtConnComponentsExpression::BoundarySet::BoundarySet()
{
    empty = true;
}

// ****************************************************************************
//  Method: BoundarySet destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   January  25, 2007
//
// ****************************************************************************

avtConnComponentsExpression::BoundarySet::~BoundarySet()
{
    Clear();
}


// ****************************************************************************
//  Method: BoundarySet::AddMesh
//
//  Purpose:
//      Adds a mesh to the boundary set. After all meshes have been added
//      use Finalize() to prepare the boundary set for queries.
//
//  Arguments:
//     mesh     Mesh to add to the boundary set
//
//  Notes: Based on Hank Child's avtPosCMFEAlgorithm::FastLookupGrouping
//
//  Programmer: Cyrus Harrison
//  Creation:   January  25, 2007
//
// ****************************************************************************
void
avtConnComponentsExpression::BoundarySet::AddMesh(vtkDataSet *mesh)
{
    // increment the vtk ref counter
    mesh->Register(NULL);
    // add the mesh the boundary set
    meshes.push_back(mesh);
}

// ****************************************************************************
//  Method: BoundarySet::Finalize
//
//  Purpose:
//      Prepares the boundary set for queries by building the supporting 
//      interval trees and computing the boundary set bounds.
// 
//  Notes: Based on Hank Child's avtPosCMFEAlgorithm::FastLookupGrouping
//
//  Programmer: Cyrus Harrison
//  Creation:   January  25, 2007
//
//  Modifications:
//    Cyrus Harrison, Thu Mar  1 07:47:58 PST 2007
//    Added guard for empty input datasets. 
//
// ****************************************************************************
void
avtConnComponentsExpression::BoundarySet::Finalize()
{
    // loop indices
    int i, j, k;

    // set bounds defaults
    bounds[0] =  DBL_MAX;
    bounds[1] = -DBL_MAX;
    bounds[2] =  DBL_MAX;
    bounds[3] = -DBL_MAX;
    bounds[4] =  DBL_MAX;
    bounds[5] = -DBL_MAX;

    // get the number of sets
    int nsets = meshes.size();

    // cleanup old itrees if they exist 
    if(itrees.size() > 0)
    {
        if(itrees[i] != NULL)
            delete itrees[i];
    }
    itrees.clear();

    // if empty do not try to create the itrees
    if(nsets == 0)
    {
        empty = true;
        return;
    }

    empty = false;

    itrees.resize(nsets);

    for(i = 0; i < nsets ; i++)
    {
        // for each data set
        double curr_bounds[6];
        vtkDataSet *curr_set = meshes[i];
        curr_set->GetBounds(curr_bounds);
        // update the bounding box
        if(curr_bounds[0] < bounds[0])
            bounds[0] = curr_bounds[0];
        if(curr_bounds[1] > bounds[1])
            bounds[1] = curr_bounds[1];

        if(curr_bounds[2] < bounds[2])
            bounds[2] = curr_bounds[2];
        if(curr_bounds[3] > bounds[3])
            bounds[3] = curr_bounds[3];

        if(curr_bounds[4] < bounds[4])
            bounds[4] = curr_bounds[4];
        if(curr_bounds[5] > bounds[5])
            bounds[5] = curr_bounds[5];

        // add cells to the current mesh's interval tree
        int curr_ncells = curr_set->GetNumberOfCells();
        // make sure we dont create an empty itree
        if(curr_ncells > 0 )
        {
            avtIntervalTree *curr_itree = new avtIntervalTree(curr_ncells,3);

            for(j = 0; j< curr_ncells; j++)
            {
                // add each cell's bounds to the interval tree
                vtkCell *cell = curr_set->GetCell(j);
                double bounds[6];
                cell->GetBounds(bounds);
                curr_itree->AddElement(j,bounds);
            }
            // build the interval tree for the current mesh
            curr_itree->Calculate(true);
            itrees[i] = curr_itree;
        }
        else
        {itrees[i] = NULL;}
    }

}


// ****************************************************************************
//  Method: BoundarySet::Clear
//
//  Purpose:
//      Removes all meshes from the boundary set and cleans up internal 
//      data structures. 
// 
//  Programmer: Cyrus Harrison
//  Creation:   January  25, 2007
//
//  Modifications:
//    Cyrus Harrison, Thu Mar  1 07:46:46 PST 2007
//    Added check for null itrees
//
// ****************************************************************************
void
avtConnComponentsExpression::BoundarySet::Clear()
{
    // unregister all meshes and clean up per mesh itrees
    int nsets = meshes.size();
    for(int i=0;i<nsets;i++)
    {
        meshes[i]->Delete();
        if(itrees[i])
            delete itrees[i];
    }
    meshes.clear();
    itrees.clear();

}

// ****************************************************************************
//  Method: BoundarySet::GetMeshes
//
//  Purpose:
//      Gets a vector containing pointers to all datasets contained in the
//      boundary set. 
// 
//  Returns: A vector holding pointers to all boundary set meshes. 
//
//  Programmer: Cyrus Harrison
//  Creation:   January  25, 2007
//
// ****************************************************************************
vector<vtkDataSet *>
avtConnComponentsExpression::BoundarySet::GetMeshes() const
{
    return meshes;
}

// ****************************************************************************
//  Method: BoundarySet::GetBounds
//
//  Purpose:
//      Gets the bounding box of the entire boundary set.
// 
//  Arguments:
//      bounds   Holds bounds output (o)
//
//  Programmer: Cyrus Harrison
//  Creation:   January  25, 2007
//
// ****************************************************************************
void 
avtConnComponentsExpression::BoundarySet::GetBounds(double *bounds) const
{
    if(! empty)
    {
        // copy out bounds values
        memcpy(bounds,this->bounds,sizeof(double)*6);
    }
    else
    {
        memset(bounds,0,sizeof(double)*6);
    }

}


// ****************************************************************************
//  Method: BoundarySet::GetIntersectionSet
//
//  Purpose:
//      Gets the intersection between two meshes. 
//
//  Arguments:
//      src_mesh_index       BoundarySet index of the source mesh 
//      can_mesh_index       BoundarySet index of the candidate mesh 
//      src_cells            Resulting source cell indices
//      can_cells            Resulting candidate cell indices
//
//  Programmer: Cyrus Harrison
//  Creation:   February  16, 2007
//
//  Modifications:
//
//    Cyrus Harrison, Thu Mar  1 07:49:44 PST 2007
//    Added case for empty input datasets.
//
// ****************************************************************************
void
avtConnComponentsExpression::BoundarySet::GetIntersectionSet(
                                       int src_mesh_index,
                                       int can_mesh_index,
                                       vector<int> &src_cells,
                                       vector<int> &can_cells ) const
{
    int i, j, k;
    double src_bounds[6], can_bounds[6], isect_bounds[6], src_cell_bounds[6];
    double lb[3],ub[3];
    vector<int> src_isect_cells;
    vector<int> can_isect_cells;
    int nsrc_isect_cells, ncan_isect_cells;

    // clear result vectors
    src_cells.clear();
    can_cells.clear();

    // dont handle self intersect
    if(src_mesh_index == can_mesh_index)
        return;

    // get source and candidate meshes & itrees
    vtkDataSet *src_mesh = meshes[src_mesh_index];
    vtkDataSet *can_mesh = meshes[can_mesh_index];

    avtIntervalTree *src_itree = itrees[src_mesh_index];
    avtIntervalTree *can_itree = itrees[can_mesh_index];

    // if an input dataset is empty, its itree will be null
    // check for this case
    if(src_itree == NULL || can_itree == NULL)
        return;

    // get source and candidate bounds
    src_mesh->GetBounds(src_bounds);
    can_mesh->GetBounds(can_bounds);

    // make sure they intersect - if not we are done
    if( !GetBoundsIntersection(src_bounds,can_bounds,isect_bounds) )
        return;

    // find source cells in the intersection
    lb[0] = isect_bounds[0];
    lb[1] = isect_bounds[2];
    lb[2] = isect_bounds[4];

    ub[0] = isect_bounds[1];
    ub[1] = isect_bounds[3];
    ub[2] = isect_bounds[5];

    src_itree->GetElementsListFromRange(lb,ub,src_isect_cells);
    nsrc_isect_cells = src_isect_cells.size();

    // if there are no source cells in this range, we are done
    if (nsrc_isect_cells == 0)
        return;

    // for each source cell, find any candidate cells that may intersect
    // and test for intersection

    for( i = 0; i< nsrc_isect_cells ; i++)
    {
        // get the source cell and its bounds
        int      src_cell_idx =  src_isect_cells[i];
        vtkCell *src_cell = src_mesh->GetCell(src_cell_idx);
        src_cell->GetBounds(src_cell_bounds);

        // query the candidate set itree for cells in the source cells bounds
        lb[0] = src_cell_bounds[0];
        lb[1] = src_cell_bounds[2];
        lb[2] = src_cell_bounds[4];

        ub[0] = src_cell_bounds[1];
        ub[1] = src_cell_bounds[3];
        ub[2] = src_cell_bounds[5];

        can_itree->GetElementsListFromRange(lb,ub,can_isect_cells);
        ncan_isect_cells = can_isect_cells.size();
        // if there are no source cells in this range, check next source cell
        if (ncan_isect_cells == 0)
            continue;

        for( j = 0; j < ncan_isect_cells; j++)
        {
            // get the candidate cell, cell index and cell
            int       can_cell_idx = can_isect_cells[j];
            vtkCell  *can_cell = can_mesh->GetCell(can_cell_idx);
            int       n_can_pts  = can_cell->GetNumberOfPoints();
            int       n_src_pts  = src_cell->GetNumberOfPoints();
            bool      isect = false;

            double pt_val[3];
            int    pt_id;

            // check for intersection
            for( k = 0; k < n_can_pts && !isect ; k++)
            {
                // get current point id
                pt_id = can_cell->GetPointId(k);
                // get current point value
                can_mesh->GetPoint(pt_id,pt_val);
                // check for intersection
                isect = vtkVisItUtility::CellContainsPoint(src_cell,
                                                           pt_val);
            }

            for( k = 0; k < n_src_pts && !isect ; k++)
            {
                // get current point id
                pt_id = src_cell->GetPointId(k);
                // get current point value
                src_mesh->GetPoint(pt_id,pt_val);
                // check for intersection
                isect = vtkVisItUtility::CellContainsPoint(can_cell,
                                                           pt_val);
            }


            // if they intersect add to result vectors
            if(isect)
            {
                src_cells.push_back(src_cell_idx);
                can_cells.push_back(can_cell_idx);
            }
        }
    }
}


// ****************************************************************************
//  Method: BoundarySet::GetBoundsIntersection
//
//  Purpose:
//      Finds the intersection of two bounding boxes.
// 
//  Arguments:
//      a        Boundary to test
//      b        Boundary to test
//      res      The intersection of a and b (o)
//
//  Returns: True if the input boxes intersect, false otherwise.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 15, 2007
//
// ****************************************************************************
bool
avtConnComponentsExpression::BoundarySet::GetBoundsIntersection
(double *a, double *b, double *res) const
{

    // check x coord
    // ax_l, bx_l, ax_h, bx_h
    if( a[0] <= b[0]  &&  b[0] <= a[1] && a[1] <=b[1] )
    {
        res[0] = b[0];
        res[1] = a[1];
    }
    // ax_l, bx_l, bx_h, ax_h
    else if( a[0] <= b[0]  &&  b[1] <= a[1] )
    {
        res[0] = b[0];
        res[1] = b[1];
    }
    // bx_l, ax_l, ax_h, bx_h
    else if( b[0] <= a[0] && a[1] <= b[1])
    {
        res[0] = a[0];
        res[1] = a[1];
    }
    // bx_l, ax_l, bx_h, ax_h
    else if( b[0] <= a[0]  &&  a[0] <= b[1] && b[1] <=a[1] )
    {
        res[0] = a[0];
        res[1] = b[1];
    }
    else
    {
        return false;
    }

    // check y coord
    // ay_l, by_l, ay_h, by_h
    if( a[2] <= b[2]  &&  b[2] <= a[3] && a[3] <=b[3] )
    {
        res[2] = b[2];
        res[3] = a[3];
    }
    // ay_l, by_l, by_h, ay_h
    else if( a[2] <= b[2]  &&  b[3] <= a[3] )
    {
        res[2] = b[2];
        res[3] = b[3];
    }
    // by_l, ay_l, ay_h, by_h
    else if( b[2] <= a[2] && a[3] <= b[3])
    {
        res[2] = a[2];
        res[3] = a[3];
    }
    // by_l, ay_l, by_h, ay_h
    else if( b[2] <= a[2]  &&  a[2] <= b[3] && b[3] <=a[3] )
    {
        res[2] = a[2];
        res[3] = b[3];
    }
    else
    {
        return false;
    }

    // check z coord
    // az_l, bz_l, az_h, bz_h
    if( a[4] <= b[4]  &&  b[4] <= a[5] && a[5] <=b[5] )
    {
        res[4] = b[4];
        res[5] = a[5];
    }
    // az_l, bz_l, bz_h, az_h
    else if( a[4] <= b[4]  &&  b[5] <= a[5] )
    {
        res[4] = b[4];
        res[5] = b[5];
    }
    // bz_l, ay_l, az_h, bz_h
    else if( b[2] <= a[2] && a[3] <= b[3])
    {
        res[4] = a[4];
        res[5] = a[5];
    }
    // bz_l, az_l, bz_h, az_h
    else if( b[4] <= a[4]  &&  a[4] <= b[5] && b[5] <=a[5] )
    {
        res[4] = a[4];
        res[5] = b[5];
    }
    else
    {
        return false;
    }

    return true;
}

// ****************************************************************************
//  Method: BoundarySet::RelocateUsingPartition
//
//  Purpose:
//      Relocates mesh data to the proper processor in the spatial partition.
//      Adapted from avtPosCMFEAlgorithm::FastLookupGrouping
// 
//  Arguments:
//      spart      Spatial Partition used to relocate cells.
//
//  Notes: Adapted from Hank Child's avtPosCMFEAlgorithm::FastLookupGrouping
//
//  Programmer: Cyrus Harrison
//  Creation:   February 2, 2007
//
// ****************************************************************************
void
avtConnComponentsExpression::BoundarySet::RelocateUsingPartition
(const SpatialPartition &spart) 
{
#ifdef PARALLEL
    // loop indices
    int i,j,k;

    // get the current processor id and the # of processors
    int nprocs = PAR_Size();
    int procid = PAR_Rank();
    
    // used to hold temporary meshes created to send to other processors
    vtkUnstructuredGrid **snd_meshes = new vtkUnstructuredGrid*[nprocs];
    // holds the current number of cells in the temporary meshes
    int                  *snd_ncells = new int[nprocs];

    // holds appenders used to concatenate temporary data sets 
    // into a single data set for each processor
    vtkAppendFilter     **appenders  = new vtkAppendFilter*[nprocs];
    
    // create the append filters
    for( i = 0 ; i< nprocs;i++)
    {
        appenders[i] = vtkAppendFilter::New();
    }

    // used to hold which processor a cell needs to be sent to 
    vector<int> proc_list;

    // get number of local meshes
    int nmeshes = meshes.size();

    // for each local mesh
    for( i = 0; i < nmeshes ; i++)
    {
        // find which cells from the current mesh to send
        vtkUnstructuredGrid *mesh = (vtkUnstructuredGrid*) meshes[i];
        int ncells = mesh->GetNumberOfCells();
        vtkIntArray *labels = (vtkIntArray*)mesh->GetCellData()->GetArray("ccl");
      
        // initialize temporary mesh and cell count holder        
        for( j = 0; j < nprocs; j++)
        {
            snd_meshes[j] = NULL;
            snd_ncells[j] = 0;
        }

        // for each cell
        for( j = 0; j < ncells; j++)
        {
            // find which processors need this cell
            vtkCell *cell = mesh->GetCell(j);
            spart.GetProcessorList(cell,proc_list);
            int n_isect_procs = proc_list.size();

            // copy the cell data to the proper temporary mesh
            for( k = 0; k < n_isect_procs; k++)
            {
                // get the destination processor
                int des_proc = proc_list[k];
                // get pointer to the temporary mesh for the dest processor
                vtkUnstructuredGrid *des_mesh = snd_meshes[des_proc]; 

                // if this mesh does not exist, create it
                if( des_mesh == NULL)
                {
                    // if this mesh does not exist, create it and
                    // copy necessary data from the source mesh
                    des_mesh = vtkUnstructuredGrid::New();
                    vtkPoints *pts = vtkVisItUtility::GetPoints(mesh);
                    des_mesh->SetPoints(pts);
                    des_mesh->GetPointData()->ShallowCopy(mesh->GetPointData());
                    des_mesh->GetCellData()->CopyAllocate(mesh->GetCellData());
                    des_mesh->Allocate(ncells*9);
                    pts->Delete();
                    snd_meshes[des_proc] = des_mesh;
                }

                // copy this cell's data to the temporary mesh
                int cell_type  = mesh->GetCellType(j);
                vtkIdList *ids = cell->GetPointIds();
                des_mesh->InsertNextCell(cell_type,ids);
                des_mesh->GetCellData()->CopyData(mesh->GetCellData(),
                                                  j,
                                                  snd_ncells[des_proc]);
                snd_ncells[des_proc]++;
            }

        }

        // add any temporary meshes to the proper appenders
        for( j = 0; j < nprocs ; j++)
        {
            vtkUnstructuredGrid *des_mesh =  snd_meshes[j];
            if(des_mesh != NULL)
            {
                // use this filter to remove unnecessary points
                vtkUnstructuredGridRelevantPointsFilter *ugrpf=
                    vtkUnstructuredGridRelevantPointsFilter::New();
                ugrpf->SetInput(des_mesh);
                ugrpf->Update();
                // add the filter output to the proper appender
                appenders[j]->AddInput(ugrpf->GetOutput());
                // dec ref count for the filter
                ugrpf->Delete();
                // delete the temporary mesh
                des_mesh->Delete();
            }
        }

    }

    // clean up arrays used for temporary mesh data
    delete [] snd_meshes;
    delete [] snd_ncells;

    
    // use the appenders to concatenate mesh data to produce the final dataset
    // for each processor & and serialize the data for sending

    int  *snd_count = new int[nprocs];
    char **snd_msgs = new char*[nprocs];

    for(i=0;i<nprocs;i++)
    {
        // make sure the appender has data
        if(appenders[i]->GetTotalNumberOfInputConnections() == 0)
        {
            // if it does not have any input data sets
            // set default message values and clean up
            snd_count[i] = 0;
            snd_msgs[i] = NULL;
            // delete the current appender
            appenders[i]->Delete();
            continue;
        }

        // exe the current appender
        appenders[i]->Update();

        // serialize mesh data to a char array using an UnstructredGrid Writer
        vtkUnstructuredGridWriter *wtr = vtkUnstructuredGridWriter::New();
        wtr->SetInput(appenders[i]->GetOutput());
        wtr->SetWriteToOutputString(1);
        wtr->SetFileTypeToBinary();
        wtr->Write();
        // set up the message for the des processor
        snd_count[i] = wtr->GetOutputStringLength();
        snd_msgs[i] = wtr->RegisterAndGetOutputString();
       
        // delete the writer
        wtr->Delete();
        // delete the current appender
        appenders[i]->Delete();
    }

    // clean up array used to hold appenders
    delete [] appenders;

    // calculate the total message size
    int total_msg_size = 0;
    for(i = 0; i<nprocs;i++)
        total_msg_size += snd_count[i];

    // allocate space for the total message
    char *snd_msg = new char[total_msg_size];
    char *ptr = snd_msg;
    
    // pack messages into the send buffer
    for(i=0;i<nprocs;i++)
    {
        if(snd_msgs[i] != NULL)
        {
            memcpy(ptr,snd_msgs[i],snd_count[i]*sizeof(char));
            ptr+=snd_count[i]*sizeof(char);
            delete [] snd_msgs[i];
        }
    }
    // cleanup the array holding the per processor messages
    delete [] snd_msgs;

    // comm to find out the message sizes from other processors
    int *rcv_count = new int[nprocs];

    MPI_Alltoall(snd_count,1,MPI_INT,
                 rcv_count,1,MPI_INT,VISIT_MPI_COMM);


    // use a big comm to send mesh data to the proper processors
    char **rcv_msgs = new char*[nprocs];
    char *rcv_msg = CreateMessageStrings(rcv_msgs,rcv_count,nprocs);
    
    // set up the displacement arrays for send and receive buffers
    int *snd_disp = new int[nprocs];
    int *rcv_disp = new int[nprocs];
    
    snd_disp[0] = 0;
    rcv_disp[0] = 0;
    
    for(i=1;i<nprocs;i++)
    {
        snd_disp[i] = snd_disp[i-1] + snd_count[i-1];
        rcv_disp[i] = rcv_disp[i-1] + rcv_count[i-1];
    }
    
    // perform big com
    MPI_Alltoallv(snd_msg,snd_count, snd_disp, MPI_CHAR,
                  rcv_msg,rcv_count, rcv_disp, MPI_CHAR,
                  VISIT_MPI_COMM);
 
    // clear all current meshes
    Clear();

    // read new meshes and rebuild the boundary set
    for( i = 0; i < nprocs; i++)
    {
        if(rcv_count[i] == 0)
            continue;
        
        vtkCharArray *char_array = vtkCharArray::New();
        // when setting array make sure to tell vtk that 
        // we own the memory, and it should not delete it
        // this is what the "1" is for
        char_array->SetArray(rcv_msgs[i],rcv_count[i],1);
        
        // read the mesh data from proc i
        vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
        reader->SetReadFromInputString(1);
        reader->SetInputArray(char_array);

        vtkUnstructuredGrid *mesh = reader->GetOutput();
        mesh->Update();

        // add new mesh to the boundary set
        AddMesh(mesh);

        // delete the reader and the data array
        reader->Delete();
        char_array->Delete();
        
    }
    // prepare the boundary set
    Finalize();


    // cleanup comm related arrays
    delete [] snd_count;    
    delete [] snd_disp;    
    delete [] snd_msg; 


    delete [] rcv_count;
    delete [] rcv_disp;
    delete [] rcv_msgs;
    delete [] rcv_msg;



#endif
}


// ****************************************************************************
//  Method: SpatialPartition constructor
//
//  Programmer: Hank Childs
//  Creation:   October 12, 2005
//
// ****************************************************************************

avtConnComponentsExpression::SpatialPartition::SpatialPartition()
{
    itree = NULL;
}


// ****************************************************************************
//  Method: SpatialPartition destructor
//
//  Programmer: Hank Childs
//  Creation:   October 12, 2005
//
// ****************************************************************************

avtConnComponentsExpression::SpatialPartition::~SpatialPartition()
{
    delete itree;
}


// ****************************************************************************
//  Class: PartitionBoundary
//
//  Purpose:
//      This class is for setting up a spatial partition.  It contains methods
//      that allow the spatial partitioning routine to not be so cumbersome.
// 
//  Notes: Adapted from Hank Child's Boundary class within 
//         avtPosCMFEAlgorithm.C. Renamed to PartitionBoundary to avoid
//         multiple symbol def problems.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 2, 2007
//
// ****************************************************************************

typedef enum
{
    X_AXIS,
    Y_AXIS, 
    Z_AXIS
} Axis;

const int npivots = 5;
class PartitionBoundary
{
   public:
                      PartitionBoundary(const float *, int, Axis);
     virtual         ~PartitionBoundary() {;};

     float           *GetBoundary() { return bounds; };
     bool             AttemptSplit(PartitionBoundary *&, PartitionBoundary *&);

     bool             IsDone(void) { return isDone; };
     bool             IsLeaf(void) { return (numProcs == 1); };

     void             AddPoint(const float *);

     static void      SetIs2D(bool b) { is2D = b; };
     static void      PrepareSplitQuery(PartitionBoundary **, int);
     
   protected:
     float            bounds[6];
     float            pivots[npivots];
     int              numCells[npivots+1];
     int              numProcs;
     int              nAttempts;
     Axis             axis;
     bool             isDone;
     static bool      is2D;
};

bool PartitionBoundary::is2D = false;


// ****************************************************************************
//  Method: PartitionBoundary constructor
//
//  Programmer: Hank Childs
//  Creation:   January 9, 2006
// 
// ****************************************************************************

PartitionBoundary::PartitionBoundary(const float *b, int n, Axis a)
{
    int  i;

    for (i = 0 ; i < 6 ; i++)
        bounds[i] = b[i];
    numProcs = n;
    axis = a;
    isDone = (numProcs == 1);
    nAttempts = 0;

    //
    // Set up the pivots.
    //
    int index = 0;
    if (axis == Y_AXIS)
        index = 2;
    else if (axis == Z_AXIS)
        index = 4;
    float min = bounds[index];
    float max = bounds[index+1];
    float step = (max-min) / (npivots+1);
    for (i = 0 ; i < npivots ; i++)
    {
        pivots[i] = min + (i+1)*step;
    }
    for (i = 0 ; i < npivots+1 ; i++)
        numCells[i] = 0;
}


// ****************************************************************************
//  Method: PartitionBoundary::PrepareSplitQuery
//
//  Purpose:
//      Each Boundary is operating only with the information on this processor.
//      When it comes time to determine if we can split a boundary, the info
//      from each processor needs to be unified.  That is the purpose of this
//      method.  It unifies the information so that Boundaries can later make
//      good decisions regarding whether or not they can split themselves.
//
//  Programmer: Hank Childs
//  Creation:   January 9, 2006
// 
// ****************************************************************************

void
PartitionBoundary::PrepareSplitQuery(PartitionBoundary **b_list, int listSize)
{
    int   i, j;
    int   idx;

    int  num_vals = listSize*(npivots+1);
    int *in_vals = new int[num_vals];
    idx = 0;
    for (i = 0 ; i < listSize ; i++)
        for (j = 0 ; j < npivots+1 ; j++)
            in_vals[idx++] = b_list[i]->numCells[j];

    int *out_vals = new int[num_vals];
    SumIntArrayAcrossAllProcessors(in_vals, out_vals, num_vals);

    idx = 0;
    for (i = 0 ; i < listSize ; i++)
        for (j = 0 ; j < npivots+1 ; j++)
            b_list[i]->numCells[j] = out_vals[idx++];

    delete [] in_vals;
    delete [] out_vals;
}


// ****************************************************************************
//  Method: PartitionBoundary::AddPoint
//
//  Purpose:
//      Adds a point to the boundary.
//
//  Programmer: Hank Childs
//  Creation:   January 9, 2006
// 
// ****************************************************************************

void
PartitionBoundary::AddPoint(const float *pt)
{
    float p = (axis == X_AXIS ? pt[0] : axis == Y_AXIS ? pt[1] : pt[2]);
    for (int i = 0 ; i < npivots ; i++)
        if (p < pivots[i])
        {
            numCells[i]++;
            return;
        }
    numCells[npivots]++;
}


// ****************************************************************************
//  Method: PartitionBoundary::PrepareSplitQuery
//
//  Purpose:
//      Sees if the boundary has found an acceptable pivot to split around.
//
//  Programmer: Hank Childs
//  Creation:   January 9, 2006
// 
// ****************************************************************************

bool
PartitionBoundary::AttemptSplit(PartitionBoundary *&b1, PartitionBoundary *&b2)
{
    int  i;

    int numProcs1 = numProcs/2;
    int numProcs2 = numProcs-numProcs1;
    
    int totalCells = 0;
    for (i = 0 ; i < npivots+1 ; i++)
        totalCells += numCells[i]; 

    if (totalCells == 0)
    {
        // Should never happen...
        debug1 << "Error condition occurred when making boundaries" << endl;
        isDone = true;
        return false;
    }

    int cellsSoFar = 0;
    float amtSeen[npivots];
    for (i = 0 ; i < npivots ; i++)
    {
        cellsSoFar += numCells[i];
        amtSeen[i] = cellsSoFar / ((float) totalCells);
    }

    float proportion = ((float) numProcs1) / ((float) numProcs);
    float closest  = fabs(proportion - amtSeen[0]); // == proportion
    int   closestI = 0;
    for (i = 1 ; i < npivots ; i++)
    {
        float diff = fabs(proportion - amtSeen[i]);
        if (diff < closest)
        {
            closest  = diff;
            closestI = i;
        }
    }

    nAttempts++;
    if (closest < 0.02 || nAttempts > 3)
    {
        float b_tmp[6];
        for (i = 0 ; i < 6 ; i++)
            b_tmp[i] = bounds[i];
        if (axis == X_AXIS)
        {
            b_tmp[1] = pivots[closestI];
            b1 = new PartitionBoundary(b_tmp, numProcs1, Y_AXIS);
            b_tmp[0] = pivots[closestI];
            b_tmp[1] = bounds[1];
            b2 = new PartitionBoundary(b_tmp, numProcs2, Y_AXIS);
        }
        else if (axis == Y_AXIS)
        {
            Axis next_axis = (is2D ? X_AXIS : Z_AXIS);
            b_tmp[3] = pivots[closestI];
            b1 = new PartitionBoundary(b_tmp, numProcs1, next_axis);
            b_tmp[2] = pivots[closestI];
            b_tmp[3] = bounds[3];
            b2 = new PartitionBoundary(b_tmp, numProcs2, next_axis);
        }
        else
        {
            b_tmp[5] = pivots[closestI];
            b1 = new PartitionBoundary(b_tmp, numProcs1, X_AXIS);
            b_tmp[4] = pivots[closestI];
            b_tmp[5] = bounds[5];
            b2 = new PartitionBoundary(b_tmp, numProcs2, X_AXIS);
        }
        isDone = true;
    }
    else
    {
        //
        // Set up the pivots.  We are going to reset the pivot positions to be
        // in between the two closest pivots.
        //
        int firstBigger = -1;
        int amtSeen = 0;
        for (i = 0 ; i < npivots+1 ; i++)
        {
            amtSeen += numCells[i];
            float soFar = ((float) amtSeen) / ((float) totalCells);
            if (soFar > proportion)
            {
                firstBigger = i;
                break;
            }
        }

        float min, max;

        int index = 0;
        if (axis == Y_AXIS)
            index = 2;
        else if (axis == Z_AXIS)
            index = 4;

        if (firstBigger <= 0)
        {
            min = pivots[0] - (pivots[1] - pivots[0]);
            max = pivots[0];
        } 
        else if (firstBigger >= npivots)
        {
            min = pivots[npivots-1];
            max = pivots[npivots-1] + (pivots[1] - pivots[0]);
        }
        else
        {
            min = pivots[firstBigger-1];
            max = pivots[firstBigger];
        }
        float step = (max-min) / (npivots+1);
        for (i = 0 ; i < npivots ; i++)
            pivots[i] = min + (i+1)*step;
        for (i = 0 ; i < npivots+1 ; i++)
            numCells[i] = 0;

        return false;
    }

    return true;
}


// ****************************************************************************
//  Method: SpatialPartition::CreatePartition
//
//  Purpose:
//      Creates a partition that is balanced for both the desired points and
//      the fast lookup grouping.
//
//  Notes: Adapted from Hank Child's avtPosCMFEAlgorithm::SpatialPartition
//
//
//  Programmer: Cyrus Harrison
//  Creation:   February 2, 2007
//
//
// ****************************************************************************

void
avtConnComponentsExpression::SpatialPartition::CreatePartition
(const BoundarySet &bset, double *bounds)
{
    int   i, j, k;
    int t0 = visitTimer->StartTimer();

    if (itree != NULL)
        delete itree;

    //
    // Here's the gameplan:
    // We are going to start off with a single "boundary".  Ultimately, we
    // are going to want to have N boundaries, where N is the number of
    // processors.  So we tell this initial boundary that it represents N
    // processors.  Then we tell it to choose some pivots that it thinks
    // might allow itself to split into two boundaries, each with half the 
    // amount of work and each representing half the number of processor. 
    // Now we have two boundaries, and we keep splitting them (across 
    // different axes) until we get N boundaries, where each one represents
    // a single processor.
    //
    // Once we do that, we can construct an interval tree of the boundaries,
    // which represents our spatial partitioning.
    //
    bool is2D = (bounds[4] == bounds[5]);
    PartitionBoundary::SetIs2D(is2D);
    int nProcs = PAR_Size();
    PartitionBoundary **b_list = new PartitionBoundary*[2*nProcs];
    float fbounds[6];
    fbounds[0] = bounds[0];
    fbounds[1] = bounds[1];
    fbounds[2] = bounds[2];
    fbounds[3] = bounds[3];
    fbounds[4] = bounds[4];
    fbounds[5] = bounds[5];
    if (is2D)
    {
        fbounds[4] -= 1.;
        fbounds[5] += 1.;
    }
    b_list[0] = new PartitionBoundary(fbounds, nProcs, X_AXIS);
    int listSize = 1;
    int *bin_lookup = new int[2*nProcs];
    bool keepGoing = (nProcs > 1);
    while (keepGoing)
    {
        // Figure out how many boundaries need to keep going.
        int nBins = 0;
        for (i = 0 ; i < listSize ; i++)
            if (!(b_list[i]->IsDone()))
            {
                bin_lookup[nBins] = i;
                nBins++;
            }

        // Construct an interval tree out of the boundaries.  We need this
        // because we want to be able to quickly determine which boundaries
        // a point falls in.
        avtIntervalTree it = avtIntervalTree(nBins, 3);
        nBins = 0;
        for (i = 0 ; i < listSize ; i++)
        {
            if (b_list[i]->IsDone())
                continue;
            float *b = b_list[i]->GetBoundary();
            double db[6] = {b[0], b[1], b[2], b[3], b[4], b[5]};
            it.AddElement(nBins, db);
            nBins++;
        }
        it.Calculate(true);

        // Now add each point to the boundary it falls in.  Start by doing
        // the points that come from unstructured or structured meshes.
        // Now do the cells.  We are using the cell centers, which is a decent
        // approximation.
        vector<int> list;
        vector<vtkDataSet *> meshes = bset.GetMeshes();
        for (i = 0 ; i < meshes.size() ; i++)
        {
            const int ncells = meshes[i]->GetNumberOfCells();
            double bbox[6];
            double pt[3];
            for (j = 0 ; j < ncells ; j++)
            {
                vtkCell *cell = meshes[i]->GetCell(j);
                cell->GetBounds(bbox);
                pt[0] = (bbox[0] + bbox[1]) / 2.;
                pt[1] = (bbox[2] + bbox[3]) / 2.;
                pt[2] = (bbox[4] + bbox[5]) / 2.;
                it.GetElementsListFromRange(pt, pt, list);
                float fpt[3] = {pt[0], pt[1], pt[2]};
                for (k = 0 ; k < list.size() ; k++)
                {
                    PartitionBoundary *b = b_list[bin_lookup[list[k]]];
                    b->AddPoint(fpt);
                }
            }
        }

        // See which boundaries found a suitable pivot and can now split.
        PartitionBoundary::PrepareSplitQuery(b_list, listSize);
        int numAtStartOfLoop = listSize;
        for (i = 0 ; i < numAtStartOfLoop ; i++)
        {
            if (b_list[i]->IsDone())
                continue;
            PartitionBoundary *b1, *b2;
            if (b_list[i]->AttemptSplit(b1, b2))
            {
                b_list[listSize++] = b1;
                b_list[listSize++] = b2;
            }
        }

        // See if there are any boundaries that need more processing.  
        // Obviously, all the boundaries that were just split need more 
        // processing, because they haven't done any yet.
        keepGoing = false;
        for (i = 0 ; i < listSize ; i++)
            if (!(b_list[i]->IsDone()))
                keepGoing = true;
    }

    // Construct an interval tree out of the boundaries.  This interval tree
    // contains the actual spatial partitioning.
    itree = new avtIntervalTree(nProcs, 3);
    int count = 0;
    for (i = 0 ; i < listSize ; i++)
    {
        if (b_list[i]->IsLeaf())
        {
            float *b = b_list[i]->GetBoundary();
            double db[6] = {b[0], b[1], b[2], b[3], b[4], b[5]};
            itree->AddElement(count++, db);
        }
    }

    // create final interval tree
    itree->Calculate(true);

    // Clean up.
    for (i = 0 ; i < listSize ; i++)
        delete b_list[i];
    delete [] b_list;
    delete [] bin_lookup;

    visitTimer->StopTimer(t0, "Creating spatial partition");
}

// ****************************************************************************
//  Method: SpatialPartition::GetProcessorList
//
//  Purpose:
//      Gets the processor that contains this cell.  This should be called
//      when a list of processors contain a cell.
//
// Notes: Adapted from Hank Child's avtPosCMFEAlgorithm::SpatialPartition
//
//
//  Programmer: Cyrus Harrison
//  Creation:   Feburary 2, 2005
//
// ****************************************************************************

void
avtConnComponentsExpression::SpatialPartition::GetProcessorList(vtkCell *cell,
                                                  std::vector<int> &list) const
{
    list.clear();

    double bounds[6];
    cell->GetBounds(bounds);
    double mins[3];
    mins[0] = bounds[0];
    mins[1] = bounds[2];
    mins[2] = bounds[4];
    double maxs[3];
    maxs[0] = bounds[1];
    maxs[1] = bounds[3];
    maxs[2] = bounds[5];

    itree->GetElementsListFromRange(mins, maxs, list);
}

