#include <stdlib.h>

#include <vtkDataSetReader.h>
#include <vtkDataSetWriter.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        cerr << "Usage: " << argv[0] << " <surface-1> <surface-2> <A> <B>"
             << endl;
        cerr << "\tThe result is S1*A - S2 + B" << endl;
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

    vtkUnstructuredGrid *diffed = vtkUnstructuredGrid::New();
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
    newPoints->SetNumberOfPoints(2*npts);
    vtkFloatArray *newVals = vtkFloatArray::New();
    newVals->SetNumberOfTuples(2*npts);

    vtkPoints *pts1 = pd1->GetPoints();
    vtkPoints *pts2 = pd2->GetPoints();

    vtkDataArray *arr1 = pd1->GetPointData()->GetScalars();
    vtkDataArray *arr2 = pd2->GetPointData()->GetScalars();

    int i;
    for (i = 0 ; i < npts ; i++)
    {
        float pt[3];
        pts1->GetPoint(i, pt);
        newPoints->SetPoint(2*i, pt);
        pts2->GetPoint(i, pt);
        newPoints->SetPoint(2*i+1, pt);

        float val1 = arr1->GetTuple1(i);
        float val2 = arr2->GetTuple1(i);
        float final_val = A*val1 - val2 + B;
        newVals->SetTuple1(2*i, final_val);
        newVals->SetTuple1(2*i+1, final_val);
    }

    diffed->SetPoints(newPoints);
    diffed->GetPointData()->SetScalars(newVals);

    int ncells = pd1->GetNumberOfCells();
    diffed->Allocate(ncells*6);
    only1->Allocate(ncells*3);
    only2->Allocate(ncells*3);
    vtkUnsignedCharArray *uca1 = (vtkUnsignedCharArray *)
                                  pd1->GetCellData()->GetArray("cell_valid");
    vtkUnsignedCharArray *uca2 = (vtkUnsignedCharArray *)
                                  pd2->GetCellData()->GetArray("cell_valid");
    for (i = 0 ; i < ncells ; i++)
    {
        unsigned char is1 = uca1->GetValue(i);
        unsigned char is2 = uca2->GetValue(i);
        vtkIdList *pts1 = pd1->GetCell(i)->GetPointIds();
        vtkIdList *pts2 = pd2->GetCell(i)->GetPointIds();
        if (is1 && is2)
        {
            vtkIdType wedge[6];
            wedge[0] = 2*pts1->GetId(0);
            wedge[1] = 2*pts1->GetId(1);
            wedge[2] = 2*pts1->GetId(2);
            wedge[3] = 2*pts2->GetId(0) + 1;
            wedge[4] = 2*pts2->GetId(1) + 1;
            wedge[5] = 2*pts2->GetId(2) + 1;
            diffed->InsertNextCell(VTK_WEDGE, 6, wedge);
        }
        else if (is1)
        {
            only1->InsertNextCell(VTK_TRIANGLE, pts1);
        }
        else if (is2)
        {
            only2->InsertNextCell(VTK_TRIANGLE, pts2);
        }
    }

    vtkDataSetWriter *wrtr_all = vtkDataSetWriter::New();
    wrtr_all->SetInput(diffed);
    wrtr_all->SetFileName("both.vtk");
    wrtr_all->Write();
    vtkDataSetWriter *wrtr1 = vtkDataSetWriter::New();
    wrtr1->SetInput(only1);
    wrtr1->SetFileName("only1.vtk");
    wrtr1->Write();
    vtkDataSetWriter *wrtr2 = vtkDataSetWriter::New();
    wrtr2->SetInput(only2);
    wrtr2->SetFileName("only2.vtk");
    wrtr2->Write();
}
