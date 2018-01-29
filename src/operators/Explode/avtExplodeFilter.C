/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkCellIterator.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkCell.h>
#include <vtkFieldData.h>
#include <vtkDoubleArray.h>
#include <vtkAppendFilter.h>

#include <vtkIntArray.h>
#include <vtkCellArray.h>
#include <vtkDataSet.h>
#include <MapNode.h>

#include <vtkVisItUtility.h>
#include <DebugStream.h>
#include <UnexpectedValueException.h>
#include <ImproperUseException.h>

#include <avtDatasetExaminer.h>
#include <avtDataTree.h>
#include <avtDatabaseMetaData.h>
#include <avtMaterialMetaData.h>
#include <avtMetaData.h>
#include <avtIntervalTree.h>

#include <avtParallelContext.h>
#include <avtParallel.h>

#ifdef PARALLEL
#include <mpi.h>
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
// ****************************************************************************

avtExplodeFilter::avtExplodeFilter()
{
    globalMatExtents = NULL;
    explosion        = NULL;
    scaleFactor      = 0.0;
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
    if (explosion != NULL)
    {
        delete explosion;
    }
    if (globalMatExtents != NULL)
    {
        delete [] globalMatExtents;
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
//      of its extents within globalMatExtents is returned. Otherwise,
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
//  Method: avtExplodeFilter::UpdateGlobalExtents
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
//      Alister Maguire, Mon Jan 22 11:12:47 PST 2018
//      Added a safety check for globalMatExtents. 
//
// ****************************************************************************

void
avtExplodeFilter::UpdateGlobalExtents(double *localExtents, std::string matName)
{
    if (globalMatExtents == NULL)
    {
        char recieved[256];
        char expected[256];
        sprintf(expected, "globalMatExtents to be non-NULL");
        sprintf(recieved, "NULL globalMatExtents");
        EXCEPTION2(UnexpectedValueException, expected, recieved);
        return;
    }

    int matIdx = GetMaterialIndex(matName);
    if (matIdx < 0)
    {
        //
        // Material doesn't exist on this domain. 
        //
        return;
    }

    for (int i = 0; i < 3; ++i)
    {
        int gMatIdx = matIdx + (i*2);
        int locIdx  = i*2;
        if (localExtents[locIdx] < globalMatExtents[gMatIdx])
        {
            globalMatExtents[gMatIdx] = localExtents[locIdx];
        }

        gMatIdx += 1;
        locIdx  += 1;
        if (localExtents[locIdx] > globalMatExtents[gMatIdx])
        {
            globalMatExtents[gMatIdx] = localExtents[locIdx];
        }
    } 
}


// ****************************************************************************
//  Method: avtExplodeFilter::MergeDomains
//
//  Purpose:
//      Given an input tree, merge all leaves that have identical
//      domain ids. 
//
//  Arguments:
//      inTree    The input data tree.
//      labels    The original input labels. 
//
//  Returns:
//      An avtDataTree pointer whose leaves all have unique domain
//      ids. 
//
//  Programmer: Alister Maguire
//  Creation: Thu Jan 25 11:33:44 PST 2018
//
// ****************************************************************************

avtDataTree_p
avtExplodeFilter::MergeDomains(avtDataTree_p inTree, stringVector labels)
{
    vtkAppendFilter *appendFilter = vtkAppendFilter::New(); 
    std::vector<int> domainIds;
    std::vector<int> uniqueDomainIds;
    inTree->GetAllDomainIds(domainIds);

    int numInputLeaves;
    vtkDataSet **inLeaves = inTree->GetAllLeaves(numInputLeaves);

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

    for (int i = 0; i < numInputLeaves; ++i)
    {
        int mdIdx = domainIds[i] - minDomain;

        if (inLeaves[i] == NULL)
        {
            continue;
        }
        else if (mergedDomains[mdIdx] == NULL)
        {
            vtkUnstructuredGrid *fromLeaf = vtkUnstructuredGrid::New();
            fromLeaf->DeepCopy(inLeaves[i]);
            mergedDomains[mdIdx] = fromLeaf;
        }
        else
        {
            appendFilter->AddInputData(mergedDomains[mdIdx]);
            appendFilter->AddInputData(inLeaves[i]);
            appendFilter->Update();
            vtkUnstructuredGrid *combined = vtkUnstructuredGrid::New();
            combined->DeepCopy(appendFilter->GetOutput());
            mergedDomains[mdIdx] = combined;
            appendFilter->RemoveAllInputs();            
        }
    }
   
    avtDataTree_p outTree = NULL;

    //
    // If we only have one leaf, we need to construct a tree with
    // a single domain. Otherwise, we construct with a list of domains.  
    //
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
        return NULL;

    int numChildren = inTree->GetNChildren();

    if (numChildren <= 0 && !inTree->HasData())
        return NULL;

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
// ****************************************************************************

avtDataTree_p
avtExplodeFilter::GetMaterialSubsets(avtDataRepresentation *in_dr)
{
    //
    // Get the VTK data set, the domain number, and the label.
    //
    vtkDataSet *in_ds = in_dr->GetDataVTK();
    int domain = in_dr->GetDomain();
    std::string label = in_dr->GetLabel();

    if (in_ds == NULL)
    {
        return NULL;
    }

    stringVector   labels;
    int            nDataSets     = 0;
    vtkDataSet   **outDS        = NULL;
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
        // Break up the dataset into a collection of datasets, one
        // per boundary.
        //
        int *boundaryList = ((vtkIntArray*)boundaryArray)->GetPointer(0);
        
        vtkUnstructuredGrid *in_ug = vtkUnstructuredGrid::New();
        in_ug->DeepCopy(in_ds);

        //
        // Get the data ready for transfer
        //
        int nCells          = in_ug->GetNumberOfCells();
        vtkPointData *inPD  = in_ug->GetPointData();
        vtkCellData  *inCD  = in_ug->GetCellData();

        int numCells        = in_ug->GetNumberOfCells();
        vtkPoints *pts      = vtkPoints::New();
        vtkIdList *cellPts  = vtkIdList::New();
        vtkIdList *ptIds    = vtkIdList::New();

        // 
        // Since we are removing connectivity between nodes, 
        // we will need more space to store them. The following
        // is a rough (usually over) estimation which imagines
        // that each cell could be a  VTK VOXEL or HEXAHEDRON 
        // type (8 nodes). We squeeze out the extra space later. 
        // NOTE: VTK has less common cell types that can have 
        // as many as 19 nodes, in which case this could actually 
        // be an under-estimation...
        //
        pts->Allocate(numCells * 8);
        ptIds->Allocate(numCells * 8);
        cellPts->Allocate(numCells);

        int numTotalCells = in_ug->GetNumberOfCells();

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
            int s = selectedBoundaries[i];

            if (boundaryCounts[s] > 0)
            {
                //
                // Create a new unstructured grid
                //
                vtkUnstructuredGrid *out_ug = vtkUnstructuredGrid::New();
                vtkPointData *outPD = out_ug->GetPointData();
                vtkCellData  *outCD = out_ug->GetCellData();
                vtkCellIterator *it = in_ug->NewCellIterator();

                out_ug->Allocate(nCells);
                outCD->CopyAllocate(inCD);
                outPD->CopyAllocate(inPD);
                outPD->Allocate(numCells * 8);

                for (it->InitTraversal(); !it->IsDoneWithTraversal(); 
                    it->GoToNextCell())
                {
                 
                    vtkIdType cellId = it->GetCellId();
                    if (boundaryList[cellId] == s)
                    {
                        int cellType      = it->GetCellType();
                        in_ug->GetCellPoints(cellId, cellPts);
                        int numIds        = cellPts->GetNumberOfIds();

                        for (int i = 0; i < numIds; ++i)
                        {
                            double point[3];
                            in_ug->GetPoint(cellPts->GetId(i), point);
                            int nxtPtId = pts->InsertNextPoint(point);
                            ptIds->InsertNextId(nxtPtId);
                            outPD->CopyData(inPD, cellPts->GetId(i), nxtPtId);
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

                //
                // Remove the avtSubsets array and set the label
                // for this dataset to be the material label. 
                //
                out_ug->SetPoints(pts); 
                labels.push_back(selectedBoundaryNames[i]);
                outDS[nDataSets] = out_ug;
                nDataSets++;

                //
                // Update the extents for this material. 
                //
                double bounds[6];
                out_ug->GetBounds(bounds);
                std::string matName(selectedBoundaryNames[i]);
                UpdateGlobalExtents(bounds, matName);
            }
        }

        delete [] boundaryCounts;
        delete [] selectedBoundaryNames;
        delete [] selectedBoundaries;
        delete [] cLabelStorage;

        in_ug->Delete();
    }
    else
    {
        //
        // The dataset represents a single boundary, so just turn it into
        // a data tree.
        //
        labels.push_back(label);
        outDS = new vtkDataSet *[1];
        outDS[0] = in_ds;
        outDS[0]->Register(NULL);  // This makes it symmetric with the 'if'
                                    // case so we can delete it blindly later.

        nDataSets = 1;
                
        double bounds[6];
        in_ds->GetBounds(bounds);
        std::string matName(label);
        UpdateGlobalExtents(bounds, label);
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
// ****************************************************************************

void
avtExplodeFilter::PreExecute(void)
{
    //
    // Calculate the scale factor for this dataset. 
    // Note: this was mostly copied from avtTubeFilter.
    //
    int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    double bounds[6];
    avtIntervalTree *it = GetMetaData()->GetSpatialExtents();
    if (it != NULL)
        it->GetExtents(bounds);
    else
        GetSpatialExtents(bounds);

    int numReal   = 0;
    double volume = 1.0;
    for (int i = 0 ; i < dim ; i++)
    {
        if (bounds[2*i] != bounds[2*i+1])
        {
            numReal++;
            volume *= (bounds[2*i+1]-bounds[2*i]);
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
    if (globalMatExtents != NULL)
    {
        delete [] globalMatExtents;
    }

    int numMat       = atts.GetBoundaryNames().size();
    int numExtents   = numMat * 6;
    globalMatExtents = new double[numExtents];
    for (int i = 0; i < numMat; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            int idx = i*6 + j*2;
            globalMatExtents[idx]   = FLT_MAX;
            globalMatExtents[idx+1] = FLT_MIN;
        } 
    }

    //
    // Create the explosion object.
    //
    if (explosion != NULL)
    {
        delete explosion;
    } 
    
    switch (atts.GetExplosionType())
    {
        case ExplodeAttributes::Point:
        {
            explosion = new PointExplosion();
            (dynamic_cast <PointExplosion *> 
                (explosion))->explosionPoint = atts.GetExplosionPoint();
        }
        break;
        case ExplodeAttributes::Plane:
        {
            explosion = new PlaneExplosion();
            (dynamic_cast <PlaneExplosion *> 
                (explosion))->planePoint = atts.GetPlanePoint();
            (dynamic_cast <PlaneExplosion *> 
                (explosion))->planeNorm  = atts.GetPlaneNorm();
        }
        break;
        case ExplodeAttributes::Cylinder:
        {
            explosion = new CylinderExplosion();
            (dynamic_cast <CylinderExplosion *> 
                (explosion))->cylinderPoint1 = atts.GetCylinderPoint1();
            (dynamic_cast <CylinderExplosion *> 
                (explosion))->cylinderPoint2 = atts.GetCylinderPoint2();
            (dynamic_cast <CylinderExplosion *> 
                (explosion))->cylinderRadius = atts.GetCylinderRadius();
        }
        break;
        default:
        {
            char recieved[256];
            char expected[256];
            sprintf(expected, "A known explosion type (0-2)");
            sprintf(recieved, "%d", atts.GetExplosionType());
            EXCEPTION2(UnexpectedValueException, expected, recieved);
            SetOutputDataTree(NULL); 
        }
        break;
    }

    explosion->materialName         = atts.GetMaterial();
    explosion->matExplosionFactor   = atts.GetMaterialExplosionFactor();
    explosion->explosionPattern     = atts.GetExplosionPattern();

    if (atts.GetExplodeMaterialCells() && !atts.GetExplodeAllCells())
    {
        explosion->explodeMaterialCells = true;
        explosion->matExplosionFactor   = atts.GetMaterialExplosionFactor();
        explosion->cellExplosionFactor  = atts.GetCellExplosionFactor();
    }
    else if (atts.GetExplodeAllCells())
    {
        explosion->cellExplosionFactor = atts.GetCellExplosionFactor();
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
    if (atts.GetExplodeAllCells() || 
        atts.GetBoundaryNames().empty())
    {
        int nLeaves;
        vtkDataSet **inLeaves = inTree->GetAllLeaves(nLeaves);
        avtDataRepresentation **outLeaves = 
            new avtDataRepresentation*[nLeaves];
        stringVector outLabels(nLeaves, "");

        if (inLabels.size() == nLeaves)
        {
            outLabels = inLabels;
        }

        for (int i = 0; i < nLeaves; ++i)
        {
            vtkUnstructuredGrid *new_leaf = vtkUnstructuredGrid::New();
            explosion->ExplodeAllCells(inLeaves[i], new_leaf, scaleFactor);
            avtDataRepresentation *leaf_rep = 
                new avtDataRepresentation(new_leaf, i, outLabels[i]);
            outLeaves[i] = leaf_rep;
        }

        avtDataTree_p outTree = new avtDataTree(nLeaves, outLeaves);

        for (int i = 0; i < nLeaves; ++i)
        {
            if (outLeaves[i] != NULL)
            {
                delete outLeaves[i];
            }
        }
        delete [] outLeaves;

        SetOutputDataTree(outTree); 
        return;
    }

    //
    // We have materials => we need to extract them from
    // all domains. 
    //
    avtDataTree_p materialTree = 
        ExtractMaterialsFromDomains(inTree);
   
#ifdef PARALLEL

    //
    // If we're in parallel, we need to update 
    // material extents across all processors.
    //
    if (PAR_Size() > 1)
    {
        int numExtents = 6 * atts.GetBoundaryNames().size();
        int half       = numExtents / 2;

        double *curMinExtents = new double[half];
        double *curMaxExtents = new double[half];
        for (int i = 0; i < half; ++i)
        {
            int idx = i*2;
            curMinExtents[i] = globalMatExtents[idx];
            curMaxExtents[i] = globalMatExtents[idx + 1];
        }
 
        double *trueMinExtents = new double[half];
        double *trueMaxExtents = new double[half];

        MPI_Allreduce(&curMinExtents[0], &trueMinExtents[0], half,
            MPI_DOUBLE, MPI_MIN, VISIT_MPI_COMM);
        MPI_Allreduce(&curMaxExtents[0], &trueMaxExtents[0], half,
            MPI_DOUBLE, MPI_MAX, VISIT_MPI_COMM);
        
        for (int i = 0; i < half; ++i)
        {
            int idx = i*2;
            globalMatExtents[idx]     = trueMinExtents[i];
            globalMatExtents[idx + 1] = trueMaxExtents[i];
        }
        delete [] curMinExtents;
        delete [] curMaxExtents;
        delete [] trueMinExtents;
        delete [] trueMaxExtents;
    }

#endif

    if (materialTree == NULL)
    {
        debug1 << "ExtractMaterialsFromDomains returned a NULL materialTree..." 
            << endl;
        SetOutputDataTree(materialTree); 
        return;
    }

    int nLeaves;
    stringVector matLabels; 
    std::vector<int> materialDomains;
    materialTree->GetAllDomainIds(materialDomains);
    materialTree->GetAllLabels(matLabels);
    vtkDataSet **matLeaves = materialTree->GetAllLeaves(nLeaves);

    if (nLeaves == 0)
    {
        delete [] matLeaves;
        SetOutputDataTree(materialTree); 
        return;
    }
        
    if (matLabels.size() < nLeaves)
    {
        char expected[256];
        char recieved[256];
        sprintf(expected, "number of labels to be >= "
            "number of leaves");
        sprintf(recieved, "Num labels: %d  Num leaves: %d  ", 
            matLabels.size(), nLeaves);
        EXCEPTION2(UnexpectedValueException, expected, recieved);
        SetOutputDataTree(NULL); 
        return;
    }

    avtDataRepresentation **outLeaves = new avtDataRepresentation*[nLeaves];

    //
    // Look for the material that has been selected for
    // explosion, and blow it up. 
    //
    for (int i = 0; i < nLeaves; ++i)
    {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->DeepCopy(matLeaves[i]);

        if ( explosion->materialName == matLabels[i] )
        {
            double matExtents[6]; 
            int matIdx = GetMaterialIndex(matLabels[i]);
  
            if (matIdx < 0)
            {
                SetOutputDataTree(NULL); 
                return;
            }

            for (int i = 0; i < 6; ++i)
            {
                matExtents[i] = globalMatExtents[matIdx + i];
            }
                
            if (explosion->explodeMaterialCells)
            {
                explosion->ExplodeAndDisplaceMaterial(ugrid, 
                    matExtents, scaleFactor);
            }
            else
            {
                explosion->DisplaceMaterial(ugrid, matExtents, 
                    scaleFactor);
            }
        }

        //
        //TODO: this creates a tree with more leaves than 
        //      the original input tree (each domain is split
        //      into material leaves). May want to merge materials
        //      back into domains, but this will cause materials
        //      to be re-calculated when more explosions are added... 
        //
        avtDataRepresentation *ugrid_rep = 
            new avtDataRepresentation(ugrid, materialDomains[i], inLabels[0]);
        outLeaves[i] = ugrid_rep;
    }
        
    avtDataTree_p outTree = new avtDataTree(nLeaves, outLeaves);

    //
    // Clean up memory.
    //
    for (int i = 0; i < nLeaves; ++i)
    {
        if (outLeaves[i] != NULL)
        {
            delete outLeaves[i];
        }
    }
    delete [] outLeaves;

    avtDataTree_p mergedDomainTree = MergeDomains(outTree, inLabels);
    SetOutputDataTree(mergedDomainTree);
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
// ****************************************************************************

avtContract_p   
avtExplodeFilter::ModifyContract(avtContract_p contract)
{
    avtContract_p rv = new avtContract(contract);

    rv->GetDataRequest()->ForceMaterialInterfaceReconstructionOn();
    rv->GetDataRequest()->ForceMaterialLabelsConstructionOn();
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
Explosion::ExplodeAndDisplaceMaterial(vtkUnstructuredGrid *ugrid,  
                                      double *matExtents, 
                                      double scaleFactor)
{
    double dataCenter[3];
    for (int i = 0; i < 3; ++i)
        dataCenter[i] = (matExtents[i*2] + matExtents[(i*2)+1]) / 2.0;

    int numPoints        = ugrid->GetNumberOfPoints();
    vtkPoints *ugridPts  = ugrid->GetPoints();
    vtkPoints *newPoints = vtkPoints::New();
    vtkIdList *cellPts   = vtkIdList::New();
    newPoints->DeepCopy(ugridPts);

    //
    // First, we need to calculate a displacement of the
    // entire material and copy this into a container. 
    //
    double initialDisplacement[3];
    CalcDisplacement(dataCenter, matExplosionFactor, scaleFactor, false);
    for (int i = 0; i < 3; ++i)
    {
        initialDisplacement[i] = displaceVec[i];
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
            scaleFactor, normalize); 

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
               point[j] += displaceVec[j] + initialDisplacement[j];
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
//      ugrid      The output dataset. 
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
    
    int numPoints       = in_ds->GetNumberOfPoints();
    int numCells        = in_ds->GetNumberOfCells();
    vtkCellIterator *it = in_ds->NewCellIterator();
    vtkPoints *pts      = vtkPoints::New();
    vtkIdList *cellPts  = vtkIdList::New();
    vtkIdList *ptIds    = vtkIdList::New();

    // 
    // Since we are removing connectivity between nodes, 
    // we will need more space to store them. The following
    // is a rough (usually over) estimation which imagines
    // that each cell could be a  VTK VOXEL or HEXAHEDRON 
    // type (8 nodes). We squeeze out the extra space later. 
    // NOTE: VTK has less common cell types that can have 
    // as many as 19 nodes, in which case this could actually 
    // be an under-estimation...
    //
    outPD->Allocate(numCells * 8);
    pts->Allocate(numCells * 8);
    ptIds->Allocate(numCells * 8);
    cellPts->Allocate(numCells);

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
            scaleFactor, normalize); 

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
//
//      scaleFactor    A factor to scale by. 
//
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
// ****************************************************************************

PointExplosion::PointExplosion() 
{
    explosionPoint = NULL;
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
//
//      expFactor     A factor to displace the data by. 
//
//      scaleFactor   A factor to scale the exposion by. 
//
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
// ****************************************************************************

void
PointExplosion::CalcDisplacement(double *dataCenter, double expFactor, 
                                 double scaleFactor, bool normalize)
{
    if (explosionPoint == NULL)
    {
        char recieved[256];
        char expected[256];
        sprintf(expected, "explosionPoint to be non-NULL");
        sprintf(recieved, "NULL explosionPoint");
        EXCEPTION2(UnexpectedValueException, expected, recieved);
        return;
    }

    //
    //  Find the distance between the data center and the
    //  explosion point.
    //
    for (int i = 0; i < 3; ++i)
    {
        displaceVec[i] = dataCenter[i] - explosionPoint[i];
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
// ****************************************************************************

PlaneExplosion::PlaneExplosion() 
{
    planePoint = NULL;
    planeNorm  = NULL;
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
//
//      expFactor     A factor to displace the data by. 
//
//      scaleFactor   A factor to scale the exposion by. 
//
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
// ****************************************************************************

void
PlaneExplosion::CalcDisplacement(double *dataCenter, double expFactor, 
                                 double scaleFactor, bool normalize)
{
    if (planePoint == NULL)
    {
        char recieved[256];
        char expected[256];
        sprintf(expected, "planePoint to be non-NULL");
        sprintf(recieved, "NULL planePoint");
        EXCEPTION2(UnexpectedValueException, expected, recieved);
        return;
    }
    else if (planeNorm == NULL)
    {
        char recieved[256];
        char expected[256];
        sprintf(expected, "planeNorm to be non-NULL");
        sprintf(recieved, "NULL planeNorm");
        EXCEPTION2(UnexpectedValueException, expected, recieved);
        return;
    }

    //
    // Project from our data center onto a plane. 
    //
    double denom = 0.0;
    double alpha = 0.0;
    for (int i = 0; i < 3; ++i)
    {
        alpha += (planeNorm[i] * planePoint[i]) - 
            (planeNorm[i] * dataCenter[i]);
        denom += planeNorm[i] * planeNorm[i];
    }

    alpha /= denom;

    //
    // Subtract the projection from the data center
    // to get the distance from the plane. 
    //
    for (int i = 0; i < 3; ++i)
    {
        displaceVec[i] = dataCenter[i] - 
            (dataCenter[i] + (alpha * planeNorm[i]));
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
// ****************************************************************************

CylinderExplosion::CylinderExplosion() 
{
    cylinderPoint1 = NULL;
    cylinderPoint2 = NULL;
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
//
//      expFactor     A factor to displace the data by. 
//
//      scaleFactor   A factor to scale the exposion by. 
//
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
// ****************************************************************************

void
CylinderExplosion::CalcDisplacement(double *dataCenter, double expFactor, 
                                    double scaleFactor, bool normalize)
{
    if (cylinderPoint1 == NULL)
    {
        char recieved[256];
        char expected[256];
        sprintf(expected, "cylinderPoint1 to be non-NULL");
        sprintf(recieved, "NULL cylinderPoint1");
        EXCEPTION2(UnexpectedValueException, expected, recieved);
        return;
    }
    else if (cylinderPoint2 == NULL)
    {
        char recieved[256];
        char expected[256];
        sprintf(expected, "cylinderPoint2 to be non-NULL");
        sprintf(recieved, "NULL cylinderPoint2");
        EXCEPTION2(UnexpectedValueException, expected, recieved);
        return;
    }

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

    //
    // If the data is within the cylinder's radius, we don't want to 
    // displace it, so just set the displacement to 0.0 and return. 
    //
    double dist = 0.0;
    for (int i = 0; i < 3; ++i)
    {
        dist += (dataCenter[i] - projection[i]) * 
            (dataCenter[i] - projection[i]);
    }
    if (dist <= cylinderRadius)
    {
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
