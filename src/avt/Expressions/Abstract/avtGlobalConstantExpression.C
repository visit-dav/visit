// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtGlobalConstantExpression.C                        //
// ************************************************************************* //
#include <avtGlobalConstantExpression.h>

#include <avtParallel.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>

#include <DebugStream.h>
#include <ExpressionException.h>

// ****************************************************************************
//  Class: intermediateResults
//
//  Purpose:
//      This class contains data arrays that we use to reduce across domains and 
//      processors. It also can track the number of non-ghosted tuples, which is
//      needed by some expressions. It also tracks the number of components and 
//      number of tuples, which are used to set up the output vtkDataArray where
//      results are to be placed at the end of expression execution. This class
//      also holds a pointer to that vtkDataArray.
//      Instances of this class are passed down the call stack for use in 
//      various methods that require different pieces of the stored information.
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
//
// ****************************************************************************
class intermediateResults
{
public:
    // we need a vector to store results per component
    std::vector<double> constant_results;

    // some expressions (global_std_dev and global_variance) require a second per-component result
    std::vector<double> extra_constant_results;

    // sometimes we need to track the number of non-ghosted tuples that we are reducing across
    int                 num_non_ghosted_tuples;

    // we track the number of components, which is also the length of these vectors
    int                 ncomps;

    // we track the number of tuples for setting up our output arrays
    int                 ntuples;

    // the data array where we will place results when we are done
    vtkDataArray       *target_data_array;

    intermediateResults()
        : constant_results(), extra_constant_results(),
          num_non_ghosted_tuples(0), ncomps(0), ntuples(0), 
          target_data_array(nullptr) {}

    ~intermediateResults()
    {
        if (target_data_array)
        {
            target_data_array->Delete();
            target_data_array = nullptr;
        }
    }
};

// ****************************************************************************
//  Method: avtGlobalConstantExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
// ****************************************************************************

avtGlobalConstantExpression::avtGlobalConstantExpression()
{
    // TODO do I need this
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtGlobalConstantExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
// ****************************************************************************

avtGlobalConstantExpression::~avtGlobalConstantExpression()
{

}

// ****************************************************************************
//  Method: avtGlobalConstantExpression::GetLocalNumTuples
//
//  Purpose:
//      Sums the local number of non-ghosted tuples recorded by the 
//      intermediate_results_map.
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
int
avtGlobalConstantExpression::GetLocalNumTuples(
    const std::map<int, intermediateResults> &intermediate_results_map)
{
    int total_ntuples = 0;
    std::for_each(intermediate_results_map.begin(),
                  intermediate_results_map.end(),
                  [&total_ntuples](const auto &pair) { total_ntuples += pair.second.num_non_ghosted_tuples; });
    return total_ntuples;
}

// ****************************************************************************
//  Method: avtGlobalConstantExpression::IdentifyGhostedNodes
//
//  Purpose:
//      This function determines which nodes ought to be used if we are
//      taking ghosts into account. We mark all nodes touching non-ghosted 
//      zones as good to count, and mark all nodes that are ghost nodes
//      as nodes that should not be counted.
// 
//  Returns:
//      A vector that is number of nodes long containing true when the node
//      should be ignored in calculations and false when the node should
//      not be ignored.
//
//  Programmer: Justin Privitera
//  Creation:   10/24/24
//
//  Modifications:
//    Justin Privitera, Tue Oct  7 17:37:07 PDT 2025
//    This method was moved from avtGhostAwareUnaryMathExpression, which was
//    deleted, to avtGlobalConstantExpression.
//    I also changed it to take a reference to a vector of ints instead of
//    returning one.
// ****************************************************************************
void
avtGlobalConstantExpression::IdentifyGhostedNodes(vtkDataSet *in_ds,
                                                  vtkDataArray *ghostZones,
                                                  vtkDataArray *ghostNodes,
                                                  std::vector<int> &nodeShouldBeIgnored)
{
    const int nPoints = in_ds->GetNumberOfPoints();

    // we create an array to track if this point should be counted
    nodeShouldBeIgnored.resize(nPoints);
    if (ghostZones)
    {
        // if there are ghost zones, we want to initialize all points to not being counted
        fill(nodeShouldBeIgnored.begin(), nodeShouldBeIgnored.end(), true);
    }
    else
    {
        // If there are ghost nodes and NOT ghost zones, we want to initialize all points
        // to being counted
        // Alternatively, we will hit this case if there are neither ghost zones nor ghost
        // nodes. In that case, we shouldn't even be in this function, but if we are here
        // we might as well say all the points are valid since they are.
        fill(nodeShouldBeIgnored.begin(), nodeShouldBeIgnored.end(), false);
    }

    if (ghostZones)
    {
        const int nCells = in_ds->GetNumberOfCells();
        // iterate through the cells and mark points that are touching non-ghosts
        // as points that should be counted
        for (int cellId = 0; cellId < nCells; cellId ++)
        {
            // if this zone is not a ghost zone
            if (0 == ghostZones->GetComponent(cellId, 0))
            {
                vtkIdType numCellPoints = 0;
                const vtkIdType *cellPoints = nullptr;
                vtkIdList *ptIds = vtkIdList::New();
                // we get the points for this zone
                in_ds->GetCellPoints(cellId, numCellPoints, cellPoints, ptIds);

                // and mark them as valid points
                if (numCellPoints && cellPoints)
                {
                    for (int cellPointId = 0; cellPointId < numCellPoints; cellPointId ++)
                    {
                        const int pointId = cellPoints[cellPointId];
                        nodeShouldBeIgnored[pointId] = false;
                    }
                }
                ptIds->Delete();
            }
        }
    }

    if (ghostNodes)
    {
        // iterate through all points and make sure points marked as ghost
        // nodes are not counted
        for (int pointId = 0; pointId < nPoints; pointId ++)
        {
            // if this node is a ghost node
            if (0 != ghostNodes->GetComponent(pointId, 0))
            {
                nodeShouldBeIgnored[pointId] = true;
            }
        }
    }
}

// ****************************************************************************
//  Method: avtGlobalConstantExpression::DoOperation
//
//  Purpose:
//      All ghost-aware unary expressions have the same underlying logic, which
//      we provide here. If we are operating on zonal data, we need to check
//      for ghost zones and only calculate using the non-ghosted zones. If we 
//      are operating on nodal data, we need to use IdentifyGhostedNodes() to
//      determine which nodes we can use. We then call CalculateWithGhosts() or
//      CalculateWithoutGhosts() which are defined in classes that inherit from
//      avtGhostAwareUnaryMathExpression. CalculateWithoutGhosts() is an 
//      optimized path that we can use when there are no ghosts we need to 
//      worry about.
//
//  Arguments:
//      inputArray             The input data array.
//      ncomponents            The number of components ('1' for scalar, '2' or 
//                             '3' for vectors, etc.)
//      ntuples                The number of tuples (i.e. 'npoints' or 'ncells')
//      in_ds                  The input dataset.
//      constant_results       An array where we can store intermediate data to
//                             be reduced locally and globally.
//      extra_constant_results An additional array where we can store
//                             intermediate data to be reduced locally and
//                             globally.
//      num_non_ghosted_tuples The number of non-ghosted tuples, to be set here
//                             and passed back up the call-chain for use 
//                             elsewhere.
//
//  Programmer: Justin Privitera
//  Creation:   09/30/24
//
//  Modifications:
//    Justin Privitera, Tue Oct  7 17:37:07 PDT 2025
//    This method was moved from avtGhostAwareUnaryMathExpression, which was
//    deleted, to avtGlobalConstantExpression.
//    I removed the output vtkDataArray as we no longer write to the output in
//    this method. I added constant_results and extra_constant_results 
//    arguments, which are data arrays where we can store computed data that
//    will then be reduced later and used to calculate the final result of the 
//    expression. I also added num_non_ghosted_tuples, which is set here and 
//    used up the callstack if requested.
// ****************************************************************************
void
avtGlobalConstantExpression::DoOperation(vtkDataArray *inputArray,
                                         const int ncomponents,
                                         const int ntuples,
                                         vtkDataSet *in_ds,
                                         std::vector<double> &constant_results,
                                         std::vector<double> &extra_constant_results,
                                         int &num_non_ghosted_tuples)
{
    // TODO we need to error here if there is no non-ghosted data
    // pull the error (and the error includes) out of the child classes

    vtkDataArray *ghostZones = in_ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *ghostNodes = in_ds->GetPointData()->GetArray("avtGhostNodes");

    if (AVT_ZONECENT == centering)
    {
        if (ghostZones)
        {
            if (NeedsNTuples())
            {
                num_non_ghosted_tuples = [&]() -> int 
                {
                    int running_sum = 0;
                    for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
                    {
                        // 0 means it is NOT ghosted
                        if (0 == ghostZones->GetComponent(tuple_id, 0))
                        {
                            running_sum ++;
                        }
                    }
                    return running_sum;
                }();
            }

            // we pass a lambda to CalculateWithGhosts() that
            // looks at the ghostZones to determine if a cell
            // is valid and ignores the nodeShouldBeIgnoredPtr.
            CalculateWithGhosts(inputArray, ncomponents, ntuples,
                                [](vtkDataArray *ghostZones,
                                   int *nodeShouldBeIgnoredPtr,
                                   int tuple_id) -> int 
                                   { return ghostZones->GetComponent(tuple_id, 0); },
                                ghostZones,
                                nullptr,
                                constant_results,
                                extra_constant_results);
        }
        else // no ghosts or just ghost nodes
        {
            if (NeedsNTuples())
            {
                num_non_ghosted_tuples = ntuples;
            }

            CalculateWithoutGhosts(inputArray, ncomponents, ntuples, 
                                   constant_results, extra_constant_results);
        }
    }
    else // AVT_NODECENT == centering
    {
        // if we have any kind of ghosts
        if (ghostZones || ghostNodes)
        {
            // we need to identify which nodes should be ignored
            std::vector<int> nodeShouldBeIgnored;
            IdentifyGhostedNodes(in_ds, ghostZones, ghostNodes, nodeShouldBeIgnored);

            if (NeedsNTuples())
            {
                num_non_ghosted_tuples = std::count(nodeShouldBeIgnored.begin(),
                                                    nodeShouldBeIgnored.end(),
                                                    0); // 0 means it is NOT ghosted
            }

            // we pass a lambda to CalculateWithGhosts() that
            // looks at the nodeShouldBeIgnoredPtr to determine 
            // if a node is valid and ignores the ghostZones.
            CalculateWithGhosts(inputArray, ncomponents, ntuples,
                                [](vtkDataArray *ghostZones,
                                   int *nodeShouldBeIgnoredPtr,
                                   int tuple_id) -> int 
                                   { return nodeShouldBeIgnoredPtr[tuple_id]; },
                                ghostZones,
                                nodeShouldBeIgnored.data(),
                                constant_results,
                                extra_constant_results);
        }
        else // no ghosts
        {
            if (NeedsNTuples())
            {
                num_non_ghosted_tuples = ntuples;
            }

            CalculateWithoutGhosts(inputArray, ncomponents, ntuples,
                                   constant_results, extra_constant_results);
        }
    }
}


// ****************************************************************************
//  Method: avtGlobalConstantExpression::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      void
//
//  Programmer:   Sean Ahern
//  Creation:     Wed Jun 12 16:44:28 PDT 2002
//
//  Notes:
//      Sean Ahern, Fri Jun 14 11:52:33 PDT 2002
//      Since the centering that's stored in
//      GetInput()->GetInfo().GetAttributes().GetCentering() is not on a
//      per-variable basis, we can't rely on it for the centering
//      information.  Instead, get the scalars from the point and cell
//      data.  Whichever one is non-NULL is the one we want.

//      Justin Privitera, Tue Oct  7 17:37:07 PDT 2025
//      This method was borrowed from avtUnaryMathExpression and then modified.
//
//  Modifications:
//    Justin Privitera, Tue Oct  7 17:37:07 PDT 2025
//    We no longer need to pass the currentDomainsIndex so it was removed as an
//    argument.
//    This method now has a void return type and stores results in the 
//    per_leaf_results intermediateResults object that is passed in.
// ****************************************************************************
void
avtGlobalConstantExpression::DeriveVariable(vtkDataSet *in_ds,
                                            intermediateResults &per_leaf_results)
{
    vtkDataArray *cell_data = nullptr;
    vtkDataArray *point_data = nullptr;
    vtkDataArray *data = nullptr;

    if (activeVariable == nullptr)
    {
        //
        // This hack is getting more and more refined.  This situation comes up
        // when we don't know what the active variable is (mostly for the
        // constant creation filter).  We probably need more infrastructure
        // to handle this.
        // Iteration 1 of this hack said take any array.
        // Iteration 2 said take any array that isn't vtkGhostLevels, etc.
        // Iteration 3 says take the first scalar array if one is available,
        //             provided that array is not vtkGhostLevels, etc.
        //             This is because most constants we create are scalar.
        //
        // Note: this hack used to be quite important because we would use
        // the resulting array to determine the centering of the variable.
        // Now we use the IsPointVariable() method.  So this data array is
        // only used to get the type.
        //
        const int ncellArray = in_ds->GetCellData()->GetNumberOfArrays();
        for (int i = 0 ; i < ncellArray ; i++)
        {
            vtkDataArray *candidate = in_ds->GetCellData()->GetArray(i);
            if (strstr(candidate->GetName(), "vtk") != NULL)
                continue;
            if (strstr(candidate->GetName(), "avt") != NULL)
                continue;
            if (candidate->GetNumberOfComponents() == 1)
            {
                // Definite winner
                cell_data = candidate;
                break;
            }
            else
                // Potential winner -- keep looking
                cell_data = candidate;
        }
        const int npointArray = in_ds->GetPointData()->GetNumberOfArrays();
        for (int i = 0 ; i < npointArray ; i++)
        {
            vtkDataArray *candidate = in_ds->GetPointData()->GetArray(i);
            if (strstr(candidate->GetName(), "vtk") != NULL)
                continue;
            if (strstr(candidate->GetName(), "avt") != NULL)
                continue;
            if (candidate->GetNumberOfComponents() == 1)
            {
                // Definite winner
                point_data = candidate;
                break;
            }
            else
                // Potential winner -- keep looking
                point_data = candidate;
        }

        if (cell_data != NULL && cell_data->GetNumberOfComponents() == 1)
        {
            data = cell_data;
            centering = AVT_ZONECENT;
        }
        else if (point_data != NULL && point_data->GetNumberOfComponents()== 1)
        {
            data = point_data;
            centering = AVT_NODECENT;
        }
        else if (cell_data != NULL)
        {
            data = cell_data;
            centering = AVT_ZONECENT;
        }
        else
        {
            data = point_data;
            centering = AVT_NODECENT;
        }
    }
    else
    {
        cell_data = in_ds->GetCellData()->GetArray(activeVariable);
        point_data = in_ds->GetPointData()->GetArray(activeVariable);

        if (cell_data != nullptr)
        {
            data = cell_data;
            centering = AVT_ZONECENT;
        }
        else
        {
            data = point_data;
            centering = AVT_NODECENT;
        }
    }

    if (data == nullptr)
    {
        // One way to get here is to have vtkPolyData Curve plots.
        EXCEPTION2(ExpressionException, outputVariableName,
             "An internal error occurred when "
             "trying to calculate your expression.  Please contact a "
             "VisIt developer.");
    }

    int ncomps, ntuples;
    if (activeVariable == nullptr)
    {
        ntuples = (IsPointVariable() ? 
                   in_ds->GetNumberOfPoints() :
                   in_ds->GetNumberOfCells());
    }
    else
    {
        ntuples = data->GetNumberOfTuples();
    }

    ncomps = data->GetNumberOfComponents();

    vtkDataArray *dv = data->NewInstance();
    dv->SetNumberOfComponents(ncomps);
    dv->SetNumberOfTuples(ntuples);

    // we are caching this info so we can easily write to it later
    per_leaf_results.ncomps = ncomps;
    per_leaf_results.ntuples = ntuples;
    per_leaf_results.constant_results.resize(ncomps);
    if (NeedsExtraIntermediateData())
    {
        per_leaf_results.extra_constant_results.resize(ncomps);
    }
    per_leaf_results.target_data_array = dv;

    DoOperation(data,
                ncomps,
                ntuples,
                in_ds,
                per_leaf_results.constant_results,
                per_leaf_results.extra_constant_results,
                per_leaf_results.num_non_ghosted_tuples);
}

// ****************************************************************************
//  Method: avtGlobalConstantExpression::ConstantEvaluation
//
//  Purpose:
//      This method serves as the entry point into calcualting intermediate 
//      results. It traverses the input data tree recursively, numbering the
//      leaves as it goes and added calculated data for them into the 
//      intermediate_results_map based on the unique leaf number.
// 
//  Notes:
//      Justin Privitera, Tue Oct  7 17:37:07 PDT 2025
//      This method was taken from avtSIMODataTreeIterator::Execute and heavily
//      modified.
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
int
avtGlobalConstantExpression::ConstantEvaluation(avtDataTree_p inputDataTree,
                                                std::map<int, intermediateResults> &intermediate_results_map,
                                                int leaf_number)
{
    const int numChildren = inputDataTree->GetNChildren();

    if (numChildren <= 0 && !inputDataTree->HasData())
    {
        return leaf_number;
    }

    if (numChildren == 0)
    {
        avtDataRepresentation *in_dr = &(inputDataTree->GetDataRepresentation());
        vtkDataSet *in_ds = in_dr->GetDataVTK();
        intermediate_results_map.emplace(leaf_number, intermediateResults{});

        //
        // Sometimes we are asked to calculate a variable twice.  The easiest way
        // to catch this is to see if we already have the requested variable and
        // not re-derive it if we do.
        //
        vtkDataArray *&dat = intermediate_results_map.at(leaf_number).target_data_array;
        dat = in_ds->GetPointData()->GetArray(outputVariableName);
        if (dat == nullptr)
        {
            dat = in_ds->GetCellData()->GetArray(outputVariableName);
        }
        
        if (dat != nullptr)
        {
            debug1 << "NOTE: variable " << outputVariableName 
                   << " already exists and it is not being recalculated." << endl;
            dat->Register(nullptr);
        }
        else
        {
            DeriveVariable(in_ds, intermediate_results_map.at(leaf_number));
        }

        UpdateProgress(currentProgress++, totalSteps);

        return leaf_number + 1;
    }
    else
    {
        // there is more than one input dataset to process
        for (int childId = 0; childId < numChildren; childId ++)
        {
            // are children present?
            if (inputDataTree->ChildIsPresent(childId))
            {
                leaf_number = ConstantEvaluation(inputDataTree->GetChild(childId), 
                                                 intermediate_results_map,
                                                 leaf_number);
            }
        }

        return leaf_number;
    }
}

// ****************************************************************************
//  Method: avtGlobalConstantExpression::WriteData_VTK
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      in_dr                   The input data representation.
//      per_leaf_results        An object containing the output vtkDataArray
//                              pointer, the number of components, and the
//                              number of tuples.
//      global_constant_results A vector containing a single global result per
//                              variable component.
//
//  Returns:      The output data representation.
//
//  Programmer:   Hank Childs/Justin Privitera
//  Creation:     June 7, 2002/October 7, 2025
// 
//  Notes:
//      Justin Privitera, Tue Oct  7 17:37:07 PDT 2025
//      Taken from avtExpressionDataTreeIterator::ExecuteData and heavily
//      modified.
//
//  Modifications:
// ****************************************************************************
avtDataRepresentation *
avtGlobalConstantExpression::WriteData_VTK(avtDataRepresentation *in_dr,
                                           intermediateResults &per_leaf_results,
                                           std::vector<double> &global_constant_results)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();

    //
    // Start off by having the derived type calculate the derived variable.
    //
    // the intermediateResults destructor will free this array later 
    vtkDataArray *&dat = per_leaf_results.target_data_array;
    const int ncomps = per_leaf_results.ncomps;
    if (ncomps != 0)
    {
        // this case means that the variable does not already exist and 
        // we calculated it and now must write it
        const int ntuples = per_leaf_results.ntuples;
        for (int comp_id = 0; comp_id < ncomps; comp_id ++)
        {
            const double global_result_for_comp = global_constant_results[comp_id];
            for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
            {
                dat->SetComponent(tuple_id, comp_id, global_result_for_comp);
            }
        }

        dat->SetName(outputVariableName);

        // the omitted else case has been handled before; if this variable
        // already exists then we did not recalculate it, instead, we made
        // our pointer point to the existing variable already.
    }

    const int vardim = dat->GetNumberOfComponents();

    //
    // Now make a copy of the input and add the derived variable as its output.
    //
    vtkDataSet *rv = (vtkDataSet *) in_ds->NewInstance();
    rv->ShallowCopy(in_ds);
    int npts   = rv->GetNumberOfPoints();
    int ncells = rv->GetNumberOfCells();
    int ntups  = dat->GetNumberOfTuples();

    bool isPoint = false;
    if ((ntups == npts) && (ntups == ncells))
    {
        isPoint = IsPointVariable();
    }
    else if (ntups == 1) // Constant singleton.
    {
        isPoint = IsPointVariable();
    }
    else if ((ntups == ncells) && (ntups != npts))
    {
        isPoint = false;
    }
    else if ((ntups == npts) && (ntups != ncells))
    {
        isPoint = true;
    }
    else
    {
        debug1 << "Number of tuples cannot be point or cell variable."
               << endl;
        debug1 << "Var = " << dat->GetName() << endl;
        debug1 << "Ntuples = " << ntups << endl;
        debug1 << "Ncells = " << ncells << endl;
        debug1 << "Npts = " << npts << endl;

        avtDataRepresentation *out_dr = new avtDataRepresentation(rv,
            in_dr->GetDomain(), in_dr->GetLabel());

        rv->Delete();

        return out_dr;
    }

    if (isPoint)
    {
        rv->GetPointData()->AddArray(dat);
        if (vardim == 1)
        {
            rv->GetPointData()->SetActiveScalars(outputVariableName);
        }
        else if (vardim == 3)
        {
            rv->GetPointData()->SetActiveVectors(outputVariableName);
        }
        else if (vardim == 9)
        {
            rv->GetPointData()->SetActiveTensors(outputVariableName);
        }
    }
    else
    {
        rv->GetCellData()->AddArray(dat);
        if (vardim == 1)
        {
            rv->GetCellData()->SetActiveScalars(outputVariableName);
        }
        else if (vardim == 3)
        {
            rv->GetCellData()->SetActiveVectors(outputVariableName);
        }
        else if (vardim == 9)
        {
            rv->GetCellData()->SetActiveTensors(outputVariableName);
        }
    }

    avtDataRepresentation *out_dr = new avtDataRepresentation(rv,
        in_dr->GetDomain(), in_dr->GetLabel());

    rv->Delete();

    return out_dr;
}

// ****************************************************************************
//  Method: avtGlobalConstantExpression::WriteDataTree
//
//  Purpose:
//      Serves as a wrapper for the WriteData_VTK method.
//
//  Arguments:
//      in_dr   The data representation to pass to the derived type.
//      per_leaf_results        An object containing the output vtkDataArray
//                              pointer, the number of components, and the
//                              number of tuples.
//      global_constant_results A vector containing a single global result per
//                              variable component, to be written to the 
//                              output.
//
//  Programmer: Kathleen Bonnell/Justin Privitera
//  Creation:   February 9, 2001/October 7, 2025
// 
//  Notes:
//      Justin Privitera, Tue Oct  7 17:37:07 PDT 2025
//      Taken from avtDataTreeIterator::ExecuteDataTree and heavily
//      modified.
// 
//  Modifications:
// ****************************************************************************
avtDataTree_p
avtGlobalConstantExpression::WriteDataTree(avtDataRepresentation *in_dr,
                                           intermediateResults &per_leaf_results,
                                           std::vector<double> &global_constant_results)
{
    avtDataRepresentation *out_dr = WriteData_VTK(in_dr,
                                                  per_leaf_results,
                                                  global_constant_results);

    if (out_dr == nullptr)
    {
        return nullptr;
    }

    // This code ends up creating a copy of out_dr in the data tree.
    avtDataTree_p retval = new avtDataTree(out_dr);

    // If the derived type created a new avtDataRepresentation instance then
    // it's been copied in the avtDataTree. We need to remove the instance
    // that was returned or we'll end up with a VTK reference count leak.
    if(out_dr != in_dr)
    {
        delete out_dr;
    }

    return retval;
}

// ****************************************************************************
//  Method: avtGlobalConstantExpression::WriteResult
//
//  Purpose:
//    A recursive Execute method.  Walks down the tree and calls the virtual 
//    function ExecuteDataTree on the leaves.
//
//  Arguments:
//      inputDataTree            The tree to traverse.
//      outputDataTree           The new tree we are writing the results to.
//      intermediate_results_map A map containing the, for each leaf number,
//                               an object holding the output vtkDataArray
//                               pointer, the number of components, and the
//                               number of tuples.
//      global_constant_results  A vector containing a single global result per
//                               variable component, to be written to the 
//                               output.
//      leaf_number              The current unique leaf number. We assign a 
//                               unique number to each leaf.
//
//  Programmer:  Kathleen Bonnell/Justin Privitera
//  Creation:    April 12, 2001/October 7, 2025
//
//  Notes:
//      Justin Privitera, Tue Oct  7 17:37:07 PDT 2025
//      Taken from avtSIMODataTreeIterator::Execute and heavily
//      modified.
//
//  Modifications:
// ****************************************************************************
int
avtGlobalConstantExpression::WriteResult(avtDataTree_p inputDataTree, 
                                         avtDataTree_p &outputDataTree,
                                         std::map<int, intermediateResults> &intermediate_results_map,
                                         std::vector<double> &global_constant_results,
                                         int leaf_number)
{
    const int numChildren = inputDataTree->GetNChildren();

    if (numChildren <= 0 && !inputDataTree->HasData())
    {
        return leaf_number;
    }

    if (numChildren == 0)
    {
        outputDataTree = new avtDataTree();
        avtDataRepresentation *in_dr = &(inputDataTree->GetDataRepresentation());
        avtDataTree_p resultDataTree = WriteDataTree(in_dr,
                                                     intermediate_results_map.at(leaf_number),
                                                     global_constant_results);
        if (*resultDataTree)
        {
            outputDataTree = resultDataTree;
        }

        UpdateProgress(currentProgress++, totalSteps);

        return leaf_number + 1;
    }
    else
    {
        //
        // there is more than one input dataset to process
        // and we need an output datatree for each
        //
        avtDataTree_p *localOutputDataTree = new avtDataTree_p[numChildren];
        for (int childId = 0; childId < numChildren; childId ++)
        {
            // are children present?
            if (inputDataTree->ChildIsPresent(childId))
            {
                localOutputDataTree[childId].SetReference( new avtDataTree );
                leaf_number = WriteResult(inputDataTree->GetChild(childId), 
                                          localOutputDataTree[childId],
                                          intermediate_results_map,
                                          global_constant_results,
                                          leaf_number);
            }
            else // adults only
            {
                localOutputDataTree[childId] = nullptr;
            }
        }
        outputDataTree = new avtDataTree(numChildren, localOutputDataTree);
        delete [] localOutputDataTree;

        return leaf_number;
    }
}


// ****************************************************************************
//  Method: avtGlobalConstantExpression::Execute
//
//  Purpose:
//      Calculated a global constant expression result in a several steps:
//      1. Fetch the data tree.
//      2. Calculate local results for each data tree leaf.
//      3. Reduce those results across the local rank.
//      4. Reduce that result across all global ranks.
//      5. Calculate final results with the global data.
//      6. Write those results to a new data tree.
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
// ****************************************************************************
void
avtGlobalConstantExpression::Execute()
{
    // TODO useful error messages
    // TODO parallel error checking

    //
    // Fetch our data tree and create the output data tree
    //
    avtDataTree_p tree = GetInputDataTree();
    avtDataTree_p newTree;

    const int numLeaves = tree->GetNumberOfLeaves();
    totalSteps = numLeaves * 2;
    currentProgress = 0;

    //
    // Calculate per rank results
    //
    std::map<int, intermediateResults> intermediate_results_map;
    ConstantEvaluation(tree, intermediate_results_map);

    //
    // Ensure that the number of components is in agreement across all our local
    // results.
    //
    const int ncomps = (intermediate_results_map.empty() ? 
                        0 : 
                        intermediate_results_map.begin()->second.ncomps);

    if (! intermediate_results_map.empty())
    {
        if (std::any_of(std::next(intermediate_results_map.begin()),
                        intermediate_results_map.end(),
                        [ncomps](const auto &pair)
                        {
                            return pair.second.ncomps != ncomps;
                        }))
        {
            // TODO parallel error checking
            EXCEPTION2(ExpressionException, outputVariableName,
                       "An internal error occurred when "
                       "trying to calculate your expression. Please contact a "
                       "VisIt developer.");
        }
    }

    //
    // Calculate the local results across all domains on this rank
    //
    std::vector<double> local_constant_results;
    std::vector<double> local_extra_constant_results;

    // set our arrays to be the value of the first intermediate result array
    if (! intermediate_results_map.empty())
    {
        local_constant_results = intermediate_results_map.begin()->second.constant_results;
        local_extra_constant_results = intermediate_results_map.begin()->second.extra_constant_results;
        
        // now iterate through the remaining arrays and update our result arrays
        std::for_each(std::next(intermediate_results_map.begin()),
                      intermediate_results_map.end(),
                      [this,
                       &local_constant_results,
                       &local_extra_constant_results,
                       ncomps](const auto &pair)
                      {
                          const int ntuples = pair.second.ntuples;
                          if (ntuples > 0)
                          {
                              const auto &curr_leaf_results = pair.second.constant_results;
                              for (int comp_id = 0; comp_id < ncomps; comp_id ++)
                              {
                                  local_constant_results[comp_id] = 
                                      LocalIntermediateReduction(local_constant_results[comp_id],
                                                                 curr_leaf_results[comp_id]);
                              }
                              if (NeedsExtraIntermediateData())
                              {
                                  const auto &curr_leaf_extra_results = pair.second.extra_constant_results;
                                  for (int comp_id = 0; comp_id < ncomps; comp_id ++)
                                  {
                                      local_extra_constant_results[comp_id] = 
                                          LocalIntermediateReduction(local_extra_constant_results[comp_id],
                                                                     curr_leaf_extra_results[comp_id]);
                                  }
                              }
                          }
                      });
    }

    //
    // Calculate the local total number of non-ghosted tuples
    //
    const int local_total_ntuples = (NeedsNTuples() ? GetLocalNumTuples(intermediate_results_map) : 0);

    //
    // Ensure all ranks agree on the number of components
    //
    int global_ncomps;
#ifdef PARALLEL
    global_ncomps = UnifyMaximumValue(ncomps);
    int global_ncomps_min = UnifyMinimumValue(ncomps);
    if (global_ncomps_min != global_ncomps)
    {
        if (ncomps != 0 && ncomps != global_ncomps)
        {
            // TODO parallel error handling
            EXCEPTION2(ExpressionException, outputVariableName,
                       "An internal error occurred when "
                       "trying to calculate your expression. Please contact a "
                       "VisIt developer.");
        }
    }
#else
    global_ncomps = ncomps;
#endif

    // if we have no data on our rank, we need to prepare our arrays for parallel
    // communication anyway
    if (intermediate_results_map.empty())
    {
        local_constant_results.resize(global_ncomps);
        std::fill(local_constant_results.begin(), local_constant_results.end(), GetUnusedValue());
        
        if (NeedsExtraIntermediateData())
        {
            local_extra_constant_results.resize(global_ncomps);
            std::fill(local_extra_constant_results.begin(), local_extra_constant_results.end(), GetUnusedValue());
        }
    }

    int global_ntuples = local_total_ntuples;
#ifdef PARALLEL
    if (NeedsNTuples())
    {
        SumIntAcrossAllProcessors(global_ntuples);
    }
#endif

    //
    // Calculate global result
    //
    std::vector<double> global_constant_results(global_ncomps);
#ifdef PARALLEL
    GlobalIntermediateReduction(local_constant_results, global_constant_results, global_ncomps);
#else
    global_constant_results = std::move(local_constant_results);
#endif

    std::vector<double> global_extra_constant_results;
    if (NeedsExtraIntermediateData())
    {
#ifdef PARALLEL
        global_extra_constant_results.resize(global_ncomps);
        GlobalIntermediateReduction(local_extra_constant_results, global_extra_constant_results, global_ncomps);
#else
        global_extra_constant_results = std::move(local_extra_constant_results);
#endif
    }

    //
    // Write result
    //
    std::vector<double> final_results;
    CalculateFinalResults(global_constant_results,
                          global_extra_constant_results,
                          global_ncomps,
                          global_ntuples,
                          final_results);
    WriteResult(tree, newTree, intermediate_results_map, final_results);

    //
    // Lots of code assumes that the root tree is non-NULL. Put a dummy
    // tree in its place if it is.
    //
    if (*newTree == nullptr)
    {
        newTree = new avtDataTree();
    }

    SetOutputDataTree(newTree);

    // Set progress to complete
    UpdateProgress(totalSteps, totalSteps);
}

