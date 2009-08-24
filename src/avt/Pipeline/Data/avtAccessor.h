#ifndef AVT_ACCESSOR_H
#define AVT_ACCESSOR_H
#include <vtkDataArray.h>
#include <string.h>

// ****************************************************************************
// Class: avtDirectAccessor
//
// Purpose:
//   This template class provides a way to quickly get at vtkDataArray data
//   without having to access it using vtkDataArray Get/SetTuple1 methods.
//
// Notes:      
//   T : The type of data contained in the vtkDataArray.
//   N : The number of components in the data (default == 1).
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 23 09:02:01 PST 2008
//
// Modifications:
//   
// ****************************************************************************

template <class T>
class avtDirectAccessor
{
public:
    avtDirectAccessor(vtkDataArray *arr)
    {
        array = arr;
        N = arr->GetNumberOfComponents();
        ptr = start = (T *)arr->GetVoidPointer(0);
        end = ptr + (N * arr->GetNumberOfTuples());
    }

    ~avtDirectAccessor()                                      { }

    int GetNumberOfTuples() const                             { return array->GetNumberOfTuples(); }
    int GetNumberOfComponents() const                         { return N; }

    void SetTuple1(T val)                                     { *ptr = val;                            }
    void SetTuple1(vtkIdType i, T val)                        { start[i*N] = val;                      }
    void SetTuple(vtkIdType i, const T *val)                  { memcpy(start + i*N, val, sizeof(T)*N); }

    T GetTuple1() const                                       { return *ptr;        }
    T GetTuple1(vtkIdType i) const                            { return start[i*N];  }
    T *GetTuple(vtkIdType i) const                            { return start + i*N; }
    T *GetTuple() const                                       { return ptr; }

    void SetComponent(vtkIdType tuple, int comp, T val)       { start[tuple * N + comp] = val; }
    void SetComponent(int comp, T val)                        { ptr[comp] = val; }

    T GetComponent(vtkIdType tuple, int comp) const           { return start[tuple * N + comp]; }
    T GetComponent(int comp) const                            { return ptr[comp]; }

    void InitTraversal()                                      { ptr = start; }
    bool Iterating() const                                    { return ptr < end; }

    void operator ++()                                        { ptr += N; }
    void operator ++(int i)                                   { ptr += N; }
private:
    vtkDataArray *array;
    T *start;
    T *ptr;
    T *end;
    int N;
};

// ****************************************************************************
// Class: avtTupleAccessor
//
// Purpose:
//   This class provides an "accessor" interface for accessing data in the
//   vtkDataArray using the Get/SetTuple methods.
//
// Notes:      
//   Since this class is inline, there's no penalty for this thin extra layer
//   over vtkDataArray.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 23 09:02:01 PST 2008
//
// Modifications:
//   
// ****************************************************************************

class avtTupleAccessor
{
public:
    avtTupleAccessor(vtkDataArray *arr)
    {
        array = arr;
        index = 0;
        size = array->GetNumberOfTuples();
    }
    ~avtTupleAccessor()                      { }
    
    int GetNumberOfTuples() const            { return array->GetNumberOfTuples(); }
    int GetNumberOfComponents() const        { return array->GetNumberOfComponents(); }

    void SetTuple1(double val)               { array->SetTuple1(index, val);   }
    void SetTuple1(vtkIdType i, double val)  { array->SetTuple1(i, val);       }
    void SetTuple(vtkIdType i, const double *val) { array->SetTuple(i, val);   }

    double GetTuple1()                       { return array->GetTuple1(index); }
    double GetTuple1(vtkIdType i)            { return array->GetTuple1(i);     }
    double *GetTuple(vtkIdType i)            { return array->GetTuple(i);      }//does this work?

    void SetComponent(vtkIdType tuple, int comp, double val) { array->SetComponent(tuple, comp, val); }
    void SetComponent(int comp, double val)                  { array->SetComponent(index, comp, val); }

    double GetComponent(vtkIdType tuple, int comp) const     { return array->GetComponent(tuple, comp); }
    double GetComponent(int comp) const                      { return array->GetComponent(index, comp); }

    void InitTraversal()                     { index = 0; }
    bool Iterating() const                   { return index < size; }

    void operator ++()                       { index ++; }
    void operator ++(int)                    { index ++; }

private:
    vtkDataArray *array;
    vtkIdType     index;
    vtkIdType     size;
};


#endif
