// ************************************************************************* //
//                               avtNMatsFilter.C                            //
// ************************************************************************* //

#include <avtNMatsFilter.h>

#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <avtMaterial.h>
#include <avtMetaData.h>


// ****************************************************************************
//  Method: avtNMatsFilter::DeriveVariable
//
//  Purpose:
//      Determines the number of materials in a cell.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     August 21, 2003
//
//  Modifications:
//      Sean Ahern, Thu Aug 21 12:17:27 PDT 2003
//      Fixed up a small error and a warning.
//
// ****************************************************************************

vtkDataArray *
avtNMatsFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int    i;

    int ncells = in_ds->GetNumberOfCells();

    //
    // The 'currentDomainsIndex' is a data member of the base class that is
    // set to be the id of the current domain right before DeriveVariable is
    // called.  We need that index to make sure we are getting the right mat.
    //
    avtMaterial *mat = GetMetaData()->GetMaterial(currentDomainsIndex);
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(ncells);

    //
    // Walk through the material data structure and determine the number of
    // materials for each cell.
    //
    const int *matlist = mat->GetMatlist();
    const int *mix_next = mat->GetMixNext();
    for (i = 0 ; i < ncells ; i++)
    {
        int nmats = 0;
        if (matlist[i] >= 0)
        {
            nmats = 1;
        }
        else
        {
            int start = -matlist[i]-1;
            nmats = mix_next[start] - start;
        }
        rv->SetTuple1(i, (float) nmats);
    }

    return rv;
}


