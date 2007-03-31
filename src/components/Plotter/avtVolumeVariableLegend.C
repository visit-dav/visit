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
//   Eric Brugger, Mon Jul 14 15:54:59 PDT 2003
//   Remove initialization of scalar bar title.
//
//   Eric Brugger, Wed Jul 16 16:04:18 PDT 2003
//   I added barVisibility and rangeVisibility.  I changed the default
//   size and position of the legend.
//
// ****************************************************************************

avtVolumeVariableLegend::avtVolumeVariableLegend() : avtVariableLegend(1)
{
    sBar = vtkVerticalScalarBarWithOpacityActor::New();
    sBar->SetShadow(0);
    sBar->SetLookupTable(lut);

    size[0] = 0.08;
    size[1] = 0.26;
    sBar->SetPosition2(size[0], size[1]);

    SetLegendPosition(0.05, 0.72);

    barVisibility = 1;
    rangeVisibility = 1;

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
