#ifndef SIMV2_GET_MESH_H
#define SIMV2_GET_MESH_H

class avtPolyhedralSplit;

#define NOTHING(CODE)

#define simV2MemoryCopyMacro(FUNC, MEMORY, NTUPLES, OFFSET, STRIDE, CPPTYPE, DATA, ERROR) \
if(MEMORY == VISIT_MEMORY_CONTIGUOUS) \
{ \
    CPPTYPE *src = reinterpret_cast<CPPTYPE*>(DATA); \
    for(int _i = 0; _i < NTUPLES; ++_i) \
        FUNC(_i, src[_i]); \
    ERROR = false; \
} \
else if(MEMORY == VISIT_MEMORY_STRIDED) \
{ \
    unsigned char *base = reinterpret_cast<unsigned char *>(DATA); \
    unsigned char *ptr = base + OFFSET; \
    for(int _i = 0; _i < NTUPLES; ++_i) \
    { \
        const CPPTYPE *src = reinterpret_cast<const CPPTYPE *>(ptr); \
        FUNC(_i, *src); \
        ptr += STRIDE; \
    } \
    ERROR = false; \
}

#define simV2TemplateDataArrayIterateMacro(H, COMP, SETNTUPLES, FUNC, ERROR) \
{ \
    ERROR = true; \
    int _owner, _memory, _offset, _stride, _dataType, _nComps, _nTuples = 0; \
    void *_data = 0; \
    if(simv2_VariableData_getArrayData(H, COMP, _memory, _owner, _dataType,  \
        _nComps, _nTuples, _offset, _stride, _data)) \
    { \
        switch (_dataType) \
        { \
        simV2TemplateMacro( \
            SETNTUPLES(_nTuples); \
            simV2MemoryCopyMacro(FUNC, _memory, _nTuples, _offset, _stride, simV2_TT::cppType, _data, ERROR);\
        ); \
        } \
    } \
}

int SimV2_GetVTKType(int simv2type);

vtkDataSet *SimV2_GetMesh_Curvilinear(visit_handle h);
vtkDataSet *SimV2_GetMesh_Rectilinear(visit_handle h);
vtkDataSet *SimV2_GetMesh_Unstructured(int, visit_handle h, avtPolyhedralSplit **);
vtkDataSet *SimV2_GetMesh_Point(visit_handle h);
vtkDataSet *SimV2_GetMesh_CSG(visit_handle h);

#endif
