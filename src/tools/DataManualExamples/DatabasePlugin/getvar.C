// NOTE - This code incomplete and requires underlined portions
// to be replaced with code to read values from your file format.

#include <vtkFloatArray.h>

vtkDataArray *
avtXXXFileFormat::GetVar(const char *varname)
{
    int nvals;
    // Read the number of values contained in the array
    // specified by varname.
    nvals = NUMBER OF VALUES IN ARRAY NAMED BY varname;
    
    // Allocate the return vtkFloatArray object. Note that
    // you can use vtkFloatArray, vtkDoubleArray,
    // vtkUnsignedCharArray, vtkIntArray, etc.
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(nvals);
    float *data = (float *)arr->GetVoidPointer(0);
    READ nvals FLOAT NUMBERS INTO THE data ARRAY.

    return arr;
}
