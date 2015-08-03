#ifndef VTK_COMPONENT_DATA_ARRAY_H
#define VTK_COMPONENT_DATA_ARRAY_H
#include <vtkTypedDataArray.h>
#include <vtkIdList.h>

#include "vtkArrayComponent.h"

// NOTE: this macro is crucial so NewInstance() calls on the array will return
//       a normal VTK data array that has the normal set/interpolate methods.

// Adds an implementation of NewInstanceInternal() that returns a standard
// (unmapped) VTK array, if possible. Use this with classes that derive from
// vtkTypeTemplate, otherwise, use vtkMappedDataArrayTypeMacro.
#define vtkMappedDataArrayNewInstanceMacro(thisClass) \
  protected: \
  vtkObjectBase *NewInstanceInternal() const \
  { \
    if (vtkDataArray *da = \
        vtkDataArray::CreateDataArray(thisClass::VTK_DATA_TYPE)) \
      { \
      return da; \
      } \
    return thisClass::New(); \
  } \
  public:

// Same as vtkTypeMacro, but adds an implementation of NewInstanceInternal()
// that returns a standard (unmapped) VTK array, if possible.
#define vtkMappedDataArrayTypeMacro(thisClass, superClass) \
  vtkAbstractTypeMacroWithNewInstanceType(thisClass, superClass, vtkDataArray) \
  vtkMappedDataArrayNewInstanceMacro(thisClass)

int zc_getvoidpointer();

// ****************************************************************************
// Class: vtkComponentDataArray
//
// Purpose: This class assembles a vtkDataArray out of various component data 
//          sources which may exist as AOS or SOA data. This class provides the
//          means to access them all as a vtkDataArray, while delegating the
//          actual data access to ComponentData classes. This is useful for
//          helping adapt various data layouts to a view that is compatible
//          for use in VTK, while still permitting zero-copy for most accesses.
//          This class currently provides read-only views of the data.
//
// Notes:    vtkMappedDataArray was not used because its support for 
//           GetVoidPointer relied on iterators that do not support 
//           multi-component data. This version provides GetVoidPointer support
//           that can copy the multi-component data into a temp array with
//           the "Scalar" precision.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 19 12:15:08 PDT 2015
//
// Modifications:
//
// ****************************************************************************

template <typename Scalar, typename ComponentData = vtkArrayComponentStride>
class vtkComponentDataArray : public vtkTypedDataArray<Scalar>
{
public:
    typedef vtkComponentDataArray<Scalar,ComponentData> ThisClass;
    vtkMappedDataArrayTypeMacro( vtkComponentDataArray, vtkTypedDataArray<Scalar> );

    static vtkComponentDataArray<Scalar,ComponentData> *New()
    {
        return new vtkComponentDataArray<Scalar,ComponentData>();
    }

    void SetComponentData(vtkIdType comp, const ComponentData &cd)
    {
        InvalidateVoidPointerArray();

        if(components == NULL)
        {
            components = new ComponentData[this->GetNumberOfComponents()];
            temporaryTuple = new double[this->GetNumberOfComponents()];
        }

        if(comp < this->GetNumberOfComponents())
        {
            components[comp] = cd;
        }
    }

    virtual void Initialize()
    {
        voidPtrValues = 0;
        voidPtrNumValues = 0;
        components = NULL;
        temporaryTuple = NULL;
    }

    virtual bool HasStandardMemoryLayout() { return false; }

    virtual void SetNumberOfTuples(vtkIdType nt)
    {
        InvalidateVoidPointerArray();
        this->MaxId = nt * this->GetNumberOfComponents() - 1;
    }

    virtual void GetTuples(vtkIdList *ptIds, vtkAbstractArray *out)
    {
        vtkDataArray *da = vtkDataArray::SafeDownCast(out);
        if (!da)
        {
            vtkWarningMacro("Input is not a vtkDataArray.");
            return;
        }
        if ((da->GetNumberOfComponents() != this->GetNumberOfComponents()))
        {
            vtkWarningMacro("Number of components for input and output do not match");
            return;
        }

        vtkIdType num = ptIds->GetNumberOfIds();
        for (vtkIdType i=0; i<num; i++)
        {
            da->SetTuple(i, this->GetTuple(ptIds->GetId(i)));
        }
    }

    virtual void GetTuples(vtkIdType p1, vtkIdType p2, vtkAbstractArray *out)
    {
        vtkDataArray *da = vtkDataArray::SafeDownCast(out);
        if (!da)
        {
            vtkWarningMacro("Input is not a vtkDataArray.");
            return;
        }
        if ((da->GetNumberOfComponents() != this->GetNumberOfComponents()))
        {
            vtkWarningMacro("Number of components for input and output do not match");
            return;
        }

        vtkIdType num = p2-p1+1;
        for (vtkIdType i = 0; i < num; i++)
        {
            da->SetTuple(i, this->GetTuple(p1+i));
        }
    }

    virtual double *GetTuple(vtkIdType id)
    {
        GetTuple(id, temporaryTuple);
        return temporaryTuple;
    }

    virtual void GetTuple(vtkIdType id, double *out)
    {
        GatherValues(id, this->GetNumberOfComponents(), out);
    }

    virtual void GetTupleValue(vtkIdType id, Scalar *out)
    {
        GatherValues(id, this->GetNumberOfComponents(), out);
    }

    virtual Scalar &GetValueReference(vtkIdType arrayIndex)
    {
        // The array index assumes (t0c0, t0c1, t0c2) (t1c0, t1c1, t1c2), ...
        vtkIdType id = arrayIndex / this->GetNumberOfComponents();
        vtkIdType comp = arrayIndex % this->GetNumberOfComponents();

        components[comp].GetValue(id, tempReference);
        return tempReference;
    }

    virtual Scalar GetValue(vtkIdType arrayIndex)
    {
        return this->GetValueReference(arrayIndex);
    }

    vtkVariant GetVariantValue(vtkIdType arrayIndex)
    {
        return vtkVariant(this->GetValue(arrayIndex));
    }

    virtual void *GetVoidPointer(vtkIdType id)
    {
#if 1
        // Call this function for an easy to find breakpoint when you want
        // to see which functions in VisIt are calling GetVoidPointer.
        zc_getvoidpointer();
#endif
        vtkWarningMacro(<< "GetVoidPointer causes memory to be copied.");
        vtkIdType nElem = this->GetNumberOfComponents() * this->GetNumberOfTuples();
        if(voidPtrValues == NULL || nElem != voidPtrNumValues)
        {
            delete [] voidPtrValues;
            voidPtrValues = new Scalar[nElem];
            voidPtrNumValues = nElem;
        }

        Scalar *ptr = reinterpret_cast<Scalar *>(voidPtrValues);
        for(vtkIdType i = 0; i < this->GetNumberOfTuples(); ++i)
        {
            this->GetTupleValue(i, ptr);
            ptr += this->GetNumberOfComponents();
        }

        return static_cast<void*>(voidPtrValues);
    }

public:
    // Methods that don't matter since this class is a read-only view.

    virtual int Allocate(vtkIdType, vtkIdType)
    { vtkWarningMacro( << "Allocate: read only do nothing"); return 1; }
    virtual void SetTuple(vtkIdType, vtkIdType, vtkAbstractArray*)
    { vtkWarningMacro( << "SetTuple: read only do nothing"); }
    virtual void InsertTuple(vtkIdType, vtkIdType, vtkAbstractArray*)
    { vtkWarningMacro( << "InsertTuple: read only do nothing"); }
    virtual void InsertTuples(vtkIdList*, vtkIdList*, vtkAbstractArray*)
    { vtkWarningMacro( << "InsertTuples: read only do nothing"); }
    virtual void InsertTuples(vtkIdType, vtkIdType, vtkIdType, vtkAbstractArray*)
    { vtkWarningMacro( << "InsertTuples: read only do nothing"); }
    virtual vtkIdType InsertNextTuple(vtkIdType, vtkAbstractArray*)
    { vtkWarningMacro( << "InsertNextTuple: read only do nothing"); return 0; }
    virtual void DeepCopy(vtkAbstractArray *arr)
    {
#if 1
        vtkComponentDataArray<Scalar,ComponentData> *da = SafeDownCast(arr);
        if(da != NULL)
        {
            InvalidateVoidPointerArray();

            if(components != NULL)
                delete [] components;
            components = new ComponentData[this->GetNumberOfComponents()];
            for(int i = 0; i < this->GetNumberOfComponents(); ++i)
                components[i] = da->components[i];

            if(temporaryTuple != NULL)
                delete [] temporaryTuple;
            temporaryTuple = new double[this->GetNumberOfComponents()];
        }
#endif
    }
    virtual void DeepCopy(vtkDataArray *arr)
    {
#if 1
        vtkComponentDataArray<Scalar,ComponentData> *da = SafeDownCast(arr);
        if(da != NULL)
        {
            InvalidateVoidPointerArray();

            if(components != NULL)
                delete [] components;
            components = new ComponentData[this->GetNumberOfComponents()];
            for(int i = 0; i < this->GetNumberOfComponents(); ++i)
                components[i] = da->components[i];

            if(temporaryTuple != NULL)
                delete [] temporaryTuple;
            temporaryTuple = new double[this->GetNumberOfComponents()];
        }
#endif
    }
    virtual void InterpolateTuple(vtkIdType i, vtkIdList *ptIndices,
                                  vtkAbstractArray* source,  double* weights)
    { vtkWarningMacro( << "InterpolateTuple: read only do nothing"); }
    virtual void Squeeze()
    { vtkWarningMacro( << "Squeeze: read only do nothing"); }
    virtual int Resize(vtkIdType)
    { vtkWarningMacro( << "Resize: read only do nothing"); return 0; }
    virtual void SetTuple(vtkIdType, const float*)
    { vtkWarningMacro( << "SetTuple: read only do nothing"); }
    virtual void SetTuple(vtkIdType, const double*)
    { vtkWarningMacro( << "SetTuple: read only do nothing"); }
    virtual void SetTuple(vtkIdType &, double*)
    { vtkWarningMacro( << "SetTuple: read only do nothing"); }
    virtual void InsertTuple(vtkIdType, const float*)
    { vtkWarningMacro( << "InsertTuple: read only do nothing"); }
    virtual void InsertTuple(vtkIdType, const double*)
    { vtkWarningMacro( << "InsertTuple: read only do nothing"); }
    virtual vtkIdType InsertNextTuple(const float*)
    { vtkWarningMacro( << "InsertNextTuple: read only do nothing"); return 0; }
    virtual vtkIdType InsertNextTuple(const double*)
    { vtkWarningMacro( << "InsertNextTuple: read only do nothing"); return 0; }
    virtual void RemoveTuple(vtkIdType)
    { vtkWarningMacro( << "RemoveTuple: read only do nothing"); }
    virtual void RemoveFirstTuple()
    { vtkWarningMacro( << "RemoveFirstTuple: read only do nothing"); }
    virtual void RemoveLastTuple()
    { vtkWarningMacro( << "RemoveLastTuple: read only do nothing");/* read only; do nothing. */ }

    virtual void SetValue(vtkIdType, Scalar)
    { vtkWarningMacro( << "SetValue: read only do nothing");/* read only; do nothing. */ }
    virtual vtkIdType InsertNextValue(Scalar)
    { vtkWarningMacro( << "InsertNextValue: read only do nothing");/* read only; do nothing. */ return 0; }
    virtual void InsertValue(vtkIdType, Scalar)
    { vtkWarningMacro( << "InsertValue: read only do nothing");/* read only; do nothing. */ }

    void InterpolateTuple(vtkIdType, vtkIdType, vtkAbstractArray*, vtkIdType, vtkAbstractArray*, double)
    { vtkWarningMacro( << "InterpolateTuple: read only do nothing");/* read only; do nothing. */ }
    void SetVariantValue(vtkIdType, vtkVariant)
    { vtkWarningMacro( << "SetVariantValue: read only do nothing");/* read only; do nothing. */ }
    virtual void ClearLookup()
    { vtkWarningMacro( << "ClearLookup: read only do nothing");/* read only; do nothing. */ }
    void SetTupleValue(vtkIdType, const Scalar*)
    { vtkWarningMacro( << "SetTupleValue: read only do nothing");/* read only; do nothing. */ }
    void InsertTupleValue(vtkIdType, const Scalar*)
    { vtkWarningMacro( << "InsertTupleValue: read only do nothing");/* read only; do nothing. */ }
    vtkIdType InsertNextTupleValue(const Scalar*)
    { vtkWarningMacro( << "InsertNextTupleValue: read only do nothing"); return 0; /* read only; do nothing. */ }

    vtkIdType LookupTypedValue(Scalar)
    {
        vtkErrorMacro(<< "LookupTypedValue not implemented.");
        return 0;
    }

    void LookupTypedValue(Scalar, vtkIdList*)
    {
        vtkErrorMacro(<< "LookupTypedValue not implemented.");
    }

    virtual vtkArrayIterator* NewIterator()
    {
        vtkErrorMacro(<< "NewIterator not implemented.");
        return NULL;
    }

    virtual vtkIdType LookupValue(vtkVariant)
    {
        vtkErrorMacro(<< "LookupValue not implemented.");
        return 0;
    }

    virtual void LookupValue(vtkVariant, vtkIdList*)
    {
        vtkErrorMacro(<< "LookupValue not implemented.");
    }

    virtual void SetVoidArray(void*, vtkIdType, int)
    {
        vtkErrorMacro(<< "SetVoidArray not implemented.");
    }

    virtual void *WriteVoidPointer(vtkIdType, vtkIdType)
    {
        vtkErrorMacro(<< "WriteVoidPointer not implemented.");
        return NULL;
    }

    virtual void DataChanged()
    {
        InvalidateVoidPointerArray();
    }

protected:
    vtkComponentDataArray() : vtkTypedDataArray<Scalar>()
    {
        Initialize();
    }

    virtual ~vtkComponentDataArray()
    {
        InvalidateVoidPointerArray();

        if(components != NULL)
        {
            for(int i = 0; i < this->GetNumberOfComponents(); ++i)
            {
                components[i].Delete();
            }
            delete [] components;
            components = NULL;
        }

        if(temporaryTuple != NULL)
        {
            delete [] temporaryTuple;
            temporaryTuple = NULL;
        }
    }

    void InvalidateVoidPointerArray()
    {
        if(voidPtrValues != NULL)
        {
            delete [] voidPtrValues;
            voidPtrValues = NULL;
            voidPtrNumValues = 0;
        }
    }

    template <typename DestinationType>
    void GatherValues(vtkIdType id, int nComps, DestinationType *out) 
    {
        if(id >= this->GetNumberOfTuples())
            return;

        for(int i = 0; i < nComps; ++i)
        {
            components[i].GetValue(id, out[i]);
        }
    }

    ComponentData *components;
    double        *temporaryTuple;
    Scalar         tempReference;
    Scalar        *voidPtrValues;
    vtkIdType      voidPtrNumValues;
};

#endif
