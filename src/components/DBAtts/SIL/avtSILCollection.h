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
//                            avtSILCollection.h                             //
// ************************************************************************* //

#ifndef AVT_SIL_COLLECTION_H
#define AVT_SIL_COLLECTION_H
#include <dbatts_exports.h>


#include <visitstream.h>

#include <string>
#include <vector>

#include <ref_ptr.h>


class     avtSILNamespace;


typedef enum
{
    SIL_TOPOLOGY        = 0,
    SIL_PROCESSOR,     /* 1 */
    SIL_BLOCK,         /* 2 */
    SIL_DOMAIN,        /* 3 */
    SIL_ASSEMBLY,      /* 4 */
    SIL_MATERIAL,      /* 5 */
    SIL_BOUNDARY,      /* 6 */
    SIL_SPECIES,       /* 7 */
    SIL_ENUMERATION,   /* 8 */
    SIL_USERD          /* 9 */
} SILCategoryRole;


// ****************************************************************************
//  Class: avtSILCollection
//
//  Purpose:
//      This is the representation of a collection for a SIL.  It resembles a
//      SAF collection and draws its name from it.  A collection is a group of
//      maps that all have their domain come from a common set and all have the
//      same category.  Because the GUI would have to group maps into
//      collections anyways, there is no explicit information about a map -- it
//      is all stored in a collection.
//
//  Programmer: Hank Childs
//  Creation:   March 8, 2001
//
//  Modifications:
//    Dave Bremer, Thu Dec 20 14:23:29 PST 2007
//    Added the ContainsElement method
// ****************************************************************************

class DBATTS_API avtSILCollection
{
  public:
                            avtSILCollection(std::string, SILCategoryRole,
                                         int supersetIndex, avtSILNamespace *);
    virtual                ~avtSILCollection();

    int                     GetSupersetIndex(void) const
                                      { return supersetIndex; };
    const avtSILNamespace  *GetSubsets(void) const
                                      { return subsets; };
    const std::string      &GetCategory(void) const
                                      { return category; };
    SILCategoryRole         GetRole(void) const
                                      { return role; };

    void                    Print(ostream &) const;

    const std::vector<int> &GetSubsetList(void) const;

    bool                    ContainsElement(int e) const;

  protected:
    std::string             category;
    SILCategoryRole         role;
    int                     supersetIndex;
    avtSILNamespace        *subsets;
};


typedef ref_ptr<avtSILCollection> avtSILCollection_p;


#endif


