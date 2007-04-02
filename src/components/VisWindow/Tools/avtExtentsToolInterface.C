#include <avtExtentsToolInterface.h>
#include <ExtentsAttributes.h>


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


avtExtentsToolInterface::avtExtentsToolInterface(const VisWindow *v) : avtToolInterface(v)
{
    atts = new ExtentsAttributes;
}


avtExtentsToolInterface::~avtExtentsToolInterface()
{
    // nothing
}


void avtExtentsToolInterface::SetScalarNames(const stringVector &scalarNames_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetScalarNames(scalarNames_);
}


void avtExtentsToolInterface::SetScalarMinima(const doubleVector &scalarMinima_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetScalarMinima(scalarMinima_);
}


void avtExtentsToolInterface::SetScalarMaxima(const doubleVector &scalarMaxima_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetScalarMaxima(scalarMaxima_);
}


void avtExtentsToolInterface::SetMinima(const doubleVector &minima_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetMinima(minima_);
}


void avtExtentsToolInterface::SetMaxima(const doubleVector &maxima_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetMaxima(maxima_);
}


void avtExtentsToolInterface::SetMinTimeOrdinals(const intVector &minTimeOrdinals_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetMinTimeOrdinals(minTimeOrdinals_);
}


void avtExtentsToolInterface::SetMaxTimeOrdinals(const intVector &maxTimeOrdinals_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetMaxTimeOrdinals(maxTimeOrdinals_);
}


void avtExtentsToolInterface::SetLeftSliderX(const double leftSliderX_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetLeftSliderX(leftSliderX_);
}


void avtExtentsToolInterface::SetRightSliderX(const double rightSliderX_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetRightSliderX(rightSliderX_);
}


void avtExtentsToolInterface::SetSlidersBottomY(const double slidersBottomY_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetSlidersBottomY(slidersBottomY_);
}


void avtExtentsToolInterface::SetSlidersTopY(const double slidersTopY_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetSlidersTopY(slidersTopY_);
}


const stringVector &avtExtentsToolInterface::GetScalarNames() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetScalarNames();
}


const doubleVector &avtExtentsToolInterface::GetScalarMinima() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetScalarMinima();
}


const doubleVector &avtExtentsToolInterface::GetScalarMaxima() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetScalarMaxima();
}


const doubleVector &avtExtentsToolInterface::GetMinima() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetMinima();
}


const doubleVector &avtExtentsToolInterface::GetMaxima() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetMaxima();
}


const intVector &avtExtentsToolInterface::GetMinTimeOrdinals() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetMinTimeOrdinals();
}


const intVector &avtExtentsToolInterface::GetMaxTimeOrdinals() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetMaxTimeOrdinals();
}


double avtExtentsToolInterface::GetLeftSliderX() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetLeftSliderX();
}


double avtExtentsToolInterface::GetRightSliderX() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetRightSliderX();
}


double avtExtentsToolInterface::GetSlidersBottomY() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetSlidersBottomY();
}


double avtExtentsToolInterface::GetSlidersTopY() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetSlidersTopY();
}
