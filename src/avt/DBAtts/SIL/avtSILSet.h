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
//                                avtSILSet.h                                //
// ************************************************************************* //

#ifndef AVT_SIL_SET_H
#define AVT_SIL_SET_H
#include <dbatts_exports.h>


#include <visitstream.h>

#include <string>
#include <vector>

#include <ref_ptr.h>


// ****************************************************************************
//  Class: avtSILSet
//
//  Purpose:
//      This is the representation of a set for a SIL.  It contains the name
//      of the set and links to all of the collections involving that set.
//
//  Programmer: Hank Childs
//  Creation:   March 8, 2001
//
//  Modifications:
//
//    Hank Childs, Mon May 21 09:12:17 PDT 2001
//    Add methods to make more usable.
//
//    Hank Childs, Fri Nov 15 10:25:02 PST 2002
//    Add concept of matrices.
//
//    Dave Bremer, Thu Dec 20 10:31:43 PST 2007
//    Removed unused data used in matrix operations, and consolidated
//    AddMatrixRow and AddMatrixColumn into AddMatrixMapOut.
//
//    Dave Bremer, Fri Jan 25 13:07:02 PST 2008
//    Made GetIdentifier() a const method.
// ****************************************************************************

class DBATTS_API avtSILSet
{
  public:
                            avtSILSet(const std::string &name, int id);
    virtual                ~avtSILSet() {;};

    const std::string      &GetName(void) const
                                        { return name; };

    void                    AddMapIn(int);
    void                    AddMapOut(int);
    void                    AddMatrixMapOut(int);

    int                     GetIdentifier(void) const { return id; };

    void                    Print(ostream &) const;

    const std::vector<int> &GetMapsIn(void) const  { return mapsIn; };
    const std::vector<int> &GetMapsOut(void) const { return allMapsOut; };
    const std::vector<int> &GetRealMapsOut(void) const { return mapsOut; };

  protected:
    std::string             name;
    int                     id;
    std::vector<int>        mapsIn;
    std::vector<int>        mapsOut;
    std::vector<int>        allMapsOut;
};


typedef ref_ptr<avtSILSet> avtSILSet_p;


#endif


