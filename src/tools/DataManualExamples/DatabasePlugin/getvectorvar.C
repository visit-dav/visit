// NOTE - This code incomplete and requires underlined portions
// to be replaced with code to read values from your file format.

#include <vtkFloatArray.h>
#include <InvalidVariableException.h>

vtkDataArray *
avtXXXFileFormat::GetVectorVar(const char *varname)
{
    int nvals, ncomps = 3;

    // Read the number of values contained in the array
    // specified by varname.
    nvals = NUMBER OF VALUES IN ARRAY NAMED BY varname;
    ncomps = NUMBER OF VECTOR COMPONENTS IN ARRAY NAMED BY varname;

    // Read component 1 from the file.
    float *comp1 = new float[nvals];
    READ nvals FLOAT VALUES INTO comp1

    // Read component 2 from the file.
    float *comp2 = new float[nvals];
    READ nvals FLOAT VALUES INTO comp2

    // Read component 3 from the file.
    float *comp3 = 0;
    if(ncomps > 2)
    {
        comp3 = new float[nvals];
        READ nvals FLOAT VALUES INTO comp3
    }

    // Allocate the return vtkFloatArray object. Note that
    // you can use vtkFloatArray, vtkDoubleArray,
    // vtkUnsignedCharArray, vtkIntArray, etc.
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfComponents(3);
    arr->SetNumberOfTuples(nvals);
    float *data = (float *)arr->GetVoidPointer(0);
    float *c1 = comp1;
    float *c2 = comp2;
    float *c3 = comp3;
    if(ncomps == 3)
    {
        for(int i = 0; i < nvals; ++i)
        {
            *data++ = *c1++;
            *data++ = *c2++;
            *data++ = *c3++;
        }
    }
    else if(ncomps == 2)
    {
        for(int i = 0; i < nvals; ++i)
        {
            *data++ = *c1++;
            *data++ = *c2++;
            *data++ = 0.;
        }
    }
    else
    {
        delete [] comp1;
        delete [] comp2;
        delete [] comp3;
        arr->Delete();
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Delete temporary arrays.
    delete [] comp1;
    delete [] comp2;
    delete [] comp3;

    return arr;
}
