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
//                         avtSourceFromAVTDataset.h                         //
// ************************************************************************* //

#ifndef AVT_SOURCE_FROM_AVT_DATASET_H
#define AVT_SOURCE_FROM_AVT_DATASET_H

#include <pipeline_exports.h>


#include <avtInlinePipelineSource.h>
#include <avtOriginatingDatasetSource.h>


// ****************************************************************************
//  Class: avtSourceFromAVTDataset
//
//  Purpose:
//      A source object (pipeline terminator) that is created from an AVT
//      dataset.
//
//  Programmer: Hank Childs
//  Creation:   June 19, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep 11 14:58:14 PDT 2001
//    Re-worked inheritance hierarchy.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Fri Dec 31 11:02:02 PST 2004
//    Allow the data tree to be reset.
//
// ****************************************************************************

class PIPELINE_API avtSourceFromAVTDataset
     : virtual public avtOriginatingDatasetSource,
       virtual public avtInlinePipelineSource
{
  public:
                          avtSourceFromAVTDataset(avtDataset_p ds);
    virtual              ~avtSourceFromAVTDataset();

    void                  ResetTree(avtDataTree_p t) { tree = t; };

  protected:
    avtDataTree_p         tree;

    virtual bool          FetchDataset(avtDataRequest_p,avtDataTree_p &);
};


#endif


