// ************************************************************************* //
//                         avtCondenseDatasetFilter.C                        //
// ************************************************************************* //

#include <avtCondenseDatasetFilter.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataRelevantPointsFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridRelevantPointsFilter.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtCondenseDatasetFilter constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 07, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Nov 12 18:26:21 PST 2003
//    Initialize keepAVTandVTK.
//
//    Kathleen Bonnell, Wed Apr 14 17:51:36 PDT 2004 
//    Initialize bypassHeuristic.
//
// ****************************************************************************

avtCondenseDatasetFilter::avtCondenseDatasetFilter()
{
    rpfPD = vtkPolyDataRelevantPointsFilter::New();
    rpfUG = vtkUnstructuredGridRelevantPointsFilter::New();
    keepAVTandVTK = false;
    bypassHeuristic = false;
}


// ****************************************************************************
//  Method: avtCondenseDatasetFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 07, 2000
//
//  Modifications:
//
// ****************************************************************************

avtCondenseDatasetFilter::~avtCondenseDatasetFilter()
{
    rpfPD->Delete();
    rpfPD = NULL;
    rpfUG->Delete();
    rpfUG = NULL;
}


// ****************************************************************************
//  Method: avtCondenseDatasetFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the CondenseDataset filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//
//  Returns:       The output dataset.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 07, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Mar  2 17:31:01 PST 2001
//    Added check for input datasets with no cells
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Renamed method from ExecuteDomain to ExecuteData. 
//
//    Hank Childs, Thu Sep 20 17:53:57 PDT 2001
//    Added heuristic to not apply the filter when it won't rule out very many
//    irrelevant points.
//
//    Hank Childs, Fri Jan 25 10:18:32 PST 2002
//    Fix memory leak.
//
//    Hank Childs, Fri Jul 25 21:25:31 PDT 2003
//    Removed unneeded variables.
//
//    Kathleen Bonnell, Wed Nov 12 18:26:21 PST 2003 
//    Allow AVT and VTK variables to be kept if requested. 
//
//    Kathleen Bonnell, Wed Apr 14 17:51:36 PDT 2004 
//    Allow execution of relevant points filter to be forced, bypassing
//    heurisitic. 
//
//    Kathleen Bonnell, Tue Oct 12 16:06:20 PDT 2004
//    Allow avtOriginalNodeNumbers and avtOriginalCellNumbers to be kept
//    around, independent of the value of keepAVTandVTK.  Needed by pick for
//    Vector Plots and Point Meshes.
//
//    Brad Whitlock, Wed Dec 22 11:20:42 PDT 2004
//    Added an heuristic to skip relevant points for line data that has 
//    many more points than cells. Also set the flag to skip relevant points
//    for point meshes so it agrees with the debug log message about skipping
//    relevant points.
//
// ****************************************************************************

vtkDataSet *
avtCondenseDatasetFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    int i;

    //
    // Remove any variable that has "VTK" or "AVT" in its name.
    //
    vtkDataSet *no_vars = (vtkDataSet *) in_ds->NewInstance();
    no_vars->ShallowCopy(in_ds);
    if (!keepAVTandVTK)
    {
        bool keepNodeZone = 
            GetInput()->GetInfo().GetAttributes().GetKeepNodeZoneArrays();
        for (i = no_vars->GetPointData()->GetNumberOfArrays()-1 ; i >= 0 ; i--)
        {
            vtkDataArray *arr = no_vars->GetPointData()->GetArray(i);
            const char *name = arr->GetName();
            if (name == NULL)
                continue;
            if (strstr(name, "vtk") != NULL)
                no_vars->GetPointData()->RemoveArray(name);
            else if (strstr(name, "avt") != NULL)
            {
                if (keepNodeZone && 
                    ((strcmp(name, "avtOriginalNodeNumbers") == 0) ||
                     (strcmp(name, "avtOriginalCellNumbers") == 0)))
                    continue;
                else
                    no_vars->GetPointData()->RemoveArray(name);
            }
        }
        for (i = no_vars->GetCellData()->GetNumberOfArrays()-1 ; i >= 0 ; i--)
        {
            vtkDataArray *arr = no_vars->GetCellData()->GetArray(i);
            const char *name = arr->GetName();
            if (name == NULL)
                continue;
            if (strstr(name, "vtk") != NULL)
                no_vars->GetCellData()->RemoveArray(name);
            else if (strstr(name, "avt") != NULL)
            {
                if (keepNodeZone && 
                    ((strcmp(name, "avtOriginalNodeNumbers") == 0) ||
                     (strcmp(name, "avtOriginalCellNumbers") == 0)))
                    continue;
                else
                    no_vars->GetCellData()->RemoveArray(name);
            }
        }
    }
 
    //
    // Decide whether or not we need to determine the relevant points.
    //
    int  nPoints = no_vars->GetNumberOfPoints();
    int  nCells  = no_vars->GetNumberOfCells();
    bool shouldTakeRelevantPoints = true;
    if (!bypassHeuristic && 2*nCells > nPoints)
    {
        debug5 << "Relevant points filter stopped by heuristic.  Points = "
               << nPoints << ", cells = " << nCells << endl;
        shouldTakeRelevantPoints = false;
    }
    else if (!bypassHeuristic &&
             GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() 
             == 0)
    {
        debug5 << "Not taking relevant points because we have a point mesh."
               << endl;
        shouldTakeRelevantPoints = false;
    }
    else if (!bypassHeuristic &&
             GetInput()->GetInfo().GetAttributes().GetTopologicalDimension()==1&&
             nPoints > (nCells*5))
    {
        debug5 << "Not taking relevant points because we have a mesh "
                  "consisting of lines and the number of points is more "
                  "than 5x the number of cells. This is common with "
                  "streamlines and we need to skip because the relevant "
                  "points filter tends to die right now with line data of "
                  "this kind."
               << endl;
        shouldTakeRelevantPoints = false;
    }
    else
    {
        if (!bypassHeuristic)
        {
            debug5 << "Relevant points filter allowed to execute by heuristic.  " 
                   << "Points = " << nPoints << ", cells = " << nCells << endl;
        }
        else 
        {
            debug5 << "Relevant points filter forced to execute." << endl; 
        }
    }

    vtkDataSet *out_ds = NULL;  
    if (shouldTakeRelevantPoints)
    {
        vtkPolyData *out_pd = NULL;
        vtkUnstructuredGrid *out_ug = NULL;

        if (no_vars->GetNumberOfCells() == 0)
        {
            out_ds = no_vars;
        }
        else
        {
            switch(no_vars->GetDataObjectType())
            {
                case VTK_POLY_DATA:
                    rpfPD->SetInput((vtkPolyData*)no_vars);
                    out_pd = vtkPolyData::New();
                    rpfPD->SetOutput(out_pd);
                    out_pd->Delete();
                    rpfPD->Update();
                    out_ds = (vtkDataSet*)out_pd;
                    break;
    
                case VTK_UNSTRUCTURED_GRID:
                    rpfUG->SetInput((vtkUnstructuredGrid*)no_vars);
                    out_ug = vtkUnstructuredGrid::New();
                    rpfUG->SetOutput(out_ug);
                    out_ug->Delete();
                    rpfUG->Update();
                    out_ds = (vtkDataSet*)out_ug;
                    break;
    
                default :
                // We don't know what type this is.  It is probably a mistake that
                // this was called, so minimize the damage by passing this through.
                    out_ds = no_vars;
                    break;
            }
        }
    }
    else
    {
        out_ds = no_vars;
    }

    ManageMemory(out_ds);
    no_vars->Delete();

    return out_ds;
}


// ****************************************************************************
//  Method: avtCondenseDatasetFilter::ReleaseData
//
//  Purpose:
//      Releases all of the problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Mar  4 08:12:25 PST 2005
//    Do not set outputs of filters to NULL, since this will prevent them
//    from re-executing correctly in DLB-mode.
//
//    Hank Childs, Fri Mar 11 07:37:05 PST 2005
//    Fix non-problem size leak introduced with last fix.
//
// ****************************************************************************

void
avtCondenseDatasetFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    rpfPD->SetInput(NULL);
    vtkPolyData *p = vtkPolyData::New();
    rpfPD->SetOutput(p);
    p->Delete();

    rpfUG->SetInput(NULL);
    vtkUnstructuredGrid *u = vtkUnstructuredGrid::New();
    rpfUG->SetOutput(u);
    u->Delete();
}


