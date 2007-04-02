/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                         avtDatasetCollection.h                            //
// ************************************************************************* //

#ifndef AVT_DATASET_COLLECTION_H
#define AVT_DATASET_COLLECTION_H

#include <database_exports.h>

#include <vector>
#include <string>

#include <array_ref_ptr.h>
#include <void_ref_ptr.h>

#include <avtDataTree.h>
#include <avtMaterial.h>
#include <avtSpecies.h>

class     vtkDataSet;

class     avtMixedVariable;


// ****************************************************************************
//  Struct: avtDatasetCollection
//
//  Purpose:
//      This struct is only used by the avtGenericDatabase.  It is used to
//      assemble intermediate results in an organized way.  It is only pulled
//      off into its own file (as opposed to coupling it with the generic
//      database code) in an attempt to clean up portions of the code.
//
//  Programmer: Hank Childs
//  Creation:   July 4, 2002  [Split into its own file]
//
//  Modifications:
//    Brad Whitlock, Mon Jul 15 16:27:25 PST 2002
//    Added API.
//
//    Hank Childs, Sun Mar 13 10:41:17 PST 2005
//    Fixed problem with memory leak.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added code for setting/getting Var and Vars2nd
// ****************************************************************************

struct DATABASE_API avtDatasetCollection
{
  public:
                                            avtDatasetCollection(int);
    virtual                                ~avtDatasetCollection();
 
    avtDataTree_p                          *avtds;
    std::vector<std::vector<std::string> >  labels;
    std::vector<std::vector<std::string> >  matnames;
    std::vector<bool>                       needsMatSelect;
    std::vector<int>                        numMats;
    void                                    SetVar(const char *cvar)
                                                {  var = cvar; };
    const std::string                       GetVar() const
                                                { return var; };
    void                                    SetVars2nd(const std::vector<CharStrRef> &v2nd)
                                                { vars2nd = v2nd; };
    const std::vector<CharStrRef>          &GetVars2nd() const
                                                { return vars2nd; };
    void                                    SetNumMaterials(int dom,int nmats);
    int                                     GetNDomains(void)
                                                { return nDomains; };
    void                                    SetDataset(int, int, vtkDataSet *);
    vtkDataSet                             *GetDataset(int i, int j)
                                                { return vtkds[i][j]; };
    vtkDataSet                            **GetDatasetList(int i)
                                                { return vtkds[i]; };
    avtDataTree_p                           AssembleDataTree(
                                                           std::vector<int> &);
 
    void                                    SetMaterial(int i,avtMaterial *mat)
                                                { materials[i] = mat; };
    avtMaterial                            *GetMaterial(int i)
                                                { return materials[i]; };
    void                                    MaterialsShouldBeFreed(void)
                                              {materialsShouldBeFreed = true;};
 
    void                                    SetSpecies(int i, avtSpecies *spec)
                                                { species[i] = spec; };
    avtSpecies                             *GetSpecies(int i)
                                                { return species[i]; };
    void                                    SpeciesShouldBeFreed(void)
                                              {speciesShouldBeFreed = true;};
 
    void                                    AddMixVar(int i, void_ref_ptr mix);
    void_ref_ptr                            GetMixVar(int i, std::string);
    std::vector<void_ref_ptr>              &GetAllMixVars(int i);
    void                                    ReplaceMixVar(int i,
                                                          void_ref_ptr mix);

  private:
    std::string                             var;
    std::vector<CharStrRef>                 vars2nd;
    int                                     nDomains;
    vtkDataSet                           ***vtkds;
    avtMaterial                           **materials;
    avtSpecies                            **species;
    std::vector<void_ref_ptr>              *mixvars;
    bool                                    materialsShouldBeFreed;
    bool                                    speciesShouldBeFreed;
};


#endif


