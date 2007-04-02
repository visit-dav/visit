#ifndef EXTENTSATTRIBUTES_H
#define EXTENTSATTRIBUTES_H
#include <state_exports.h>
#include <AttributeSubject.h>

#define EA_DEFAULT_NUMBER_OF_EXTENTS    2

#define EA_DEFAULT_LEFT_SLIDER_X        0.04
#define EA_DEFAULT_RIGHT_SLIDER_X       0.96
#define EA_DEFAULT_SLIDERS_BOTTOM_Y     0.12
#define EA_DEFAULT_SLIDERS_TOP_Y        0.88

// ****************************************************************************
// Class: ExtentsAttributes
//
// Purpose: Attributes for a list of extents set by sliders
//
// Programmer: Mark Blair
// Creation:   Thu Oct 27 19:36:00 PDT 2005
//
// Modifications:
//   
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added methods and members related to slider change time ordinals.
//   
// ****************************************************************************

class STATE_API ExtentsAttributes : public AttributeSubject
{
public:
    ExtentsAttributes();
    ExtentsAttributes(const ExtentsAttributes &obj);
    virtual ~ExtentsAttributes();

    virtual void operator = (const ExtentsAttributes &obj);
    virtual bool operator == (const ExtentsAttributes &obj) const;
    virtual bool operator != (const ExtentsAttributes &obj) const;

    virtual const std::string TypeName() const;
    virtual bool              CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property setting methods
    void SetScalarNames(const stringVector &scalarNames_);
    void SetScalarMinima(const doubleVector &scalarMinima_);
    void SetScalarMaxima(const doubleVector &scalarMaxima_);
    void SetMinima(const doubleVector &minima_);
    void SetMaxima(const doubleVector &maxima_);
    void SetMinTimeOrdinals(const intVector &minTimeOrdinals_);
    void SetMaxTimeOrdinals(const intVector &maxTimeOrdinals_);
    void SetLeftSliderX(double leftSliderX_);
    void SetRightSliderX(double rightSliderX_);
    void SetSlidersBottomY(double slidersBottomY_);
    void SetSlidersTopY(double slidersTopY_);

    // Property getting methods
    const stringVector &GetScalarNames() const;
    const doubleVector &GetScalarMinima() const;
    const doubleVector &GetScalarMaxima() const;
    const doubleVector &GetMinima() const;
    const doubleVector &GetMaxima() const;
    const intVector    &GetMinTimeOrdinals() const;
    const intVector    &GetMaxTimeOrdinals() const;
    double              GetLeftSliderX() const;
    double              GetRightSliderX() const;
    double              GetSlidersBottomY() const;
    double              GetSlidersTopY() const;

    int                 GetNumberOfExtents() const;

    // Property selection methods
    virtual void SelectAll();
    void SelectScalarNames();
    void SelectScalarMinima();
    void SelectScalarMaxima();
    void SelectMinima();
    void SelectMaxima();
    void SelectMinTimeOrdinals();
    void SelectMaxTimeOrdinals();
    void SelectLeftSliderX();
    void SelectRightSliderX();
    void SelectSlidersBottomY();
    void SelectSlidersTopY();

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

private:
    stringVector scalarNames;
    doubleVector scalarMinima;
    doubleVector scalarMaxima;
    doubleVector minima;
    doubleVector maxima;
    intVector    minTimeOrdinals;
    intVector    maxTimeOrdinals;

    double       leftSliderX;
    double       rightSliderX;
    double       slidersBottomY;
    double       slidersTopY;
};

#endif
