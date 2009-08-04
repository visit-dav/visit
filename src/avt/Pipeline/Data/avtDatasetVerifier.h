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
//                            avtDatasetVerifier.h                           //
// ************************************************************************* //

#ifndef AVT_DATASET_VERIFIER_H
#define AVT_DATASET_VERIFIER_H

#include <pipeline_exports.h>

#include <avtDataTree.h>

class     vtkDataArray;
class     vtkDataSet;
class     vtkDataSetAttributes;


// ****************************************************************************
//  Class: avtDatasetVerifier
//
//  Purpose:
//      Looks through an avtDataTree and verifies that each dataset looks
//      reasonable.  Issues warnings and corrects them if they are not.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Fri Jan  9 09:36:01 PST 2004
//    Add a routine that is accessible for the generic database.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Fri Nov 12 08:22:29 PST 2004 
//    Changed arguments for CorrectVarMismatch, to make the method able to
//    handle more var types. 
//
//    Hank Childs, Tue Jul  5 16:22:56 PDT 2005
//    Add variable name to IssueWarning call.
//
// ****************************************************************************

class PIPELINE_API avtDatasetVerifier
{
  public:
                avtDatasetVerifier();
    virtual    ~avtDatasetVerifier();

    void        VerifyDatasets(int, vtkDataSet **, std::vector<int> &domains);

  protected:
    bool        issuedWarningForVarMismatch;

    void        VerifyDataset(vtkDataSet *, int);
    void        CorrectVarMismatch(vtkDataArray *, vtkDataSetAttributes*, int); 

    void        IssueVarMismatchWarning(int, int, bool, int, const char *);
};


#endif


