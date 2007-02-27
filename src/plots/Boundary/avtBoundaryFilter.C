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
//                            avtBoundaryFilter.C                            // 
// ************************************************************************* // 


#include <avtBoundaryFilter.h>
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
//  Method: avtBoundaryFilter constructor
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 10, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtBoundaryFilter::avtBoundaryFilter()
{
    keepNodeZone = false; 
}


// ****************************************************************************
//  Method: avtBoundaryFilter::SetPlotAtts
//
//  Purpose:    Sets the BoundaryAttributes needed for this filter.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

void
avtBoundaryFilter::SetPlotAtts(const BoundaryAttributes *atts)
{
    plotAtts = *atts;
}


// ****************************************************************************
//  Method: avtBoundaryFilter::ExecuteDataTree
//
//  Purpose:    Break up the dataset into a collection of datasets, one
//              per boundary.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 12, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Jun 21 10:51:29 PDT 2003
//    Made more efficient by removing unnecessary work.
//
//    Kathleen Bonnell, Mon Sep  8 13:43:30 PDT 2003 
//    Add test for NO cells for early termination.
//
// ****************************************************************************

avtDataTree_p
avtBoundaryFilter::ExecuteDataTree(vtkDataSet *in_ds, int domain, string label)
{
    if (in_ds == NULL || in_ds->GetNumberOfPoints() == 0 ||
        in_ds->GetNumberOfCells() == 0)
    {
        return NULL;
    }

    vector<string> labels;
    int            nDataSets = 0;
    vtkDataSet   **out_ds = NULL;

    vtkDataArray *boundaryArray = in_ds->GetCellData()->GetArray("avtSubsets");

    // the materials will never start split for this case
    bool splitMats = false;

    if (boundaryArray && !splitMats)
    {
        if (label.find(";") == string::npos)
        {
            debug1 << "POSSIBLE ERROR CONDITION:  " << endl;
            debug1 << "    avtBoundaryFilter encountered a label ("
                   << label.c_str() << ")" << endl;
            debug1 << "    that cannot be parsed correctly.  This can happen "
                   << "if" << endl;
            debug1 << "    another filter has over-written the boundary labels "
                   << "in" << endl;
            debug1 << "    its output data tree.  avtBoundaryFilter is returning"
                   << endl;
            debug1 << "    an empty data tree." << endl;
            avtDataTree_p rv = new avtDataTree();
            return rv;
        }
        //
        // Break up the dataset into a collection of datasets, one
        // per boundary.
        //
        int *boundaryList = ((vtkIntArray*)boundaryArray)->GetPointer(0);

        if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
        {
            EXCEPTION0(ImproperUseException);
        }

        vtkPolyData *in_pd = (vtkPolyData *)in_ds;
        int ntotalcells = in_pd->GetNumberOfCells();

        vtkCellData *in_CD = in_ds->GetCellData();

        //
        // Determine the total number of boundaries, the number that
        // were selected, and the labels for the boundaries.
        //
        char *cLabelStorage = new char[label.length()+1];
        strcpy(cLabelStorage, label.c_str());
        char *cLabel = cLabelStorage;

        int nSelectedBoundaries;
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
        for (i = 0; i < ntotalcells; i++)
        {
            boundaryCounts[boundaryList[i]]++;
        }

        //
        // Create a dataset for each boundary.
        //
        out_ds = new vtkDataSet *[nSelectedBoundaries];

        //
        // The following call is a workaround for a VTK bug.  It turns
        // out that when GetCellType if called for the first time for a
        // PolyData it calls its BuildCells method which causes the iterator
        // used by InitTraversal and GetNextCell to be put at the end of
        // the list.
        //
        in_pd->GetCellType(0);
        
        for (i = 0; i < nSelectedBoundaries; i++)
        {
            int s = selectedBoundaries[i];

            if (boundaryCounts[s] > 0)
            {
                // Create a new polydata
                vtkPolyData *out_pd = vtkPolyData::New();

                // Copy the points and point data.
                out_pd->SetPoints(in_pd->GetPoints());
                out_pd->GetPointData()->PassData(in_pd->GetPointData());

                // Prepare cell data for copy. 
                vtkCellData *out_CD = out_pd->GetCellData();
                out_CD->CopyAllocate(in_CD, boundaryCounts[s]);

                // insert the cells
                out_pd->Allocate(boundaryCounts[s]);

                vtkIdType npts, *pts;
                int numNewCells = 0;
                for (int j = 0; j < ntotalcells; j++)
                {
                    if (boundaryList[j] == s)
                    {
                        in_pd->GetCellPoints(j, npts, pts);
                        out_pd->InsertNextCell(in_pd->GetCellType(j),
                                               npts, pts);
                        out_CD->CopyData(in_CD, j, numNewCells++); 
                    }
                }
                out_CD->RemoveArray("avtSubsets");
                labels.push_back(selectedBoundaryNames[i]);
                out_ds[nDataSets] = out_pd;
                nDataSets++;
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
        // The dataset represents a single boundary, so just turn it into
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
//  Method: avtBoundaryFilter::RefashionDataObjectInfo
//
//  Purpose:  Retrieves the boundary names from the plot attributes and
//            sets them as labels in the data attributes. 
//
//  Programmer: Jeremy Meredith
//  Creation:   June 10, 2003
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 10:23:09 PST 2004
//    Added call to SetKeepNodeZoneArrays.
//
// ****************************************************************************

void
avtBoundaryFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetLabels(plotAtts.GetBoundaryNames());
    outAtts.SetKeepNodeZoneArrays(keepNodeZone);
}

 
// ****************************************************************************
//  Method: avtBoundaryFilter::PerformRestriction
//
//  Purpose:  Turn on domain labels in the data spec if needed.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 10, 2003
//
//  Modifications:
//    
//    Hank Childs, Wed Aug 13 07:55:35 PDT 2003
//    Explicitly tell data spec when to do MIR.
//
//    Kathleen Bonnell, Fri Nov 12 10:23:09 PST 2004
//    If working with a point mesh (topodim == 0), then determine if a point
//    size var secondary variable needs to be added to the pipeline, and
//    whether or not we need to keep Node and Zone numbers around. 
//
// ****************************************************************************

avtPipelineSpecification_p
avtBoundaryFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    if (plotAtts.GetBoundaryType() == BoundaryAttributes::Material)
    {
        spec->GetDataSpecification()->ForceMaterialInterfaceReconstructionOn();
    }
    spec->GetDataSpecification()->TurnBoundarySurfacesOn();

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        string pointVar = plotAtts.GetPointSizeVar();
        avtDataSpecification_p dspec = spec->GetDataSpecification();

        //
        // Find out if we REALLY need to add the secondary variable.
        //
        if (plotAtts.GetPointSizeVarEnabled() && 
            pointVar != "default" &&
            pointVar != "\0" &&
            pointVar != dspec->GetVariable() &&
            !dspec->HasSecondaryVariable(pointVar.c_str()))
        {
            spec->GetDataSpecification()->AddSecondaryVariable(pointVar.c_str());
        }

        avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();
        if (spec->GetDataSpecification()->MayRequireZones())
        {
            keepNodeZone = true;
            spec->GetDataSpecification()->TurnNodeNumbersOn();
        }
        else
        {
            keepNodeZone = false;
        }
    }

    return spec;
}

// ****************************************************************************
//  Method: avtBoundaryFilter::PostExcecute
//
//  Purpose:  
//    Sets the output's label attributes to reflect what is currently
//    present in the tree.  
//
//  Programmer: Jeremy Meredith
//  Creation:   May  7, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//    
// ****************************************************************************

void
avtBoundaryFilter::PostExecute(void)
{
    vector <string> treeLabels;
    GetDataTree()->GetAllUniqueLabels(treeLabels);
    GetOutput()->GetInfo().GetAttributes().SetLabels(treeLabels);
}
