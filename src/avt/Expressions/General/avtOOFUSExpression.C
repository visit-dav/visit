// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtOOFUSExpression.C                        //
// ************************************************************************* //
#include <avtOOFUSExpression.h>

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
//  Class: intermediateResults
//
//  Purpose:
//      TODO
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
    std::vector<double> per_leaf_constant_results;
    int                 ncomps;
    int                 ntuples;
    vtkDataArray       *target_data_array;

    intermediateResults()
        : per_leaf_constant_results(), ncomps(0), ntuples(0), target_data_array(nullptr) {}

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
//  Method: avtOOFUSExpression constructor
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

avtOOFUSExpression::avtOOFUSExpression()
{
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtOOFUSExpression destructor
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

avtOOFUSExpression::~avtOOFUSExpression()
{

}

// ****************************************************************************
//  Method: avtOOFUSExpression::CalculateWithoutGhosts
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
void
avtOOFUSExpression::CalculateWithoutGhosts(vtkDataArray *in, 
                                           const int ncomponents,
                                           const int ntuples,
                                           std::vector<double> &per_leaf_constant_results)
{
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        double comp_max = in->GetComponent(0, comp_id);
        // start at 1 since we already looked at the 0th element
        for (int tuple_id = 1; tuple_id < ntuples; tuple_id ++)
        {
            const double val = in->GetComponent(tuple_id, comp_id);
            if (val > comp_max)
            {
                comp_max = val;
            }
        }

        per_leaf_constant_results[comp_id] = comp_max;
    }
}

// ****************************************************************************
//  Method: avtOOFUSExpression::CalculateWithGhosts
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
void
avtOOFUSExpression::CalculateWithGhosts(vtkDataArray *in,
                                        const int ncomponents,
                                        const int ntuples,
                                        int (getNodeOrCellValid)(vtkDataArray *, int *, int),
                                        vtkDataArray *ghostZones,
                                        int *nodeShouldBeIgnoredPtr,
                                        std::vector<double> &per_leaf_constant_results)
{
    for (int comp_id = 0; comp_id < ncomponents; comp_id ++)
    {
        int start_tuple_id = 0;
        double comp_max = [&]() -> double
        {
            for (int tuple_id = 0; tuple_id < ntuples; tuple_id ++)
            {
                if (0 == getNodeOrCellValid(ghostZones, nodeShouldBeIgnoredPtr, tuple_id))
                {
                    start_tuple_id = tuple_id + 1;
                    return in->GetComponent(tuple_id, comp_id);
                }
            }
            EXCEPTION2(ExpressionException, outputVariableName,
                 "Everything is ghosted so the OOFUS expression is not valid.");
            return 0; // return so the compiler is happy
        }();

        // start at start_tuple_id since it is the second non-ghosted tuple and we
        // have already looked at the first.
        for (int tuple_id = start_tuple_id; tuple_id < ntuples; tuple_id ++)
        {
            if (0 == getNodeOrCellValid(ghostZones, nodeShouldBeIgnoredPtr, tuple_id))
            {
                const double val = in->GetComponent(tuple_id, comp_id);
                if (val > comp_max)
                {
                    comp_max = val;
                }
            }
        }

        per_leaf_constant_results[comp_id] = comp_max;
    }
}

// ****************************************************************************
//  Method: avtOOFUSExpression::IdentifyGhostedNodes
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
std::vector<int>
avtOOFUSExpression::IdentifyGhostedNodes(vtkDataSet *in_ds,
                                         vtkDataArray *ghostZones,
                                         vtkDataArray *ghostNodes)
{
    const int nPoints = in_ds->GetNumberOfPoints();

    // we create an array to track if this point should be counted
    std::vector<int> nodeShouldBeIgnored(nPoints);
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

    return nodeShouldBeIgnored;
}

// ****************************************************************************
//  Method: avtOOFUSExpression::DoOperation
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
void
avtOOFUSExpression::DoOperation(vtkDataArray *inputArray,
                                const int ncomponents,
                                const int ntuples,
                                vtkDataSet *in_ds,
                                std::vector<double> &per_leaf_constant_results)
{
    vtkDataArray *ghostZones = in_ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *ghostNodes = in_ds->GetPointData()->GetArray("avtGhostNodes");

    if (AVT_ZONECENT == centering)
    {
        if (ghostZones)
        {
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
                                per_leaf_constant_results);
        }
        else // no ghosts or just ghost nodes
        {
            CalculateWithoutGhosts(inputArray, ncomponents, ntuples, per_leaf_constant_results);
        }
    }
    else // AVT_NODECENT == centering
    {
        // if we have any kind of ghosts
        if (ghostZones || ghostNodes)
        {
            // we need to identify which nodes should be ignored
            std::vector<int> nodeShouldBeIgnored = IdentifyGhostedNodes(
                in_ds, ghostZones, ghostNodes);

            // we pass a lambda to CalculateWithGhosts() that
            // looks at the nodeShouldBeIgnoredPtr to determine 
            // if a node is valid and ignores the ghostZones.
            CalculateWithGhosts(inputArray, ncomponents, ntuples,
                                [](vtkDataArray *ghostZones,
                                   int *nodeShouldBeIgnoredPtr,
                                   int tuple_id) -> int 
                                   { return nodeShouldBeIgnoredPtr[tuple_id]; },
                                ghostZones,
                                nodeShouldBeIgnored.data(), per_leaf_constant_results);
        }
        else // no ghosts
        {
            CalculateWithoutGhosts(inputArray, ncomponents, ntuples, per_leaf_constant_results);
        }
    }
}

// ****************************************************************************
//  Method: avtOOFUSExpression::CreateArray
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
vtkDataArray *
avtOOFUSExpression::CreateArray(vtkDataArray *in1)
{
    return in1->NewInstance();
}


// ****************************************************************************
//  Method: avtOOFUSExpression::DeriveVariable
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
void
avtOOFUSExpression::DeriveVariable(vtkDataSet *in_ds,
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

    vtkDataArray *dv = CreateArray(data);
    dv->SetNumberOfComponents(ncomps);
    dv->SetNumberOfTuples(ntuples);

    // we are caching this info so we can easily write to it later
    per_leaf_results.ncomps = ncomps;
    per_leaf_results.ntuples = ntuples;
    per_leaf_results.per_leaf_constant_results.resize(ncomps);
    per_leaf_results.target_data_array = dv;

    DoOperation(data, ncomps, ntuples, in_ds, per_leaf_results.per_leaf_constant_results);
}

// ****************************************************************************
//  Method: avtOOFUSExpression::ConstantEvaluation
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
int
avtOOFUSExpression::ConstantEvaluation(avtDataTree_p inputDataTree,
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
            dat->Register(nullptr);  // At the end of the routine, we will free this.
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
//  Method: avtOOFUSExpression::WriteData_VTK
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
avtDataRepresentation *
avtOOFUSExpression::WriteData_VTK(avtDataRepresentation *in_dr,
                                  intermediateResults &per_leaf_results,
                                  std::vector<double> global_constant_results)
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
//  Method: avtOOFUSExpression::WriteDataTree
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
avtDataTree_p
avtOOFUSExpression::WriteDataTree(avtDataRepresentation *in_dr,
                                  intermediateResults &per_leaf_results,
                                  std::vector<double> global_constant_results)
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
//  Method: avtOOFUSExpression::WriteResult
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   September 26, 2025
//
//  Modifications:
// ****************************************************************************
int
avtOOFUSExpression::WriteResult(avtDataTree_p inputDataTree, 
                                avtDataTree_p &outputDataTree,
                                std::map<int, intermediateResults> &intermediate_results_map,
                                std::vector<double> global_constant_results,
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
//  Method: avtOOFUSExpression::Execute
//
//  Purpose:
//      TODO
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
// ****************************************************************************
void
avtOOFUSExpression::Execute()
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
    if (intermediate_results_map.empty())
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "An internal error occurred when "
                   "trying to calculate your expression. Please contact a "
                   "VisIt developer.");
    }

    //
    // Ensure that the number of components is in agreement across all our local
    // results.
    //
    const int ncomps = intermediate_results_map.begin()->second.ncomps;
    if (std::any_of(std::next(intermediate_results_map.begin()),
                    intermediate_results_map.end(),
                    [ncomps](const auto &pair)
                    {
                        return pair.second.ncomps != ncomps;
                    }))
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "An internal error occurred when "
                   "trying to calculate your expression. Please contact a "
                   "VisIt developer.");
    }

    //
    // Calculate the local result across all domains on this rank
    //
    std::vector<double> local_constant_results = intermediate_results_map.begin()->second.per_leaf_constant_results;
    std::for_each(std::next(intermediate_results_map.begin()),
                  intermediate_results_map.end(),
                  [&local_constant_results, ncomps](const auto &pair)
                  {
                      const auto &curr_leaf_results = pair.second.per_leaf_constant_results;
                      for (int comp_id = 0; comp_id < ncomps; comp_id ++)
                      {
                          local_constant_results[comp_id] = std::max(local_constant_results[comp_id], 
                                                                     curr_leaf_results[comp_id]);
                      }
                  });

    //
    // Ensure all ranks agree on the number of components
    //
#ifdef PARALLEL
    int global_ncomps_max, global_ncomps_min;
    MPI_Allreduce(&ncomps, &global_ncomps_min, 1, MPI_INT, MPI_MIN, VISIT_MPI_COMM);
    MPI_Allreduce(&ncomps, &global_ncomps_max, 1, MPI_INT, MPI_MAX, VISIT_MPI_COMM);
    if (global_ncomps_min != global_ncomps_max)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "An internal error occurred when "
                   "trying to calculate your expression. Please contact a "
                   "VisIt developer.");
    }
#endif

    //
    // Calculate global result
    //
    std::vector<double> global_constant_results(ncomps);
#ifdef PARALLEL
    MPI_Allreduce(local_constant_results.data(), global_constant_results.data(),
                  ncomps, MPI_DOUBLE, MPI_MAX, VISIT_MPI_COMM);
#else
    global_constant_results = local_constant_results;
#endif

    //
    // Write result
    //
    WriteResult(tree, newTree, intermediate_results_map, global_constant_results);

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


// ****************************************************************************
//  Method: avtOOFUSExpression::CheckForProperGhostZones
//
//  Purpose:
//     Checks for ghost zone info that can be used in to reduce parallel 
//     communication. 
//
//  Arguments:
//    sets        Input data sets.
//    nsets       Number of input data sets. 
//
//  Programmer: Cyrus Harrison
//  Creation:   October 10, 20078
//
//  Modifications:
//    Cyrus Harrison, Tue Nov  9 10:32:01 PST 2010
//    Added timing info.
//
//    Cyrus Harrison, Tue Nov  9 10:32:01 PST 2010
//    Loosen the check for valid ghost zones.
//
// ****************************************************************************
bool
avtOOFUSExpression::CheckForProperGhostZones(vtkDataSet **sets,
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
//  Method: avtOOFUSExpression::LabelGhostNeighbors
//
//  Purpose:
//     Identifies cells that have ghost neighbors, storing the info in
//     a vtkUnsignedCharArray named "avtOnBoundary".
//
//  Arguments:
//    data_set     Input mesh
//
//  Programmer: Cyrus Harrison
//  Creation:   August 11, 2007
//
//  Modifications:
//    Cyrus Harrison, Tue Nov  9 10:32:01 PST 2010
//    Added timing info.
//
//    Cyrus Harrison, Tue Nov  9 10:32:01 PST 2010
//    Explicit check for DUPLICATED_ZONE_INTERNAL_TO_PROBLEM.
//
//    Ryan Bleile, Wed Jun 11 09:53:23 CDT 2014
//    Changed avtGhostZoneNeighbors to avtOnBoundary
// ****************************************************************************
void
avtOOFUSExpression::LabelGhostNeighbors(vtkDataSet *data_set)
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
//  Method: avtOOFUSExpression::LabelBoundaryNeighbors
//
//  Purpose:
//     Identifies cells that lie on the boundary, storing the results in 
//     a vtkUnsignedCharArray named "avtOnBoundary".
//
//  Arguments:
//    data_set     Input mesh
//
//  Programmer: Hank Childs
//  Creation:   November 30, 2013
//
//  Modifications:
//    Eric Brugger, Mon Jul 21 12:06:33 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

void
avtOOFUSExpression::LabelBoundaryNeighbors(vtkDataSet *data_set)
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
//  Method: avtGradientExpression::ModifyContract
//
//  Purpose:
//      Request ghost zones.
//
//  Programmer: Justin Privitera
//  Creation:   August 18, 2025
//
//  Modifications:
// ****************************************************************************
avtContract_p
avtOOFUSExpression::ModifyContract(avtContract_p in_spec)
{
    avtContract_p spec = 
                            avtExpressionFilter::ModifyContract(in_spec);
    spec->GetDataRequest()->SetDesiredGhostDataType(GHOST_ZONE_DATA);
    return spec;
}

