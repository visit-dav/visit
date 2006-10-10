#ifndef VISIT_EXTENTS_TOOL_INTERFACE_H
#define VISIT_EXTENTS_TOOL_INTERFACE_H
#include <viswindow_exports.h>
#include <avtToolInterface.h>

#include <vectortypes.h>


// ****************************************************************************
// Class: avtExtentsToolInterface
//
// Purpose: This class contains the information passed to users of the
//          extents tool.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 16:11:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

class VISWINDOW_API avtExtentsToolInterface : public avtToolInterface
{
public:
    avtExtentsToolInterface(const VisWindow *v);
    virtual ~avtExtentsToolInterface();

    void SetScalarNames(const stringVector &scalarNames_);
    void SetScalarMinima(const doubleVector &scalarMinima_);
    void SetScalarMaxima(const doubleVector &scalarMaxima_);
    void SetMinima(const doubleVector &minima_);
    void SetMaxima(const doubleVector &maxima_);
    void SetMinTimeOrdinals(const intVector &minTimeOrdinals_);
    void SetMaxTimeOrdinals(const intVector &maxTimeOrdinals_);
    void SetLeftSliderX(const double leftSliderX_);
    void SetRightSliderX(const double rightSliderX_);
    void SetSlidersBottomY(const double slidersBottomY_);
    void SetSlidersTopY(const double slidersTopY_);

    const stringVector &GetScalarNames() const;
    const doubleVector &GetScalarMinima() const;
    const doubleVector &GetScalarMaxima() const;
    const doubleVector &GetMinima() const;
    const doubleVector &GetMaxima() const;
    const intVector &GetMinTimeOrdinals() const;
    const intVector &GetMaxTimeOrdinals() const;
    double GetLeftSliderX() const;
    double GetRightSliderX() const;
    double GetSlidersBottomY() const;
    double GetSlidersTopY() const;
};

#endif
