// ************************************************************************* //
//                            avtVolumeVariableLegend.h                            //
// ************************************************************************* //

#ifndef AVT_VOLUME_VARIABLE_LEGEND_H
#define AVT_VOLUME_VARIABLE_LEGEND_H
#include <plotter_exports.h>
#include <avtVariableLegend.h>

// ****************************************************************************
//  Class: avtVolumeVariableLegend
//
//  Purpose:
//      The legend for any volume rendering plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 19 15:33:32 PST 2001
//
//  Modifications:
//
// ****************************************************************************

class PLOTTER_API avtVolumeVariableLegend : public avtVariableLegend
{
  public:
                               avtVolumeVariableLegend();
    virtual                   ~avtVolumeVariableLegend();
    void                       SetLegendOpacities(const unsigned char *opacity);
};

#endif
