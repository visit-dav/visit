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
//                           avtPointSelection.h                             //
// ************************************************************************* //

#ifndef AVT_POINT_SELECTION_H
#define AVT_POINT_SELECTION_H 

#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

#include <vector>


// ****************************************************************************
//  Class: avtPointSelection
//
//  Purpose: 
//      Specify data selection using a point location.
//
//  Programmer: Hank Childs
//  Creation:   March 22, 2009
//
// ****************************************************************************

class PIPELINE_API avtPointSelection : public avtDataSelection 
{
  public:
                            avtPointSelection();
    virtual                ~avtPointSelection();

    static void             Destruct(void *);

    virtual const char *    GetType() const
                                { return "Point Selection"; }; 

    void                    SetPoint(const double *pt_) 
                                                  { pt[0] = pt_[0];
                                                    pt[1] = pt_[1];
                                                    pt[2] = pt_[2]; };
    const double           *GetPoint(void) const { return pt; };

    bool                    operator==(const avtPointSelection &) const;

  private:
    double                  pt[3];

    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                    avtPointSelection(const avtPointSelection &) {;};
    avtPointSelection &operator=(const avtPointSelection &) { return *this; };
};

typedef ref_ptr<avtPointSelection> avtPointSelection_p;


#endif


