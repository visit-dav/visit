/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                           avtDatasetCollection.C                          //
// ************************************************************************* //

#include <avtDatasetCollection.h>

#include <vtkDataSet.h>

#include <avtMixedVariable.h>

#include <DebugStream.h>


using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtDatasetCollection constructor
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Nov 25 20:57:40 PST 2001
//    Add support for materials.
//
//    Hank Childs, Wed Dec  5 10:33:52 PST 2001
//    Add support for mixvars.
//
//    Hank Childs, Thu Jul  4 13:24:11 PDT 2002
//    Add support for multiple mixed variables.
//
//    Hank Childs, Mon Sep 30 15:06:42 PDT 2002
//    Initialize materials to NULL.
//
//    Hank Childs, Sun Mar 13 10:43:10 PST 2005
//    Initialize species to NULL.
//
// ****************************************************************************
 
avtDatasetCollection::avtDatasetCollection(int nd)
{
    nDomains = nd;
    labels.resize(nDomains);
    needsMatSelect.resize(nDomains);
    matnames.resize(nDomains);
    numMats.resize(nDomains);
    vtkds    = new vtkDataSet**[nDomains];
    avtds    = new avtDataTree_p[nDomains];
    materials = new avtMaterial*[nDomains];
    species = new avtSpecies*[nDomains];
 
    for (int i = 0 ; i < nDomains ; i++)
    {
        vtkds[i] = NULL;
        avtds[i] = NULL;
        materials[i] = NULL;
        species[i] = NULL;
    }
 
    mixvars   = new vector<void_ref_ptr>[nDomains];

    materialsShouldBeFreed = false;
    speciesShouldBeFreed = false;
}


// ****************************************************************************
//  Method: avtDatasetCollection destructor
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Nov 25 20:57:40 PST 2001
//    Add support for materials.
//
//    Hank Childs, Wed Dec  5 10:33:52 PST 2001
//    Add support for mixvars.
//
//    Hank Childs, Thu Jul  4 13:24:11 PDT 2002
//    Add support for multiple mixed variables.
//
//    Hank Childs, Sun Mar 13 10:43:53 PST 2005
//    Free up materials and species if we own them.
//
// ****************************************************************************
 
avtDatasetCollection::~avtDatasetCollection()
{
    for (int i = 0 ; i < nDomains ; i++)
    {
        if (vtkds[i] != NULL)
        {
            for (int j = 0 ; j < numMats[i] ; j++)
            {
                if (vtkds[i][j] != NULL)
                {
                    vtkds[i][j]->Delete();
                    vtkds[i][j] = NULL;
                }
            }
            delete [] vtkds[i];
            vtkds[i] = NULL;
        }
        avtds[i] = NULL;

        if (materialsShouldBeFreed && materials[i] != NULL)
            delete materials[i];
        if (speciesShouldBeFreed && species[i] != NULL)
            delete species[i];
    }
    delete [] vtkds;
    vtkds = NULL;
    delete [] avtds;
    avtds = NULL;
    delete [] materials;
    materials = NULL;
    delete [] species;
    species = NULL;
    delete [] mixvars;
    mixvars = NULL;
}


// ****************************************************************************
//  Method: avtDatasetCollection::SetNumMaterials
//
//  Purpose:
//      Sets the number of materials for a domain.
//
//  Arguments:
//      dom     The domain number.
//      nmats   The number of materials.
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2001
//
// ****************************************************************************
 
void
avtDatasetCollection::SetNumMaterials(int dom, int nmats)
{
    vtkds[dom] = new vtkDataSet*[nmats];
    for (int i = 0 ; i < nmats ; i++)
    {
        vtkds[dom][i] = NULL;
    }
    labels[dom].resize(nmats);
    numMats[dom] = nmats;
}
 
 
// ****************************************************************************
//  Method: avtDatasetCollection::SetDataset
//
//  Purpose:
//      Provides a wrapper for setting a dataset.
//
//  Arguments:
//      dom     The domain index.
//      mat     The material index.
//      ds      The VTK dataset.
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2001
//
// ****************************************************************************
 
void
avtDatasetCollection::SetDataset(int dom, int mat, vtkDataSet *ds)
{
    if (vtkds[dom][mat] != NULL)
    {
        vtkds[dom][mat]->Delete();
    }
    vtkds[dom][mat] = ds;
    if (vtkds[dom][mat] != NULL)
    {
        vtkds[dom][mat]->Register(NULL);
    }
}


// ****************************************************************************
//  Method: avtDatasetCollection::AddMixVar
//
//  Purpose:
//      Adds the mixed variable to the dataset collection.
//
//  Arguments:
//      dom     The domain number.
//      mv      The actual mixed variable (stored as a void_ref_ptr).
//
//  Programmer: Hank Childs
//  Creation:   July 4, 2002
//
// ****************************************************************************

void
avtDatasetCollection::AddMixVar(int dom, void_ref_ptr mv)
{
    mixvars[dom].push_back(mv);
}


// ****************************************************************************
//  Method: avtDatasetCollection::GetMixVar
//
//  Purpose:
//      Gets the mixed variable corresponding to the domain and name.
//
//  Arguments:
//      dom     The domain number.
//      name    The name of the mixvar.
//
//  Returns:    The mixed variable as a void reference.
//
//  Programmer: Hank Childs
//  Creation:   July 4, 2002
//
// ****************************************************************************

void_ref_ptr
avtDatasetCollection::GetMixVar(int dom, std::string name)
{
    for (size_t i = 0 ; i < mixvars[dom].size() ; i++)
    {
        avtMixedVariable *mixvar = (avtMixedVariable *) *mixvars[dom][i];
        if (mixvar->GetVarname() == name)
        {
            return mixvars[dom][i];
        }
    }

    return void_ref_ptr();
}


// ****************************************************************************
//  Method: avtDatasetCollection::GetAllMixVars
//
//  Purpose:
//      Gets every mixed variable associated with a domain.
//
//  Arguments:
//      dom     The domain number.
//
//  Returns:    The mixed variables as a vector of void references.
//
//  Programmer: Hank Childs
//  Creation:   July 4, 2002
//
// ****************************************************************************

vector<void_ref_ptr> &
avtDatasetCollection::GetAllMixVars(int dom)
{
    return mixvars[dom];
}


// ****************************************************************************
//  Method: avtDatasetCollection::ReplaceMixVar
//
//  Purpose:
//      Replaces an entry in the dataset collection.
//
//  Arguments:
//      dom     The domain number.
//      mv      The new mixvar.
//
//  Programmer: Hank Childs
//  Creation:   July 4, 2002
//
// ****************************************************************************

void
avtDatasetCollection::ReplaceMixVar(int dom, void_ref_ptr mix)
{
    avtMixedVariable *mv = (avtMixedVariable *) *mix;

    for (size_t i = 0 ; i < mixvars[dom].size() ; i++)
    {
        avtMixedVariable *nmv = (avtMixedVariable *) *mixvars[dom][i];
        if (nmv->GetVarname() == mv->GetVarname())
        {
            mixvars[dom][i] = mix;
            return;
        }
    }

    //
    // Here's how we get to this point.  We start off with no mixed variable,
    // but when we communicate ghost zones, we end up getting a few ghost
    // zones along the boundary and a mixed variable to go with it.  
    // Just add the mixed variable and pretend nothing happened.
    //
    AddMixVar(dom, mix);
}



// ****************************************************************************
//  Method: avtDatasetCollection::AssembleDataTree
//
//  Purpose:
//      Assembles a data tree from its data members.
//
//  Arguments:
//      domains   A map that takes our 0-indexing scheme to domain numbers.
//
//  Returns:      The constructed data tree.
//
//  Programmer:   Hank Childs
//  Creation:     October 26, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Aug 19 16:35:48 PST 2004
//    Made it use a different constructor if the size of the labels array
//    is zero as part of a fix to prevent VisIt from crashing on Windows using
//    the new compiler. I have no idea how it was working elsewhere.
//
// ****************************************************************************
 
avtDataTree_p
avtDatasetCollection::AssembleDataTree(std::vector<int> &domains)
{
    for (int i = 0 ; i < nDomains ; i++)
    {
        if (*avtds[i] == NULL)
        {
            if(labels[i].size() > 0)
            {
                avtds[i] = new avtDataTree(numMats[i], vtkds[i], domains[i],
                                           labels[i]);
            }
            else
            {
                avtds[i] = new avtDataTree(numMats[i], vtkds[i], domains[i]);
            }
        }
    }
 
    return new avtDataTree(nDomains, avtds);
}
