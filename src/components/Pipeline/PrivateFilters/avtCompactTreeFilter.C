// ************************************************************************* //
//                           avtCompactTreeFilter.C                          //
// ************************************************************************* // 


#include <avtCompactTreeFilter.h>


#include <avtSourceFromAVTDataset.h>
#include <avtDataSpecification.h>
#include <avtMetaData.h>
#include <avtPipelineSpecification.h>
#include <avtCommonDataFunctions.h>

#include <vtkAppendFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

#include <DebugStream.h>


// ****************************************************************************
//  Constructor: avtCompactTreeFilter
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 12, 2001 
//
//  Modifications:
//
//    Hank Childs, Wed Aug 24 15:45:14 PDT 2005
//    Initialized createCleanPolyData.
//
// ****************************************************************************

avtCompactTreeFilter::avtCompactTreeFilter()
{
    executionDependsOnDLB = false;
    createCleanPolyData = false;
}

// ****************************************************************************
//  Method: avtCompactTreeFilter::Execute
//
//  Purpose:
//    Compacts the data tree based on number and type of labels in info: 
//
//    If there are no labels, compacts all domains into one vtkDataSet.
//    If labels are present, will group all like-labeled leaves into
//    the same child tree.  
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 18, 2001 
//
//  Modifications: 
//    Kathleen Bonnell, Fri Oct 12 11:38:41 PDT 2001 
//    Removed domain-pruning as it is not really necessary.
//    Execute this filter when dynamically load-balanced, only if requested
//    via executionDependsOnDLB.  Added logic to handle the (improbable)
//    case where we have both polydata & unstructured grids in the tree.
//
//    Hank Childs, Thu Oct 18 18:08:44 PDT 2001
//    Fix deletion from the stack.
//
//    Kathleen Bonnell, Thu Nov  1 11:10:39 PST 2001
//    Don't execute if input tree is empty. 
//
//    Kathleen Bonnell, Fri Feb 15 14:29:51 PST 2002 
//    Added tests to ensure all the leaves of the input tree are accounted for
//    via the append filters.  If not, we return the input tree. 
//
//    Hank Childs, Tue Mar 26 13:37:07 PST 2002
//    Fix small memory leak.
//
//    Kathleen Bonnell, Thu May 30 09:31:36 PDT 2002 
//    Added debug lines for developer information if PruneTree returns
//    am empty tree.
//
//    Hank Childs, Wed Aug 24 15:45:14 PDT 2005
//    Create clean poly data if requested.
//
// ****************************************************************************

void
avtCompactTreeFilter::Execute(void)
{
    if (executionDependsOnDLB && 
        GetInput()->GetInfo().GetValidity().GetIsThisDynamic())
    {
        //
        //  Dynamic load balancing executes this filter during cleanup
        //  so no need to execute it at the the end of the avtPlot pipeline.
        //
        SetOutputDataTree(GetInputDataTree());
        return;
    }

    avtDataTree_p inTree = GetInputDataTree();

    if (inTree->IsEmpty())
    {
        //
        //  No need to compact an empty tree! 
        //  This is not an exception, because contour plots will return
        //  an empty tree for constant-valued variables.
        //
        SetOutputDataTree(inTree);
        return;
    }

    avtDataTree_p outTree; 
    bool dummy = false;

    vector<string> labels;

    GetInput()->GetInfo().GetAttributes().GetLabels(labels);

    struct map
    {
        vtkAppendFilter *filter;
        vtkAppendPolyData *polyFilter;
    } *pmap;
    
    pmap = new struct map;

    if (labels.empty())
    {
        int nleaves = inTree->GetNumberOfLeaves();
        if (1 == nleaves)
        {
            //
            //  No need for compacting a single-leaved tree!
            //
            SetOutputDataTree(inTree);
            delete pmap;
            return;
        }
        pmap->filter = vtkAppendFilter::New();
        pmap->polyFilter = vtkAppendPolyData::New();
        inTree->Traverse(CAddInputToAppendFilter, pmap, dummy);
        vtkDataSet *ds; 
        int nPolyInput = pmap->polyFilter->GetNumberOfInputs();
        int nUGridInput = pmap->filter->GetNumberOfInputs();
        //
        // 2D datasets sometimes have sgrids and ugrids/pdata
        // so don't use filter output unless we've accounted for
        // all the leaves.
        //
        if ( (nPolyInput > 1) && (nUGridInput > 1)  &&
             ((nPolyInput + nUGridInput) == nleaves))
             
        {
            vtkDataSet *ds[2];

            ds[0] = vtkUnstructuredGrid::New();
            pmap->filter->SetOutput((vtkUnstructuredGrid*)ds[0]);
            pmap->filter->Update();

            ds[1] = vtkPolyData::New();
            pmap->polyFilter->SetOutput((vtkPolyData*)ds[1]);
            pmap->polyFilter->Update();

            outTree = new avtDataTree(2, ds, -1);

            ds[0]->Delete();
            ds[1]->Delete();
        }
        else if (nUGridInput > 1 && nUGridInput == nleaves)
        {
            ds = vtkUnstructuredGrid::New();
            pmap->filter->SetOutput((vtkUnstructuredGrid*)ds);
            pmap->filter->Update();
            outTree = new avtDataTree(ds, -1);
            ds->Delete();
        }
        else if (nPolyInput > 1 && nPolyInput == nleaves)
        {
            ds = vtkPolyData::New();
            pmap->polyFilter->SetOutput((vtkPolyData*)ds);
            if (createCleanPolyData)
            {
                vtkCleanPolyData *cpd = vtkCleanPolyData::New();
                cpd->SetInput((vtkPolyData *) ds);
                cpd->Update();
                outTree = new avtDataTree(cpd->GetOutput(), -1);
                cpd->Delete();
            }
            else
            {
                ds->Update();
                outTree = new avtDataTree(ds, -1);
            }
            ds->Delete();
        }
        else 
        {
            //
            // Filters append only polydata or ugrids,
            // so simply return the intput tree.
            //
            outTree = inTree; 
        }
        pmap->filter->Delete();
        pmap->polyFilter->Delete();
    }
    else
    {
        //
        // Prune by labels.  Compact into n labels datasets.
        //
        vector<string> newLabels;
        avtDataTree_p prunedTree = inTree->PruneTree(labels, newLabels);

        if (prunedTree->IsEmpty())
        {
            debug1 << "\nPOSSIBLE ERROR CONDITION: " << endl;
            debug1 << "    avtCompactTreeFilter attempted to prune the data tree with" << endl;
            debug1 << "    avtDataAttribute labels that had no corresponding match" << endl;
            debug1 << "    in the tree.  This can happen if avtDataAttribute labels" << endl;
            debug1 << "    were set incorrectly by a filter. Tree can not be compacted.\n" << endl;
            SetOutputDataTree(inTree);
            return;
        }

        int nc = prunedTree->GetNChildren();
        vtkDataSet **ds = new vtkDataSet *[nc];
        vtkAppendFilter **filters = new vtkAppendFilter *[nc];
        vtkAppendPolyData **polyFilters = new vtkAppendPolyData *[nc];
        avtDataTree_p *temp = new avtDataTree_p [nc]; 
        avtDataTree_p child;
        int i;
        for (i = 0; i < nc; i ++)
        {
            ds[i] = NULL;
            child = prunedTree->GetChild(i);
            filters[i] = vtkAppendFilter::New();
            polyFilters[i] = vtkAppendPolyData::New();
            pmap->filter = filters[i];
            pmap->polyFilter = polyFilters[i];
            child->Traverse(CAddInputToAppendFilter, pmap, dummy);
            if (filters[i]->GetNumberOfInputs() > 1)
            {
                ds[i] = vtkUnstructuredGrid::New();
                filters[i]->SetOutput((vtkUnstructuredGrid*)ds[i]);
                filters[i]->Update();
                temp[i] = new avtDataTree(ds[i], -1, newLabels[i]);
            }
            else if (polyFilters[i]->GetNumberOfInputs() > 1)
            {
                ds[i] = vtkPolyData::New();
                polyFilters[i]->SetOutput((vtkPolyData*)ds[i]);
                polyFilters[i]->Update();
                temp[i] = new avtDataTree(ds[i], -1, newLabels[i]);
            }
            else
            {
                temp[i] = child; 
            }
        }
        outTree = new avtDataTree(nc, temp);
        for (i = 0; i < nc; i ++)
        {
            if (ds[i] != NULL)
                ds[i]->Delete();
            filters[i]->Delete();
            polyFilters[i]->Delete();
        }
        delete [] ds;
        delete [] filters;
        delete [] polyFilters;
        delete [] temp;
    }
    delete pmap;
    SetOutputDataTree(outTree);
}

