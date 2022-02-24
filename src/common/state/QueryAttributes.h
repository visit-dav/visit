// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QUERYATTRIBUTES_H
#define QUERYATTRIBUTES_H
#include <state_exports.h>
#include <string>
#include <AttributeSubject.h>

#include <visitstream.h>

// ****************************************************************************
// Class: QueryAttributes
//
// Purpose:
//    This class contains attributes used for query.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class STATE_API QueryAttributes : public AttributeSubject
{
public:
    enum VarType
    {
        Mesh,
        Scalar,
        Vector,
        Tensor,
        Symmetric_Tensor,
        Array,
        Label,
        Material,
        Species,
        Curve,
        Unknown
    };

    // These constructors are for objects of this class
    QueryAttributes();
    QueryAttributes(const QueryAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    QueryAttributes(private_tmfs_t tmfs);
    QueryAttributes(const QueryAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~QueryAttributes();

    virtual QueryAttributes& operator = (const QueryAttributes &obj);
    virtual bool operator == (const QueryAttributes &obj) const;
    virtual bool operator != (const QueryAttributes &obj) const;
private:
    void Init();
    void Copy(const QueryAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectResultsMessage();
    void SelectResultsValue();
    void SelectFloatResultsValue();
    void SelectVarTypes();
    void SelectXUnits();
    void SelectYUnits();
    void SelectFloatFormat();
    void SelectXmlResult();
    void SelectQueryInputParams();
    void SelectDefaultName();
    void SelectDefaultVars();

    // Property setting methods
    void SetResultsMessage(const std::string &resultsMessage_);
    void SetResultsValue(const doubleVector &resultsValue_);
    void SetFloatResultsValue(const floatVector &floatResultsValue_);
    void SetTimeStep(int timeStep_);
    void SetVarTypes(const intVector &varTypes_);
    void SetPipeIndex(int pipeIndex_);
    void SetXUnits(const std::string &xUnits_);
    void SetYUnits(const std::string &yUnits_);
    void SetFloatFormat(const std::string &floatFormat_);
    void SetXmlResult(const std::string &xmlResult_);
    void SetSuppressOutput(bool suppressOutput_);
    void SetQueryInputParams(const MapNode &queryInputParams_);
    void SetDefaultName(const std::string &defaultName_);
    void SetDefaultVars(const stringVector &defaultVars_);

    // Property getting methods
    const std::string  &GetResultsMessage() const;
          std::string  &GetResultsMessage();
    const doubleVector &GetResultsValue() const;
          doubleVector &GetResultsValue();
    const floatVector  &GetFloatResultsValue() const;
          floatVector  &GetFloatResultsValue();
    int                GetTimeStep() const;
    const intVector    &GetVarTypes() const;
          intVector    &GetVarTypes();
    int                GetPipeIndex() const;
    const std::string  &GetXUnits() const;
          std::string  &GetXUnits();
    const std::string  &GetYUnits() const;
          std::string  &GetYUnits();
    const std::string  &GetFloatFormat() const;
          std::string  &GetFloatFormat();
    const std::string  &GetXmlResult() const;
          std::string  &GetXmlResult();
    bool               GetSuppressOutput() const;
    const MapNode      &GetQueryInputParams() const;
          MapNode      &GetQueryInputParams();
    const std::string  &GetDefaultName() const;
          std::string  &GetDefaultName();
    const stringVector &GetDefaultVars() const;
          stringVector &GetDefaultVars();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Enum conversion functions
    static std::string VarType_ToString(VarType);
    static bool VarType_FromString(const std::string &, VarType &);
protected:
    static std::string VarType_ToString(int);
public:

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    void SetVariables(const stringVector &variables_);
    const std::string &GetName() const;
    std::string &GetName();
    const stringVector &GetVariables() const;
    stringVector &GetVariables();
    void Reset();
    void PrintSelf(ostream &os);
    void SetResultsValue(const double);
    void SetResultsValues(const double*, const int);
    QueryAttributes &operator=(QueryAttributes&&);
    QueryAttributes(QueryAttributes&&);
    void Move(QueryAttributes&&);

    // IDs that can be used to identify fields in case statements
    enum {
        ID_resultsMessage = 0,
        ID_resultsValue,
        ID_floatResultsValue,
        ID_timeStep,
        ID_varTypes,
        ID_pipeIndex,
        ID_xUnits,
        ID_yUnits,
        ID_floatFormat,
        ID_xmlResult,
        ID_suppressOutput,
        ID_queryInputParams,
        ID_defaultName,
        ID_defaultVars,
        ID__LAST
    };

private:
    std::string  resultsMessage;
    doubleVector resultsValue;
    floatVector  floatResultsValue;
    int          timeStep;
    intVector    varTypes;
    int          pipeIndex;
    std::string  xUnits;
    std::string  yUnits;
    std::string  floatFormat;
    std::string  xmlResult;
    bool         suppressOutput;
    MapNode      queryInputParams;
    std::string  defaultName;
    stringVector defaultVars;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define QUERYATTRIBUTES_TMFS "sd*f*ii*issssbmss*"

#endif
