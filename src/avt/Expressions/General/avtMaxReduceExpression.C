// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtMaxReduceExpression.C                        //
// ************************************************************************* //
#include <avtMaxReduceExpression.h>

#include <math.h>

#include <avtDatabaseMetaData.h>
#include <avtExprNode.h>
#include <avtExpressionEvaluatorFilter.h>
#include <avtFacelistFilter.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtParallel.h>
#include <avtOriginatingSource.h>

#include <vtkAppendFilter.h>
#include <vtkCharArray.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridRelevantPointsFilter.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkVisItUtility.h>

#include <sstream>
#include <DebugStream.h>
#include <ExpressionException.h>
#include <TimingsManager.h>
#include <Utility.h>


#ifdef PARALLEL
  #include <mpi.h>
#endif

#include <string>
#include <vector>

// ****************************************************************************
//  Method: avtMaxReduceExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************

avtMaxReduceExpression::avtMaxReduceExpression()
{
    nFinalMax = 0;
    enableGhostNeighbors = 0;
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtMaxReduceExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   TODO
//
// ****************************************************************************

avtMaxReduceExpression::~avtMaxReduceExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMaxReduceExpression::ProcessArguments
//
//  Purpose:
//      Parses optional arguments.
//
//  Arguments:
//      args      Expression arguments
//      state     Expression pipeline state
//
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modificaions:
//
// ****************************************************************************
void
avtMaxReduceExpression::ProcessArguments(ArgsExpr *args,
                                         ExprPipelineState *state)
{
    // get the argument list and # of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    size_t nargs = arguments->size();

    // check for call with wrong args
    if (nargs != 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "max_reduce() Incorrect syntax.\n"
                   " usage: max_reduce(expr)");
    }

    // first argument is the var name, let it do its own magic
    ArgExpr *first_arg = (*arguments)[0];
    avtExprNode *first_tree = dynamic_cast<avtExprNode*>(first_arg->GetExpr());
    first_tree->CreateFilters(state);
}


// ****************************************************************************
//  Method: avtMaxReduceExpression::GetFinalMax
//
//  Purpose:
//      After expression execution returns the final number of components.
//
//  Programmer: Justin Privitera
//  Creation:   TODO
//
// ****************************************************************************
int 
avtMaxReduceExpression::GetFinalMax()
{
    return nFinalMax;
}


// ****************************************************************************
//  Method: avtMaxReduceExpression::Execute
//
//  Purpose:
//      Labels the connected components of an unstructured mesh.
//
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************
void
avtMaxReduceExpression::Execute()
{
    int t_full = visitTimer->StartTimer();

    // initialize final max to zero 
    nFinalMax = 0;

    // get input data tree to obtain datasets
    avtDataTree_p tree = GetInputDataTree();
    // holds number of datasets
    int nsets;

    // get datasets
    vtkDataSet **data_sets = tree->GetAllLeaves(nsets);

    // get dataset domain ids
    std::vector<int> domain_ids;
    tree->GetAllDomainIds(domain_ids);

    // check for ghosts
    bool have_ghosts = false;
    if(nsets > 0)
        have_ghosts = data_sets[0]->GetCellData()->GetArray("avtGhostZones");

    // set progress related vars
#ifdef PARALLEL
    totalSteps = nsets *4;
    if(have_ghosts)
        totalSteps+= nsets;
#else
    totalSteps = nsets *2;
#endif
    currentProgress = 0;

    ///////////////////////////super important

    int t_gzrm = visitTimer->StartTimer();
    // filter out any ghost cells
    vtkDataSetRemoveGhostCells **ghost_filters = NULL;

    if(have_ghosts)
    {
        ghost_filters = new vtkDataSetRemoveGhostCells*[nsets];

        for(int i = 0 ; i < nsets ; i++)
        {
            ghost_filters[i] = vtkDataSetRemoveGhostCells::New();
            ghost_filters[i]->SetInputData(data_sets[i]);
            ghost_filters[i]->Update();
            data_sets[i] = ghost_filters[i]->GetOutput();
        }
    }
    visitTimer->StopTimer(t_gzrm,"Ghost Zone Removal");

    ///////////////////////////end super important

    int t_local_lbl = visitTimer->StartTimer();
    // array to hold output sets
    avtDataTree_p *leaves = new avtDataTree_p[nsets];

    // vectors to hold result sets and their component labels
    std::vector<vtkDataSet *>  result_sets;
    std::vector<vtkIntArray *> result_arrays;
    // vector to hold the number of components per set
    std::vector<int> results_num_comps;

    result_sets.resize(nsets);
    result_arrays.resize(nsets);
    results_num_comps.resize(nsets);

    int num_local_comps=0;
    int num_comps = 0;
    int num_local_cells=0;

    for (int i = 0; i < nsets; i ++)
    {
        bool cell_centered

        // get current set
        vtkDataSet *curr_set = data_sets[i];
        const int num_local_cells = curr_set->GetNumberOfCells();
    }

    // process all local sets
    for(int i = 0; i < nsets ; i++)
    {
        // get current set
        vtkDataSet *curr_set = data_sets[i];
        num_local_cells += curr_set->GetNumberOfCells();

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

        if(res_array->GetNumberOfTuples() > 0) // add result as new leaf
            leaves[i] = new avtDataTree(res_set,domain_ids[i]);
        else // if the dataset only contained ghost zones we could end up here
            leaves[i] = NULL;

        // update progress
        UpdateProgress(currentProgress++,totalSteps);
    }

    // create a boundary set 
    // this is used to for fast boundary queries to resolve components across
    // multiple datasets

    BoundarySet bset;
    for(int i=0;i<nsets;i++)
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

    std::ostringstream oss;
    oss << "Connected Components Labeling of " << nsets << " local datasets (" 
        << num_local_cells << " cells, " << num_comps << " comps)";
    visitTimer->StopTimer(t_local_lbl,oss.str());

#ifdef PARALLEL

    //
    // At this point each processor has resolved the labels across all local 
    // datasets.  Components on each processor are labeled 0 <-> (number of 
    // local comps - 1).  In the parallel case we need to ensure that every 
    // component has a unique label so we first perform a global label shift.
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
    for(int i = 0; i< nsets; i++)
    {
       // dec ref pointer for each set
       result_sets[i]->Delete();
       // dec ref pointer for each set's label array
       result_arrays[i]->Delete();
       // cleanup ghost filters
       if(have_ghosts)
           ghost_filters[i]->Delete();
    }
    // cleanup ghost filters array
    if(have_ghosts)
        delete [] ghost_filters;

    // Set progress to complete
    UpdateProgress(totalSteps,totalSteps);

    // set the final number of components
    nFinalMax  = num_comps;

    visitTimer->StopTimer(t_full,"Full Connected Components Labeling");
}


// ****************************************************************************
//  Method: avtMaxReduceExpression::CheckForProperGhostZones
//
//  Purpose:
//     Checks for ghost zone info that can be used in to reduce parallel 
//     communication. 
//
//  Arguments:
//    sets        Input data sets.
//    nsets       Number of input data sets. 
//
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************
bool
avtMaxReduceExpression::CheckForProperGhostZones(vtkDataSet **sets,
                                                      int nsets)
{
    int t0 = visitTimer->StartTimer();

    int found_ghosts = 0;
    int total_ncells = 0;
    for(int i=0; i < nsets && found_ghosts == 0; i++)
    {
        int ncells = sets[i]->GetNumberOfCells();
        total_ncells += ncells;
        vtkUnsignedCharArray *gz_array = (vtkUnsignedCharArray *) sets[i]
                                    ->GetCellData()->GetArray("avtGhostZones");
        if(gz_array)
        {
            unsigned char *gz_ptr = (unsigned char *)gz_array->GetPointer(0);
            for(int j=0; j < ncells && found_ghosts == 0; j++)
            {
                if(gz_ptr[j] & 1) // Bit 0 == DUPLICATED_ZONE_INTERNAL_TO_PROBLEM
                    found_ghosts = 1;
            }
        }
    }

    //
    // If we found a single instance of a proper ghost zone
    // we want to try to use the ghost zone neighbors optimization.
    // Note: It would be better if the data attributes simply told
    // us that the proper type of ghost zones were generated ...
    //

    found_ghosts = UnifyMaximumValue(found_ghosts);

    visitTimer->StopTimer(t0,"Check For Proper Ghost Zones");
    return (found_ghosts == 1);
}

// ****************************************************************************
//  Method: avtMaxReduceExpression::LabelGhostNeighbors
//
//  Purpose:
//     Identifies cells that have ghost neighbors, storing the info in
//     a vtkUnsignedCharArray named "avtOnBoundary".
//
//  Arguments:
//    data_set     Input mesh
//
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
// ****************************************************************************
void
avtMaxReduceExpression::LabelGhostNeighbors(vtkDataSet *data_set)
{
    int t0 = visitTimer->StartTimer();
    // loop indices
    int i,j,k;
    vtkUnsignedCharArray *gz_array = (vtkUnsignedCharArray *) data_set
                             ->GetCellData()->GetArray("avtGhostZones");

    // if the data set does not have ghosts, we are done
    if (!gz_array)
        return;

    unsigned char *gz_ptr = (unsigned char *)gz_array->GetPointer(0);
    int ncells = data_set->GetNumberOfCells();

    vtkUnsignedCharArray *gzn_array = vtkUnsignedCharArray::New();
    gzn_array->SetName("avtOnBoundary");
    gzn_array->SetNumberOfComponents(1);
    gzn_array->SetNumberOfTuples(ncells);

    unsigned char *gzn_ptr = (unsigned char *)gzn_array->GetPointer(0);

    // init the ghost zone neighbors array
    memset(gzn_ptr,0,ncells * sizeof(unsigned char));

    for ( i=0; i < ncells; i++)
    {
        // if this cell has ghost zones, label it's neighbors
        if(gz_ptr[i] & 1) // Bit 0 == DUPLICATED_ZONE_INTERNAL_TO_PROBLEM
        {
            // get cell neighbors
            vtkIdList *gcell_pts = data_set->GetCell(i)->GetPointIds();
            int ngcell_pts = gcell_pts->GetNumberOfIds();
            for( j=0; j < ngcell_pts; j++)
            {
                // neighbors share points with the current cell
                vtkIdList *gpt = vtkIdList::New();
                gpt->SetNumberOfIds(1);
                gpt->SetId(0,gcell_pts->GetId(j));
                vtkIdList *nei_cells = vtkIdList::New();
                data_set->GetCellNeighbors(i,gpt,nei_cells);
                int nnei = nei_cells->GetNumberOfIds();

                // tag neighbors
                for ( k = 0; k < nnei; k++)
                    gzn_ptr[nei_cells->GetId(k)] = 1;

                gpt->Delete();
                nei_cells->Delete();
            }
        }
    }

    data_set->GetCellData()->AddArray(gzn_array);
    gzn_array->Delete();
    visitTimer->StopTimer(t0,"Labeling Ghost Neighbors");
}


// ****************************************************************************
//  Method: avtMaxReduceExpression::LabelBoundaryNeighbors
//
//  Purpose:
//     Identifies cells that lie on the boundary, storing the results in 
//     a vtkUnsignedCharArray named "avtOnBoundary".
//
//  Arguments:
//    data_set     Input mesh
//
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************

void
avtMaxReduceExpression::LabelBoundaryNeighbors(vtkDataSet *data_set)
{
    int i;
    int t0 = visitTimer->StartTimer();

    int ncells = data_set->GetNumberOfCells();

    // make a clone of the input that has no variable
    // (less variables mean less operations when manipulating it)
    vtkDataSet *clone_ds = data_set->NewInstance();
    clone_ds->ShallowCopy(data_set);
    int numPointArrays = clone_ds->GetPointData()->GetNumberOfArrays();
    for (i = numPointArrays-1 ; i>=0 ; i--)
        clone_ds->GetPointData()->RemoveArray(i);
    int numCellArrays = clone_ds->GetCellData()->GetNumberOfArrays();
    for (i = numCellArrays-1 ; i>=0 ; i--)
        clone_ds->GetCellData()->RemoveArray(i);

    // set up a variable that has the cell ID for each cell.
    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(ncells);
    for (vtkIdType i = 0 ; i < ncells ; i++)
        arr->SetValue(i, (int)i);
    const char *varname = "_avt_id";
    arr->SetName(varname);
    clone_ds->GetCellData()->AddArray(arr);
    arr->Delete();

    // use external routine to find which cells are external
    avtFacelistFilter *flf = new avtFacelistFilter();
    avtDataRepresentation clone_dr(clone_ds, -1, "");
    avtDataTree_p tree = flf->FindFaces(&clone_dr,
                                  GetInput()->GetInfo(), false, false,
                                  true, true, NULL);
    delete flf;
    clone_ds->Delete();
    // we do not need to delete tree, since it is a ref_ptr

    // init the boundary neighbors array
    vtkUnsignedCharArray *b_array = vtkUnsignedCharArray::New();
    b_array->SetName("avtOnBoundary");
    b_array->SetNumberOfComponents(1);
    b_array->SetNumberOfTuples(ncells);
    unsigned char *b_ptr = (unsigned char *)b_array->GetPointer(0);
    memset(b_ptr,0,ncells * sizeof(unsigned char));

    // go through external cells and update array for which are on boundary
    vtkDataSet *just_exteriors = tree->GetSingleLeaf();
    vtkIntArray *outsides = (vtkIntArray *) just_exteriors->GetCellData()->GetArray(varname);
    int numOutsideCells = outsides->GetNumberOfTuples();
    for (i = 0 ; i < numOutsideCells ; i++)
        b_ptr[outsides->GetValue(i)] = 1;

    data_set->GetCellData()->AddArray(b_array);
    b_array->Delete();

    visitTimer->StopTimer(t0,"Labeling Boundary Neighbors");
}


// ****************************************************************************
//  Method: avtMaxReduceExpression::SingleSetLabel
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
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************
vtkIntArray *
avtMaxReduceExpression::SingleSetLabel(vtkDataSet *data_set,
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

    int *res_ptr = res_array->GetPointer(0);

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

    int t_uf_gen = visitTimer->StartTimer();
    // create a UnionFind object with # of entries  = # of points
    UnionFind union_find(npoints,false);

    std::ostringstream oss;
    oss << "Single Set UnionFind Generate (" << npoints << " entries)";
    visitTimer->StopTimer(t_uf_gen,oss.str());
    oss.str("");

    int t_uf_sweep = visitTimer->StartTimer();

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
            // get the point ids
            int j_id = ids->GetId(j);

            if (nids == 1)
            {
                union_find.SetValid(j_id, true);
            }
            else
            {
                for( k=0; k < j; k++)
                {
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
    }

    oss << "Single Set UnionFind Sweep (" << ncells << " cells)";
    visitTimer->StopTimer(t_uf_sweep,oss.str());
    oss.str("");

    int tfz = visitTimer->StartTimer();
    // flatten to resolve final labels
    num_comps = union_find.FinalizeLabels();
    visitTimer->StopTimer(tfz,"Single Set Label Finalize Labels");
    // if we only found a single component, make sure to increment
    // the number of components
    if(ncells > 0 && num_comps == 0 ) num_comps++;

    // use union find label to set final output value
    for( i = 0; i < ncells; i++)
    {
        // get current cell
        vtkCell *cell = data_set->GetCell(i);
        // get label by finding the label of the first point in the cell
        // (any point would suffice)
        // place result in array
        res_ptr[i] = union_find.GetFinalLabel(cell->GetPointId(0));
    }

    oss << "Single Set Connected Components Labeling (" << ncells << " cells, " << num_comps << " comps)";
    visitTimer->StopTimer(t0,oss.str());

    return res_array;
}


// ****************************************************************************
//  Method: avtMaxReduceExpression::MultiSetResolve
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
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************
int
avtMaxReduceExpression::MultiSetResolve(int num_comps,
                                            const BoundarySet &bset,
                                            const std::vector<vtkDataSet*> &sets,
                                            const std::vector<vtkIntArray*> &labels)
{
    // loop indices
    int i,j;

    // find out the number of sets
    int nsets = (int)sets.size();

    // if we have 0 or 1 set(s) multi-set resolve is not necessary
    if(nsets < 2) return num_comps; // not multi-set!

    int t0 = visitTimer->StartTimer();

    // create union find for the multi-set resolve
    // in this case, all union find representatives are valid
    UnionFind union_find(num_comps,true);

    // vectors to hold boundary query results
    std::vector<int> src_cells;
    std::vector<int> can_cells;
    int src_label, can_label;

    int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    avtIntervalTree tree(nsets, dim, false);
    for (i = 0 ; i < nsets ; i++)
    {
        double bounds[6];
        sets[i]->GetBounds(bounds);
        tree.AddElement(i, bounds);
    }
    tree.Calculate(true);

    // loop over all sets
    for( i = 0; i < nsets; i++)
    {
        std::vector<int> possible_matches;
        double bounds[6];
        sets[i]->GetBounds(bounds);
        double lower[3];
        double upper[3];
        lower[0] = bounds[0];
        lower[1] = bounds[2];
        lower[2] = bounds[4];
        upper[0] = bounds[1];
        upper[1] = bounds[3];
        upper[2] = bounds[5];
        tree.GetElementsListFromRange(lower, upper, possible_matches);

        vtkIntArray  *src_labels   = labels[i];

        // get intersection between set i and possible matches
        for (size_t m = 0 ; m < possible_matches.size() ; m++)
        {
            j = possible_matches[m];
            // self intersection test not necessary
            if ( j == i )
                continue;

            vtkIntArray  *can_labels   = labels[j]; 
            bset.GetIntersectionSet(i,j,src_cells,can_cells);
            
            size_t nisect = src_cells.size();
            // check if these cell labels need to be merged. 
            for(size_t k = 0; k < nisect; k++)
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
        vtkIdType          ncells     = curr_array->GetNumberOfTuples();

        for(vtkIdType jj = 0; jj < ncells; ++jj)
        {
            // get the current label value
            int lbl = curr_array->GetValue(jj);
            // find the final label value
            lbl = union_find.GetFinalLabel(lbl);
            // set the final label 
            curr_array->SetValue(jj,lbl);
        }
    }

    visitTimer->StopTimer(t0,"Multi-Set Component Label Resolve");

    // return the final number of components
    return num_comps;
}


// ****************************************************************************
//  Method: avtMaxReduceExpression::MultiSetList
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
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************
void
avtMaxReduceExpression::MultiSetList(int num_comps,
                                         const BoundarySet &bset,
                                         const std::vector<vtkDataSet*> &sets,
                                         const std::vector<vtkIntArray*> &labels,
                                         std::vector<int> &u_src,
                                         std::vector<int> &u_des)
{
    // loop indices
    int i,j;

    // find out the number of sets
    int nsets = (int)sets.size();
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
    std::vector<int> src_cells;
    std::vector<int> can_cells;
    int src_label, can_label;

    int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    avtIntervalTree tree(nsets, dim, false);
    for (i = 0 ; i < nsets ; i++)
    {
        double bounds[6];
        sets[i]->GetBounds(bounds);
        tree.AddElement(i, bounds);
    }
    tree.Calculate(true);

    // loop over all sets
    for( i = 0; i < nsets; i++)
    {
        std::vector<int> possible_matches;
        double bounds[6];
        sets[i]->GetBounds(bounds);
        double lower[3];
        double upper[3];
        lower[0] = bounds[0];
        lower[1] = bounds[2];
        lower[2] = bounds[4];
        upper[0] = bounds[1];
        upper[1] = bounds[3];
        upper[2] = bounds[5];
        tree.GetElementsListFromRange(lower, upper, possible_matches);

        vtkIntArray  *src_labels   = labels[i];

        // get intersection between set i and possible matches
        for (size_t m = 0 ; m < possible_matches.size() ; m++)
        {
            j = possible_matches[m];
            // self intersection test not necessary
            if ( j == i )
                continue;

            vtkIntArray  *can_labels   = labels[j]; 
            bset.GetIntersectionSet(i,j,src_cells,can_cells);

            size_t nisect = src_cells.size();
            // check if these cell labels need to be merged. 
            for(size_t k = 0; k < nisect; k++)
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
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************
int
avtMaxReduceExpression::GlobalLabelShift
(int num_local_comps, const std::vector<vtkIntArray*> &labels)
{
    // in the serial case the is no shift and the number of labels 
    // is unchanged
    int num_comps = num_local_comps;

#ifdef PARALLEL
    // get the processor id and # of processors
    int procid = PAR_Rank();
    int nprocs = PAR_Size();

    if (visitTimer->Enabled())
    {
        int tb = visitTimer->StartTimer();
        Barrier();
        visitTimer->StopTimer(tb, "Waiting for all processors to enter new stage.");
    }

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
    for(int i=0;i<nprocs;i++)
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
    size_t nsets = labels.size();
    for(size_t i = 0; i < nsets ;i++)
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
//  Method: avtMaxReduceExpression::GlobalResolve
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
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************
int
avtMaxReduceExpression::GlobalResolve(int num_comps,
                                      BoundarySet &bset,
                                      const std::vector<vtkDataSet*> &local_sets,
                                      const std::vector<vtkIntArray*> &local_labels)
{
#ifdef PARALLEL
    int t0 = visitTimer->StartTimer();

    // The spatial partition is used to evenly distributed mesh data
    // across available processors 
    SpatialPartition     spart;

    // vectors used to access the relocated datasets and their label arrays
    std::vector<vtkDataSet*>  sets;
    std::vector<vtkIntArray*> labels;

    // To create the spatial partition, we first need to know the bounds of 
    // the entire dataset.

    double bounds[6];
    // get the bounds of the local sets
    bset.GetBounds(bounds);

    // unify to get the bounds of the entire dataset
    UnifyMinMax(bounds,6);

    // create the spatial partition
    int tp  = visitTimer->StartTimer();
    spart.CreatePartition(bset,bounds);
    visitTimer->StopTimer(tp, "Creating spatial partition");

    // Relocate proper cells using boundary set
    int t1 = visitTimer->StartTimer();
    bset.RelocateUsingPartition(spart,outputVariableName);
    visitTimer->StopTimer(t1, "Relocating using spatial partition (communication)");

    // get the relocated datasets
    sets = bset.GetMeshes();
    size_t n_reloc_sets = sets.size();

    // get the component label arrays
    labels.resize(n_reloc_sets);
    for(size_t i=0; i < n_reloc_sets; i++)
    {
        labels[i] = (vtkIntArray*)sets[i]->GetCellData()->
                                       GetArray(outputVariableName);
    }

    // list the union operations required to resolve the relocated data
    std::vector<int> union_src, union_des;
    MultiSetList(num_comps,bset, sets, labels,  union_src,union_des);

    // perform global union using the listed union operations
    num_comps = GlobalUnion(num_comps,union_src,union_des,local_labels);

    visitTimer->StopTimer(t0,"Global Label Resolve");
#endif
    // return the final number of components
    return num_comps;
}


// ****************************************************************************
//  Method: avtMaxReduceExpression::GlobalUnion
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
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************
int
avtMaxReduceExpression::GlobalUnion(int num_comps,
                                      const std::vector<int> &u_src,
                                      const std::vector<int> &u_des,
                                      const std::vector<vtkIntArray*> &local_labels)
{
#ifdef PARALLEL
    // loop indices
    int i,j;

    int t0 = visitTimer->StartTimer();

    // get the number of processors and the current processor id
    int nprocs = PAR_Size();

    // create a union find data structure for resolving the labels
    // (in this case all union representatives are valid)
    UnionFind union_find(num_comps,true);

    // get the number of local union operations
    int n_local_unions =(int) u_src.size();

    // pack the union operation label ids into an array
    int msg_size = n_local_unions*2;

    int *snd_msg = new int[msg_size];
    int *ptr = snd_msg;

    // perform local unions & prepare union list for other processors
    for( i = 0; i < n_local_unions; i++)
    {
        union_find.Union(u_src[i],u_des[i]);

        ptr[0] = u_src[i];
        ptr[1] = u_des[i];
        ptr+=2;
    }

    int t_rcv_pairs = visitTimer->StartTimer();
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

    std::ostringstream oss;
    oss << "Receive of " << n_rcv_unions << " UnionFind Pairs";
    visitTimer->StopTimer(t_rcv_pairs,oss.str());

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

    int nsets = (int)local_labels.size();
    // use the union find to resolve all local component ids
    for( i = 0; i< nsets;i++)
    {
        // get the current label array and # of cells

        int          ncells     = local_labels[i]->GetNumberOfTuples();
        int         *curr_vals  = local_labels[i]->GetPointer(0);

        for(j = 0; j < ncells; j ++)
        {
            // lookup the final label value & set
            curr_vals[j] = union_find.GetFinalLabel(curr_vals[j]);
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
//  Method: avtMaxReduceExpression::ShiftLabels
//
//  Purpose:
//      Helper to shift label values by a specified amount.
//
//  Arguments:
//    labels     A component label array
//    shift      Amount of shift each label by
//
//  Programmer: Justin Privitera
//  Creation:   TODO
//
//  Modifications:
//
// ****************************************************************************
void
avtMaxReduceExpression::ShiftLabels(vtkIntArray *labels, int shift)
{
    // loop over all tuples
    vtkIdType nlabels = labels->GetNumberOfTuples();
    int *labels_ptr = labels->GetPointer(0);
    for(vtkIdType i = 0; i < nlabels ; i++)
    {
        // get the current label value
        // add the shift amount
        // & set the new label value
        labels_ptr[i] += shift;
    }
}


// ****************************************************************************
//  Method: avtGradientExpression::ModifyContract
//
//  Purpose:
//      Request ghost zones.
//
//  Programmer: Justin Privitera
//  Creation:   TODO
//
// ****************************************************************************
avtContract_p
avtMaxReduceExpression::ModifyContract(avtContract_p in_spec)
{
    avtContract_p spec = 
                            avtExpressionFilter::ModifyContract(in_spec);
    spec->GetDataRequest()->SetDesiredGhostDataType(GHOST_ZONE_DATA);
    return spec;
}

