// ************************************************************************* //
//                         avtRelevantPointsFilter.C                         //
// ************************************************************************* //

#include <avtRelevantPointsFilter.h>

#include <vtkPolyData.h>
#include <vtkPolyDataRelevantPointsFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridRelevantPointsFilter.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtRelevantPointsFilter constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 07, 2000
//
//  Modifications:
//
// ****************************************************************************

avtRelevantPointsFilter::avtRelevantPointsFilter()
{
    rpfPD = vtkPolyDataRelevantPointsFilter::New();
    rpfUG = vtkUnstructuredGridRelevantPointsFilter::New();
}


// ****************************************************************************
//  Method: avtRelevantPointsFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 07, 2000
//
//  Modifications:
//
// ****************************************************************************

avtRelevantPointsFilter::~avtRelevantPointsFilter()
{
    rpfPD->Delete();
    rpfPD = NULL;
    rpfUG->Delete();
    rpfUG = NULL;
}


// ****************************************************************************
//  Method: avtRelevantPointsFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the RelevantPoints filter.
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
// ****************************************************************************

vtkDataSet *
avtRelevantPointsFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    int  nPoints = in_ds->GetNumberOfPoints();
    int  nCells  = in_ds->GetNumberOfCells();
    if (2*nCells > nPoints)
    {
        debug5 << "Relevant points filter stopped by heuristic.  Points = "
               << nPoints << ", cells = " << nCells << endl;
        return in_ds;
    }
    else
    {
        debug5 << "Relevant points filter allowed to execute by heuristic.  " 
               << "Points = " << nPoints << ", cells = " << nCells << endl;
    }

    vtkDataSet *out_ds = NULL;  
    vtkPolyData *out_pd = NULL;
    vtkUnstructuredGrid *out_ug = NULL;

    if (in_ds->GetNumberOfCells() == 0)
    {
        out_ds = in_ds;
    }
    else
    {
        switch(in_ds->GetDataObjectType())
        {
            case VTK_POLY_DATA:
                rpfPD->SetInput((vtkPolyData*)in_ds);
                out_pd = vtkPolyData::New();
                rpfPD->SetOutput(out_pd);
                out_pd->Delete();
                rpfPD->Update();
                out_ds = (vtkDataSet*)out_pd;
                break;

            case VTK_UNSTRUCTURED_GRID:
                rpfUG->SetInput((vtkUnstructuredGrid*)in_ds);
                out_ug = vtkUnstructuredGrid::New();
                rpfUG->SetOutput(out_ug);
                out_ug->Delete();
                rpfUG->Update();
                out_ds = (vtkDataSet*)out_ug;
                break;

            default :
            // We don't know what type this is.  It is probably a mistake that
            // this was called, so minimize the damage by passing this through.
                out_ds = in_ds;
                break;
        }
    }
    return out_ds;
}


// ****************************************************************************
//  Method: avtRelevantPointsFilter::ReleaseData
//
//  Purpose:
//      Releases all of the problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
// ****************************************************************************

void
avtRelevantPointsFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    rpfPD->SetInput(NULL);
    rpfPD->SetOutput(NULL);
    rpfUG->SetInput(NULL);
    rpfUG->SetOutput(NULL);
}


