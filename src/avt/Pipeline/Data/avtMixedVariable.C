// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtMixedVariable.C                              //
// ************************************************************************* //

#include <avtMixedVariable.h>

// For NULL
#include <stdio.h>

#include <avtMaterial.h>

using std::string;


// ****************************************************************************
//  Method: avtMixedVariable constructor
//
//  Arguments:
//      b       The mixed variable buffer
//      ml      The number of elements in b.
//      vn      The variable name with this mixed var.
//
//  Programmer: Hank Childs
//  Creation:   September 27, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Tue Dec  4 13:44:28 PST 2001
//    Added const to constructor arguments.
//
//    Hank Childs, Thu Jul  4 13:02:54 PDT 2002
//    Add variable names.
//
// ****************************************************************************

avtMixedVariable::avtMixedVariable(const float *b, int ml, string vn)
{
    mixlen  = ml;
    varname = vn;
    buffer = new float[mixlen];
    for (int i = 0 ; i < mixlen ; i++)
    {
        buffer[i] = b[i];
    }
}


// ****************************************************************************
//  Method: avtMixedVariable destructor
//
//  Programmer: Hank Childs
//  Creation:   September 27, 2001
//
// ****************************************************************************

avtMixedVariable::~avtMixedVariable()
{
    if (buffer != NULL)
    {
        delete [] buffer;
        buffer = NULL;
    }
}


// ****************************************************************************
//  Method: avtMixedVariable::Destruct
//
//  Purpose:
//      Destructs a mixed variable.  This is for void-reference purposes.
//
//  Programmer: Hank Childs
//  Creation:   September 24, 2002
//
// ****************************************************************************

void
avtMixedVariable::Destruct(void *p)
{
    avtMixedVariable *mv = (avtMixedVariable *) p;
    if (mv != NULL)
    {
        delete mv;
    }
}


// ****************************************************************************
//  Method: avtMixedVariable::GetValuesForZone
//
//  Purpose:
//    Constructs the per material value list for the given zone. 
//
//  Mote: This will return 0 for all materials in the non-mixed case.
//
//  Programmer: Cyrus Harrison
//  Creation:   January 30, 2008
//
// ****************************************************************************

void
avtMixedVariable::GetValuesForZone(int zone_id,
                                   avtMaterial *mat,
                                   std::vector<float> &vals)
{
    int n_mats = mat->GetNMaterials();
    vals.clear();
    for (int m=0; m<n_mats; m++)
        vals.push_back(0.0);
        
    const int *mat_list = mat->GetMatlist();
    const int *mix_mat  = mat->GetMixMat();
    const int *mix_next = mat->GetMixNext();
    
    // mixed case
    if(mat_list[zone_id] < 0)
    {
        int mix_idx = -mat_list[zone_id] - 1;
        while(mix_idx >=0)
        {
            vals[mix_mat[mix_idx]] = buffer[mix_idx];
            mix_idx = mix_next[mix_idx] -1;
        }
    } 
}


