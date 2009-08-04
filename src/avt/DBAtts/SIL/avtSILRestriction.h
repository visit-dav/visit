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
//                             avtSILRestriction.h                           //
// ************************************************************************* //

#ifndef AVT_SIL_RESTRICTION_H
#define AVT_SIL_RESTRICTION_H

#include <dbatts_exports.h>
 
#include <ref_ptr.h>
#include <string>

#include <avtSIL.h>
#include <avtTypes.h>


class CompactSILRestrictionAttributes;
class SILRestrictionAttributes;
class avtSILRestriction;

typedef ref_ptr<avtSILRestriction> avtSILRestriction_p;


// ****************************************************************************
//  Class: avtSILRestriction
//
//  Purpose:
//      A restriction to a SIL, this is how a portion of the mesh is specified.
//
//  Terminology:
//      SIL Restriction - A SIL restriction is a way to capture which part of
//          the SIL should be processed and which should not.  This is done by
//          turning sets on and off.  The leaf sets are the only ones that
//          actually get read in, but it is meaningful to discuss their
//          supersets in the SIL.
//      Top Set - The AVT pipeline can only handle one mesh at a time.  For
//          this reason an artificial constraint of one mesh per SIL
//          restriction has been imposed.  The one mesh is called the Top Set.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2001
//
//  Modifications:
//
//    Hank Childs, Sat May 19 10:30:27 PDT 2001
//    Re-wrote to be able to actually perform SIL restriction.
//
//    Brad Whitlock, Thu Jun 21 13:38:29 PST 2001
//    Added default constructor and other convenience methods.
//
//    Hank Childs, Fri Jul 20 11:47:57 PDT 2001
//    Add GetMaterials.
//
//    Hank Childs, Fri Sep 14 09:43:42 PDT 2001
//    Add UsesAllDomains.
//
//    Hank Childs, Wed Dec 19 08:50:47 PST 2001
//    Add support for species.
//
//    Hank Childs, Thu Feb  7 13:21:08 PST 2002
//    Added [Suspend|Enable]CorrectnessChecking.
//
//    Brad Whitlock, Thu Mar 7 14:10:57 PST 2002
//    Added SetFromCompatibleRestriction.
//
//    Hank Childs, Wed Dec  4 17:23:12 PST 2002
//    Pulled out many traversal routines for the 'Traverser class.
//
//    Brad Whitlock, Fri Jul 30 14:54:53 PST 2004
//    Added ReverseSet to reverse the selection of a set and all of its
//    subsets.
//
//    Cyrus Harrison, Tue Oct 28 14:34:26 PDT 2008
//    Added higher level EnsureRestrictionCorrectness wrapper to help keep
//    enum & material selection mutually exclusive. 
//
// ****************************************************************************

class DBATTS_API avtSILRestriction : public avtSIL
{
    friend                    class avtSILRestrictionTraverser;

  public:
                              avtSILRestriction(avtSILRestriction_p);
                              avtSILRestriction(avtSIL *);
                              avtSILRestriction(const
                                                SILRestrictionAttributes &);
                              avtSILRestriction(avtSIL *,
                                      const CompactSILRestrictionAttributes &);

    void                      SetTopSet(int);
    void                      SetTopSet(const char *);
    int                       GetTopSet() const { return topSet; };
    void                      TurnOnSet(int);
    void                      TurnOffSet(int);
    void                      ReverseSet(int);

    void                      TurnOnAll(void);
    void                      TurnOffAll(void);

    void                      SuspendCorrectnessChecking(void);
    void                      EnableCorrectnessChecking(void);
    bool                      SetFromCompatibleRestriction(
                                                          avtSILRestriction_p);

    void                      FastIntersect(avtSILRestriction_p);
    void                      Intersect(avtSILRestriction_p);
    void                      Union(avtSILRestriction_p);

    void                      RestrictDomains(const std::vector<int> &);
    void                      RestrictDomainsForLoadBalance(
                                                    const std::vector<int> &);

    SILRestrictionAttributes *MakeAttributes(void) const;
    CompactSILRestrictionAttributes *
                              MakeCompactAttributes(void) const;

    void                      Print(ostream &) const;

  protected:
    std::vector<unsigned char> useSet;
    int                       topSet;
    bool                      suspendCorrectnessChecking;

    void                      TurnBoolSet(int, bool);
    SetState                  EnsureRestrictionCorrectness();
    SetState                  EnsureRestrictionCorrectness(int setId);

    void                      GetLeafSets(int, std::vector<int> &) const;
    void                      GetSubsets(int, std::vector<int> &) const;

  private:
    void                      RestrictDomains(const std::vector<int> &,
                                              bool forLoadBalance);
                              avtSILRestriction();
                              avtSILRestriction(const avtSILRestriction &);
    const avtSILRestriction  &operator=(const avtSILRestriction &);
};


#endif


