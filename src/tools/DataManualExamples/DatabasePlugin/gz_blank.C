// NOTE - This code incomplete and requires underlined portions
// to be replaced with code to read values from your file format.

#include <avtGhostData.h>
#include <vtkUnsignedCharArray.h>

vtkDataSet *
avtXXXXFileFormat::GetMesh(const char *meshname)
{
    // Code to create your mesh goes here.
    vtkDataSet *retval = CODE TO CREATE YOUR MESH;

    // Now that you have your mesh, figure out which cells need
    // to be removed.
    int nCells = retval->GetNumberOfCells();
    int *blanks = new int[nCells];
    READ nCells INTEGER VALUES INTO blanks ARRAY.

    // Now that we have the blanks array, create avtGhostZones.
    unsigned char realVal = 0, ghost = 0;
    avtGhostData::AddGhostZoneType(ghost, ZONE_NOT_APPLICABLE_TO_PROBLEM);
    vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
    ghostCells->SetName("avtGhostZones");
    ghostCells->Allocate(nCells);
    for(int i = 0; i < nCells; ++i)
    {
        if(blanks[i])
            ghostCells->InsertNextValue(realVal);
        else
            ghostCells->InsertNextValue(ghost);
    }
    retval->GetCellData()->AddArray(ghostCells);
    retval->SetUpdateGhostLevel(0);
    ghostCells->Delete();

    // Clean up
    delete [] blanks;

    return retval;
}
