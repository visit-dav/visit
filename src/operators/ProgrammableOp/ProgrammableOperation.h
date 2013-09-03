#ifndef PROGRAMMABLEOPERATION_H
#define PROGRAMMABLEOPERATION_H

#include <vectortypes.h>
#include <MapNode.h>

#include <avtDataset.h>
#include <avtDataObject.h>
#include <avtContract.h>
#include <vtkAbstractArray.h>


class avtPythonFilterEnvironment;

/// a shaped VTK DataArray
struct vtkShapedDataArray
{
    intVector shape;
    vtkAbstractArray* vtkarray;
    vtkShapedDataArray()
    {
        vtkarray = 0;
    }
};

class ProgrammableOpArguments
{
    friend class ProgrammableOpFilter;
public:
    int input_domain;
    avtDataObject_p input;
    avtContract_p contract;
    vtkDataSet* input_mesh;
    avtPythonFilterEnvironment* pythonFilter;

    std::vector<Variant> args; ///simplest case..

    /// dataArray with shape information..
    /// I am really trying to avoid bringing in the NUMPY API
    /// but if this does not work then I will pursue that route..
    std::map<int, vtkShapedDataArray> dataArrayMap;
    std::map<int, std::vector<Variant> > variantVector;

    /// functions..
    int GetInputDomain() { return input_domain; }
    vtkDataSet* GetInputDataSet() { return input_mesh; }
    avtDataObject_p GetInput() { return input; }
    avtContract_p GetContract() { return contract; }
    avtPythonFilterEnvironment* GetPythonEnvironment() { return pythonFilter; }


    size_t getArgSize() { args.size(); }
    Variant getArg(size_t i) { return args[i]; }

    void* getArgAsVoidPtr(size_t i)
    {
//        if(datamap.count(i) > 0)
//            return datamap[i];
        return (void*)&args[i];
    }

    vtkShapedDataArray getArgAsShapedDataArray(int i)
    {
        if(dataArrayMap.count(i) > 0)
            return dataArrayMap[i];
        return vtkShapedDataArray();
    }

    std::vector<Variant> getArgAsVariantVector(int i)
    {
        if(variantVector.count(i) > 0)
            return variantVector[i];
    
        std::vector<Variant> tmp;
        tmp.push_back(args[i]);
        return tmp;
    }
};

class ProgrammableOperation;

class ProgrammableOpManager
{
public:
    virtual void Register(ProgrammableOperation* operation) = 0;
};

class ProgrammableOpController
{
public:
    virtual void RegisterOperations(ProgrammableOpManager* parent) = 0;
};

class ProgrammableOperation
{
public:
    enum ResponseType
    {
        CONSTANT,
        VTK_DATA_ARRAY,
        VTK_MULTI_DIMENSIONAL_DATA_ARRAY,
        VTK_DATASET,
        AVT_DATA_SET
    };

    /// TODO: I really want to use the Variant/MapNode class here
    /// but I also need an ability to return VTK_DATA_ARRAY_TYPE as an option..
    /// not sure how I am going to honor vtkDataSets from R?
    typedef enum
    {
        EMPTY_TYPE = 0,
        BOOL_TYPE, CHAR_TYPE, UNSIGNED_CHAR_TYPE, INT_TYPE, LONG_TYPE,
        FLOAT_TYPE, DOUBLE_TYPE, STRING_TYPE,
        BOOL_VECTOR_TYPE, CHAR_VECTOR_TYPE, UNSIGNED_CHAR_VECTOR_TYPE,
        INT_VECTOR_TYPE, LONG_VECTOR_TYPE, FLOAT_VECTOR_TYPE,
        DOUBLE_VECTOR_TYPE, STRING_VECTOR_TYPE, 
        VARIANT_TYPE, VARIANT_VECTOR_TYPE,
        VTK_DATA_ARRAY_TYPE, VTK_DATASET_TYPE, VTK_AVTDATASET_TYPE
    } ScriptType;

    //DRP. Return type based methods....
    virtual bool func(ProgrammableOpArguments& args,
                      Variant& result) 
    { 
        (void) args; 
        (void) result; 
        return false; 
    }

    virtual bool func(ProgrammableOpArguments& args,
                      vtkShapedDataArray& result)
    { 
        (void) args; 
        (void) result; 
        return false; 
    }

    virtual bool func(ProgrammableOpArguments& args,
                      vtkDataSet*& result)
    {  
        (void) args; 
        (void) result; 
        return false; 
    }

    virtual bool func(ProgrammableOpArguments& args,
                      avtDataset_p& result)
    { 
        (void) args; 
        (void) result; 
        return false; 
    }

    virtual ResponseType getSignature(std::string& name,
                              stringVector& argnames,
                              std::vector<ScriptType>& argtypes) = 0;
};

#endif
