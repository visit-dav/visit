// ************************************************************************* //
//                        avtFilledBoundaryFilter.C                                  // 
// ************************************************************************* // 


#include <avtFilledBoundaryFilter.h>
#include <avtDataAttributes.h>
#include <avtTerminatingSource.h>
#include <vtkCellArray.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <DebugStream.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtFilledBoundaryFilter::SetPlotAtts
//
//  Purpose:    Sets the FilledBoundaryAttributes needed for this filter.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

void
avtFilledBoundaryFilter::SetPlotAtts(const FilledBoundaryAttributes *atts)
{
    plotAtts = *atts;
}


// ****************************************************************************
//  Method: avtFilledBoundaryFilter::ExecuteDataTree
//
//  Purpose:    Break up the dataset into a collection of datasets, one
//              per boundary.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
// ****************************************************************************

avtDataTree_p
avtFilledBoundaryFilter::ExecuteDataTree(vtkDataSet *in_ds, int domain, string label)
{
    if (in_ds == NULL || in_ds->GetNumberOfPoints() == 0)
    {
        return NULL;
    }

    vector<string> labels;
    int            nDataSets = 0;
    vtkDataSet   **out_ds = NULL;

    vtkDataArray *boundaryArray = in_ds->GetCellData()->GetArray("avtSubsets");

    bool splitMats = plotAtts.GetDrawInternal();
    if (boundaryArray && !splitMats)
    {
        if (label.find(";") == string::npos)
        {
            debug1 << "POSSIBLE ERROR CONDITION:  " << endl;
            debug1 << "    avtFilledBoundaryFilter encountered a label ("
                   << label.c_str() << ")" << endl;
            debug1 << "    that cannot be parsed correctly.  This can happen "
                   << "if" << endl;
            debug1 << "    another filter has over-written the boundary labels "
                   << "in" << endl;
            debug1 << "    its output data tree.  avtFilledBoundaryFilter is returning"
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
        // Determine the total number of boundarys, the number that
        // were selected, and the labels for the boundarys.
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

        vtkCellArray *cells = in_pd->GetPolys();

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
                cells->InitTraversal();
                int numNewCells = 0;
                for (int j = 0; j < ntotalcells; j++)
                {
                    cells->GetNextCell(npts, pts);
                    if (boundaryList[j] == s)
                    {
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
//  Method: avtFilledBoundaryFilter::RefashionDataObjectInfo
//
//  Purpose:  Retrieves the boundary names from the plot attributes and
//            sets them as labels in the data attributes. 
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
// ****************************************************************************

void
avtFilledBoundaryFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetLabels(plotAtts.GetBoundaryNames());
}

 
// ****************************************************************************
//  Method: avtFilledBoundaryFilter::PerformRestriction
//
//  Purpose:  Turn on domain labels in the data spec if needed.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 13, 2003
//
//  Modifications:
//    
// ****************************************************************************

avtPipelineSpecification_p
avtFilledBoundaryFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    if (plotAtts.GetDrawInternal())
    {
        spec->GetDataSpecification()->TurnInternalSurfacesOn();
    }
    if (plotAtts.GetCleanZonesOnly())
    {
        spec->GetDataSpecification()->SetNeedCleanZonesOnly(true);
    }
    return spec;
}

// ****************************************************************************
//  Method: avtFilledBoundaryFilter::PostExcecute
//
//  Purpose:  
//    Sets the output's label attributes to reflect what is currently
//    present in the tree.  
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2003
//
//  Note:  taken almost verbatim from the Subset plot
//
//  Modifications:
//    
// ****************************************************************************

void
avtFilledBoundaryFilter::PostExecute(void)
{
    vector <string> treeLabels;
    GetDataTree()->GetAllUniqueLabels(treeLabels);
    GetOutput()->GetInfo().GetAttributes().SetLabels(treeLabels);
}
