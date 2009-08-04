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
//                             avtMixedVariable.h                            //
// ************************************************************************* //

#ifndef AVT_MIXED_VARIABLE_H
#define AVT_MIXED_VARIABLE_H

#include <string>
#include <vector>

#include <pipeline_exports.h>


// ****************************************************************************
//  Class: avtMixedVariable
//
//  Purpose:
//      Contains a mixed variable.  This can then be used by the MIR to assign
//      the mixed materials in a zone their exact values.
//
//  Programmer: Hank Childs
//  Creation:   September 27, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Tue Dec  4 13:43:54 PST 2001
//    Added const to constructor arguments.
//
//    Hank Childs, Thu Jul  4 13:02:54 PDT 2002
//    Added names for mixed variables.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Cyrus Harrison, Wed Jan 30 11:15:25 PST 2008
//    Added helper to obtain the per material values of a given zone.
//
// ****************************************************************************

class avtMaterial;

class PIPELINE_API avtMixedVariable
{
  public:
                        avtMixedVariable(const float *, int, std::string);
    virtual            ~avtMixedVariable();

    static void         Destruct(void *);

    const float        *GetBuffer(void) { return buffer; };
    int                 GetMixlen(void) { return mixlen; };
    const std::string  &GetVarname(void) { return varname; };
    void                GetValuesForZone(int zone_id,
                                         avtMaterial *mat,
                                         std::vector<float> &vals); 
  
  protected:
    std::string         varname;
    float              *buffer;
    int                 mixlen;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtMixedVariable(const avtMixedVariable &) {;};
    avtMixedVariable    &operator=(const avtMixedVariable &) { return *this; };
};


#endif


