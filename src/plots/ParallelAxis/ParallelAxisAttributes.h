#ifndef PARALLEL_AXIS_ATTRIBUTES_H
#define PARALLEL_AXIS_ATTRIBUTES_H

#define AXIS_VAR_DATA_FILE_NAME   "__AXIS_VARIABLE_DATA__"

#define PCP_LEFT_AXIS_X_FRACTION          0.04
#define PCP_RIGHT_AXIS_X_FRACTION         0.96
#define PCP_H_BOTTOM_AXIS_Y_FRACTION      0.09
#define PCP_V_BOTTOM_AXIS_Y_FRACTION      0.12
#define PCP_H_TOP_AXIS_Y_FRACTION         0.93
#define PCP_V_TOP_AXIS_Y_FRACTION         0.88

#define PCP_MAX_HORIZONTAL_TITLE_AXES     7

#include <AttributeSubject.h>

#include <vector>
#include <string>


// ****************************************************************************
// Class: ParallelAxisAttributes
//
// Purpose: Maintains plot attributes for the ParallelAxis plot.
//
// Notes:
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//   
//    Mark Blair, Wed Sep 20 10:59:41 PDT 2006
//    Added time ordinals, for those operators and tools that need them.
//   
// ****************************************************************************

class ParallelAxisAttributes : public AttributeSubject
{
public:
    ParallelAxisAttributes();
    ParallelAxisAttributes(const ParallelAxisAttributes &obj);
    virtual ~ParallelAxisAttributes();

    virtual ParallelAxisAttributes& operator = (const ParallelAxisAttributes &obj);
    virtual bool operator == (const ParallelAxisAttributes &obj) const;
    virtual bool operator != (const ParallelAxisAttributes &obj) const;

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property setting methods
    void SetOrderedAxisNames(const stringVector &orderedAxisNames_);
    void SetShownVariableAxisPosition(int shownVarAxisPosition_);
    void SetAxisMinima(const doubleVector &axisMinima_);
    void SetAxisMaxima(const doubleVector &axisMaxima_);
    void SetExtentMinima(const doubleVector &extentMinima_);
    void SetExtentMaxima(const doubleVector &extentMaxima_);
    void SetExtMinTimeOrds(const intVector &extMinTimeOrds_);
    void SetExtMaxTimeOrds(const intVector &extMaxTimeOrds_);

    // Property changing methods
    void InsertAxis(const std::string &axisName_);
    void DeleteAxis(const std::string &axisName_, int minAxisCount);
    void SwitchToLeftAxis(const std::string &axisName_);
    void ShowPreviousAxisVariableData();
    void ShowNextAxisVariableData();
    bool UpdateAxisBoundsIfPossible();

    // Property getting methods
    const stringVector          &GetOrderedAxisNames() const;
    const std::string           &GetShownVariableAxisName() const;
    std::string                 &GetShownVariableAxisName();
    int                          GetShownVariableAxisPosition() const;
    int                          GetShownVariableAxisNormalHumanPosition() const;
    double                       GetShownVariableAxisMinimum() const;
    double                       GetShownVariableAxisMaximum() const;
    double                       GetShownVariableExtentMinimum() const;
    double                       GetShownVariableExtentMaximum() const;
    const doubleVector          &GetAxisMinima() const;
    const doubleVector          &GetAxisMaxima() const;
    const doubleVector          &GetExtentMinima() const;
    const doubleVector          &GetExtentMaxima() const;
    const intVector             &GetExtMinTimeOrds() const;
    const intVector             &GetExtMaxTimeOrds() const;

    // Property selection methods
    virtual void        SelectAll();

    void                SelectOrderedAxisNames();
    void                SelectShownVarAxisPosition();
    void                SelectAxisMinima();
    void                SelectAxisMaxima();
    void                SelectExtentMinima();
    void                SelectExtentMaxima();
    void                SelectExtMinTimeOrds();
    void                SelectExtMaxTimeOrds();

    // Python compatibility methods
    void                SetShownVarAxisPosition(int);

    stringVector       &GetOrderedAxisNames();
    int                 GetShownVarAxisPosition() const;
    doubleVector       &GetAxisMinima();
    doubleVector       &GetAxisMaxima();
    doubleVector       &GetExtentMinima();
    doubleVector       &GetExtentMaxima();
    intVector          &GetExtMinTimeOrds();
    intVector          &GetExtMaxTimeOrds();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index,
                                          const AttributeGroup *rhs) const;

    // User-defined methods
    bool ChangesRequireRecalculation(const ParallelAxisAttributes &obj);
    bool AttributesAreConsistent() const;

private:
    stringVector        orderedAxisNames;
    int                 shownVarAxisPosition;
    doubleVector        axisMinima;
    doubleVector        axisMaxima;
    doubleVector        extentMinima;
    doubleVector        extentMaxima;
    intVector           extMinTimeOrds;
    intVector           extMaxTimeOrds;
};

#endif
