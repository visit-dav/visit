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
//                         avtIsolevelsSelection.h                           //
// ************************************************************************* //

#ifndef AVT_ISOLEVELS_SELECTION_H
#define AVT_ISOLEVELS_SELECTION_H

#include <pipeline_exports.h>

#include <float.h>
#include <string>
#include <vector>
 
#include <ref_ptr.h>

#include <avtDataSelection.h>


// ****************************************************************************
//  Class: avtIsolevelsSelection
//
//  Purpose: 
//      Specify a data selection consisting of multiple isolevels.
// 
//  Programmer: Hank Childs
//  Creation:   January 5, 2008
//
// ****************************************************************************

class PIPELINE_API avtIsolevelsSelection : public avtDataSelection 
{
  public:
                            avtIsolevelsSelection();
                            avtIsolevelsSelection(const std::string _var, 
                                            const std::vector<double> &values);
    virtual                ~avtIsolevelsSelection();

    virtual const char *    GetType() const { return "Isolevels Selection"; }; 

    void                    SetVariable(const std::string _var)
                                { var = _var; };
    void                    SetIsolevels(const std::vector<double> &_isolevels)
                                { isolevels = _isolevels; };

    std::string             GetVariable() const
                                { return var; };
    const std::vector<double> &GetIsolevels() const
                                { return isolevels; };

  private:
    std::string var;
    std::vector<double> isolevels;

    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                       avtIsolevelsSelection(const avtIsolevelsSelection&) {;};
    avtIsolevelsSelection  &operator=(const avtIsolevelsSelection &) 
                                                             { return *this; };
};

typedef ref_ptr<avtIsolevelsSelection> avtIsolevelsSelection_p;


#endif


