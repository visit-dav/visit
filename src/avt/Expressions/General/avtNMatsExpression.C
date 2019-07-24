// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtNMatsExpression.C                            //
// ************************************************************************* //

#include <avtNMatsExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>

#include <avtMaterial.h>
#include <avtMetaData.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtNMatsExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNMatsExpression::avtNMatsExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtNMatsExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNMatsExpression::~avtNMatsExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtNMatsExpression::DeriveVariable
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
//
//    Sean Ahern, Thu Aug 21 12:17:27 PDT 2003
//    Fixed up a small error and a warning.
//
//    Hank Childs, Wed Feb 11 11:23:28 PST 2004
//    Fix bug with calculation of mixed zones.  Also operate on zones where
//    the connectivity has changed.
//
//    Kathleen Bonnell, Mon Jun 28 07:48:55 PDT 2004
//    Send currentTimeState to GetMaterial. 
//
// ****************************************************************************

vtkDataArray *
avtNMatsExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    vtkIdType ncells = in_ds->GetNumberOfCells();

    //
    // The 'currentDomainsIndex' is a data member of the base class that is
    // set to be the id of the current domain right before DeriveVariable is
    // called.  We need that index to make sure we are getting the right mat.
    //
    // The 'currentTimeState' is a data member of the base class that is
    // set to be the current timestep during ExamineContract. 
    // We need that timestep to make sure we are getting the right mat.
    //
 
    avtMaterial *mat = GetMetaData()->GetMaterial(currentDomainsIndex,
                                                  currentTimeState);
    vtkIntArray *rv = vtkIntArray::New();
    rv->SetNumberOfTuples(ncells);

    vtkDataArray *zn =in_ds->GetCellData()->GetArray("avtOriginalCellNumbers");
    if (zn == NULL)
    {
        debug1 << "Unable to find original cell numbers (needed for nmats)."
               << endl;
        return NULL;
    }
    unsigned int *ptr = (unsigned int *) zn->GetVoidPointer(0);
    vtkIdType entry_size = zn->GetNumberOfComponents();
    vtkIdType offset = entry_size-1;

    //
    // Walk through the material data structure and determine the number of
    // materials for each cell.
    //
    const int *matlist = mat->GetMatlist();
    const int *mix_next = mat->GetMixNext();
    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        int nmats = 0;
        bool shouldSkip = false;
        if (entry_size == 2)
        {
            if (ptr[entry_size*i + 0] != (unsigned int)currentDomainsIndex)
            {
                nmats = 1;
                shouldSkip = true;
            }
        }
        if (!shouldSkip)
        {
            int zone = ptr[entry_size*i + offset];
            if (matlist[zone] >= 0)
            {
                nmats = 1;
            }
            else
            {
                int current = -matlist[zone]-1;
                nmats = 1;
                // nmats < 1000 just to prevent infinite loops if someone
                // set this structure up wrong.
                while ((mix_next[current] != 0) && (nmats < 1000))
                {
                    current = mix_next[current]-1;
                    nmats++;
                }
            }
        }
        rv->SetValue(i, nmats);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtNMatsExpression::ModifyContract
//
//  Purpose:
//      State that we need the zone numbers.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2004
//
// ****************************************************************************

avtContract_p
avtNMatsExpression::ModifyContract(avtContract_p spec)
{
    avtContract_p rv = 
                      avtSingleInputExpressionFilter::ModifyContract(spec);

    avtDataRequest_p ds = spec->GetDataRequest();
    ds->TurnZoneNumbersOn();

    return rv;
}


