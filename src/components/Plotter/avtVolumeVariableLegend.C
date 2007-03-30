// ************************************************************************* //
//                             avtVolumeVariableLegend.C                           //
// ************************************************************************* //

#include <vtkVerticalScalarBarWithOpacityActor.h>
#include <avtVolumeVariableLegend.h>
#include <vtkLookupTable.h>

// ****************************************************************************
// Method: avtVolumeVariableLegend::avtVolumeVariableLegend
//
// Purpose: 
//   Constructor for the avtVolumeVariableLegend class.
//
// Notes:
//   This constructor does not use the default constructor of avtVariableLegend.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 15:37:06 PST 2001
//
// Modifications:
//   
//   Hank Childs, Thu Mar 14 15:57:09 PST 2002
//   Make legend a bit smaller.
//
// ****************************************************************************

avtVolumeVariableLegend::avtVolumeVariableLegend() : avtVariableLegend(1)
{
    sBar = vtkVerticalScalarBarWithOpacityActor::New();
    sBar->SetShadow(0);
    sBar->SetTitle("VolumeVariable Plot");
    sBar->SetLookupTable(lut);

    size[0] = 0.08;
    size[1] = 0.17;
    sBar->SetWidth(size[0]);
    sBar->SetHeight(size[1]);

    SetLegendPosition(0.05, 0.7);

    //
    // Set the legend to also point to sBar, so the base methods will work
    // without us re-defining them.
    //
    legend = sBar;
    legend->Register(NULL);
}

// ****************************************************************************
// Method: avtVolumeVariableLegend::~avtVolumeVariableLegend
//
// Purpose: 
//   Destructor for the avtVolumeVariableLegend class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 15:37:49 PST 2001
//
// Modifications:
//   
// ****************************************************************************

avtVolumeVariableLegend::~avtVolumeVariableLegend()
{
    // nothing here
}

// ****************************************************************************
// Method: avtVolumeVariableLegend::SetLegendOpacities
//
// Purpose: 
//   Sets the opacity values that the legend needs.
//
// Arguments:
//   opacity : An array of 256 opacity values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 20 12:20:48 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
avtVolumeVariableLegend::SetLegendOpacities(const unsigned char *opacity)
{
    ((vtkVerticalScalarBarWithOpacityActor *)sBar)->SetLegendOpacities(opacity);
}
