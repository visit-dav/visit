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
//                           avtCompactTreeFilter.C                          //
// ************************************************************************* // 


#include <avtCompactTreeFilter.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtSourceFromAVTDataset.h>
#include <avtDataObjectString.h>
#include <avtDataObjectReader.h>
#include <avtDataObjectWriter.h>
#include <avtDataSpecification.h>
#include <avtMetaData.h>
#include <avtParallel.h>
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
//    Hank Childs, Thu Sep 22 17:02:47 PDT 2005
//    Initialize tolerance.
//
// ****************************************************************************

avtCompactTreeFilter::avtCompactTreeFilter()
{
    executionDependsOnDLB = false;
    createCleanPolyData = false;
    tolerance = 0;
    parallelMerge = false;
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
//    Hank Childs, Thu Sep 22 17:02:23 PDT 2005
//    Account for tolerance when cleaning poly data.  Also add logic for doing
//    a parallel merge.
//
//    Kathleen Bonnell, Wed May 17 14:51:16 PDT 2006 
//    Changed GetNumberofInputs to GetTotalNumberOfInputConnections. 
//
//    Hank Childs, Fri Jun  9 13:25:31 PDT 2006
//    Use ifdef PARALLELs to remove compiler warnings.
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

    if (parallelMerge)
    {
#ifdef PARALLEL
        int mpiSendDataTag    = GetUniqueMessageTag();
        int mpiSendObjSizeTag = GetUniqueMessageTag();
#endif

        avtDataObject_p bigDS = GetTypedInput()->Clone();
        if (PAR_UIProcess())
        {
#ifdef PARALLEL
            int nprocs = PAR_Size();
            for (int i = 1 ; i < nprocs ; i++)
            {
                avtDataObjectReader reader;
                int len = 0;
                MPI_Status stat;
                MPI_Recv(&len, 1, MPI_INT, i, mpiSendObjSizeTag, 
                         MPI_COMM_WORLD, &stat);
                char *buff = new char[len];
                MPI_Recv(buff, len, MPI_CHAR, i, mpiSendDataTag, 
                         MPI_COMM_WORLD, &stat);
                reader.Read(len, buff);
                avtDataObject_p ds2 = reader.GetOutput();
                bigDS->Merge(*(ds2));
                //buff freed by reader
            }
#endif

            avtDataset_p ds2;
            CopyTo(ds2, bigDS);
            inTree = ds2->GetDataTree();
        }
        else
        {
#ifdef PARALLEL
            avtDataObjectWriter_p writer = GetInput()->InstantiateWriter();
            writer->SetInput(GetInput());
            avtDataObjectString str;
            writer->Write(str);
            int len = 0;
            char *buff = NULL;
            str.GetWholeString(buff, len);
            MPI_Send(&len, 1, MPI_INT, 0, mpiSendObjSizeTag, MPI_COMM_WORLD);
            MPI_Send(buff, len, MPI_CHAR, 0, mpiSendDataTag, MPI_COMM_WORLD);
#endif

            inTree = new avtDataTree(); // Make an empty tree, so we exit early
        }
    }

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
        int nPolyInput = pmap->polyFilter->GetTotalNumberOfInputConnections();
        int nUGridInput = pmap->filter->GetTotalNumberOfInputConnections();
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
                cpd->SetToleranceIsAbsolute(1);
                cpd->SetAbsoluteTolerance(tolerance);
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
            if (filters[i]->GetTotalNumberOfInputConnections() > 1)
            {
                ds[i] = vtkUnstructuredGrid::New();
                filters[i]->SetOutput((vtkUnstructuredGrid*)ds[i]);
                filters[i]->Update();
                temp[i] = new avtDataTree(ds[i], -1, newLabels[i]);
            }
            else if (polyFilters[i]->GetTotalNumberOfInputConnections() > 1)
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

