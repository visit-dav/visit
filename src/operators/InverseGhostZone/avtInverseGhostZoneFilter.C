// ************************************************************************* //
//  File: avtInverseGhostZoneFilter.C
// ************************************************************************* //

#include <avtInverseGhostZoneFilter.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtInverseGhostZoneFilter constructor
//
//  Programmer: childs<generated>
//  Creation:   July27,2001
//
// ****************************************************************************

avtInverseGhostZoneFilter::avtInverseGhostZoneFilter()
{
}


// ****************************************************************************
//  Method: avtInverseGhostZoneFilter destructor
//
//  Programmer: childs<generated>
//  Creation:   July27,2001
//
//  Modifications:
//
// ****************************************************************************

avtInverseGhostZoneFilter::~avtInverseGhostZoneFilter()
{
}


// ****************************************************************************
//  Method:  avtInverseGhostZoneFilter::Create
//
//  Programmer:  childs<generated>
//  Creation:    July27,2001
//
// ****************************************************************************

avtFilter *
avtInverseGhostZoneFilter::Create()
{
    return new avtInverseGhostZoneFilter();
}


// ****************************************************************************
//  Method:      avtInverseGhostZoneFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer:  childs<generated>
//  Creation:    July27,2001
//
// ****************************************************************************

void
avtInverseGhostZoneFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const InverseGhostZoneAttributes*)a;
}


// ****************************************************************************
//  Method: avtInverseGhostZoneFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtInverseGhostZoneFilter with the given
//      parameters would result in an equivalent avtInverseGhostZoneFilter.
//
//  Programmer: childs<generated>
//  Creation:   July27,2001
//
// ****************************************************************************

bool
avtInverseGhostZoneFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(InverseGhostZoneAttributes*)a);
}


// ****************************************************************************
//  Method: avtInverseGhostZoneFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the InverseGhostZone
//      filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: childs<generated>
//  Creation:   July27,2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov  2 11:32:17 PST 2001
//    Return an empty dataset if there are no ghost zones.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Wed Sep 11 08:41:39 PDT 2002
//    Fixed memory leak.
//
//    Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002   
//    Use NewInstance instead of MakeObject in order to match vtk's new api. 
//    
// ****************************************************************************

vtkDataSet *
avtInverseGhostZoneFilter::ExecuteData(vtkDataSet *in_ds, int dom, std::string)
{
    vtkDataArray *gz = in_ds->GetCellData()->GetArray("vtkGhostLevels");
    if (gz == NULL)
    {
        debug1 << "Domain: " << dom << ", no ghost levels to invert." << endl;
        return NULL;
    }   

    //
    // Make an output that is identical to the input.
    //
    vtkDataSet *out_ds = (vtkDataSet *) in_ds->NewInstance();
    out_ds->ShallowCopy(in_ds);

    int nCells = in_ds->GetNumberOfCells();
    unsigned char *gza = ((vtkUnsignedCharArray*)gz)->GetPointer(0);
    vtkUnsignedCharArray *ghostZones = vtkUnsignedCharArray::New();
    ghostZones->SetName("vtkGhostLevels");
    ghostZones->Allocate(nCells);
    for (int i = 0 ; i < nCells ; i++)
    {
        unsigned char ghost = (gza[i] == 0 ? 1 : 0);
        ghostZones->InsertNextValue(ghost);
    }
    out_ds->GetCellData()->RemoveArray("vtkGhostLevels");
    out_ds->GetCellData()->AddArray(ghostZones);

    if (atts.GetConstantData())
    {
        vtkDataArray *pd = out_ds->GetPointData()->GetScalars();
        if (pd != NULL)
        {
            int np = out_ds->GetNumberOfPoints();
            for (int j = 0 ; j < np ; j++)
            {
                pd->SetTuple1(j, 0.);
            }
        }
        vtkDataArray *cd = out_ds->GetCellData()->GetScalars();
        if (cd != NULL)
        {
            for (int j = 0 ; j < nCells ; j++)
            {
                cd->SetTuple1(j, 0.);
            }
        }
    }
    ManageMemory(out_ds);
    out_ds->Delete();
    return out_ds;
}


// ****************************************************************************
//  Method: avtInverseGhostZoneFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Indicates the zones no longer correspond to the original problem.
//      (They of course do, but information about the zones, like facelists,
//       no longer apply).
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2001
//
// ****************************************************************************

void
avtInverseGhostZoneFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


