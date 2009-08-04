/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                        avtSILRestrictionTraverser.h                       //
// ************************************************************************* //

#ifndef AVT_SIL_RESTRICTION_TRAVERSER_H
#define AVT_SIL_RESTRICTION_TRAVERSER_H

#include <dbatts_exports.h>

#include <vector>

#include <avtSILRestriction.h>


// ****************************************************************************
//  Class: avtSILRestrictionTraverser
//
//  Purpose:
//      A module whose devoted purpose is to traverse SIL restriction objects.
//      The purpose of putting this in its own module is solely to make the
//      interfaces cleaner.
//
//  Programmer: Hank Childs
//  Creation:   November 22, 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue Jun  1 17:00:17 PDT 2004
//    Added method 'UsesAllMaterials'.
//
//    Jeremy Meredith, Mon Aug 28 16:19:48 EDT 2006
//    Added support for scalar enumerations.
//
//    Mark C. Miller, Wed Mar 26 16:23:27 PDT 2008
//    Added support for scalar enumerations w/graphs
//
//    Hank Childs, Sat Nov 15 18:02:46 CST 2008
//    Add a data member that allows for material selection queries to go much
//    faster in simple cases.
//
// ****************************************************************************

class DBATTS_API avtSILRestrictionTraverser
{
  public:
                               avtSILRestrictionTraverser(avtSILRestriction_p);

    bool                       Equal(avtSILRestriction_p);

    void                       GetDomainList(std::vector<int> &);
    void                       GetDomainListAllProcs(std::vector<int> &);
    bool                       UsesAllData(void);
    bool                       UsesAllDomains(void);
 
    bool                       UsesData(int setId) const;
    SetState                   UsesSetData(int setId) const;

    const std::vector<std::string> &
                               GetMaterials(int, bool &);
    bool                       GetSpecies(std::vector<bool> &);
    bool                       UsesAllMaterials(void);

    int                        GetEnumerationCount();
    bool                       GetEnumerationFromGraph(int, std::vector<bool> &,
                                              std::string&);
    bool                       GetEnumeration(int, std::vector<bool> &,
                                              std::string&);
    void                       GetEnumerationMinMaxSetIds(int parentId,
                                   int *minId, int *maxId);


  protected:
    avtSILRestriction_p        silr;

    bool                       preparedForMaterialSearches;
    bool                       noMaterials;
    std::vector<MaterialList>  materialList;
    std::vector<int>           materialListForChunk;
    std::vector<bool>          shouldMatSelect;
 
    void                       PrepareForMaterialSearches(void);
    void                       AddMaterialList(int, MaterialList &, bool);

  private:
                               avtSILRestrictionTraverser() {;};
                               avtSILRestrictionTraverser(
                                       const avtSILRestrictionTraverser &) {;};
    const avtSILRestrictionTraverser &    
                               operator=(const avtSILRestrictionTraverser &t)
                                    { return t; };
    void                       GetDomainList(std::vector<int> &, bool allProcs);
};


#endif


