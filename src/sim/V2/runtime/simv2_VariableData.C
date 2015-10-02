#include <snprintf.h>
#include <string.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_VariableData.h"
#include "simv2_TypeTraits.hxx"
#include <stdio.h>
#include <map>
#include <sstream>
#include <utility>
using std::ostringstream;
using std::map;
using std::pair;

////////////////////////////////////////////////////////////////////////////////

class VisIt_ArrayData
{
public:
    VisIt_ArrayData();
    ~VisIt_ArrayData();
    int CopyData();

    int memory;
    int owner;
    int dataType;
    int nComponents;
    int nTuples;
    int offset;
    int stride;
    void *data;
};

VisIt_ArrayData::VisIt_ArrayData() :
    memory(VISIT_MEMORY_CONTIGUOUS), owner(VISIT_OWNER_VISIT),
    dataType(VISIT_DATATYPE_FLOAT), nComponents(1), nTuples(0),
    offset(0), stride(0), data(NULL)
{}

VisIt_ArrayData::~VisIt_ArrayData()
{
    if (owner == VISIT_OWNER_VISIT)
        free(data);
}

typedef std::map<int, VisIt_ArrayData*> ArrayDataMap;
typedef ArrayDataMap::iterator ArrayDataMapIt;

template <typename Scalar>
void *
CopyStridedData(unsigned char *base, int nTuples, int offset, int &stride)
{
    size_t sz = nTuples * sizeof(Scalar);
    Scalar *newData = (Scalar *)malloc(sz);
    if (newData)
    {
        for(int i = 0; i < nTuples; ++i)
        {
            unsigned char *src = base + offset + i * stride;
            Scalar *tsrc = (Scalar *)(src);
            newData[i] = *tsrc;
        }

        stride = sizeof(Scalar);
    }
    return (void*)newData;
}

int
VisIt_ArrayData::CopyData()
{
    int realOwner = owner;
    void *realData = data;
    int realOffset = offset;
    int realStride = stride;
    if (owner == VISIT_OWNER_COPY)
    {
        if (memory == VISIT_MEMORY_CONTIGUOUS)
        {
            switch (dataType)
            {
            simV2TemplateMacro(
                size_t sz = nComponents*nTuples*sizeof(simV2_TT::cppType);
                realData = malloc(sz);
                if (realData)
                {
                    memcpy(realData, data, sz);
                    realOwner = VISIT_OWNER_VISIT;
                }
                else
                {
                    VisItError("Could not allocate memory to copy data");
                    return VISIT_ERROR;
                }
                );
            }
        }
        else
        {
            unsigned char *base = static_cast<unsigned char *>(data);

            /* We copy the strided data to a contiguous block. */
            void *newData = NULL;
            switch (dataType)
            {
            case VISIT_DATATYPE_CHAR:
                newData = CopyStridedData<unsigned char>(base, nTuples, offset, stride);
                break;
            case VISIT_DATATYPE_INT:
                newData = CopyStridedData<int>(base, nTuples, offset, stride);
                break;
            case VISIT_DATATYPE_LONG:
                newData = CopyStridedData<long>(base, nTuples, offset, stride);
                break;
            case VISIT_DATATYPE_FLOAT:
                newData = CopyStridedData<float>(base, nTuples, offset, stride);
                break;
            case VISIT_DATATYPE_DOUBLE:
                newData = CopyStridedData<double>(base, nTuples, offset, stride);
                break;
            }

            if (newData)
            {
                realOwner = VISIT_OWNER_VISIT;
                realOffset = 0;
                realStride = stride;
                realData = newData;
            }
            else
            {
                VisItError("Could not allocate memory to copy data");
                return VISIT_ERROR;
            }
        }

        owner = realOwner;
        data = realData;
        offset = realOffset;
        stride = realStride;
        memory = VISIT_MEMORY_CONTIGUOUS;
    }
    return VISIT_OKAY;
}

////////////////////////////////////////////////////////////////////////////////

class VisIt_VariableData : public VisIt_ObjectBase
{
public:
    VisIt_VariableData();
    virtual ~VisIt_VariableData();

    void             SetArray(int arrId, VisIt_ArrayData *arr);
    VisIt_ArrayData *GetArray(int comp);
    int              GetNumberOfArrays() { return (int)arrays.size(); }

    ArrayDataMap arrays;
    void (*callback)(void *);
    void *callbackData;
};

VisIt_VariableData::VisIt_VariableData() :
    VisIt_ObjectBase(VISIT_VARIABLE_DATA), arrays(), callback(NULL),
    callbackData(NULL)
{}

VisIt_VariableData::~VisIt_VariableData()
{
    // This gets installed on VTK when the data are served. We call it here in
    // case the data were *not* given to VTK. If they were given to VTK then
    // the SimV2 reader will have nulled out the callback.
    if (callback)
        (*callback)(callbackData);

    ArrayDataMapIt it = arrays.begin();
    ArrayDataMapIt end = arrays.end();
    for(; it != end; ++it)
        delete it->second;

    arrays.clear();
}

void
VisIt_VariableData::SetArray(int arrId, VisIt_ArrayData *arr)
{
    pair<ArrayDataMapIt, bool> ins =
        arrays.insert(ArrayDataMap::value_type(arrId, arr));

    if (!ins.second)
        delete ins.first->second;

    ins.first->second = arr;
}

VisIt_ArrayData *
VisIt_VariableData::GetArray(int comp)
{
    ArrayDataMapIt it = arrays.find(comp);
    if (it != arrays.end())
        return it->second;
    return NULL;
}

static VisIt_VariableData *
GetVariableDataObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_VariableData *obj = (VisIt_VariableData *)VisItGetPointer(h);
    if (obj)
    {
        if (obj->objectType() != VISIT_VARIABLE_DATA)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to a "
                "VariableData object.", fname);
            VisItError(tmp);
            obj = NULL;
        }
    }
    else
    {
        SNPRINTF(tmp, 100, "%s: An invalid handle was provided for a "
            "VariableData object.", fname);
        VisItError(tmp);
    }

    return obj;
}

static int
simv2_VariableData_setArrayData_internal(visit_handle h,
    int arrIndex, int memory,
    int owner, int dataType,
    int nComps, int nTuples, int offset, int stride, void *data)

{
    if (arrIndex < 0)
    {
        VisItError("The array index must be greater than or equal to 0.");
        return VISIT_ERROR;
    }

    if ((memory != VISIT_MEMORY_CONTIGUOUS) &&
       (memory != VISIT_MEMORY_STRIDED))
    {
        VisItError("The memory type must be either "
            "VISIT_MEMORY_CONTIGUOUS or VISIT_MEMORY_STRIDED.");
        return VISIT_ERROR;
    }

    if ((owner != VISIT_OWNER_SIM) &&
       (owner != VISIT_OWNER_VISIT) &&
       (owner != VISIT_OWNER_VISIT_EX) &&
       (owner != VISIT_OWNER_COPY))
    {
        VisItError("VariableData's owner must be set to VISIT_OWNER_SIM"
            ", VISIT_OWNER_VISIT, VISIT_OWNER_VISIT_EX, or VISIT_OWNER_COPY.");
        return VISIT_ERROR;
    }

    if (!simV2_ValidDataType(dataType))
    {
        VisItError("VariableData's data type must be set to one of: "
            "VISIT_DATATYPE_CHAR, VISIT_DATATYPE_INT, VISIT_DATATYPE_FLOAT, "
            "VISIT_DATATYPE_DOUBLE, VISIT_DATATYPE_LONG");
        return VISIT_ERROR;
    }

    if (nComps <= 0)
    {
        VisItError("VariableData's number of components must be greater than 0.");
        return VISIT_ERROR;
    }

    if (nTuples <= 0)
    {
        VisItError("VariableData's number of tuples must be greater than 0.");
        return VISIT_ERROR;
    }

    if (!data)
    {
        VisItError("VariableData's component data must not be NULL.");
        return VISIT_ERROR;
    }

    int retval = VISIT_ERROR;
    VisIt_VariableData *obj =
        GetVariableDataObject(h, "simv2_VariableData_setArrayData_internal");
    if (obj)
    {
        VisIt_ArrayData *arr = new VisIt_ArrayData;
        arr->memory = memory;
        arr->owner = owner;
        arr->dataType = dataType;
        arr->nComponents = nComps;
        arr->nTuples = nTuples;
        arr->data = data;
        arr->offset = offset;
        arr->stride = stride;

        // If we need to copy the data, do it here.
        retval = arr->CopyData();

        obj->SetArray(arrIndex, arr);
    }
    return retval;
}

/*******************************************************************************
 * Public functions, available to C
 ******************************************************************************/

int
simv2_VariableData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_VariableData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_VariableData_free(visit_handle h)
{
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_free");
    int retval = VISIT_ERROR;
    if (obj)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

/* Set functions */
int
simv2_VariableData_setData(visit_handle h, int owner, int dataType, int nComps,
    int nTuples, void *data)
{
    int arrIndex = 0;
    int memory = VISIT_MEMORY_CONTIGUOUS;
    int offset = 0;
    int stride = 0;
    switch(dataType)
    {
    case VISIT_DATATYPE_CHAR:
        stride = sizeof(char);
        break;
    case VISIT_DATATYPE_INT:
        stride = sizeof(int);
        break;
    case VISIT_DATATYPE_LONG:
        stride = sizeof(long);
        break;
    case VISIT_DATATYPE_FLOAT:
        stride = sizeof(float);
        break;
    case VISIT_DATATYPE_DOUBLE:
        stride = sizeof(double);
        break;
    default:
        break;
    }
    return simv2_VariableData_setArrayData_internal(h, arrIndex, memory,
               owner, dataType, nComps, nTuples, offset, stride, data);
}

int
simv2_VariableData_setDataEx(visit_handle h, int owner, int dataType, int nComps,
    int nTuples, void *data, void (*callback)(void*), void *callbackData)
{

    if (owner == VISIT_OWNER_VISIT_EX)
    {
        if (!callback)
        {
            VisItError("VISIT_OWNER_VISIT_EX specified "
                       "but a callback was not provided.");
            return VISIT_ERROR;
        }

        VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_setDataEx");
        if (!obj)
        {
            ostringstream oss;
            oss << "Error: " << __FILE__ << ":" << __LINE__<< endl
                << "Failed to locate the object from the given handle." << endl;
            VisItError(oss.str().c_str());
            return VISIT_ERROR;
        }

        obj->callback = callback;
        obj->callbackData = callbackData;
    }
    return simv2_VariableData_setData(h, owner, dataType, nComps, nTuples, data);
}

int
simv2_VariableData_setArrayData(visit_handle h, int arrIndex, int owner, int dataType,
    int nTuples, int offset, int stride, void *data)
{
    int memory = VISIT_MEMORY_STRIDED;
    int nComps = 1;

    // TODO -- enable zero copy for ref counted arrays etc. when you pass to
    // VTK you need to create a delete event observer to dec the ref count and
    // zero out libsim's array and delete callback before destructing libsim
    // object.
    if (owner == VISIT_OWNER_VISIT_EX)
    {
        VisItError("VISIT_OWNER_VISIT_EX is not allowed on array components.");
        return VISIT_ERROR;
    }

    return simv2_VariableData_setArrayData_internal(h, arrIndex, memory,
               owner, dataType, nComps, nTuples, offset, stride, data);
}

int
simv2_VariableData_setDeletionCallback(visit_handle h,
    void(*callback)(void*), void *callbackData)
{
    int retval = VISIT_ERROR;
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_setDeletionCallback");
    if (obj)
    {
        obj->callback = callback;
        obj->callbackData = callbackData;
    }
    else
    {
        ostringstream oss;
        oss << "Error: " << __FILE__ << ":" << __LINE__<< endl
            << "Failed to locate the object from the given handle." << endl;
        VisItError(oss.str().c_str());
    }

    return retval;
}

/* Get functions */

/**
 This function is callable from the public API via the _getData and _getDataX
 functions. The function is likely not used much.
 */
int
simv2_VariableData_getData2(visit_handle h, int *owner, int *dataType, int *nComps,
    int *nTuples, void **data)
{
    int arrIndex = 0;
    int memory = VISIT_MEMORY_CONTIGUOUS;
    int stride = 0, offset = 0;
    int retval = simv2_VariableData_getArrayData(h, arrIndex, memory,
                                                 *owner, *dataType, *nComps, *nTuples,
                                                 offset, stride, *data);
#if 0
    if (memory == VISIT_MEMORY_STRIDED)
    {
        VisItWarning("simv2_VariableData_getData2 does not return all "
                     "information about arrays that have strided memory access. "
                     "Instead, call simv2_VariableData_getArrayData.");
    }
#endif
    return retval;
}

/*******************************************************************************
 * C++ code callable from the SimV2 plugin and within the runtime
 ******************************************************************************/

int simv2_VariableData_getArrayData(visit_handle h,
                                    int arrIndex, int &memory,
                                    int &owner,   int &dataType,
                                    int &nComps,  int &nTuples,
                                    int &offset,  int &stride,
                                    void *&data)
{
    int retval = VISIT_ERROR;
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_getArrayData");
    if (obj)
    {
        VisIt_ArrayData *arr = obj->GetArray(arrIndex);
        if (arr)
        {
            if (!arr->data)
            {
                VisItError("The data array does not contain any data");
                return VISIT_ERROR;
            }

            memory = arr->memory;
            owner = arr->owner;
            dataType = arr->dataType;
            nComps = arr->nComponents;
            nTuples = arr->nTuples;
            offset = arr->offset;
            stride = arr->stride;
            data = arr->data;

            retval = VISIT_OKAY;
        }
    }
    else
    {
        ostringstream oss;
        oss << "Error: " << __FILE__ << ":" << __LINE__<< endl
            << "Failed to locate the object from the given handle." << endl;
        VisItError(oss.str().c_str());
    }
    return retval;
}

int
simv2_VariableData_getData(visit_handle h, int &owner, int &dataType, int &nComps,
    int &nTuples, void *&data)
{
    return simv2_VariableData_getData2(h, &owner, &dataType, &nComps, &nTuples, &data);
}

int
simv2_VariableData_getDeletionCallback(visit_handle h, void (*&callback)(void*), void *&callbackData)
{
    int retval = VISIT_ERROR;
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_getDeletionCallback");
    if (obj)
    {
        callback = obj->callback;
        callbackData = obj->callbackData;

        retval = VISIT_OKAY;
    }
    else
    {
        ostringstream oss;
        oss << "Error: " << __FILE__ << ":" << __LINE__<< endl
            << "Failed to locate the object from the given handle." << endl;
        VisItError(oss.str().c_str());
    }
    return retval;
}

int
simv2_VariableData_getNumArrays(visit_handle h, int *nArrays)
{
    int retval = VISIT_ERROR;
    *nArrays = 0;
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_getNumArrays");
    if (obj)
    {
        *nArrays = obj->GetNumberOfArrays();
        retval = VISIT_OKAY;
    }
    else
    {
        ostringstream oss;
        oss << "Error: " << __FILE__ << ":" << __LINE__<< endl
            << "Failed to locate the object from the given handle." << endl;
        VisItError(oss.str().c_str());
    }
    return retval;
}

int
simv2_VariableData_nullData(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_nullData");
    if (obj)
    {
        // NULL out the storage for all of the arrays because we stole them.
        for(int i = 0; i < obj->GetNumberOfArrays(); ++i)
        {
            VisIt_ArrayData *arr = obj->GetArray(i);
            if (arr)
            {
                arr->data = NULL;
                arr->nTuples = 0;
                arr->nComponents = 0;

                retval = VISIT_OKAY;
            }
        }

        obj->callback = NULL;
        obj->callbackData = NULL;
    }
    return retval;
}
