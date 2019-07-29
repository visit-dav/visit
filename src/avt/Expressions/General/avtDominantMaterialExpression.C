// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtDominantMaterialExpression.C                     //
// ************************************************************************* //

#include <avtDominantMaterialExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIntArray.h>

#include <avtMaterial.h>
#include <avtMetaData.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDominantMaterialExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2008
//
// ****************************************************************************

avtDominantMaterialExpression::avtDominantMaterialExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDominantMaterialExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2008
//
// ****************************************************************************

avtDominantMaterialExpression::~avtDominantMaterialExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDominantMaterialExpression::DeriveVariable
//
//  Purpose:
//      Determines the number of the largest material in a cell.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     December 29, 2008
//
// ****************************************************************************

vtkDataArray *
avtDominantMaterialExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
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

    std::vector<std::string> matnames = mat->GetMaterials();
    std::vector<int> matnums(matnames.size());
    for (size_t i = 0 ; i < matnames.size() ; i++)
        matnums[i] = atoi(matnames[i].c_str());

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
    int offset = entry_size-1;

    //
    // Walk through the material data structure and determine the number of
    // materials for each cell.
    //
    const int *matlist  = mat->GetMatlist();
    const int *mix_next = mat->GetMixNext();
    const float *mix_vf = mat->GetMixVF();
    const int *mix_mat  = mat->GetMixMat();
    for (vtkIdType i = 0 ; i < ncells ; i++)
    {
        int dom_mat = 0;
        bool shouldSkip = false;
        if (entry_size == 2)
        {
            if ((int)ptr[entry_size*i + 0] != currentDomainsIndex)
            {
                dom_mat = 0;
                shouldSkip = true;
            }
        }
        if (!shouldSkip)
        {
            int zone = ptr[entry_size*i + offset];
            if (matlist[zone] >= 0)
            {
                dom_mat = matnums[matlist[zone]];
            }
            else
            {
                int current = -matlist[zone]-1;
                float biggest = 0.;
                int nmats = 1;
                bool keepGoing = true;
                while (keepGoing)
                {
                    if (mix_vf[current] > biggest)
                    {
                        dom_mat = matnums[mix_mat[current]];
                        biggest = mix_vf[current];
                    }

                    if (mix_next[current] == 0)
                        keepGoing = false;
                    else
                        current = mix_next[current]-1;

                    // nmats < 1000 just to prevent infinite loops if someone
                    // set this structure up wrong.
                    if (nmats++ > 1000)
                        keepGoing = false;
                }
            }
        }
        rv->SetValue(i, dom_mat);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtDominantMaterialExpression::ModifyContract
//
//  Purpose:
//      State that we need the zone numbers.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2008
//
//  Modifications:
//    Brad Whitlock, Mon Jun 25 16:11:56 PDT 2012
//    Turn off ghost zones to avoid a problem with 0's getting into the final
//    data array as a result of the mesh being larger than the avtMaterial used
//    as the source of the data values.
//
// ****************************************************************************

avtContract_p
avtDominantMaterialExpression::ModifyContract(avtContract_p c)
{
    avtContract_p rv = avtSingleInputExpressionFilter::ModifyContract(c);

    avtDataRequest_p ds = rv->GetDataRequest();
    ds->TurnZoneNumbersOn();

    // Turn off ghost zone communication because we need the avtMaterial that
    // we use to match the size of the mesh that we get. Theoretically, we can
    // use ds->SetNeedPostGhostMaterialInfo(true) to get an avtMaterial that is
    // the right size but that approach didn't work 100%. This approach, while
    // it may be less than ideal, does work when you do variable recentering, etc.
    // For the post-ghost approach, see avtPerMaterialValueExpression.
    ds->SetMaintainOriginalConnectivity(true);

    return rv;
}


