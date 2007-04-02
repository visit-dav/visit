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
//                        avtLabelSubsetsFilter.C                            // 
// ************************************************************************* // 


#include <avtLabelSubsetsFilter.h>
#include <avtDataAttributes.h>
#include <avtTerminatingSource.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <DebugStream.h>
#include <ImproperUseException.h>



// ****************************************************************************
//  Method: avtLabelSubsetsFilter constructor
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtLabelSubsetsFilter::avtLabelSubsetsFilter()
{
    needMIR = false;
}


// ****************************************************************************
//  Method: avtLabelSubsetsFilter::ExecuteDataTree
//
//  Purpose:    Break up the dataset into a collection of datasets, one
//              per subset.
//
//  Programmer: Eric Brugger
//  Creation:   December 14, 2001 
//
//  Modifications:
//    
// ****************************************************************************

avtDataTree_p
avtLabelSubsetsFilter::ExecuteDataTree(vtkDataSet *in_ds, int domain, string label)
{
    if (in_ds == NULL || in_ds->GetNumberOfPoints() == 0 ||
        in_ds->GetNumberOfCells() == 0)
    {
        return NULL;
    }

    vector<string> labels;
    int            nDataSets = 0;
    vtkDataSet   **out_ds = NULL;

    vtkDataArray *subsetArray = in_ds->GetCellData()->GetArray("avtSubsets");

    if (subsetArray)
    {
        if (label.find(";") == string::npos)
        {
            debug1 << "POSSIBLE ERROR CONDITION:  " << endl;
            debug1 << "    avtLabelSubsetsFilter encountered a label ("
                   << label.c_str() << ")" << endl;
            debug1 << "    that cannot be parsed correctly.  This can happen "
                   << "if" << endl;
            debug1 << "    another filter has over-written the subset labels "
                   << "in" << endl;
            debug1 << "    its output data tree.  avtLabelSubsetsFilter is returning"
                   << endl;
            debug1 << "    an empty data tree." << endl;
            avtDataTree_p rv = new avtDataTree();
            return rv;
        }
        //
        // Break up the dataset into a collection of datasets, one
        // per subset.
        //
        int *subsetList = ((vtkIntArray*)subsetArray)->GetPointer(0);

        if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
        {
            EXCEPTION0(ImproperUseException);
        }

        vtkPolyData *in_pd = (vtkPolyData *)in_ds;
        int ntotalcells = in_pd->GetNumberOfCells();

        vtkCellData *in_CD = in_ds->GetCellData();

        //
        // Determine the total number of subsets, the number that
        // were selected, and the labels for the subsets.
        //
        char *cLabelStorage = new char[label.length()+1];
        strcpy(cLabelStorage, label.c_str());
        char *cLabel = cLabelStorage;

        int nSelectedSubsets;
        sscanf(cLabel, "%d", &nSelectedSubsets);
        cLabel = strchr(cLabel, ';') + 1;

        int i, *selectedSubsets = new int[nSelectedSubsets];
        char **selectedSubsetNames = new char*[nSelectedSubsets];
        for (i = 0; i < nSelectedSubsets; i++)
        {
            sscanf(cLabel, "%d", &selectedSubsets[i]);
            cLabel = strchr(cLabel, ';') + 1;
            selectedSubsetNames[i] = cLabel;
            cLabel = strchr(cLabel, ';');
            cLabel[0] = '\0';
            cLabel = cLabel + 1;
        }

        int maxSubset = selectedSubsets[0];
        for (i = 1; i < nSelectedSubsets; i++)
        {
            maxSubset = selectedSubsets[i] > maxSubset ?
                        selectedSubsets[i] : maxSubset;
        }

        //
        // Count the number of cells of each subset.
        //
        int *subsetCounts = new int[maxSubset+1];
        for (int s = 0; s < maxSubset + 1; s++)
        {
            subsetCounts[s] = 0;
        }
        for (i = 0; i < ntotalcells; i++)
        {
            subsetCounts[subsetList[i]]++;
        }

        //
        // Create a dataset for each subset.
        //
        out_ds = new vtkDataSet *[nSelectedSubsets];

        //
        // The following call is a workaround for a VTK bug.  It turns
        // out that when GetCellType if called for the first time for a
        // PolyData it calls its BuildCells method which causes the iterator
        // used by InitTraversal and GetNextCell to be put at the end of
        // the list.
        //
        in_pd->GetCellType(0);
        
        for (i = 0; i < nSelectedSubsets; i++)
        {
            int s = selectedSubsets[i];

            if (subsetCounts[s] > 0)
            {
                // Create a new polydata
                vtkPolyData *out_pd = vtkPolyData::New();

                // Copy the points and point data.
                out_pd->SetPoints(in_pd->GetPoints());
                out_pd->GetPointData()->PassData(in_pd->GetPointData());

                // Prepare cell data for copy. 
                vtkCellData *out_CD = out_pd->GetCellData();
                out_CD->CopyAllocate(in_CD, subsetCounts[s]);

                // insert the cells
                out_pd->Allocate(subsetCounts[s]);

                vtkIdType npts, *pts;
                int numNewCells = 0;
                for (int j = 0; j < ntotalcells; j++)
                {
                    if (subsetList[j] == s)
                    {
                        in_pd->GetCellPoints(j, npts, pts);
                        out_pd->InsertNextCell(in_pd->GetCellType(j),
                                               npts, pts);
                        out_CD->CopyData(in_CD, j, numNewCells++); 
                    }
                }
                out_CD->RemoveArray("avtSubsets");
                labels.push_back(selectedSubsetNames[i]);
                out_ds[nDataSets] = out_pd;
                nDataSets++;
            }
        }

        delete [] subsetCounts;
        delete [] selectedSubsetNames;
        delete [] selectedSubsets;
        delete [] cLabelStorage;
    }
    else
    {
        //
        // The dataset represents a single subset, so just turn it into
        // a data tree.
        //
        labels.push_back(label);

        out_ds = new vtkDataSet *[1];
        out_ds[0] = in_ds;
        out_ds[0]->Register(NULL);  // This makes it symmetric with the 'if'
                                    // case so we can delete it blindly later.
        nDataSets = 1;
    }

    if (nDataSets == 0)
    {
        delete [] out_ds;

        return NULL;
    }

    avtDataTree_p outDT = new avtDataTree(nDataSets, out_ds, domain, labels);

    for (int i = 0 ; i < nDataSets ; i++)
    {
        if (out_ds[i] != NULL)
        {
            out_ds[i]->Delete();
        }
    }
    delete [] out_ds;

    return outDT;
}


// ****************************************************************************
//  Method: avtLabelSubsetsFilter::PerformRestriction
//
//  Purpose:  Turn on domain labels in the data spec if needed.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   Oct 18, 2001
//
//  Modifications:
//
// ****************************************************************************

avtPipelineSpecification_p
avtLabelSubsetsFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    if(needMIR)
        spec->GetDataSpecification()->ForceMaterialInterfaceReconstructionOn();

    return spec;
}


// ****************************************************************************
//  Method: avtLabelSubsetsFilter::PostExcecute
//
//  Purpose:  
//    Sets the output's label attributes to reflect what is currently
//    present in the tree.  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 29, 2002 
//
//  Modifications:
//    
// ****************************************************************************

void
avtLabelSubsetsFilter::PostExecute(void)
{
    vector <string> treeLabels;
    GetDataTree()->GetAllUniqueLabels(treeLabels);
    GetOutput()->GetInfo().GetAttributes().SetLabels(treeLabels);
#if 0
    debug4 << "avtLabelSubsetsFilter::PostExecute: Labels = " << endl;
    for(int i = 0; i < treeLabels.size(); ++i)
        debug4 << "\tlabel["<<i<<"] = " << treeLabels[i] << endl;
    debug4 << endl;
#endif
}
