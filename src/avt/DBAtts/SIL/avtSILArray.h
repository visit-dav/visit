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
//                               avtSILArray.h                              //
// ************************************************************************* //

#ifndef AVT_SIL_ARRAY_H
#define AVT_SIL_ARRAY_H

#include <dbatts_exports.h>
#include <string>
#include <ref_ptr.h>
#include <avtSILSet.h>
#include <avtSILCollection.h>
#include <avtTypes.h>
#include <vectortypes.h>

class SILArrayAttributes;


// ****************************************************************************
//  Class: avtSILArray
//
//  Purpose:    
//      Conceptually it contains a number of avtSILSets that would be created
//      with names that follow a pattern, and which can be grouped into one
//      collection.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 12:12:30 PST 2007
//
//  Modifications:
//    Dave Bremer, Fri Jan 25 13:07:02 PST 2008
//    Added GetSILSetID method.
//
//    Dave Bremer, Fri Mar 28 19:38:29 PDT 2008
//    Modified to handle an explicit list of names.
// ****************************************************************************

class DBATTS_API avtSILArray
{
  public:
                        avtSILArray(const std::string &pfx, int nSets, 
                                    int firstSetName,  bool uniqueIDs,
                                    const std::string &cat,
                                    SILCategoryRole r, int parent);
                        avtSILArray(const stringVector &names, 
                                    int nSets, int firstSetName, 
                                    bool uniqueIDs, const std::string &cat,
                                    SILCategoryRole r, int parent);
                        avtSILArray(const SILArrayAttributes &atts);
    virtual            ~avtSILArray() {;};


    int                 GetNumSets() const   { return iNumSets; }
    avtSILSet_p         GetSILSet(int index) const;
    avtSILCollection_p  GetSILCollection() const;

    int                 GetSILSetID(int index) const;
    int                 GetParent() const    { return iColParent; }
    void                SetFirstSetIndex(int s)    {iFirstSet = s;}
    void                SetCollectionIndex(int c)  {iColIndex = c;}
    SetState            GetSetState(const std::vector<unsigned char> &useSet) const;
    void                TurnSet(std::vector<unsigned char> &useSet,
                                SetState val, bool forLoadBalance) const;
    int                 GetSetIndex(const std::string &name) const;

    SILArrayAttributes *MakeAttributes(void) const;

  protected:
    //For making the sets
    std::string      prefix;         //prefix or template for the block name
    stringVector     names;  //explicit list of block names

    int              iNumSets;
    int              iFirstSetName;  //Often 0 or 1, depending for example on a 
                                     //code's block numbering preference
    bool             bUseUniqueIDs;

    //For making the collection
    int              iFirstSet;      //Index of first set in the collection, 
                                     //in the containing SIL's index space.
    int              iColIndex;      //Index of this collection, in the 
                                     //containing SIL.
    std::string      category;
    SILCategoryRole  role;
    int              iColParent;

};


typedef ref_ptr<avtSILArray> avtSILArray_p;


#endif



