// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_COMPONENT_DATA_ARRAY_H
#define VTK_COMPONENT_DATA_ARRAY_H
#include <vtkTypedDataArray.h>
#include <vtkIdList.h>

#include "vtkArrayComponent.h"

// NOTE: this macro is crucial so NewInstance() calls on the array will return
//       a normal VTK data array that has the normal set/interpolate methods.

// Adds an implementation of NewInstanceInternal() that returns a standard
// (unmapped) VTK array, if possible. Use this with classes that derive from
// vtkTypeTemplate, otherwise, use vtkComponentDataArrayTypeMacro.
#define vtkComponentDataArrayNewInstanceMacro(thisClass) \
  protected: \
  vtkObjectBase *NewInstanceInternal() const override \
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
#define vtkComponentDataArrayTypeMacro(thisClass, superClass) \
  vtkAbstractTypeMacroWithNewInstanceType(thisClass, superClass, vtkDataArray, "vtkComponentDataArray") \
  vtkComponentDataArrayNewInstanceMacro(thisClass)

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
//    Kathleen Biagas, Tue May 10 16:54:28 PDT 2016
//    Modified for VTK-8: GetTupleValue->GetTypedTuple, etc, and deal with
//    inconsistent const-ness.
//
//    Kathleen Biagas, Thu Apr  8 16:15:59 PDT 2021
//    Add ExportToVoidPointer, so that this class will work correctly with
//    vtkDataWriter.
//
// ****************************************************************************

template <typename Scalar, typename ComponentData = vtkArrayComponentStride>
class vtkComponentDataArray : public vtkTypedDataArray<Scalar>
{
public:
    typedef vtkComponentDataArray<Scalar,ComponentData> ThisClass;
    vtkComponentDataArrayTypeMacro( vtkComponentDataArray, vtkTypedDataArray<Scalar> );

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

    void Initialize() override
    {
        voidPtrValues = 0;
        voidPtrNumValues = 0;
        components = NULL;
        temporaryTuple = NULL;
    }

    bool HasStandardMemoryLayout() const override { return false; }

    void SetNumberOfTuples(vtkIdType nt) override
    {
        InvalidateVoidPointerArray();
        this->MaxId = nt * this->GetNumberOfComponents() - 1;
    }

    void GetTuples(vtkIdList *ptIds, vtkAbstractArray *out) override
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

    void GetTuples(vtkIdType p1, vtkIdType p2, vtkAbstractArray *out) override
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

    double *GetTuple(vtkIdType id) override
    {
        GetTuple(id, temporaryTuple);
        return temporaryTuple;
    }

    void GetTuple(vtkIdType id, double *out) override
    {
        GatherValues(id, this->GetNumberOfComponents(), out);
    }

    void GetTypedTuple(vtkIdType id, Scalar *out) const override
    {
        ThisClass *tmp = const_cast<vtkComponentDataArray*>(this);
        tmp->GatherValues(id, tmp->GetNumberOfComponents(), out);
    }

    Scalar &GetValueReference(vtkIdType arrayIndex) override
    {
        // The array index assumes (t0c0, t0c1, t0c2) (t1c0, t1c1, t1c2), ...
        vtkIdType id = arrayIndex / this->GetNumberOfComponents();
        vtkIdType comp = arrayIndex % this->GetNumberOfComponents();

        components[comp].GetValue(id, tempReference);
        return tempReference;
    }

    Scalar GetValue(vtkIdType arrayIndex) const override
    {
        ThisClass *tmp = const_cast<vtkComponentDataArray*>(this);
        return tmp->GetValueReference(arrayIndex);
    }

    vtkVariant GetVariantValue(vtkIdType arrayIndex) override
    {
        return vtkVariant(this->GetValue(arrayIndex));
    }

    //
    // This method copies the array data to the void pointer specified
    // by the user.  It is up to the user to allocate enough memory for
    // the void pointer.
    //
    void ExportToVoidPointer(void *out_ptr) override
    {
        Scalar *ptr = reinterpret_cast<Scalar *>(out_ptr);
        for(vtkIdType i = 0; i < this->GetNumberOfTuples(); ++i)
        {
            this->GetTypedTuple(i, ptr);
            ptr += this->GetNumberOfComponents();
        }
    }

    void *GetVoidPointer(vtkIdType id) override
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
            this->GetTypedTuple(i, ptr);
            ptr += this->GetNumberOfComponents();
        }

        return static_cast<void*>(voidPtrValues);
    }

public:
    // Methods that don't matter since this class is a read-only view.

    int Allocate(vtkIdType, vtkIdType) override
    { vtkWarningMacro( << "Allocate: read only do nothing"); return 1; }
    void SetTuple(vtkIdType, vtkIdType, vtkAbstractArray*) override
    { vtkWarningMacro( << "SetTuple: read only do nothing"); }
    void InsertTuple(vtkIdType, vtkIdType, vtkAbstractArray*) override
    { vtkWarningMacro( << "InsertTuple: read only do nothing"); }
    void InsertTuples(vtkIdList*, vtkIdList*, vtkAbstractArray*) override
    { vtkWarningMacro( << "InsertTuples: read only do nothing"); }
    void InsertTuples(vtkIdType, vtkIdType, vtkIdType, vtkAbstractArray*) override
    { vtkWarningMacro( << "InsertTuples: read only do nothing"); }
    vtkIdType InsertNextTuple(vtkIdType, vtkAbstractArray*) override
    { vtkWarningMacro( << "InsertNextTuple: read only do nothing"); return 0; }
    void DeepCopy(vtkAbstractArray *arr) override
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
    void DeepCopy(vtkDataArray *arr) override
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
    void InterpolateTuple(vtkIdType i, vtkIdList *ptIndices,
                          vtkAbstractArray* source,  double* weights) override
    { vtkWarningMacro( << "InterpolateTuple: read only do nothing"); }
    void Squeeze() override
    { vtkWarningMacro( << "Squeeze: read only do nothing"); }
    int Resize(vtkIdType) override
    { vtkWarningMacro( << "Resize: read only do nothing"); return 0; }
    void SetTuple(vtkIdType, const float*) override
    { vtkWarningMacro( << "SetTuple: read only do nothing"); }
    void SetTuple(vtkIdType, const double*) override
    { vtkWarningMacro( << "SetTuple: read only do nothing"); }
    virtual void SetTuple(vtkIdType &, double*)
    { vtkWarningMacro( << "SetTuple: read only do nothing"); }
    void InsertTuple(vtkIdType, const float*) override
    { vtkWarningMacro( << "InsertTuple: read only do nothing"); }
    void InsertTuple(vtkIdType, const double*) override
    { vtkWarningMacro( << "InsertTuple: read only do nothing"); }
    vtkIdType InsertNextTuple(const float*) override
    { vtkWarningMacro( << "InsertNextTuple: read only do nothing"); return 0; }
    vtkIdType InsertNextTuple(const double*) override
    { vtkWarningMacro( << "InsertNextTuple: read only do nothing"); return 0; }
    void RemoveTuple(vtkIdType) override
    { vtkWarningMacro( << "RemoveTuple: read only do nothing"); }
    void RemoveFirstTuple() override
    { vtkWarningMacro( << "RemoveFirstTuple: read only do nothing"); }
    void RemoveLastTuple() override
    { vtkWarningMacro( << "RemoveLastTuple: read only do nothing");/* read only; do nothing. */ }

    void SetValue(vtkIdType, Scalar) override
    { vtkWarningMacro( << "SetValue: read only do nothing");/* read only; do nothing. */ }
    vtkIdType InsertNextValue(Scalar) override
    { vtkWarningMacro( << "InsertNextValue: read only do nothing");/* read only; do nothing. */ return 0; }
    void InsertValue(vtkIdType, Scalar) override
    { vtkWarningMacro( << "InsertValue: read only do nothing");/* read only; do nothing. */ }

    void InterpolateTuple(vtkIdType, vtkIdType, vtkAbstractArray*, vtkIdType, vtkAbstractArray*, double) override
    { vtkWarningMacro( << "InterpolateTuple: read only do nothing");/* read only; do nothing. */ }
    void SetVariantValue(vtkIdType, vtkVariant) override
    { vtkWarningMacro( << "SetVariantValue: read only do nothing");/* read only; do nothing. */ }
    void ClearLookup() override
    { vtkWarningMacro( << "ClearLookup: read only do nothing");/* read only; do nothing. */ }
    void SetTypedTuple(vtkIdType, const Scalar*) override
    { vtkWarningMacro( << "SetTypedTuple: read only do nothing");/* read only; do nothing. */ }
    void InsertTypedTuple(vtkIdType, const Scalar*) override
    { vtkWarningMacro( << "InsertTypedTuple: read only do nothing");/* read only; do nothing. */ }
    vtkIdType InsertNextTypedTuple(const Scalar*) override
    { vtkWarningMacro( << "InsertNextTypedTuple: read only do nothing"); return 0; /* read only; do nothing. */ }

    vtkIdType LookupTypedValue(Scalar) override
    {
        vtkErrorMacro(<< "LookupTypedValue not implemented.");
        return 0;
    }

    void LookupTypedValue(Scalar, vtkIdList*) override
    {
        vtkErrorMacro(<< "LookupTypedValue not implemented.");
    }

    vtkArrayIterator* NewIterator() override
    {
        vtkErrorMacro(<< "NewIterator not implemented.");
        return NULL;
    }

    vtkIdType LookupValue(vtkVariant) override
    {
        vtkErrorMacro(<< "LookupValue not implemented.");
        return 0;
    }

    void LookupValue(vtkVariant, vtkIdList*) override
    {
        vtkErrorMacro(<< "LookupValue not implemented.");
    }

    void SetVoidArray(void*, vtkIdType, int) override
    {
        vtkErrorMacro(<< "SetVoidArray not implemented.");
    }

    void *WriteVoidPointer(vtkIdType, vtkIdType) override
    {
        vtkErrorMacro(<< "WriteVoidPointer not implemented.");
        return NULL;
    }

    void DataChanged() override
    {
        InvalidateVoidPointerArray();
    }

protected:
    vtkComponentDataArray() : vtkTypedDataArray<Scalar>()
    {
        Initialize();
    }

    ~vtkComponentDataArray() override
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
