/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                                   avtSIL.h                                //
// ************************************************************************* //

#ifndef AVT_SIL_H
#define AVT_SIL_H

#include <dbatts_exports.h>

#include <visitstream.h>
#include <vector>

#include <avtSILCollection.h>
#include <avtSILMatrix.h>
#include <avtSILSet.h>


class   SILAttributes;


// ****************************************************************************
//  Class: avtSIL
//
//  Purpose:
//      Defines a Subset Inclusion Lattice (SIL).  A subset inclusion lattice
//      consists of sets and collections.  A set is some subset of the whole
//      and collection is all of the maps of a certain category that have 
//      elements from a certain set in their domain.
//
//  Terminology:
//      SIL - The subset inclusion lattice.  This has ideas similar to the SIL
//          of SAF, but is "dummied down" so that it will never contain problem
//          size data.  The SIL is a language for restricting sets (meshes).
//          The SIL is often represented in graphical form.  It is a dag
//          (directed acyclic graph).
//      Sets - These are meshes in actuality.  They are typically represented
//          as the vertices in the dag.
//      Maps - Maps denote subset relations.  They go between two sets, the
//          superset and one of its subsets.  There is a category associated
//          with a map.  The category is a name (string), but it has a meaning
//          (referred to as a role) which indicates the type of subsetting -
//          material, domain, processor, etc.
//      Collections - Maps with the same superset and category role typically
//          partition the superset.  These maps taken as a group are called a
//          collection.  For VisIt's representation, it is convenient to
//          consider collections rather than maps.
//      Namespace - Although many elements in SAF can be thought of as sets,
//          representing them this way would cause a SIL to become problem
//          size.  For this reason, a map's range (range meaning the range of 
//          a function) can be represented as an enumeration or as a range of
//          numbers.
//          
//  Programmer: Hank Childs
//  Creation:   March 8, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Jun 20 17:08:45 PST 2001
//    I added convenience methods for returning sets from the SIL.
//
//    Eric Brugger, Thu Nov 29 12:59:30 PST 2001
//    I added const to the methods GetSILSet and GetSILCollection.
//
//    Kathleen Bonnell, Thu Aug 8 17:48:38 PDT 2002  
//    I added the method GetCollectionIndex.
//
//    Hank Childs, Thu Nov 14 15:43:28 PST 2002
//    Added concept of SIL matrices.
//
//    Kathleen Bonnell, Thu Jan 26 07:44:00 PST 2006 
//    Added int arg to GetCollectionIndex, added overloaded GetSetIndex
//    with an int arg.
//
// ****************************************************************************

class DBATTS_API avtSIL
{
  public:
                                      avtSIL();
                                      avtSIL(avtSIL *);
                                      avtSIL(const SILAttributes &);
    virtual                          ~avtSIL() {;};
    const avtSIL                     &operator=(const avtSIL &);

    void                              AddCollection(avtSILCollection_p);
    int                               AddSubset(avtSILSet_p);
    int                               AddWhole(avtSILSet_p);
    void                              AddMatrix(avtSILMatrix_p);

    int                               GetSetIndex(std::string) const;
    int                               GetSetIndex(std::string, int) const;
    int                               GetCollectionIndex(std::string, int) const;

    void                              Print(ostream &) const;
    void                              Print(ostream &,
                                          std::vector< std::string > perSetInfo,
                                          std::vector< std::string > perCollInfo,
                                          std::vector< std::string > perMatrixInfo) const;

    SILAttributes                    *MakeSILAttributes(void) const;

    const std::vector<int>           &GetWholes(void) const
                                          { return wholesList; }

    avtSILSet_p                       GetSILSet(int index) const;
    avtSILCollection_p                GetSILCollection(int index) const;

    int                               GetNumSets() const;
    int                               GetNumCollections() const;

  protected:
    std::vector<bool>                 isWhole;
    std::vector<int>                  wholesList;

    int                               AddSet(avtSILSet_p);
    int                               GetNumRealSets() const
                                           { return sets.size(); };
    bool                              RealCollection(int ind)
                                        { return (ind < collections.size()); };
    void                              TranslateCollectionInfo(int,
                                                      avtSILMatrix_p &, int &);

  private:
                                      avtSIL(const avtSIL &) {;};
    void                              ReAddMatrix(avtSILMatrix_p);

    std::vector<avtSILSet_p>          sets;
    std::vector<avtSILCollection_p>   collections;
    std::vector<avtSILMatrix_p>       matrices;
    bool                              haveAddedMatrices;
};


#endif


