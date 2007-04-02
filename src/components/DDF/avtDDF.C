// ************************************************************************* //
//                                avtDDF.C                                   //
// ************************************************************************* //

#include <avtDDF.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <avtBinningScheme.h>
#include <avtDDFFunctionInfo.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtDDF constructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtDDF::avtDDF(avtDDFFunctionInfo *info, float *v)
{
    functionInfo = info;
    vals         = v;
}


// ****************************************************************************
//  Method: avtDDF destructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtDDF::~avtDDF()
{
    delete [] vals;
    delete functionInfo;
}


// ****************************************************************************
//  Method: avtDDF::CreateGrid
//
//  Purpose:
//      Creates a grid so the DDF can be visualized.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

vtkDataSet *
avtDDF::CreateGrid(void)
{
    avtBinningScheme *bs = functionInfo->GetBinningScheme();
    vtkDataSet *rv = bs->CreateGrid();
    int nCells = rv->GetNumberOfCells();

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetName(functionInfo->GetCodomainName().c_str());
    arr->SetNumberOfTuples(nCells);
    for (int i = 0 ; i < nCells ; i++)
    {
        vtkCell *cell = rv->GetCell(i);
        float bbox[6];
        cell->GetBounds(bbox);

        // Note the assumption here that the X-axis corresponds to tuple 0,
        // the Y-axis to tuple 1, etc.
        float center[3];
        center[0] = (bbox[0] + bbox[1]) / 2.;
        center[1] = (bbox[2] + bbox[3]) / 2.;
        center[2] = (bbox[4] + bbox[5]) / 2.;

        int binId = bs->GetBinId(center);
        arr->SetValue(i, vals[binId]);
    }

    rv->GetCellData()->AddArray(arr);
    arr->Delete();

    return rv;
}


// ****************************************************************************
//  Method: avtDDF::ApplyFunction
//
//  Purpose:
//      Applies the DDF to a data set.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

vtkDataArray *
avtDDF::ApplyFunction(vtkDataSet *ds)
{
    int   i, j, k;
    int   nvars = functionInfo->GetDomainNumberOfTuples();

    bool hasError = false;
    bool isNodal = true;
    const char   *varname0 = functionInfo->GetCodomainName().c_str();
    vtkDataArray *arr0 = ds->GetPointData()->GetArray(varname0);
    if (arr0 == NULL)
    {
        arr0 = ds->GetCellData()->GetArray(varname0);
        if (arr0 == NULL)
            hasError = true;
        isNodal = false;
    }

    vtkDataArray **arr = new vtkDataArray*[nvars];
    for (k = 0 ; k < nvars ; k++)
    {
        const char *varname = functionInfo->GetDomainTupleName(k).c_str();
        arr[k] = (isNodal ? ds->GetPointData()->GetArray(varname)
                          : ds->GetCellData()->GetArray(varname));
        if (arr == NULL)
            hasError = true;
    }

    if (hasError)
    {
        debug1 << "Could not locate one of the tuples from the "
               << "domain.  Or the variables have different centerings."
               << endl;
        // Put burden caller to issue better error message.
        return NULL;
    }

    int nvals = (isNodal ? ds->GetNumberOfPoints() : ds->GetNumberOfCells());
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetName(functionInfo->GetCodomainName().c_str());
    rv->SetNumberOfTuples(nvals);
    float *v = new float[nvars];
    avtBinningScheme *bs = functionInfo->GetBinningScheme();
    for (i = 0 ; i < nvals ; i++)
    {
        for (j = 0 ; j < nvars ; j++)
            v[j] = arr[j]->GetTuple1(i);
        int binId = bs->GetBinId(v);
        rv->SetValue(i, vals[binId]);
    }

    delete [] arr;
    delete [] v;

    return rv;
}


