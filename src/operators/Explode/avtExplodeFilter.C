/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//  File: avtExplodeFilter.C
// ************************************************************************* //

#include <avtExplodeFilter.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkCellIterator.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkCell.h>
#include <vtkAppendFilter.h>

#include <vtkIntArray.h>
#include <vtkDataSet.h>

#include <vtkVisItUtility.h>
#include <DebugStream.h>
#include <UnexpectedValueException.h>
#include <ImproperUseException.h>

#include <avtDatasetExaminer.h>
#include <avtDataTree.h>
#include <avtDatabaseMetaData.h>
#include <avtMaterialMetaData.h>
#include <avtMetaData.h>

#include <avtParallelContext.h>
#include <avtParallel.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#ifndef TINY_FACTOR
#define TINY_FACTOR 1.0e-15
#endif


// ****************************************************************************
//  Method: avtExplodeFilter constructor
//
//  Purpose:
//      Initialize class variables. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Wed Jan 17 15:28:46 PST 2018
//      Added globalMatExtents.
//
//      Alister Maguire, Mon Jan 22 11:12:47 PST 2018
//      Added init of scaleFactor. 
//
//      Alister Maguire, Wed Feb 14 14:36:02 PST 2018
//      Added hasMaterials, onlyCellExp, and init of 
//      datasetExtents. 
//
//      Alister Maguire, Tue Feb 27 11:08:51 PST 2018
//      Removed datasetExtents. 
//
//      Alister Maguire, Tue Sep 25 11:21:25 PDT 2018
//      Init oneSubPerDomain. Assume true and prove false. 
//
// ****************************************************************************

avtExplodeFilter::avtExplodeFilter()
{
    materialExtents    = NULL;
    explosions         = NULL;
    scaleFactor        = 0.0;
    numExplosions      = 0;
    hasMaterials       = false;
    onlyCellExp        = true;
    oneSubPerDomain    = true;
}


// ****************************************************************************
//  Method: avtExplodeFilter destructor
//
//  Purpose:
//      Handle any needed memory clean-up. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Wed Jan 17 15:28:46 PST 2018
//      Added globalMatExtents.
//
// ****************************************************************************

avtExplodeFilter::~avtExplodeFilter()
{
    if (explosions != NULL)
    {
        for (int i = 0; i < numExplosions; ++i)
        {
            if (explosions[i] != NULL)
                delete explosions[i];
        }
        delete [] explosions;
    }
    if (materialExtents != NULL)
    {
        delete [] materialExtents;
    }
}


// ****************************************************************************
//  Method:  avtExplodeFilter::Create
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
// ****************************************************************************

avtFilter *
avtExplodeFilter::Create()
{
    return new avtExplodeFilter();
}


// ****************************************************************************
//  Method:      avtExplodeFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
// ****************************************************************************

void
avtExplodeFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const ExplodeAttributes*)a;
}


// ****************************************************************************
//  Method: avtExplodeFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtExplodeFilter with the given
//      parameters would result in an equivalent avtExplodeFilter.
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
// ****************************************************************************

bool
avtExplodeFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(ExplodeAttributes*)a);
}


// ****************************************************************************
//  Method: avtExplodeFilter::GetMaterialIndex
//
//  Purpose:
//      Retrieve the index for a particular material within
//      gobalMatExtents. 
//
//  Arguments:
//      matName    The name of the target material. 
//
//  Returns:
//      If the material extents exist, the index to the start
//      of its extents within materialExtents is returned. Otherwise,
//      -1 is returned. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Jan  9 10:55:28 PST 2018
//
// ****************************************************************************

int
avtExplodeFilter::GetMaterialIndex(std::string matName)
{
    stringVector matNames = atts.GetBoundaryNames();
    int numMat = matNames.size();
    for (int i = 0; i < numMat; ++i)
    {
        if (matName == matNames[i])
        {
            return i * 6;
        }
    }
    return -1;
}


// ****************************************************************************
//  Method: avtExplodeFilter::UpdateExtentsAcrossProcs
//
//  Purpose:
//      Update the material extents across all processors. 
//
//  Programmer: Alister Maguire
//  Creation:   Wed Feb  7 10:26:21 PST 2018
//
//  Modifications:
//
//      Alister Maguire, Wed Feb 14 14:36:02 PST 2018
//      Added updating of dataset extents. 
//
//      Alister Maguire, Tue Feb 27 11:08:51 PST 2018
//      Removed update of datasetExtents. 
//
// ****************************************************************************

void
avtExplodeFilter::UpdateExtentsAcrossProcs()
{
    #ifdef PARALLEL
    if (materialExtents == NULL)
    {
        ResetMaterialExtents(true);
    }
    
    int numMat     = atts.GetBoundaryNames().size();
    int numExtents = numMat * 6;
    int half       = numExtents / 2;

    double *curMinExtents = new double[half];
    double *curMaxExtents = new double[half];
    for (int i = 0; i < half; ++i)
    {
        int idx = i*2;
        curMinExtents[i] = materialExtents[idx];
        curMaxExtents[i] = materialExtents[idx + 1];
    }
 
    double *trueMinExtents = new double[half];
    double *trueMaxExtents = new double[half];

    MPI_Allreduce(&curMinExtents[0], &trueMinExtents[0], half,
        MPI_DOUBLE, MPI_MIN, VISIT_MPI_COMM);
    MPI_Allreduce(&curMaxExtents[0], &trueMaxExtents[0], half,
        MPI_DOUBLE, MPI_MAX, VISIT_MPI_COMM);
    
    //
    // Update the material extents from across all procs. 
    //
    for (int i = 0; i < half; ++i)
    {
        int idx = i*2;
        materialExtents[idx]     = trueMinExtents[i];
        materialExtents[idx + 1] = trueMaxExtents[i];
    }

    delete [] curMinExtents;
    delete [] curMaxExtents;
    delete [] trueMinExtents;
    delete [] trueMaxExtents;
    #endif
}
 

// ****************************************************************************
//  Method: avtExplodeFilter::UpdateExtentsAcrossDomains
//
//  Purpose:
//      Update the material extents across all domains. 
//
//  Arguments:
//      localExtents    The extents of a single material on a single domain. 
//      matName         The name of the material. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Jan  9 10:55:28 PST 2018
//
//  Modifications:
//
//      Alister Maguire, Mon Jan 22 11:12:47 PST 2018
//      Added a safety check for globalMatExtents. 
//
//      Alister Maguire, Wed Feb 14 14:36:02 PST 2018
//      Altered safety check to init the extents if 
//      they are null. Also added a section to update
//      the dataset extents. 
//
//      Alister Maguire, Tue Feb 27 11:08:51 PST 2018
//      Removed update of datasetExtents. 
//
// ****************************************************************************

void
avtExplodeFilter::UpdateExtentsAcrossDomains(double *localExtents, 
                                             std::string matName)
{
    if (materialExtents == NULL)
    {
        ResetMaterialExtents(true);
    }

    int matIdx = GetMaterialIndex(matName);
    if (matIdx < 0)
    {
        //
        // Material doesn't exist on this domain. 
        //
        return;
    }

    //
    // Update material extents. 
    //
    for (int i = 0; i < 3; ++i)
    {
        int gMatIdx = matIdx + (i*2);
        int locIdx  = i*2;
        if (localExtents[locIdx] < materialExtents[gMatIdx])
        {
            materialExtents[gMatIdx] = localExtents[locIdx];
        }

        gMatIdx += 1;
        locIdx  += 1;
        if (localExtents[locIdx] > materialExtents[gMatIdx])
        {
            materialExtents[gMatIdx] = localExtents[locIdx];
        }
    } 
}


// ****************************************************************************
//  Method: avtExplodeFilter::ResetMaterialExtents
//
//  Purpose:
//      Reset the materialExtents. 
//
//  Arguments:
//      fullReset    If true, reset all of the material extents. If false, 
//                   only reset a specified material. 
//      matIdx       The index to the material to reset when fullReset is 
//                   false. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Feb 12 15:54:41 PST 2018
//
//  Modifications:
//
// ****************************************************************************

void 
avtExplodeFilter::ResetMaterialExtents(bool fullReset, int matIdx)
{
    if (!fullReset && (matIdx > 0))
    {
        if (materialExtents == NULL)
        {
            fullReset = true;
        }
        else
        {
            for (int j = 0; j < 3; ++j)
            {
                int idx = matIdx + (j*2);
                materialExtents[idx]   = DBL_MAX;
                materialExtents[idx+1] = DBL_MIN;
            } 
        }
    }

    if (fullReset)
    {
        int numMat       = atts.GetBoundaryNames().size();
        int numExtents   = numMat * 6;

        if (materialExtents == NULL)
        {
            materialExtents = new double[numExtents];
        }

        for (int i = 0; i < numMat; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                int idx = i*6 + j*2;
                materialExtents[idx]   = DBL_MAX;
                materialExtents[idx+1] = DBL_MIN;
            } 
        }
    }
}


// ****************************************************************************
//  Method: avtExplodeFilter::CreateDomainTree
//
//  Purpose:
//      Merge all datasets that reside on the same domain, and
//      create an avtDataTree whose leaves are domain datasets. 
//
//  Arguments:
//      dsets     Input datasets. 
//      numDSets  The number of datasets. 
//      domainIds All domain ids for the given datasets. 
//      labels    The original input labels. 
//
//  Returns:
//      An avtDataTree pointer whose leaves all have unique domain
//      ids. 
//
//  Programmer: Alister Maguire
//  Creation: Thu Jan 25 11:33:44 PST 2018
//
//  Modifications: 
//
//      Alister Maguire, Wed Feb  7 10:26:21 PST 2018
//      Changed name and input args. 
//
// ****************************************************************************

avtDataTree_p
avtExplodeFilter::CreateDomainTree(vtkDataSet **dsets,
                                   int numDSets,
                                   std::vector<int> domainIds, 
                                   stringVector labels)
{
    vtkAppendFilter *appendFilter = vtkAppendFilter::New(); 
    std::vector<int> uniqueDomainIds;

    //
    // We need the min and max domains so that we can map
    // them to their merged indices. 
    //
    int maxDomain = INT_MIN;
    int minDomain = INT_MAX;
    for (std::vector<int>::iterator it = domainIds.begin();
        it != domainIds.end(); ++it)
    {
        maxDomain = maxDomain > (*it) ? maxDomain : (*it);
        minDomain = minDomain < (*it) ? minDomain : (*it);
    }

    int mergeMaxIdx = maxDomain - minDomain;
    if (mergeMaxIdx < 0)
    {
        debug1 << "avtExplodeFilter: mergeMaxIdx < 0!!" << endl;
        return NULL;
    }

    for (std::vector<int>::iterator it = domainIds.begin();
         it != domainIds.end(); ++it)
    {
        if (std::find(uniqueDomainIds.begin(), uniqueDomainIds.end(), (*it))
            == uniqueDomainIds.end())
        {
            uniqueDomainIds.push_back(*it);
        } 
    }

    int numUniqueDomains = uniqueDomainIds.size();

    vtkDataSet **mergedDomains = new vtkDataSet *[numUniqueDomains];
    for (int i = 0; i < numUniqueDomains; ++i)
    {
        mergedDomains[i] = NULL;
    }

    for (int i = 0; i < numDSets; ++i)
    {
        int mdIdx = domainIds[i] - minDomain;

        if (dsets[i] == NULL)
        {
            continue;
        }
        //
        // A NULL position implies we need to place the
        // first dataset at this index into mergedDomains. 
        //
        else if (mergedDomains[mdIdx] == NULL)
        {
            vtkUnstructuredGrid *dsetCpy = vtkUnstructuredGrid::New();
            dsetCpy->DeepCopy(dsets[i]);
            mergedDomains[mdIdx] = dsetCpy;
        }
        //
        // If there's already a dataset at this index, 
        // we need to merge our current dataset with the one 
        // already in the mergedDomains array. 
        //
        else
        {
            vtkUnstructuredGrid *dsetCpy = vtkUnstructuredGrid::New();
            dsetCpy->DeepCopy(dsets[i]);

            appendFilter->AddInputData(mergedDomains[mdIdx]);
            appendFilter->AddInputData(dsetCpy);

            appendFilter->Update();

            vtkUnstructuredGrid *combined = vtkUnstructuredGrid::New();
            combined->DeepCopy(appendFilter->GetOutput());

            mergedDomains[mdIdx]->Delete(); 
            dsetCpy->Delete();
            mergedDomains[mdIdx] = combined;
         
            appendFilter->RemoveAllInputs();            
        }
    }
   
    avtDataTree_p outTree = NULL;

    if (numUniqueDomains == 1)
    {
        outTree = new avtDataTree(1, mergedDomains,
            uniqueDomainIds[0], labels);
    }
    else
    {
        outTree = new avtDataTree(numUniqueDomains, mergedDomains,
            uniqueDomainIds, labels);
    }

    for (int i = 0; i < numUniqueDomains; ++i)
    {
        if (mergedDomains[i] != NULL)
        {
            mergedDomains[i]->Delete();
        }
    }
    delete [] mergedDomains;

    if (appendFilter != NULL)
    {
        appendFilter->Delete();
    }

    return outTree;
}


// ****************************************************************************
//  Method: avtExplodeFilter::ExtractMaterialsFromDomains
//
//  Purpose:
//      Extract materials from all domains. 
//
//  Arguments:
//      inTree    The input data tree.     
//
//  Returns:
//      An avtDataTree pointer whose leaves are material
//      datasets. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Jan  9 10:55:28 PST 2018
//
// ****************************************************************************

avtDataTree_p
avtExplodeFilter::ExtractMaterialsFromDomains(avtDataTree_p inTree)
{
    if (*inTree == NULL)
    { 
        return NULL;
    }

    int numChildren = inTree->GetNChildren();

    if (numChildren <= 0 && !inTree->HasData())
    {
        return NULL;
    }

    if (numChildren == 0)
    {
        //
        // there is only one dataset to process
        //
        avtDataRepresentation domain_dr = inTree->GetDataRepresentation();
    
        //
        // Create a data tree where each leaf is a 
        // material dataset.
        //
        avtDataTree_p materialTree = GetMaterialSubsets(&domain_dr);

        return materialTree;
    }
    else
    {
        avtDataTree_p *outDT = new avtDataTree_p[numChildren];
        for (int j = 0; j < numChildren; j++)
        {
            if (inTree->ChildIsPresent(j))
                outDT[j] = ExtractMaterialsFromDomains(inTree->GetChild(j));
            else
                outDT[j] = NULL;
        }

        avtDataTree_p outTree = new avtDataTree(numChildren, outDT);
        delete [] outDT;
        return outTree;
    }
}


// ****************************************************************************
//  Method: avtExplodeFilter::GetMaterialSubsets
//
//  Purpose:
//      Create a dataset for every material within our input data, 
//      and create a tree whose leaves are material datasets. 
//
//  Arguments:
//      in_dr    The input data representation. 
//
//  Returns:
//      An avtDataTree whose leaves are material datasets. 
//
//  Programmer:  Alister Maguire
//  Creation:    Thu Dec 21 14:05:14 PST 2017 
//
//  Note:  Much of this was taken from the boundary plot filter. The 
//         output conversion has been changed from polydata to 
//         unstructured grid. The resulting meshes are fully disconnected. 
//
//  Modifications:
//
//      Alister Maguire, Mon Jan 29 10:12:44 PST 2018 
//      When the in_ds has no points or cells, return a tree with
//      that empty ds instead of NULL (NULL causes domain merging 
//      issues). Also, don't remove the avtSubsets array as it 
//      may be needed in future explosions.  
//
//      Alister Maguire, Wed Feb  7 10:26:21 PST 2018
//      Fixed bug with point data extraction. 
//
//      Alister Maguire, Tue Sep 25 11:21:25 PDT 2018
//      Added oneSubPerDomain. Also, if our domain has only one
//      subset, we still need to disconnect all of it's cells.  
//
// ****************************************************************************

avtDataTree_p
avtExplodeFilter::GetMaterialSubsets(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set, the domain number, and the label.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();
    int domain        = in_dr->GetDomain();
    std::string label = in_dr->GetLabel();
    
    if (in_ds == NULL)
    {
        return NULL;
    }

    stringVector   labels;
    int            nDataSets     = 0;
    vtkDataSet   **outDS         = NULL;
    vtkDataArray  *boundaryArray = 
        in_ds->GetCellData()->GetArray("avtSubsets");

    //
    // If our dataset has no points or cells, no extraction
    // is needed. 
    //
    if (in_ds->GetNumberOfPoints() == 0 ||
        in_ds->GetNumberOfCells()  == 0)
    {
        outDS    = new vtkDataSet *[1];
        outDS[0] = in_ds;
        outDS[0]->Register(NULL); 
        stringVector labels;
        labels.push_back(label);
        avtDataTree_p outDT = new avtDataTree(1, outDS, domain, labels);
        return outDT;
    }

    vtkAppendFilter *appendFilter = NULL;
    if (in_ds->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
    {
        appendFilter = vtkAppendFilter::New();
        appendFilter->SetInputData(in_ds);
        appendFilter->Update();
        in_ds = appendFilter->GetOutput();
    }
    vtkUnstructuredGrid *in_ug = (vtkUnstructuredGrid*)in_ds;

    //
    // If we have a boundary array, then we have materials to 
    // work with. 
    //
    if (boundaryArray)
    {
        if (label.find(";") == std::string::npos)
        {
            debug1 << "POSSIBLE ERROR CONDITION:  " << endl;
            debug1 << "    avtExplodeFilter encountered a label ("
                   << label.c_str() << ")" << endl;
            debug1 << "    that cannot be parsed correctly.  This can happen "
                   << "if" << endl;
            debug1 << "   another filter has over-written the boundary labels "
                   << "in" << endl;
            debug1 << "   its output data tree.  avtExplodeFilter is returning"
                   << endl;
            debug1 << "    an empty data tree." << endl;
            avtDataTree_p outTree = new avtDataTree();
            return outTree;
        }

        //
        // Raise flag to notify that domains have multiple 
        // subsets. 
        //
        oneSubPerDomain = false;

        //
        // Break up the dataset into a collection of datasets, one
        // per boundary.
        //
        int *boundaryList = ((vtkIntArray*)boundaryArray)->GetPointer(0);
        
        //
        // Get the data ready for transfer
        //
        int nCells          = in_ug->GetNumberOfCells();
        vtkPointData *inPD  = in_ug->GetPointData();
        vtkCellData  *inCD  = in_ug->GetCellData();
        int numTotalCells   = in_ug->GetNumberOfCells();

        //
        // Determine the total number of boundarys
        // and the labels for the boundarys.
        //
        char *cLabelStorage = new char[label.length()+1];
        strcpy(cLabelStorage, label.c_str());
        char *cLabel = cLabelStorage;

        int nSelectedBoundaries = 0;
        sscanf(cLabel, "%d", &nSelectedBoundaries);
        cLabel = strchr(cLabel, ';') + 1;

        int i, *selectedBoundaries = new int[nSelectedBoundaries];
        char **selectedBoundaryNames = new char*[nSelectedBoundaries];
        for (i = 0; i < nSelectedBoundaries; i++)
        {
            sscanf(cLabel, "%d", &selectedBoundaries[i]);
            cLabel = strchr(cLabel, ';') + 1;
            selectedBoundaryNames[i] = cLabel;
            cLabel = strchr(cLabel, ';');
            cLabel[0] = '\0';
            cLabel = cLabel + 1;
        }

        int maxBoundary = selectedBoundaries[0];
        for (i = 1; i < nSelectedBoundaries; i++)
        {
            maxBoundary = selectedBoundaries[i] > maxBoundary ?
                        selectedBoundaries[i] : maxBoundary;
        }

        //
        // Count the number of cells of each boundary.
        //
        int *boundaryCounts = new int[maxBoundary+1];
        for (int s = 0; s < maxBoundary + 1; s++)
        {
            boundaryCounts[s] = 0;
        }
        for (i = 0; i < numTotalCells; i++)
        {
            boundaryCounts[boundaryList[i]]++;
        }

        //
        // Create a dataset for each boundary.
        //
        outDS = new vtkDataSet *[nSelectedBoundaries];

        //
        // The following call is a workaround for a VTK bug.  It turns
        // out that when GetCellType if called for the first time for a
        // PolyData it calls its BuildCells method which causes the iterator
        // used by InitTraversal and GetNextCell to be put at the end of
        // the list.
        //
        in_ug->GetCellType(0);

        //
        // For each boundary, create a new unstructured grid that
        // is fully disconnected.
        //
        for (i = 0; i < nSelectedBoundaries; i++)
        {
            int boundary = selectedBoundaries[i];

            if (boundaryCounts[boundary] > 0)
            {
                //
                // Create the unstructured grid.
                //
                vtkPoints *pts               = vtkPoints::New();
                vtkIdList *cellPts           = vtkIdList::New();
                vtkIdList *ptIds             = vtkIdList::New();
                vtkUnstructuredGrid *out_ug  = vtkUnstructuredGrid::New();
                vtkPointData *outPD          = out_ug->GetPointData();
                vtkCellData  *outCD          = out_ug->GetCellData();
                vtkCellIterator *it          = in_ug->NewCellIterator();

                outCD->CopyAllocate(inCD, boundaryCounts[boundary]);
                out_ug->Allocate(boundaryCounts[boundary]);
                outPD->CopyAllocate(inPD);

                for (it->InitTraversal(); !it->IsDoneWithTraversal(); 
                    it->GoToNextCell())
                {
                    vtkIdType cellId = it->GetCellId();

                    if (boundaryList[cellId] == boundary)
                    {
                        int cellType      = it->GetCellType();
                        in_ug->GetCellPoints(cellId, cellPts);
                        int numIds        = cellPts->GetNumberOfIds();

                        for (int m = 0; m < numIds; ++m)
                        {
                            double point[3];
                            vtkIdType prevPtId = cellPts->GetId(m);
                            in_ug->GetPoint(prevPtId, point);
                            vtkIdType nxtPtId = pts->InsertNextPoint(point);
                            ptIds->InsertNextId(nxtPtId);
                            outPD->CopyData(inPD, prevPtId, nxtPtId);
                        }

                        int newId = out_ug->InsertNextCell(cellType, ptIds);
                        outCD->CopyData(inCD, cellId, newId);
                        cellPts->Reset(); 
                        ptIds->Reset();
                    }
                }

                //
                // Reclaim unused space.
                //
                outPD->Squeeze();

                out_ug->SetPoints(pts); 
                labels.push_back(selectedBoundaryNames[i]);
                outDS[nDataSets] = out_ug;
                nDataSets++;
                if (pts != NULL)
                {
                    pts->Delete();
                }
                if (cellPts != NULL)
                {
                    cellPts->Delete();
                }
                if (ptIds != NULL)
                {
                    ptIds->Delete();
                }
                if (it != NULL)
                {
                    it->Delete();
                }

                // 
                // Update the extents for this material. 
                // 
                double bounds[6];
                out_ug->GetBounds(bounds);
                std::string matName(selectedBoundaryNames[i]);
                UpdateExtentsAcrossDomains(bounds, matName);
            }
        }

        delete [] boundaryCounts;
        delete [] selectedBoundaryNames;
        delete [] selectedBoundaries;
        delete [] cLabelStorage;

    }
    else
    {
        //
        // The dataset represents a single boundary, BUT we still need
        // to disconnect it. Copy all of its data to a new, disconnected
        // ugrid.  
        //

        //
        // Get the data ready for transfer
        //  
        vtkUnstructuredGrid *out_grid = vtkUnstructuredGrid::New();
        int nCells               = in_ug->GetNumberOfCells();
        vtkPointData *inPD       = in_ug->GetPointData();
        vtkCellData  *inCD       = in_ug->GetCellData();
        vtkPointData *outPD      = out_grid->GetPointData();
        vtkCellData  *outCD      = out_grid->GetCellData();

        out_grid->Allocate(nCells);
        outCD->CopyAllocate(inCD);
        outPD->CopyAllocate(inPD);
        
        vtkCellIterator *it = in_ug->NewCellIterator();
        vtkPoints *pts      = vtkPoints::New();
        vtkIdList *cellPts  = vtkIdList::New();
        vtkIdList *ptIds    = vtkIdList::New();

        //
        // Copy the data over to our new ugrid. 
        //
        for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextCell())
        {
            int cellType      = it->GetCellType();
            vtkIdType  cellId = it->GetCellId();
            in_ug->GetCellPoints(cellId, cellPts);
            int numIds = cellPts->GetNumberOfIds();

            for (int i = 0; i < numIds; ++i)
            {
                double point[3];
                in_ug->GetPoint(cellPts->GetId(i), point);
                int nxtPtId = pts->InsertNextPoint(point);
                ptIds->InsertNextId(nxtPtId);
                outPD->CopyData(inPD, cellPts->GetId(i), nxtPtId);
            }

            int newId = out_grid->InsertNextCell(cellType, ptIds);
            outCD->CopyData(inCD, cellId, newId);
           
            cellPts->Reset(); 
            ptIds->Reset();
        }

        out_grid->SetPoints(pts); 
 
        //
        // Reclaim unused space.
        //
        outPD->Squeeze();

        //
        // Clean-up memory.
        //
        pts->Delete();
        cellPts->Delete();
        ptIds->Delete();
        it->Delete();

        labels.push_back(label);
        nDataSets = 1;

        outDS     = new vtkDataSet *[1];
        outDS[0]  = (vtkDataSet*)out_grid;

        double bounds[6];
        in_ds->GetBounds(bounds);
        std::string matName(label);
        UpdateExtentsAcrossDomains(bounds, label);
    }

    if (appendFilter != NULL)
    {
        appendFilter->Delete();
    }

    if (nDataSets == 0)
    {
        delete [] outDS;

        return NULL;
    }

    avtDataTree_p outDT = new avtDataTree(nDataSets, outDS, domain, labels);

    for (int i = 0 ; i < nDataSets ; i++)
    {
        if (outDS[i] != NULL)
        {
            outDS[i]->Delete();
        }
    }
    delete [] outDS;

    return outDT;
}


// ****************************************************************************
//  Method: avtExplodeFilter::PreExecute
//
//  Purpose:
//      Create an Explosion that will contain all the information
//      needed to perform our actual explosion. 
//
//  Programmer: Aliseter Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Wed Jan 17 10:06:58 PST 2018
//      Added globalMatExtents for multi-domain data. 
//
//      Alister Maguire, Mon Jan 22 11:12:47 PST 2018 
//      Added calculation of scaleFactor. 
//
//      Alister Maguire, Wed Feb  7 10:26:21 PST 2018
//      Refactored for multiple explosions. 
//
// ****************************************************************************

void
avtExplodeFilter::PreExecute(void)
{
    //
    // Calculate the scale factor for this dataset. 
    //
    double datasetExtents[6];
    GetSpatialExtents(datasetExtents);

    int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    int numReal   = 0;
    double volume = 1.0;
    for (int i = 0 ; i < dim ; i++)
    {
        if (datasetExtents[2*i] != datasetExtents[2*i+1])
        {
            numReal++;
            volume *= (datasetExtents[2*i+1] - datasetExtents[2*i]);
        }
    }
    if (volume < 0)
        volume *= -1.;
    if (numReal > 0)
        scaleFactor = pow(volume, 1.0/numReal);
    else
        scaleFactor = 1;

    //
    // Initialize the global material extents.
    //
    if (materialExtents != NULL)
    {
        delete [] materialExtents;
    }

    ResetMaterialExtents(true);

    if (!atts.GetBoundaryNames().empty())
    {
        hasMaterials = true;
    }

    numExplosions = atts.GetNumExplosions(); 

    if (explosions != NULL)
    {
        for (int i = 0; i < numExplosions; ++i)
        {
            delete explosions[i];
        }

        delete [] explosions;
    } 

    std::vector<ExplodeAttributes> expsAtts;

    //
    // If the explosion list is empty, we just
    // use the current attributes as our single
    // explosion. 
    //
    if (numExplosions == 0)
    {
        ExplodeAttributes attsCpy = ExplodeAttributes(atts);
        expsAtts.push_back(attsCpy);
        numExplosions++;
    }
    else
    {
        for (int i = 0; i < atts.GetNumExplosions(); ++i)
        {
            ExplodeAttributes attsCpy = 
                ExplodeAttributes(atts.GetExplosions(i));
            expsAtts.push_back(attsCpy);
        }
    }

    explosions = new Explosion *[numExplosions];

    //
    // Create the explosions. 
    //
    for (int i = 0; i < numExplosions; ++i)
    {
        int explosionType = expsAtts[i].GetExplosionType();

        switch (explosionType)
        {
            case ExplodeAttributes::Point:
            {
                PointExplosion *pointExp = new PointExplosion();
                double *expPoint         = expsAtts[i].GetExplosionPoint();

                for (int  j = 0;  j < 3; ++j)
                {
                    pointExp->explosionPoint[j] = expPoint[j]; 
                }

                explosions[i] = pointExp;
            }
            break;
            case ExplodeAttributes::Plane:
            {
                PlaneExplosion *planeExp = new PlaneExplosion();
                double *planePoint       = expsAtts[i].GetPlanePoint();
                double *planeNorm        = expsAtts[i].GetPlaneNorm();

                for (int  j = 0;  j < 3; ++j)
                {
                    planeExp->planePoint[j] = planePoint[j];
                    planeExp->planeNorm[j]  = planeNorm[j];
                }
 
                explosions[i] = planeExp;
            }
            break;
            case ExplodeAttributes::Cylinder:
            {
                CylinderExplosion *cylExp = new CylinderExplosion();
                double *cylinderPoint1    = expsAtts[i].GetCylinderPoint1();
                double *cylinderPoint2    = expsAtts[i].GetCylinderPoint2();

                for (int  j = 0;  j < 3; ++j)
                {
                    cylExp->cylinderPoint1[j] = cylinderPoint1[j];
                    cylExp->cylinderPoint2[j] = cylinderPoint2[j];
 
                }

                cylExp->cylinderRadius = expsAtts[i].GetCylinderRadius();
                explosions[i]          = cylExp;
            }
            break;
            default:
            {
                char recieved[256];
                char expected[256];
                int expType = expsAtts[i].GetExplosionType();
                sprintf(expected, "A known explosion type (0-2)");
                sprintf(recieved, "%d", expType);
                EXCEPTION2(UnexpectedValueException, expected, recieved);
                avtDataTree_p emptyTree = new avtDataTree(); 
                SetOutputDataTree(emptyTree); 
                return;
            }
            break;
        }

        explosions[i]->materialName       = expsAtts[i].GetMaterial();
        explosions[i]->matExplosionFactor = 
            expsAtts[i].GetMaterialExplosionFactor();
        explosions[i]->explosionPattern   = expsAtts[i].GetExplosionPattern();

        bool expAllCells = expsAtts[i].GetExplodeAllCells();
        bool expMatCells = expsAtts[i].GetExplodeMaterialCells();
        onlyCellExp      = expAllCells && onlyCellExp;

        if (expMatCells && !expAllCells)
        {
            explosions[i]->explodeMaterialCells = true;
            explosions[i]->matExplosionFactor   = 
                expsAtts[i].GetMaterialExplosionFactor();
            explosions[i]->cellExplosionFactor  = 
                expsAtts[i].GetCellExplosionFactor();
        }
        else if (expAllCells)
        {
            explosions[i]->explodeAllCells     = true; 
            explosions[i]->cellExplosionFactor = 
                expsAtts[i].GetCellExplosionFactor();
        }
    }
}


// ****************************************************************************
//  Method: avtExplodeFilter::Execute
//
//  Purpose:
//      Explode a dataset based on the given attributes. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Wed Jan 17 10:06:58 PST 2018
//      Refactored to handle multi-domain data in both serial
//      and parallel. 
//
//      Alister Maguire, Mon Jan 22 11:12:47 PST 2018
//      Integrated scaleFactor into execution. 
//
//      Alister Maguire, Mon Jan 29 10:12:44 PST 2018
//      Keeping track of original domains now. 
//
//      Alister Maguire, Wed Feb  7 10:26:21 PST 2018
//      Changed output leaves from data rep to vtk datasets. 
//
//      Alister Maguire, Tue Feb 13 14:58:23 PST 2018
//      Refactored for multiple explosions. 
//
//      Alister Maguire, Mon Sep 24 11:28:36 PDT 2018
//      Added check for repeat labels. 
//
//      Alister Maguire, Tue Sep 25 11:21:25 PDT 2018
//      Only re-create a domain tree if we need to. 
//
// ****************************************************************************

void
avtExplodeFilter::Execute(void)
{
    //
    // Get the input data tree, domain ids, and 
    // labels. 
    //
    avtDataTree_p    inTree = GetInputDataTree();
    std::vector<int> domainIds;
    stringVector     inLabels; 
    inTree->GetAllDomainIds(domainIds);
    inTree->GetAllLabels(inLabels);

    //
    // If we are exploding all cells, we don't need to 
    // worry about materials. Also, if we don't have 
    // any materials, cell explosion is the only option. 
    //
    if (!hasMaterials || onlyCellExp)
    {
        int nLeaves;
        vtkDataSet **inLeaves  = inTree->GetAllLeaves(nLeaves);
        vtkDataSet **outLeaves = new vtkDataSet *[nLeaves];

        for (int expIdx = 0; expIdx < numExplosions; ++expIdx)
        {
            for (int i = 0; i < nLeaves; ++i)
            {
                vtkUnstructuredGrid *newLeaf = vtkUnstructuredGrid::New();
                explosions[expIdx]->ExplodeAllCells(inLeaves[i], 
                    newLeaf, scaleFactor);
                outLeaves[i] = (vtkDataSet *)newLeaf;
            }
        }

        avtDataTree_p outTree = NULL;
        if (nLeaves > 1)
        {
            if (inLabels.empty())
                outTree = new avtDataTree(nLeaves, outLeaves, domainIds);
            else
                outTree = new avtDataTree(nLeaves, outLeaves, domainIds, inLabels);
        }
        else if (nLeaves == 1)
        {
            if (inLabels.empty())
                outTree = new avtDataTree(nLeaves, outLeaves, domainIds[0]);
            else
                outTree = new avtDataTree(nLeaves, outLeaves, domainIds[0], inLabels);
        }
        else 
        {
            outTree = inTree;
        }

        for (int i = 0; i < nLeaves; ++i)
        {
            if (outLeaves[i] != NULL)
            {
                outLeaves[i]->Delete();
            }
        }
        delete [] outLeaves;
        delete [] inLeaves;

        SetOutputDataTree(outTree); 
        return;
    }

    //
    // We have materials => we need to extract them from
    // all domains. 
    //
    avtDataTree_p materialTree =
        ExtractMaterialsFromDomains(inTree);
   
    //
    // If we're in parallel, we need to update 
    // material extents across all processors.
    //
    #ifdef PARALLEL
    if (PAR_Size() > 1)
    {
        UpdateExtentsAcrossProcs();
    }
    #endif

    if (*materialTree == NULL)
    {
        debug1 << "ExtractMaterialsFromDomains returned a NULL materialTree..." 
            << endl;
        SetOutputDataTree(materialTree); 
        return;
    }

    int nLeaves;
    stringVector matLabels; 
    std::vector<int> matDomains;
    materialTree->GetAllDomainIds(matDomains);
    materialTree->GetAllLabels(matLabels);
    vtkDataSet **matLeaves = materialTree->GetAllLeaves(nLeaves);

    //TODO: if we can't find labels, should we try to use the
    //      atts boundary names?
    if (matLabels.size() < nLeaves)
    {
        char expected[256];
        char recieved[256];
        sprintf(expected, "number of labels to be >= "
            "number of leaves");
        sprintf(recieved, "Num labels: %d  Num leaves: %d  ", 
                (int) matLabels.size(), nLeaves);
        EXCEPTION2(UnexpectedValueException, expected, recieved);
        SetOutputDataTree(materialTree); 
        return;
    }

    bool *materialsExploded = new bool[nLeaves];
    for (int i = 0; i < nLeaves; ++i)
        materialsExploded[i] = 0;

    //
    // Iterate through all explosions. 
    //
    for (int expIdx = 0; expIdx < numExplosions; ++expIdx)
    {
        //
        // Look for the material that has been selected for
        // explosion, and blow it up. 
        //
        for (int i = 0; i < nLeaves; ++i)
        {
            if (explosions[expIdx]->explodeAllCells)
            {
                vtkUnstructuredGrid *newLeaf = 
                    vtkUnstructuredGrid::New();
                explosions[expIdx]->ExplodeAllCells(
                    (vtkUnstructuredGrid *)matLeaves[i], 
                    newLeaf, 
                    scaleFactor);

                ((vtkUnstructuredGrid *)matLeaves[i])->Reset();
                ((vtkUnstructuredGrid *)matLeaves[i])->DeepCopy(newLeaf);
                newLeaf->Delete();

                materialsExploded[i] = 1;
            }                    

            else if (explosions[expIdx]->materialName == matLabels[i])
            {
                double matExtents[6]; 
                int matIdx = GetMaterialIndex(matLabels[i]);
  
                if (matIdx < 0)
                {
                    SetOutputDataTree(materialTree); 
                    return;
                }

                for (int j = 0; j < 6; ++j)
                {
                    matExtents[j] = materialExtents[matIdx + j];
                }
 
                if (explosions[expIdx]->explodeMaterialCells)
                {
                    explosions[expIdx]->ExplodeAndDisplaceMaterial(
                        (vtkUnstructuredGrid *)matLeaves[i], 
                        matExtents, 
                        scaleFactor);
                }
                else
                {
                    explosions[expIdx]->DisplaceMaterial(
                        (vtkUnstructuredGrid *)matLeaves[i], 
                        matExtents, 
                        scaleFactor);
                }

                materialsExploded[i] = 1;
            }
        }

        //
        // If we have multiple explosions, we need
        // to update the extents each time. 
        //
        if ( (expIdx + 1) < numExplosions)
        {
            for (int i = 0; i < nLeaves; ++i)
            {
                //
                // We only need to update for materials that
                // have been exploded. 
                //
                if (materialsExploded[i])
                {
                    int matIdx = GetMaterialIndex(matLabels[i]);
                    ResetMaterialExtents(false, matIdx);

                    double bounds[6];
                    matLeaves[i]->GetBounds(bounds);
                    std::string matName(matLabels[i]);
                    UpdateExtentsAcrossDomains(bounds, matName);
                }
            }
        }
        #ifdef PARALLEL
        if (PAR_Size() > 1)
        {
            UpdateExtentsAcrossProcs();
        }
        #endif
    }
    delete [] materialsExploded;

    //
    // Note: when in parallel, we cannot safely return until
    //       we reach this point. 
    //
    if (nLeaves == 0)
    {
        if (matLeaves != NULL)
        {
            delete [] matLeaves;
        }
        SetOutputDataTree(materialTree); 
        return;
    }

    //
    // Our labels will sometimes have repeats. We need to 
    // condense them down before creating our domain tree. 
    // 
    stringVector compressedLabels;
    int prev = domainIds[0];
    compressedLabels.push_back(inLabels[0]);
    for (int i = 1; i < domainIds.size(); ++i)
    {
        if (prev != domainIds[i])
        {
            compressedLabels.push_back(inLabels[i]);
            prev = domainIds[i]; 
        }
    }
    

    //
    // If we've exploded domains, and we know that we 
    // have one subset per domain, we shouln't need to 
    // re-create a domain tree. In other cases, we do. 
    //
    avtDataTree_p outTree = NULL;
    if (atts.GetSubsetType() == ExplodeAttributes::Domain && oneSubPerDomain)
    {
        outTree = new avtDataTree(nLeaves, matLeaves, matDomains, compressedLabels);
    }
    else
    {
        outTree = CreateDomainTree(matLeaves, nLeaves, 
            matDomains, compressedLabels);
    }

    //
    // Clean up memory.
    //
    if (matLeaves != NULL)
    {
        delete [] matLeaves;
    }

    SetOutputDataTree(outTree);
}


// ****************************************************************************
//  Method: avtExplodeFilter::PostExecute
//
//  Purpose:
//      Update attributes after execution. 
//
//  Programmer: Alister Maguire
//  Creation:   Wed Nov  8 10:12:34 PST 2017
//
//  Modifications:
//
// ****************************************************************************

void
avtExplodeFilter::PostExecute(void)
{
    //
    // Spatial extents could have all changed, so we 
    // need to clear them. 
    //
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.GetOriginalSpatialExtents()->Clear();
    outAtts.GetDesiredSpatialExtents()->Clear();
    outAtts.GetActualSpatialExtents()->Clear();

    //
    // Update the spatial extents. 
    //
    double bounds[6];
    avtDataset_p ds = GetTypedOutput();
    avtDatasetExaminer::GetSpatialExtents(ds, bounds);
    outAtts.GetThisProcsOriginalSpatialExtents()->Set(bounds);

    stringVector treeLabels;
    GetDataTree()->GetAllUniqueLabels(treeLabels);
    GetOutput()->GetInfo().GetAttributes().SetLabels(treeLabels);
}


// ****************************************************************************
//  Method: avtExplodeFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Update the information about our output. 
//
//  Programmer: Alister Maguire
//  Creation:   Wed Nov  8 10:12:34 PST 2017
// 
//  Modifications:
//
// ****************************************************************************

void
avtExplodeFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateDataMetaData();
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();

    //
    // The dataset has become disconnected and reorganized. 
    //
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().InvalidateNodes();
    GetOutput()->GetInfo().GetValidity().ZonesSplit();
    
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetLabels(atts.GetBoundaryNames());
    outAtts.AddFilterMetaData("Explode");
}


// ****************************************************************************
//  Method: avtExplodeFilter::ModifyContract
//
//  Purpose:
//      Update the contract. In particular, we need to tell VisIt 
//      to construct materials and material labels. 
//
//  Programmer: Alister Maguire
//  Creation:   Wed Nov  8 10:12:34 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Mon Sep 17 13:31:13 PDT 2018
//      Only force MIR if we are exploding materials. 
//
// ****************************************************************************

avtContract_p   
avtExplodeFilter::ModifyContract(avtContract_p contract)
{
    avtContract_p rv = new avtContract(contract);

    //
    // Only force MIR if we are exploding materials. 
    //
    if (atts.GetSubsetType() == ExplodeAttributes::Material)
    {
        rv->GetDataRequest()->ForceMaterialInterfaceReconstructionOn();
        rv->GetDataRequest()->ForceMaterialLabelsConstructionOn();
    }
    rv->SetCalculateMeshExtents(true); 

    if (contract->GetDataRequest()->MayRequireZones())
    {
        rv->GetDataRequest()->TurnZoneNumbersOn();
    }
    if (contract->GetDataRequest()->MayRequireNodes())
    {
        rv->GetDataRequest()->TurnNodeNumbersOn();
    }
    
    return rv;
}


// ****************************************************************************
//  Method: Explosion constructor
//
//  Purpose: 
//      Initialize the explosion variables. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Mon Jan 22 09:38:39 PST 2018
//      Removed variables specific to explosion types. 
//
// ****************************************************************************

Explosion::Explosion()
{
    materialName         = "";
    explodeMaterialCells = false;
    explodeAllCells      = false;
    explosionPattern     = 0;
    matExplosionFactor   = 0.0;
    cellExplosionFactor  = 0.0;
    for (int i = 0; i < 3; ++i)
    {
        displaceVec[i]    = 0.0;
    }
}


// ****************************************************************************
//  Method: Explosion::DisplaceMaterial
//
//  Purpose: 
//      Translate/Displace the spatial coordinates of an entire
//      dataset.     
//
//  Arguments:
//      ugrid    An unstructured grid to be displaced/translated. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Mon Jan 22 11:12:47 PST 2018
//      Added scaleFactor argument. 
//
// ****************************************************************************

void
Explosion::DisplaceMaterial(vtkUnstructuredGrid *ugrid, 
                            double *matExtents, double scaleFactor)
{
    double dataCenter[3];
    for (int i = 0; i < 3; ++i)
    {
        dataCenter[i] = (matExtents[i*2] + matExtents[(i*2)+1]) / 2.0;
    }

    //
    // Calculate the explosion displacement. The method used
    // for displacement will depend and the child class's
    // implementation. 
    //
    CalcDisplacement(dataCenter, matExplosionFactor, scaleFactor, true);

    int numPoints        = ugrid->GetNumberOfPoints();
    vtkPoints *ugridPts  = ugrid->GetPoints();
    vtkPoints *newPoints = vtkPoints::New();
    newPoints->DeepCopy(ugridPts);

    //
    // Do the actual displacement. 
    //
    for (int ptIdx = 0; ptIdx < numPoints; ++ptIdx)
    {
        double *point; 
        point = ugridPts->GetPoint(ptIdx);
        for (int j = 0; j < 3; ++j)
        {
            point[j] += displaceVec[j];
        }
        newPoints->SetPoint(ptIdx, point);
    }
    ugrid->SetPoints(newPoints);
    newPoints->Delete();
}


// ****************************************************************************
//  Method: Explosion::ExplodeAndDisplaceMaterial
//
//  Purpose: 
//      Displace an entire material and then explode the cells
//      within that material. 
//
//  Arguments:
//      ugrid          An unstructured grid to be displaced/translated. 
//      matExtents     The extents of of the input material. 
//      scaleFactor    A factor to scale the explosion by. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Mon Jan 22 11:12:47 PST 2018
//      Added scaleFactor argument. 
//
// ****************************************************************************

void
Explosion::ExplodeAndDisplaceMaterial(vtkUnstructuredGrid *ugrid,  
                                      double *matExtents, 
                                      double scaleFactor)
{
    double dataCenter[3];
    for (int i = 0; i < 3; ++i)
    {
        dataCenter[i] = (matExtents[i*2] + matExtents[(i*2)+1]) / 2.0;
    }

    vtkPoints *ugridPts  = ugrid->GetPoints();
    vtkPoints *newPoints = vtkPoints::New();
    vtkIdList *cellPts   = vtkIdList::New();
    newPoints->DeepCopy(ugridPts);

    //
    // First, we need to calculate a displacement of the
    // entire material and copy this into a container. 
    //
    double matDisplacement[3];
    CalcDisplacement(dataCenter, matExplosionFactor, scaleFactor, false);
    for (int i = 0; i < 3; ++i)
    {
        matDisplacement[i] = displaceVec[i];
    }

    bool normalize;
    switch (explosionPattern)
    {
        case ExplodeAttributes::Impact:
        {
            normalize = true;
        }
        break;
        case ExplodeAttributes::Scatter:
        {
            normalize = false;
        }
        break;
    }
    
    //
    // Iterate over all cells and displace them by both material
    // and cell displacements. Since we are displacing cells, we 
    // need to disconect the dataset as well.  
    //
    vtkCellIterator *it = ugrid->NewCellIterator();
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextCell())
    {
        vtkIdType  cellId = it->GetCellId();
        ugrid->GetCellPoints(cellId, cellPts);
        vtkCell *cell     = ugrid->GetCell(cellId);

        //
        // Get the cell's center for calculating displacement. 
        //
        int    subId;
        double cellCenter[3];
        double weights[8];
        double pCenter[3];
        cell->GetParametricCenter(pCenter);
        cell->EvaluateLocation(subId, pCenter, cellCenter, weights);

        //
        // Calculate the displacement for this cell. 
        //
        CalcDisplacement(cellCenter, cellExplosionFactor, 
            scaleFactor, normalize, true); 

        //
        // Create a new set of points that are identical to the old
        // but displaced. 
        //
        int numIds = cellPts->GetNumberOfIds();
        for (int i = 0; i < numIds; ++i)
        {
            double point[3];
            ugrid->GetPoint(cellPts->GetId(i), point);
            for (int j = 0; j < 3; ++j)
            {
               point[j] += displaceVec[j] + matDisplacement[j];
            }
            newPoints->SetPoint(cellPts->GetId(i), point);
        }
        cellPts->Reset(); 
    }

    ugrid->SetPoints(newPoints);
    newPoints->Delete();
    cellPts->Delete();
    it->Delete();
}


// ****************************************************************************
//  Method: Explosion::ExplodeAllCells
//
//  Purpose:
//      Explode all of the cells within a dataset. 
//
//  Arguments:
//      in_ds          The input dataset. 
//      out_grid       The output dataset. 
//      scaleFactor    A factor to scale the explosion by. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Nov 21 11:19:55 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Mon Jan 22 11:12:47 PST 2018
//      Added scaleFactor argument. 
//
// ****************************************************************************

void
Explosion::ExplodeAllCells(vtkDataSet *in_ds,
                           vtkUnstructuredGrid *out_grid, 
                           double scaleFactor)
{
    //
    // Get the data ready for transfer
    //  
    int nCells               = in_ds->GetNumberOfCells();
    vtkPointData *inPD       = in_ds->GetPointData();
    vtkCellData  *inCD       = in_ds->GetCellData();
    vtkPointData *outPD      = out_grid->GetPointData();
    vtkCellData  *outCD      = out_grid->GetCellData();

    out_grid->Allocate(nCells);
    outCD->CopyAllocate(inCD);
    outPD->CopyAllocate(inPD);
    
    vtkCellIterator *it = in_ds->NewCellIterator();
    vtkPoints *pts      = vtkPoints::New();
    vtkIdList *cellPts  = vtkIdList::New();
    vtkIdList *ptIds    = vtkIdList::New();

    bool normalize;
    switch (explosionPattern)
    {
        case ExplodeAttributes::Impact:
        {
            normalize = true;
        }
        break;
        case ExplodeAttributes::Scatter:
        {
            normalize = false;
        }
        break;
    }

    //
    // Iterate over cells, displace their position based 
    // on an explosion, and insert these new cells into 
    // our out_grid. 
    //
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextCell())
    {
        int cellType      = it->GetCellType();
        vtkIdType  cellId = it->GetCellId();
        in_ds->GetCellPoints(cellId, cellPts);
        vtkCell *cell     = in_ds->GetCell(cellId);

        //
        // Get the cell's center for calculating displacement. 
        //
        int    subId;
        double cellCenter[3];
        double weights[8];
        double pCenter[3];
        cell->GetParametricCenter(pCenter);
        cell->EvaluateLocation(subId, pCenter, cellCenter, weights);
        
        //
        // Calculate the displacement for this cell. 
        //
        CalcDisplacement(cellCenter, cellExplosionFactor, 
            scaleFactor, normalize, true); 

        //
        // Create a new set of points that are identical to the old
        // but displaced. 
        //
        int numIds = cellPts->GetNumberOfIds();
        for (int i = 0; i < numIds; ++i)
        {
            double point[3];
            in_ds->GetPoint(cellPts->GetId(i), point);
            for (int j = 0; j < 3; ++j)
            {
               point[j] += displaceVec[j];
            }
            int nxtPtId = pts->InsertNextPoint(point);
            ptIds->InsertNextId(nxtPtId);
            outPD->CopyData(inPD, cellPts->GetId(i), nxtPtId);
        }

        int newId = out_grid->InsertNextCell(cellType, ptIds);
        outCD->CopyData(inCD, cellId, newId);
       
        cellPts->Reset(); 
        ptIds->Reset();
    }

    out_grid->SetPoints(pts); 
 
    //
    // Reclaim unused space.
    //
    outPD->Squeeze();

    //
    // Clean-up memory.
    //
    pts->Delete();
    cellPts->Delete();
    ptIds->Delete();
    it->Delete();
}


// ****************************************************************************
//  Method: Explosion::ScaleExplosion
//
//  Purpose: 
//      Scale and possibly normalize the displacement vector. 
//
//  Arguments:
//      expFactor      A factor to explode by. 
//      scaleFactor    A factor to scale by. 
//      normalize      Wheter or not we normalize the vector. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Jan 22 11:12:47 PST 2018
//
//  Modifications:
//
// ****************************************************************************

void
Explosion::ScaleExplosion(double expFactor, 
                          double scaleFactor, 
                          bool normalize)
{
    double mag = sqrt(displaceVec[0]*displaceVec[0]+
                      displaceVec[1]*displaceVec[1]+
                      displaceVec[2]*displaceVec[2]);

    if (mag == 0.0)
    {
        mag = 1.0;
    }

    if (normalize)
    {
        expFactor *= scaleFactor / mag;
        for (int i = 0; i < 3; ++i)
        {
            displaceVec[i] *= expFactor;
        }
    }
    else
    {
        expFactor *= mag / scaleFactor;
        for (int i = 0; i < 3; ++i)
        {
            displaceVec[i] *= expFactor;
        }
    }
}


// ****************************************************************************
//  Method: PointExplosion constructor
//
//  Purpose:
//      Empty constructor. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Mon Jan 22 09:38:39 PST 2018
//      Added explosionPoint. 
//
//      Alister Maguire, Wed Feb  7 10:26:21 PST 2018
//      Changed pointers to lists. 
//
// ****************************************************************************

PointExplosion::PointExplosion() 
{
    for (int i = 0; i < 3; ++i)
    {
        explosionPoint[i] = 0.0;
    }
}


// ****************************************************************************
//  Method: PointExplosion::CalcDisplacement
//
//  Purpose:
//      Calculate the displacement for a particular piece of data 
//      (this could be a chunk of data representing a material
//      or an individual cell). This displacement will be calculated
//      by exploding from a given point in space. 
//
//  Arguments:
//      dataCenter    The x,y,z coords of the center of the data
//                    to be displaced. 
//      expFactor     A factor to displace the data by. 
//      scaleFactor   A factor to scale the exposion by. 
//      normalize     Should we normalize the displacement vector?
//                    Yes produces 'impact' explosion, and no produces
//                    'scatter' explosion. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Mon Jan 22 11:12:47 PST 2018
//      Added scaleFactor argument, replaced scale
//      section with class method, and added safety
//      checks. 
//
//      Alister Maguire, Wed May  2 10:32:48 PDT 2018
//      Added recenter argument. If recenter is enabled, 
//      create a different data point when appropriate. 
//
// ****************************************************************************

void
PointExplosion::CalcDisplacement(double *dataCenter, double expFactor, 
                                 double scaleFactor, bool normalize,
                                 bool recenter)
{
    //
    //  Find the distance between the data center and the
    //  explosion point.
    //
    double sum = 0.0;
    for (int i = 0; i < 3; ++i)
    {
        displaceVec[i] = dataCenter[i] - explosionPoint[i];
        sum           += displaceVec[i];
    }

    //
    // If our explosion point matches our data center and
    // recenter is enabled, create a tiny displacement vector
    // which assumes exploding from a data point very close
    // to the data center. 
    //
    if (sum == 0.0 && recenter)
    {
        for (int i = 0; i < 3; ++i)
        {
            displaceVec[i] = TINY_FACTOR;
        }
    }

    ScaleExplosion(expFactor, scaleFactor, normalize);
}


// ****************************************************************************
//  Method: PlaneExplosion constructor
//
//  Purpose:
//      Empty constructor. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Mon Jan 22 09:38:39 PST 2018
//      Added planePoint and planeNorm. 
//
//      Alister Maguire, Wed Feb  7 10:26:21 PST 2018
//      Changed pointers to lists. 
//
// ****************************************************************************

PlaneExplosion::PlaneExplosion() 
{
    for (int i = 0; i < 3; ++i)
    {
        planePoint[i] = 0.0;
        planeNorm[i]  = 0.0;
    }
}


// ****************************************************************************
//  Method: PlaneExplosion::CalcDisplacement
//
//  Purpose:
//      Calculate the displacement for a particular piece of data 
//      (this could be a chunk of data representing a material
//      or an individual cell). This displacement will be calculated
//      by a given plane in space. 
//
//  Arguments:
//      dataCenter    The x,y,z coords of the center of the data
//                    to be displaced. 
//      expFactor     A factor to displace the data by. 
//      scaleFactor   A factor to scale the exposion by. 
//      normalize     Should we normalize the displacement vector?
//                    Yes produces 'impact' explosion, and no produces
//                    'scatter' explosion. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Mon Jan 22 11:12:47 PST 2018
//      Added scaleFactor argument, replaced scale
//      section with class method, and added safety
//      checks. 
//
//      Alister Maguire, Fri Apr 27 11:13:22 PDT 2018
//      If the data center lies on the plane, use a data point
//      that is slightly skewed from the center. 
//
//      Alister Maguire, Wed May  2 10:32:48 PDT 2018
//      Added recenter argument. Only recenter the data point
//      when recenter is enabled. 
//
// ****************************************************************************

void
PlaneExplosion::CalcDisplacement(double *dataCenter, double expFactor, 
                                 double scaleFactor, bool normalize, 
                                 bool recenter)
{
    double dataPt[] = {0.0, 0.0, 0.0};
    
    for (int i = 0; i < 3; ++i)
    {
        dataPt[i] = dataCenter[i];
    }

    if (recenter)
    {
        //
        // If recenter is enabled, check if our data point lies
        // on the plane. If it does, use a new data point that
        // isn't the cell center but still should be in the cell. 
        //
        double planeDist = 0.0;
        for (int i = 0; i < 3; ++i)
        {
            planeDist += (dataPt[i] - planePoint[i]) * planeNorm[i];
        }

        if  (planeDist == 0.0)
        {
            //
            // Create a new point's that slightly off the center. 
            //
            for (int i = 0; i < 3; ++i)
            {
                dataPt[i] = dataPt[i] + planeNorm[i] * TINY_FACTOR;
            }
        }
    }

    //
    // Project from our data point onto a plane. 
    //
    double denom = 0.0;
    double alpha = 0.0;
    for (int i = 0; i < 3; ++i)
    {
        alpha += (planeNorm[i] * planePoint[i]) - 
                 (planeNorm[i] * dataPt[i]);
        denom += planeNorm[i] * planeNorm[i];
    }

    if (denom == 0.0)
        denom = 1.0;

    alpha /= denom;

    //
    // Subtract the projection from the data point
    // to get the distance from the plane. 
    //
    for (int i = 0; i < 3; ++i)
    {
        displaceVec[i] = dataPt[i] - 
            (dataPt[i] + (alpha * planeNorm[i]));
    }

    ScaleExplosion(expFactor, scaleFactor, normalize);
}


// ****************************************************************************
//  Method: CylinderExplosion constructor
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//      Alister Maguire, Mon Jan 22 09:38:39 PST 2018
//      Added cylinderPoint1, cylinderPoint2, and cylinderRadius. 
//
//      Alister Maguire, Wed Feb  7 10:26:21 PST 2018
//      Changed pointers to lists. 
//
// ****************************************************************************

CylinderExplosion::CylinderExplosion() 
{
    for (int i = 0; i < 3; ++i)
    {
        cylinderPoint1[i] = 0.0;
        cylinderPoint2[i] = 0.0;
    }
    cylinderRadius = 0.0;
}


// ****************************************************************************
//  Method: CylinderExplosion::CalcDisplacement
//
//  Purpose:
//      Calculate the displacement for a particular piece of data 
//      (this could be a chunk of data representing a material
//      or an individual cell). This displacement will be calculated
//      by a given cylinder in space. If the radius of the cylinder 
//      is 0, then we explode from a line. If the radius > 0, then 
//      anything in the radius is unaffected by the explosion. 
//
//  Arguments:
//      dataCenter    The x,y,z coords of the center of the data
//                    to be displaced. 
//      expFactor     A factor to displace the data by. 
//      scaleFactor   A factor to scale the exposion by. 
//      normalize     Should we normalize the displacement vector?
//                    Yes produces 'impact' explosion, and no produces
//                    'scatter' explosion. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Oct 23 15:52:30 PST 2017
//
//  Modifications:
//
//      Alister Maguire, Mon Jan 22 11:12:47 PST 2018
//      Added scaleFactor argument, replaced scale
//      section with class method, and added safety 
//      checks. 
//
//      Alister Maguire, Wed May  2 10:32:48 PDT 2018
//      Added recenter argument. If enabled, recenter the 
//      cell's data point when appropriate. 
//
// ****************************************************************************

void
CylinderExplosion::CalcDisplacement(double *dataCenter, double expFactor, 
                                    double scaleFactor, bool normalize, 
                                    bool recenter)
{
    //
    // Project from our data center to the line which runs 
    // through the center of our cylinder. 
    //
    double AP[3];
    double AB[3];
    for (int i = 0; i < 3; ++i)
    {
        AP[i] = dataCenter[i] - cylinderPoint1[i];
        AB[i] = cylinderPoint2[i] - cylinderPoint1[i];
    }

    double dot1 = 0.0;
    double dot2 = 0.0;
    for (int i = 0; i < 3; ++i)
    {
       dot1 += AP[i] * AB[i];
       dot2 += AB[i] * AB[i];
    }
    
    double projection[3];
    for (int i = 0; i < 3; ++i)
        projection[i] = cylinderPoint1[i] + ((dot1 / dot2) * AB[i]);

    double dist = 0.0;
    for (int i = 0; i < 3; ++i)
    {
        dist += (dataCenter[i] - projection[i]) * 
                (dataCenter[i] - projection[i]);
    }

    //
    // Check to see if our data center is in the cylinder's
    // radius. 
    //
    if (dist <= cylinderRadius)
    {
        //
        // If our radius is 0.0 and recenter is enabled, our
        // data center lies within a 'non-existent' radius. 
        // Let's create a tiny perpendicular vector for our
        // displacement. 
        //
        if (recenter && cylinderRadius == 0.0)
        {
            //
            // Create a vector perpendicular to our line. Use 
            // the fact that dot(V1, V2) = 0 IFF V1 and V2 are
            // perpendicular. 
            //
            double parVec[] = {0.0, 0.0, 0.0};
            double parMag   = 0.0;

            for (int i = 0; i < 3; ++i)
            {
                parVec[i] = cylinderPoint1[i] - cylinderPoint2[i];
                parMag   += parVec[i]*parVec[i];
            }
            parMag = sqrt(parMag);

            int nonZeroIdx   = -1;
            double numerator = 0.0;
            for (int i = 0; i < 3; ++i)
            {
                parVec[i] /= parMag;
                if (nonZeroIdx < 0 && parVec[i] != 0.0)
                {
                    nonZeroIdx = i;
                }
                else
                {
                    numerator -= parVec[i];
                }
            }

            if (nonZeroIdx < 0 || nonZeroIdx > 2)
            {
                char recieved[256];
                char expected[256];
                sprintf(expected, "An index in the range 0-2");
                sprintf(recieved, "%d", nonZeroIdx);
                EXCEPTION2(UnexpectedValueException, expected, recieved);
                return;
            }

            double perpVec[]    = {1.0, 1.0, 1.0};
            perpVec[nonZeroIdx] = 0.0;
            perpVec[nonZeroIdx] = numerator / parVec[nonZeroIdx];

            double perpMag   = sqrt(perpVec[0]*perpVec[0] +
                                    perpVec[1]*perpVec[1] + 
                                    perpVec[2]*perpVec[2]);

            //
            // Scale our perpendicular vector to be very small, 
            // and use this tiny vector as our displacement. 
            //
            for (int i = 0; i < 3; ++i)
            {
                perpVec[i]    /= perpMag;
                displaceVec[i] = perpVec[i] * TINY_FACTOR;
            }

            ScaleExplosion(expFactor, scaleFactor, normalize);
            return;
        }

        //
        // If the data is within a positive cylinder radius, 
        // we don't want to displace it, so just set the 
        // displacement to 0.0 and return. 
        //
        for (int i = 0; i < 3; ++i)
            displaceVec[i] = 0.0;
        return;
    }

    //
    // Find the distance between our data center and the
    // projection onto the line. 
    //
    for (int i = 0; i < 3; ++i)
    {
        displaceVec[i] = dataCenter[i] - projection[i];
    }

    ScaleExplosion(expFactor, scaleFactor, normalize);
}
