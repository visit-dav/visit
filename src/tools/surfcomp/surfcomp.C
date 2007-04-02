#include <stdlib.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSetReader.h>
#include <vtkDataSetWriter.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>


// ****************************************************************************
//  Function: SurfMerge
//
//  Purpose:
//      Merges two surfaces.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2005
//
// ****************************************************************************

void
SurfMerge(int argc, char *argv[])
{
    int  i;

    if (argc != 5)
    {
        cerr << "Usage: "<<argv[0] << " -merge <ds-1> <ds-2> <output>" << endl;
        exit(EXIT_FAILURE);
    }

    vtkDataSetReader *rdr1 = vtkDataSetReader::New();
    rdr1->SetFileName(argv[2]);
    rdr1->Update();
    vtkDataSetReader *rdr2 = vtkDataSetReader::New();
    rdr2->SetFileName(argv[3]);
    rdr2->Update();

    vtkDataSet *ds1 = rdr1->GetOutput();
    vtkDataSet *ds2 = rdr2->GetOutput();
    if (ds1->GetNumberOfPoints() != ds2->GetNumberOfPoints())
    {
        cerr << "Different number of points" << endl;
        exit(EXIT_FAILURE);
    }
    if (ds1->GetNumberOfCells() != ds2->GetNumberOfCells())
    {
        cerr << "Different number of cells" << endl;
        exit(EXIT_FAILURE);
    }

    for (i = 0 ; i < ds2->GetPointData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *da = ds2->GetPointData()->GetArray(i);
        char name[1024];
        sprintf(name, "ds2_%s", da->GetName());
        da->SetName(name);
        ds1->GetPointData()->AddArray(da);
    }
    for (i = 0 ; i < ds2->GetCellData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *da = ds2->GetCellData()->GetArray(i);
        char name[1024];
        sprintf(name, "ds2_%s", da->GetName());
        da->SetName(name);
        ds1->GetCellData()->AddArray(da);
    }

    vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
    wrtr->SetInput(ds1);
    wrtr->SetFileName(argv[4]);
    wrtr->Write();
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Tue Feb  1 15:28:04 PST 2005
//    Added SurfMerge.
//
// ****************************************************************************

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "-merge") == 0)
    {
        SurfMerge(argc, argv);
        return 0;
    }

    if (argc != 5)
    {
        cerr << "Usage: " << argv[0] << " <surface-1> <surface-2> <A> <B>"
             << endl;
        cerr << "\tThe result is S1*A - S2 + B" << endl;
        cerr << "Or:  " << argv[0] << " -merge <ds-1> <ds-2> <output>" << endl;
        exit(EXIT_FAILURE);
    }

    vtkDataSetReader *rdr1 = vtkDataSetReader::New();
    rdr1->SetFileName(argv[1]);
    vtkDataSet *ds = rdr1->GetOutput();
    if (ds == NULL || ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        cerr << "Unable to read in " << argv[1] << endl;
        exit(EXIT_FAILURE);
    }
    vtkPolyData *pd1 = (vtkPolyData *) ds;

    vtkDataSetReader *rdr2 = vtkDataSetReader::New();
    rdr2->SetFileName(argv[2]);
    ds = rdr2->GetOutput();
    if (ds == NULL || ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        cerr << "Unable to read in " << argv[2] << endl;
        exit(EXIT_FAILURE);
    }
    vtkPolyData *pd2 = (vtkPolyData *) ds;

    if (pd1->GetNumberOfPoints() != pd2->GetNumberOfPoints())
    {
        cerr << "The surfaces do not have the same number of points." << endl;
        cerr << "They cannot be compared." << endl;
        exit(EXIT_FAILURE);
    }

    if (pd1->GetNumberOfCells() != pd2->GetNumberOfCells())
    {
        cerr << "The surfaces do not have the same cells." << endl;
        cerr << "They cannot be compared." << endl;
        exit(EXIT_FAILURE);
    }

    vtkPolyData *diffed = vtkPolyData::New();
    vtkPolyData *only1  = vtkPolyData::New();
    vtkPolyData *only2  = vtkPolyData::New();

    only1->SetPoints(pd1->GetPoints());
    only1->GetPointData()->SetScalars(pd1->GetPointData()->GetScalars());
    only2->SetPoints(pd2->GetPoints());
    only2->GetPointData()->SetScalars(pd1->GetPointData()->GetScalars());

    double A = atof(argv[3]);
    double B = atof(argv[4]);
    int npts = pd1->GetNumberOfPoints();

    vtkPoints *newPoints = vtkPoints::New();
    newPoints->SetNumberOfPoints(npts);
    vtkFloatArray *newVals = vtkFloatArray::New();
    newVals->SetNumberOfTuples(npts);

    vtkPoints *pts1 = pd1->GetPoints();
    vtkPoints *pts2 = pd2->GetPoints();

    vtkDataArray *arr1 = pd1->GetPointData()->GetScalars();
    vtkDataArray *arr2 = pd2->GetPointData()->GetScalars();

    int i;
    for (i = 0 ; i < npts ; i++)
    {
        double pt1[3];
        pts1->GetPoint(i, pt1);
        double pt2[3];
        pts2->GetPoint(i, pt2);
        double pt[3];
        pt[0] = (pt1[0] + pt2[0]) / 2.;
        pt[1] = (pt1[1] + pt2[1]) / 2.;
        pt[2] = (pt1[2] + pt2[2]) / 2.;
        newPoints->SetPoint(i, pt);

        float val1 = arr1->GetTuple1(i);
        float val2 = arr2->GetTuple1(i);
        float final_val = A*val1 - val2 + B;
        newVals->SetTuple1(i, final_val);
    }

    diffed->SetPoints(newPoints);
    diffed->GetPointData()->SetScalars(newVals);

    int ncells = pd1->GetNumberOfCells();
    diffed->Allocate(ncells*3);
    only1->Allocate(ncells*3);
    only2->Allocate(ncells*3);
    vtkFloatArray *uca1 = (vtkFloatArray *)
                                  pd1->GetCellData()->GetArray("cell_valid");
    vtkFloatArray *uca2 = (vtkFloatArray *)
                                  pd2->GetCellData()->GetArray("cell_valid");
    if (uca1 != NULL && uca2 != NULL)
    {
        for (i = 0 ; i < ncells ; i++)
        {
            float is1 = uca1->GetValue(i);
            float is2 = uca2->GetValue(i);
            vtkIdList *pts1 = pd1->GetCell(i)->GetPointIds();
            vtkIdList *pts2 = pd2->GetCell(i)->GetPointIds();
            if ((is1 > 0.) && (is2 > 0.))
            {
                diffed->InsertNextCell(VTK_TRIANGLE, pts1);
            }
            else if (is1 > 0.)
            {
                only1->InsertNextCell(VTK_TRIANGLE, pts1);
            }
            else if (is2 > 0.)
            {
                only2->InsertNextCell(VTK_TRIANGLE, pts2);
            }
        }
    }
    else
    {
        for (i = 0 ; i < ncells ; i++)
        {
            diffed->InsertNextCell(pd1->GetCellType(i), 
                                  pd1->GetCell(i)->GetPointIds());
        }
    }

    vtkDataSetWriter *wrtr_all = vtkDataSetWriter::New();
    wrtr_all->SetFileTypeToBinary();
    wrtr_all->SetInput(diffed);
    wrtr_all->SetFileName("both.vtk");
    wrtr_all->Write();
    vtkDataSetWriter *wrtr1 = vtkDataSetWriter::New();
    wrtr1->SetFileTypeToBinary();
    wrtr1->SetInput(only1);
    wrtr1->SetFileName("only1.vtk");
    wrtr1->Write();
    vtkDataSetWriter *wrtr2 = vtkDataSetWriter::New();
    wrtr2->SetFileTypeToBinary();
    wrtr2->SetInput(only2);
    wrtr2->SetFileName("only2.vtk");
    wrtr2->Write();
}
